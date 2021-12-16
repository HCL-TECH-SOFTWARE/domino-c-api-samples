PGM
     MONMSG MSGID(CPF2103)

     CRTCMOD MODULE(QNTCSDK/HOTSPOT) +
          SRCSTMF('/QNTCSDK/SAMPLES/RICHTEXT/HOTSPOT/HOTSPOT.C') +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          DEFINE(OS400 UNIX) +
          TGTCCSID(37) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          INCDIR('/QIBM/PRODDATA/QADRT/INCLUDE' '/QIBM/PRODDATA/LOTUS/NOTESAPI/INCLUDE')

     CRTCMOD MODULE(QNTCSDK/CDRECORD) +
          SRCSTMF('/QNTCSDK/SAMPLES/RICHTEXT/HOTSPOT/CDRECORD.C') +
          SYSIFCOPT(*IFSIO) +
          DBGVIEW(*ALL) +
          DEFINE(OS400 UNIX QSRCSTMF) +
          TGTCCSID(37) +
          TERASPACE(*YES) +
          STGMDL(*INHERIT) +
          INCDIR('/QIBM/PRODDATA/QADRT/INCLUDE' '/QIBM/PRODDATA/LOTUS/NOTESAPI/INCLUDE')

     CRTPGM PGM(QNTCSDK/HOTSPOT) +
          MODULE(QNTCSDK/HOTSPOT QNTCSDK/CDRECORD) +
          ENTMOD(QADRT/QADRTMAIN2) +
          BNDSRVPGM(QNOTES/LIBNOTES *LIBL/QADRTTS) +
          OPTION(*DUPVAR *DUPPROC) +
          DETAIL(*BASIC)

ENDPGM
