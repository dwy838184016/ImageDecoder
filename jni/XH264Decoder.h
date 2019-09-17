#include "CAndroidDecoder.h"
#include "NeonDecoder.h"

class XH264Decoder
{
public:

	XH264Decoder();
	~XH264Decoder();

	CAndroidDecoder* m_H264Codec;
	NeonDecoder* m_NeonDecoder;
	bool m_IsSupportNeon;
	char m_filePath[512];
	//1-----单个播放；2----循环播放
	int m_playType;
	bool m_isStop;
	int m_decoderWidth;
	int m_decoderHeight;
	int m_outWidth;
	int m_outHeight;
	long m_frameDurationValue;
};
