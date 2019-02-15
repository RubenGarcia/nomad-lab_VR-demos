This directory contains the demo for Google Cardboard using Google Cardboard SDK
for IOS.

Follow https://developers.google.com/vr/develop/ios/get-started to install
the SDK.

Copy NOMADVRIOS in gvr-ios-sdk/Samples

Copy ../NOMADVRLib to gvr-ios-sdk/Samples/NOMADVRIOS

Copy ../GoogleCardboard to gvr-ios-sdk/Samples/NOMADVRIOS

Copy from ../libs/ the directories rply, happyhttp, rapidjson
	to gvr-ios-sdk/Samples/NOMADVRIOS

Enter gvr-ios-sdk/Samples/TreasureHuntNDK and run

	pod update

Enter gvr-ios-sdk/Samples/NOMADVRIOS and run 

	pod update

If you get this error

	[!] Failed to connect to GitHub to update the CocoaPods/Specs specs repo - Please check if you are offline, or that GitHub is down

follow this:

	https://stackoverflow.com/questions/38993527/cocoapods-failed-to-connect-to-github-to-update-the-cocoapods-specs-specs-repo

Open Xcode and select NOMADVR.xcworkspace

Setup Developer Account / Profiles / Code signing
(You may need to change the Bundle Identifier)

Compile and run

