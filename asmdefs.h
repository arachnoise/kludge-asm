//asmdefs.h
//
//useful assembler definitions

#ifndef ASMDEFS_H
#define ASMDEFS_H

#define numberOfInstr	10
#define numEscChars		9

//instruction types
#define instrAllArgs	0
#define instrConstArgs	1
#define instrNoConstArgs 2
#define instrNoArgs		3

//argument types
#define argUnset		0
#define constArg		1
#define	memArg			2
#define ptrArg			3
#define indArg			4

//miscellaneous
#define intSize			4
#define lineSize		1024
#define numberOfDataKw	3
#define numCharsPerInt	8
#define numOctalPerByte 3
#define maxCharVal		255
#define	maxSymbolLength	64
#define maxSymbols		256
#define maxArgLength	256		//I don't really know what the maximum
								//is yet. I'll put this here for now
#define maxProgLength	65535

extern const char delim[];
extern const char escChars[];
extern const char escCharMap[];
extern const char constDeclarationKeyword[];
extern const char dataDeclarationKeywords[][maxSymbolLength];
extern const char instructionMnemonics[][maxSymbolLength];
extern const int instructionTypeMap[];

typedef struct SymbolRecord
{
	char name[maxSymbolLength];
	int address;
	int isConstant;
	int lineNumber;
} SymbolRecord;


#endif