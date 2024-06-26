PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/MKTABLE) +
          SRCFILE(QNTCSDK/MKTABLE) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/CDRECORD) +
          SRCFILE(QNTCSDK/MKTABLE) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/CDTABLE) +
          SRCFILE(QNTCSDK/MKTABLE) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTPGM PGM(QNTCSDK/MKTABLE) +
          MODULE(QNTCSDK/MKTABLE QNTCSDK/CDRECORD QNTCSDK/CDTABLE) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
