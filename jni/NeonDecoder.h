#include <arm_neon.h>
#include <android/log.h>
#include "cpu-features.h"
#include <string.h>
#include <stdlib.h>

#define  LOG_TAG    "NeonDecoder"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

class NeonDecoder
{
public:

	NeonDecoder();
	~NeonDecoder();
	void NV12toBGRA(uint8_t *yuv, uint8_t *rgb, int nSrcWidth, int nSrcHeight, int nSrcStride, int nSrcHeightStride, int nOutWidth, int nOutHeight);
	void YUV420toBGRA(uint8_t *yuv, uint8_t *rgb, int nSrcWidth, int nSrcHeight, int nSrcStride, int nSrcHeightStride, int nOutWidth, int nOutHeight);
	bool IsSupportNeonCommand();

private:
	void Neon_NV12_To_BGRA(uint8_t * __restrict src_y, uint8_t *__restrict src_uv, uint8_t * __restrict dest_rgb, int n);
	void Neon_NV12_To_BGRA2(uint8_t * __restrict src_y, uint8_t *__restrict src_uv, uint8_t * __restrict dest_rgb, int n);
    void Neon_YUV420_To_BGRA(uint8_t * __restrict src_y, uint8_t *__restrict src_u, uint8_t *__restrict src_v, uint8_t * __restrict dest_rgb, int n);
	void Neon_YUV420_To_BGRA2(uint8_t * __restrict src_y, uint8_t *__restrict src_u, uint8_t *__restrict src_v, uint8_t * __restrict dest_rgb, int n);
	void Yuv2BgraForNV12(uint8_t *y, uint8_t *uv, uint8_t *rgb, int w);
	void C_Convert(uint8_t y, uint8_t u, uint8_t v,uint8_t *rgb);
	void C_Convert2(uint8_t y, uint8_t u, uint8_t v,uint8_t *rgb);
};
