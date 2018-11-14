This directory contains the support files for the CGI backend to obtain 
NOMAD VR files from the Encyclopedia and Archive.

Directories:
	htdocs: Example files related to NOMAD VR
	cgi-bin: CGI-backend

Files:
	Readme.md: This document
	Readme-containerization.md: Quick info on how to set up servives in container.
	pass: Information on the NOMAD user under which the data is accessed.
	
	cgi-bin/NOMAD/material: 
	
	Entry point for NOMAD Encyclopedia.
	See also the VR tutorial at
	https://www.nomad-coe.eu/the-project/graphics/vrtutorial6
	In the Encyclopedia webpage for a given material:
		https://encyclopedia.nomad-coe.eu/gui/#/material/<mat id>
	the link "Virtual Reality files" -> "Get VR file" points to
		http://nomad.srv.lrz.de/cgi-bin/NOMAD/material?<mat_id>
	This entry point creates a zip file with a the crystal structure of mat_id.
	
	cgi-bin/NOMAD/materialAnalytics:
	
	Entry point for NOMAD Archive / Analytics.
	See also the VR tutorial at
	https://www.nomad-coe.eu/the-project/graphics/vrtutorial5
	In the analytics toolkit tutorial webpage, the tutorial
		"Querying and visualizing the content of the NOMAD Archive" at
	https://analytics-toolkit.nomad-coe.eu/notebook-edit/data/shared/tutorialsNew/nomad-query/nomad-query.bkr
	The results of the queries have the form:
	nmd://<string>
	with the link "Virtual Reality" pointing to
	http://nomad.srv.lrz.de/cgi-bin/NOMAD/materialAnalytics?<string>
	This entry point creates a zip file with the structure or molecular dynamics
	for <string>.
	
Prerequisites:
	-Ensure that the scripts in cgi-bin/NOMAD have unix-like newlines 
		(use dos2unix or similar)
	-Ensure that the pass file has a valid username/password and is located
		at the same directory that cgi-bin is.
	-Install the following software:
		-apache 2
		-bash
		-grep
		-zip
		-wget
		-curl
