

typedef struct 
{
	unsigned char  *buf;                                            //用來指向datatemp
	unsigned char  startcodnum;              //目前start code有幾個
	unsigned char  refill;                   //1則代表buffer A已經填滿，0則代表buffer B已經填滿
	unsigned char lastdata;
	int len;                      //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	int bufpos;                   //目前buffer的位置  int dlen;
	int streamidx;
	int bytecount;                //此nalu的size有多少個byte
	int dataperonetime;	
	unsigned char  datatemp[4096];           //用來儲存從檔案讀取出來的h.264的data，共分成兩個buffer每個皆為2048byte
	//Add,Jeff Yang,20110214
	int g_nfileflag;
	int g_ndataendcount;
} NALU_t;

//int g_nfileflag;
//int g_ndataendcount;

void IniDecSyntax(NALU_t* nalu);

int ReadNextNalu( NALU_t* nalu, unsigned char* StreamBuf, FILE * f_bitsream );



void IniDecSyntax(NALU_t* nalu)
{
	nalu->bufpos = -1;
	nalu->bytecount = 0;
	nalu->len = 0;
	nalu->startcodnum = 0;
	nalu->refill = 0;
	nalu->streamidx = 0;
	nalu->lastdata = 0;
	//g_nfileflag =0;
	//g_ndataendcount = 0;
	//Add,Jeff Yang,20110214
	nalu->g_nfileflag = 0;
	nalu->g_ndataendcount = 0;

}



int ReadNextNalu( NALU_t* nalu, unsigned char* StreamBuf, FILE * f_bitsream )
{

	//int pos=0;
	int nStartCodeFound=0;
	int  nbufposidx;
	int  nfilelen=0;
	int  streamidx=0;
	int  nbuffersize,nhalfbufsize;

	nbuffersize = 4096;
	nhalfbufsize=nbuffersize>>1;

	streamidx = nalu->streamidx;

	//第一次讀取4096之後每做完一半就填充1024
	if( nalu->bufpos == -2)
	{
		nalu->len = -1;
		return nalu->len;
	}
	else if(nalu->bufpos == -1 )
	{
		nfilelen=(int)fread( nalu->datatemp, 1,nbuffersize, f_bitsream );
		nalu->buf = &nalu->datatemp[0];//將buf的指標指向datatemp[0]
		nalu->bufpos=0;
		nalu->dataperonetime = nbuffersize;
	}
	else{}


	while ( !nStartCodeFound )//直到找到下一個start code才結束
	{

		for ( nbufposidx = nalu->bufpos; nbufposidx < nbuffersize; nbufposidx++ )//從上一個結束的byte繼續往下
		{
			nalu->bytecount++;//計算目前的byte數
			if (nbufposidx >= nhalfbufsize && nalu->refill == 0)
			{

				nalu->refill = 1;
				if(  nalu->lastdata == 1 )
				{
					nalu->g_nfileflag = -1;
				}
				else
				{
					nfilelen =(int)fread( nalu->datatemp, 1, nhalfbufsize, f_bitsream );//如果buffer的資料已經讀完重s讀取新的2000筆data
					if(nfilelen < nhalfbufsize) nalu->lastdata = 1;
					nalu->dataperonetime = nfilelen;
					
				}
			}
			else
			{
				nfilelen = nalu->dataperonetime;
			}
			//判斷所讀取的資料是否為最後一筆了，一次讀取2048筆資料如果小於2048就代表

			if ( nalu->g_nfileflag == -1 )//如果已經沒有資料了
			{
				nalu->g_ndataendcount++;
				if( nalu->g_ndataendcount >= nfilelen )
				{
					nStartCodeFound = 1;
					nalu->len = nalu->bytecount + 4;//記錄這個nalu有多少個byte
					nalu->bytecount = 0;
					nalu->buf++;//因為這裡就break了所以需要在此執熲uf++
					nalu->bufpos = -2;
					break;
				}
			}

			if ( *nalu->buf == 0x0 )//如果為0x0就累加
			{
				nalu->startcodnum++;
			}
			else if ( nalu->startcodnum >= 3 && *nalu->buf == 0x1)//如果前面有000000，3個0後面1個1則為start code且長度大於100將sps和pps和IDR視為同一個frame
			{
				nalu->streamidx = 4;
				nalu->startcodnum++;
				if ( ( nalu->bytecount - 4 ) > 0 )//如果扣掉4筆startcode(00000001)的長度為0則代表為第一個startcode
				{
					if( nalu->bytecount > 100 )
					{
					    nStartCodeFound = 1;
					    nalu->len = nalu->bytecount;//記錄這個nalu有多少個byte
						nalu->bufpos = nbufposidx+1;//記錄目
						if(nalu->bufpos >= nbuffersize) nalu->bufpos = 0;
						nalu->bytecount = 0;
						nalu->buf++;//因為這裡就break了所以需要在此執行buf++
						break;
					}
				}
				else
				{
					nStartCodeFound=0;//如果是第一個000001則需再找到下一個
					nalu->bytecount=0;
					nalu->startcodnum=0;
				}
			}
			else
			{
				nalu->startcodnum=0;

			}
			StreamBuf[streamidx] = *nalu->buf;
			nalu->buf ++;
			streamidx ++;
		}
        
		if(nbufposidx >= nbuffersize){//如果讀到buffer的最後一筆，將指標移至最前筆，並將buffer B填滿
			nalu->bufpos = 0;
			nalu->refill = 0;

			if(  nalu->lastdata == 1 )
			{
				nalu->g_nfileflag = -1;
			}
			else
			{
				//nfilelen = fread( nalu->datatemp, 1, nhalfbufsize, f_bitsream );//如果buffer的資料已經讀完重新讀取s的2000筆data
				nfilelen =(int)fread( nalu->datatemp+nhalfbufsize, 1,nhalfbufsize*sizeof(char), f_bitsream );//如果buffer的資料已經讀完重新讀取新的2000筆data
				if(nfilelen < nhalfbufsize) nalu->lastdata = 1;
				nalu->dataperonetime = nfilelen;
			}
			nalu->buf = &nalu->datatemp[0];//將buf的指標指向datatemp[0]
		}
	}

	return nalu->len;
}