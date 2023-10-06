#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined ( __cplusplus)
extern "C"
{
#include "x265.h"
};
#else
#include "x265.h"
#endif

class MyPacket {
public:
    uint8_t* data;
    int size;
};

class Recon {
public:
    uint8_t* data[3];

};

typedef struct 
{
    int bits;
    int width;
    int height;
    int flag;
}EndBytes;

class MyEncoder
{
private:

    char* buff;
    x265_param* pParam;
    x265_encoder* pHandle;
    x265_picture* pPic_in;
    x265_picture* pPic_out;

    

public:
    int encoder_init(int w, int h, int fr, int br, int gop);

    int encoder_encode_frame(uint8_t* fdata[3], MyPacket* packet, int pts, Recon* recon);

    int encoder_close();
    
    int width;
    int height;
    int frame_rate;
    int bit_rate;
    int gop_size;
};
