PGM
     MONMSG MSGID(CPF2103)

     ADDLIBLE QNOTESAPI
     ADDLIBLE QNTCSDK
     CHGSYSLIBL QADRT

     CRTCMOD MODULE(QNTCSDK/EXTPWD) +
          SRCFILE(QNTCSDK/EXTPWD) +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          DEFINE(OS400 UNIX)

     CRTSRVPGM SRVPGM(QNTCSDK/LIBEXTPWD) +
          MODULE(QNTCSDK/EXTPWD) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC) +
          STGMDL(*INHERIT) +
          EXPORT(*ALL) 

ENDPGM
