Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
schedule - Allows the user to add appointments, meeting invitations,
           delete the scheduled events or get the busy/free time
           information from their schedule.
           Note that to simplify the "adding meeting invitations"
           process, this sample only demonstrate the procedure of
           creating a meeting invitation, without sending the notice
           to the invitees.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64bit


FILES
-----
readme.txt   - This file specifies how to use this example.

cdrecord.c   - The sub-program creating CD records.

schedule.c   - The main program.

sched.h      - header file.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_schedule_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_schedule_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.
                   

RUNNING schedule:
-----------------

Note: Entering a date/time in the past is not supported by the API and
      may return inaccurate results.

- Under Windows, type
  the following at the system command prompt:

  schedule <command> <server> <mail file> <start time> <end time>
             or
  schedule
  
- Under IBM i, type
    RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/SCHEDULE)
    PARM(<command> <server> <mail file> <start time> <end time>))
    BATCH(*NO)
        or
    RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/SCHEDULE)) 
    BATCH(*NO)

  where:
        <command>    is one of the following schedule command to perform
                        add-i : add an invitation
                        add-a : add an appointment
                        del   : delete a schedule
                        busy  : check busy schedule
        <server>     is the server name the user's mail database is on
                     (ex. zathras).
        <mail file>  is the mail file name of the user (ex. mail\jdoe.nsf)
        <start time> is the start time of the range of time that you are
                     interested in (ex. 08/16/2005 02:00 pm")
                     Note: you must enter any single digit date/time
                     with a leading zero.
        <end time>   is the end time of the range of time that you are
                     interested in (ex. 08/16/2005 05:00 pm")
                     Note: you must enter any single digit date/time
                     with a leading zero.

  If no command line arguments are provided, the program will prompt for
  them.



EXAMPLE - ADD an appointment
----------------------------
  schedule add-a zathras mail\jdoe.nsf "08/16/2005 09:00 am" "08/16/2005 10:00 am"

- Under IBM i, type
    RUNDOMCMD SERVER(ctoolkit) CMD(CALL PGM(QNTCSDK/SCHEDULE)
        PARM('add-a' 'ctoolkit' 'mail/admin.nsf' '07/26/2008 09:00 am' '07/26/2008 10:00 am'))
        BATCH(*NO)
    
  Note: Stay within a day's boundary for the add-a command.


EXAMPLE - ADD a meeting invitation
----------------------------------
  schedule add-i zathras mail\jdoe.nsf "08/16/2005 10:00 am" "08/16/2005 11:00 am"
  
- Under IBM i, type
    RUNDOMCMD SERVER(ctoolkit) CMD(CALL PGM(QNTCSDK/SCHEDULE)
        PARM('add-i' 'ctoolkit' 'mail/admin.nsf' '07/26/2008 10:00 am' '07/26/2008 11:00 am'))
        BATCH(*NO)
  Note: Stay within a day's boundary for the add-i command.


EXAMPLE - DELETE scheduled entries
----------------------------------
  schedule del zathras mail\jdoe.nsf "08/16/2005 09:00 am" "08/16/2005 10:00 pm"

- Under IBM i, type
    RUNDOMCMD SERVER(ctoolkit) CMD(CALL PGM(QNTCSDK/SCHEDULE)
        PARM('del' 'ctoolkit' 'mail/admin.nsf' '07/26/2008 09:00 am' '07/26/2008 10:00 am'))
        BATCH(*NO)
  Note: End time is not used in finding the schedule entry but must be
        supplied.


EXAMPLE - Get BUSY/FREE time information
----------------------------------------
  schedule busy zathras mail\jdoe.nsf "08/16/2005 09:00 am" "08/16/2005 05:00 pm"

- Under IBM i, type
    RUNDOMCMD SERVER(ctoolkit) CMD(CALL PGM(QNTCSDK/SCHEDULE)
        PARM('busy' 'ctoolkit' 'mail/admin.nsf' '07/26/2008 09:00 am' '07/26/2008 05:00 pm'))
        BATCH(*NO)
  Note: You must specify a time range that is at least 60 minutes.
        You also may specify a time that expands to the next day.


CHECKING THE RESULTS
--------------------
This program must be run against a mail file that uses a 4.5 or higher mail
template and resides on a 4.5 or higher HCL Domino Server.

- Verify Adding an appointment:

From Notes, open your calendar and confirm that there is an appointment for
the date/time specified.


- Verify Adding a meeting invitation:

From Notes, open your calendar and confirm that there is a meeting
invitation for the date/time specified.


- Verify Deleting scheduled entries:

From Notes, open your calendar and confirm that the scheduled entries for
the date/time specified has been deleted.


- Verify Busy/Free time:

This program will print out the first free time and all the busy times
during the time periods you specified.

From Notes, open your calendar and look at the dates you typed in. Confirm
that the program returned the correct information about your schedule for
the days you typed in.

You can select daily allowable free time in your Calendar Profile:
If you have a R5 mail template:
   click the menu item Actions->Tools->Preferences, then Free Time tag.
If you have a R4 mail template:
   click the menu item Actions->Calendar Tools.

