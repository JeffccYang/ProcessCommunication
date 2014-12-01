#ifndef __FFDECODER_H__
#define __FFDECODER_H__
 
 #ifdef __cplusplus
extern "C"{
#endif
	#include "libavcodec/avcodec.h"
	#include "libavformat/avformat.h"
	#include "libswscale/swscale.h"


typedef unsigned int	AVDHANDLE;

//Decoder callback funtion
typedef void (*DecodedFrameFunc)(void *pContext, int nChannel, 
	unsigned char *pData, unsigned int nWidth, unsigned int nHeight, unsigned int nPitch, 
	unsigned int	nFrameType);


typedef struct tagAVDParam{
	int					nChannelID;
    int					bGotFrame;			//是否解完一張frame
    int					nWidth;				//寬度
    int					nHeight;			//高度
	int					nPitch;				//寬的Pitch
	unsigned char		*pDecodedFrame[3];	//YUV Buffer, 0: Y 1:U 2:V
    unsigned char           *pRgbFrame;             //存放RGB的Buffer
}AVDParam;

/*
 typedef struct tagAVSCodecParam{
	AVDParam			AvdParam;
 
	AVCodec				*codec;			//codec handle
	AVCodecContext		*context;		//codec context
	AVFrame				*picture;		//construct frame buffer
	AVPacket			avpkt;
	enum AVCodecID		codecID;
 	
 	DecodedFrameFunc	pDecodedCB;
	struct SwsContext	*hImgConvertCtx;

	int					nErrCount;		//記錄Decode發生幾次錯誤
}AVSCodecParamm;
*/
typedef struct tagAVSCodecParam
{
	AVDParam			AvdParam;
	struct SwsContext	*hImgConvertCtx;
	AVCodec				*codec;			//codec handle
	AVCodecContext		*context;		//codec context
	AVFormatContext		*fmt_ctx;		//add by glen
	AVFrame				*picture;		//construct frame buffer
	AVPacket			avpkt;
	enum AVCodecID				codecID;
	int					nPreWidth;
	int					nPreHeight;
	int					nStreamType;	//add by glen
	char				*RgbBuffer;		//rgb buffer
	char				*YuvBuffer;		//rgb buffer
	unsigned char		*pVideo[3];		//真實輸出的buffer pointer
 
	DecodedFrameFunc	pDecodedCB;
	int					nErrCount;		//記錄Decode發生幾次錯誤
}AVSCodecParamm;


typedef enum _AVD_CODECID
{
	//VIDEO
	AVD_CODECID_H264		= 0 ,
 
	AVD_CODECID_MPEG4			,
	AVD_CODECID_JPEG			,
	//AUDIO
	AVS_CODECID_PCM				,
	AVS_CODECID_ULAW			,
	AVS_CODECID_ALAW			,
	AVS_CODECID_G726		
}AVD_CODECID;

typedef enum _AVDSTATUS
{
	AVD_STATUS_OK				=	 0, 
	AVD_STATUS_FAIL_OPEN		=	-1,
	AVD_STATUS_FAIL_DE			=	-2,
	AVD_STATUS_FAIL_CB			=	-3,
}AVDSTATUS;

typedef AVSCodecParamm*  LPAVSCodecParamm;


AVDHANDLE AVS_OpenAVDecoder(int	nChannelID, AVD_CODECID	avd_codecID, DecodedFrameFunc pDecodedFunc);
void AVS_CloseAVDecoder(AVDHANDLE *hAvd);
AVDSTATUS AVS_DecodeVideo(AVDHANDLE hAvd, unsigned char *pBStream, int nFrameSize, void *pContext);

void Write_bmp(const char *fname_s,const unsigned char *Pic,const int *W,const int *H);

#ifdef __cplusplus
}
#endif

#endif  