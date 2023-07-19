#
#  makefile for Notes API sample program intro
#                 AIX 64-bit version
#
#

# set TARGET to the name of the executable to create
TARGET = idvault

# set SOURCES to the list of C source files in this program
SOURCES = $(TARGET).c


# set OBJECTS to the list of object files that must be linked
OBJECTS = $(TARGET).o


# CC defines the compiler.
CC = xlc_r

# Set CCOPTS - the compiler options.
CCOPTS = -c -g -q64 \
   -qsuppress=1506-356:1500-010:1506-280  -c -qfullpath -qtbtable=full \
   -qstaticinline -qchars=unsigned -qsrcmsg -qthreaded -qcpluscmt \
   -qhalt=s -O -qalias=noansi -qarch=ppc -qMAXMEM=8192 \
   -qlargepage -qnoinline


# set NOTESDIR to specify where to search for the Notes library file
NOTESDIR = $(Notes_ExecDirectory)

# Set LINKOPTS - the linker options passed to CC when linking.
# -o $(TARGET) causes compiler to create target rather than a.out

LINKOPTS = -o $(TARGET) -q64\
   -qarch=ppc -I/usr/include -I/usr  -O -qalias=noansi \
   -qsuppress=0711-224:0711-345


# Notes API header files require UNIX to be defined.
DEFINES = -DUNIX -DAIX -DW -DW32 -DAIX64 -DND64 -D_ALL_SOURCE -DNDUNIX64 \
   -D_REENTRANT -DUSE_THREADSAFE_INTERFACES -DIBM_AIX \
   -DAIX_ATOMICS -DHAS_DLOPEN -DHAS_BOOL -DLARGE64_FILES \
   -D_LARGE_FILE_API -DNO_OVERLOAD_DEL_OPERATOR -DHANDLE_IS_32BITS \
   -DND64 -DNDUNIX64 -DPRODUCTION_VERSION -DOVERRIDEDEBUG

# set INCDIR to specify where to search for include files
INCDIR = $(LOTUS)/notesapi/include

# set LIBS to list all the libraries ld should link with.

LIBS = -lnotes_r -lm -L/usr/lib/threads \
   -L/usr/lib -lC_r -lc_r -lpthreads -ltli_r

# the executable depends on the objects.
$(TARGET): $(OBJECTS)
	$(CC) $(LINKOPTS) $(OBJECTS) -L$(NOTESDIR) $(LIBS)

# the object files depend on the corresponding source files
.c.o:
	$(CC) $(CCOPTS) $(DEFINES) -I$(INCDIR) $(SOURCES)
