PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/FILEATT) +
          SRCFILE(QNTCSDK/FILEATT) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTPGM PGM(QNTCSDK/FILEATT) +
          MODULE(*PGM) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
