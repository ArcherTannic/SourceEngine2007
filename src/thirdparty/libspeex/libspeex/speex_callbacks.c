#include "speex_callbacks.h"
#include "misc.h"
#include <windows.h>
int speex_inband_handler(SpeexBits* bits, SpeexCallback* callback_list, void* state)
{
    int id;
    SpeexCallback* callback;
    id = speex_bits_unpack_unsigned(bits, 4);
    callback = callback_list + id;

    if (callback->func)
    {
        return callback->func(bits, state, callback->data);
    }
    else
    {
        int adv;
        if (id < 2)
            adv = 1;
        else if (id < 8)
            adv = 4;
        else if (id < 10)
            adv = 8;
        else if (id < 12)
            adv = 16;
        else if (id < 14)
            adv = 32;
        else
            adv = 64;
        speex_bits_advance(bits, adv);
    }
    return 0;
}

int speex_std_mode_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int m;
    m = speex_bits_unpack_unsigned(bits, 4);
    speex_encoder_ctl(data, SPEEX_SET_MODE, &m);
    return 0;
}

int speex_std_low_mode_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int m;
    m = speex_bits_unpack_unsigned(bits, 4);
    speex_encoder_ctl(data, SPEEX_SET_LOW_MODE, &m);
    return 0;
}

int speex_std_high_mode_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int m;
    m = speex_bits_unpack_unsigned(bits, 4);
    speex_encoder_ctl(data, SPEEX_SET_HIGH_MODE, &m);
    return 0;
}

int speex_std_vbr_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int vbr;
    vbr = speex_bits_unpack_unsigned(bits, 1);
    speex_encoder_ctl(data, SPEEX_SET_VBR, &vbr);
    return 0;
}

int speex_std_enh_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int enh;
    enh = speex_bits_unpack_unsigned(bits, 1);
    speex_decoder_ctl(data, SPEEX_SET_ENH, &enh);
    return 0;
}

int speex_std_vbr_quality_request_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int qual;
    qual = speex_bits_unpack_unsigned(bits, 4);
    speex_encoder_ctl(data, SPEEX_SET_VBR_QUALITY, &qual);
    return 0;
}

int speex_std_char_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    unsigned char ch;
    ch = speex_bits_unpack_unsigned(bits, 8);
    _speex_putc(ch, data);
    return 0;
}

int speex_default_user_handler(SpeexBits* bits, void* state, void* data)
{
    UNREFERENCED_PARAMETER(state);

    int req_size = speex_bits_unpack_unsigned(bits, 4);
    speex_bits_advance(bits, 5 + 8 * req_size);
    return 0;
}
