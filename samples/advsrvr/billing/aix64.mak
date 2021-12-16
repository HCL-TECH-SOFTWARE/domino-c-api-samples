#
#  makefile for Notes API sample program billing
#                 AIX 64-bit version
#
#

# set TARGET to the name of the executable to create
TARGET1 = libbillmgr.a
TARGET2 = billses
TARGET = $(TARGET1) $(TARGET2)

# set SOURCES to the list of C source files in this program
SOURCES = billmngr.c $(TARGET2).c

# set HEADERS to the list of C include files in this program
HEADERS = bill_ext.h billses.h

# set OBJECTS to the list of object files that must be linked
OBJECTS1 = billmngr.o
OBJECTS2 = $(TARGET2).o

# Link this program with the bootstrap code notes0.o because
# this program is structured as a NotesMain.
BOOTOBJS = $(LOTUS)/notesapi/lib/aix64/notes0.o $(LOTUS)/notesapi/lib/aix64/notesai0.o

# CC defines the compiler.
CC = xlc_r


# set NOTESDIR to specify where to search for the Notes library file
NOTESDIR = $(Notes_ExecDirectory)

# Set LINKOPTS - the linker options passed to CC when linking.
# -o $(TARGET) causes compiler to create target rather than a.out

LINKOPTS1 = -o $(TARGET1) $(LINKOPTS) -bE:exports.aix -bmodtype:SRE \
					-bnoentry -q64
LINKOPTS2 = -o $(TARGET2) -qarch=ppc -I/usr/include -I/usr  -O \
					-qalias=noansi -qsuppress=0711-224:0711-345 -q64


# Set CCOPTS - the compiler options.
CCOPTS = -c -g -q64 \
   -qsuppress=1506-356:1500-010:1506-280  -c -qfullpath -qtbtable=full \
   -qstaticinline -qchars=unsigned -qsrcmsg -qthreaded -qcpluscmt \
   -qhalt=s -O -qalias=noansi -qarch=ppc -qMAXMEM=8192 \
   -qlargepage -qnoinline


# set NOTESDIR to specify where to search for the Notes library file
NOTESDIR = $(Notes_ExecDirectory)



# Notes API header files require UNIX to be defined.
DEFINES = -DUNIX -DAIX -DW -DW32 -DAIX64 -DND64 -D_ALL_SOURCE -DNDUNIX64 \
   -D_REENTRANT -DUSE_THREADSAFE_INTERFACES -DIBM_AIX \
   -DAIX_ATOMICS -DHAS_DLOPEN -DHAS_BOOL -DLARGE64_FILES \
   -D_LARGE_FILE_API -DNO_OVERLOAD_DEL_OPERATOR -DHANDLE_IS_32BITS \
   -DND64 -DNDUNIX64 -DPRODUCTION_VERSION -DOVERRIDEDEBUG
   

# set INCDIR to specify where to search for include files
INCDIR = $(LOTUS)/notesapi/include

# set LIBS to list all the libraries ld should link with.

LIBS1 = -L/usr/lib/threads -L/usr/lib -lC_r -lc_r -lpthreads -ltli_r -lnotes_r \
			-bnolibpath -lodm -lcfg -lm -lPW -lperfstat -bI:/usr/lib/pse.exp

LIBS2 = -lnotes_r -lm -L/usr/lib/threads \
   		-L/usr/lib -lC_r -lc_r -lpthreads -ltli_r

# the executable depends on the objects.
$(TARGET): $(OBJECTS1) $(OBJECTS2)
	$(CC) $(LINKOPTS1) $(OBJECTS1) -L$(NOTESDIR) $(LIBS1)
	$(CC) $(LINKOPTS2) $(OBJECTS2) $(BOOTOBJS) -L$(NOTESDIR) $(LIBS2)

# the object files depend on the corresponding source files
.c.o:
	$(CC) $(CCOPTS) $(DEFINES) -I$(INCDIR) $(SOURCES)
