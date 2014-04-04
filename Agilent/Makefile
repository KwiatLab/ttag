#The name of the program. A .a or .so extension compiles as a static or shared library.
NAME:=AgilentBuffer

#If compiling a shared library, passed on to linker in the form $(NAME).$(ABIVERSION)
ABIVERSION:=1

#Linker line - add libraries that need to be referenced here
LDFLAGS:=-O3 -lrt -lboost_thread -L./ -L../libttag -lttag -lAqDrv4

#Enable warnings and choose things here
CFLAGS:=-Wall -D_LINUX -D_ACQIRIS -I./include -I../libttag/src
#-ansi -pedantic

#The compiler to be used
CC:=g++
#The standard library. Can be blank for default
STDLIB= #-std=c++0x     #Uses the c++11 stdlib

############################################################################################################

#Get the type of file to create
TYPE:=$(suffix $(NAME))

#When making a dynamic library, the soname has to be set along with an ABI version to pass to the linker
SONAME:=$(NAME).$(ABIVERSION)

#Set the necessary variables for a shared library
ifeq (.so,$(TYPE))
	CFLAGS+= -fPIC
	LDFLAGS:= -shared -Wl,-soname,$(SONAME) $(LDFLAGS)
endif

#Get the list of files from the src directory
SRC=$(wildcard src/*.c) $(wildcard src/*.cpp)
#If you don't want to compile all files in the folder, just give it a list
#SRC:=

#Get the list of objects from the file list
OBJ=$(patsubst src/%,bin/%.o,$(SRC))

#Add the chosen standard library to the CFLAGS
CFLAGS:= $(CFLAGS) $(STDLIB)

.PHONY: clean all dependencies
all: $(NAME)

#A static library uses ar rather than $(CC) to create the lib.
$(NAME): $(OBJ)
ifeq (.a,$(TYPE))
	ar rcs $@ $^
else
	$(CC) $^ -o $@ $(LDFLAGS)
endif

bin/%.o: src/% bin dependencies
	$(CC) -c $< -o $@ $(CFLAGS)
bin:
	mkdir bin
clean:
	rm -rf bin $(NAME)

############################################################################################################
#Dependencies of the project
############################################################################################################

dependencies:
	cd ../libttag;make


