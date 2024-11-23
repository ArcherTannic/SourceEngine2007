#include "quant_lsp.h"
#include <math.h>

extern int lsp_nb_vqid[64];
static float quant_weight[MAX_LSP_SIZE];

/* Note: x is modified*/
static int lsp_quant(float *x, signed char *cdbk, int nbVec, int nbDim)
{
   int i,j;
   float dist, tmp;
   float best_dist=0;
   int best_id=0;
   signed char *ptr=cdbk;
   for (i=0;i<nbVec;i++)
   {
      dist=0;
      for (j=0;j<nbDim;j++)
      {
         tmp=(x[j]-*ptr++);
         dist+=tmp*tmp;
      }
      if (dist<best_dist || i==0)
      {
         best_dist=dist;
         best_id=i;
      }
   }

   for (j=0;j<nbDim;j++)
      x[j] -= cdbk[best_id*nbDim+j];
    
   return best_id;
}

/* Note: x is modified*/
static int lsp_weight_quant(float *x, float *weight, signed char *cdbk, int nbVec, int nbDim)
{
   int i,j;
   float dist, tmp;
   float best_dist=0;
   int best_id=0;
   signed char *ptr=cdbk;
   for (i=0;i<nbVec;i++)
   {
      dist=0;
      for (j=0;j<nbDim;j++)
      {
         tmp=(x[j]-*ptr++);
         dist+=weight[j]*tmp*tmp;
      }
      if (dist<best_dist || i==0)
      {
         best_dist=dist;
         best_id=i;
      }
   }
   
   for (j=0;j<nbDim;j++)
      x[j] -= cdbk[best_id*nbDim+j];
   return best_id;
}


void lsp_quant_nb(float *lsp, float *qlsp, int order, SpeexBits *bits)
{
   int i;
   float tmp1, tmp2;
   int id;

   for (i=0;i<order;i++)
      qlsp[i]=lsp[i];

   quant_weight[0] = 1/(qlsp[1]-qlsp[0]);
   quant_weight[order-1] = 1/(qlsp[order-1]-qlsp[order-2]);
   for (i=1;i<order-1;i++)
   {
#if 1
       tmp1 = static_cast<float>(1 / ((0.15f + qlsp[i] - qlsp[i - 1]) * (0.15f + qlsp[i] - qlsp[i - 1])));
       tmp2 = static_cast<float>(1 / ((0.15f + qlsp[i + 1] - qlsp[i]) * (0.15f + qlsp[i + 1] - qlsp[i])));
#else
      tmp1 = 1/(qlsp[i]-qlsp[i-1]);
      tmp2 = 1/(qlsp[i+1]-qlsp[i]);
#endif
      quant_weight[i] = tmp1 > tmp2 ? tmp1 : tmp2;
   }
   for (i=0;i<order;i++)
       qlsp[i] -= static_cast<float>(0.25f * i + 0.25f);
   for (i=0;i<order;i++)
      qlsp[i]*=256;

   id = lsp_quant(qlsp, cdbk_nb, NB_CDBK_SIZE, order);
   speex_bits_pack(bits, id, 6);

   for (i=0;i<order;i++)
      qlsp[i]*=2;
 
   id = lsp_weight_quant(qlsp, quant_weight, cdbk_nb_low1, NB_CDBK_SIZE_LOW1, 5);
   speex_bits_pack(bits, id, 6);

   for (i=0;i<5;i++)
      qlsp[i]*=2;

   id = lsp_weight_quant(qlsp, quant_weight, cdbk_nb_low2, NB_CDBK_SIZE_LOW2, 5);
   speex_bits_pack(bits, id, 6);

   id = lsp_weight_quant(qlsp+5, quant_weight+5, cdbk_nb_high1, NB_CDBK_SIZE_HIGH1, 5);
   speex_bits_pack(bits, id, 6);

   for (i=5;i<10;i++)
      qlsp[i]*=2;

   id = lsp_weight_quant(qlsp+5, quant_weight+5, cdbk_nb_high2, NB_CDBK_SIZE_HIGH2, 5);
   speex_bits_pack(bits, id, 6);

   for (i=0;i<order;i++)
      qlsp[i]*=.00097656;

   for (i=0;i<order;i++)
      qlsp[i]=lsp[i]-qlsp[i];
}

void lsp_unquant_nb(float* lsp, int order, SpeexBits* bits)
{
    int i, id;

    // Инициализация массива lsp
    for (i = 0; i < order; i++)
        lsp[i] = 0.25f * i + 0.25f;

    // Первый блок
    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 10; i++)
        lsp[i] += 0.0039062f * cdbk_nb[id * 10 + i];

    // Второй блок
    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i] += 0.0019531f * cdbk_nb_low1[id * 5 + i];

    // Третий блок
    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i] += 0.00097656f * cdbk_nb_low2[id * 5 + i];

    // Четвертый блок
    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i + 5] += 0.0019531f * cdbk_nb_high1[id * 5 + i];

    // Пятый блок
    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i + 5] += 0.00097656f * cdbk_nb_high2[id * 5 + i];
}

void lsp_quant_lbr(float *lsp, float *qlsp, int order, SpeexBits *bits)
{
   int i;
   float tmp1, tmp2;
   int id;

   for (i=0;i<order;i++)
      qlsp[i]=lsp[i];

   quant_weight[0] = 1/(qlsp[1]-qlsp[0]);
   quant_weight[order-1] = 1/(qlsp[order-1]-qlsp[order-2]);
   for (i=1;i<order-1;i++)
   {
#if 1
       tmp1 = 1 / ((0.15f + qlsp[i] - qlsp[i - 1]) * (0.15f + qlsp[i] - qlsp[i - 1]));
       tmp2 = 1 / ((0.15f + qlsp[i + 1] - qlsp[i]) * (0.15f + qlsp[i + 1] - qlsp[i]));
#else
      tmp1 = 1/(qlsp[i]-qlsp[i-1]);
      tmp2 = 1/(qlsp[i+1]-qlsp[i]);
#endif
      quant_weight[i] = tmp1 > tmp2 ? tmp1 : tmp2;
   }

   for (i=0;i<order;i++)
       qlsp[i] -= (0.25f * i + 0.25f);  // Добавлен суффикс f для float
   for (i=0;i<order;i++)
      qlsp[i]*=256;
   
   id = lsp_quant(qlsp, cdbk_nb, NB_CDBK_SIZE, order);
   speex_bits_pack(bits, id, 6);

   for (i=0;i<order;i++)
      qlsp[i]*=2;
   
   id = lsp_weight_quant(qlsp, quant_weight, cdbk_nb_low1, NB_CDBK_SIZE_LOW1, 5);
   speex_bits_pack(bits, id, 6);

   id = lsp_weight_quant(qlsp+5, quant_weight+5, cdbk_nb_high1, NB_CDBK_SIZE_HIGH1, 5);
   speex_bits_pack(bits, id, 6);

   for (i=0;i<order;i++)
      qlsp[i]*=0.0019531;

   for (i=0;i<order;i++)
      qlsp[i]=lsp[i]-qlsp[i];
}

void lsp_unquant_lbr(float* lsp, int order, SpeexBits* bits)
{
    int i, id;
    for (i = 0; i < order; i++)
        lsp[i] = 0.25f * i + 0.25f; // Добавлен суффикс f для float

    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 10; i++)
        lsp[i] += 0.0039062f * cdbk_nb[id * 10 + i]; // Добавлен суффикс f для float

    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i] += 0.0019531f * cdbk_nb_low1[id * 5 + i]; // Добавлен суффикс f для float

    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < 5; i++)
        lsp[i + 5] += 0.0019531f * cdbk_nb_high1[id * 5 + i]; // Добавлен суффикс f для float
}

extern signed char high_lsp_cdbk[];
extern signed char high_lsp_cdbk2[];

void lsp_quant_high(float* lsp, float* qlsp, int order, SpeexBits* bits)
{
    int i;
    float tmp1, tmp2;
    int id;
    for (i = 0; i < order; i++)
        qlsp[i] = lsp[i];

    quant_weight[0] = 1 / (qlsp[1] - qlsp[0]);
    quant_weight[order - 1] = 1 / (qlsp[order - 1] - qlsp[order - 2]);
    for (i = 1; i < order - 1; i++)
    {
        tmp1 = 1 / (qlsp[i] - qlsp[i - 1]);
        tmp2 = 1 / (qlsp[i + 1] - qlsp[i]);
        quant_weight[i] = tmp1 > tmp2 ? tmp1 : tmp2;
    }

    for (i = 0; i < order; i++)
        qlsp[i] -= (0.3125f * i + 0.75f); // Добавлен суффикс f для float
    for (i = 0; i < order; i++)
        qlsp[i] *= 256;

    id = lsp_quant(qlsp, high_lsp_cdbk, 64, order);
    speex_bits_pack(bits, id, 6);

    for (i = 0; i < order; i++)
        qlsp[i] *= 2;

    id = lsp_weight_quant(qlsp, quant_weight, high_lsp_cdbk2, 64, order);
    speex_bits_pack(bits, id, 6);

    for (i = 0; i < order; i++)
        qlsp[i] *= 0.0019531f; // Добавлен суффикс f для float

    for (i = 0; i < order; i++)
        qlsp[i] = lsp[i] - qlsp[i];
}

void lsp_unquant_high(float* lsp, int order, SpeexBits* bits)
{
    int i, id;
    for (i = 0; i < order; i++)
        lsp[i] = (0.3125f * i + 0.75f); // Добавлен суффикс f для float

    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < order; i++)
        lsp[i] += 0.0039062f * high_lsp_cdbk[id * order + i]; // Добавлен суффикс f для float

    id = speex_bits_unpack_unsigned(bits, 6);
    for (i = 0; i < order; i++)
        lsp[i] += 0.0019531f * high_lsp_cdbk2[id * order + i]; // Добавлен суффикс f для float
}
