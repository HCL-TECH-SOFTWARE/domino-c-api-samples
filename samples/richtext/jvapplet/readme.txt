Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
jvapplet - Sample HCL C API for Domino and Notes Program. Shows how to
embed a Java Applet in a rich text field in a document in the embedole.nsf
database.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux 64 bit
IBM i


FILES
-----
readme.txt             - This file specifies what is needed to use
                         this example.

jvapplet.c             - Main program.

IntroLocalApplet.java  - Java Applet source file.

IntroLocalApplet.class - Java Applet class file.

mswin32.mak     - Makefile for Windows 32-bit.

mswin64.mak     - Makefile for Windows 64-bit.


linux.mak              - Makefile for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_jvapplet_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_jvapplet_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

(Note: If you use iSeries ILE C compiler with the source code being 
 in a source physical file, the file IntroLocalApplet.class is stored 
 as IFS file: /qntcsdk/notedata/IntroLocalApplet.class.) 


BEFORE RUNNING THIS SAMPLE:
---------------------------
1. Enable Java and back-end class accessing in your Notes client.


RUNNING jvapplet:
-----------------
- Before compiling the program, ensure the variable "CodeStringFullPath"
in the jvapplet.c program reflects the fully-qualified path for the
IntroLocalApplet.class file.

- Run this sample program with the embedole.nsf Domino database
provided with the API toolkit.  You must copy the embedole.nsf file
to the Domino or Notes data directory before running this program.

- After the program is successfully compiled, at the system prompt type:

        jvapplet embedole
- Under IBM i:

  Copy the embedole.nsf file and the IntroLocalApplet.class file to the 
  Domino data directory; specify "CodeStringFullPath" in the jvapplet.c
  program to be the fully-qualified path for the IntroLocalApplet.class
  file: "<the server data directory/IntroLocalApplet.class" and then 
  type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/jvapplet)
        parm(embedole)) batch(*no)


CHECKING THE RESULT
-------------------
After running the jvapplet program, from Notes UI:

- Select File - Preferences - User Preferences, be sure the
"Enable Java applets" option is checked.

- Select File - Security - User Security....  After supplying your ID's
password, select What Others Do - Using Applets.  Highlight the user
name used when creating the applet, under the When applet is signed by:
title, check all the boxes under the Allow: title.  This will give you
enough privilege to avoid the access privilege warnings when executing
the applet as described at the next step.

- Open the database embedole.nsf and the document just created.  This
should trigger the execution of the Applet in the document, and it
should display information similar to the following:


Title of the current database is
   EMBEDOLE test database

(This Java Applet is inserted in this document by the jvapplet C API sample.)


