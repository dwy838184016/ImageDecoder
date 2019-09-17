package com.example.vediodecoder;

import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.view.Surface;

public class NativeMethod {

	static{
		System.loadLibrary("VedioDecoder");
	}
	
	public static native long createInstance(String filePath, int playType, long frameDurationValue);
	
	public static native boolean initMediaCodec(long instance, int width, int height);
	
	public static native void startPlay(long instance, int width, int height, Surface surface);
	
	public static native void stopPlay(long instance);
	
	public static Bitmap getNumBitmap(int width, int height, int num){
		Bitmap bmp = Bitmap.createBitmap(width, height, Config.ARGB_8888);
		bmp.eraseColor(Color.BLACK);
		Paint paint = new Paint();
		Canvas canvas = new Canvas(bmp);
		paint.setTextSize(500);
		paint.setColor(Color.RED);
		paint.setFlags(Paint.ANTI_ALIAS_FLAG);
		paint.setStyle(Paint.Style.FILL_AND_STROKE); //用于设置字体填充的类型
        canvas.drawText(new Integer(num).toString(),width/2-200,500,paint);
		return bmp;
	}
	
}
