#
# Makefile for Notes API sample program MAKEFORM 
# Windows 64-bit version using
# Microsoft Visual Studio 2010 SP1
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the NotesAPI and C "include" and "lib" directories.

# Standard Windows 64-bit make definitions
!include <ntwin32.mak>

# The name of the program.

PROGNAME = makeform

##
!IF ([cl /? 2>&1 | findstr /C:"Version 19" > nul] == 0)
RUNTIME_FLAG =  libvcruntime.lib ucrt.lib libcmt.lib  
!ELSEIF ([cl /? 2>&1 | findstr /C:"Version 16" > nul] == 0)
RUNTIME_FLAG = msvcrt.lib
!ENDIF
# Dependencies

$(PROGNAME).EXE: $(PROGNAME).OBJ cdrecord.obj
$(PROGNAME).OBJ: $(PROGNAME).C $(PROGNAME).H
cdrecord.obj   : cdrecord.c $(PROGNAME).H

# Compilation command.

.C.OBJ:
    !cl -nologo -c /MD /Zi /Ot /O2 /Ob2 /Oy- -Gd /Gy /GF /Gs4096 /GS- /favor:INTEL64 /EHsc /Zc:wchar_t- /Zc:forScope- -Zl -W3 -DNT -DW32 -DW -DW64 -DND64 -D_AMD64_ -DDTRACE -D_CRT_SECURE_NO_WARNINGS -DND64SERVER -DPRODUCTION_VERSION  -DDUMMY $*.c

# Link command.

.OBJ.EXE:
    link -nologo -opt:ref -machine:AMD64 -nodefaultlib /LARGEADDRESSAWARE -map:w64_cmp.mpx  -debug -debugtype:cv -out:$@ $** notes.lib \
        $(RUNTIME_FLAG) kernel32.lib user32.lib
       
       

