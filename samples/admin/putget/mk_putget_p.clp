PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/FILE_IO) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/MISC) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/FIELDS) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/PUTNOTE) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/GETNOTE) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/PUT_UI) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/GET_UI) +
          SRCFILE(QNTCSDK/PUTGET) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTPGM PGM(QNTCSDK/PUT_UI) +
          MODULE(QNTCSDK/PUT_UI QNTCSDK/PUTNOTE QNTCSDK/MISC QNTCSDK/FILE_IO QNTCSDK/FIELDS) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

     CRTPGM PGM(QNTCSDK/GET_UI) +
          MODULE(QNTCSDK/GET_UI QNTCSDK/GETNOTE QNTCSDK/MISC QNTCSDK/FILE_IO QNTCSDK/FIELDS) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
