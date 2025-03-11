#
# makefile for Notes API sample program EVENTS
# Windows 64-bit version using
# Microsoft Visual Studio 2010 SP1
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Windows 64-bit make definitions
!include <makeEnvWin.mak>

PROGNAME1 = consume
PROGNAME2 = produce
outfilename1 = $(PROGNAME1)
outfilename2 = $(PROGNAME2)

##

all: $(outfilename1).exe $(outfilename2).exe

# Update the executable file if necessary, and if so, add the resource
# back in.
$(outfilename1).exe: $(PROGNAME1).obj $(PROGNAME1).res
	link /INCREMENTAL:NO /NOLOGO /SUBSYSTEM:CONSOLE $(PROGNAME1).obj notes0.obj notesai0.obj \
	$(PROGNAME1).res $(LIBS) msvcrt.lib OLDNAMES.lib notes.lib user32.lib -out:$(outfilename1).exe

# Update the resource file if necessary.
$(PROGNAME1).res : $(PROGNAME1).rc $(PROGNAME1).h
        rc -r -fo $(PROGNAME1).res -DWIN32 -D_WIN32 /DNT $(PROGNAME1).rc

# Update the object file if necessary.
$(PROGNAME1).obj : $(PROGNAME1).c $(PROGNAME1).h
	!cl $(COPTIONS)   $(PROGNAME1).c

# Update the executable file if necessary, and if so, add the resource
# back in.
$(outfilename2).exe: $(PROGNAME2).obj $(PROGNAME2).res
	link /INCREMENTAL:NO /NOLOGO /SUBSYSTEM:CONSOLE $(PROGNAME2).obj notes0.obj notesai0.obj \
	$(PROGNAME2).res $(LIBS) msvcrt.lib OLDNAMES.lib notes.lib user32.lib -out:$(outfilename2).exe

# Update the resource file if necessary.
$(PROGNAME2).res : $(PROGNAME2).rc $(PROGNAME2).h
        rc -r -fo $(PROGNAME2).res -DWIN32 -D_WIN32 /DNT $(PROGNAME2).rc

# Update the object file if necessary.
$(PROGNAME2).obj : $(PROGNAME2).c $(PROGNAME2).h
	!cl $(COPTIONS)  $(PROGNAME2).c

