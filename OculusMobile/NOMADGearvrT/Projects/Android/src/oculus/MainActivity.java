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

    public static native long nativeSetAppInterface( VrActivity act, String fromPackageNameString, String commandString, String uriString );

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

		Intent intent = getIntent();
		String commandString = VrActivity.getCommandStringFromIntent( intent );
		String fromPackageNameString = VrActivity.getPackageStringFromIntent( intent );
		String uriString = VrActivity.getUriStringFromIntent( intent );

		//rgh: if intent, save to tmp file and pass it over
		if (uriString.startsWith("content://")) {
			//http://stackoverflow.com/questions/14364091/retrieve-file-path-from-caught-downloadmanager-intent
			//http://stackoverflow.com/questions/1477269/write-a-binary-downloaded-file-to-disk-in-java
			//http://stackoverflow.com/questions/4864875/folder-for-temporary-files-creation-in-android-why-does-data-local-tmp-doesnt
			uriString="file:///data/local/tmp/material.ncfg";
			try {
				java.io.InputStream input = getContentResolver().openInputStream(intent.getData());
				byte[] buffer = new byte[8 * 1024];
				java.io.FileOutputStream output = new java.io.FileOutputStream("/data/local/tmp/material.ncfg");
				try{
					int bytesRead;
					while((bytesRead = input.read()) != -1){
						output.write(buffer, 0, bytesRead);
					}
				} finally {
					try{
						output.close();
					} catch (Exception e) {
						uriString="error";
					}
					try {
						input.close();
					} catch (java.io.IOException e) {
						uriString="error";
					}
				}
			} catch (java.io.IOException e) {
				uriString="error";
			} 
		}
		
		setAppPtr( nativeSetAppInterface( this, fromPackageNameString, commandString, uriString ) );
    }   
}
