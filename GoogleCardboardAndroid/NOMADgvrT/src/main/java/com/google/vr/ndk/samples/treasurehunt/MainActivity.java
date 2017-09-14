/* Copyright 2017 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.vr.ndk.samples.treasurehunt;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Vibrator;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import com.google.vr.ndk.base.AndroidCompat;
import com.google.vr.ndk.base.GvrLayout;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/** A Gvr API sample application. */
public class MainActivity extends Activity {
  private GvrLayout gvrLayout;
  private long nativeTreasureHuntRenderer;
  private GLSurfaceView surfaceView;

  // This is done on the GL thread because refreshViewerProfile isn't thread-safe.
  private final Runnable refreshViewerProfileRunnable =
      new Runnable() {
        @Override
        public void run() {
          gvrLayout.getGvrApi().refreshViewerProfile();
        }
      };

  static {
    System.loadLibrary("gvr");
    System.loadLibrary("gvr_audio");
    System.loadLibrary("treasurehunt_jni");
  }

//http://stackoverflow.com/questions/8854359/exception-open-failed-eacces-permission-denied-on-android
// Storage Permissions
private static final int REQUEST_EXTERNAL_STORAGE = 1;
private static String[] PERMISSIONS_STORAGE = {
        android.Manifest.permission.READ_EXTERNAL_STORAGE,
        android.Manifest.permission.WRITE_EXTERNAL_STORAGE
};

/**
 * Checks if the app has permission to write to device storage
 *
 * If the app does not has permission then the user will be prompted to grant permissions
 *
 * @param activity
 */
public static void verifyStoragePermissions(Activity activity) {
    // Check if we have write permission
    int permission =  	android.support.v4.app.ActivityCompat.checkSelfPermission(activity, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);

    if (permission != android.content.pm.PackageManager.PERMISSION_GRANTED) {
        // We don't have permission so prompt the user
        android.support.v4.app.ActivityCompat.requestPermissions(
                activity,
                PERMISSIONS_STORAGE,
                REQUEST_EXTERNAL_STORAGE
        );
    }
}

    @Override
    protected void onActivityResult(int requestCode, int resultCode, android.content.Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode==123 && resultCode==RESULT_OK) {
		String s=data.getDataString();
		android.util.Log.d("NOMADgvrT","OnActivityResult, s="+s);
		String uriString="";
     		if (s.startsWith("file://")) {
			uriString=s.substring(7);
		} else if (s.startsWith("content://com.asus.filemanager.OpenFileProvider/file")) {
			uriString=s.substring(52);
		} else {
			uriString=s;
		}
		android.util.Log.d("NOMADgvrT","OnActivityResult, uri="+uriString);
		nativeSetConfigFile(uriString, android.os.Environment.getExternalStorageDirectory().getPath() + "/");   
		nativeLoadConfigFile(nativeTreasureHuntRenderer);    
        }
    }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

	verifyStoragePermissions(this);

//http://stackoverflow.com/questions/36557879/how-to-use-native-android-file-open-dialog
/////////
        //setContentView(R.layout.activity_main);

 
///////////



	String externalsd=android.os.Environment.getExternalStorageDirectory().getPath() + "/";
	//intents
	android.content.Intent intent = getIntent();
	
	String s=intent.getDataString();
	String uriString=externalsd+"material.ncfg";
	if (s==null) {
		android.util.Log.d("NOMADgvrT","String intent is null");
	       android.content.Intent intentSent = new android.content.Intent()
		.setType("*/*")
		.setAction(android.content.Intent.ACTION_GET_CONTENT);

		startActivityForResult(android.content.Intent.createChooser(intentSent, "Select NOMAD VR config file (.ncfg)"), 123);		
		s=intentSent.getDataString();
	}
	else
		android.util.Log.d("NOMADgvrT","String intent is <"+ s+">");

	android.util.Log.d("NOMADgvrT","String intent finally is <"+s+">");	
	if (s!=null) {
		if (s.startsWith("content://")) {//this only works with ncfg with no associated data
			android.util.Log.d("NOMADgvrT","String intent is <"+s+">");
			try {
				java.io.InputStream input = getContentResolver().openInputStream(intent.getData());
				byte[] buffer = new byte[8 * 1024];
				java.io.FileOutputStream output = new java.io.FileOutputStream(uriString);
				try{
					int bytesRead;
					while((bytesRead = input.read(buffer)) != -1){
						output.write(buffer, 0, bytesRead);
					}
				} catch (Exception e) {
					 android.util.Log.d("NOMADgvrT","Exception writing intent to disk, "+e);
				}finally {
					try{
						output.close();
					} catch (Exception e) {
						uriString=null;
						android.util.Log.d("NOMADgvrT","Exception closing output stream, "+e);
					}
					try {
						input.close();
					} catch (java.io.IOException e) {
						uriString=null;
						android.util.Log.d("NOMADgvrT","Exception closing input stream, "+e);
					}
				}
			} catch (java.io.IOException e) {
				uriString=null;
				android.util.Log.d("NOMADgvrT","Exception saving intent to disk, "+e);
			} 
		if (uriString!=null)
			nativeSetConfigFile(uriString, externalsd);
		} else if (s.startsWith("file://")) {
			uriString=s.substring(7);
			nativeSetConfigFile(uriString, externalsd);
		} else {
			android.util.Log.d("NOMADgvrT","Unknown protocol in intent:"+ s);
		}
	} else { //use default
		nativeSetConfigFile(externalsd+"Default.ncfg", externalsd);
	}

    // Ensure fullscreen immersion.
    setImmersiveSticky();
    getWindow()
        .getDecorView()
        .setOnSystemUiVisibilityChangeListener(
            new View.OnSystemUiVisibilityChangeListener() {
              @Override
              public void onSystemUiVisibilityChange(int visibility) {
                if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
                  setImmersiveSticky();
                }
              }
            });

    // Initialize GvrLayout and the native renderer.
    gvrLayout = new GvrLayout(this);
    nativeTreasureHuntRenderer =
        nativeCreateRenderer(
            getClass().getClassLoader(),
            this.getApplicationContext(),
            gvrLayout.getGvrApi().getNativeGvrContext());

    // Add the GLSurfaceView to the GvrLayout.
    surfaceView = new GLSurfaceView(this);
    surfaceView.setEGLContextClientVersion(3);
    surfaceView.setEGLConfigChooser(8, 8, 8, 0, 0, 0);
    surfaceView.setPreserveEGLContextOnPause(true);
    surfaceView.setRenderer(
        new GLSurfaceView.Renderer() {
          @Override
          public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            nativeInitializeGl(nativeTreasureHuntRenderer);
          }

          @Override
          public void onSurfaceChanged(GL10 gl, int width, int height) {}

          @Override
          public void onDrawFrame(GL10 gl) {
            nativeDrawFrame(nativeTreasureHuntRenderer);
          }
        });
    surfaceView.setOnTouchListener(
        new View.OnTouchListener() {
          @Override
          public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
              // Give user feedback and signal a trigger event.
              ((Vibrator) getSystemService(Context.VIBRATOR_SERVICE)).vibrate(50);
              nativeOnTriggerEvent(nativeTreasureHuntRenderer);
              return true;
            }
            return false;
          }
        });
    gvrLayout.setPresentationView(surfaceView);

    // Add the GvrLayout to the View hierarchy.
    setContentView(gvrLayout);

    // Enable scan line racing.
    if (gvrLayout.setAsyncReprojectionEnabled(true)) {
      // Scanline racing decouples the app framerate from the display framerate,
      // allowing immersive interaction even at the throttled clockrates set by
      // sustained performance mode.
      AndroidCompat.setSustainedPerformanceMode(this, true);
    }

//intents
	//java.lang.String s=GvrIntent.GetData();
	//android.util.Log.d("NOMADgvrT, intent=", s);

    // Enable VR Mode.
    AndroidCompat.setVrModeEnabled(this, true);

    // Prevent screen from dimming/locking.
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
  }

  @Override
  protected void onPause() {
    super.onPause();
    nativeOnPause(nativeTreasureHuntRenderer);
    gvrLayout.onPause();
    surfaceView.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    nativeOnResume(nativeTreasureHuntRenderer);
    gvrLayout.onResume();
    surfaceView.onResume();
    surfaceView.queueEvent(refreshViewerProfileRunnable);
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    // Destruction order is important; shutting down the GvrLayout will detach
    // the GLSurfaceView and stop the GL thread, allowing safe shutdown of
    // native resources from the UI thread.
    gvrLayout.shutdown();
    nativeDestroyRenderer(nativeTreasureHuntRenderer);
  }

  @Override
  public void onWindowFocusChanged(boolean hasFocus) {
    super.onWindowFocusChanged(hasFocus);
    if (hasFocus) {
      setImmersiveSticky();
    }
  }

  @Override
  public boolean dispatchKeyEvent(KeyEvent event) {
    // Avoid accidental volume key presses while the phone is in the VR headset.
    if (event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_UP
        || event.getKeyCode() == KeyEvent.KEYCODE_VOLUME_DOWN) {
      return true;
    }
    return super.dispatchKeyEvent(event);
  }

  private void setImmersiveSticky() {
    getWindow()
        .getDecorView()
        .setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
  }

  private native long nativeCreateRenderer(
      ClassLoader appClassLoader, Context context, long nativeGvrContext);

  private native void nativeDestroyRenderer(long nativeTreasureHuntRenderer);

  private native void nativeInitializeGl(long nativeTreasureHuntRenderer);

  private native long nativeDrawFrame(long nativeTreasureHuntRenderer);

  private native void nativeOnTriggerEvent(long nativeTreasureHuntRenderer);

  private native void nativeOnPause(long nativeTreasureHuntRenderer);

  private native void nativeOnResume(long nativeTreasureHuntRenderer);

  private native void nativeSetConfigFile(String s, String e);
  private native void nativeLoadConfigFile(long nativeTreasureHuntRenderer);

  public void DisplayMessage (final String s)
  {
//http://stackoverflow.com/questions/3875184/cant-create-handler-inside-thread-that-has-not-called-looper-prepare
//rgh: this hangs, need to investigate
	new Thread()
	{
	    public void run()
	    {
		MainActivity.this.runOnUiThread(new Runnable()
		{
		    public void run()
		    {
			android.widget.Toast toast = android.widget.Toast.makeText(MainActivity.this.getApplicationContext(), s,
				android.widget.Toast.LENGTH_LONG);		        
		    }
		});
	    }
	}.start();

  }
}
