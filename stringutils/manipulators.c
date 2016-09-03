//manipulators.c
//
//string formatting and conversions

#include <string.h>
#include <stdio.h>
#include "../asmdefs.h"
#include "util.h"

//int GetEscCharData(char**):
//	maps character-based escape sequences of type 
//	"\<char>" and "\<num><num><num>" to
//	a number value for use as a control character.
//
//	Standard character values for newline, tab, and so on
//	are stored in asmdefs.c as escCharMap[]
//
//	Assumes numeric escape sequences are in octal format.
//
int GetEscCharData(char **seq)
{
	int i;
	int escConst;
	int numSeq;
	int savedVal;
	int toReturn;
	char *s;
	
	i = 0;
	savedVal = 0;
	escConst = 0;
	numSeq = 1;
	toReturn = 0;
	s = *seq;
	
	if(*s != '\\')
		return 0;
	
	//check to  see if this is a 3 digit sequence
	for(i = 0; i < numOctalPerByte; i++)
	{
		savedVal <<= 3;
		s++;
		
		if(*s < '0' || *s > '7')
		{
			numSeq = 0;
			break;
		}
		else
			savedVal += *s - '0';
	}
	
	if(!numSeq)
	{
		s = (*seq) + 1;	//reset to first character after backslash
		
		//check to see if in array of escaped characters
		for(i = 0; i < numEscChars; i++)
		{
			if(*s == escChars[i])
			{
				escConst = 1;
				toReturn = escCharMap[i];
				break;
			}
		}
	}
	else
		toReturn = savedVal;
	
	
	if(*s != '\0')
		*seq = s;		//skip to end
	
	return toReturn;
}

void StripTrailingComma(char *str)
{
	char *last;
	
	if(*str == '\0')
		return;
	
	while(*str != '\0')
		last = str++;
	
	if(*last == ',')
		*last = '\0';
}

//replaced with simpler version
void StripBrackets(char *str)
{
	char *to;
	char *from;
	
	to = str;
	from = str;
	
	while(*to != '\0')
	{
		if(*to != '[' && *to != ']')
		{
			*from = *to;		
			from++;
		}
		
		to++;
	}
	
	*from = 0;
}

//Strips out the substring "+i"
void StripIndexSuffix(char *str)
{
	char *to;
	char *from;
	
	to = strstr(str, "+i");

	if(to != NULL)
	{
		//this substring is found so skip past it
		from = to+2;
	
		while(*from != '\0')
			*to++ = *from++;
	
		*to = '\0';
	}
}

//int a2h()
//naively reads an n byte ascii string as hex value
int a2h(char *str)
{
	int retval = 0;
	
	//printf("calling a2h for %s\n", str);
	
	while(*str != '\0')
	{
		retval <<= 4;
		
		if(*str >= '0' && *str <= '9')
			retval += *str - '0';
			
		else if(tolower(*str) >= 'a' && tolower(*str) <= 'f')	
			retval += tolower(*str) - 'a' + 10;
		else
			return 0;
	
		str++;
	}
	
	return retval;
}

//int a2o()
//naively reads an n byte ascii string as oct value
int a2o(char *str)
{
	int retval = 0;
	
	while(*str != '\0')
	{
		retval <<= 3;
		
		if(*str >= '0' && *str <= '7')
			retval += *str - '0';
		else
			return 0;
		
		str++;
	}
	
	return retval;
}