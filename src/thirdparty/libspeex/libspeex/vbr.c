#include "vbr.h"
#include <math.h>


#define sqr(x) ((x)*(x))

#define MIN_ENERGY 6000
#define NOISE_POW .3


float vbr_nb_thresh[9][11]={
   {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}, /*   CNG   */
   { 3.5,  2.5,  2.0,  1.2,  0.5,  0.0, -0.5, -0.7, -0.8, -0.9, -1.0}, /*  2 kbps */
   {10.0,  6.5,  5.2,  4.5,  3.9,  3.5,  3.0,  2.5,  2.3,  1.8,  1.0}, /*  6 kbps */
   {11.0,  8.8,  7.5,  6.5,  5.0,  3.9,  3.9,  3.9,  3.5,  3.0,  1.0}, /*  8 kbps */
   {11.0, 11.0,  9.9,  9.0,  8.0,  7.0,  6.5,  6.0,  5.0,  4.0,  2.0}, /* 11 kbps */
   {11.0, 11.0, 11.0, 11.0,  9.5,  9.0,  8.0,  7.0,  6.5,  5.0,  3.0}, /* 15 kbps */
   {11.0, 11.0, 11.0, 11.0, 11.0, 11.0,  9.5,  8.5,  8.0,  6.5,  4.0}, /* 18 kbps */
   {11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0,  9.8,  7.5,  5.5}, /* 24 kbps */ 
   { 8.0,  5.0,  3.7,  3.0,  2.5,  2.0,  1.8,  1.5,  1.0,  0.0,  0.0}  /*  4 kbps */
};


float vbr_hb_thresh[5][11]={
   {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}, /* silence */
   {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}, /*  2 kbps */
   {11.0, 11.0,  9.5,  8.5,  7.5,  6.0,  5.0,  3.9,  3.0,  2.0,  1.0}, /*  6 kbps */
   {11.0, 11.0, 11.0, 11.0, 11.0,  9.5,  8.7,  7.8,  7.0,  6.5,  4.0}, /* 10 kbps */
   {11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0, 11.0,  9.8,  7.5,  5.5}  /* 18 kbps */ 
};

float vbr_uhb_thresh[2][11]={
   {-1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0}, /* silence */
   { 3.9,  2.5,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0, -1.0}  /*  2 kbps */
};

void vbr_init(VBRState *vbr)
{
   int i;

   vbr->average_energy=0;
   vbr->last_energy=1;
   vbr->accum_sum=0;
   vbr->energy_alpha=.1;
   vbr->soft_pitch=0;
   vbr->last_pitch_coef=0;
   vbr->last_quality=0;

   vbr->noise_accum = .05*pow(MIN_ENERGY, NOISE_POW);
   vbr->noise_accum_count=.05;
   vbr->noise_level=vbr->noise_accum/vbr->noise_accum_count;
   vbr->consec_noise=0;


   for (i=0;i<VBR_MEMORY_SIZE;i++)
      vbr->last_log_energy[i] = log(MIN_ENERGY);
}
float vbr_analysis(VBRState *vbr, float *sig, int len, int pitch, float pitch_coef)
{
   int i;
   float ener=0, ener1=0, ener2=0;
   float qual=7;
   int va;
   float log_energy;
   float non_st=0;
   float voicing;
   float pow_ener;

   for (i=0;i<len>>1;i++)
      ener1 += sig[i]*sig[i];

   for (i=len>>1;i<len;i++)
      ener2 += sig[i]*sig[i];
   ener=ener1+ener2;

   log_energy = log(ener+MIN_ENERGY);
   for (i=0;i<VBR_MEMORY_SIZE;i++)
      non_st += sqr(log_energy-vbr->last_log_energy[i]);
   non_st =  non_st/(30*VBR_MEMORY_SIZE);
   if (non_st>1)
      non_st=1;

   voicing = 3*(pitch_coef-.4)*fabs(pitch_coef-.4);
   vbr->average_energy = (1-vbr->energy_alpha)*vbr->average_energy + vbr->energy_alpha*ener;
   vbr->noise_level=vbr->noise_accum/vbr->noise_accum_count;
   pow_ener = pow(ener,NOISE_POW);
   if (vbr->noise_accum_count<.06 && ener>MIN_ENERGY)
      vbr->noise_accum = .05*pow_ener;

   if ((voicing<.3 && non_st < .2 && pow_ener < 1.2*vbr->noise_level)
       || (voicing<.3 && non_st < .05 && pow_ener < 1.5*vbr->noise_level)
       || (voicing<.4 && non_st < .05 && pow_ener < 1.2*vbr->noise_level)
       || (voicing<0 && non_st < .05))
   {
      float tmp;
      va = 0;
      vbr->consec_noise++;
      if (pow_ener > 3*vbr->noise_level)
         tmp = 3*vbr->noise_level;
      else 
         tmp = pow_ener;
      if (vbr->consec_noise>=4)
      {
         vbr->noise_accum = .95*vbr->noise_accum + .05*tmp;
         vbr->noise_accum_count = .95*vbr->noise_accum_count + .05;
      }
   } else {
      va = 1;
      vbr->consec_noise=0;
   }

   if (pow_ener < vbr->noise_level && ener>MIN_ENERGY)
   {
      vbr->noise_accum = .95*vbr->noise_accum + .05*pow_ener;
      vbr->noise_accum_count = .95*vbr->noise_accum_count + .05;      
   }

   /* Checking for very low absolute energy */
   if (ener < 30000)
   {
      qual -= .7;
      if (ener < 10000)
         qual-=.7;
      if (ener < 3000)
         qual-=.7;
   } else {
      float short_diff, long_diff;
      short_diff = log((ener+1)/(1+vbr->last_energy));
      long_diff = log((ener+1)/(1+vbr->average_energy));
      /*fprintf (stderr, "%f %f\n", short_diff, long_diff);*/

      if (long_diff<-5)
         long_diff=-5;
      if (long_diff>2)
         long_diff=2;

      if (long_diff>0)
         qual += .6*long_diff;
      if (long_diff<0)
         qual += .5*long_diff;
      if (short_diff>0)
      {
         if (short_diff>5)
            short_diff=5;
         qual += .5*short_diff;
      }
      /* Checking for energy increases */
      if (ener2 > 1.6*ener1)
         qual += .5;
   }
   vbr->last_energy = ener;
   vbr->soft_pitch = .6*vbr->soft_pitch + .4*pitch_coef;
   qual += 2.2*((pitch_coef-.4) + (vbr->soft_pitch-.4));

   if (qual < vbr->last_quality)
      qual = .5*qual + .5*vbr->last_quality;
   if (qual<4)
      qual=4;
   if (qual>10)
      qual=10;
   if (vbr->consec_noise>=3)
      qual=4;

   if (vbr->consec_noise)
      qual -= 1.0 * (log(3.0 + vbr->consec_noise)-log(3));
   if (qual<0)
      qual=0;
   
   if (ener<60000)
   {
      if (vbr->consec_noise>2)
         qual-=0.5*(log(3.0 + vbr->consec_noise)-log(3));
      if (ener<10000&&vbr->consec_noise>2)
         qual-=0.5*(log(3.0 + vbr->consec_noise)-log(3));
      if (qual<0)
         qual=0;
      qual += .3*log(ener/60000.0);
   }
   if (qual<-1)
      qual=-1;
   vbr->last_pitch_coef = pitch_coef;
   vbr->last_quality = qual;

   for (i=VBR_MEMORY_SIZE-1;i>0;i--)
      vbr->last_log_energy[i] = vbr->last_log_energy[i-1];
   vbr->last_log_energy[0] = log_energy;
   return qual;
}
void vbr_destroy(VBRState *vbr)
{
}
