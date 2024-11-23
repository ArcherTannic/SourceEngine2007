#include "speex_stereo.h"
#include "speex_callbacks.h"
#include "vq.h"
#include <math.h>

static float e_ratio_quant[4] = { .25f, .315f, .397f, .5f };

void speex_encode_stereo(float* data, int frame_size, SpeexBits* bits)
{
    int i, tmp;
    float e_left = 0.0f, e_right = 0.0f, e_tot = 0.0f;
    float balance, e_ratio;
    for (i = 0; i < frame_size; i++)
    {
        e_left += data[2 * i] * data[2 * i];
        e_right += data[2 * i + 1] * data[2 * i + 1];
        data[i] = 0.5f * (data[2 * i] + data[2 * i + 1]);
        e_tot += data[i] * data[i];
    }
    balance = (e_left + 1.0f) / (e_right + 1.0f);
    e_ratio = e_tot / (1.0f + e_left + e_right);

    /*Quantization*/
    speex_bits_pack(bits, 14, 5);
    speex_bits_pack(bits, SPEEX_INBAND_STEREO, 4);

    balance = 4.0f * logf(balance);

    /*Pack sign*/
    if (balance > 0.0f)
        speex_bits_pack(bits, 0, 1);
    else
        speex_bits_pack(bits, 1, 1);
    balance = floorf(0.5f + fabsf(balance));
    if (balance > 30.0f)
        balance = 31.0f;

    speex_bits_pack(bits, (int)balance, 5);

    /*Quantize energy ratio*/
    tmp = vq_index(&e_ratio, e_ratio_quant, 1, 4);
    speex_bits_pack(bits, tmp, 2);
}

void speex_decode_stereo(float* data, int frame_size, SpeexStereoState* stereo)
{
    float balance, e_ratio;
    int i;
    float e_tot = 0.0f, e_left, e_right, e_sum;

    balance = stereo->balance;
    e_ratio = stereo->e_ratio;
    for (i = frame_size - 1; i >= 0; i--)
    {
        e_tot += data[i] * data[i];
    }
    e_sum = e_tot / e_ratio;
    e_left = e_sum * balance / (1.0f + balance);
    e_right = e_sum - e_left;

    e_left = sqrtf(e_left / (e_tot + 0.01f));
    e_right = sqrtf(e_right / (e_tot + 0.01f));

    for (i = frame_size - 1; i >= 0; i--)
    {
        float ftmp = data[i];
        stereo->smooth_left = 0.98f * stereo->smooth_left + 0.02f * e_left;
        stereo->smooth_right = 0.98f * stereo->smooth_right + 0.02f * e_right;
        data[2 * i] = stereo->smooth_left * ftmp;
        data[2 * i + 1] = stereo->smooth_right * ftmp;
    }
}

int speex_std_stereo_request_handler(SpeexBits* bits, void* data)
{
    SpeexStereoState* stereo;
    float sign = 1.0f;
    int tmp;

    stereo = (SpeexStereoState*)data;
    if (speex_bits_unpack_unsigned(bits, 1))
        sign = -1.0f;
    tmp = speex_bits_unpack_unsigned(bits, 5);
    stereo->balance = expf(sign * 0.25f * tmp);

    tmp = speex_bits_unpack_unsigned(bits, 2);
    stereo->e_ratio = e_ratio_quant[tmp];

    return 0;
}
