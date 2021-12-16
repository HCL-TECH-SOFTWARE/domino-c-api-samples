PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/CONSUMER) +
          SRCFILE(QNTCSDK/EVENTS) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

      CRTCMOD MODULE(QNTCSDK/PRODUCER) +
          SRCFILE(QNTCSDK/EVENTS) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTPGM PGM(QNTCSDK/CONSUMER) +
          MODULE(QNTCSDK/CONSUMER QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

    CRTPGM PGM(QNTCSDK/PRODUCER) +
          MODULE(QNTCSDK/PRODUCER QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
