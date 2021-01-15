#include <stdio.h>
#include "Archive.h"

int main (void)
{
int cresult=0;
int A=12;
int B=21;

int res=ParseArchiveResults(A, B, cresult, "Infobox");

fprintf (stderr, "Result: %d\n", res);

return 0;
}
