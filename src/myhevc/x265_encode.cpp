#include "x265_encode.h"


int MyEncoder::encoder_init(int w, int h, int fr, int br, int gop)
{
    width = w;
    height = h;
    frame_rate = fr;
    bit_rate = br;
    gop_size = gop;

    buff = NULL;
    pParam = NULL;
    pHandle = NULL;
    pPic_in = NULL;
    pPic_out = NULL;
    pParam = x265_param_alloc();
    x265_param_default_preset(pParam, "medium", "zerolatency");

    pParam->bRepeatHeaders = 1; // write sps,pps before keyframe
    pParam->internalCsp = X265_CSP_I420;
    pParam->sourceWidth = w;
    pParam->sourceHeight = h;
    pParam->fpsNum = fr;
    pParam->fpsDenom = 1;
    pParam->keyframeMax = 1; // all intra
    pParam->rc.rateControlMode = X265_RC_CQP;
    // pParam->rc.
    pParam->rc.qp = 45;
    // Init
    pHandle = x265_encoder_open(pParam);
    if (pHandle == NULL)
    {
        printf("x265_encoder_open err\n");
        return 0;
    }

    int y_size = pParam->sourceWidth * pParam->sourceHeight;

    pPic_in = x265_picture_alloc();
    pPic_out = x265_picture_alloc();
    x265_picture_init(pParam, pPic_in);
    x265_picture_init(pParam, pPic_out);

    buff = (char *)malloc(y_size * 3 / 2);
    pPic_in->planes[0] = buff;
    pPic_in->planes[1] = buff + y_size;
    pPic_in->planes[2] = buff + y_size * 5 / 4;
    pPic_in->stride[0] = w;
    pPic_in->stride[1] = w / 2;
    pPic_in->stride[2] = w / 2;

    return 0;
}

int MyEncoder::encoder_encode_frame(uint8_t *fdata[3], MyPacket *packet, int pts, Recon* recon)
{


     //prepare frame (YUV)
    memcpy(pPic_in->planes[0], fdata[0], width * height);
    memcpy(pPic_in->planes[1], fdata[1], width * height / 4);
    memcpy(pPic_in->planes[2], fdata[2], width * height / 4);

    x265_nal* pNals = NULL;
    uint32_t iNal = 0;

    int ret = x265_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
    fprintf(stderr,"Succeed encode %5d frames\n", pts);

    int pkt_size = 0;
    for (int j = 0; j < iNal; j++)
    {
        pkt_size += pNals[j].sizeBytes;
    }
    fprintf(stderr,"%d\n", pkt_size);
    packet->data = (uint8_t *)malloc(pkt_size);
    packet->size = 0;
    for (int j = 0; j < iNal; j++)
    {
        memcpy(packet->data + packet->size, pNals[j].payload, pNals[j].sizeBytes);
        packet->size += pNals[j].sizeBytes;
    }
    fprintf(stderr,"recon %d %d\n", pPic_out->height, pPic_out->stride[2]);
    // recon->data[0] = (uint8_t*)pPic_out->planes[0];
    // recon->data[1] = (uint8_t*)pPic_out->planes[1];
    // recon->data[2] = (uint8_t*)pPic_out->planes[2];

    //malloc recon.data to fit in h and w

    // Y
    if(pPic_out->stride[0] != width){
        recon->data[0] = (uint8_t*)malloc(width * height);
        for(int i = 0; i < height; i++){
            memcpy(recon->data[0] + i * width, (uint8_t*)pPic_out->planes[0] + i * pPic_out->stride[0], width);
        }
    }
    else{
        recon->data[0] = (uint8_t*)pPic_out->planes[0];
    }

    // U
    if(pPic_out->stride[1] != width / 2){
        recon->data[1] = (uint8_t*)malloc(width * height / 4);
        for(int i = 0; i < height / 2; i++){
            memcpy(recon->data[1] + i * width / 2, (uint8_t*)pPic_out->planes[1] + i * pPic_out->stride[1], width / 2);
        }
    }
    else{
        recon->data[1] = (uint8_t*)pPic_out->planes[1];
    }

    // V
    if(pPic_out->stride[2] != width / 2){
        recon->data[2] = (uint8_t*)malloc(width * height / 4);
        for(int i = 0; i < height / 2; i++){
            memcpy(recon->data[2] + i * width / 2, (uint8_t*)pPic_out->planes[2] + i * pPic_out->stride[2], width / 2);
        }
    }
    else{
        recon->data[2] = (uint8_t*)pPic_out->planes[2];
    }




    return 0;
}


int MyEncoder::encoder_close()
{


	x265_encoder_close(pHandle);
	x265_picture_free(pPic_in);
	x265_param_free(pParam);
	free(buff);
    return 0;
}