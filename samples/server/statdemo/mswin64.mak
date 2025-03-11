#
# makefile for Notes API sample program STATDEMO
# Windows 64-bit version using
# Microsoft Visual Studio 2010 SP1
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Windows 64-bit make definitions
!include <makeEnvWin.mak>

##
outfilename = statdemo

# Update the executable file if necessary, and if so, add the resource
# back in.
$(outfilename).exe: statdemo.obj statdemo.res
	link /INCREMENTAL:NO /NOLOGO /SUBSYSTEM:CONSOLE statdemo.obj $(NOTES_LIBS) \
		statdemo.res  $(LIBS) mswsock.lib -out:$(outfilename).exe

# Update the resource file if necessary.
statdemo.res : statdemo.rc statdemo.h
        rc -r -fo statdemo.res -DWIN32 -D_WIN32 /DNT statdemo.rc

# Update the object file if necessary.
statdemo.obj : statdemo.c statdemo.h
	!cl $(COPTIONS) statdemo.c
