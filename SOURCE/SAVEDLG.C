/****************************************************************************
 * SAVEDLG.C - Save File dialog box routines.                               *
 *                                                                          *
 *  Modifications -                                                         *
 *      21-Aug-1989 : Initial version.                                      *
 *      08-Sep-1989 : Fixed failure to update current directory display to  *
 *                    reflect drive/directory changes after an attempt to   *
 *                    open a file.                                          *
 *      21-Sep-1989 : Restored SAVE button as default button when the       *
 *                    focus passes to a non-button control.                 *
 *                    Returned focus to file name edit control after        *
 *                    an error occurs while opening the file.               *
 *      11-Oct-1989 : Changed to DLL version of ErrMessageBox function.     *
 *      19-Nov-1989 : Fixed protection violation caused when default        *
 *                    filename string is in a read-only segment.            *
 *      11-Oct-1990 : Added long file name support.                         *
 *                    Eliminated code to set SAVE button as default         *
 *                    button when focus passes to file name edit control.   *
 *      14-Nov-1990 : Disabled hard-error processing while executing        *
 *                      FileSaveDlg function.                               *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WIN
    #define INCL_DOS
    #define INCL_GPIBITMAPS
    #include <os2.h>

    #include <string.h>
    #include <io.h>
    #include <errmsg.h>

    #include "filedlg.h"
    #include "dialog.h"
    #define INCL_ARROWS
    #include "tools.h"
    #include "static.h"

/****************************************************************************
 *  Internal data structure definitions                                     *
 ****************************************************************************/
    typedef struct {
        PSZ     pszTitle;       // dialog box title
        PSZ     pszIns;         // dialog box instructions
        void (EXPENTRY *pfnHelpProc)(HWND hDlg); // ptr to help procedure
        PSZ     pszFile;        // ptr to name of opened file
        PHFILE  phf;            // ptr to file handle
        ULONG   ulFileSize;     // initial file size
        PUSHORT pusAction;      // action taken on open
        USHORT  usAttribute;    // file attribute
        USHORT  fsOpenFlags;    // open flags
        USHORT  fsOpenMode;     // open mode
        ULONG   ulReserved;     // reserved
        USHORT  usMaxPathLen;   // maximum path name length
        PSZ     pszScratch;     // ptr to scratch data area
        PSZ     pszCurDir;          // pointer to current directory string.
        USHORT  iFirstChar;         // index of first character of current
                                    //  directory string that is to be displayed.
        HBITMAP hbmLeft;            // Left arrow bitmap handle.
        HBITMAP hbmLeftPressed;     // Pressed left arrow bitmap handle.
        HBITMAP hbmLeftDisabled;    // Disabled left arrow bitmap handle.
        HBITMAP hbmRight;           // Right arrow bitmap handle.
        HBITMAP hbmRightPressed;    // Pressed right arrow bitmap handle.
        HBITMAP hbmRightDisabled;   // Disabled right arrow bitmap handle.
        } DATA;

    typedef DATA * PDATA;
/****************************************************************************/


/****************************************************************************
 *  Internal procedure declarations                                         *
 ****************************************************************************/
    MRESULT EXPENTRY _SaveDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 );
    static USHORT NEAR OpenFile( HWND hDlg,PDATA pData );
    static VOID NEAR UpdateDir( HWND hDlg,PDATA pData );
/****************************************************************************/


/****************************************************************************
 *  FileOpenDlg()                                                           *
 ****************************************************************************/
    USHORT EXPENTRY FileSaveDlg( HWND hwndOwner,
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
                                 ULONG ulReserved )
    {
        USHORT  usMaxPathLen;
        SEL     sel;
        PDATA   pData;
        USHORT  rc;
        HMODULE hmod = 0;
        PSZ     pszTemp;

   /* Disable hard-error processing */
        DosError( HARDERROR_DISABLE );

   /* Set pszTitle and pszIns to default if NULL */
        if ( pszTitle == NULL ) pszTitle = szDefSaveTitle;
        if ( pszIns == NULL ) pszIns = szDefSaveIns;

    /* Get maximum pathname length */
        DosQSysInfo( 0,(PBYTE)&usMaxPathLen,sizeof(USHORT) );

    /* Get module handle for FILEDLG dynamic-link library
     *  Note: Remove this code section if the file dialog box
     *        resources are not located in a dynamic-link library.
     */
        if ( (rc = DosGetModHandle(szDLLName,&hmod)) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }

    /* Allocate memory for dialog data */
        if ( (rc = DosAllocSeg(sizeof(DATA),&sel,SEG_NONSHARED)) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData = MAKEP(sel,0);

    /* Allocate scratch data areas */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData->pszScratch = MAKEP(sel,0);

    /* Allocate memory to store current directory */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData->pszScratch) );
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData->pszCurDir = MAKEP(sel,0);

    /* Set current drive and directory to drive and directory listed         */
    /* in default file name, and store filename portion in scratch data area */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData->pszScratch) );
            DosFreeSeg( SELECTOROF(pData->pszCurDir) );
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pszTemp = MAKEP(sel,0);
        strcpy( pszTemp,pszDefault );
        if ( rc = ParseFileName(pszTemp,pData->pszScratch,szUnnamed) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            strcpy( pData->pszScratch,szUnnamed );
            }
        else
            strcpy( pData->pszScratch,strrchr(pData->pszScratch,'\\')+1 );
        DosFreeSeg( SELECTOROF(pszTemp) );

    /* Initialize contents of dialog box data structure */
        pData->pszTitle     = pszTitle;
        pData->pszIns       = pszIns;
        pData->pfnHelpProc  = pfnHelpProc;
        pData->pszFile      = pszFile;
        pData->phf          = phf;
        pData->ulFileSize   = ulFileSize;
        pData->pusAction    = pusAction;
        pData->usAttribute  = usAttribute;
        pData->fsOpenFlags  = fsOpenFlags;
        pData->fsOpenMode   = fsOpenMode;
        pData->ulReserved   = ulReserved;
        pData->usMaxPathLen = usMaxPathLen;
        pData->hbmLeft          = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROW);
        pData->hbmLeftPressed   = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROWDEP);
        pData->hbmLeftDisabled  = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROWDIS);
        pData->hbmRight         = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROW);
        pData->hbmRightPressed  = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROWDEP);
        pData->hbmRightDisabled = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROWDIS);

    /* Activate open file dialog box */
        rc = WinDlgBox( HWND_DESKTOP,hwndOwner,_SaveDlgProc,
                        hmod,IDD_SAVE,pData );

    /* Free resources */
        GpiDeleteBitmap( pData->hbmLeft );
        GpiDeleteBitmap( pData->hbmLeftPressed );
        GpiDeleteBitmap( pData->hbmLeftDisabled );
        GpiDeleteBitmap( pData->hbmRight );
        GpiDeleteBitmap( pData->hbmRightPressed );
        GpiDeleteBitmap( pData->hbmRightDisabled );
        DosFreeSeg( SELECTOROF(pData->pszCurDir) );
        DosFreeSeg( SELECTOROF(pData->pszScratch) );
        DosFreeSeg( SELECTOROF(pData) );

    /* re-enable hard error processing and return */
EXIT:   DosError( HARDERROR_ENABLE );
        return rc;
    }
/****************************************************************************/


/****************************************************************************
 * SaveDlgProc()                                                            *
 ****************************************************************************/
    MRESULT EXPENTRY _SaveDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        USHORT  usResult;

        switch ( msg ) {
            case WM_INITDLG:
                pData = PVOIDFROMMP( mp2 );
                WinSetWindowULong( hwnd,QWL_USER,(ULONG)pData );
                if ( pData->pfnHelpProc == NULL )
                    WinDestroyWindow( WinWindowFromID(hwnd,SAVE_HELP) );
                WinSetWindowText( WinWindowFromID(hwnd,FID_TITLEBAR),
                                  pData->pszTitle );
                WinSetDlgItemText( hwnd,SAVE_HLPTEXT,pData->pszIns );
                WinSendDlgItemMsg( hwnd,SAVE_FNAME,EM_SETTEXTLIMIT,
                                   MPFROMSHORT(pData->usMaxPathLen),NULL );
                WinSetDlgItemText( hwnd,SAVE_FNAME,pData->pszScratch );
                UpdateDir( hwnd,pData );
                return 0;

            case WM_CONTROL:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                switch ( SHORT1FROMMP(mp1) ) {
                    case SAVE_FNAME:
                        if ( SHORT2FROMMP(mp1) == EN_SETFOCUS )
                            {
                            usResult = WinQueryDlgItemTextLength( hwnd,SAVE_FNAME );
                            WinSendDlgItemMsg( hwnd,SAVE_FNAME,EM_SETSEL,
                                               MPFROM2SHORT(0,usResult),0L );
                            return 0L;
                            }

                    case SAVE_LEFTARROW:
                        return LeftArrow(hwnd,msg,mp1,mp2,
                                         pData->hbmLeft,
                                         pData->hbmLeftPressed,
                                         pData->hbmLeftDisabled);


                    case SAVE_RIGHTARROW:
                        return RightArrow(hwnd,msg,mp1,mp2,
                                          pData->hbmRight,
                                          pData->hbmRightPressed,
                                          pData->hbmRightDisabled);
                    }
                break;

            case WM_COMMAND:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                switch (COMMANDMSG(&msg)->cmd) {
                    case DID_OK:
                    case SAVE_OK:
                        WinQueryDlgItemText( hwnd,SAVE_FNAME,
                                             pData->usMaxPathLen,
                                             pData->pszScratch );
                        if ( !OpenFile(hwnd,pData) )
                            WinDismissDlg( hwnd,FDLG_OK );
                        else
                            {
                            UpdateDir( hwnd,pData );
                            WinSetFocus( HWND_DESKTOP,
                                         WinWindowFromID(hwnd,SAVE_FNAME) );
                            }
                        return 0;

                    case DID_CANCEL:
                    case SAVE_CANCEL:
                        WinDismissDlg( hwnd,FDLG_CANCEL );
                        return 0;

                    case SAVE_LEFTARROW:
                        if ( pData->iFirstChar == 0 )
                            WinAlarm( HWND_DESKTOP,WA_ERROR );
                        else
                            {
                            pData->iFirstChar--;
                            WinSetDlgItemText( hwnd,SAVE_CURDIR,
                                               &pData->pszCurDir[pData->iFirstChar] );
                            }
                        return 0L;

                    case SAVE_RIGHTARROW:
                        {
                        size_t len = strlen(pData->pszCurDir);

                        if ( len == 0 || pData->iFirstChar == len-1 )
                            WinAlarm( HWND_DESKTOP,WA_ERROR );
                        else
                            {
                            pData->iFirstChar++;
                            WinSetDlgItemText( hwnd,SAVE_CURDIR,
                                               &pData->pszCurDir[pData->iFirstChar] );
                            }
                        return 0L;
                        }
                    }
                break;

            case WM_HELP:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                if ( pData->pfnHelpProc != NULL )
                    {
                    (*pData->pfnHelpProc)( hwnd );
                    return 0L;
                    }
                break;
            }
        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * OpenFile() - This function attempts to open the file specified           *
 *              in the scratch data area.                                   *
 *                                                                          *
 *              This function returns a non-zero value if an error occured. *
 ****************************************************************************/
    static USHORT NEAR OpenFile( HWND hDlg,PDATA pData )
    {
        USHORT  usResult;

        usResult = ParseFileName( pData->pszScratch,
                                  pData->pszFile,
                                  szStarDotStar );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return 1;
            }

        if ( 0 == access(pData->pszFile,0) )
            {
            switch ( ErrMessageBox(hDlg,pData->pszTitle,
                                   OVERWRITE_MSG,
                                   appMsgList,appMsgCount) )
                {
                case MBID_YES:      break;

                case MBID_CANCEL:   WinDismissDlg( hDlg,FDLG_CANCEL );
                                    return 1;

                default:            return 1;
                }
            }

        usResult = DosOpen( pData->pszFile,
                            pData->phf,
                            pData->pusAction,
                            pData->ulFileSize,
                            pData->usAttribute,
                            pData->fsOpenFlags,
                            pData->fsOpenMode,
                            pData->ulReserved );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return 1;
            }
        else
            return 0;
    }
/****************************************************************************/


/****************************************************************************
 * UpdateDir() - This function updates the current directory display        *
 *               to reflect changes in the current drive/directory.         *
 ****************************************************************************/
    static VOID NEAR UpdateDir( HWND hwnd,PDATA pData )
    {
        USHORT  usResult;
        USHORT  usCount;
        USHORT  usDriveNum;
        ULONG   ulMap;

        pData->iFirstChar = 0;

        if ( usResult = DosQCurDisk(&usDriveNum,&ulMap) )
            {
            WinSetDlgItemText( hwnd,SAVE_CURDIR,"" );
            pData->pszCurDir = 0;
            ErrMessageBox( hwnd,pData->pszTitle,usResult,NULL,0 );
            return;
            }

        pData->pszCurDir[0] = (CHAR)( usDriveNum + '@' );
        pData->pszCurDir[1] = ':';
        pData->pszCurDir[2] = '\\';
        pData->pszCurDir[3] = '\0';

        usCount = pData->usMaxPathLen-3;
        if ( usResult = DosQCurDir(0,pData->pszCurDir+3,&usCount) )
            ErrMessageBox( hwnd,pData->pszTitle,usResult,NULL,0 );

        WinSetDlgItemText( hwnd,SAVE_CURDIR,pData->pszCurDir );

        return;
    }
/****************************************************************************/
