The files in this folder are the matlab interface to the timetagger buffer library
Each of these files corresponds to a different matlab function.

Unfortunately, the default matlab compiler fails to compile these. Setting matlab to use MSVC should work.
MSVC fails at recognizing certain parts of C99, so the linker option /TP is added to compile as C++

The compiling needs all the source files from libttag, meaning that to get the matlab functions you need to
do the following commands:

mex tt_start.c -v ../libttag/src/buffer.c
mex tt_stop.c -v ../libttag/src/buffer.c
mex tt_resolution.c -v ../libttag/src/buffer.c
mex tt_channels.c -v ../libttag/src/buffer.c
mex tt_dump.c -v ../libttag/src/buffer.c
mex tt_rawdump.c -v ../libttag/src/buffer.c
mex tt_timedump.c -v ../libttag/src/buffer.c
mex tt_singles.c -v COMPFLAGS="$COMPFLAGS /TP" ../libttag/src/buffer.c ../libttag/src/analysis.c
mex tt_coincidences.c -v COMPFLAGS="$COMPFLAGS /TP" ../libttag/src/buffer.c ../libttag/src/analysis.c
mex tt_multicoincidences.c -v COMPFLAGS="$COMPFLAGS /TP" ../libttag/src/buffer.c ../libttag/src/analysis.c
mex tt_correlate.c -v COMPFLAGS="$COMPFLAGS /TP" ../libttag/src/buffer.c ../libttag/src/analysis.c

Old files (no longer work):

mex tt_delays.c ../libttag/src/buffer.c ../libttag/src/analysis.c ../libttag/src/stats.c
mex tt_stats.c ../libttag/src/buffer.c ../libttag/src/analysis.c ../libttag/src/stats.c

On Linux you need to add -lrt, -lm and you also need to mess with matlab's configuration file so that
it does not have -pedantic, which completely makes the compiler explode..
