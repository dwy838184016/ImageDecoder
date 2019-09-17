#include "XH264Reader.h"


#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "avlog.h"


XH264Reader::XH264Reader()
{
	fp = NULL;
	filebuf_ = NULL;
	filesize_ = 0;
}

XH264Reader :: ~XH264Reader()
{

}

bool XH264Reader :: H264FrameReader_Init(const char* filename)
{
    fp = fopen(filename, "rb");
    filebuf_ = 0;
    filesize_ = 0;

    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        filesize_ = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        return true;
    }
    else
    {
    	LOGE("File[%s] don't exist.", filename);
    	return false;
    }
}

int XH264Reader :: H264FrameReader_Reset()
{
	if(fp)
	{
		fseek(fp, 0, SEEK_SET);
	}
}

void XH264Reader :: H264FrameReader_Free()
{
    if(fp)
    {
    	fclose(fp);
    }
}

int XH264Reader :: H264FrameReader_ReadFrame(char* outBuf, int* outBufSize)
{
    if(fp)
    {
    	char sizeArray[4] = {NULL};
    	fread(sizeArray, 1, 4, fp);
    	int size = 0;
    	memcpy(&size, sizeArray, 4);

    	if(size > 0)
    	{
    		*outBufSize = size;
    		fread(outBuf, 1, size, fp);

    		LOGE("[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x] ", outBuf[0], outBuf[1], outBuf[2], outBuf[3], outBuf[4]);

    	}

    }

    return 1;
}

/**
 * 自定义数据结构
 * [一帧数据大小(4字节)][帧宽(4字节)][帧高(4字节)][一帧数据]...[一帧数据大小(4字节)][帧宽(4字节)][帧高(4字节)][一帧数据]...
 */
int XH264Reader :: H264FrameReader_ReadFrame2(char* outBuf, int* outBufSize, int* decodeWidth, int* decodeHeight)
{
	if(fp)
	{
		//H264一帧数据大小
		int size = 0;
		char sizeArray[4] = {NULL};
		fread(sizeArray, 1, 4, fp);
		memcpy(&size, sizeArray, 4);

		int width = 0;
		int height = 0;
		char widthArray[4] = {NULL};
		char heightArray[4] = {NULL};
		fread(widthArray, 1, 4, fp);
		fread(heightArray, 1, 4, fp);
		memcpy(&width, widthArray, 4);
		memcpy(&height, heightArray, 4);
		if(width != 0 && height != 0)
		{
			*decodeWidth = width;
			*decodeHeight = height;
		}

		if(size > 0)
		{
			*outBufSize = size;
			fread(outBuf, 1, size, fp);

			LOGE("[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x], Width[%d], Height[%d]", outBuf[0], outBuf[1], outBuf[2], outBuf[3], outBuf[4], *decodeWidth, *decodeHeight);

		}

		/*if(size == 276){
			LOGE("--------------------------------------------------");
			LOGE("--------------------------------------------------");
			LOGE("---------------Current size is 276.---------------");
			LOGE("---------------Read Frame Start---------------");
			for(int i=0; i<size; i+=20){
				if(i == 0){
					LOGE("DATA[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
							outBuf[i], outBuf[i+1], outBuf[i+2], outBuf[i+3], outBuf[i+4],outBuf[i+5], outBuf[i+6], outBuf[i+7], outBuf[i+8], outBuf[i+9],
							outBuf[i+10], outBuf[i+11], outBuf[i+12], outBuf[i+13], outBuf[i+14],outBuf[i+15], outBuf[i+16], outBuf[i+17], outBuf[i+18], outBuf[i+19]);
				} else if(size-i < 20){
					if(size - i == 16){
						LOGE("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]",
							outBuf[i], outBuf[i+1], outBuf[i+2], outBuf[i+3], outBuf[i+4],outBuf[i+5], outBuf[i+6], outBuf[i+7], outBuf[i+8], outBuf[i+9],
							outBuf[i+10], outBuf[i+11], outBuf[i+12], outBuf[i+13], outBuf[i+14],outBuf[i+15]);
					}
				} else {
					LOGE("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
							outBuf[i], outBuf[i+1], outBuf[i+2], outBuf[i+3], outBuf[i+4],outBuf[i+5], outBuf[i+6], outBuf[i+7], outBuf[i+8], outBuf[i+9],
							outBuf[i+10], outBuf[i+11], outBuf[i+12], outBuf[i+13], outBuf[i+14],outBuf[i+15], outBuf[i+16], outBuf[i+17], outBuf[i+18], outBuf[i+19]);
				}

			}
			LOGE("---------------Read Frame End---------------");
		}*/

	}

	return 1;
}

/*int main(int argc, char **argv)
{
    unsigned long max_size = 1280 * 720;
    int tmpbuf_len = 0;
    int current_read_len = 0;
    char* tmpbuf = (char*)malloc(max_size * 10);

    FILE *fp = fopen("out.h264", "wb+");
    if (!fp)
    {
        printf("open file error\n");
        return -1;
    }


    H264FrameReader_Init("test.h264");
    printf("file size = %d\n", filesize_);
    while (current_read_len < filesize_)
    {
        if (H264FrameReader_ReadFrame(tmpbuf, &tmpbuf_len))
        {
            printf("tmpbuf_len = %d\n", tmpbuf_len);
            fwrite(tmpbuf, tmpbuf_len, 1, fp);
            current_read_len += tmpbuf_len;
        }
    }
    fclose(fp);
    H264FrameReader_Free();

    return 0;
}*/
