#
#  makefile for Notes API sample program intro
#               Linux for zSeries
#
#

# set TARGET to the name of the executable to create
TARGET = idvault

# set SOURCES to the list of C source files in this program
SOURCES = $(TARGET).c

# set HEADERS to the list of C include files in this program
HEADERS =

# set OBJECTS to the list of object files that must be linked
OBJECTS = $(TARGET).o

# CC defines the compiler.
CC = gcc

# Set CCOPTS - the compiler options.
CCOPTS = -c -m64
# Use -g flag for debugging:
#CCOPTS = -c -g

# set NOTESDIR to specify where to search for the Notes library file
NOTESDIR = $(Notes_ExecDirectory)

# Set LINKOPTS - the linker options passed to CC when linking.
# -o $(TARGET) causes compiler to create target rather than a.out
LINKOPTS = -o $(TARGET) -m64

# Notes API header files require UNIX to be defined.
DEFINES =  -DUNIX -DLINUX -DZLINUX -DND64 -DW32 -DLINUX64 -DW -DZLINUX64 -DDPOOL_BLOCK_OPT  -DDTRACE -DPTHREAD_KERNEL -D_REENTRANT -DUSE_THREADSAFE_INTERFACES -D_POSIX_THREAD_SAFE_FUNCTIONS  -DHANDLE_IS_32BITS -DHAS_IOCP -DHAS_BOOL -DHAS_DLOPEN -DUSE_PTHREAD_INTERFACES -DLARGE64_FILES -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -DTLOG_HARDWARE_COMPRESS -DNDUNIX64 -DLONGIS64BIT -DZSERIES64BIT  -DPRODUCTION_VERSION -DOVERRIDEDEBUG -fPIC

# set INCDIR to specify where to search for include files
INCDIR = $(LOTUS)/notesapi/include

# set LIBS to list all the libraries ld should link with.
LIBS = -lnotes -lm -lnsl -lc -lpthread -lresolv -ldl

# the executable depends on the objects.
$(TARGET): $(OBJECTS)
	$(CC) $(LINKOPTS) $(OBJECTS) -L$(NOTESDIR) -Wl,-rpath-link $(NOTESDIR) $(LIBS)

# the object files depend on the corresponding source files
.c.o:
	$(CC) $(CCOPTS) $(DEFINES) -I$(INCDIR) $(SOURCES)

