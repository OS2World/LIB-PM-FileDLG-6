# LIB-PM-FileDLG-6

Version 6.1 of the OS/2 PM **File Open** and **File Save** dialog library by Rick Yoder.
Provides polished, threaded file-selection dialogs with long filename support as a pair
of dynamic link libraries (`FILEDLG.DLL` + `ERRMSG.DLL`).

## Features

- Standard **Open File** and **Save File** dialog boxes for OS/2 PM applications
- Long filename support (OS/2 v1.2+)
- Multi-threaded file-find dialog — the user can open a file while a directory
  search is still running
- Horizontal scroll bars in file and directory list boxes for long names
- Help button integration via a caller-supplied callback
- Comma-separated search patterns in Find File (e.g. `*.c,*.h`)
- Hard-error suppression while dialogs are active

## API

Both functions return `FDLG_OK` (0) or `FDLG_CANCEL` (1).

### FileOpenDlg

```c
USHORT EXPENTRY FileOpenDlg(
    HWND   hwndOwner,
    PSZ    pszTitle,       /* NULL -> "Open File"                    */
    PSZ    pszIns,         /* NULL -> "Select file or type filename" */
    PSZ    pszShowSpec,    /* file filter, e.g. "*.*"               */
    USHORT usShowAttr,     /* FILE_NORMAL | FILE_HIDDEN | ...        */
    void (EXPENTRY *pfnHelpProc)(HWND), /* NULL -> no Help button    */
    PSZ    pszFile,        /* [out] selected filename               */
    PHFILE phf,            /* [out] opened file handle              */
    ULONG  ulFileSize,
    PUSHORT pusAction,     /* [out] FILE_CREATED / FILE_EXISTED / ...*/
    USHORT usAttribute,
    USHORT fsOpenFlags,    /* FILE_OPEN | FILE_CREATE | ...          */
    USHORT fsOpenMode,     /* OPEN_ACCESS_* | OPEN_SHARE_*          */
    ULONG  ulReserved      /* must be 0                             */
);
```

### FileSaveDlg

```c
USHORT EXPENTRY FileSaveDlg(
    HWND   hwndOwner,
    PSZ    pszTitle,       /* NULL -> "Save File"    */
    PSZ    pszIns,         /* NULL -> "Type filename" */
    void (EXPENTRY *pfnHelpProc)(HWND),
    PSZ    pszDefault,     /* default filename       */
    PSZ    pszFile,        /* [out] selected filename */
    PHFILE phf,            /* [out] opened file handle */
    ULONG  ulFileSize,
    PUSHORT pusAction,
    USHORT usAttribute,
    USHORT fsOpenFlags,
    USHORT fsOpenMode,
    ULONG  ulReserved
);
```

See [h/FILEDLG.H](h/FILEDLG.H) for the full parameter reference.

## Runtime Requirements

Both `FILEDLG.DLL` and `ERRMSG.DLL` must be present in a directory listed in
the system `LIBPATH` (e.g. copy them to `C:\OS2\DLL`).

## Using the Library in Your Project

1. Add `h\` to your compiler's include path.
2. Link against `lib\FILEDLG.LIB` and `lib\ERRMSG.LIB`.
3. Ship both DLLs with your application.

### Open Watcom example

```
wcc386 -bt=os2 -bm -I h myapp.c
wlink system os2v2_pm name myapp.exe file myapp.obj libpath lib lib FILEDLG.LIB lib ERRMSG.LIB option stack=8192
```

### GCC / WLINK example

```sh
gcc -O2 -I h -I/@unixroot/usr/include -Zomf -c -o myapp.obj myapp.c
wlink sys os2pm name myapp.exe file myapp.obj libpath lib lib FILEDLG.LIB lib ERRMSG.LIB op stack=8192
```

## Compiling the Samples

The `sample/` directory contains two minimal PM test programs.

| Program       | Tests         |
|---------------|---------------|
| OPENTEST.EXE  | FileOpenDlg   |
| SAVETEST.EXE  | FileSaveDlg   |

`FILEDLG.DLL` is also copied to `sample/` so the test programs can find it at runtime
without modifying `LIBPATH`.

### Build with Open Watcom (recommended)

```
cd sample
wmake -f Makefile.wat 2>&1 | tee compile.log
```

### Build with GCC / kLIBC

```sh
cd sample
sh build.sh
```

Output is saved to `sample/compile.log` in both cases.

### Sample GCC 32-bit adaptations

The samples were originally written for 16-bit Microsoft C. The following changes
were made for 32-bit compilation (both GCC and Open Watcom):

| Original (16-bit)                      | Replaced with (32-bit)                      |
|----------------------------------------|---------------------------------------------|
| `_fmalloc(n)`                          | `malloc(n)`                                 |
| `_fstrcpy(dst, src)`                   | `strcpy(dst, src)`                          |
| `DosQSysInfo(0, buf, sizeof(USHORT))`  | `DosQuerySysInfo(QSV_MAX_PATH_LENGTH, ...)`  |
| `#include <malloc.h>`                  | `#include <stdlib.h>`                       |
| `USHORT usPathLen`                     | `ULONG usPathLen`                           |

## Building the DLLs from Source

Run from the project root:

```
wmake -f Makefile.wat 2>&1 | tee compile.log
```

The Makefile builds `ERRMSG.DLL` first (FILEDLG depends on it), then `FILEDLG.DLL`.
Both DLLs are written to `dll\` and their import libraries to `lib\`. The resource
file (`DIALOG.RES`) is compiled with `wrc` and bound into the DLL by the linker.
Exports are declared inline in the wlink directives; no separate `.DEF` files are used.

The build produces zero errors and zero warnings with Open Watcom 2.0.

### Source Porting Notes (v6.1)

The DLL source was originally written for **16-bit OS/2 1.x** with Microsoft C 6.00.
Version 6.1 replaced all 16-bit APIs with their 32-bit equivalents:

| File | 16-bit API | 32-bit replacement |
|------|-----------|-------------------|
| `ERRMSG/errmsg.c` | `DosSemRequest` / `DosSemClear` | `DosRequestMutexSem` / `DosReleaseMutexSem` |
| `ERRMSG/errmsg.c` | `cdecl far _loadds _export` | `EXPENTRY` |
| `FILEDLG/opendlg.c` | `DosQSysInfo` | `DosQuerySysInfo` |
| `FILEDLG/opendlg.c` | `DosGetModHandle` | `DosQueryModuleHandle` |
| `FILEDLG/opendlg.c` | `DosAllocSeg` + `MAKEP` | `DosAllocMem(PAG_COMMIT\|PAG_READ\|PAG_WRITE)` |
| `FILEDLG/opendlg.c` | `DosFreeSeg(SELECTOROF(p))` | `DosFreeMem(p)` |
| `FILEDLG/opendlg.c` | `DosSelectDisk` | `DosSetDefaultDisk` |
| `FILEDLG/opendlg.c` | `DosQCurDisk` | `DosQueryCurrentDisk` |
| `FILEDLG/opendlg.c` | `DosChDir` | `DosSetCurrentDir` |
| `FILEDLG/opendlg.c` | `DosQCurDir` | `DosQueryCurrentDir` |
| `FILEDLG/opendlg.c` | `WinQueryFocus(hwnd, FALSE)` | `WinQueryFocus(hwnd)` |
| `FILEDLG/opendlg.c` | `FILEFINDBUF` | `FILEFINDBUF3` (`oNextEntryOffset` traversal) |
| `FILEDLG/finddlg.c` | `DosSemSet/Clear/Wait` | `DosCreateEventSem` / `DosPostEventSem` / `DosWaitEventSem` |
| `FILEDLG/*.c` | `near`, `far`, `_loadds`, `SEL` | removed (flat model) |
| `FILEDLG/*.c` | `DosOpen` param 3 `PUSHORT` | `PULONG`; param 8 `ULONG` -> `PEAOP2` |

`HARDERROR_ENABLE` and `HARDERROR_DISABLE` are defined locally in `opendlg.c` and
`savedlg.c` because Open Watcom's 32-bit OS/2 headers do not define them.

## File Layout

```
Makefile.wat         Open Watcom build for both DLLs (run from project root)
CHANGES.TXT          Change history
README.TXT           Original distribution README (16-bit era)

dll\
  FILEDLG.DLL        Pre-built (or rebuilt) dialog DLL
  ERRMSG.DLL         Pre-built (or rebuilt) error message DLL

lib\
  FILEDLG.LIB        Import library for FILEDLG.DLL
  ERRMSG.LIB         Import library for ERRMSG.DLL

h\
  FILEDLG.H          Public API header -- FileOpenDlg / FileSaveDlg
  ERRMSG.H           Public API header -- ErrMessageBox

doc\
  FILEDLG.HLP        QuickHelp reference
  ERRMSG.HLP         QuickHelp reference

sample\
  OPENTEST.C         FileOpenDlg test (32-bit adapted)
  OPENTEST.DEF       Module definition
  OPENTEST.EXE       Pre-built binary
  OPENTEST.MAK       Original Microsoft C makefile (reference)
  SAVETEST.C         FileSaveDlg test (32-bit adapted)
  SAVETEST.DEF       Module definition
  SAVETEST.EXE       Pre-built binary
  SAVETEST.MAK       Original Microsoft C makefile (reference)
  FILEDLG.DLL        Copy of DLL for runtime use by the test programs
  Makefile.wat       Open Watcom build file for samples
  Makefile.os2       GCC 9.2 / WLINK build file for samples
  build.sh           GCC build script with log output
  compile_wat.cmd    Open Watcom build script with log output

src\
  FILEDLG\
    OPENDLG.C        Open dialog implementation
    SAVEDLG.C        Save dialog implementation
    FINDDLG.C        Find file dialog implementation
    BUTTONS.C        Custom button controls
    PARSE.C          Filename pattern parser
    STATIC.C         Static control helpers
    FILEDLG.RC       Dialog resources
    DIALOG.DLG       Dialog template source
    DIALOG.H         Dialog control IDs
    DIALOG.RES       Compiled resources (build artifact)
    FILEDLG.DEF      Original 16-bit module definition (reference only)
    FILEDLG.MAK      Original Microsoft C makefile (reference only)
  ERRMSG\
    ERRMSG.C         Error message DLL implementation
    SYSERR.C         System error code table
    PMERR.C          PM error code table
    HELPERR.C        Help Manager error code table
    ERRCONST.H       Internal string constants
    EXAMPLE.C        ErrMessageBox usage example
    ERRMSG.DEF       Original 16-bit module definition (reference only)
    ERRMSG.MAK       Original Microsoft C makefile (reference only)
```

## License

User-supported shareware — see [README.TXT](README.TXT) for the original terms.
Free to use; registration was $25 to Rick Yoder (Indianapolis, IN).
Modifications require renaming the DLLs.

## Author

Rick Yoder (1989-1990) — original 16-bit library

32-bit port to Open Watcom 2.0 (v6.1, 2026)

## Links

- [OS/2 World wiki entry](http://www.os2world.com/wiki/index.php/FileDLG)
- [PC Corner archive](http://www.pcorner.com/list/OS2/FILDL2.ZIP/INFO/)
