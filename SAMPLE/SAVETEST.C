/*
 * SAVETEST.C -- Program to test operation of FileSaveDlg function.
 */

#define INCL_DOSFILEMGR
#define INCL_WINWINDOWMGR
#define INCL_WINDIALOGS
#define INCL_DOSMISC
#include <os2.h>
#include <string.h>
#include <malloc.h>
#include <filedlg.h>

void EXPENTRY HelpProc( HWND hwnd );

void main( void )
{
    HAB     hab = WinInitialize( 0 );
    HMQ     hmq = WinCreateMsgQueue( hab,DEFAULT_QUEUE_SIZE );
    USHORT  usPathLen;
    PSZ     pszFile,pszBuf;
    HFILE   hf;
    USHORT  usAction;
    USHORT  usResult;

    DosQSysInfo( 0,(PBYTE)&usPathLen,sizeof(USHORT) );
    pszFile = _fmalloc( usPathLen );
    pszBuf  = _fmalloc( usPathLen );

    usResult = FileSaveDlg( HWND_DESKTOP,
                            NULL,
                            NULL,
                            HelpProc,
                            "unnamed",
                            pszFile,
                            &hf,
                            0L,
                            &usAction,
                            FILE_NORMAL,
                            FILE_OPEN,
                            OPEN_ACCESS_READONLY|OPEN_SHARE_DENYWRITE,
                            0L );

    switch ( usResult ) {
        case FDLG_OK:
            _fstrcpy( pszBuf,pszFile );
            break;

        case FDLG_CANCEL:
            _fstrcpy( pszBuf,"Save file dialog box was cancelled." );
            break;

        default:
            _fstrcpy( pszBuf,"Internal error in FileSaveDlg procedure." );
            break;
        }

    WinMessageBox( HWND_DESKTOP,HWND_DESKTOP,
                   pszBuf,"Debugging information",
                   0,MB_OK|MB_NOICON );

    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
}


void EXPENTRY HelpProc( HWND hwnd )
{
    USHORT  id;
    PSZ     pszMsg;

    switch ( id=WinQueryWindowUShort(hwnd,QWS_ID) ) {
        case FDLG_OPEN: pszMsg = "Help requested by open dialog.";
                        break;

        case FDLG_FIND: pszMsg = "Help requested by find dialog.";
                        break;

        case FDLG_SAVE: pszMsg = "Help requested by save dialog.";
                        break;

        default:        pszMsg = "Internal error in help.";
                        break;
        }

    WinMessageBox( HWND_DESKTOP,hwnd,pszMsg,"Help",0,MB_OK|MB_NOICON );
}
