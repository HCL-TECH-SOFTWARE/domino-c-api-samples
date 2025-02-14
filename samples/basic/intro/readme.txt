Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
intro - Introductory HCL C API for Domino and Notes program that opens
        a local or remote Domino database and displays the database title.


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
IBM AIX
IBM i
Linux 64 bit
Linux for zSeries
MAC OS X


FILES
-----
intro.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


linux64.mak    - Makefile for Linux 64 bit.

mk_intro_i.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_intro_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


IDENTIFY A TEST DATABASE:
-------------------------

Determine the name of a Domino database file, such as intro.nsf, that
resides in a local or remote Domino or Notes data directory.
intro will work with any valid Domino database file.


RUNNING intro:
-------------

The command line of intro is  -

    intro [server name - optional] <database filename>

where:

    [server name - optional] is the name of a server where the database
                         resides. 
                         Required for remote access, 
                         omit it for local access. 

    <database filename> is the filename of the Domino database to read


EXAMPLE:

Under Windows, or UNIX (AIX or Linux),
for local access type:

    intro  intro.nsf

for remote access type:

    intro  valid_server_name intro.nsf


Under IBM i,
for local access type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/intro)
    parm(intro.nsf)) batch(*no)
for remote access type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/intro)
    parm(valid_server_name intro.nsf)) batch(*no)
If the program runs correctly, it should display a message with the
HCL C API for Domino and Notes release number followed by:

    The database is local with path : test.nsf

    The title for the database, intro.nsf, is:

    API Test Database (intro)


EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

To see if 'intro' worked correctly:

    Use Notes to open intro.nsf

    Choose File/Database/Properties...

    You should see the same database title as displayed by this sample.



