# Kludge assembler makefile

CC = gcc
DEBUG_FLAG =
OBJS = kluasm.o asmdefs.o manipulators.o printers.o validators.o
EXE_NAME = kluasm.exe

kluasm: $(OBJS)
	$(CC) $(DEBUG_FLAG) -o $(EXE_NAME) $(OBJS)

kluasm.o: kluasm.c stringutils/util.h asmdefs.h asmdefs.c
	$(CC) $(DEBUG_FLAG) -c kluasm.c
	
manipulators.o: stringutils/manipulators.c stringutils/util.h asmdefs.h asmdefs.c
	$(CC) $(DEBUG_FLAG) -c stringutils/manipulators.c

printers.o: stringutils/printers.c stringutils/util.h asmdefs.h asmdefs.c
	$(CC) $(DEBUG_FLAG) -c stringutils/printers.c

validators.o: stringutils/validators.o stringutils/util.h asmdefs.h asmdefs.c
	$(CC) $(DEBUG_FLAG) -c stringutils/validators.c
	
asmdefs.o: asmdefs.c asmdefs.h
	$(CC) $(DEBUG_FLAG) -c asmdefs.c

.PHONY : clean
clean:
	rm $(EXE_NAME) $(OBJS)