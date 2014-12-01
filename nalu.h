

typedef struct 
{
	unsigned char  *buf;                                            //�Ψӫ��Vdatatemp
	unsigned char  startcodnum;              //�ثestart code���X��
	unsigned char  refill;                   //1�h�N��buffer A�w�g�񺡡A0�h�N��buffer B�w�g��
	unsigned char lastdata;
	int len;                      //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	int bufpos;                   //�ثebuffer����m  int dlen;
	int streamidx;
	int bytecount;                //��nalu��size���h�֭�byte
	int dataperonetime;	
	unsigned char  datatemp[4096];           //�Ψ��x�s�q�ɮ�Ū���X�Ӫ�h.264��data�A�@�������buffer�C�ӬҬ�2048byte
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

	//�Ĥ@��Ū��4096����C�����@�b�N��R1024
	if( nalu->bufpos == -2)
	{
		nalu->len = -1;
		return nalu->len;
	}
	else if(nalu->bufpos == -1 )
	{
		nfilelen=(int)fread( nalu->datatemp, 1,nbuffersize, f_bitsream );
		nalu->buf = &nalu->datatemp[0];//�Nbuf�����Ы��Vdatatemp[0]
		nalu->bufpos=0;
		nalu->dataperonetime = nbuffersize;
	}
	else{}


	while ( !nStartCodeFound )//������U�@��start code�~����
	{

		for ( nbufposidx = nalu->bufpos; nbufposidx < nbuffersize; nbufposidx++ )//�q�W�@�ӵ�����byte�~�򩹤U
		{
			nalu->bytecount++;//�p��ثe��byte��
			if (nbufposidx >= nhalfbufsize && nalu->refill == 0)
			{

				nalu->refill = 1;
				if(  nalu->lastdata == 1 )
				{
					nalu->g_nfileflag = -1;
				}
				else
				{
					nfilelen =(int)fread( nalu->datatemp, 1, nhalfbufsize, f_bitsream );//�p�Gbuffer����Ƥw�gŪ����sŪ���s��2000��data
					if(nfilelen < nhalfbufsize) nalu->lastdata = 1;
					nalu->dataperonetime = nfilelen;
					
				}
			}
			else
			{
				nfilelen = nalu->dataperonetime;
			}
			//�P�_��Ū������ƬO�_���̫�@���F�A�@��Ū��2048����Ʀp�G�p��2048�N�N��

			if ( nalu->g_nfileflag == -1 )//�p�G�w�g�S����ƤF
			{
				nalu->g_ndataendcount++;
				if( nalu->g_ndataendcount >= nfilelen )
				{
					nStartCodeFound = 1;
					nalu->len = nalu->bytecount + 4;//�O���o��nalu���h�֭�byte
					nalu->bytecount = 0;
					nalu->buf++;//�]���o�̴Nbreak�F�ҥH�ݭn�b�����buf++
					nalu->bufpos = -2;
					break;
				}
			}

			if ( *nalu->buf == 0x0 )//�p�G��0x0�N�֥[
			{
				nalu->startcodnum++;
			}
			else if ( nalu->startcodnum >= 3 && *nalu->buf == 0x1)//�p�G�e����000000�A3��0�᭱1��1�h��start code�B���פj��100�Nsps�Mpps�MIDR�����P�@��frame
			{
				nalu->streamidx = 4;
				nalu->startcodnum++;
				if ( ( nalu->bytecount - 4 ) > 0 )//�p�G����4��startcode(00000001)�����׬�0�h�N���Ĥ@��startcode
				{
					if( nalu->bytecount > 100 )
					{
					    nStartCodeFound = 1;
					    nalu->len = nalu->bytecount;//�O���o��nalu���h�֭�byte
						nalu->bufpos = nbufposidx+1;//�O����
						if(nalu->bufpos >= nbuffersize) nalu->bufpos = 0;
						nalu->bytecount = 0;
						nalu->buf++;//�]���o�̴Nbreak�F�ҥH�ݭn�b������buf++
						break;
					}
				}
				else
				{
					nStartCodeFound=0;//�p�G�O�Ĥ@��000001�h�ݦA���U�@��
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
        
		if(nbufposidx >= nbuffersize){//�p�GŪ��buffer���̫�@���A�N���в��̫ܳe���A�ñNbuffer B��
			nalu->bufpos = 0;
			nalu->refill = 0;

			if(  nalu->lastdata == 1 )
			{
				nalu->g_nfileflag = -1;
			}
			else
			{
				//nfilelen = fread( nalu->datatemp, 1, nhalfbufsize, f_bitsream );//�p�Gbuffer����Ƥw�gŪ�����sŪ��s��2000��data
				nfilelen =(int)fread( nalu->datatemp+nhalfbufsize, 1,nhalfbufsize*sizeof(char), f_bitsream );//�p�Gbuffer����Ƥw�gŪ�����sŪ���s��2000��data
				if(nfilelen < nhalfbufsize) nalu->lastdata = 1;
				nalu->dataperonetime = nfilelen;
			}
			nalu->buf = &nalu->datatemp[0];//�Nbuf�����Ы��Vdatatemp[0]
		}
	}

	return nalu->len;
}