//validators.c
//
//data checking functions

#include <string.h>
#include <stdio.h>
#include "../asmdefs.h"
#include "util.h"

int hasTrailingComma(char *str)
{
	char *last;
	
	if(*str == '\0')
		return 0;
	
	while(*str != '\0')
		last = str++;
	
	return (*last == ',');
}

//returns 1 if true
//
int isInstruction(char *str)
{
	int i;
	
	for(i = 0; i < numberOfInstr; i++)
	{
		if(strcmp(str, instructionMnemonics[i]) == 0)
		return 1;
	}
	
	return 0;
}

//check to see if this is a constant
//
//constant arguments take the form of
//	value		label
//
//if the string has brackets then it's something else
int isConstArg(char *str)
{
	if(*str == '\0')
		return 0;
		
	if(*str == ';')
		return 0;

	
	//exclude memory and pointer arguments
	if(*str == '[')
		return 0;

	return 1;
}

//check to see if this is an indexed argument
//
//indexed memory arguments take the form of
//	[value+i]	[label+i]
//
//
int isIndArg(char *str)
{
	if(*str == '\0')
		return 0;

	if(*str == ';')
		return 0;
	
	//pull out the first bracket
	if(*str != '[')
		return 0;
	
	str++;
	
	//exclude pointer arguments
	if(*str == '[')
		return 0;
		
	//seek to "+i]"
	if(strstr(str, "+i]") != NULL)
		return 1;
		
	return 0;
}

//check to see if this is a memory argument
//
//memory arguments take the form of
//	[value]		[label]
//
//
int isMemArg(char *str, int line)
{
	if(*str == '\0')
		return 0;

	if(*str == ';')
		return 0;
	
	//pull out the first bracket
	if(*str != '[')
		return 0;
		
	str++;
	
	//exclude pointer arguments
	if(*str == '[')
		return 0;
		
	//check for matching end bracket
	while(*str != '\0' && *str != ']')
		str++;
	
	if(*str == '\0')
		return 0;

	str++;
	
	//If we got this far brackets match
	//check for proper termination
	if(*str != '\0')
		return 0;
	
	return 1;
}

//check to see if this is a pointer argument
//
//pointer arguments take the form of
//	[[value]]	[[label]]
//
int isPtrArg(char *str, int line)
{
	if(*str == '\0')
		return 0;
		
	if(*str == ';')
		return 0;
		
	//extract bracket 1
	if(*str != '[')
		return 0;
	
	str++;

	//extract bracket 2
	if(*str != '[')
		return 0;
	
	//ParseFile verifies data don't do it here
	//just check the end brackets
	while(*str != '\0' && *str != ']')
		str++;
	
	//look for matching end brackets and proper termination
	if(*str == '\0')
		return 0;
	
	str++;
	
	if(*str != ']')
		return 0;
	
	str++;
	
	if(*str != '\0')
		return 0;
	
	return 1;
}

int isConstDecl(char *str)
{
	if(strcmp(str, constDeclarationKeyword) == 0)
		return 1;
		
	return 0;
}

//
//Returns the index of the data type keyword size
//used to update address counter
//TODO: give it a better name
//
int isDataDecl(char *str)
{
	int i;
	
	for(i = 0; i < numberOfDataKw; i++)
	{
		if(strcmp(str, dataDeclarationKeywords[i]) == 0)
			return 1 << i;
	}
	
	return 0;
}

//boolean function
//returns 1 if condition is true
//
int isLabel(char *str)
{
	char *s = str;
	
	while(*s != '\0' && *s != ':')
		s++;
		
	if(*s == ':')
		return 1;
		
	return 0;
}

//token ends with "+<number>" or "-<number>"
//only works if string termination immediately follows the number
//
//derp+0xFF00 works
//dw+++e+0xFF00 does not
int isAddConst(char *str)
{
	char *s = str;
	
	while(*s != '\0' && *s != '+' && *s != '-')
		s++;
	
	if(*s == '\0')
		return 0;
	
	if(isDecimal(s))
		return 1;
			
	//skip sign
	s++;
	
	if(isHex(s))
		return 1;
		
	return 0;
}

int isDecimal(char *str)
{
	char *s = str;
	
	if(*s == '\0')
		return 0;
	
	if(*s == '-' || *s == '+')
		s++;
		
	while(*s != '\0')
	{	
		if(*s < '0' || *s > '9')
			return 0;
		
		s++;
	}
	
	return 1;
}

int isHex(char *str)
{
	char *s = str;

	if(*s == '\0')
		return 0;
	
	if(*s == '0')	//check prefix for 0x
	{
		s++;
		
		if(*s == 'x')
			s++;
		else
			return 0;
	}
	else
		return 0;
	
	while(*s != '\0')
	{	
		//if not 0-9 or a-f
		
		if(!((*s >= '0' && *s <= '9') ||
			(tolower(*s) >= 'a' && tolower(*s) <= 'f')))
			return 0;
		
		s++;
	}
	
	return 1;
}

//boolean function
//returns 1 if condition is true
//
int isComment(char *str)
{
	if(*str == ';')
		return 1;
		
	return 0;
}

//boolean function
//
//returns 1 if start of string
//returns 0 if not
//
int isStrBegin(char *str)
{
	return (*str == '\"');
}

//isStrMatch( )
//matches a quoted string out of a token stream
//returns 1 if true
//		  0 if false
//
//	checks by double quotes
//		"this is a valid string" 
//	works
//  	"this string has trailing cha"rs
//	does not
//		"this string does not terminate properly
//	doesn't work either
//
//Side-effects: changes tok and pulls tokens out of token stream
//
int isString(char **tok)
{
	int bFound;
	char *s;
	
	bFound = 0;
	
	if(*tok != NULL && **tok == '\"')
	{
		s = (*tok)+1;
		
		while(*tok != NULL && !bFound)
		{
			while(*s != '\0' && *s != '\"') //scan token for matching quote
				s++;
			
			if(*s == '\"')  		//if found look for trailing chars
			{
				bFound = 1;
				s++;
				
				if(*s != '\0')
					*tok = s;
				else
					*tok = NULL;
			}
			else					//try the next one
			{
				*tok = strtok(NULL, delim);
				s = *tok;
			}
		}
	}
	
	if(bFound && *tok != NULL)	//disallow trailing input for the time being
		bFound = 0;
		
	return bFound;
}

//escCharCheck(char**, int)
//
//returns 1 if error
//returns 0 if this is a valid escaped character
//
//If escCharCheck finds invalid data it will print out an error message
//Otherwise it scans past the escaped character
//
int EscCharCheck(char **seq, int line)
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
	{
		printf("Line %d - Error:  no escaped characters in string ", line);
		PrintSanitized(*seq);
		printf("\n");
		return 1;
	}
	
	
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
				break;
			}
		}
		
		if(!escConst)
		{
			printf("Line %d - Error: Invalid escape character ", line);
			PrintEscSeq(*seq);
			printf(" in string ");
			PrintSanitized(*seq);
			printf("\n");
			toReturn = 1;
		}
		else
			toReturn = 0;
		
	}
	else if(savedVal > maxCharVal)
	{
		printf("Line %d - Error:  escape constant ", line);
		PrintEscSeq(*seq);
		printf("  overflows char size in string ");
		PrintSanitized(*seq);
		printf("\n");
	}
	
	if(*s != '\0')
		*seq = s;		//skip to end
	
	return toReturn;
}


//returns 1 if error
//returns 0 if the label is clean
//
int LabelCheck(char* label, int line)
{
	char *s = label;
	
	//printf("Label check called for %s\n", s);
	
	//must start with letter
	if(*s >= '0' && *s <= '9')
	{
		printf("Line %d - Error: Label %s starts with a number\n", 
				line, label);
		return 1;
	}
	else if(tolower(*s) < 'a' ||
			 tolower(*s) > 'z')
	{
		printf("Line %d - Error: Label %s has an illegal character\n", 
				line, label);
		return 1;
	}
	
	while(*s != '\0' && *s != ':')
	{
		if((*s >= '0' && *s <= '9') || 
				(tolower(*s) >= 'a' && tolower(*s) <= 'z'))
		{
			//printf("%c is valid\n", *s);
		}
		else
		{
			printf("Line %d - Error: Label %s has an illegal character\n", line, label);
			return 1;
		}
		s++;
	}
		
	if(*s == ':')
	{
		//remove the trailing colon
		*s = '\0';
	}
		
	return 0;
} //end LabelCheck
