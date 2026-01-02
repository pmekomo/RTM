#include "utile.h"

int mystrcmp(char * ch1, const char *ch2)
{
	unsigned int i = 0;
	while((ch1[i]!='\0') && (ch1[i]== ch2[i]))
		i++;
	if (i == strlen(ch2))
		return 0;
	else
		return 1;
}

