PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/MSG_Q) +
          SRCFILE(QNTCSDK/MSG_Q) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTCMOD MODULE(QNTCSDK/MSG_T) +
          SRCFILE(QNTCSDK/MSG_Q) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTPGM PGM(QNTCSDK/MSG_Q) +
          MODULE(QNTCSDK/MSG_Q QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

     CRTPGM PGM(QNTCSDK/MSG_T) +
          MODULE(QNTCSDK/MSG_T QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
