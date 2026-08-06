# MSC V6.00A NMAKE file for: ERRMSG.DLL
#

# Macro definitions
#
PROJ            = errmsg
DEBUG           = N
CFLAGS_D        = -Od -Zi
CFLAGS_R        = -Oxaz
CFLAGS_G        = -ML -W3 -c -Gs -BATCH -NT _TEXT
LFLAGS_D        = /NOI /NOD /CO /MAP /BATCH
LFLAGS_R        = /NOI /NOD /MAP /BATCH
AFLAGS          = /ML /W2

!IF ( "$(DEBUG)"=="Y" || "$(DEBUG)"=="y" )
CC              = cl $(CFLAGS_G) $(CFLAGS_D)
LINK            = link $(LFLAGS_D)
!ELSE
CC              = cl $(CFLAGS_G) $(CFLAGS_R)
LINK            = link $(LFLAGS_R)
!ENDIF
AS              = masm $(AFLAGS)

SRCS            = errmsg.c syserr.c pmerr.c helperr.c
OBJS            = $(SRCS:.c=.obj) fltused.obj
LIBS            = llibcdll os2

ALL: $(PROJ).lib $(PROJ).hlp $(PROJ).dll

$(PROJ).lib:    $(PROJ).def
    implib $(PROJ).lib $(PROJ).def

$(PROJ).hlp:    $(PROJ).txt
    helpmake /e15 /t /a: /o$(PROJ).hlp $(PROJ).txt

errmsg.obj:     errmsg.c    ; $(CC) $?
syserr.obj:     syserr.c    ; $(CC) -ND SYSERRDATA $?
pmerr.obj:      pmerr.c     ; $(CC) -ND PMERRDATA $?
helperr.obj:    helperr.c   ; $(CC) -ND HLPMGRDATA $?
fltused.obj:    fltused.asm ; $(AS) $?;

$(PROJ).dll:    $(OBJS) $(PROJ).def
    $(LINK) $(OBJS),$@,,$(LIBS),$(PROJ).def
