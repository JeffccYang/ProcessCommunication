


#include "ffdecoder.h"

void AVS_CloseAVDecoder(AVDHANDLE *hAvd)
{

	LPAVSCodecParamm h;

	h = *(LPAVSCodecParamm*)hAvd;
	if (h == (LPAVSCodecParamm)AVD_STATUS_FAIL_OPEN)
	{
		return ;
	}
 
	if (h->hImgConvertCtx)
	{
		sws_freeContext(h->hImgConvertCtx);
		h->hImgConvertCtx = 0;
	}
	if (h->context)
	{
		avcodec_close(h->context);
		av_free(h->context);
		h->context = 0;
	}
	if (h->picture)
	{
		av_free(h->picture);
		h->picture = 0;
	}
 
	av_free(h);
	*hAvd = 0;
 
}



unsigned char *yuvBuffer;
unsigned char *scaleBuf;
int YUVStride[3];
//------------------------------
	FILE	*outFile;
	char 	szDestFile[256];
	static int yuv420pSaveInitFlag = 0;
//------------------------------


/** 
 * save yuv frame when buf==NULL
 */  
int yuv420p_save(AVFrame *pFrame, AVCodecContext *pCodecCtx, unsigned char *buf )  
{  
  
	if(yuv420pSaveInitFlag==0 && buf==NULL){
		//------------------------------		
		sprintf(szDestFile,"./c.yuv");
		//------------------------------
		outFile = fopen(szDestFile, "wb+");
			if(outFile==NULL){
				printf("[FAIL OPEN FILE!!!]\n");
				return -1;
			}
		yuv420pSaveInitFlag =1;		
	}

    int i = 0;  
  
    int width = pCodecCtx->width, height = pCodecCtx->height;  
    int height_half = height / 2, width_half = width / 2;  
    int y_wrap = pFrame->linesize[0];  
    int u_wrap = pFrame->linesize[1];  
    int v_wrap = pFrame->linesize[2];  
  
    unsigned char *y_buf = pFrame->data[0];  
    unsigned char *u_buf = pFrame->data[1];  
    unsigned char *v_buf = pFrame->data[2];  
  
  if(buf==NULL){
    //save y  
  	for (i = 0; i < height; i++)  
    	fwrite(y_buf + i * y_wrap, 1, width, outFile);  
    //save u  
    for (i = 0; i < height_half; i++)  
        fwrite(u_buf + i * u_wrap, 1, width_half, outFile);  
    //save v  
    for (i = 0; i < height_half; i++)  
        fwrite(v_buf + i * v_wrap, 1, width_half, outFile);  
    fflush(outFile);  
  }
  else{
	for (i = 0 ; i < height; i++){
		memcpy(buf, y_buf + i * y_wrap,  width);
		buf += width;
	}
	for (i = 0 ; i < height_half; i++){
		memcpy(buf, u_buf + i * u_wrap,  width_half);
		buf += width_half;
	}
	for (i = 0 ; i < height_half; i++){
		memcpy(buf, v_buf + i * v_wrap,  width_half);
		buf += width_half;
	}
  }


    return 0;
}  


AVDSTATUS AVS_DecodeVideo(AVDHANDLE hAvd, unsigned char *pBStream, int nFrameSize, void *pContext)
{
	LPAVSCodecParamm h;

	int len;
	struct timeval	tvTimeStart , tvTimeEnd;
	h = (LPAVSCodecParamm)hAvd;


//	if (h==(LPAVSCodecParamm)AVD_STATUS_FAIL_OPEN || pBStream==NULL || h->pDecodedCB==NULL || nFrameSize<0 )	
	if (h==(LPAVSCodecParamm)AVD_STATUS_FAIL_OPEN || pBStream==NULL  || nFrameSize<0 )
		return (AVDSTATUS)AVD_STATUS_FAIL_DE;

	h->AvdParam.nWidth=0;	h->AvdParam.nHeight = 0;


	if (nFrameSize <= 0)
	{
		return AVD_STATUS_FAIL_DE;	
	}
	h->avpkt.size = nFrameSize;
	h->avpkt.data = pBStream;

 
	while (h->avpkt.size > 0) 
	{
		len = 0;

		//如果已經被關閉就跳出
		if (!h)
		{
			return AVD_STATUS_FAIL_DE;
		}
		if (h->context ==0 || h->picture == 0)
		{
			return AVD_STATUS_FAIL_DE;
		}
 		
 		len = avcodec_decode_video2( h->context, h->picture, &h->AvdParam.bGotFrame, &h->avpkt);
 
		if (len < 0) 
		{
 
			h->nErrCount++;
			if (h->nErrCount >= 1000)
			{
 
				//AVCodec_ReInit(h);
				h->nErrCount = 0;
			}

			return (AVDSTATUS)AVD_STATUS_FAIL_DE;
		}
		if (h->AvdParam.bGotFrame) 
		{	
			//g_TotalFrame++;
			//printf("framenum%d , time %f sec, framerate:%f\n",g_TotalFrame , ((float)g_lTime / 1000.0), (float)g_TotalFrame / ((float)g_lTime / 1000.0));

			h->AvdParam.pDecodedFrame[0] = (unsigned char*)h->picture->data[0];
			h->AvdParam.pDecodedFrame[1] = (unsigned char*)h->picture->data[1];
			h->AvdParam.pDecodedFrame[2] = (unsigned char*)h->picture->data[2];

			h->AvdParam.nHeight = h->context->height;
			h->AvdParam.nWidth = h->context->width;


printf("---> %d,%d \n",h->context->width, h->context->height);

 /*
			//init scale function
			if (h->hImgConvertCtx == 0)
			{
				h-> hImgConvertCtx = sws_getContext(
					h->context->width , h->context->height , PIX_FMT_YUV420P,
					h->context->width , h->context->height , PIX_FMT_RGB24 ,
					SWS_FAST_BILINEAR ,
					NULL, NULL, NULL);

				scaleBuf = (unsigned char *)malloc((h->context->width * h->context->height)*3);
				YUVStride[0] =  h->context->width * 3;


printf("  <<<<<<<<< hImgConvertCtx >>>>>>>> \n" );
			}
	*/	
						//added By Peter, 新增輸出強制為RGB32
			if (	( (h->context->width != 0 && h->context->height != 0 ) && 
					  (h->context->width != h->nPreWidth || h->context->height != h->nPreHeight) ) ||
						( h->hImgConvertCtx == 0 )
				)
			{	//寬高跟之前的frame不同, 重新init swscalecontext
				if (h->hImgConvertCtx)
				{
					sws_freeContext(h->hImgConvertCtx);
				}
				if (h->RgbBuffer)
				{
					 free(h->RgbBuffer);
				}
				if (h->YuvBuffer)
				{
					 free(h->YuvBuffer);
				}

				//(AVPixelFormat)h->picture->format
				h->hImgConvertCtx = sws_getContext(	h->context->width, h->context->height, PIX_FMT_YUV420P, 
                                                    h->context->width, h->context->height, PIX_FMT_RGB24, SWS_POINT,
													NULL, NULL, NULL);
				//改成16bytes alignment的malloc
                h->RgbBuffer = (char*)malloc(h->context->width * h->context->height * 3);
				h->YuvBuffer = (char*)malloc(h->context->width * h->context->height * 2);

				h->nPreWidth = h->context->width;
				h->nPreHeight = h->context->height;

			}
		
// 			unsigned char *pYUV[3];
// 			pYUV[0] = scaleBuf;
// 			sws_scale(h->hImgConvertCtx, h->picture->data, h->picture->linesize, 0, h->AvdParam.nHeight, pYUV, YUVStride);
		
		//--------------------------------------------------------------------------------------		
			//store yuv
			{
	 			// yuv420p_save(h->picture, h->context, NULL);  
			 
			/*
					if(yuv420pSaveInitFlag==0){
					//------------------------------		
						sprintf(szDestFile,"./c.yuv");
					//------------------------------
						outFile = fopen(szDestFile, "wb+");
						if(outFile==NULL){
							printf("[FAIL OPEN FILE!!!]\n");
							return -1;
						}
						yuv420pSaveInitFlag =1;		
					}
			*/
					char *pYUV = h->YuvBuffer;
				
					yuv420p_save(h->picture, h->context, pYUV);  

				///	yuv420p_save(h->picture, h->context, NULL);  
			/*
					fwrite(h->YuvBuffer, 1, h->context->width * h->context->height * 1.5, outFile);  
					fflush(outFile);  
			*/

			}	
	
			h->pDecodedCB(pContext, h->AvdParam.nChannelID,(unsigned char*) h->YuvBuffer, h->context->width, 
					h->context->height, h->picture->linesize[0], 0);
			
		//--------------------------------------------------------------------------------------


		}
		else
		{
			printf("decode no complete\n");
			//avs_printf("decode no complete\n");
			return AVD_STATUS_FAIL_DE;
			
		}
		h->avpkt.size -= len;
		h->avpkt.data += len; 
		pBStream += len;
		nFrameSize -= len;
		return AVD_STATUS_OK;
	}


	if (h->AvdParam.bGotFrame) 
		return AVD_STATUS_OK;
	return AVD_STATUS_FAIL_DE;

}



AVDHANDLE AVS_OpenAVDecoder(int	nChannelID, AVD_CODECID	avd_codecID , DecodedFrameFunc	pDecodedFunc)
{
	LPAVSCodecParamm			h = 0;


	//if (!Init_VideoDecoder())
	//{		
	//return (AVDHANDLE)AVD_STATUS_FAIL_OPEN;
	//}

	h = (LPAVSCodecParamm) av_malloc(sizeof(AVSCodecParamm));
	if(!h) 
	{
		return (AVDHANDLE)AVD_STATUS_FAIL_OPEN;
	} 
	memset(h , 0 , sizeof(AVSCodecParamm));


	avcodec_register_all();

	//g_hOpenCloseCodecMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, OPENCLOSECODEC_MUTEX);
	//WaitForSingleObject(g_hOpenCloseCodecMutex, INFINITE);

	switch(avd_codecID)
	{
	case AVD_CODECID_H264:
		h->codecID = AV_CODEC_ID_H264;
		break;
	case AVS_CODECID_ULAW:
		//h->codecID = CODEC_ID_PCM_MULAW;
		break;
	case AVD_CODECID_MPEG4:
		//h->codecID = CODEC_ID_MPEG4;
		break;
	case AVD_CODECID_JPEG:
		//h->codecID = CODEC_ID_MJPEG;
		break;
	}

	//配置codec
	//h->hImgConvertCtx = 0;
	h->codec = avcodec_find_decoder(h->codecID);
	if (!h->codec)
	{
		goto ERR;
	}
	h->picture = avcodec_alloc_frame();
	if (!h->picture)
	{
		goto ERR;
	}
	  


	h->context = avcodec_alloc_context3(h->codec); //avcodec_alloc_context();
	if (!h->context)
	{
		goto ERR;
	}

	h->context->thread_count = 1;

	h->pDecodedCB = NULL;
	h->AvdParam.nChannelID = nChannelID;
	//h->context->flags|=CODEC_FLAG_TRUNCATED | CODEC_FLAG_LOW_DELAY;
	//h->context->flags2 |=CODEC_FLAG2_CHUNKS;

	//for audio
	h->context->channels = 1;
	h->context->bit_rate = 8000;
	h->context->sample_rate = 8000;
	h->context->block_align = 8;
	//h->context->frame_bits = 4;
	h->context->bits_per_coded_sample = 8000;
	//init picture
	h->picture->key_frame= 1;	
	//h->got_picture=0;

 
	//open codec handle
	if (avcodec_open2(h->context, h->codec, NULL) < 0 )
	{
		goto ERR;
	}
 
	h->hImgConvertCtx = 0;

	if(pDecodedFunc!=NULL)
	{
		h->pDecodedCB = pDecodedFunc;
	}

	return (AVDHANDLE)h;
ERR:
		printf("AVS_OpenAVDecoder ERROR!!!\n");
	//ReleaseMutex(g_hOpenCloseCodecMutex);
	AVS_CloseAVDecoder((AVDHANDLE*)&h);
	return (AVDHANDLE)AVD_STATUS_FAIL_OPEN;
}

 