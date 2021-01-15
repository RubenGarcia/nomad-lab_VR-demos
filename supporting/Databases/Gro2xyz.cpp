#include <stdio.h>
#include <string.h>

void discardline (FILE *f1)
{
char buff [200];
fgets (buff, 200, f1);
while (buff[strlen(buff)-1] != '\n') {
	fgets (buff, 200, f1);

}
}

int copyline (FILE *f1, FILE* f2)
{
char buff [200];
fgets (buff, 200, f1);
fputs (buff, f2);
while (buff[strlen(buff)-1] != '\n') {
	fgets (buff, 200, f1);
	fputs (buff, f2);
}
return 0;
}


int gro2xyz (const char *gro, const char *xyz, bool hasV)
{
FILE *f1, *f2;

f1=fopen (gro, "r");
if (f1==nullptr)
	return 1;
f2=fopen(xyz, "w");
if (f2==nullptr) {
	fclose (f1);
	return 2;
}

discardline (f1);
int numatoms;
fscanf (f1, "%d", &numatoms);
fprintf (f2, "%d\n", numatoms);

fprintf (f2, "Converted from %s\n", gro);

char token [20];
float pos;
int current=0;
while (!feof(f1) && current<numatoms) {
	fscanf (f1, "%s", token);//discard
	fscanf (f1, "%s", token);//atom name
	fprintf (f2, "%s\t", token);
	fscanf (f1, "%s", token);//discard	
	for (int i=0;i<3; i++) {
		fscanf (f1, "%f", &pos); //nm 
		fprintf (f2, "%f\t", pos*10); //angstrom
	}
	if (hasV)
		for (int i=0;i<3; i++) { //velocities, discard
			fscanf (f1, "%s", token); 
		}
	fprintf (f2, "\n");
	current++;
}//discard box

fclose (f1);
fclose (f2);
return 0;
}

/*
int main (void) 
{
gro2xyz ("outframe.gro", "outframe.xyz", false);
gro2xyz ("npt.gro", "npt.xyz", true);
}
*/
