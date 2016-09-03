//util.h
//
//data checking and string handling functions

#ifndef UTIL_H
#define UTIL_H

//string checking functions
//definitions in validators.c

int hasTrailingComma(char *str);

int isInstruction(char *str);
int isConstArg(char *str);
int isIndArg(char *str);
int isMemArg(char *str, int line);
int isPtrArg(char *str, int line);

int isConstDecl(char *str);
int isDataDecl(char *str);

int isLabel(char *str);
int isAddConst(char *str);
int isDecimal(char *str);
int isHex(char *str);

int isComment(char *str);
int isStrBegin(char *str);
int isString(char **tok);

int EscCharCheck(char **seq, int line);
int InstructionCheck(char *instr, int line); //unused so far... 
											 //do something about it
int DataDeclarationCheck(char *dataDecl, int line);
int LabelCheck(char* label, int line);

//printing functions
//definitions in printers.c

void PrintComment(FILE *toWrite, char *str);
void PrintEscSeq(char *str);
void PrintData(FILE *toWrite,
	const char* savBuf, 
	char *str, 
	int *baseAddress,
	int size);
void PrintSanitized(char *str);

//manipulator functions
//definitions in manipulators.c

int GetEscCharData(char **seq);
void StripTrailingComma(char *str);
void StripBrackets(char *str);
void StripIndexSuffix(char *str);

int a2h(char* str);
int a2o(char *str);

#endif