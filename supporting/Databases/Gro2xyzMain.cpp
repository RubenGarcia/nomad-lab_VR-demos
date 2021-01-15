#include <stdio.h>

#include "Gro2xyz.h"

int main (int argc, char ** argv)
{
if (argc != 4) {
	fprintf (stderr, "%s <gro file> <xyz file> <F=gro has no velocity, T=gro has velocity>\n", argv[0]);
}

gro2xyz (argv[1], argv[2], argv[3][0]=='T');

}
