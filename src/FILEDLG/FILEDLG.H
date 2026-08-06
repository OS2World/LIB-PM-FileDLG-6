/****************************************************************************
 * FILEDLG.H                                                                *
 *                                                                          *
 *  This header file contains the function declarations and constant        *
 *  definitions for the standard open file and save file dialog             *
 *  boxes.                                                                  *
 *                                                                          *
 *                                                                          *
 *  Modifications -                                                         *
 *      09-Aug-1989 : Initial version.                                      *
 *      11-Oct-1990 : Changed "CALLBACK" to "EXPENTRY"                      *
 *                                                                          *
 *  (c)Copyright 1990 Rick Yoder                                            *
 ****************************************************************************/

#if !defined(_FILEDLG_INCLUDED)
#define _FILEDLG_INCLUDED

#if !defined(_QC)
#pragma comment( lib,"filedlg.lib" )
#endif

/****************************************************************************
 *  Constant Definitions                                                    *
 ****************************************************************************/
    #define FDLG_OK         0   // Function exit codes
    #define FDLG_CANCEL     1

    #define FDLG_OPEN       1   // Used by help functions.
    #define FDLG_SAVE       2
    #define FDLG_FIND       3
/****************************************************************************/


/****************************************************************************
 *  Open File dialog box procedure                                          *
 ****************************************************************************/
extern USHORT EXPENTRY FileOpenDlg( HWND hwndOwner,
                                    PSZ pszTitle,PSZ pszIns,
                                    PSZ pszShowSpec,USHORT usShowAttr,
                                    void (EXPENTRY *pfnHelpProc)(HWND hDlg),
                                    PSZ pszFile,
                                    PHFILE phf,
                                    ULONG ulFileSize,
                                    PUSHORT pusAction,
                                    USHORT usAttribute,
                                    USHORT fsOpenFlags,
                                    USHORT fsOpenMode,
                                    ULONG ulReserved  );
/****************************************************************************\
The FileOpenDlg function displays a dialog box and opens the file selected
by the user. This function returns a handle that can be used to read from
and write to the file, as well as to retrieve information about the file.

Parameters   Description
----------------------------------------------------------------------------
hwndOwner    Identifies the owner window.

pszTitle     Points to the null-terminated string that specifies the
             dialog box title. If pszTitle is NULL then the default value
             "Open File" is used.

pszIns       Points to the null-terminated string that specifies the
             dialog box instructions. If pszIns is NULL then the default
             value "Select file or type filename" is used.

pszShowSpec  Points the the null-terminated string that specifies the
             initial search specifications for the files that are to
             be listed in the file list box.

usShowAttr   Specifies the attribute(s) of the files that are to be
             listed in the file list box. This parameter can be a
             combination of the following values

             Value          Meaning
             ---------------------------------------------------------------
             FILE_NORMAL    List all normal files.

             FILE_READONLY  List read-only files.

             FILE_HIDDEN    List hidden files.

             FILE_SYSTEM    List system files.

             FILE_ARCHIVED  List archived files.

pfnHelpProc  Points to the user supplied help procedure. This procedure
             is called whenever the help button or the F1 key is pressed.
             The HelpProc function is defined as follows:

             void EXPENTRY HelpProc( HWND hDlg )
             The hDlg parameter is a handle to the open file dialog box.

             When the help procedure is called, the identity of the
             dialog box that requested the help can be determined
             by a call to WinQueryWindowUShort(hDlg,QWS_ID). The
             value returned can be one of the following:

                FDLG_OPEN ..... Help requested by open dialog box.
                FDLG_FIND ..... Help requested by find file dialog box.
                FDLG_SAVE ..... Help requested by save file dialog box.

             If no help is available, set the pfnHelpProc parameter to
             NULL. This will cause the Help button to be removed from
             the dialog box.

pszFileName  Points to the location where the fully qualified filename of
             the file selected by the user is to be stored. Use the
             DosQSysInfo function to get the maximum path length in order
             to determine how large to make the memory block pointed to
             by pszFile.

phf          Points to the variable that receives the handle of the opened
             file.

pusAction    Points to the variable receiving the value that specifies the
             action taken by the FileOpenDlg function. If the dialog box is
             cancelled then this value has no meaning. Otherwise, it is
             one of the following values:

             Value           Meaning
             ---------------------------------------------------------------
             FILE_CREATED    File was created.

             FILE_EXISTED    File already existed.

             FILE_TRUNCATED  File existed and was truncated.

ulFileSize   Specifies the file's new size (in bytes). This parameter
             applies only if the file is created or truncated. The size
             specification has no effect on a file that is opened only for
             reading.

usAttribute  Specifies the file attributes. This parameter can be a
             combination of the following values:

             Value          Meaning
             ---------------------------------------------------------------
             FILE_NORMAL    File can be read from or written to.

             FILE_READONLY  File can be read from, but not written to.

             FILE_HIDDEN    File is hidden and does not appear in a
                            directory listing.

             FILE_SYSTEM    File is a system file.

             FILE_ARCHIVED  File has been archived.

             File attributes apply only if the file is created.

fsOpenFlags  Specifies the action to take both when the file exists and when
             it does not exist. This parameter may be one of the following
             values:

             Value                        Meaning
             ---------------------------------------------------------------
             FILE_CREATE                  Create a new file; fail if the
                                          file already exists.

             FILE_OPEN                    Open an existing file; fail if the
                                          file does not exist.

             FILE_OPEN | FILE_CREATE      Open an existing file or create
                                          the file if it does not exist.

             FILE_TRUNCATE                Open an existing file and change
                                          to a given size.

             FILE_TRUNCATE | FILE_CREATE  Open an existing file and truncate
                                          it, or create the file if it does
                                          not exist.

fsOpenMode   Specifies the modes with which to open the file. It consists of
             one access mode and one share mode. The other values are option
             and can be given in any combination:

             Value                     Meaning
             ---------------------------------------------------------------
             OPEN_ACCESS_READONLY         Data can be read from the file but
                                          not written to it.

             OPEN_ACCESS_READWRITE        Data can be read from or written
                                          to the file.

             OPEN_ACCESS_WRITEONLY        Data can be written to the file
                                          but not read from it.

             OPEN_SHARE_DENYNONE          Other processes can open the file
                                          for any access: read-only,
                                          write-only, or read-write.

             OPEN_SHARE_DENYREAD          Other processes can open the file
                                          for write-only access but they
                                          cannot open it for read-only or
                                          read-write access.

             OPEN_SHARE_DENYREADWRITE     The current process has exclusive
                                          access to the file. The file
                                          cannot be opened by any process
                                          (including the current process).

             OPEN_SHARE_DENYWRITE         Other processes can open the file
                                          for read-only access but they
                                          cannot open it for write-only or
                                          read-write access.

             OPEN_FLAGS_FAIL_ON_ERROR     Any function that uses the file
                                          handle returns immediately with an
                                          error value if there is an I/O
                                          error--for example, when the drive
                                          door is open or a sector is
                                          missing. If this value is not
                                          specified, the system passes the
                                          error to the system critical-error
                                          handler, which then reports the
                                          error to the user with a
                                          hard-error popup. The
                                          fail-on-error flag is not
                                          inherited by child processes.

                                          The fail-on-error flag applies to
                                          all functions that use the file
                                          handle, with the exception of the
                                          DosDevIOCtl function.

             OPEN_FLAGS_NOINHERIT         The file handle is not available
                                          to any child process started by
                                          the current process. If this value
                                          is not specified, any child
                                          process started by the current
                                          process may use the file handle.

             OPEN_FLAGS_WRITE_THROUGH     This flag applies to functions,
                                          such as DosWrite, that write data
                                          to the file. If this value is
                                          specified, the system writes data
                                          to the device before the given
                                          function returns. Otherwise, the
                                          system may store the data in an
                                          internal file buffer and write the
                                          data to the device only when the
                                          buffer is full or the file is
                                          closed.

             OPEN_FLAGS_NO_LOCALITY       There is no specific information
                                          regarding the locality of
                                          reference (the degree of
                                          randomness with which the file is
                                          accessed).

             OPEN_FLAGS_SEQUENTIAL        The file is accessed
                                          sequentially.

             OPEN_FLAGS_RANDOM            The file is accessed randomly.

             OPEN_FLAGS_RANDOMSEQUENTIAL  The file is accessed randomly, but
                                          that there is a degree of
                                          sequential I/O within that random
                                          access. For example, this flag is
                                          specified if large blocks of data
                                          are to be read or written at
                                          random locations in the file.

             OPEN_FLAGS_NO_CACHE          The disk drive should not cache
                                          data in I/O operations on this
                                          file.

ulReserved   Specifies a reserved value; must be zero.

Return Value

The return value may be one of the following:

    FDLG_OK
    FDLG_CANCEL

Comments

On exit from this function, hard-error processing will be enabled.
\****************************************************************************/


/****************************************************************************
 *  Save File dialog box procedure                                          *
 ****************************************************************************/
extern USHORT EXPENTRY FileSaveDlg( HWND hwndOwner,
                                    PSZ pszTitle,PSZ pszIns,
                                    void (EXPENTRY *pfnHelpProc)(HWND hDlg),
                                    PSZ pszDefault,
                                    PSZ pszFile,
                                    PHFILE phf,
                                    ULONG ulFileSize,
                                    PUSHORT pusAction,
                                    USHORT usAttribute,
                                    USHORT fsOpenFlags,
                                    USHORT fsOpenMode,
                                    ULONG ulReserved  );
/****************************************************************************\
The FileSaveDlg function displays a dialog box and opens the file selected
by the user. This function returns a handle that can be used to read from
and write to the file, as well as to retrieve information about the file.

Parameters   Description
----------------------------------------------------------------------------
hwndOwner    Identifies the owner window.

pszTitle     Points to the null-terminated string that specifies the
             dialog box title. If pszTitle is NULL then the default value
             "Save File" is used.

pszIns       Points to the null-terminated string that specifies the
             dialog box instructions. If pszIns is NULL then the default
             value "Type filename" is used.

pfnHelpProc  Points to the user supplied help procedure. This procedure
             is called whenever the help button or the F1 key is pressed.
             The HelpProc function is defined as follows:

             void EXPENTRY HelpProc( HWND hDlg )
             The hDlg parameter is a handle to the save file dialog box.

             When the help procedure is called, the identity of the
             dialog box that requested the help can be determined
             by a call to WinQueryWindowUShort(hDlg,QWS_ID). The
             value returned can be one of the following:

                FDLG_OPEN ..... Help requested by open dialog box.
                FDLG_FIND ..... Help requested by find file dialog box.
                FDLG_SAVE ..... Help requested by save file dialog box.

             If no help is available, set the pfnHelpProc parameter to
             NULL. This will cause the Help button to be removed from
             the dialog box.

pszDefault   Points to the null-terminated string that specifies the
             default save file name.

pszFileName  Points to the location where the fully qualified filename of
             the file selected by the user is to be stored. Use the
             DosQSysInfo function to get the maximum path length in order
             to determine how large to make the memory block pointed to
             by pszFile.

phf          Points to the variable that receives the handle of the opened
             file.

pusAction    Points to the variable receiving the value that specifies the
             action taken by the FileSaveDlg function. If the dialog box is
             cancelled then this value has no meaning. Otherwise, it is
             one of the following values:

             Value           Meaning
             ---------------------------------------------------------------
             FILE_CREATED    File was created.

             FILE_EXISTED    File already existed.

             FILE_TRUNCATED  File existed and was truncated.

ulFileSize   Specifies the file's new size (in bytes). This parameter
             applies only if the file is created or truncated. The size
             specification has no effect on a file that is opened only for
             reading.

usAttribute  Specifies the file attributes. This parameter can be a
             combination of the following values:

             Value          Meaning
             ---------------------------------------------------------------
             FILE_NORMAL    File can be read from or written to.

             FILE_READONLY  File can be read from, but not written to.

             FILE_HIDDEN    File is hidden and does not appear in a
                            directory listing.

             FILE_SYSTEM    File is a system file.

             FILE_ARCHIVED  File has been archived.

             File attributes apply only if the file is created.

fsOpenFlags  Specifies the action to take both when the file exists and when
             it does not exist. This parameter may be one of the following
             values:

             Value                        Meaning
             ---------------------------------------------------------------
             FILE_CREATE                  Create a new file; fail if the
                                          file already exists.

             FILE_OPEN                    Open an existing file; fail if the
                                          file does not exist.

             FILE_OPEN | FILE_CREATE      Open an existing file or create
                                          the file if it does not exist.

             FILE_TRUNCATE                Open an existing file and change
                                          to a given size.

             FILE_TRUNCATE | FILE_CREATE  Open an existing file and truncate
                                          it, or create the file if it does
                                          not exist.

fsOpenMode   Specifies the modes with which to open the file. It consists of
             one access mode and one share mode. The other values are option
             and can be given in any combination:

             Value                     Meaning
             ---------------------------------------------------------------
             OPEN_ACCESS_READONLY         Data can be read from the file but
                                          not written to it.

             OPEN_ACCESS_READWRITE        Data can be read from or written
                                          to the file.

             OPEN_ACCESS_WRITEONLY        Data can be written to the file
                                          but not read from it.

             OPEN_SHARE_DENYNONE          Other processes can open the file
                                          for any access: read-only,
                                          write-only, or read-write.

             OPEN_SHARE_DENYREAD          Other processes can open the file
                                          for write-only access but they
                                          cannot open it for read-only or
                                          read-write access.

             OPEN_SHARE_DENYREADWRITE     The current process has exclusive
                                          access to the file. The file
                                          cannot be opened by any process
                                          (including the current process).

             OPEN_SHARE_DENYWRITE         Other processes can open the file
                                          for read-only access but they
                                          cannot open it for write-only or
                                          read-write access.

             OPEN_FLAGS_FAIL_ON_ERROR     Any function that uses the file
                                          handle returns immediately with an
                                          error value if there is an I/O
                                          error--for example, when the drive
                                          door is open or a sector is
                                          missing. If this value is not
                                          specified, the system passes the
                                          error to the system critical-error
                                          handler, which then reports the
                                          error to the user with a
                                          hard-error popup. The
                                          fail-on-error flag is not
                                          inherited by child processes.

                                          The fail-on-error flag applies to
                                          all functions that use the file
                                          handle, with the exception of the
                                          DosDevIOCtl function.

             OPEN_FLAGS_NOINHERIT         The file handle is not available
                                          to any child process started by
                                          the current process. If this value
                                          is not specified, any child
                                          process started by the current
                                          process may use the file handle.

             OPEN_FLAGS_WRITE_THROUGH     This flag applies to functions,
                                          such as DosWrite, that write data
                                          to the file. If this value is
                                          specified, the system writes data
                                          to the device before the given
                                          function returns. Otherwise, the
                                          system may store the data in an
                                          internal file buffer and write the
                                          data to the device only when the
                                          buffer is full or the file is
                                          closed.

             OPEN_FLAGS_NO_LOCALITY       There is no specific information
                                          regarding the locality of
                                          reference (the degree of
                                          randomness with which the file is
                                          accessed).

             OPEN_FLAGS_SEQUENTIAL        The file is accessed
                                          sequentially.

             OPEN_FLAGS_RANDOM            The file is accessed randomly.

             OPEN_FLAGS_RANDOMSEQUENTIAL  The file is accessed randomly, but
                                          that there is a degree of
                                          sequential I/O within that random
                                          access. For example, this flag is
                                          specified if large blocks of data
                                          are to be read or written at
                                          random locations in the file.

             OPEN_FLAGS_NO_CACHE          The disk drive should not cache
                                          data in I/O operations on this
                                          file.

ulReserved   Specifies a reserved value; must be zero.

Return Value

The return value may be one of the following:

    FDLG_OK
    FDLG_CANCEL

Comments

On exit from this function, hard-error processing will be enabled.
\****************************************************************************/
#endif /* _FILEDLG_INCLUDED */
