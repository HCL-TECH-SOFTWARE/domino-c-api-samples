#
# makefile for Notes API sample program XVIEW
# Windows 32-bit version using Microsoft Visual Studio 2010
# compiler and linker.
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Standard Windows 32-bit make definitions
!include <ntwin32.mak>

!IF ([cl /? 2>&1 | findstr /C:"Version 16" > nul] == 0)
cpuflags = /Zp
!ENDIF   

outfilename = nxview
defname = mswin32

# Master dependency

all : $(outfilename).dll

# Link command.

$(outfilename).dll : nxview.obj nxview.res $(defname).def
    $(link) $(linkdebug) -base:0x1C000000 \
    	-dll -entry:_DllMainCRTStartup$(DLLENTRY) \
      -def:$(defname).def \
    	-out:$(outfilename).dll \
    	nxview.obj \
    	nxview.res \
		$(conlibs) $(guilibs) notes.lib

# Update the resource if necessary

nxview.res: mswin32.rc xview.h
    rc -v -r -fo nxview.res mswin32.rc

# Compilation command.

nxview.obj : xview.c xview.h
        $(cc) $(cdebug) $(cflags) $(cpuflags) /optimize -DW32 $(cvars) -Fonxview.obj xview.c
