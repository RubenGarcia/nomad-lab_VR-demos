Programs:
PeriodicTable 
	takes one filename as input and generates an XYZ with the atoms at the standard periodic table positions.
Simplemove
	Parameters: <server> <port> <secret>
	Usage: 
		Run the program in a shared filesystem folder.
		The program connects to a NOMADVR proxy server, then generates config and xyz files
		Other OpenVR NOMADVR instances can use these config files to enable atom Drag-and-drop functionality,
			using the touchpad in the second controller.
		The XYZ file in the disk contains the latest atom configuration.
	
Simplemove can be used as a sample program to interface OpenVR NOMAD VR with interactive molecular dynamics codes.

Compilation:
Requires: happyhttp NOMADVRLib rapidjson
Copy these folders here and run "make".

Subdirectories:
  Access to databases:
     EBI
     ChemSpider
     MaterialsProject
     Smiles: Search by smiles string in https://cactus.nci.nih.gov/translate/