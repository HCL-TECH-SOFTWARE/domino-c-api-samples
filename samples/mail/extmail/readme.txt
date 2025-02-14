Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
extmail - Example Extension Manager

This sample program will register the Extension Manager hook EM_MAILSENDNOTE.

When the program traps an EM_MAILSENDNOTE hook that is triggered when a
mail message has been composed and sent by the Notes UI, the program then
processes the hook's notification type.  If the notification type is EM_BEFORE
(before the function is called), and the mail message subject text is
"Extension Manager", the mail message body text is modified notifying the user
that the Extension Manager has intercepted the mail.

If the notification type is EM_AFTER (after the function is called), a new mail
message is created with user mail information and an attached Extension Manager
log file.  This mail message is sent back to the user.

ENVIRONMENTS
------------
Windows (32-bit)


FILES
-----
extmail.c    - Main C source code file for Extension Manager
extmail.h    - Constants and function prototypes.

mswin32.mak     - Make file for Windows 32-bit.
mswin32.def  - Module definition file for Windows 32-bit.

readme.txt   - This file specifies what is needed to use this example.


OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
To install and run this program:

    1.  Exit Notes if it is running.

    2.  Copy NEXTMAIL.DLL to the Notes program directory.

    3.  Edit the notes.ini file.
        If an EXTMGR_ADDINS entry does not exist,

        EXTMGR_ADDINS=EXTMAIL

        Save the modified notes.ini file.

        If an EXTMGR_ADDINS entry already exists in notes.ini, add
        ",EXTMAIL" to the end of the existing line. The one-character
        prefix in the extension manager program name is optional.

    4.  Launch Notes.

    5.  From a Notes client, open the mail database for the logged in user.

    6.  Create a new memo and enter the logged in user name in the "To:" field.
        In the "Subject:" field enter "Extension Manager" and then enter some
        text in the body of the message.  Send the message.


CHECKING THE RESULTS
--------------------

    When you recieve the message sent to the user logged in, verify the
    Extension Manager has appended the following information to the mail
    message body:

    *** This mail message has been intercepted by the EXTENSION MANAGER! ***
    You will be receiving a message report shortly.

    You should then receive a message from the Extension Manager that has user
    information and an attached log file of the Extension Manager processing.

    Exit Notes to deregister the Extension Manager hook.
