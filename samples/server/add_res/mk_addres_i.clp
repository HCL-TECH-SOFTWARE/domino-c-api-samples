PGM
     MONMSG MSGID(CPF2103)

     CRTCMOD MODULE(QNTCSDK/ADD_RES) +
          SRCSTMF('/QNTCSDK/SAMPLES/SERVER/ADD_RES/ADD_RES.C') +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          DEFINE(OS400 UNIX) +
          TGTCCSID(37) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          INCDIR('/QIBM/PRODDATA/QADRT/INCLUDE' '/QIBM/PRODDATA/LOTUS/NOTESAPI/INCLUDE')

     CRTPGM PGM(QNTCSDK/ADD_RES) +
          MODULE(QNTCSDK/ADD_RES QNOTESAPI/NOTESAI0) +
          ENTMOD(QNOTESAPI/NOTES0) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
