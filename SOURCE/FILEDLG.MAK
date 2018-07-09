# MSC V6.00A NMAKE file for: FILEDLG.DLL
#
# (c)Copyright 1990 Rick Yoder

# Macro definitions
#
PROJ            = filedlg
DEBUG           = N
CFLAGS_D        = -Od -Zi
CFLAGS_R        = -Oxaz
CFLAGS_G        = -ML -W3 -c -Gs -FR -BATCH
LFLAGS_D        = /NOI /NOD /CO /MAP /BATCH
LFLAGS_R        = /NOI /NOD /MAP /BATCH

!IF ( "$(DEBUG)"=="Y" || "$(DEBUG)"=="y" )
CC              = cl $(CFLAGS_G) $(CFLAGS_D)
LINK            = link $(LFLAGS_D)
!ELSE
CC              = cl $(CFLAGS_G) $(CFLAGS_R)
LINK            = link $(LFLAGS_R)
!ENDIF

SRCS            = opendlg.c finddlg.c savedlg.c parse.c static.c buttons.c
OBJS            = $(SRCS:.c=.obj)
SBRS            = $(SRCS:.c=.sbr)
LIBS            = llibcdll errmsg os2


ALL: $(PROJ).lib $(PROJ).hlp $(PROJ).dll $(PROJ).bsc


# Resources
#
$(PROJ).res:    $(PROJ).rc dialog.h dialog.dlg
    rc -r $(PROJ).rc

#Import Libraries
$(PROJ).lib:    $(PROJ).def
    implib $(PROJ).lib $(PROJ).def

#QuickHelp files
#
$(PROJ).hlp:    $(PROJ).txt
    helpmake /e15 /a: /t $(PROJ).txt /o$(PROJ).hlp

# Compile source files
#
opendlg.obj opendlg.sbr:    opendlg.c opendata.h   ; $(CC) -NT OPENDLG_TEXT opendlg.c
finddlg.obj finddlg.sbr:    finddlg.c opendata.h   ; $(CC) -NT FINDDLG_TEXT finddlg.c
savedlg.obj savedlg.sbr:    savedlg.c   ; $(CC) -NT SAVEDLG_TEXT savedlg.c
parse.obj   parse.sbr  :    parse.c     ; $(CC) -NT TOOLS_TEXT parse.c
buttons.obj buttons.sbr:    buttons.c   ; $(CC) -NT TOOLS_TEXT buttons.c
static.obj  static.sbr :    static.c    ; $(CC) -NT _TEXT -ND STATICDATA static.c

$(PROJ).bsc:    $(SBRS)
    PWBRMAKE /o $@ $**

# Generate DLL
#
$(PROJ).dll:    $(OBJS) $(PROJ).res $(PROJ).def
    $(LINK) $(OBJS),$(PROJ).dll,,$(LIBS),$(PROJ).def;
    rc $(PROJ).res $(PROJ).dll
