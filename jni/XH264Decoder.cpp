#include "XH264Decoder.h"

XH264Decoder :: XH264Decoder()
{
	m_H264Codec = new CAndroidDecoder();
	m_NeonDecoder = new NeonDecoder();
	m_IsSupportNeon = m_NeonDecoder->IsSupportNeonCommand();
	memset(m_filePath, 0, 512);
	m_playType = 1;
	m_isStop = false;
	m_decoderWidth = 0;
	m_decoderHeight = 0;
	m_outWidth = 0;
	m_outHeight = 0;
	m_frameDurationValue = 0;
}

XH264Decoder :: ~XH264Decoder()
{
	if(m_H264Codec)
	{
		delete m_H264Codec;
	}

	if(m_NeonDecoder)
	{
		delete m_NeonDecoder;
	}
}
