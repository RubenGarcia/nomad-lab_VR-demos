Subdirectories:
	OpenVR: Demos for HTC Vive.
	OculusMobile: Demos for GearVR.
	GoogleCardboardAndroid: Demos for Google Cardboard (android)
	CAVE: Demos for LRZ CAVE-like environment (linux)

	NOMADGearVRChooser: Android app to select the dataset for GearVR, which
		calls the OculusMobile app as a subroutine.
	
	NOMADVRLib: Shared code between HTC Vive, GearVR and Google Cardboard demos related to NOMAD.
	libs: Other (external) supporting libs shared by HTC Vive, GearVR, Cardboard and CAVE demos
	
	webserver: 
		htdocs: Web pages containing the VR software and documentation.
		cgi-bin: cgi scripts to create suitable config files from a material number.
	
	RemoteVisualization: NOMAD2xyz app 
		Transforms a json from encyclopedia or archive into an XYZ file	
		Useful for the remote visualization infrastructure
	
More platforms will be added in the future.

The code is distributed under the Apache 2.0 License (See LICENSE file).

