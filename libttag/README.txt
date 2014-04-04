To start off, you probably want to look at the documentation located in ../Documentation/TimeTagger.pdf
Functinality of libttag is also documented in a less friendly mode in the main header file: ./src/ttag.h

You will probably want to compile the tests to make sure the library is working on your system.
On linux, running make in ./test shoud compile the "libttag_test" executable, which can then be run.
Look at the makefile to see dependencies.
On windows, MSVC project is provided in ./MSVC/libTTag/libTTag.sln

Refer to the documentation if you have any trouble.

