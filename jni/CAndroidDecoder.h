#pragma once
extern "C"
{
#include "mediacodec_wrapper.h"
}

class CAndroidDecoder
{
public:
	CAndroidDecoder();
	~CAndroidDecoder();

	bool InitDecoder(char* pMime, int nWidth,int nHeight,int nProfileType);
	void FinitDecoder();

	int GetDecoderWidth();
	int GetDecoderHeight();

	uint8_t* GetFrameOut(bool bCacheOnly, int* pWidth, int* pHeight, int* pPixType, int* pStride, int* pOutSize);

	bool AddFrameData(uint8_t* pFrameData, int nFrameSize,int* pLeft,bool isEnd);

	void DecoderFlush();
private:
	FFAMediaCodec* m_pH264Decoder;
	int m_nFrameCount;
	int m_nGetFrameCount;
	FFAMediaFormat* m_pOutFormat;

	int m_nWidth;
	int m_nHeight;

};

