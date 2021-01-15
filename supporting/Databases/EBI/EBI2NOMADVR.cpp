/*
 # Copyright 2018-2019 Ruben Jesus Garcia-Hernandez
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


#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"

//#define SMILES to use smiles API search
//undefined uses general search
//#define ACCESSION

using namespace std;

//https://stackoverflow.com/questions/154536/encode-decode-urls-in-c
string url_encode(const string &value) {
    ostringstream escaped;
    escaped.fill('0');
    escaped << hex;

    for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
            continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char) c);
        escaped << nouppercase;
    }

    return escaped.str();
}


void lowercase (char *c) {
	while (*c!='\0') {
		*c=tolower(*c);
		c++;
	}
}

int main (int argc, char ** argv)
{
char command [200];

if (argc < 2) {
#ifdef ACCESSION
	fprintf (stderr, "Usage: %s <accesion>\n"
		"Multiple accesions are accepted\n", argv[0]);
#else
	fprintf (stderr, "Usage: %s <protein>\n"
		"Multiple proteins are accepted\n", argv[0]);
#endif
	return 1;
}

for (int c=1;c<argc;c++) {
//change this later to for loop with blocks of 20
#ifdef ACCESSION
	sprintf (command, "EBIACC.sh \"%s\" -1 0", argv[c]);
#else
	sprintf (command, "EBISearch.sh \"%s\" -1 0", url_encode(argv[c]).c_str());
#endif
	system (command);

	FILE *pfile=fopen ("data.json", "r");
	if (pfile==0) {
		fprintf (stderr, "Problem obtaining json, exiting\n");
		return -4;
	}
	char readBuffer[65536];
	rapidjson::FileReadStream is(pfile, readBuffer, sizeof(readBuffer));
	rapidjson::Document json;
	json.ParseStream(is);
	fclose(pfile);

	if (json.HasParseError()) {
		fprintf (stderr, "json parsing error\n");
		return 2;
	}

	char *PDBid=0;
	float resolution=-1;

	if (!json.IsArray()) {
		fprintf (stderr, "json unexpected format\n");
		return -3;
	}

	for (rapidjson::SizeType i = 0; i < json.Size(); i++) {
//		if (PDBid)
//			break;
		if (json[i].HasMember("dbReferences")) {
			printf ("dbReferences\n");
			rapidjson::Value &ref=json[i]["dbReferences"];
			if (!ref.IsArray()) {
				fprintf (stderr, "json unexpected format\n");
				return -4;
			}
			for (rapidjson::SizeType j = 0; j < ref.Size(); j++) {
				rapidjson::Value &v=ref[j];
				if (!v.HasMember("type")) {
					continue;
				}
				if (strcmp (v["type"].GetString(), "PDB")) {
					continue;
				}
				//get id, properties
#if 0 //get all and concatenate
				PDBid=strdup(v["id"].GetString());
				lowercase (PDBid);

				sprintf (command, "wget http://www.ebi.ac.uk/pdbe/entry-files/download/pdb%s.ent -O data.ent",
					 PDBid);
				system (command);

				sprintf (command, "obabel -i pdb data.ent -O temp.xyz");
				system (command);

				sprintf (command, "cat temp.xyz >> %d.xyz", c);
				system (command);
				free (PDBid);
				PDBid=0;
#endif
//get the one with the smallest resolution and download after exiting loop
				float nres;
				const rapidjson::Value & prop = v["properties"].GetObject();
				//format ""<float> A"
				if (!prop.HasMember("resolution")) //NMR method, discard
					continue;
				int read;
				read=sscanf (prop["resolution"].GetString(), "%f", &nres);
				if (read==1 && (resolution <0 || nres < resolution)) {
					resolution=nres;
					free(PDBid);
					PDBid=strdup (v["id"].GetString());
				}
			}
			
		} else {
			continue;
		}

	}

	//download pbd
	lowercase (PDBid);
	sprintf (command, "wget http://www.ebi.ac.uk/pdbe/entry-files/download/pdb%s.ent -O data.ent", PDBid);
	system (command);

	sprintf (command, "obabel -i pdb data.ent -O %d.xyz", c);
	system (command);

	char *tmp=strdup (argv[c]);
	int cur=0;
	char *current=tmp;
	while (*current!='\0') {
		if (cur>30 && *current==' ') {
			cur=0;
			*current='\n';
		}
		cur++;
		current++;
	}
	sprintf (command, "CreateInfoPNG.sh "
		"\"%s\nType: PDB, id: %s\nResolution:%f A\" 768 %d.png", 
		tmp, PDBid, resolution, c);
	system (command);
	free (tmp);

	char name[100];
	sprintf (name, "%d.ncfg", c);	
	pfile=fopen (name, "w");
	fprintf (pfile, 
		"clippingplanes 0.1 300\n"
		"menubutton Infobox\n"
		"sidebuttontimestep 1\n"
		"showcontrollers\n"
		"disablereloadreset\n"
		"xyzfile \"%d.xyz\"\n"
		"info 150 150 150 50 -1 \"%d.png\"\n"
		, c, c);
	fclose (pfile);
	free (PDBid);
	PDBid=0;
}

FILE *f=fopen ("nomad.bat", "w");
fprintf (f, "NOMADViveT.exe ");
for (int c=1; c<argc; c++)
	fprintf (f, "%d.ncfg ", c);
fprintf (f, "\n");
fclose (f);
return 0;
}
