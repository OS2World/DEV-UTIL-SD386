# Makefile for Esp

CSRC=
ASRC=
COBJ=
AOBJ=

mem=n
opt=n
prf=n

memopts = /Tm+
dbgopts = /O-
optopts = /O+
prfopts = /Gh+
msgopts1 =/W3 /Wall-cnd+eff+enu+ord+par-pro+rea+ret-
msgopts2 =tru+use+

copts   = icc /c /Se /Sp /Ss /Gm+ /Ti+ /Tl+ /D__ESP__

cc=$(copts)

cc=$(cc) $(msgopts1)$(msgopts2)

!IF "$(mem)"=="y"
cc=$(cc) $(memopts)
!ENDIF

!IF  "$(opt)"=="y"
cc=$(cc) $(optopts)
!ELSE
cc=$(cc) $(dbgopts)
!ENDIF

!IF  "$(prf)"=="y"
cc=$(cc) $(prfopts)
!ENDIF

aa=masm386  /DFARCALLS /ML /ZI /ZD     $*;

{$(CSRC)}.c{$(COBJ)}.obj:
 $(cc) /Fo$@ $<  >>of

{$(ASRC)}.asm{$(AOBJ)}.obj:
 $(aa) $<,$(AOBJ)$(@B).obj; >>of

targets.all: sd386.msg esp.exe

sd386.msg: sd386.txt  esp.inp
        mkmsgf sd386.txt sd386.msg


#
#-dependency list for build of esp.exe-------------------------------
#
dlist= \
xsrvmain.obj  \
xsrvgo_e.obj  \
xsrvbrk.obj   \
xsrvmte.obj   \
xsrvdis.obj   \
xsrvfile.obj  \
xsrvdbif.obj  \
xsrvfexe.obj  \
espstrte.obj  \
xsrvthds.obj  \
xsrvunwd.obj  \
xsrvxcpt.obj  \
xsrvacvt.obj  \
xsrvwps.obj   \
bytes.obj     \
davetoll.obj  \
com.obj       \
rx.obj        \
log.obj       \
getmsg.obj    \
q.obj         \
pipe.obj      \
router.obj    \
netbios.obj   \
tcpip.obj     \
spawn_e.obj   \
espq.obj      \
vb_e.obj      \
pid_e.obj     \
sema4_e.obj   \
debug.obj

#
#-SD386.EXE build description ------------------------------------------
#
esp.exe: $(dlist)
  ilink /NOFREE /CO /m /STACK:0x0E000 @esp.lk;
  msgbind  esp.inp

#
#-*.OBJ build descriptions --------------------------------------------
#

xsrvmain.obj: xsrvmain.c
xsrvgo_e.obj: xsrvgo.c
 $(cc) /Fo$@ xsrvgo.c >>of
xsrvmte.obj:  xsrvmte.c
xsrvbrk.obj:  xsrvbrk.c
xsrvdis.obj:  xsrvdis.c
xsrvfile.obj: xsrvfile.c
xsrvdbif.obj: xsrvdbif.c
xsrvfexe.obj: xsrvfexe.c
espstrte.obj: espstart.c
 $(cc) /Fo$@ espstart.c >>of
xsrvthds.obj: xsrvthds.c
xsrvunwd.obj: xsrvunwd.c
xsrvxcpt.obj: xsrvxcpt.c
xsrvacvt.obj: xsrvacvt.c
xsrvwps.obj:  xsrvwps.c
bytes.obj:    bytes.asm
davetoll.obj: davetoll.c
com.obj:      com.c
rx.obj:       rx.c
log.obj:      log.c
getmsg.obj:   getmsg.c
q.obj:        q.c
pipe.obj:     pipe.c
router.obj:   router.c
netbios.obj:  netbios.c
tcpip.obj:    tcpip.c
spawn_e.obj:  spawn.c
 $(cc) /Fo$@ spawn.c >>of
espq.obj:     espq.c
vb_e.obj:     vb.c
 $(cc) /Fo$@ vb.c    >>of
pid_e.obj:    pid.c
 $(cc) /Fo$@ pid.c   >>of
sema4_e.obj:    sema4.c
 $(cc) /Fo$@ sema4.c   >>of
debug.obj:    debug.c
