#include <stdio.h>
//#include "XH264Decoder.h"

class XH264Reader{
public:
	XH264Reader();

	~XH264Reader();

	bool H264FrameReader_Init(const char* filename);

	void H264FrameReader_Free();

	int H264FrameReader_ReadFrame(char* outBuf, int* outBufSize);

	int H264FrameReader_ReadFrame2(char* outBuf, int* outBufSize, int* decodeWidth, int* decodeHeight);

	int H264FrameReader_Reset();


	char* filebuf_;
	const char* pbuf_;
	int filesize_;
	FILE* fp;
};
