#include <jni.h>
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include <android/bitmap.h>
#include <stdio.h>
#include "avlog.h"
#include <unistd.h>
#include <time.h>
extern "C"
{
	#include "avjni.h"
}

#include "com_example_vediodecoder_NativeMethod.h"
#include "XH264Decoder.h"
#include "XH264Reader.h"

#define H264_MIME   "video/avc"
jmethodID jMethodID = NULL;
jclass classId = NULL;
JavaVM* m_vm = NULL;
int version = 0;

JNIEXPORT jlong JNICALL Java_com_example_vediodecoder_NativeMethod_createInstance
  (JNIEnv *env, jclass jc, jstring fileName, jint playType, jlong frameDurationValue)
{
	XH264Decoder* xh264Decoder = new XH264Decoder();

	const char* filePath = NULL;
	filePath = env->GetStringUTFChars(fileName, false);

	if(filePath)
	{
		memcpy(xh264Decoder->m_filePath, filePath, strlen(filePath));
		LOGI("File path[%s]", xh264Decoder->m_filePath);
	}

	xh264Decoder->m_playType = playType;
	xh264Decoder->m_frameDurationValue = frameDurationValue;

	env->ReleaseStringUTFChars(fileName, filePath);



	return (jlong)xh264Decoder;
}

JNIEXPORT jboolean JNICALL Java_com_example_vediodecoder_NativeMethod_initMediaCodec
  (JNIEnv *env, jclass jc, jlong instance, jint width, jint height)
{
	XH264Decoder* xh264Decoder = (XH264Decoder*)instance;
	if(xh264Decoder)
	{
		xh264Decoder->m_decoderWidth = width;
		xh264Decoder->m_decoderHeight = height;
		return xh264Decoder->m_H264Codec->InitDecoder(H264_MIME, width, height, -1);
	}
	else
	{
		LOGE("XH264Decoder Object is null.");
		return false;
	}
}

#if 0
JNIEXPORT void JNICALL Java_com_example_vediodecoder_NativeMethod_startPlay
  (JNIEnv *env, jclass jc, jlong instance, jint width, jint height, jobject surface)
{
	XH264Decoder* xh264Decoder = (XH264Decoder*)instance;
	if(xh264Decoder)
	{
		ANativeWindow* nWindow = ANativeWindow_fromSurface(env, surface);

		int current_read_len = 0;
		int tmpbuf_len = 0;
		char* tmpbuf = (char*)malloc(width * height * 10);
//		char* tmpbuf = NULL;
		uint8_t* pOutData = (uint8_t*)malloc(width * height * 4);

		XH264Reader* xh264Reader = new XH264Reader();
		xh264Reader->H264FrameReader_Init(xh264Decoder->m_filePath);
		int count = 0;

		LOGE("H264 file size[%d]", xh264Reader->filesize_);
		while (!xh264Decoder->m_isStop)
		{
			if(current_read_len >= xh264Reader->filesize_){
				if(xh264Decoder->m_playType == 1){
					xh264Decoder->m_isStop = true;
					break;
				}
				else if(xh264Decoder->m_playType == 2)
				{
					xh264Reader->H264FrameReader_Reset();
					current_read_len = 0;
				}
			}

			if (xh264Reader->H264FrameReader_ReadFrame(tmpbuf, &tmpbuf_len))
			{
				LOGE("tmpbuf_len = %d\n", tmpbuf_len);
				if(tmpbuf_len == 0)
				{
					continue;
				}

				int nLeft = 0;
				int nDecodePixtype = 0;
				int nDecodeStride = 0;
				int nDecodeWidth = 0;
				int nDecodeHeight = 0;
				int getcount = 0;
				uint8_t* pDecodeData = NULL;
				int nDecodedSize = 0;

				bool bRet = xh264Decoder->m_H264Codec->AddFrameData((unsigned char*)tmpbuf, tmpbuf_len, &nLeft);
				pDecodeData = xh264Decoder->m_H264Codec->GetFrameOut(false, &nDecodeWidth, &nDecodeHeight, &nDecodePixtype, &nDecodeStride, &nDecodedSize);

				while (pDecodeData == NULL && getcount < 10)
				{
					bRet = xh264Decoder->m_H264Codec->AddFrameData((unsigned char*)tmpbuf, tmpbuf_len, &nLeft);
					pDecodeData = xh264Decoder->m_H264Codec->GetFrameOut(false, &nDecodeWidth, &nDecodeHeight, &nDecodePixtype, &nDecodeStride, &nDecodedSize);
					getcount++;
				}

				if(pDecodeData)
				{
					if(xh264Decoder->m_IsSupportNeon){
						if(nDecodePixtype == 21){//NV12
							xh264Decoder->m_NeonDecoder->NV12toBGRA(pDecodeData, pOutData, width, nDecodeHeight, nDecodeWidth, width, height);
						} else if(nDecodePixtype == 19){//YUV420P
							xh264Decoder->m_NeonDecoder->YUV420toBGRA(pDecodeData, pOutData, width, nDecodeHeight, nDecodeWidth, width, height);
						}

						ANativeWindow_Buffer outBuffer ;

						ANativeWindow_setBuffersGeometry(nWindow, width, height, WINDOW_FORMAT_RGBA_8888);
						int lockStatus = ANativeWindow_lock(nWindow, &outBuffer, NULL);
						if(lockStatus == 0){
							memcpy(outBuffer.bits, pOutData, width*height*4);
							outBuffer.format = WINDOW_FORMAT_RGBA_8888;
							ANativeWindow_unlockAndPost(nWindow);
						}
					}
				}
				else
				{
					LOGE("Decoder data is null.");
				}

				current_read_len += tmpbuf_len;

				if(pDecodeData){
					free(pDecodeData);
				}

			}
		}

		LOGE("current_read_len[%d]", current_read_len);
		free(pOutData);
		if(tmpbuf)
		{
			free(tmpbuf);
		}

		xh264Reader->H264FrameReader_Free();

		delete xh264Decoder;
	}
	else
	{
		LOGE("Start display failed. XH264Decoder Object is null.");
	}
}
#endif

JNIEXPORT void JNICALL Java_com_example_vediodecoder_NativeMethod_startPlay
  (JNIEnv *env, jclass jc, jlong instance, jint width, jint height, jobject surface)
{
	XH264Decoder* xh264Decoder = (XH264Decoder*)instance;
	if(xh264Decoder)
	{
		ANativeWindow* nWindow = ANativeWindow_fromSurface(env, surface);

		int current_read_len = 0;
		int tmpbuf_len = 0;
		char* tmpbuf = (char*)malloc(width * height * 10);
		uint8_t* pOutData = (uint8_t*)malloc(width * height * 4);
		int decodeWidth = 0;
		int decodeHeight = 0;

		XH264Reader* xh264Reader = new XH264Reader();
		xh264Reader->H264FrameReader_Init(xh264Decoder->m_filePath);
		int count = 0;
		int noneDisplayCount = 0;

		LOGE("H264 file size[%d]", xh264Reader->filesize_);
		char fileName[256] = {NULL};
		char stime[256] = { NULL };
		time_t now_time;
		time(&now_time);
		strftime(stime, sizeof(stime), "%Y%m%d%H%M%S", localtime(&now_time));
//		struct timeval time;
//		gettimeofday(&time,NULL);
//		sprintf(fileName, "/mnt/sdcard/real_data_1664x1080_%dmsdelay_%s.h264", xh264Decoder->m_frameDurationValue, stime);
//		FILE* m_fp = fopen("/mnt/sdcard/real_data_123.h264", "wb");
//		xh264Decoder->m_H264Codec->DecoderFlush();
		while (!xh264Decoder->m_isStop)
		{
			if(current_read_len >= xh264Reader->filesize_){
//				if(m_fp)
//				{
//					fclose(m_fp);
//					m_fp = NULL;
//				}

				if(xh264Decoder->m_playType == 1){
					xh264Decoder->m_isStop = true;
					break;
				}
				else if(xh264Decoder->m_playType == 2)
				{
					xh264Reader->H264FrameReader_Reset();
					current_read_len = 0;
				}
			}

			LOGE("/****************************Frame Start********************************/");
			if (xh264Reader->H264FrameReader_ReadFrame2(tmpbuf, &tmpbuf_len, &decodeWidth, &decodeHeight))
			{
				bool isEnd = false;
				current_read_len += tmpbuf_len;
				/*if(current_read_len >= xh264Reader->filesize_)
				{
					isEnd = true;
				}*/
				if(xh264Decoder->m_decoderWidth != decodeWidth
						|| xh264Decoder->m_decoderHeight != decodeHeight)
				{
//					xh264Decoder->m_H264Codec->FinitDecoder();
					/*if((xh264Decoder->m_decoderWidth == 1664 && xh264Decoder->m_decoderHeight == 1080))
					{
						if(m_fp)
						{
							fclose(m_fp);
							m_fp = NULL;
						}
					}*/
					xh264Decoder->m_H264Codec->InitDecoder(H264_MIME, decodeWidth%2==0?decodeWidth:decodeWidth+1,
							decodeHeight%2==0?decodeHeight:decodeHeight+1, -1);
					xh264Decoder->m_decoderWidth = decodeWidth;
					xh264Decoder->m_decoderHeight = decodeHeight;
				}

				LOGE("tmpbuf_len = %d\n", tmpbuf_len);
				if(tmpbuf_len == 0)
				{
					continue;
				}

				int nLeft = 0;
				int nDecodePixtype = 0;
				int nDecodeStride = 0;
				int nDecodeWidth = 0;
				int nDecodeHeight = 0;
				int getcount = 0;
				uint8_t* pDecodeData = NULL;
				int nDecodedSize = 0;
				struct timeval start, end;

				/*if(m_fp)
				{
					fwrite(tmpbuf, tmpbuf_len, 1, m_fp);
				}*/

				gettimeofday(&start,NULL);
				bool bRet = xh264Decoder->m_H264Codec->AddFrameData((unsigned char*)tmpbuf, tmpbuf_len, &nLeft, isEnd);
				pDecodeData = xh264Decoder->m_H264Codec->GetFrameOut(false, &nDecodeWidth, &nDecodeHeight, &nDecodePixtype, &nDecodeStride, &nDecodedSize);
				gettimeofday(&end,NULL);
				LOGE("DWY-----Mediacodec decode time is %ld ms", (end.tv_sec-start.tv_sec)*1000
																							+(end.tv_usec-start.tv_usec)/1000);//(endTime-startTime));

				/*if(isEnd)
				{
					xh264Decoder->m_H264Codec->DecoderFlush();
				}*/
				/*while (pDecodeData == NULL && getcount < 10)
				{
					if(m_fp && decodeWidth == 1664 && decodeHeight == 1080)
					{
						fwrite(tmpbuf, tmpbuf_len, 1, m_fp);
					}
					bRet = xh264Decoder->m_H264Codec->AddFrameData((unsigned char*)tmpbuf, tmpbuf_len, &nLeft);
					pDecodeData = xh264Decoder->m_H264Codec->GetFrameOut(false, &nDecodeWidth, &nDecodeHeight, &nDecodePixtype, &nDecodeStride, &nDecodedSize);
					getcount++;
				}*/

				if(pDecodeData)
				{
					if(xh264Decoder->m_IsSupportNeon){
						/*int tmpWidth = width > xh264Decoder->m_decoderWidth ? xh264Decoder->m_decoderWidth : width;
						int tmpHeight = height > xh264Decoder->m_decoderHeight ? xh264Decoder->m_decoderHeight : height;*/
						int tmpWidth = width > xh264Decoder->m_decoderWidth ? xh264Decoder->m_decoderWidth : width;
						int tmpHeight = height > xh264Decoder->m_decoderHeight ? xh264Decoder->m_decoderHeight : height;

						tmpWidth = tmpWidth % 2 == 0 ? tmpWidth : tmpWidth - 1;
						tmpHeight = tmpHeight % 2 == 0 ? tmpHeight : tmpHeight - 1;
						/*LOGE("DWY-----DecodeWidth[%d], DecodeHeight[%d], Width[%d], Height[%d], Type[%d]",
								xh264Decoder->m_decoderWidth, xh264Decoder->m_decoderHeight, tmpWidth, tmpHeight, nDecodePixtype);*/
						gettimeofday(&start,NULL);
						if(nDecodePixtype == 21){//NV12
							xh264Decoder->m_NeonDecoder->NV12toBGRA(pDecodeData, pOutData,
									tmpWidth, tmpHeight, nDecodeWidth, nDecodeHeight, tmpWidth, tmpHeight);
							count ++;
						} else if(nDecodePixtype == 19){//YUV420P
							xh264Decoder->m_NeonDecoder->YUV420toBGRA(pDecodeData, pOutData, tmpWidth, tmpHeight, nDecodeWidth, nDecodeHeight, tmpWidth, tmpHeight);
							count ++;
						}
						gettimeofday(&end,NULL);
						LOGE("DWY-----Neon decode time is %ld ms", (end.tv_sec-start.tv_sec)*1000
																			+(end.tv_usec-start.tv_usec)/1000);//(endTime-startTime));

						/*if(count == 15){
							FILE* fp = fopen("/mnt/sdcard/testyuv.rgb", "wb");
							if(fp)
							{
								fwrite(pOutData, tmpWidth*tmpHeight*4, 1, fp);
								fclose(fp);
								fp = NULL;
							}
							LOGE("DWY---[%d], Save testyuv.rgb Success", count);
						}*/

						ANativeWindow_Buffer outBuffer ;

						ANativeWindow_setBuffersGeometry(nWindow, tmpWidth, tmpHeight, WINDOW_FORMAT_RGBA_8888);
						int lockStatus = ANativeWindow_lock(nWindow, &outBuffer, NULL);
						if(lockStatus == 0){
//							LOGE("Width[%d], Stride[%d]", outBuffer.width, outBuffer.stride);
							if(outBuffer.width == outBuffer.stride)
							{
								memcpy(outBuffer.bits, pOutData, tmpWidth*tmpHeight*4);
							}
							else
							{
								memset(outBuffer.bits, 0xFF, outBuffer.stride*outBuffer.height);
								for(int i=0; i<outBuffer.height; i++)
								{
									memcpy(outBuffer.bits+i*outBuffer.stride*4, pOutData+i*outBuffer.width*4, outBuffer.width*4);
								}
							}

							outBuffer.format = WINDOW_FORMAT_RGBA_8888;
							ANativeWindow_unlockAndPost(nWindow);
						}
					}
					noneDisplayCount = 0;
				}
				else
				{
					LOGE("DWY-----Decoder Data Is Null, Show Number.");
					noneDisplayCount ++;
					if(m_vm != NULL){
						JNIEnv *cur_env = NULL;
						int status = m_vm->GetEnv((void**)&(cur_env), version);
						if(status < 0)
						{
							LOGE("GetEnv failed, try again!");
							status = m_vm->AttachCurrentThread(&(cur_env), NULL);
							if(status < 0)
							{
								LOGE("GetEnv failed");
							}
						}
						if(status >= 0 && classId != NULL && jMethodID != NULL)
						{

							jobject bmp = cur_env->CallStaticObjectMethod(classId, jMethodID, width, height, noneDisplayCount);
							if(bmp != NULL){
								AndroidBitmapInfo infobmp;
								unsigned char* pixelsbmp;
								int ret = AndroidBitmap_getInfo(cur_env, bmp, &infobmp);
								if(ret < 0)
								{
									LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
								} else
								{
									ret = AndroidBitmap_lockPixels(cur_env, bmp, (void**)&pixelsbmp);
									if( ret < 0)
									{
										LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
									}
									else
									{
										ANativeWindow_Buffer outBuffer ;
										ANativeWindow_setBuffersGeometry(nWindow, width, height, WINDOW_FORMAT_RGBA_8888);
										int lockStatus = ANativeWindow_lock(nWindow, &outBuffer, NULL);
										if(lockStatus == 0){
				//							LOGE("Width[%d], Stride[%d]", outBuffer.width, outBuffer.stride);
											memcpy(outBuffer.bits, pixelsbmp, width*height*4);

											outBuffer.format = WINDOW_FORMAT_RGBA_8888;
											ANativeWindow_unlockAndPost(nWindow);
										}
										AndroidBitmap_unlockPixels(cur_env, bmp);
									}
								}



							}
						}
					}
				}

//				current_read_len += tmpbuf_len;

				if(pDecodeData){
					free(pDecodeData);
				}

//				LOGE("DWY----[%ld]", xh264Decoder->m_frameDurationValue);
				usleep(xh264Decoder->m_frameDurationValue*1000);

			}
			LOGE("/****************************Frame End********************************/");
		}

		LOGE("current_read_len[%d]", current_read_len);
		free(pOutData);
		if(tmpbuf)
		{
			free(tmpbuf);
		}

		/*if(m_fp)
		{
			fclose(m_fp);
			m_fp = NULL;
		}*/

		xh264Reader->H264FrameReader_Free();

		delete xh264Decoder;
	}
	else
	{
		LOGE("Start display failed. XH264Decoder Object is null.");
	}
}


JNIEXPORT void JNICALL Java_com_example_vediodecoder_NativeMethod_stopPlay
  (JNIEnv *env, jclass jc, jlong instance)
{
	XH264Decoder* xh264Decoder = (XH264Decoder*)instance;
	if(xh264Decoder)
	{
		xh264Decoder->m_isStop = true;
	}
	else
	{
		LOGE("Stop display failed. XH264Decoder Object is null.");
	}
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv *env;
	LOGE("JNI_OnLoad called");
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGE("Failed to get the environment using GetEnv()");
		return -1;
	}

	av_jni_set_java_vm(vm,NULL);

	m_vm = vm;

	version = env->GetVersion();

	classId = env->FindClass("com/example/vediodecoder/NativeMethod");

	if(classId != NULL)
	{
		classId = (jclass)env->NewGlobalRef(classId);
	}
	else
	{
		LOGE("Find Class com/example/vediodecoder/NativeMethod.java error \r\n");
	}

	jMethodID =env->GetStaticMethodID(classId ,
					"getNumBitmap" , "(III)Landroid/graphics/Bitmap;");

	if(jMethodID == NULL){
		LOGE("Find Method getNumBitmap() error \r\n");
	}

	return JNI_VERSION_1_4;
}
