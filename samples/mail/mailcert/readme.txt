Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
    PROGRAM:    mailcert

PURPOSE
-------
    This program demonstrates the SECGetSignerInfoFromMail function, 
    get certificate from the s-MIME mail documents


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i


FILES
-----
readme.txt         - This file specifies detailed information to use this example.
mail_cert.c	   - Demostrate how to get sign info from S-MIME mail
mswin32.mak	   - make file which will be used to build c file on windows 32-bit.
mswin64.mak	   - make file which will be used to build c file on windows 64-bit.
linux64.mak	   - make file which will be used to build c file on linux   64-bit.
mk_mailcert_i.clp  - Make file (CL Program) for IBM i using ILE C
                    with the source code being an IFS file.
mk_mailcert_p.clp  - Make file (CL Program) for IBM i using ILE C
                    with the source code being in a source physical file.



About this sample:
--------------------
- In order to test SECGetSignerInfoFromMail, you need to have an SMIME document in a Notes 
database. And then opens the document and calls the SECGetSignerInfoFromMail function.  
Be sure to provide a callback function in order to verify *pCert. 

- The two key areas that will take a little bit of research on your part is how to create or get
the SMIME document (an internet mail document with an x.509 certificate) and how to verify *pCert 
that is given to you in your callback function.  You may use the file SignedMail.nsf, which already has 
an SMIME message that you can use.  Or you can create an SMIME message yourself- you will need to 
create an Internet certificate and have Notes use MIME. Please refer to notes help documents.

- To run this sample, you can copy the file SignedMail.nsf from the notesdata directory, and then run 
command as mailcert SignedMail.nsf or you can just specify the detailed path of the SignedMail.nsf, such 
as mailcert c:\notesapi\notesdata\SignedMail.nsf.

Verify the result:
------------------
The sample runs successfully if you get result as below:

        Inserted note 90A into table.
        Inserted note 922 into table.
        Processing note 90A.
this mail was signed

No of entries[3]
        Processing note 90A.
This mail was signed

SUBJECT:Your certificate request has been approved
This is Inet signed certificate
  SECGetSignerInfoFromMail had no errors.

        Processing note 922.
This mail was signed

SUBJECT:sign from OE
This is Inet signed certificate
  Inside VerifyCertCallBack...
  Internet certificate found - CertSize = 751

  SECGetSignerInfoFromMail had no errors.

This mail was signed

Program completed successfully.
