/*
 # Copyright 2016-2018 Ruben Jesus Garcia-Hernandez
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
*/

/* Requires: obabel in PATH
 */

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"

//#define SMILES to use smiles API search
//undefined uses general search
//#define SMILES

int main (int argc, char ** argv)
{
char command [200];

if (argc < 2) {
#ifdef SMILES
	fprintf (stderr, "Usage: %s <smiles string>\n"
		"Multiple smiles strings are accepted", argv[0]);
#else
	fprintf (stderr, "Usage: %s <compound>\n"
		"Multiple compounds are accepted\n", argv[0]);
#endif
	return 1;
}

for (int c=1;c<argc;c++) {
#ifdef SMILES
	sprintf (command, "ChemSpiderSmiles.sh \"%s\"", argv[c]);
#else
	sprintf (command, "ChemSpider.sh \"%s\"", argv[c]);
#endif
	int ret=system (command);
	if (ret!=0) {
		fprintf (stderr, "Error getting compound (not found in database?)\n");
		return 4;
	}

	FILE *pfile=fopen ("data.json", "r");
	char readBuffer[65536];
	rapidjson::FileReadStream is(pfile, readBuffer, sizeof(readBuffer));
	rapidjson::Document json;
	json.ParseStream(is);
	fclose(pfile);

	if (json.HasParseError()) {
		fprintf (stderr, "json parsing error\n");
		return 2;
	}

	const char *formula, *mol3D, *smiles, *CommonName;
	float molecularWeight;

	if (!json.HasMember("formula")) {
		fprintf (stderr, "Formula missing from json\n");
		formula = "Not given";
	} else {
		formula = json["formula"].GetString();
	}

	if (!json.HasMember("commonName")) {
		fprintf (stderr, "Common Name missing from json\n");
		CommonName="Not given";
	} else {
		CommonName=json["commonName"].GetString();
	}

	if (!json.HasMember("molecularWeight")) {
		fprintf (stderr, "molecularWeight missing from json\n");
		molecularWeight=-1;
	} else {
		molecularWeight=json["molecularWeight"].GetFloat();
	}

	if (!json.HasMember("smiles")) {
		fprintf (stderr, "smiles missing from json\n");
		smiles="Not given";
	} else {
		smiles=json["smiles"].GetString();
	}

	if (!json.HasMember("mol3D")) {
		fprintf (stderr, "mol3D missing from json\n");
		return 3;
	}

	mol3D=json["mol3D"].GetString();
	pfile=fopen ("data.mol", "w");
	//rgh FIXME, this may be unsafe if ChemSpider server is compromised.
	fprintf (pfile, "%s", mol3D);
	fclose (pfile);

	sprintf (command, "obabel -i mol data.mol -O %d.xyz", c);
	system (command);

	sprintf (command, "CreateInfoPNG.sh "
		"\"%s\nformula=%s\nSmiles=\n%s\nCommon Name=\n%s\nMolecular Weight: %.2f\" 768 %d.png", 
		argv[c], formula, smiles, CommonName, molecularWeight, c);
	system (command);

	char name[100];
	sprintf (name, "%d.ncfg", c);	
	pfile=fopen (name, "w");
	fprintf (pfile, "menubutton Infobox\n"
		"sidebuttontimestep 1\n"
		"showcontrollers\n"
		"disablereloadreset\n"
		"xyzfile \"%d.xyz\"\n"
		"info -3 -3 -3 2 -1 \"%d.png\"\n"
		, c, c);
	fclose (pfile);
}

FILE *f=fopen ("nomad.bat", "w");
fprintf (f, "NOMADViveT.exe ");
for (int c=1; c<argc; c++)
	fprintf (f, "%d.ncfg ", c);
fprintf (f, "\n");
fclose (f);
return 0;
}
