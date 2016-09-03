//asmdefs.c
//
//useful assembler definitions

#include "asmdefs.h"

//For token processing
const char delim[] = " \t\n";

const char escChars[] = 
{
	'n', 't', 'b', 'r', 'f', '\\', '\'', '\"', '\0'
};

const char escCharMap[] = 
{
	'\n', '\t', '\b', '\r', '\f', '\\', '\'', '\"', '\0'
};

//keywords and instructions

const char constDeclarationKeyword[] = "equ";
const char dataDeclarationKeywords[][maxSymbolLength] = 
{
	"dword", "qword", "para"
};

const char instructionMnemonics[][maxSymbolLength] = 
{
	"load", "sto", "lir", "jmp", "jnz", "add", "shl", "halt", "in", "out"
};

const int instructionTypeMap[] =
{
	instrAllArgs,			//load
	instrNoConstArgs,		//sto
	instrAllArgs,			//lir
	instrConstArgs,			//jmp
	instrConstArgs,			//jnz
	instrAllArgs,			//add
	instrAllArgs,			//shl
	instrNoArgs,			//halt
	instrConstArgs,			//in
	instrConstArgs			//out
};