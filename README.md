Purpose of this code:
	The NOMADVR application can be used to visualize in virtual reality
	different materials science datasets provided by NOMAD or final users.
	Multiple platforms are supported by this code.
	
Context:
	NOMADVR is provided in the context of the NOMAD Center of Excellence.
	Datasets from the Encyclopedia and Archive are supported by NOMADVR.

Requirements:
	-Supported virtual reality hardware: HTC Vive or Oculus Rift, GearVR,
		Google Cardboard for Android, or the LRZ CAVE environment.

Usage Instructions:
	See https://www.nomad-coe.eu/the-project/graphics/VR-prototype
		
Subdirectories:
	OpenVR: Demos for HTC Vive.
	OculusMobile: Demos for GearVR.
	GoogleCardboardAndroid: Demos for Google Cardboard (android)
	CAVE: Demos for LRZ CAVE-like environment (linux)

	NOMADGearVRChooser: Android app to select the dataset for GearVR, which
		calls the OculusMobile app as a subroutine.
	
	NOMADVRLib: Shared code between HTC Vive, GearVR and Google Cardboard demos related to NOMAD.
	libs: Other (external) supporting libs shared by HTC Vive, GearVR, Cardboard and CAVE demos
		happyhttp (Zlib license)
		LodePNG (Zlib license)
		rapidjson (MIT license)
		rply (BSD 3-clause license)
	
	webserver: 
		htdocs: Web pages containing the VR software and documentation.
		cgi-bin: cgi scripts to create suitable config files from a material number.
	
	RemoteVisualization: NOMAD2xyz app 
		Transforms a json from encyclopedia or archive into an XYZ file	
		Useful for the remote visualization infrastructure
	
More platforms will be added in the future.

The code is distributed under the Apache 2.0 License (See LICENSE file).

