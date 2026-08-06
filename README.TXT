LIB-PM-FileDLG-6  Version 6.1
==============================

Following is a list of the main files in this distribution.

    README.TXT    ... This file.
    CHANGES.TXT   ... List of changes made from the previous version.
    README.md     ... Project overview and build instructions (Markdown).
    Makefile.wat  ... Open Watcom 2.0 build file for both DLLs.

    dll\
      FILEDLG.DLL ... Dynamic link library containing file dialog functions.
      ERRMSG.DLL  ... Dynamic link library containing error handler.

    lib\
      FILEDLG.LIB ... Import library for FILEDLG.DLL.
      ERRMSG.LIB  ... Import library for ERRMSG.DLL.

    h\
      FILEDLG.H   ... Header file with FileOpenDlg / FileSaveDlg declarations.
      ERRMSG.H    ... Header file with ErrMessageBox declaration.

    doc\
      FILEDLG.HLP ... QuickHelp reference for file dialog functions.
      ERRMSG.HLP  ... QuickHelp reference for error handler.

    sample\
      OPENTEST.C  ... Sample program for FileOpenDlg.
      SAVETEST.C  ... Sample program for FileSaveDlg.
      Makefile.wat... Open Watcom 2.0 build file for samples.

    src\          ... Full source code for both DLLs.

Note that for the file dialog functions to work, *both* FILEDLG.DLL
and ERRMSG.DLL must be located in a directory listed in the LIBPATH
(e.g. copy them to C:\OS2\DLL).

---------------------------------------------------------------------------
Version 6.1 (2026) -- 32-bit OS/2 port

This version ports the original 16-bit library to 32-bit OS/2 (flat model),
buildable with Open Watcom 2.0 on ArcaOS / OS/2 Warp.  All 16-bit APIs have
been replaced with their 32-bit equivalents; the build produces zero errors
and zero warnings.  See CHANGES.TXT for the full list of changes.

---------------------------------------------------------------------------
Original version 6.0 (1990) -- Rick Yoder

This software is user-supported software and may be freely
copied and used by anyone who wishes to do so as long as the
following conditions are met:

1.   These functions may not be used by any program for
     which a fee is charged unless the owner of the program
     has paid the registration fee.

2.   If you transmit a copy of this software to another
     party you must give them the complete unaltered
     contents of the FILEDLG.ZIP file.

3.   If you make any modifications to these functions you
     must change the name of the dynamic link library files
     FILEDLG.DLL and ERRMSG.DLL to some other name.
