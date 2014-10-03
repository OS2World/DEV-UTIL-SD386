# Makefile for sd386

CSRC=
ASRC=
COBJ=
AOBJ=

mem=n
mym=n
opt=n
prf=n
msh=n

memopts = /Tm+
mymopts = /D__MYMALLOC
dbgopts = /O-
optopts = /O+
prfopts = /Gh+
msgopts1 =/W3 /Wall-cnd+eff+enu+ord+par-pro+rea+ret-
msgopts2 =tru+use+trd-
mshopts  =/I$(MSHHOME)\src /DMSH /DSD386 /DDLJ /DOS2

copts   = icc /c /Se /Sp /Ss /Gm+ /Ti+ /Tl+ /D__DBG__

cc=$(copts)

cc=$(cc) $(msgopts1)$(msgopts2)

!IF "$(mem)"=="y"
cc=$(cc) $(memopts)
!ENDIF

!IF "$(mym)"=="y"
cc=$(cc) $(mymopts)
!ENDIF

!IF  "$(opt)"=="y"
cc=$(cc) $(optopts)
!ELSE
cc=$(cc) $(dbgopts)
!ENDIF

!IF  "$(prf)"=="y"
cc=$(cc) $(prfopts)
!ENDIF

!IF  "$(msh)"=="y"
cc=$(cc) $(mshopts)
!ENDIF

aa=masm386  /DFARCALLS /ML /ZI /ZD     $*;

{$(CSRC)}.c{$(COBJ)}.obj:
 $(cc) /Fo$@ $<  >>of

{$(ASRC)}.asm{$(AOBJ)}.obj:
 $(aa) $<,$(AOBJ)$(@B).obj; >>of

targets.all: sd386.msg sd386.exe

sd386.msg: sd386.txt
        mkmsgf sd386.txt sd386.msg

#
#-dependency list for build of SD386.EXE --------------------------------
#
olist= $(COBJ)sd386.obj      \
$(COBJ)invoke.obj    \
$(COBJ)vio.obj       \
$(COBJ)putrc.obj     \
$(COBJ)bytes.obj     \
$(COBJ)vbox.obj      \
$(COBJ)encode.obj    \
$(COBJ)putup.obj     \
$(COBJ)misc.obj      \
$(COBJ)lowercas.obj  \
$(COBJ)findvstr.obj  \
$(COBJ)panic.obj     \
$(COBJ)kbd.obj       \
$(COBJ)help.obj      \
$(COBJ)getmsg.obj    \
$(COBJ)debfile.obj   \
$(COBJ)fileacc.obj   \
$(COBJ)dbif.obj      \
$(COBJ)run.obj       \
$(COBJ)brk.obj       \
$(COBJ)go.obj        \
$(COBJ)threads.obj   \
$(COBJ)findfp.obj    \
$(COBJ)makefp.obj    \
$(COBJ)ldsource.obj  \
$(COBJ)frstuff.obj   \
$(COBJ)pagefp.obj    \
$(COBJ)dbsegs.obj    \
$(COBJ)sd386pro.obj   \
$(COBJ)showdz.obj    \
$(COBJ)dll.obj       \
$(COBJ)exe.obj       \
$(COBJ)findfile.obj  \
$(COBJ)dbifext.obj   \
$(COBJ)findpro.obj   \
$(COBJ)showc.obj     \
$(COBJ)showcf.obj    \
$(COBJ)cbrk.obj      \
$(COBJ)keystr.obj    \
$(COBJ)setcolor.obj  \
$(COBJ)unwind.obj    \
$(COBJ)symbols.obj   \
$(COBJ)disasm.obj    \
$(COBJ)davetoll.obj  \
$(COBJ)expr.obj      \
$(COBJ)types.obj     \
$(COBJ)showdk.obj    \
$(COBJ)actbar.obj    \
$(COBJ)menu.obj      \
$(COBJ)browser.obj   \
$(COBJ)boxmenu.obj   \
$(COBJ)showd.obj     \
$(COBJ)showd2.obj    \
$(COBJ)showdx.obj    \
$(COBJ)showdz.obj    \
$(COBJ)showaf.obj    \
$(COBJ)viewasm.obj   \
$(COBJ)findexec.obj  \
$(COBJ)idbrk.obj     \
$(COBJ)zoomdata.obj  \
$(COBJ)showdf.obj    \
$(COBJ)term.obj      \
$(COBJ)freefp.obj    \
$(COBJ)getfile.obj   \
$(COBJ)showdlls.obj  \
$(COBJ)asminit.obj   \
$(COBJ)pointer.obj   \
$(COBJ)scrolla.obj   \
$(COBJ)setexcep.obj  \
$(COBJ)showvreg.obj  \
$(COBJ)showcorg.obj  \
$(COBJ)setwps.obj    \
$(COBJ)acvt.obj      \
$(COBJ)xsrvacvt.obj  \
$(COBJ)mouse.obj     \
$(COBJ)dialog.obj    \
$(COBJ)popups.obj    \
$(COBJ)cuamenu.obj   \
$(COBJ)cuamap.obj    \
$(COBJ)cuasetwp.obj  \
$(COBJ)cuathds.obj   \
$(COBJ)cuaclstk.obj  \
$(COBJ)action.obj    \
$(COBJ)import.obj    \
$(COBJ)showa.obj     \
$(COBJ)maptypes.obj  \
$(COBJ)mapsyms.obj   \
$(COBJ)fmtdata.obj   \
$(COBJ)mte.obj       \
$(COBJ)xbox.obj      \
$(COBJ)xsrvgo_d.obj  \
$(COBJ)xsrvbrk.obj   \
$(COBJ)xsrvmte.obj   \
$(COBJ)xsrvdis.obj   \
$(COBJ)xsrvdbif.obj  \
$(COBJ)xsrvfexe.obj  \
$(COBJ)espstrtd.obj  \
$(COBJ)xsrvthds.obj  \
$(COBJ)xsrvunwd.obj  \
$(COBJ)xsrvwps.obj   \
$(COBJ)xsrvxcpt.obj  \
$(COBJ)com.obj       \
$(COBJ)tx.obj        \
$(COBJ)log.obj       \
$(COBJ)q.obj         \
$(COBJ)dbgq.obj      \
$(COBJ)pipe.obj      \
$(COBJ)router.obj    \
$(COBJ)netbios.obj   \
$(COBJ)tcpip.obj     \
$(COBJ)spawn_d.obj   \
$(COBJ)pid_d.obj     \
$(COBJ)vb_d.obj      \
$(COBJ)cuaproc.obj   \
$(COBJ)sema4_d.obj   \
$(COBJ)debug.obj     \
$(COBJ)qproc.obj     \
$(COBJ)comdmp.obj    \
$(COBJ)breakpnt.obj  \
$(COBJ)linnum.obj    \
$(COBJ)talloc.obj    \
$(COBJ)showclas.obj  \

!IF  "$(msh)"=="y"
 $(COBJ)cpydata.obj   \
 $(COBJ)browsem.obj   \
 $(COBJ)msh.obj       \
 $(COBJ)findd.obj     \
 $(COBJ)findd2.obj    \
!ENDIF

#
#-SD386.EXE build description ------------------------------------------
#
sd386.exe:  $(olist)
  ilink /NOFREE     /STACK:0x50000 /m @sd386.lk
  msgbind  sd386.inp

#
#-*.OBJ build descriptions --------------------------------------------
#

$(COBJ)putrc.obj:    $(CSRC)putrc.asm
$(COBJ)bytes.obj:    $(CSRC)bytes.asm
$(COBJ)vbox.obj:     $(CSRC)vbox.asm
$(COBJ)encode.obj:   $(CSRC)encode.asm
$(COBJ)putup.obj:    $(CSRC)putup.asm
$(COBJ)misc.obj:     $(CSRC)misc.asm
$(COBJ)lowercas.obj: $(CSRC)lowercas.asm
$(COBJ)findvstr.obj: $(CSRC)findvstr.asm
$(COBJ)sd386.obj:    $(CSRC)sd386.c
$(COBJ)invoke.obj:   $(CSRC)invoke.c
$(COBJ)vio.obj:      $(CSRC)vio.c
$(COBJ)panic.obj:    $(CSRC)panic.c
$(COBJ)kbd.obj:      $(CSRC)kbd.c
$(COBJ)help.obj:     $(CSRC)help.c
$(COBJ)getmsg.obj:   $(CSRC)getmsg.c
$(COBJ)debfile.obj:  $(CSRC)debfile.c
$(COBJ)fileacc.obj:  $(CSRC)fileacc.c
$(COBJ)dbif.obj:     $(CSRC)dbif.c
$(COBJ)run.obj:      $(CSRC)run.c
$(COBJ)brk.obj:      $(CSRC)brk.c
$(COBJ)go.obj:       $(CSRC)go.c
$(COBJ)threads.obj:  $(CSRC)threads.c
$(COBJ)findfp.obj:   $(CSRC)findfp.c
$(COBJ)makefp.obj:   $(CSRC)makefp.c
$(COBJ)ldsource.obj: $(CSRC)ldsource.c
$(COBJ)frstuff.obj:  $(CSRC)frstuff.c
$(COBJ)pagefp.obj:   $(CSRC)pagefp.c
$(COBJ)dbsegs.obj:   $(CSRC)dbsegs.c
$(COBJ)sd386pro.obj: $(CSRC)sd386pro.c
$(COBJ)showdz.obj:   $(CSRC)showdz.c
$(COBJ)dll.obj:      $(CSRC)dll.c
$(COBJ)exe.obj:      $(CSRC)exe.c
$(COBJ)findfile.obj: $(CSRC)findfile.c
$(COBJ)dbifext.obj:  $(CSRC)dbifext.c
$(COBJ)findpro.obj:  $(CSRC)findpro.c
$(COBJ)showc.obj:    $(CSRC)showc.c
$(COBJ)showcf.obj:   $(CSRC)showcf.c
$(COBJ)cbrk.obj:     $(CSRC)cbrk.c
$(COBJ)keystr.obj:   $(CSRC)keystr.c
$(COBJ)setcolor.obj: $(CSRC)setcolor.c
$(COBJ)unwind.obj:   $(CSRC)unwind.c
$(COBJ)symbols.obj:  $(CSRC)symbols.c
$(COBJ)disasm.obj:   $(CSRC)disasm.c
$(COBJ)davetoll.obj: $(CSRC)davetoll.c
 $(cc) /O- /Fo$@ davetoll.c >>of
$(COBJ)limbo.obj:    $(CSRC)limbo.c
$(COBJ)expr.obj:     $(CSRC)expr.c
$(COBJ)types.obj:    $(CSRC)types.c
$(COBJ)showdk.obj:   $(CSRC)showdk.c
$(COBJ)actbar.obj:   $(CSRC)actbar.c
$(COBJ)menu.obj:     $(CSRC)menu.c
$(COBJ)browser.obj:  $(CSRC)browser.c
$(COBJ)boxmenu.obj:  $(CSRC)boxmenu.c
$(COBJ)showd.obj:    $(CSRC)showd.c
$(COBJ)showd2.obj:   $(CSRC)showd2.c
$(COBJ)showdx.obj:   $(CSRC)showdx.c
$(COBJ)showdz.obj:   $(CSRC)showdz.c
$(COBJ)showaf.obj:   $(CSRC)showaf.c
$(COBJ)viewasm.obj:  $(CSRC)viewasm.c
$(COBJ)findexec.obj: $(CSRC)findexec.c
$(COBJ)idbrk.obj:    $(CSRC)idbrk.c
$(COBJ)zoomdata.obj: $(CSRC)zoomdata.c
$(COBJ)showdf.obj:   $(CSRC)showdf.c
$(COBJ)term.obj:     $(CSRC)term.c
$(COBJ)freefp.obj:   $(CSRC)freefp.c
$(COBJ)getfile.obj:  $(CSRC)getfile.c
$(COBJ)showdlls.obj: $(CSRC)showdlls.c
$(COBJ)asminit.obj:  $(CSRC)asminit.c
$(COBJ)pointer.obj:  $(CSRC)pointer.c
$(COBJ)scrolla.obj:  $(CSRC)scrolla.c
$(COBJ)setexcep.obj: $(CSRC)setexcep.c
$(COBJ)showvreg.obj: $(CSRC)showvreg.c
$(COBJ)showcorg.obj: $(CSRC)showcorg.c
$(COBJ)setwps.obj:   $(CSRC)setwps.c
$(COBJ)acvt.obj:     $(CSRC)acvt.c
$(COBJ)xsrvacvt.obj: $(CSRC)xsrvacvt.c
$(COBJ)mouse.obj:    $(CSRC)mouse.c
$(COBJ)dialog.obj:   $(CSRC)dialog.c
$(COBJ)popups.obj:   $(CSRC)popups.c
$(COBJ)cuamenu.obj:  $(CSRC)cuamenu.c $(CSRC)cuamenu.h
$(COBJ)cuamap.obj:   $(CSRC)cuamap.c
$(COBJ)cuasetwp.obj: $(CSRC)cuasetwp.c
$(COBJ)cuathds.obj:  $(CSRC)cuathds.c
$(COBJ)cuaclstk.obj: $(CSRC)cuaclstk.c
$(COBJ)action.obj:   $(CSRC)action.c
$(COBJ)import.obj:   $(CSRC)import.c
$(COBJ)showa.obj:    $(CSRC)showa.c
$(COBJ)maptypes.obj: $(CSRC)maptypes.c
 $(cc) /O+   /Fo$@ maptypes.c >>of
$(COBJ)mapsyms.obj:  $(CSRC)mapsyms.c
$(COBJ)fmtdata.obj:  $(CSRC)fmtdata.c
$(COBJ)mte.obj:      $(CSRC)mte.c
$(COBJ)xbox.obj:     $(CSRC)xbox.c
$(COBJ)xsrvgo_d.obj: $(CSRC)xsrvgo.c
 $(cc) /Fo$@ xsrvgo.c >>of
$(COBJ)xsrvmte.obj:  $(CSRC)xsrvmte.c
$(COBJ)xsrvbrk.obj:  $(CSRC)xsrvbrk.c
$(COBJ)xsrvdis.obj:  $(CSRC)xsrvdis.c
$(COBJ)xsrvdbif.obj: $(CSRC)xsrvdbif.c
$(COBJ)xsrvfexe.obj: $(CSRC)xsrvfexe.c
$(COBJ)espstrtd.obj: $(CSRC)espstart.c
 $(cc) /Fo$@ espstart.c >>of
$(COBJ)xsrvthds.obj: $(CSRC)xsrvthds.c
$(COBJ)xsrvunwd.obj: $(CSRC)xsrvunwd.c
$(COBJ)xsrvwps.obj:  $(CSRC)xsrvwps.c
$(COBJ)xsrvxcpt.obj: $(CSRC)xsrvxcpt.c
$(COBJ)com.obj:      $(CSRC)com.c
$(COBJ)tx.obj:       $(CSRC)tx.c
$(COBJ)log.obj:      $(CSRC)log.c
$(COBJ)q.obj:        $(CSRC)q.c
$(COBJ)dbgq.obj:     $(CSRC)dbgq.c
$(COBJ)pipe.obj:     $(CSRC)pipe.c
$(COBJ)router.obj:   $(CSRC)router.c
$(COBJ)netbios.obj:  $(CSRC)netbios.c
$(COBJ)tcpip.obj:    $(CSRC)tcpip.c
$(COBJ)pid_d.obj:    $(CSRC)pid.c
 $(cc) /Fo$@ pid.c >>of
$(COBJ)showclas.obj: $(CSRC)showclas.c
$(COBJ)spawn_d.obj:  $(CSRC)spawn.c
 $(cc) /Fo$@ spawn.c >>of
$(COBJ)vb_d.obj:  $(CSRC)vb.c
 $(cc) /Fo$@ vb.c    >>of
$(COBJ)cuaproc.obj:  $(CSRC)cuaproc.c   $(CSRC)diaproc.h
$(COBJ)sema4_d.obj:    $(CSRC)sema4.c
 $(cc) /Fo$@ sema4.c >>of
$(COBJ)debug.obj:    $(CSRC)debug.c
$(COBJ)qproc.obj:    $(CSRC)qproc.c
$(COBJ)comdmp.obj:   $(CSRC)comdmp.c
$(COBJ)breakpnt.obj: $(CSRC)breakpnt.c
$(COBJ)linnum.obj:   $(CSRC)linnum.c
$(COBJ)talloc.obj:   $(CSRC)talloc.c

!IF  "$(msh)"=="y"
 $(COBJ)msh.obj:      $(CSRC)msh.c
 $(COBJ)cpydata.obj:  $(CSRC)cpydata.c
 $(COBJ)findd.obj:    $(CSRC)findd.c
 $(COBJ)findd2.obj:   $(CSRC)findd2.c
 $(COBJ)browsem.obj:  $(CSRC)browsem.c
!ENDIF

