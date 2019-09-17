#include <jni.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include "CAndroidDecoder.h"
#include "avlog.h"

#define MIN(a,b) a<b?a:b

CAndroidDecoder::CAndroidDecoder()
{
	m_pH264Decoder = NULL;
	m_nFrameCount = 0;
	m_nGetFrameCount = 0;
	m_pOutFormat = 0;

	m_nWidth = -1;
	m_nHeight = -1;
}


CAndroidDecoder::~CAndroidDecoder()
{
	FinitDecoder();

	if (m_pOutFormat)
	{
		ff_AMediaFormat_delete(m_pOutFormat);
		m_pOutFormat = NULL;
	}
}


bool CAndroidDecoder::InitDecoder(char* pMime, int nWidth, int nHeight,int nProfileType)
{
	bool bRet = false;
	char* pCodecName = NULL;
	FFAMediaCodec* pH264Decoder = NULL;
	FFAMediaFormat* pMediaFormat = NULL;

	if(m_pH264Decoder)
	{
		pH264Decoder = m_pH264Decoder;
		LOGE("11111111111111 ModifyDecoder W x H %d x %d \r\n", nWidth, nHeight);

		ff_AMediaCodec_reset(pH264Decoder);
		pMediaFormat = ff_AMediaFormat_new();
		if (pMediaFormat == NULL)
		{
			LOGE("Create new media format fail \r\n");
		}
		else
		{
			ff_AMediaFormat_setString(pMediaFormat, "mime", pMime);
			ff_AMediaFormat_setInt32(pMediaFormat, "lowlatency", 1);
			ff_AMediaFormat_setInt32(pMediaFormat, "width", nWidth%2==0?nWidth:nWidth+1);
			ff_AMediaFormat_setInt32(pMediaFormat, "height", nHeight%2==0?nHeight:nHeight+1);
			if (ff_AMediaCodec_configure(pH264Decoder, pMediaFormat, NULL, 0, /*0*/0x02))
			{
				LOGE("media codec configure fail \r\n");
			}

			if (ff_AMediaCodec_start(pH264Decoder))
			{
				ff_AMediaCodec_flush(m_pH264Decoder);
				LOGE("MediaCodec start fail \r\n");
			}
			else
			{
				bRet = true;
				LOGE("MediaCodec start success \r\n");
			}
		}
	}
	else
	{
		LOGE("1111111111111 InitDecoder W x H  %d x %d \r\n", nWidth, nHeight);
		do
		{
			pCodecName = ff_AMediaCodecList_getCodecNameByType(pMime, nProfileType, 0, NULL);
			if (pCodecName == NULL)
			{
				LOGE("can not find decoder mime %s \r\n", pMime);
				break;
			}

			pH264Decoder = ff_AMediaCodec_createCodecByName(pCodecName);
			if (pH264Decoder == NULL)
			{
				LOGE("create decoder by name fail name %s \r\n", pCodecName);
				break;
			}

			pMediaFormat = ff_AMediaFormat_new();
			if (pMediaFormat == NULL)
			{
				LOGE("Create new media format fail \r\n");
				break;
			}

			ff_AMediaFormat_setString(pMediaFormat, "mime", pMime);
			ff_AMediaFormat_setInt32(pMediaFormat, "lowlatency", 1);
			ff_AMediaFormat_setInt32(pMediaFormat, "width", nWidth%2==0?nWidth:nWidth+1);
			ff_AMediaFormat_setInt32(pMediaFormat, "height", nHeight%2==0?nHeight:nHeight+1);

			if (ff_AMediaCodec_configure(pH264Decoder, pMediaFormat, NULL, 0, /*0*/0x02))
			{
				LOGE("media codec configure fail \r\n");
				break;
			}

			if (ff_AMediaCodec_start(pH264Decoder))
			{
				LOGE("MediaCodec start fail \r\n");
				break;
			}

			LOGE("media codec start success \r\n");
			bRet = true;
		} while (false);
	}


	if (pCodecName)
	{
		free(pCodecName);
	}

	if (pMediaFormat)
	{
		ff_AMediaFormat_delete(pMediaFormat);
		pMediaFormat = NULL;
	}

	if (bRet)
	{
		// if (m_pH264Decoder)
		// {
			// ff_AMediaCodec_stop(m_pH264Decoder);
			// ff_AMediaCodec_delete(m_pH264Decoder);
			// m_pH264Decoder = NULL;
		// }
		m_pH264Decoder = pH264Decoder;
		m_nFrameCount = 0;
		m_nGetFrameCount = 0;
		m_nWidth = nWidth;
		m_nHeight = nHeight;
	}
	else
	{
		ff_AMediaCodec_delete(pH264Decoder);
		pH264Decoder = NULL;
	}

	return bRet;
}


int CAndroidDecoder::GetDecoderHeight()
{
	return m_nHeight;
}

int CAndroidDecoder::GetDecoderWidth()
{
	return m_nWidth;
}

void CAndroidDecoder::FinitDecoder()
{
	if (m_pH264Decoder)
	{
		ff_AMediaCodec_stop(m_pH264Decoder);
		ff_AMediaCodec_delete(m_pH264Decoder);
		m_pH264Decoder = NULL;
	}
}

bool CAndroidDecoder::AddFrameData(uint8_t* pFrameData, int nFrameSize,int* pLeft,bool isEnd)
{

	bool bRet = false;
	int nLeft = nFrameSize;
	unsigned char* ptr = pFrameData;
	LOGE("----------------Add Frame Data----------------");
	if (m_pH264Decoder == NULL)
	{
		LOGE("decoder have not init \r\n");
		return bRet;
	}

	int nTryCount = 0;
	while (nLeft > 0 && nTryCount < 5)
	{
		uint8_t* pInputdata = NULL;
		size_t nInputDataSize;
		int nIndex;
		int nCopySize;
		nIndex = ff_AMediaCodec_dequeueInputBuffer(m_pH264Decoder, -1);
		if (ff_AMediaCodec_infoTryAgainLater(m_pH264Decoder, nIndex))
		{
			LOGE("DWY----------ff_AMediaCodec_infoTryAgainLater Failed, TryCount[%d]", nTryCount);
			nTryCount++;
			continue;
		}

		if (nIndex < 0)
		{
			LOGE("DWY----------Index less than zero, TryCount[%d]", nTryCount);
			break;
		}
		pInputdata = ff_AMediaCodec_getInputBuffer(m_pH264Decoder, nIndex, &nInputDataSize);
		if (pInputdata == NULL)
		{
			LOGE("get input buffer error \r\n");
			break;
		}
		
		nCopySize = MIN(nLeft, nFrameSize);
//		LOGE("FrameSize[%d], CopySize[%d]", nFrameSize, nCopySize);
//		LOGE("nCopySize[%d], nInputDataSize[%d], A[%d]", nCopySize, nInputDataSize, &pInputdata);
		memcpy(pInputdata, ptr, nCopySize);
		/*if(nFrameSize == 276){
			for(int i=0; i<nCopySize; i+=20){
				if(i == 0){
					LOGE("DATA[0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
							pInputdata[i], pInputdata[i+1], pInputdata[i+2], pInputdata[i+3], pInputdata[i+4],pInputdata[i+5], pInputdata[i+6], pInputdata[i+7], pInputdata[i+8], pInputdata[i+9],
							pInputdata[i+10], pInputdata[i+11], pInputdata[i+12], pInputdata[i+13], pInputdata[i+14],pInputdata[i+15], pInputdata[i+16], pInputdata[i+17], pInputdata[i+18], pInputdata[i+19]);
				} else if(nCopySize-i < 20){
					if(nCopySize - i == 16){
						LOGE("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x]",
								pInputdata[i], pInputdata[i+1], pInputdata[i+2], pInputdata[i+3], pInputdata[i+4],pInputdata[i+5], pInputdata[i+6], pInputdata[i+7], pInputdata[i+8], pInputdata[i+9],
															pInputdata[i+10], pInputdata[i+11], pInputdata[i+12], pInputdata[i+13], pInputdata[i+14],pInputdata[i+15]);
					}
				} else {
					LOGE("0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x ",
							pInputdata[i], pInputdata[i+1], pInputdata[i+2], pInputdata[i+3], pInputdata[i+4],pInputdata[i+5], pInputdata[i+6], pInputdata[i+7], pInputdata[i+8], pInputdata[i+9],
														pInputdata[i+10], pInputdata[i+11], pInputdata[i+12], pInputdata[i+13], pInputdata[i+14],pInputdata[i+15], pInputdata[i+16], pInputdata[i+17], pInputdata[i+18], pInputdata[i+19]);
				}

			}

		}*/
		
		if(!isEnd)
		{
			if (ff_AMediaCodec_queueInputBuffer(m_pH264Decoder, nIndex, 0, nCopySize, /*m_nFrameCount * 5*/0, 0))
			{
				LOGE("ff_AMediaCodec_queueInputBuffer fail \r\n");
				break;
			}
		}
		else
		{
			LOGE("DWY------------End---------");
			/** BUFFER_FLAG_END_OF_STREAM */
			if (ff_AMediaCodec_queueInputBuffer(m_pH264Decoder, nIndex, 0, nCopySize, /*m_nFrameCount * 5*/0, 4))
			{
				LOGE("ff_AMediaCodec_queueInputBuffer fail \r\n");
				break;
			}
		}

		ptr += nCopySize;
		nLeft -= nLeft;


		if (nLeft == 0)
		{
			bRet = true;
			m_nFrameCount++;
		}
		
	}

	*pLeft = nLeft;

	return bRet;
}

uint8_t* CAndroidDecoder::GetFrameOut(bool bCacheOnly,int* pWidth, int* pHeight, int* pPixType, int* pStride,int* pOutSize)
{
	uint8_t* pRet = NULL;

//    LOGE("get decode data out \r\n");
	LOGE("----------------Get Decode Data Out----------------");
	if (m_pH264Decoder == NULL)
	{
		return NULL;
	}
	do
	{
		int nOutIndex;
		FFAMediaCodecBufferInfo info = { 0 };
		uint8_t* pOutBuffer = NULL;
		size_t nOutBufferSize;
		/*int nTryCount = 0;
		do
		{
			if(m_nGetFrameCount == 0){
				nOutIndex = ff_AMediaCodec_dequeueOutputBuffer(m_pH264Decoder, &info, 40*1000);
			} else {
				nOutIndex = ff_AMediaCodec_dequeueOutputBuffer(m_pH264Decoder, &info, -1);
			}

			if (nOutIndex < 0)
			{
				if (ff_AMediaCodec_infoOutputFormatChanged(m_pH264Decoder, nOutIndex))
				{
					if (m_pOutFormat)
					{
						ff_AMediaFormat_delete(m_pOutFormat);
						m_pOutFormat = NULL;
					}
					m_pOutFormat = ff_AMediaCodec_getOutputFormat(m_pH264Decoder);
					if (m_pOutFormat)
					{

						char* format = ff_AMediaFormat_toString(m_pOutFormat);

						if (format)
						{
							LOGE("format change %s \r\n", format);
						}
					}


				}
				else if (ff_AMediaCodec_infoOutputBuffersChanged(m_pH264Decoder, nOutIndex))
				{

					ff_AMediaCodec_cleanOutputBuffers(m_pH264Decoder);
				}
				else if (ff_AMediaCodec_infoTryAgainLater(m_pH264Decoder, nOutIndex))
				{

				}
				if(nTryCount == 7 && nOutIndex < 0)
				{
					break;
				}
				nTryCount++;
	//			break;
			}
		} while(nOutIndex < 0 && nTryCount < 8);*/
		/*if(m_nGetFrameCount == 0){
			nOutIndex = ff_AMediaCodec_dequeueOutputBuffer(m_pH264Decoder, &info, 40*1000);
		} else {
			nOutIndex = ff_AMediaCodec_dequeueOutputBuffer(m_pH264Decoder, &info, -1);
		}*/
		nOutIndex = ff_AMediaCodec_dequeueOutputBuffer(m_pH264Decoder, &info, 8*1000);

		if (nOutIndex < 0)
		{
			if (ff_AMediaCodec_infoOutputFormatChanged(m_pH264Decoder, nOutIndex))
			{
				if (m_pOutFormat)
				{
					ff_AMediaFormat_delete(m_pOutFormat);
					m_pOutFormat = NULL;
				}
				m_pOutFormat = ff_AMediaCodec_getOutputFormat(m_pH264Decoder);
				if (m_pOutFormat)
				{

					char* format = ff_AMediaFormat_toString(m_pOutFormat);

					if (format)
					{
						LOGE("format change %s \r\n", format);
					}
				}


			}
			else if (ff_AMediaCodec_infoOutputBuffersChanged(m_pH264Decoder, nOutIndex))
			{

				ff_AMediaCodec_cleanOutputBuffers(m_pH264Decoder);
			}
			else if (ff_AMediaCodec_infoTryAgainLater(m_pH264Decoder, nOutIndex))
			{

			}
			break;
		}

		pOutBuffer = ff_AMediaCodec_getOutputBuffer(m_pH264Decoder, nOutIndex, &nOutBufferSize);
		if (pOutBuffer == NULL)
		{
			LOGE("get out buffer error \r\n");
		}
		else
		{
			if (!bCacheOnly)
			{
				pRet = (uint8_t*)malloc(nOutBufferSize);
				memcpy(pRet, pOutBuffer, nOutBufferSize);
			}
			else
			{
				pRet = (uint8_t*)1;
			}
			//要得到对齐后的宽高 modify by jason fang 
			int32_t value = 0;
			ff_AMediaFormat_getInt32(m_pOutFormat, "width", pWidth);
			ff_AMediaFormat_getInt32(m_pOutFormat, "height", pHeight);
			ff_AMediaFormat_getInt32(m_pOutFormat, "color-format", pPixType);
			ff_AMediaFormat_getInt32(m_pOutFormat, "stride", pStride);
			ff_AMediaFormat_getInt32(m_pOutFormat, "slice-height", &value);

			*pWidth =  *pStride > 0 ? *pStride : *pWidth;
			*pHeight = value > 0 ? value : *pHeight;
            
            LOGE("Width[%d], Height[%d], Color-Format[%d], Stride[%d], Slice-Height[%d]", *pWidth, *pHeight, *pPixType, *pStride, value);

			//c91计算出来的还是没对齐的宽高，其他平台待验证
			/*
			int left, right, top,bottom;

			ff_AMediaFormat_getInt32(m_pOutFormat, "crop-left", &left);
			ff_AMediaFormat_getInt32(m_pOutFormat, "crop-right", &right);
			ff_AMediaFormat_getInt32(m_pOutFormat, "crop-top", &top);
			ff_AMediaFormat_getInt32(m_pOutFormat, "crop-bottom", &bottom);

			LOGE("xx crop width %d height %d  \n", right + 1 - left, bottom + 1  - top );*/


			//LOGE("xxx width %d height %d, stride %d slice_height %d \n", *pWidth, *pHeight, *pStride, value);
			/*
			LOGE("Got output buffer %d" "width %d height %d"
				" offset=%d" " size=%d"  " ts=%lld"
				" flags=%X" "\n", nOutIndex, *pWidth,*pHeight, info.offset, info.size,
				info.presentationTimeUs, info.flags);*/

			*pOutSize = nOutBufferSize;
		}
		m_nGetFrameCount++;
		ff_AMediaCodec_releaseOutputBuffer(m_pH264Decoder, nOutIndex, 0);
		LOGE("DWY-------GetFrameCount[%d],AddFrameCount[%d]", m_nGetFrameCount, m_nFrameCount);

	} while (false);
	
	return pRet;

}

void CAndroidDecoder::DecoderFlush()
{
	if(m_pH264Decoder)
	{
		ff_AMediaCodec_flush(m_pH264Decoder);
	}
}
