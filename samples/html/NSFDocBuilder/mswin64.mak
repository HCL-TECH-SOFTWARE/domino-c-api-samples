#
# makefile for Notes API sample program ConvertHtmlToNSF
# Windows 64-bit version using
# Microsoft Visual Studio 2010 SP1
#
# This makefile assumes that the INCLUDE and LIB environment variables
# are set up to point at the Notes and C "include" and "lib" directories.

# Windows 64-bit make definitions
!include <makeEnvWin.mak>

# The name of the program.

PROGNAME = ConvertHtmlToNSF

##

# Dependencies

$(PROGNAME).EXE: $(PROGNAME).OBJ CDRecord.OBJ
$(PROGNAME).OBJ: $(PROGNAME).C
CDRecord.OBJ: CDRecord.C

# Compilation command.

.C.OBJ:
    !cl $(COPTIONS)  $*.c

# Link command.

.OBJ.EXE:
    link $(LOPTIONS_1) -out:$@ $** $(ENTRY_FLAG1) $(LIBS_1) OLDNAMES.lib
