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
	fprintf (stderr, "Usage: %s <Materials Project ID>\n"
		"Multiple IDs are accepted\n", argv[0]);
	return 1;
}

for (int c=1;c<argc;c++) {
	sprintf (command, "MP.sh \"%s\" -1 0", argv[c]);
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

	char *PDBid;
	float resolution=-1;

	if (json.HasMember("response")) {
		rapidjson::Value &ref=json["response"];
		if (!ref.IsArray()) {
			fprintf (stderr, "json unexpected format\n");
			return -4;
		}
		for (rapidjson::SizeType j = 0; j < ref.Size(); j++) {
			rapidjson::Value &v=ref[j];
			if (!v.HasMember("cif")) {
				continue;
			}

			FILE * cif=fopen ("file.cif", "w");
			fprintf (cif, v["cif"].GetString());
			fclose (cif);
			sprintf (command, "obabel -i cif file.cif -o xyz -O %d.xyz", c);
			system (command);

			//get id, properties

		}
	} else {
		printf ("Json format error");
		return -5;
	}
	sprintf (command, "CreateInfoPNG.sh "
		"\"Id: mp-%s\n\" 768 %d.png", 
		argv[c], c);
	system (command);

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
}

FILE *f=fopen ("nomad.bat", "w");
fprintf (f, "NOMADViveT.exe ");
for (int c=1; c<argc; c++)
	fprintf (f, "%d.ncfg ", c);
fprintf (f, "\n");
fclose (f);
return 0;
}
