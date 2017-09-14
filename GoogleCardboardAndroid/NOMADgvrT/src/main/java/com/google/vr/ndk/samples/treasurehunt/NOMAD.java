//http://stackoverflow.com/questions/4875114/android-save-image-from-url-onto-sd-card

class NOMAD {
public static boolean i=false;
public static void Init () 
//we are going to block anyway until we have the datasets
{
if (android.os.Build.VERSION.SDK_INT > 9) {
    android.os.StrictMode.ThreadPolicy policy = new android.os.StrictMode.ThreadPolicy.Builder().permitAll().build();
    android.os.StrictMode.setThreadPolicy(policy);
}
i=true;
}

public static void GetUrl (String url, String path) 
{
if (!i)
	Init();
 
android.util.Log.d("NOMAD", "Java NOMAD.GetUrlThread(\""+url+"\",\""+path+"\") called");

try {
  java.net.URL u = new java.net.URL(url);
  java.net.HttpURLConnection urlConnection = (java.net.HttpURLConnection) u.openConnection();
  urlConnection.setRequestMethod("GET");
  urlConnection.setDoOutput(true);   
  java.io.File file = new java.io.File(path);
  java.io.FileOutputStream fileOutput = new java.io.FileOutputStream(file);
  int status = urlConnection.getResponseCode();

  if (status != java.net.HttpURLConnection.HTTP_OK) {
	return; //http error or redirect, we probably want to exit here as NOMAD encyclopedia should not do that
  }
  java.io.InputStream inputStream = urlConnection.getInputStream();
  int totalSize = urlConnection.getContentLength();
  int downloadedSize = 0;   
  byte[] buffer = new byte[1024];
  int bufferLength = 0;
  while ( (bufferLength = inputStream.read(buffer)) > 0 ) 
  {                 
    fileOutput.write(buffer, 0, bufferLength);                  
    downloadedSize += bufferLength;                 
  }             
  fileOutput.close();

} //try
catch (Exception e) {
 e.printStackTrace();
}

} //GetUrl

} //class NOMAD
