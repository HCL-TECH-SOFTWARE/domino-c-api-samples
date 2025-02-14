Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
encrypt  -  Shows how to encrypt and decrypt data.


ENVIRONMENTS
------------
Windows (32-bit)
IBM i


FILES
-----------
readme.txt  -  This file specifies what is needed to use this example.

encrypt.c   -  Main program.

mswin32.mak     - Make file for Windows 32-bit.

mk_encrypt_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
                       
mk_encrypt_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.


RUNNING encrypt
---------------
This sample program will run against the Domino database encrypt.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy
this file to the Domino or Notes data directory before running this program.

Please note that you should have 2 users with valid fully canonicalized user names.
something like:
USER1 "CN=Jayne Doe/OU=Inside Sales/O=ABCorp"
USER2 "CN=John Doe/OU=Inside Sales/O=ABCorp"
The sample will encrypt a note with the public key of these users.
These user names has to be given at command line arguments for the encrypt sample.

Before running this sample, create a secret encryption key.  In the Notes
client, select File-Security-User Security.  Click Notes Data-Documents.
Click New Secret Key.  Enter EncKey1 for the secret key name.


- Under Windows:

  On the command line, type the full path and filename
  of this program.  You may also include the filename of the database
  and the name of the encryption key as command line arguments.  If
  no command line arguments are provided, the program will prompt for them.

- Under IBM i, type
    RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/ENCRYPT)
        PARM('encrypt.nsf' 'EncKey1')) BATCH(*NO)

EXAMPLE
-------
      encrypt  encrypt  EncKey1 <USER1> <USER2>

Upon executing this command, encrypt will create in the database
'encrypt.nsf' a document containing fields of several different types.
The field named "SECRET" will be encryption enabled but not encrypted.

A second document will be created.  This document will encrypt the
field named "SECRET" with the EncKey1 secret encryption key and with the
public keys of USER1 and USER2.

The note id's and the contents of the "SECRET"
field for each document created will then be printed.

The program will print out on the screen output similar to:

Note ID:  509E

The SECRET field is not encrypted.
Contents of SECRET field:

This is an encryption enabled field.  The data, however, is not encrypted.



Note ID:  50A2

The SECRET field has been decrypted.
Contents of SECRET field:

This is an encryption enabled field.  The data is encrypted.

CHECKING THE RESULTS
--------------------
Open the database 'encrypt.nsf'

Highlight the first document that was just created.

Choose File-Document Properties

Highlight the field named "secret" on the field tab.  This will
display the data type of the data in that field, the length of the
data, the actual data itself, and the field flags that are set for
that field.  See that "SEAL"  is one of the Field flags set.  This
signifies that encryption is enabled for that field.

Open the second document that was just created.  Notice the
message, "Decrypting Document..." is quickly displayed in the status
bar while the document is being opened.  Click on the status bar to see
this message.  Choose File-Document Properties... and notice that on the
fields tab, $SEAL, SecretEncryptionKeys, and PublicEncryptionKeys appear
in the Field listbox.

If another user who does not have the personal encryption key or
is not specified as USER1 or USER2 opens this same document, then this user
will not be able to read the contents of the encrypted field.

