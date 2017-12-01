package com.lrz.NOMADGearVRChooser;

import android.app.Activity;
import android.util.Log;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.widget.Button;
import android.view.View;
import android.view.View.OnClickListener;

public class MyAndroidAppActivity extends Activity {
String TAG="NOMADGearVRChooser";

//https://stackoverflow.com/questions/3872063/launch-an-application-from-another-application-on-android
public void startNewActivity(android.content.Context context, String packageName) {
    Intent intent = context.getPackageManager().getLaunchIntentForPackage(packageName);
    if (intent != null) {
        // We found the activity now start the activity
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
}

void nativeLoadConfigFile()
{
	startNewActivity(this.getApplicationContext(), "");
}

//check this once nomadGearVR is in the market
//https://stackoverflow.com/questions/3872063/launch-an-application-from-another-application-on-android
void nativeSetConfigFile(String s, String externalsd)
{
	java.lang.String path=android.os.Environment.getExternalStorageDirectory()+"/NOMAD";
	java.io.File dir = new java.io.File(path);
	dir.mkdir();
	
	try {
	java.io.PrintWriter pw= new java.io.PrintWriter (path+"/NOMADGearVR.cfg");
	pw.println(s);
	pw.close();
	//this.finishAffinity();
	android.content.Intent launchIntent = getPackageManager().getLaunchIntentForPackage("com.lrz.NOMADGearvrT");
	if (launchIntent != null) { 
    startActivity(launchIntent);//null pointer check in case package name was not found
	} else {
		android.util.Log.d(TAG, "com.lrz.NOMADGearvrT is not installed");
	}
	} catch (java.io.FileNotFoundException f) {
		android.util.Log.d(TAG, "File not found exception "+f);
		android.util.Log.d(TAG, "Could not save information");
	}


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
		android.util.Log.d(TAG,"OnActivityResult, s="+s);
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
			android.util.Log.d(TAG,"URISyntaxException, e="+e);
			uriString=null;
			}
		}
		android.util.Log.d("NOMADGearVRChooser","OnActivityResult, uri="+uriString);
		nativeSetConfigFile(uriString, android.os.Environment.getExternalStorageDirectory().getPath() + "/");   
		nativeLoadConfigFile();    
        }
    }
	
	//http://stackoverflow.com/questions/13136539/caused-by-android-os-networkonmainthreadexception
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		verifyStoragePermissions(this);
	

		String externalsd=android.os.Environment.getExternalStorageDirectory().getPath() + "/";
	//intents
	android.content.Intent intent = getIntent();
	
	String s=intent.getDataString();
	String uriString=externalsd+"material.ncfg";
	if (s==null) {
		android.util.Log.d(TAG,"String intent is null");
	       android.content.Intent intentSent = new android.content.Intent()
		.setType("*/*")
		.setAction(android.content.Intent.ACTION_GET_CONTENT);

		startActivityForResult(android.content.Intent.createChooser(intentSent, "Select NOMAD VR config file (.ncfg)"), 123);		
		s=intentSent.getDataString();
	}
	else
		android.util.Log.d(TAG,"String intent is <"+ s+">");

	android.util.Log.d(TAG,"String intent finally is <"+s+">");	
	if (s!=null) {
		if (s.startsWith("content://")) {//this only works with ncfg with no associated data
			android.util.Log.d(TAG,"String intent is <"+s+">");
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
					 android.util.Log.d(TAG,"Exception writing intent to disk, "+e);
				}finally {
					try{
						output.close();
					} catch (Exception e) {
						uriString=null;
						android.util.Log.d(TAG,"Exception closing output stream, "+e);
					}
					try {
						input.close();
					} catch (java.io.IOException e) {
						uriString=null;
						android.util.Log.d(TAG,"Exception closing input stream, "+e);
					}
				}
			} catch (java.io.IOException e) {
				uriString=null;
				android.util.Log.d(TAG,"Exception saving intent to disk, "+e);
			} 
		if (uriString!=null)
			nativeSetConfigFile(uriString, externalsd);
		} else if (s.startsWith("file://")) {
			uriString=s.substring(7);
			nativeSetConfigFile(uriString, externalsd);
		} else {
			android.util.Log.d(TAG,"Unknown protocol in intent:"+ s);
		}
	} else { //use default
		nativeSetConfigFile(externalsd+"Default.ncfg", externalsd);
	}

	}

}