Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
summary - Find and print all summary data of each data note in a database
          without opening the notes. Works on any database.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
Linux for zSeries
MAC OS X
IBM i


FILES
-----
summary.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_summary_i.clp  - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_summary_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 

readme.txt   - This file specifies what is needed to use this example.


RUNNING summary
---------------
You may run this program against any Domino database.

- Under Windows or UNIX (AIX, Linux ),
  type the following at the system command prompt:
  
        summary <database filename>
                 or
        summary

  where:

  <database filename> is the filename of the Domino database to operate on.

  If the database filename is not specified, the program will prompt for it.



EXAMPLE
-------

        summary    summary.nsf
- Under IBM i, type  
        rundomcmd server(<servername>) cmd(call pgm(qntcsdk/summary) 
		parm(summary.nsf)) batch(*no)

The program should print output that is similar to:

Note ID is: 0x000020fa.
        Item Name = Form
        Data Type = TYPE_TEXT
        Item Value = "SimpleDataForm"

        Item Name = plain_text
        Data Type = TYPE_TEXT
        Item Value = "This is a line of simple text."

        Item Name = number
        Data Type = TYPE_NUMBER
        Item Value = "0.000987"

        Item Name = time_date
        Data Type = TYPE_TIME
        Item Value = "01/15/88"

        Item Name = text_list
        Data Type = TYPE_TEXT_LIST
        Item Value = "one,two,three,four"



Note ID is: 0x000020fe.
        Item Name = Form
        Data Type = TYPE_TEXT
        Item Value = "SimpleDataForm"

        Item Name = plain_text
        Data Type = TYPE_TEXT
        Item Value = "This is a sentence"

        Item Name = number
        Data Type = TYPE_NUMBER
        Item Value = "836.000000"

        Item Name = time_date
        Data Type = TYPE_TIME
        Item Value = "04/20/86"

        Item Name = text_list
        Data Type = TYPE_TEXT_LIST
        Item Value = "a,b,c,d,e,f,g"



Note ID is: 0x00002102.
        Item Name = Form
        Data Type = TYPE_TEXT
        Item Value = "SimpleDataForm"

        Item Name = plain_text
        Data Type = TYPE_TEXT
        Item Value = "This is another line of simple text."

        Item Name = number
        Data Type = TYPE_NUMBER
        Item Value = "836.000000"

        Item Name = time_date
        Data Type = TYPE_TIME
        Item Value = "07/04/53 07:12:00 AM"

        Item Name = text_list
        Data Type = TYPE_TEXT_LIST
        Item Value = "a,b,c,d,e"



Note ID is: 0x00002106.
        Item Name = Form
        Data Type = TYPE_TEXT
        Item Value = "SimpleDataForm"

        Item Name = plain_text
        Data Type = TYPE_TEXT
        Item Value = "This is a line of plain text."

        Item Name = number
        Data Type = TYPE_NUMBER
        Item Value = "836.000000"

        Item Name = time_date
        Data Type = TYPE_TIME
        Item Value = "07/09/88"

        Item Name = text_list
        Data Type = TYPE_TEXT_LIST
        Item Value = "xxx,yyy,zzz"



Program completed successfully.

