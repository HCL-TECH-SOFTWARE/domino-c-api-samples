PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/BILLSES) +
          SRCFILE(QNTCSDK/BILLING) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX) +
          PACKSTRUCT(4)

     CRTCMOD MODULE(QNTCSDK/BILLMNGR) +
          SRCFILE(QNTCSDK/BILLING) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX) +
          PACKSTRUCT(4)

     CRTSRVPGM SRVPGM(QNTCSDK/LIBBILLMGR) +
          MODULE(QNTCSDK/BILLMNGR) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          STGMDL(*INHERIT) +
          EXPORT(*ALL) +
          DETAIL(*BASIC)

     CRTPGM PGM(QNTCSDK/BILLSES) +
          MODULE(QNTCSDK/BILLSES QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
