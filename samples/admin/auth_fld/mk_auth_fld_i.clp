PGM
     MONMSG MSGID(CPF2103)

     CRTCMOD MODULE(QNTCSDK/AUTH_FLD) +
          SRCSTMF('/QNTCSDK/SAMPLES/ADMIN/AUTH_FLD/AUTH_FLD.C') +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          DEFINE(OS400 UNIX) +
          TGTCCSID(37) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          INCDIR('/QIBM/PRODDATA/QADRT/INCLUDE' '/QIBM/PRODDATA/LOTUS/NOTESAPI/INCLUDE')

     CRTPGM PGM(QNTCSDK/AUTH_FLD) +
          MODULE(*PGM) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
