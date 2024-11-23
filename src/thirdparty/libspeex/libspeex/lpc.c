#include "lpc.h"
#include <iostream>
#include <vector>

float                      /* returns minimum mean square error    */
wld(
    float* lpc, /*      [0...p-1] LPC coefficients      */
    const float* ac,  /*  in: [0...p] autocorrelation values  */
    float* ref, /* out: [0...p-1] reflection coef's     */
    int p
)
{
    int i, j;
    float r, error = ac[0];

    if (ac[0] == 0) {
        for (i = 0; i < p; i++) ref[i] = 0;
        return 0;
    }

    for (i = 0; i < p; i++) {
        /* Sum up this iteration's reflection coefficient. */
        r = -ac[i + 1];
        for (j = 0; j < i; j++) r -= lpc[j] * ac[i - j];
        ref[i] = r /= error;

        /*  Update LPC coefficients and total error. */
        lpc[i] = r;
        for (j = 0; j < i / 2; j++) {
            float tmp = lpc[j];
            lpc[j] += r * lpc[i - 1 - j];
            lpc[i - 1 - j] += r * tmp;
        }
        if (i % 2) lpc[j] += lpc[j] * r;

        error *= 1.0f - r * r;
    }
    return error;
}

void _spx_autocorr(
    const float* x,   /*  in: [0...n-1] samples x   */
    float* ac,   /* out: [0...lag-1] ac values */
    int lag, int   n)
{
    float d;
    int i;
    while (lag--) {
        for (i = lag, d = 0; i < n; i++) d += x[i] * x[i - lag];
        ac[lag] = d;
    }
}
