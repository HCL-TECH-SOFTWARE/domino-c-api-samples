Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
alarm - Processes an Alarm Message received from the Alarm Daemon.


PURPOSE
-------
This program demonstrates the use of the Alarm APIs.  The client application
registers with the Alarm Daemon and receives an Alarm Message through the
Alarm Message Queue (the alarm was previously set through the Notes UI in
the users Mail database).  The Alarm information is then displayed to the
screen.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
alarm.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.


SETTING AN ALARM IN THE USER'S MAIL DATABASE
--------------------------------------------
To set an alarm in your mail database, follow these steps:

1) Make sure your Mail Database has Calendar and Scheduling functionality
(release 4.5 mail template or later) and resides on a 4.5 (or later)
HCL Domino Server.

2) Open your Mail database.

3) Make sure the alarms option has been enabled.  To enable alarms from
within the Notes client:

   for R4.x mail template:
      - select Actions - Calendar Tools - Calendar Profile, and check the
        "Enable alarm notifications" checkbox.

   for R5.0 mail template:
      - select Actions - Tools - Preferences - Calendar tab - Alarms tab,
        and check the "Enable alarm notifications" checkbox.

   for Notes/Domino 6 mail template:
      - select Actions - Tools - Preferences - Calendar tab - Alarms tab,
        and check the "Enable Alarms" checkbox.  Check the Appointments/
        Meetings box and change default to 5 minute(s) in advance.

   Or, goto File - Preferences - Calendar and To Do - Alarms tab,
       and check "Display alarm notifications" checkbox.  Check the Appointments/
        Meetings box and change default to 5 minute(s) in advance.

4) Select the Calendar view, and create an appointment entry.  Set the
appointment's starting time to 10 minutes from the current time.
Or, open the mail in the Notes Client and click on 'More' tab and schedule
a meeting with starting time to 10 minutes from the current time.

5) for R4.x mail template:
      Click the Alarm Options, and set the Alarm time to 5 minutes before
      the appointment's starting time.

   for R5.0 mail template:
      Click the "Options" tab, check the "Notify me" box, then
      the "Alarm settings..."  box.  From the "Alarm Options" screen,
      set the Alarm time to 5 minutes before the appointment's starting
      time.

   for Notes/Domino 6 mail template:
      - Click the Alarm On option.  When the Alarm Options screen is
        displayed, accept the default time set in Step 3 : the alarm to go
        off 5 minutes before the appointment's starting time.

6) Save the Calendar entry.

7) Exit the Notes client.

8) Run the alarm program as follows and wait for the alarm message.



RUNNING alarm:
-----------------

- Under Windows,
  type the following at the system command prompt:

    alarm


CHECKING THE RESULTS
--------------------
This program will receive an alarm message and print out the alarm
information to the screen.
