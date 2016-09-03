#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "asmdefs.h"
#include "stringutils/util.h"

//main program functions
int ParseFile(FILE *inFile, SymbolRecord *symTable, int *numSymbols);
void PrintSymbolTable(SymbolRecord *symTable, int numSymbols);
void OutToCode(FILE *inFile, FILE *outFile, SymbolRecord *symTable,
					int *numSymbols);

int main(int argc, char *argv[])
{
	SymbolRecord symTable[maxSymbols];
	int numSymbols;
	int numErrors;
	
	FILE *inFile;
	FILE *outFile;

	if(argc >= 2)
	{		
		char *outFileName;
		char nameBuf[maxArgLength];
		
		inFile = fopen(argv[1], "r");
		
		if(inFile == NULL)
		{
			fprintf(stderr, "Error: %s does not exist \n", argv[1]);
			exit(1);
		}

		//allow "-o" option to specify output file name
		
		if(argc > 2)
		{
			if(strcmp(argv[2], "-o") == 0)
			{
				if(argc >= 4)
					outFileName = argv[3];
			}
			else
			{
				fprintf(stderr, "Usage: %s <InputFileName> -o <OutputFileName>\n", 
						argv[0]);
				exit(1);
			}
		}
		else
		{
			//if no filename provided
			//make a clone name with .klu extension
			
			int index = 0;
			strcpy(nameBuf, argv[1]);
			
			while(nameBuf[index] != '\0' && 
				  nameBuf[index] != '.'  &&
				  index < maxArgLength)
				index++;
			
			//TODO: replace this with name truncating code
			//It makes more sense to preserve 
			//the entire file extension
			
			if(index < (maxArgLength - 2))
			{
				nameBuf[index+1] = 'k';
				nameBuf[index+2] = '\0';
			}
			
			if(index < (maxArgLength - 3))
			{
				nameBuf[index+2] = 'l';
				nameBuf[index+3] = '\0';
			}
				
			if(index < (maxArgLength - 4))
			{
				nameBuf[index+3] = 'u';
				nameBuf[index+4] = '\0';
			}
			
			outFileName = nameBuf;
		} //end else
		
		outFile = fopen(outFileName, "w");
		
		if(outFile == NULL)
		{
			fprintf(stderr, "Error: can not open %s\n", outFileName);
			exit(1);
		}
		
	} //end if
	else
	{
		printf("Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	numErrors = ParseFile(inFile, symTable, &numSymbols);
	
	if(numErrors > 0)
	{
		printf("%d errors found.\n\n", numErrors);
	}
	else
	{
		rewind(inFile);
		OutToCode(inFile, outFile, symTable, &numSymbols);
	}

	//make symbol printing optional
        PrintSymbolTable(symTable, numSymbols);
	
	fclose(inFile);
	fclose(outFile);
}

//void ParseFile(...)
//Preconditions: pointers must be valid
//inFile must point to beginning of a file
//
//This function does not allocate memory!
//
//ParseFile parses an input file and creates a 
//symbol table consisting of 3 entry length records
//
//Evaluating Expressions:
//This is a very simple parser with hard-coded rules
//
//Any line must be one of the following
//Instruction Declaration
//Data Declaration
//Label Declaration
//Comment
//
//Labels:
//Any line starting with word that has letters or numbers
//followed by ":" is a label declaration
//Labels must not start with a number
//
//
//A line with a label declaration followed by a comment (or nothing)
//is assigned the address of the next instruction or data declaration
//
//At the end Parsefile performs a naive check for address overflow
//Will have to change this when we implement code organization
//
int ParseFile(FILE *inFile, SymbolRecord *symTable, int *numSymbols)
{
	char theLine[lineSize];
	char savedLine[lineSize];
	char* tok;
	char *s;				//string loop counter
	
	int address;
	int addressInc;			//required to space large sized data core
							//eg qword and para
	int line;
	int totalErrors;
	int isError;
	int foundSymbols;

	SymbolRecord instrSymbols[maxSymbols];  //found symbols go here for name resolution
	
	*numSymbols = 0;
	foundSymbols = 0;
	address = 0;
	line = 1;
	totalErrors = 0;
	
	while(fgets(theLine, lineSize, inFile))
	{
		isError = 0;	
		strcpy(savedLine, theLine);
		tok = strtok(theLine, delim);
	
		if(tok != NULL)		//Check declaration type for token
		{						//Is it instruction, data, or label?
			if(isInstruction(tok))	//if instruction
			{						//get instruction index and look up type
				int instrType;
				int argType;
				char argBuf[maxSymbolLength];
			
				int i = 0;
				
				while(strcmp(tok, instructionMnemonics[i]) != 0 &&
						i < numberOfInstr )
					i++;
					
				instrType = instructionTypeMap[i];
				tok = strtok(NULL, delim);
				
				if(tok != NULL)	//look up argument type
				{
					if(isConstArg(tok))
						argType = constArg;
					else if(isIndArg(tok))
						argType = indArg;
					else if(isMemArg(tok, line))
						argType = memArg;
					else if(isPtrArg(tok, line))
						argType = ptrArg;
					else if(isComment(tok))
						argType = argUnset;
					else
					{
						printf("Line %d - Error: invalid data\n", 
								line, instructionMnemonics[i]);
						isError = 1;
					}
					
					strcpy(argBuf, tok);
					StripBrackets(argBuf);
					
					if(argType == indArg)
						StripIndexSuffix(argBuf);

					if(!isDecimal(argBuf) && !isHex(argBuf)) //Non-matches include
					{										  //Label, label+const
						if(isAddConst(argBuf))
						{
							char *s = argBuf;
							
							while(*s != '+' && *s != '-')		//seek to label
								s++;
							
							*s = '\0';	//number is no longer part of the string
							s++;
						}
						
						if(instrType != instrNoArgs &&
							(LabelCheck(argBuf, line) == 0))
						{
							int j;
							
							for(j = 0; j < foundSymbols; j++)
							{
								if(strcmp(argBuf, instrSymbols[j].name) == 0)
									break;
							}
							
							if(j == foundSymbols)	//have we found a new symbol?
							{
								strcpy(instrSymbols[j].name, argBuf);
								instrSymbols[j].address = address;
								
								if(instrType == instrConstArgs)
									instrSymbols[j].isConstant = 1;
								else
									instrSymbols[j].isConstant = 0;
									
								instrSymbols[j].lineNumber = line;
								foundSymbols++;
							}
						}
						else
						{
							if(instrType != instrNoArgs)
								printf("Line %d - Error: invalid argument for %s instruction\n",
										line, instructionMnemonics[i]);
						}
					} //end else
				}
				else
					argType = argUnset;
				
				if(instrType == instrAllArgs)	//match arg type to instruction type
				{
					if(argType == argUnset)
					{
						printf("Line %d - Error: missing argument for %s instruction\n",
								line, instructionMnemonics[i]);
						isError = 1;
					}
				}
				else if(instrType == instrConstArgs)
				{
					if(argType == memArg)
					{
						printf("Line %d - Error: memory argument for %s instruction\n",
								line, instructionMnemonics[i]);
						printf("\tRequires a constant value\n");
						isError = 1;
					}
					else if(argType == indArg)
					{
						printf("Line %d - Error: indexed memory argument for %s instruction\n",
								line, instructionMnemonics[i]);
						printf("\tRequires a constant value\n");
						isError = 1;
					}
					else if(argType == ptrArg)
					{
						printf("Line %d - Error: pointer argument for %s instruction\n",
								line, instructionMnemonics[i]);
						printf("\tRequires a constant value\n");
						isError = 1;
					}
					else if(argType == argUnset)
					{
						printf("Line %d - Error: missing/invalid argument for %s instruction\n",
								line, instructionMnemonics[i]);
						printf("\tRequires a constant value\n");
						isError = 1;
					}
				}
				else if(instrType == instrNoConstArgs)
				{
					if(argType == constArg)
					{
						printf("Line %d - Error: constant argument not allowed for %s instruction\n",
								line, instructionMnemonics[i]);
						isError = 1;
					}
					else if(argType == argUnset)
					{
						printf("Line %d - Error: missing/invalid argument for %s instruction\n",
								line, instructionMnemonics[i]);
						isError = 1;
					}
				}
				else if(instrType == instrNoArgs)
				{
					if(argType != argUnset)
					{
						printf("Line %d - Error: No argument necessary for %s instruction\n",
								line, instructionMnemonics[i]);
						isError = 1;
					}
				}
				
				address++;
			} //end if isInstruction
			else if(addressInc = isDataDecl(tok))
			{	
				tok = strtok(NULL, delim);
				
				if(tok != NULL)	//expect a numeric value
				{
					//string verification goes here
					
					if(isStrBegin(tok))
					{
						int minAddrInc = addressInc;
						int len = 0;
						
						s = 1 + strstr(savedLine, tok);
						
						if(isString(&tok))
						{
							while(*s != '\"')
							{
								if(*s == '\\')
									isError = EscCharCheck(&s, line);
									
								s++;
								len++;
							}
							
							//empty string requires 1 slot
							//1-4 chars requires 2
							//5-8 chars requires 3
							//etc
							
							addressInc = 1 + (len+3)/4;
							
							if(addressInc < minAddrInc)
								addressInc = minAddrInc;
						}
						else
						{
							printf("Line %d - Error: invalid string after data declaration\n", line);
							isError = 1;
						}
					}
					else if(hasTrailingComma(tok))
					{
						//if token ends with comma then 
						//another argument must follow it
						
						while(hasTrailingComma(tok) && tok != NULL)
						{
							StripTrailingComma(tok);
							
							if(!isDecimal(tok) && !isHex(tok))
							{
								printf("Line %d - Error: invalid data after data declaration\n", line);
								isError = 1;
							}
							
							tok = strtok(NULL, delim);
							address += addressInc;
						}
						
						if(tok == NULL)
						{
							printf("Line %d: - Error: need more data for multiple data declaration\n", line);
							isError = 1;
						}
					}
					else if(!isDecimal(tok) && !isHex(tok))
					{
						printf("Line %d - Error: invalid data after data declaration\n", line);
						isError = 1;
					}	
				}	//end if(tok!= NULL)
				else
				{
					printf("Line %d - Error: a number or string is expected after data declaration\n", line);
					isError = 1;
				}
				
				tok = strtok(NULL, delim);
				
				if(tok != NULL)
				{
					if(!(isComment(tok)))
					{
						printf("Line %d - Error: %s is invalid input.\n", line, tok);
						isError = 1;
					}
				}
				
				address += addressInc;
			}
			else if(isComment(tok))
			{
				//do nothing
			}
			else if(isLabel(tok))
			{
				isError = LabelCheck(tok, line);
				
				if(!isError)
				{
					char symBuf[maxSymbolLength];
					int isConst = 0;
					int savedAddress = address;
					int j;
					
					strcpy(symBuf, tok);	//save for symbol table check
					tok = strtok(NULL, delim);
	
					if(tok != NULL)	//see if data declaration follows
					{
						if(addressInc = isDataDecl(tok))
						{	
							tok = strtok(NULL, delim);
						
							if(tok != NULL)
							{
								//string verification goes here
								if(isStrBegin(tok))
								{
									int minAddrInc = addressInc;
									int len = 0;
									
									s = 1 + strstr(savedLine, tok);
									
									if(isString(&tok))
									{
										while(*s != '\"')
										{
											if(*s == '\\')
												isError = EscCharCheck(&s, line);
											
											s++;
											len++;
										}
										
										//empty string requires 1 slot
										//1-4 chars requires 2
										//5-8 chars requires 3
										//etc
										
										addressInc = 1 + (len+3)/4;
										
										if(addressInc < minAddrInc)
											addressInc = minAddrInc;
									}
									else
									{
										printf("Line %d - Error: invalid string after data declaration\n", line);
										isError = 1;
									}
								}
								else if(hasTrailingComma(tok))
								{
									//if token ends with comma then 
									//another argument must follow it
									
									while(hasTrailingComma(tok) && tok != NULL)
									{
										StripTrailingComma(tok);
										
										if(!isDecimal(tok) && !isHex(tok))
										{
											printf("Line %d - Error: invalid data after data declaration\n", line);
											isError = 1;
										}
										
										tok = strtok(NULL, delim);
										address += addressInc;
									}
									
									if(tok == NULL)
									{
										printf("Line %d: - Error: need more data for multiple data declaration\n", line);
										isError = 1;
									}
								}
								else if(!(isDecimal(tok) ||
									isHex(tok)))
								{
									printf("Line %d - Error: invalid data after data declaration\n", line);
									isError = 1;
								}	
							}
							else
							{
								printf("Line %d - Error: a number or string is expected after data declaration\n", line);
								isError = 1;
							}
							
							address += addressInc;
						}
						else if(isConstDecl(tok))
						{
							isConst = 1;
							tok = strtok(NULL, delim);
						
							if(tok != NULL)
							{
								if(isDecimal(tok))
								{
									savedAddress = atoi(tok);
								}
								else if(isHex(tok))
								{
									savedAddress = a2h(tok+2);
								}
								else
								{
									printf("Line %d - Error: invalid number after const declaration\n", line);
									isError = 1;
								}
								
							}
							else
							{
								printf("Line %d - Error: a number is expected after const declaration\n", line);
								isError = 1;
							}
						}
						else if(isComment(tok))
						{
							line++;
							continue;
						}
						else
						{
							printf("Line %d - Error: %s is invalid input.\n", line, tok);
							isError = 1;
						}
						
						tok = strtok(NULL, delim);
						
						if(tok != NULL)
						{
							if(!(isComment(tok)))
							{
								printf("Line %d - Error: %s is invalid input.\n", line, tok);
								isError = 1;
							}
						}
					} //end data declaration check
					
					for(j = 0; j < *numSymbols; j++)	
						if(strcmp(symBuf, symTable[j].name) == 0)
							break;
					
					if(j == *numSymbols)
					{
						strcpy(symTable[j].name, symBuf);
						symTable[j].address = savedAddress;
						symTable[j].isConstant = isConst;
						symTable[j].lineNumber = line;
						*numSymbols += 1;
					}
					else
					{
						isError = 1;
						printf("Line %d - Error: label declaration is a duplicate of %s at line %d\n",
								line, symBuf, symTable[j].lineNumber);
					}
				} //end if !isError
			} //end if isLabel
			else
			{
				printf("Line %d - Error: %s is invalid input.\n", line, tok);
				isError = 1;
			}
		}	//end if(tok != NULL)
		
		totalErrors += isError;
		line++;
		
	} //end while
	
	
	//printf("\nSymbols used in instructions:\n\n");
	//PrintSymbolTable(instrSymbols, foundSymbols);
	
	//seek through symbolTable for unresolved symbols
	//exhibits n^2 behavior
	//not a problem (for now)
	
	int i;
	int j;
	
	for(i = 0; i < foundSymbols; i++)
	{
		for(j = 0; j < *numSymbols; j++)
			if(strcmp(instrSymbols[i].name, symTable[j].name) == 0)
				break;
		
		if(j == *numSymbols)		//if no match found, print error
		{
			printf("Line %d - Error: Unresolved symbol %s\n", 
				instrSymbols[i].lineNumber, instrSymbols[i].name);
			totalErrors++;
		}
	}
	
	if(address > maxProgLength)	
		printf("Error: address %X is longer than program maximum length %X\n",
				address, maxProgLength);
	
	return totalErrors;
} //end ParseFile

//Prints a symbol table in human readable format
//Used for debugging purposes
void PrintSymbolTable(SymbolRecord *symTable, int numSymbols)
{
	int i;
	
	printf("Printing Symbol Table:\n");
	
	for(i = 0; i < numSymbols; i++)
	{
		printf("Record:\t%d:\n", i);
		printf("Name:\t%s\n", symTable[i].name);
		printf("Address:\t%d\n", symTable[i].address);
		printf("Is Constant:\t%d\n", symTable[i].isConstant);
		printf("Line number:\t%d\n\n", symTable[i].lineNumber);
	}
}


//translates instructions and labels into code
//
//Precondition: assumes file is clean and has no labeling errors
//				in this it depends on ParseFile
//
//Label substitution:
//OutToCode will substitute a value for a declared label
//
//ex 1:
//
//	load bacon
//
//	bacon dword 0
//
//Here the parser will substitute the address at the dword declaration
//for bacon. If this program starts at address 0x00 then this turns into
//
//	load [01]
//
//ex 2
//
//	load bacon
//
//	bacon equ 21
//
//Here the parser will substitute decimal 21 for the label bacon
//
//  load 21
void OutToCode(FILE *inFile, FILE *outFile, SymbolRecord *symTable,
					int *numSymbols)
{
	char theLine[lineSize];
	char savedLine[lineSize];
	
	char *tok;
	
	int instrCode;
	int address;
	int addressInc;
	int addressMode;
	
	unsigned short operand;
	int data;
	int line;
	
	line = 0;
	address = 0;
	
	while(fgets(theLine, lineSize, inFile))
	{
		strcpy(savedLine, theLine);
		tok = strtok(theLine, delim);
		
		if(tok != NULL)
		{
			if(isInstruction(tok))
			{
				int i = 0;
				char argBuf[maxSymbolLength];
				
				while(strcmp(tok, instructionMnemonics[i]) != 0 &&
						i < numberOfInstr )
					i++;
					
				instrCode = i;
				fprintf(outFile, "%.4X", address);	//write address 4 digits
				fprintf(outFile, "\t%.2X", instrCode+1);	//write instruction 2 digits
								
				if(instructionTypeMap[instrCode] != instrNoArgs)
				{				
					tok = strtok(NULL, delim);
				
					if(isConstArg(tok))	//write addressing mode 2 digits
						fprintf(outFile, "20"); 
					else if(isIndArg(tok))
						fprintf(outFile, "80");
					else if(isMemArg(tok, line))
						fprintf(outFile, "00"); 
					else if(isPtrArg(tok, line))
						fprintf(outFile, "40");
					
					strcpy(argBuf, tok);
					StripBrackets(argBuf);
					
					if(isIndArg)
						StripIndexSuffix(argBuf);
					
					if(isDecimal(argBuf))
					{
						operand = (short)atoi(argBuf);
						fprintf(outFile, "%.4hX", operand);
					}
					else if(isHex(argBuf))
					{
						operand = (short)a2h(argBuf+2);
						fprintf(outFile, "%.4hX", operand);
					}
					else	//exceptions include Label, label+const
					{
						int saveConst = 0;
						
						if(isAddConst(argBuf))
						{
							char *s = argBuf;
							char *save;
							
							while(*s != '+' && *s != '-')		//seek to label
								s++;
							
							save = s;
							
							if(isDecimal(s))
								saveConst = atoi(s);
							
							s++;	//skip sign character
							
							if(isHex(s))
								saveConst = a2h(s+2);
							
							*save = '\0';	//number is now no longer part of the string
						}
						
						if(LabelCheck(argBuf, line) == 0)
						{
							int i = 0;
							
							//read label from symbol Table
							while(strcmp(argBuf, symTable[i].name) != 0 &&
							i < *numSymbols)
								i++;
							
							if(i == *numSymbols)
							{
								printf("Error writing file at line %d: missing label %s\n",
										line, argBuf);
								exit(1);
							}
							else
							{
								unsigned short temp;	//for overflow detection
								
								temp = (short)symTable[i].address;		//write address from record
								operand = (short)symTable[i].address + (short)saveConst;
										
								if(saveConst > 0 && temp > operand)
								{
									printf("Error writing: address overflow at line %d - %X + %d is %X\n", 
										line, temp, saveConst, operand);
									exit(1);
								}
								
								if(saveConst < 0 && temp < operand)
								{
									printf("Error writing: address underflow at line %d - %X + %d is %X\n", 
										line, temp, saveConst, operand);
								}
								
								fprintf(outFile, "%.4hX", operand);
							}
						} //end if(LabelCheck( ))
					}//end else
				} //end isInstruction
				else
					fprintf(outFile, "000000");	//if halt instruction, no ops
				
				tok = strtok(NULL, delim);
				
				if(tok != NULL)
				{
					if(isComment(tok))
						PrintComment(outFile, tok);
				}
				else
					fprintf(outFile, "\n");
					
				address++;
			}
			else if(addressInc = isDataDecl(tok))
			{			
				tok = strtok(NULL, delim);	//load a number
				
				if(tok != NULL)
				{
					//string support here
					
					if(!isStrBegin(tok) && hasTrailingComma(tok))
					{	
						do
						{
							StripTrailingComma(tok);
							PrintData(outFile,
								savedLine,
								tok, 
								&address,
								addressInc);
							fprintf(outFile, "\n");
							tok = strtok(NULL, delim);
						}
						while(hasTrailingComma(tok));
					}
					
					PrintData(outFile,
						savedLine,
						tok, 
						&address,
						addressInc);
						
		
					//write data 8 digits
					/*if(isHex(tok))
					{
						data = a2h(tok+2);
						fprintf(outFile, "\t%.8X", data);
					}
					else if(isDecimal(tok))
					{
						data = atoi(tok);
						fprintf(outFile, "\t%.8X", data);
					}
					else
					{
						printf("Error writing: invalid data value at line %d\n", line);
						exit(1);
					}
					*/
					
					tok = strtok(NULL, delim);		//write comment
					
					if(tok != NULL)
					{
						if(isComment(tok));
							PrintComment(outFile, tok);
					}
					else
						fprintf(outFile, "\n");
				}	
				
				//address+= addressInc;
			}
			else if(isLabel(tok))
			{
				tok = strtok(NULL, delim);	//don't verify
				
				if(tok != NULL)
				{
					if(addressInc = isDataDecl(tok))
					{
						
						tok = strtok(NULL, delim);
						
						if(tok != NULL)
						{
							//string support here
							if(!isStrBegin(tok) && hasTrailingComma(tok))
							{	
								do
								{
									StripTrailingComma(tok);
									PrintData(outFile,
										savedLine,
										tok, 
										&address,
										addressInc);
									fprintf(outFile, "\n");
									tok = strtok(NULL, delim);
								}
								while(hasTrailingComma(tok));
							}
							
							PrintData(outFile,
								savedLine,
								tok, 
								&address,
								addressInc);
						
							tok = strtok(NULL, delim);	
							
							if(tok != NULL)
							{
								if(isComment(tok))
									PrintComment(outFile, tok);
							}
							else
								fprintf(outFile, "\n");
						}
						//address += addressInc;
					}
					else if(isComment(tok))
						PrintComment(outFile, tok);
				}	
			}
		} //end if(tok != NULL)
		
		line++;
	} //end while 
} //end OutToCode
