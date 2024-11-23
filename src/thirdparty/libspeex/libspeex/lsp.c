#include <math.h>
#include "lsp.h"
#include "stack_alloc.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846  /* pi */
#endif

#ifndef NULL
#define NULL 0
#endif

static float cheb_poly_eva(float* coef, float x, int m, char* stack)
{
    int i;
    float* T, sum;
    int m2 = m >> 1;
    T = PUSH(stack, m2 + 1, float);
    T[0] = 1;
    T[1] = x;
    sum = coef[m2] + coef[m2 - 1] * x;
    x *= 2;
    for (i = 2; i <= m2; i++)
    {
        T[i] = x * T[i - 1] - T[i - 2];
        sum += coef[m2 - i] * T[i];
    }

    return sum;
}

int lpc_to_lsp(float* a, int lpcrdr, float* freq, int nb, float delta, char* stack)
{
    float psuml, psumr, psumm, temp_xr, xl, xr, xm = 0;
    float temp_psumr;
    int i, j, m, flag, k;
    float* Q;
    float* P;
    float* px;
    float* qx;
    float* p;
    float* q;
    float* pt;
    int roots = 0;
    flag = 1;
    m = lpcrdr / 2;
    Q = PUSH(stack, (m + 1), float);
    P = PUSH(stack, (m + 1), float);
    px = P;
    qx = Q;
    p = px;
    q = qx;
    *px++ = 1.0;
    *qx++ = 1.0;
    for (i = 1; i <= m; i++)
    {
        *px++ = a[i] + a[lpcrdr + 1 - i] - *p++;
        *qx++ = a[i] - a[lpcrdr + 1 - i] + *q++;
    }
    px = P;
    qx = Q;
    for (i = 0; i < m; i++)
    {
        *px = 2 * *px;
        *qx = 2 * *qx;
        px++;
        qx++;
    }
    px = P;
    qx = Q;
    xr = 0;
    xl = 1.0;
    for (j = 0; j < lpcrdr; j++)
    {
        if (j % 2)
            pt = qx;
        else
            pt = px;

        psuml = cheb_poly_eva(pt, xl, lpcrdr, stack);
        flag = 1;
        while (flag && (xr >= -1.0))
        {
            float dd;
            dd = static_cast<float>(delta * (1 - 0.9 * xl * xl));
            if (fabs(psuml) < 0.2)
                dd *= 0.5;

            xr = xl - dd;
            psumr = cheb_poly_eva(pt, xr, lpcrdr, stack);
            temp_psumr = psumr;
            temp_xr = xr;
            if ((psumr * psuml) < 0.0)
            {
                roots++;

                psumm = psuml;
                for (k = 0; k <= nb; k++)
                {
                    xm = (xl + xr) / 2;
                    psumm = cheb_poly_eva(pt, xm, lpcrdr, stack);
                    if (psumm * psuml > 0.0)
                    {
                        psuml = psumm;
                        xl = xm;
                    }
                    else
                    {
                        psumr = psumm;
                        xr = xm;
                    }
                }

                freq[j] = xm;
                xl = xm;
                flag = 0;
            }
            else
            {
                psuml = temp_psumr;
                xl = temp_xr;
            }
        }
    }
    return roots;
}

void lsp_to_lpc(float* freq, float* ak, int lpcrdr, char* stack)
{
    int i, j;
    float xout1, xout2, xin1, xin2;
    float* Wp;
    float* pw, * n1, * n2, * n3, * n4 = NULL;
    int m = lpcrdr / 2;

    Wp = PUSH(stack, 4 * m + 2, float);
    pw = Wp;
    for (i = 0; i <= 4 * m + 1; i++)
    {
        *pw++ = 0.0;
    }
    pw = Wp;
    xin1 = 1.0;
    xin2 = 1.0;
    for (j = 0; j <= lpcrdr; j++)
    {
        int i2 = 0;
        for (i = 0; i < m; i++, i2 += 2)
        {
            n1 = pw + (i * 4);
            n2 = n1 + 1;
            n3 = n2 + 1;
            n4 = n3 + 1;
            xout1 = xin1 - 2 * (freq[i2]) * *n1 + *n2;
            xout2 = xin2 - 2 * (freq[i2 + 1]) * *n3 + *n4;
            *n2 = *n1;
            *n4 = *n3;
            *n1 = xin1;
            *n3 = xin2;
            xin1 = xout1;
            xin2 = xout2;
        }
        xout1 = xin1 + *(n4 + 1);
        xout2 = xin2 - *(n4 + 2);
        ak[j] = static_cast<float>((xout1 + xout2) * 0.5);
        *(n4 + 1) = xin1;
        *(n4 + 2) = xin2;

        xin1 = 0.0;
        xin2 = 0.0;
    }
}

void lsp_enforce_margin(float* lsp, int len, float margin)
{
    int i;
    if (lsp[0] < margin)
        lsp[0] = margin;
    if (lsp[len - 1] > M_PI - margin)
        lsp[len - 1] = static_cast<float>(M_PI - margin);
    for (i = 1; i < len - 1; i++)
    {
        if (lsp[i] < lsp[i - 1] + margin)
            lsp[i] = lsp[i - 1] + margin;

        if (lsp[i] > lsp[i + 1] - margin)
            lsp[i] = 0.5f * (lsp[i] + lsp[i + 1] - margin);
    }
}
