#include "NeonDecoder.h"

NeonDecoder :: NeonDecoder()
{

}

NeonDecoder :: ~NeonDecoder()
{

}

bool NeonDecoder :: IsSupportNeonCommand()
{
	if (android_getCpuFamily() != ANDROID_CPU_FAMILY_ARM) {
		LOGE("Not is ARM CPU.");
		return false;
	}

	uint64_t features = android_getCpuFeatures();
	if ((features & ANDROID_CPU_ARM_FEATURE_ARMv7) == 0) {
		LOGE("Not an ARMv7 CPU !");
		return false;
	}

	if ((features & ANDROID_CPU_ARM_FEATURE_NEON) == 0) {
		LOGE("CPU doesn't support NEON !");
		return false;
	}

	LOGE("CPU support NEON !");
	return true;
}

void NeonDecoder :: NV12toBGRA(uint8_t *yuv, uint8_t *rgb, int nSrcWidth, int nSrcHeight, int nSrcStride, int nSrcHeightStride, int nOutWidth, int nOutHeight)
{
	uint8_t* src_y = yuv;
	uint8_t* src_uv = yuv + nSrcStride*nSrcHeightStride;
	uint8_t* dst_rgb;
	dst_rgb = rgb;
	/*count++;

	if(nOutWidth == 1680 && nOutHeight == 1024 && count == 15){
		FILE* fp = NULL;
		fp = fopen("/mnt/sdcard/testyuv.yuv", "r");
		if(fp == NULL)
		{
			fp = fopen("/mnt/sdcard/testyuv.yuv", "wb");
			if(fp)
			{
				fwrite(src_y, nSrcStride*nOutHeight, 1, fp);
				fwrite(src_uv, nSrcStride*nOutHeight/2, 1, fp);
				fclose(fp);
				fp == NULL;
			}
		}
		else
		{
			fclose(fp);
			fp = NULL;
		}
	}*/

//	LOGE("SrcWidth[%d], SrcHeight[%d], WidthStride[%d], HeightStride[%d], OutWidth[%d], OutHeight[%d]",
//			nSrcWidth, nSrcHeight, nSrcStride, nSrcHeightStride, nOutWidth, nOutHeight);
	for(int i = 0; i<nOutHeight; i++ )//每次读取一行
	{
//		Neon_NV12_To_BGRA(src_y, src_uv, dst_rgb, nSrcWidth);
		Neon_NV12_To_BGRA2(src_y, src_uv, dst_rgb, nSrcWidth);

		dst_rgb += nOutWidth*4;
		src_y += nSrcStride;

		if (i & 1)
		{
			src_uv += nSrcStride;
		}
	}
}

void NeonDecoder :: YUV420toBGRA(uint8_t *yuv, uint8_t *rgb, int nSrcWidth, int nSrcHeight, int nSrcStride, int nSrcHeightStride, int nOutWidth, int nOutHeight)
{
	uint8_t * src_y = yuv;
	uint8_t * src_u = yuv + nSrcStride*nSrcHeightStride;
	uint8_t * src_v = src_u + nSrcStride*nSrcHeightStride/4;
	uint8_t * dst_rgb;
	dst_rgb = rgb;

	for(int i = 0; i<nOutHeight; i++ )//每次读取一行
	{

		//Neon_YUV420_To_BGRA(src_y, src_u, src_v, dst_rgb, nSrcWidth);
		Neon_YUV420_To_BGRA2(src_y, src_u, src_v, dst_rgb, nSrcWidth);

		dst_rgb += nOutWidth*4;
		src_y += nSrcStride;

		if (i & 1 )
		{
			src_u += nSrcStride/2;
			src_v += nSrcStride/2;
		}
	}

}

/**使用neon， YUV分量与RGB分量间的转化*/
/**
 * r = (256>>2*y + 403>>2*(v-128)) >> 6;//R分量
 * g = (256>>2*y - 48>>2*(u-128)-120>>2*(v-128)) >> 6;//G分量
 * b = (256>>2*y + 475>>2*(u-128)) >> 6;//B分量
 */
void NeonDecoder :: Neon_NV12_To_BGRA(uint8_t * __restrict src_y, uint8_t *__restrict src_uv, uint8_t * __restrict dest_rgb, int n)
{
    
	uint8x8_t all_yfac = vdup_n_u8(256>>2);
	int8x8_t g_ufac = vdup_n_s8(48>>2);
	int8x8_t b_ufac = vdup_n_s8(475>>2);
	int8x8_t r_vfac = vdup_n_s8(403>>2);
	int8x8_t g_vfac = vdup_n_s8(120>>2);
	int8x8_t addfac = vdup_n_s8(128);
    
/* 	
    uint8x8_t all_yfac = vdup_n_u8(256>>2);
    int8x8_t g_ufac = vdup_n_s8(88>>2);
    int8x8_t b_ufac = vdup_n_s8(455>>2);
    int8x8_t r_vfac = vdup_n_s8(360>>2);
    int8x8_t g_vfac = vdup_n_s8(184>>2);
    int8x8_t addfac = vdup_n_s8(128);
	*/

	int count = n % 16;
	uint8_t *n_py = src_y;
	uint8_t *n_puv = src_uv;

	n /= 16;//每次处理16位

	for(int i=0 ; i<n ; i++){

		int16x8_t tmpData;
		uint8x8x2_t r, g, b;
		uint16x8x2_t y_data;

		uint8x8x2_t y = vld2_u8(n_py);//将Y分量交错存储为Y1、Y2分量，正好与UV分量值一一对应
		int8x8x2_t uv = vld2_s8((const int8_t*)n_puv);//U、V向量

		uv.val[0] = vsub_s8(uv.val[0],addfac);//u-128
		uv.val[1] = vsub_s8(uv.val[1],addfac);//v-128

		/*
		 * 得到两组RGB数据，但数据是交错的
		 * 如：
		 * r0 : (r0 r2 r4 r6 r8 r10 r12 r14)
		 * r1 : (r1 r3 r5 r7 r9 r11 r13 r15)
		 */
		for(int j=0 ; j<2 ; j++){

			y_data.val[j] = vshll_n_u8(y.val[j], 6);
			tmpData = vmlal_s8(vreinterpretq_s16_u16(y_data.val[j]), uv.val[1], r_vfac);
			r.val[j] = vqshrun_n_s16(tmpData, 6);	//R数据

			tmpData = vmlsl_s8(vreinterpretq_s16_u16(y_data.val[j]), uv.val[0], g_ufac);
			tmpData = vmlsl_s8(tmpData, uv.val[1], g_vfac);
			g.val[j] = vqshrun_n_s16(tmpData, 6);	//G数据

			tmpData = vmlal_s8(vreinterpretq_s16_u16(y_data.val[j]), uv.val[0], b_ufac);
			b.val[j] = vqshrun_n_s16(tmpData, 6);	//B数据
		}

		/*
		 * 对交错的RGB数据进行交织，得到
		 * 如：
		 * r0 : (r0 r1 r2 r3 r4 r5 r6 r7)
		 * r1 : (r8 r9 r10 r11 r12 r13 r14 r15)
		 */
		uint8x8x2_t r_data = vzip_u8(r.val[0], r.val[1]);
		uint8x8x2_t g_data = vzip_u8(g.val[0], g.val[1]);
		uint8x8x2_t b_data = vzip_u8(b.val[0], b.val[1]);

		//将两组RGB数据加载到内存中
		uint8x8x4_t resultRgb;
		for(int j=0 ; j<2 ; j++){

			resultRgb.val[0] = r_data.val[j];
			resultRgb.val[1] = g_data.val[j];
			resultRgb.val[2] = b_data.val[j];
			resultRgb.val[3] = vdup_n_u8(255);
//			resultRgb.val[0] = b_data.val[j];
//			resultRgb.val[1] = g_data.val[j];
//			resultRgb.val[2] = r_data.val[j];
//			resultRgb.val[3] = vdup_n_u8(255);
			vst4_u8(dest_rgb, resultRgb);
			dest_rgb += 8*4;
		}

		n_py += 16;
		n_puv += 16;
	}

	if(count > 0)
	{
		Yuv2BgraForNV12(n_py, n_puv, dest_rgb, count);
	}
}

/**
 * r = (298>>2*(y-16) + 409>>2*(v-128)) >> 6;//R分量
 * g = (298>>2*(y-16) - 100>>2*(u-128)-208>>2*(v-128)) >> 6;//G分量
 * b = (298>>2*(y-16) + 516>>2*(u-128)) >> 6;//B分量
 */
void NeonDecoder :: Neon_NV12_To_BGRA2(uint8_t * __restrict src_y, uint8_t *__restrict src_uv, uint8_t * __restrict dest_rgb, int n)
{

	uint8x8_t all_yfac = vdup_n_u8(298>>2);
	uint8x8_t t_yfac = vdup_n_u8(16);
	int16x4_t g_ufac = vdup_n_s16(101>>2);
	int16x4_t b_ufac = vdup_n_s16(519>>2);
	int16x4_t r_vfac = vdup_n_s16(411>>2);
	int16x4_t g_vfac = vdup_n_s16(211>>2);
	int16x8_t addfac = vdupq_n_s16(128);

	uint8_t *n_py = src_y;
	uint8_t *n_puv = src_uv;

	int count = n % 16;

	n /= 16;//每次处理16位

	for(int i=0 ; i<n ; i++){

		int16x4_t tmpDataH;
		int16x4_t tmpDataL;
		uint8x8x2_t r, g, b;
		int16x8x2_t y_data;

		uint8x8x2_t y = vld2_u8(n_py);//将Y分量交错存储为Y1、Y2分量，正好与UV分量值一一对应
		uint8x8x2_t uvRow = vld2_u8(n_puv);//U、V向量
		int16x8x2_t uv;

		uv.val[0] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(uvRow.val[0])),addfac);//u-128
		uv.val[1] = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(uvRow.val[1])),addfac);//v-128

		y.val[0] = vqsub_u8(y.val[0], t_yfac);//Y'=Y-16
		y.val[1] = vqsub_u8(y.val[1], t_yfac);//Y'=Y-16
		y_data.val[0] = vreinterpretq_s16_u16(vmull_u8(y.val[0], all_yfac));//Y'*298>>2
		y_data.val[1] = vreinterpretq_s16_u16(vmull_u8(y.val[1], all_yfac));//Y'*298>>2

		/*
		 * 得到两组RGB数据，但数据是交错的
		 * 如：
		 * r0 : (r0 r2 r4 r6 r8 r10 r12 r14)
		 * r1 : (r1 r3 r5 r7 r9 r11 r13 r15)
		 */
		for(int j=0 ; j<2 ; j++){

			/* r = (298>>2*(y-16) + 409>>2*(v-128)) >> 6 */
			tmpDataH = vmla_s16(vget_high_s16(y_data.val[j]), vget_high_s16(uv.val[1]), r_vfac);
			tmpDataL = vmla_s16(vget_low_s16(y_data.val[j]), vget_low_s16(uv.val[1]), r_vfac);
			r.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//R数据

			/* g = (298>>2*(y-16) - 100>>2*(u-128)-208>>2*(v-128)) >> 6; */
			tmpDataH = vmls_s16(vget_high_s16(y_data.val[j]), vget_high_s16(uv.val[0]), g_ufac);
			tmpDataH = vmls_s16(tmpDataH, vget_high_s16(uv.val[1]), g_vfac);
			tmpDataL = vmls_s16(vget_low_s16(y_data.val[j]), vget_low_s16(uv.val[0]), g_ufac);
			tmpDataL = vmls_s16(tmpDataL, vget_low_s16(uv.val[1]), g_vfac);
			g.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//G数据

			/* b = (298>>2*(y-16) + 516>>2*(u-128)) >> 6 */
			tmpDataH = vmla_s16(vget_high_s16(y_data.val[j]), vget_high_s16(uv.val[0]), b_ufac);
			tmpDataL = vmla_s16(vget_low_s16(y_data.val[j]), vget_low_s16(uv.val[0]), b_ufac);
			b.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//B数据

		}

		/*
		 * 对交错的RGB数据进行交织，得到
		 * 如：
		 * r0 : (r0 r1 r2 r3 r4 r5 r6 r7)
		 * r1 : (r8 r9 r10 r11 r12 r13 r14 r15)
		 */
		uint8x8x2_t r_data = vzip_u8(r.val[0], r.val[1]);
		uint8x8x2_t g_data = vzip_u8(g.val[0], g.val[1]);
		uint8x8x2_t b_data = vzip_u8(b.val[0], b.val[1]);

		//将两组RGB数据加载到内存中
		uint8x8x4_t resultRgb;
		for(int j=0 ; j<2 ; j++){

			resultRgb.val[0] = r_data.val[j];
			resultRgb.val[1] = g_data.val[j];
			resultRgb.val[2] = b_data.val[j];
			resultRgb.val[3] = vdup_n_u8(255);
//			resultRgb.val[0] = b_data.val[j];
//			resultRgb.val[1] = g_data.val[j];
//			resultRgb.val[2] = r_data.val[j];
//			resultRgb.val[3] = vdup_n_u8(255);
			vst4_u8(dest_rgb, resultRgb);
			dest_rgb += 8*4;
		}

		n_py += 16;
		n_puv += 16;
	}

	if(count > 0)
	{
		for(int i = 0;i<count; i+=2)//解析一行的数据
		{
			C_Convert2(n_py[0],n_puv[0],n_puv[1],dest_rgb+0);
			C_Convert2(n_py[1],n_puv[0],n_puv[1],dest_rgb+4);

			n_py +=2;
			n_puv += 2;
			dest_rgb +=8;
		}
	}
}

/**使用neon， YUV分量与RGB分量间的转化*/
/**
 * r = (256>>2*y + 403>>2*(v-128)) >> 6;//R分量
 * g = (256>>2*y - 48>>2*(u-128)-120>>2*(v-128)) >> 6;//G分量
 * b = (256>>2*y + 475>>2*(u-128)) >> 6;//B分量
 */
void NeonDecoder :: Neon_YUV420_To_BGRA(uint8_t * __restrict src_y, uint8_t *__restrict src_u, uint8_t *__restrict src_v, uint8_t * __restrict dest_rgb, int n)
{
	uint8_t *n_py = src_y;
	uint8_t *n_pu = src_u;
	uint8_t *n_pv = src_v;

	uint8x8_t all_yfac = vdup_n_u8(256>>2);
	int8x8_t g_ufac = vdup_n_s8(88>>2);
	int8x8_t b_ufac = vdup_n_s8(454>>2);
	int8x8_t r_vfac = vdup_n_s8(359>>2);
	int8x8_t g_vfac = vdup_n_s8(183>>2);

	int8x8_t addfac = vdup_n_s8(128);

	int count = n % 16;

	n /= 16;//每次处理16位

	for(int i=0 ; i<n ; i++){

		int16x8_t tmpData;
		uint8x8x2_t r, g, b;
		uint16x8x2_t y_data;

		uint8x8x2_t y = vld2_u8(n_py);//将Y分量交错存储为Y1、Y2分量，正好与UV分量值一一对应
		int8x8_t u = vld1_s8((const int8_t*)n_pu);
		int8x8_t v = vld1_s8((const int8_t*)n_pv);

		u = vsub_s8(u,addfac);//u-128
		v = vsub_s8(v,addfac);//v-128

		/*
		 * 得到两组RGB数据，但数据是交错的
		 * 如：
		 * r0 : (r0 r2 r4 r6 r8 r10 r12 r14)
		 * r1 : (r1 r3 r5 r7 r9 r11 r13 r15)
		 */
		for(int j=0 ; j<2 ; j++){
			y_data.val[j] = vmull_u8(y.val[j], all_yfac);

			tmpData = vmlal_s8(vreinterpretq_s16_u16(y_data.val[j]), v, r_vfac);
			r.val[j] = vqshrun_n_s16(tmpData, 6);	//R数据

			tmpData = vmlsl_s8(vreinterpretq_s16_u16(y_data.val[j]), u, g_ufac);
			tmpData = vmlsl_s8(tmpData, v, g_vfac);
			g.val[j] = vqshrun_n_s16(tmpData, 6);	//G数据

			tmpData = vmlal_s8(vreinterpretq_s16_u16(y_data.val[j]), u, b_ufac);
			b.val[j] = vqshrun_n_s16(tmpData, 6);	//B数据
		}

		/*
		 * 对交错的RGB数据进行交织，得到
		 * 如：
		 * r0 : (r0 r1 r2 r3 r4 r5 r6 r7)
		 * r1 : (r8 r9 r10 r11 r12 r13 r14 r15)
		 */
		uint8x8x2_t r_data = vzip_u8(r.val[0], r.val[1]);
		uint8x8x2_t g_data = vzip_u8(g.val[0], g.val[1]);
		uint8x8x2_t b_data = vzip_u8(b.val[0], b.val[1]);

		//将两组RGB数据加载到内存中
		uint8x8x4_t resultRgb;
		for(int j=0 ; j<2 ; j++){

			resultRgb.val[0] = r_data.val[j];
			resultRgb.val[1] = g_data.val[j];
			resultRgb.val[2] = b_data.val[j];
			resultRgb.val[3] = vdup_n_u8(255);
//			resultRgb.val[0] = b_data.val[j];
//			resultRgb.val[1] = g_data.val[j];
//			resultRgb.val[2] = r_data.val[j];
//			resultRgb.val[3] = vdup_n_u8(255);
			vst4_u8(dest_rgb, resultRgb);
			dest_rgb += 8*4;
		}

		n_py += 16;
		n_pu += 8;
		n_pv += 8;
	}

	if(count > 0)
	{
		for(int i = 0;i<count; i+=2)//解析一行的数据
		{
			C_Convert(n_py[0],n_pu[0],n_pv[0],dest_rgb+0);
			C_Convert(n_py[1],n_pu[0],n_pv[0],dest_rgb+4);

			n_py +=2;
			n_pu += 1;
			n_pv += 1;
			dest_rgb +=8;
		}
	}
}

/**
 * r = (298>>2*(y-16) + 409>>2*(v-128)) >> 6;//R分量
 * g = (298>>2*(y-16) - 100>>2*(u-128)-208>>2*(v-128)) >> 6;//G分量
 * b = (298>>2*(y-16) + 516>>2*(u-128)) >> 6;//B分量
 */
void NeonDecoder :: Neon_YUV420_To_BGRA2(uint8_t * __restrict src_y, uint8_t *__restrict src_u, uint8_t *__restrict src_v, uint8_t * __restrict dest_rgb, int n)
{
	uint8_t *n_py = src_y;
	uint8_t *n_pu = src_u;
	uint8_t *n_pv = src_v;

	uint8x8_t all_yfac = vdup_n_u8(298>>2);
	uint8x8_t t_yfac = vdup_n_u8(16);
	int16x4_t g_ufac = vdup_n_s16(101>>2);
	int16x4_t b_ufac = vdup_n_s16(519>>2);
	int16x4_t r_vfac = vdup_n_s16(411>>2);
	int16x4_t g_vfac = vdup_n_s16(211>>2);
	int16x8_t addfac = vdupq_n_s16(128);

	int count = n % 16;

	n /= 16;//每次处理16位

	for(int i=0 ; i<n ; i++){

		int16x4_t tmpDataH;
		int16x4_t tmpDataL;
		uint8x8x2_t r, g, b;
		int16x8x2_t y_data;

		uint8x8x2_t y = vld2_u8(n_py);//将Y分量交错存储为Y1、Y2分量，正好与UV分量值一一对应
		uint8x8_t uRow = vld1_u8(n_pu);
		uint8x8_t vRow = vld1_u8(n_pv);
		int16x8_t u;
		int16x8_t v;

		u = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(uRow)),addfac);//u-128
		v = vsubq_s16(vreinterpretq_s16_u16(vmovl_u8(vRow)),addfac);//v-128

		y.val[0] = vqsub_u8(y.val[0], t_yfac);//Y'=Y-16
		y.val[1] = vqsub_u8(y.val[1], t_yfac);//Y'=Y-16
		y_data.val[0] = vreinterpretq_s16_u16(vmull_u8(y.val[0], all_yfac));//Y'*298>>2
		y_data.val[1] = vreinterpretq_s16_u16(vmull_u8(y.val[1], all_yfac));//Y'*298>>2

		/*
		 * 得到两组RGB数据，但数据是交错的
		 * 如：
		 * r0 : (r0 r2 r4 r6 r8 r10 r12 r14)
		 * r1 : (r1 r3 r5 r7 r9 r11 r13 r15)
		 */
		for(int j=0 ; j<2 ; j++){
			/* r = (298>>2*(y-16) + 409>>2*(v-128)) >> 6 */
			tmpDataH = vmla_s16(vget_high_s16(y_data.val[j]), vget_high_s16(v), r_vfac);
			tmpDataL = vmla_s16(vget_low_s16(y_data.val[j]), vget_low_s16(v), r_vfac);
			r.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//R数据

			//g = (298>>2*(y-16) - 100>>2*(u-128)-208>>2*(v-128)) >> 6;//G分量
			tmpDataH = vmls_s16(vget_high_s16(y_data.val[j]), vget_high_s16(u), g_ufac);
			tmpDataH = vmls_s16(tmpDataH, vget_high_s16(v), g_vfac);
			tmpDataL = vmls_s16(vget_low_s16(y_data.val[j]), vget_low_s16(u), g_ufac);
			tmpDataL = vmls_s16(tmpDataL, vget_low_s16(v), g_vfac);
			g.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//G数据

			//b = (298>>2*(y-16) + 516>>2*(u-128)) >> 6;//B分量
			tmpDataH = vmla_s16(vget_high_s16(y_data.val[j]), vget_high_s16(u), b_ufac);
			tmpDataL = vmla_s16(vget_low_s16(y_data.val[j]), vget_low_s16(u), b_ufac);
			b.val[j] = vqshrun_n_s16(vcombine_s16(tmpDataL, tmpDataH), 6);	//B数据
		}

		/*
		 * 对交错的RGB数据进行交织，得到
		 * 如：
		 * r0 : (r0 r1 r2 r3 r4 r5 r6 r7)
		 * r1 : (r8 r9 r10 r11 r12 r13 r14 r15)
		 */
		uint8x8x2_t r_data = vzip_u8(r.val[0], r.val[1]);
		uint8x8x2_t g_data = vzip_u8(g.val[0], g.val[1]);
		uint8x8x2_t b_data = vzip_u8(b.val[0], b.val[1]);

		//将两组RGB数据加载到内存中
		uint8x8x4_t resultRgb;
		for(int j=0 ; j<2 ; j++){

			resultRgb.val[0] = r_data.val[j];
			resultRgb.val[1] = g_data.val[j];
			resultRgb.val[2] = b_data.val[j];
			resultRgb.val[3] = vdup_n_u8(255);
//			resultRgb.val[0] = b_data.val[j];
//			resultRgb.val[1] = g_data.val[j];
//			resultRgb.val[2] = r_data.val[j];
//			resultRgb.val[3] = vdup_n_u8(255);
			vst4_u8(dest_rgb, resultRgb);
			dest_rgb += 8*4;
		}

		n_py += 16;
		n_pu += 8;
		n_pv += 8;
	}

	if(count > 0)
	{
		for(int i = 0;i<count; i+=2)//解析一行的数据
		{
			C_Convert2(n_py[0],n_pu[0],n_pv[0],dest_rgb+0);
			C_Convert2(n_py[1],n_pu[0],n_pv[0],dest_rgb+4);

			n_py +=2;
			n_pu += 1;
			n_pv += 1;
			dest_rgb +=8;
		}
	}
}

void NeonDecoder :: Yuv2BgraForNV12(uint8_t *y, uint8_t *uv, uint8_t *rgb, int w)
{
	uint8_t *tmpY = y;
	uint8_t *tmpUV = uv;
	uint8_t *tmpRGB = rgb;
	for(int i = 0;i<w; i+=2)//解析一行的数据
	{
		C_Convert(tmpY[0],tmpUV[0],tmpUV[1],tmpRGB+0);
		C_Convert(tmpY[1],tmpUV[0],tmpUV[1],tmpRGB+4);
		tmpY +=2;
		tmpUV +=2;
		tmpRGB +=8;
	}
}

/* 使用C语言， YUV分量与RGB分量间的转化
 * ******************YUV转化RGB矩阵******************
 *                +++                              +++
 *		**  **    +  ***             ***  **     **  +
 *		*  R *    +  *  256    0   403 *  *   Y   *  +
 *		*  G *  = +  *  256  -48  -120 *  * U-128 *  +  >> 8
 *		*  B *	  +	 *  256  475    0  *  * V-128 *  +
 *		**  **    +  ***             ***  **     **  +
 *                +++                              +++
 *
 */
void NeonDecoder :: C_Convert(uint8_t y, uint8_t u, uint8_t v,uint8_t *rgb)
{
	uint8_t *pRGB = rgb;
	int r,g,b;

	r = (256*y + 403*(v-128)) >> 8;//R分量
	g = (256*y - 48*(u-128)-120*(v-128)) >>8;//G分量
	b = (256*y + 475*(u-128)) >> 8;//B分量

//	pRGB[0] = (uint8_t)(r < 0) ? 0 : (r > 255 ? 255 : r);//R分量
//	pRGB[1] = (uint8_t)(g < 0) ? 0 : (g > 255 ? 255 : g);//G分量
//	pRGB[2] = (uint8_t)(b < 0) ? 0 : (b > 255 ? 255 : b);//B分量
//	pRGB[3] = 0xFF;//A分量
	pRGB[0] = (uint8_t)(b < 0) ? 0 : (b > 255 ? 255 : b);//B分量
	pRGB[1] = (uint8_t)(g < 0) ? 0 : (g > 255 ? 255 : g);//G分量
	pRGB[2] = (uint8_t)(r < 0) ? 0 : (r > 255 ? 255 : r);//R分量
	pRGB[3] = 0xFF;//A分量
}

/* 使用C语言， YUV分量与RGB分量间的转化
 * ******************YUV转化RGB矩阵******************
 *                +++                              +++
 *		**  **    +  ***              ***  **     **  +
 *		*  R *    +  *  298    0   409  *  * Y-16  *  +
 *		*  G *  = +  *  298  -100  -208 *  * U-128 *  +  >> 8
 *		*  B *	  +	 *  298  516    0   *  * V-128 *  +
 *		**  **    +  ***              ***  **     **  +
 *                +++                              +++
 *
 */
void NeonDecoder :: C_Convert2(uint8_t y, uint8_t u, uint8_t v,uint8_t *rgb)
{
	uint8_t *pRGB = rgb;
	int r,g,b;

	r = (298*(y-16) + 409*(v-128)) >> 8;//R分量
	g = (298*(y-16) - 100*(u-128)-208*(v-128)) >>8;//G分量
	b = (298*(y-16) + 516*(u-128)) >> 8;//B分量

//	pRGB[0] = (uint8_t)(r < 0) ? 0 : (r > 255 ? 255 : r);//R分量
//	pRGB[1] = (uint8_t)(g < 0) ? 0 : (g > 255 ? 255 : g);//G分量
//	pRGB[2] = (uint8_t)(b < 0) ? 0 : (b > 255 ? 255 : b);//B分量
//	pRGB[3] = 0xFF;//A分量
	pRGB[0] = (uint8_t)(b < 0) ? 0 : (b > 255 ? 255 : b);//B分量
	pRGB[1] = (uint8_t)(g < 0) ? 0 : (g > 255 ? 255 : g);//G分量
	pRGB[2] = (uint8_t)(r < 0) ? 0 : (r > 255 ? 255 : r);//R分量
	pRGB[3] = 0xFF;//A分量
}
