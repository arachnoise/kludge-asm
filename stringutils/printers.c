//printers.c
//
//various string printing functions

#include <string.h>
#include <stdio.h>
#include "../asmdefs.h"
#include "util.h"

//PrintComment is hackwork
//it breaks comment formatting.
//Be careful with tabs and multiple spaces
//Also, it only works if strtok has values loaded
//
void PrintComment(FILE *toWrite, char* str)
{
	char *tok;
	
	
	fprintf(toWrite," \t\t%s", str+1);
	
	tok = strtok(NULL, delim);
	
	while(tok != NULL)
	{
		fprintf(toWrite,"  %s", tok);
		tok = strtok(NULL, delim);
	}
	
	fprintf(toWrite, "\n");
}

//prints out a substring with an escape sequence

void PrintEscSeq(char *str)
{
	int i = 0;
	
	if(*str != '\\')
		return;
		
	putc('\"', stdout);
	putc(*str++, stdout);
		
	if(*str < '0' || *str > '7')
		putc(*str++, stdout);
	else	
		while(*str >= '0' && *str <= '7' && i < numOctalPerByte)
		{
			putc(*str++, stdout);
			i++;
		}
	
	putc('\"', stdout);
}

void PrintData(FILE *toWrite, 
	const char *savBuf, 
	char *str, 
	int *baseAddress, 
	int size)
{
	char intbuf[numCharsPerInt+1];
	int data;
	int i;
	i = 0;
	
	intbuf[numCharsPerInt] = 0;
	
	if(isStrBegin(str))
	{
		char *s;
		int i;
		
		s = 1 + strstr(savBuf, str);
		i = 0;
		data = 0;
		
		if(isString(&str))
		{
			while(*s != '\"')
			{
				if(*s != '\\')
				{
					data = (data<<8) + *s; 	//pack in byte by byte
				}
				else
				{
					int temp = GetEscCharData(&s);
					data = (data<<8) + temp;
				}
				
				i++;
				s++;
				
				if(i == intSize)		//print if container is full
				{
					fprintf(toWrite, "%.4X\t", *baseAddress);
					fprintf(toWrite, "%.8X\n", data);
					
					i = 0;
					data = 0;
					*baseAddress += 1;
					size--;
				}	
			}
			
			if(i > 0)		//if i is set we have unprinted data!
			{
				data <<= 8*(intSize-i);	//shift left so HO byte has data
				
				fprintf(toWrite, "%.4X\t", *baseAddress);
				fprintf(toWrite, "%.8X\n", data);
				*baseAddress += 1;
				size--;
			}
			
			if(size > 0)  //write here to allocate remaining space
			{
				i = 0;
				
				while(i < size)
				{					
					fprintf(toWrite, "%.4X\t", *baseAddress);
					fprintf(toWrite, "%.8X", 0);
					*baseAddress += 1;
					i++;
					
					if(i < size)
						fprintf(toWrite, "\n");
				}
			}
			else		//Make sure to write proper null termination
			{
				fprintf(toWrite, "%.4X\t", *baseAddress);
				fprintf(toWrite, "%.8X", 0);
				*baseAddress += 1;
			}
		}
	}
	else if(isDecimal(str))
	{
		//printf("Decimal string is %s\n", str);
		
		while(i < size)
		{
			strnset(intbuf, 0, numCharsPerInt);
			
			if(*str != '\0')
			{
				int j = 0;
				//printf("Dec string is %.8s\n", str);
				strncpy(intbuf, str, numCharsPerInt);
				
				while(j < numCharsPerInt && str != '\0')
				{
					j++;
					str++;
				}
			}
			
			data = atoi(intbuf);
			//printf("string is %s\n", str);
			//printf("data is %X\n", data);
			fprintf(toWrite, "%.4X\t", *baseAddress);
			fprintf(toWrite, "%.8X", data);
			
			*baseAddress += 1;
			i++;
			
			if(i < size)
				fprintf(toWrite, "\n");
		}
	}
	else if(isHex(str))
	{
		//skip past hex prefix "0x"
		//printf("Hex string is %s\n", str);
		str += 2;
		
		while(i < size)
		{
			strnset(intbuf, 0, numCharsPerInt);
			
			if(*str != '\0')
			{
				int j = 0;
				//printf("Hex string is %.8s\n", str);
				strncpy(intbuf, str, numCharsPerInt);
				
				while(j < numCharsPerInt && str != '\0')
				{
					j++;
					str++;
				}
			}
			
			data = a2h(intbuf);
			
			//printf("string is %s\n", str);
			//printf("data is %X\n", data);
			fprintf(toWrite, "%.4X\t", *baseAddress);
			fprintf(toWrite, "%.8X", data);
			
			*baseAddress += 1;
			i++;
			
			if(i < size)
				fprintf(toWrite, "\n");
		}
	}
} //end PrintData

void PrintSanitized(char *str)
{	
	putc('\"', stdout);
	
	while(*str != '\0')
	{
		if(*str == '\\')
		{
				putc('\\', stdout);
				putc('\\', stdout);
		}
		else if(*str == '\n')
		{
				putc('\\', stdout);
				putc('n', stdout);
		}
		else if(*str == '\r')
		{
				putc('\\', stdout);
				putc('r', stdout);
		}
		else if(*str == '\t')
		{
				putc('\\', stdout);
				putc('t', stdout);
		}
		else if(*str == '\b')
		{
				putc('\\', stdout);
				putc('b', stdout);
		}
		else if(*str == '\f')
		{
				putc('\\', stdout);
				putc('f', stdout);
		}
		else if(*str == '\"')
		{
			putc('\\', stdout);
			putc('\"', stdout);
		}
		else if(*str < ' ')
		{
			putc('\\', stdout);
			printf("%.3o", *str);
		}
		else
			putc(*str, stdout);
		*str++;
	}
	putc('\"', stdout);
	
}