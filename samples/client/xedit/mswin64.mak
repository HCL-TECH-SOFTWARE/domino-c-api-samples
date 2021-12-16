#
# makefile for Notes API sample program XEDIT
# Windows 32-bit version using Microsoft Visual Studio 2010 compiler
# and linker.
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Standard Windows 32-bit make definitions
!include <win.mak>

#cpuflags = -Zp
outfilename = nxedit
defname = mswin64

# Master dependency

all : $(outfilename).dll

# Link command.

$(outfilename).dll : nxedit.obj nxedit.res $(defname).def
    $(link) $(linkdebug) -base:0x1C000000 \
    	-dll -entry:_DllMainCRTStartup$(DLLENTRY) \
      -def:$(defname).def \
    	-out:$(outfilename).dll \
    	nxedit.obj \
    	nxedit.res \
		$(conlibs) $(guilibs) notes.lib

# Update the resource if necessary

nxedit.res: mswin64.rc xedit.h
    rc -v -r -fo nxedit.res mswin64.rc

# Compilation command.

nxedit.obj : xedit.c xedit.h
        $(cc) $(cdebug) $(cflags) $(cpuflags) /optimize -DW32 $(cvars) -Fonxedit.obj xedit.c
