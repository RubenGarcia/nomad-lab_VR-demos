/************************************************************************************

Filename    :   MainActivity.java
Content     :   
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.


*************************************************************************************/
package oculus;

import android.os.Bundle;
import android.util.Log;
import android.content.Intent;
import com.oculus.vrappframework.VrActivity;

public class MainActivity extends VrActivity {
	public static final String TAG = "NOMADGearvrT";

	/** Load jni .so on initialization */
	static {
		Log.d(TAG, "LoadLibrary");
		System.loadLibrary("ovrapp");
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
public static void verifyStoragePermissions(android.app.Activity activity) {
    // Check if we have write permission
 /*   int permission =  	android.support.v4.app.ActivityCompat.checkSelfPermission(activity, android.Manifest.permission.WRITE_EXTERNAL_STORAGE);

    if (permission != android.content.pm.PackageManager.PERMISSION_GRANTED) {
        // We don't have permission so prompt the user
        android.support.v4.app.ActivityCompat.requestPermissions(
                activity,
                PERMISSIONS_STORAGE,
                REQUEST_EXTERNAL_STORAGE
        );
    }*/
}
	
    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );
/*
    @Override
    protected void onActivityResult(int requestCode, int resultCode, android.content.Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if(requestCode==123 && resultCode==RESULT_OK) {
		String s=data.getDataString();
		android.util.Log.d("NOMADGearvrT","OnActivityResult, s="+s);
		String uriString="";
     		if (s.startsWith("file://")) {
			uriString=s.substring(7);
		} else if (s.startsWith("content://com.asus.filemanager.OpenFileProvider/file")) {
			uriString=s.substring(52);
		} else {
			android.net.Uri u=android.net.Uri.parse(s);
			try {
			uriString=Filepath.getFilePath (this.getApplicationContext(), u);
			} catch (java.net.URISyntaxException e) {
			android.util.Log.d("NOMADgvrT","URISyntaxException, e="+e);
			uriString=null;
			}
		}
		android.util.Log.d("NOMADgvrT","OnActivityResult, uri="+uriString);
		nativeSetConfigFile(uriString, android.os.Environment.getExternalStorageDirectory().getPath() + "/");   
		nativeLoadConfigFile(nativeTreasureHuntRenderer);    
        }
    }	
	*/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
		
        super.onCreate(savedInstanceState);
		
		verifyStoragePermissions(this);
		
		Intent intent = getIntent();
		String commandString = VrActivity.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrActivity.getPackageStringFromIntent( intent );
		String uriString = VrActivity.getUriStringFromIntent( intent );		
		android.net.Uri u=android.net.Uri.parse(uriString);
		try {
			uriString=Filepath.getFilePath (this.getApplicationContext(), u);
		} catch (java.net.URISyntaxException e) {
		android.util.Log.d("NOMADgvrT","URISyntaxException, e="+e);
		uriString=null;
		}

		setAppPtr( nativeSetAppInterface( this, fromPackageNameString, commandString, uriString ) );
    }   
}
