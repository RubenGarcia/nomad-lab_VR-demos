# This demo using the legacy Google VR SDK is no longer maintained

# About legacy Google VR SDK 

The legacy Google VR SDK, was last updated in 2019 and is no longer receiving updates.

# Old documentation

This repository contains the demos for Google cardboard using GVR Android SDK v1.20.0
	https://github.com/googlevr/gvr-android-sdk

Clone that repository and add the content of this one inside the Samples directory.
Copy the content of the ../libs directory (happyhttp, rapidjson, rply, LodePNG) and NOMADVRLib and GoogleCardboard into 
NOMADgvrT/src/main/jni

Enable support for ndk (tested: android-ndk-r10d) in settings.gradle:

	include ':samples:NOMADgvrT'


List of directories

NOMADgvrT: 

	This program can load prepared molecular dynamics simulations, and is 
	compatible with the TimestepData OpenVR demo.
