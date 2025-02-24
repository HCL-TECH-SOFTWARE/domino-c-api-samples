#
#  makefile for Notes API sample program secdom
#                Linux RedHat
#  NOTE: set NOTES_CAPI env before executing makefile
#

#make env file
include ../../../makeEnvLinux.mak

# set TARGET to the name of the executable to create
TARGET = libsecdom.so

# set SOURCES to the list of C source files in this program
SOURCES = secdom.c u_secdom.c

# set HEADERS to the list of C include files in this program
HEADERS =

# set OBJECTS to the list of object files that must be linked
OBJECTS = secdom.o u_secdom.o

# Set LINKOPTS - the linker options passed to CC when linking.
# -shared to produce a shared object which can then be linked with
# other objects to form an executable. This is necessary for creating the .so
LINKOPTS = -Wl,--no-whole-archive -L/usr/local/lib64 -L/usr/lib64 -L/lib64 -ldl -lrt -lm -lstdc++ -L/lib64 -Lcrypt -lpthread -lc -lresolv -shared

# the executable depends on the objects.
$(TARGET): $(OBJECTS)
	$(CC) $(LINKOPTS) $(OBJECTS) $(LIBNOTESSO) -o $(TARGET)

# the object files depend on the corresponding source files
.c.o:
	$(CC) $(CCOPTS) $(DEFINES) $(INCDIR) $(SOURCES)

