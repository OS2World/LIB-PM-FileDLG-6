# Makefile.wat -- Open Watcom build for OPENTEST and SAVETEST on ArcaOS/OS2
# Run with: wmake -f Makefile.wat

CC     = wcc386
# -Oaxt: optimize for time, inline math, strength reduction, loop unrolling
# -DDEFAULT_QUEUE_SIZE=0: not defined in OW OS/2 headers; 0 means use system default
CFLAGS = -bt=os2 -bm -5 -fpi -sg -ei -Oaxt -DDEFAULT_QUEUE_SIZE=0 -D_QC -I..\h

all: opentest.exe savetest.exe

# Import directly from the DLLs by ordinal (FILEDLG.LIB is MS-format, unreadable by wlink)
# FILEOPENDLG=ordinal 1, FILESAVEDLG=ordinal 2 per SOURCE/FILEDLG.DEF

opentest.exe: opentest.obj
	wlink system os2v2_pm name opentest.exe file opentest.obj import FileOpenDlg FILEDLG.#1 option stack=8192 option map

savetest.exe: savetest.obj
	wlink system os2v2_pm name savetest.exe file savetest.obj import FileSaveDlg FILEDLG.#2 option stack=8192 option map

.c.obj: .autodepend
	$(CC) $(CFLAGS) $[*

clean: .symbolic
	@if exist *.obj del *.obj
	@if exist *.exe del *.exe
	@if exist *.map del *.map
	@if exist compile.log del compile.log
