PROJ = savetest
PROJFILE = savetest.mak
DEBUG = 0

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = nmake
CC  = cl
CFLAGS_G  = /W2 /G2 /Zp /BATCH
CFLAGS_D  = /Od /Zi /Gs
CFLAGS_R  = /Ot /Ol /Og /Oe /Oi /Gs
MAPFILE_D  = NUL
MAPFILE_R  = NUL
LFLAGS_G  =  /NOI /STACK:8192  /BATCH
LFLAGS_D  =  /CO /FAR /PACKC /PACKD  /PMTYPE:PM
LFLAGS_R  = /EXE /FAR /PACKC /PACKD /PMTYPE:PM
LINKER  = link
ILINK  = ilink
LRF  = echo > NUL
RC  = rc
LLIBS_R  =  /NOD:SLIBCE SLIBCEP
LLIBS_D  =  /NOD:SLIBCE SLIBCEP

DEF_FILE  = savetest.def
OBJS  = savetest.obj
SBRS  = savetest.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .obj .c

savetest.obj : savetest.c

savetest.sbr : savetest.c


$(PROJ).bsc : $(SBRS)
        $(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(DEF_FILE) $(OBJS)
!IF $(DEBUG)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
!IF $(DEBUG)
        $(LINKER) @$(PROJ).lrf
!ELSE
        $(LINKER) @$(PROJ).lrf
!ENDIF


.c.sbr :
!IF $(DEBUG)
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_D) /FR$@ $<
!ELSE
        $(CC) /Zs $(CFLAGS_G) $(CFLAGS_R) /FR$@ $<
!ENDIF

.c.obj :
!IF $(DEBUG)
        $(CC) /c $(CFLAGS_G) $(CFLAGS_D) /Fo$@ $<
!ELSE
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
!ENDIF


run: $(PROJ).exe
        $(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
        CVP $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
