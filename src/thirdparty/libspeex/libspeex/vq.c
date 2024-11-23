#include "vq.h"

/*Finds the index of the entry in a codebook that best matches the input*/
int vq_index(float* in, float* codebook, int len, int entries)
{
    int i, j;
    float min_dist = 0;
    int best_index = 0;
    for (i = 0; i < entries; i++)
    {
        float dist = 0;
        for (j = 0; j < len; j++)
        {
            float tmp = in[j] - *codebook++;
            dist += tmp * tmp;
        }
        if (i == 0 || dist < min_dist)
        {
            min_dist = dist;
            best_index = i;
        }
    }
    return best_index;
}

/*Finds the indices of the n-best entries in a codebook*/
void vq_nbest(float* in, float* codebook, int len, int entries, float* E, int N, int* nbest, float* best_dist)
{
    int i, j, k, used;
    used = 0;
    for (i = 0; i < entries; i++)
    {
        float dist = 0.5f * E[i];
        for (j = 0; j < len; j++)
            dist -= in[j] * *codebook++;
        if (i < N || dist < best_dist[N - 1])
        {
            for (k = N - 1; (k >= 1) && (k > used || dist < best_dist[k - 1]); k--)
            {
                best_dist[k] = best_dist[k - 1];
                nbest[k] = nbest[k - 1];
            }
            best_dist[k] = dist;
            nbest[k] = i;
            used++;
        }
    }
}

/*Finds the indices of the n-best entries in a codebook with sign*/
void vq_nbest_sign(float* in, float* codebook, int len, int entries, float* E, int N, int* nbest, float* best_dist)
{
    int i, j, k, sign, used;
    used = 0;
    for (i = 0; i < entries; i++)
    {
        float dist = 0;
        for (j = 0; j < len; j++)
            dist -= in[j] * *codebook++;
        if (dist > 0)
        {
            sign = 1;
            dist = -dist;
        }
        else
        {
            sign = 0;
        }
        dist += 0.5f * E[i];
        if (i < N || dist < best_dist[N - 1])
        {
            for (k = N - 1; (k >= 1) && (k > used || dist < best_dist[k - 1]); k--)
            {
                best_dist[k] = best_dist[k - 1];
                nbest[k] = nbest[k - 1];
            }
            best_dist[k] = dist;
            nbest[k] = i;
            used++;
            if (sign)
                nbest[k] += entries;
        }
    }
}

