package com.example.vediodecoder;

import java.io.File;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.MediaCodec;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioGroup;
import android.widget.RadioGroup.OnCheckedChangeListener;
import android.widget.Toast;

public class MainActivity extends Activity {

	private final String TAG = MainActivity.class.getCanonicalName();
	
	private final int REQUEST_CHOOSEFILE = 100;
	
	private Button mStartBtn = null;
	
	private Button mChooseFileBtn = null;
	
	private EditText mFilePathText = null;
	
	private EditText mFrameDurationText = null;
	
	private MySurfaceView mSurfaceView = null;
	
	private RadioGroup mRadioGroup = null;
	
	private String chooseFilePath = null;
	
	private String mFrameDurationValue = null;
	
	private long mInstance = 0l;
	
	private Context mContext = null;
	
	/** 1----单个播放；2----循环播放 */
	private int mPlayType = 1;
	
	private long m_preClickTime = 0;
	
	private long m_preTouchTime = 0;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		mContext = this;
		initView();
	}

	private void initView(){
		mSurfaceView = (MySurfaceView)findViewById(R.id.my_surfaceview);
		mStartBtn = (Button)findViewById(R.id.start_btn);
		mChooseFileBtn = (Button)findViewById(R.id.choose_file_btn);
		mFilePathText = (EditText)findViewById(R.id.file_path_text);
		mRadioGroup = (RadioGroup)findViewById(R.id.play_type_group);
		mFrameDurationText = (EditText)findViewById(R.id.frame_duration_value_edit);
		
		mStartBtn.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				if(mFilePathText.getText() != null){
					chooseFilePath = mFilePathText.getText().toString();
				}
				
				if(mFrameDurationText.getText() != null){
					mFrameDurationValue = mFrameDurationText.getText().toString();
				}
				
				if(chooseFilePath == null || chooseFilePath.isEmpty() || !chooseFilePath.endsWith(".h264")){
					Toast.makeText(mContext, "请选择H264文件", Toast.LENGTH_SHORT).show();
					return ;
				}
				
				File file = new File(chooseFilePath);
				if(!file.exists()){
					Toast.makeText(mContext, "文件" + chooseFilePath + "不存在", Toast.LENGTH_SHORT).show();
					return ;
				}
				
				int value = 0;
				if(mFrameDurationValue == null || mFrameDurationValue.isEmpty()){
					Toast.makeText(mContext, "请输入帧间隔时间", Toast.LENGTH_SHORT).show();
					return ;
				} else {
					try {
						value = Integer.valueOf(mFrameDurationValue);
					} catch (Exception e) {
						// TODO Auto-generated catch block
						value = 0;
					}
				}
				

				mSurfaceView.setVisibility(View.VISIBLE);
				mInstance = NativeMethod.createInstance(chooseFilePath, mPlayType, value);
				Runnable runnable = new Runnable() {
					
					@Override
					public void run() {
						// TODO Auto-generated method stub
						try {
							Thread.sleep(1000);
						} catch (InterruptedException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
						int width = mSurfaceView.getWidth();
						int height = mSurfaceView.getHeight();
						
						width = width > 1920 ? 1920 : width;
						height = height > 1080 ? 1080 : height;
						width = width%2 == 0 ? width : width - 1;
						height = height%2 == 0 ? height : height - 1;
						
						boolean res = NativeMethod.initMediaCodec(mInstance, width, height);
						if(res){
							NativeMethod.startPlay(mInstance, width, height, mSurfaceView.getHolder().getSurface());
							

							mSurfaceView.post(new Runnable() {
								
								@Override
								public void run() {
									// TODO Auto-generated method stub
									mSurfaceView.setVisibility(View.GONE);
								}
							});
						}
					}
				};
				
				new Thread(runnable).start();
//				mSurfaceView.postDelayed(runnable, 2000);
				
			}
		});
		
		
		mChooseFileBtn.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
				intent.setType("*/*");//无类型限制
				intent.addCategory(Intent.CATEGORY_OPENABLE);
				startActivityForResult(intent, REQUEST_CHOOSEFILE);
				
				/*try {
					Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
					intent.setAction("com.hisense.fileexplorer");
					intent.setData(Uri.fromParts("package", getPackageName(), null)); 
					startActivityForResult(intent, REQUEST_CHOOSEFILE);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}*/
			}
		});
		
		mRadioGroup.setOnCheckedChangeListener(new OnCheckedChangeListener() {
			
			@Override
			public void onCheckedChanged(RadioGroup group, int checkedId) {
				// TODO Auto-generated method stub
				switch (checkedId) {
					case R.id.sigle_play_btn:
						mPlayType = 1;
						break;
						
					case R.id.recycle_play_btn:
						mPlayType = 2;
						break;
	
					default:
						break;
				}
			}
		});
		
		mSurfaceView.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				long curClickTime = System.currentTimeMillis();
				if(curClickTime - m_preClickTime <= 300){
					NativeMethod.stopPlay(mInstance);
					mSurfaceView.setVisibility(View.GONE);
				}
				m_preClickTime = curClickTime;
			}
		});
			
	}
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {// 选择文件返回
		super.onActivityResult(requestCode, resultCode, data);
		if (resultCode == RESULT_OK) {
			switch (requestCode) {
			case REQUEST_CHOOSEFILE:
				try {
					Uri uri = data.getData();
					chooseFilePath = FileChooseUtil.getInstance(this)
							.getChooseFileResultPath(uri);
					Log.d(TAG, "选择文件返回：" + chooseFilePath);
					if(chooseFilePath != null){
						mFilePathText.setText(chooseFilePath);
					}
				} catch (Exception e) {
					// TODO Auto-generated catch block
					Log.e(TAG, e.toString());
				}
				
				break;
			}
		}
	}
	
	
	
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		// TODO Auto-generated method stub
		
		if(event.getActionMasked() == MotionEvent.ACTION_DOWN){
			if(mSurfaceView.getVisibility() == View.GONE){
				long curClickTime = System.currentTimeMillis();
				if(curClickTime - m_preTouchTime <= 500){
					finish();
		            System.exit(0);
				} else {
					Toast.makeText(mContext, "再点击一次退出程序", Toast.LENGTH_SHORT).show();
				}
				m_preTouchTime = curClickTime;
			}
		}
		
		return super.onTouchEvent(event);
	}

	public long getXH264Instance(){
		return mInstance;
	}
	
}
