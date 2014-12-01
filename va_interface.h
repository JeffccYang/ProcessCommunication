#ifndef __VA_INTERFACE_H__
#define __VA_INTERFACE_H__
 
#ifdef __cplusplus
extern "C"{
#endif
	
#define MAX_VAPARAS 50

	typedef struct s_VAParas{
 		int left; int top; int right; int bottom;	 
 	}sVAParas;

	int VA_Interface_Init(); 
	
	void VA_Interface_Close(); 

	/* 
	FD 
	*/
 	int VA_Interface_FD( unsigned char* yData ,unsigned int w, unsigned int h, sVAParas *vaParas); 


	/* 
	Common 
	*/
  	void fillRGBImageRect(unsigned char *ImgS,int Width,int Height,
                                int left,int top,int right,int bottom,int color);

	void Write_bmp(const char *fname_s,const unsigned char *Pic,const int *W,const int *H);

	void  Read_bmp(const char *fname_s, unsigned char *Pic );

	void Gray_bmp(const unsigned char *Pic,const int W, const int H, unsigned char *Pic2);

	void Reverse_bmp( unsigned char *Pic ,int w ,int h );

#ifdef __cplusplus
}
#endif

#endif /* __VA_INTERFACE_H__ */