#include <stdio.h>
#include <stdlib.h>

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"

#include "NOMADVRLib/atoms.hpp"

#include "Archive.h"

int ParseArchiveResults(int A, int B, int cresult, const char *menubutton)
{
char command[1024];
char readBuffer[65536];
FILE *pfile;
rapidjson::Document json;

pfile=fopen ("archive.json", "r");
rapidjson::FileReadStream ais(pfile, readBuffer, sizeof(readBuffer));
json.ParseStream(ais);
fclose(pfile);
if (json.HasParseError())
	return -9;
if (!json.HasMember("status") || !json["status"].IsString())
	return -9;
const char *status;
status=json["status"].GetString();
if (0!=strcmp (status, "success"))
	return -9;
if (!json.HasMember("result"))
	return -9;
const rapidjson::GenericValue<rapidjson::UTF8<> > &result=json["result"];
if (!result.HasMember("hits"))
	return -9;
const rapidjson::GenericValue<rapidjson::UTF8<> > &hits=result["hits"];
int totalHits;
if (!hits.IsObject())
	return -9;
rapidjson::Value::ConstMemberIterator itr = hits.FindMember("total");
if (itr == hits.MemberEnd())
	return -9;
if (!itr->value.IsInt())
	return -9;
totalHits=itr->value.GetInt();
itr=hits.FindMember("hits");
if (itr == hits.MemberEnd())
	return -9;

const rapidjson::GenericValue<rapidjson::UTF8<> > & array=itr->value;
for (int i=0;i<totalHits;i++) {
	const char * nmdurl=array[i].GetObject()["_id"].GetString();
	sprintf (command, "wget http://nomad.srv.lrz.de/cgi-bin/NOMAD/materialAnalytics?%s -O A.zip", nmdurl+6); //skip nmd://
	system (command);
	sprintf (command, "unzip -o A.zip");
	system (command);
	char name[20];
	sprintf (name, "%d.ncfg", cresult++);
	if (0!=CreateArchiveNCFG (name, nmdurl+36, menubutton))
		return -10; //get calculation id
}
return cresult;
}

int CreateArchiveNCFG (const char *name, const char *nmdid, const char *menubutton)
{
FILE *f=fopen (name, "w");
if (f==nullptr)
	return -1;

fprintf (f, "analyticsjson %s/material.json\n", nmdid);
fprintf (f, "atomscaling 0.5\n");
fprintf (f, "displaybonds\n");
fprintf (f, "displayunitcell\n");
fprintf (f, "showcontrollers\n");
fprintf (f, "menubutton %s\n", menubutton);
fclose (f);
return 0;
}
