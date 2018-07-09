/****************************************************************************
 * OPENDLG.C - Open File dialog box routines.                               *
 *                                                                          *
 *  Modifications -                                                         *
 *      11-Aug-1989 : Initial version.                                      *
 *      07-Sep-1989 : Added file find function.                             *
 *      21-Sep-1989 : Cleaned up code to make it a little more readable.    *
 *                    Separated open file and find file dialog procedures   *
 *                      into separate files.                                *
 *                    Forced OPEN button to be default button whenever      *
 *                      the focus passes to a non-pushbutton control.       *
 *                    Eliminated duplicate error messages when enter key    *
 *                      is used to select a list box item.                  *
 *      11-Oct-1989 : Changed to DLL version of ErrMessageBox function.     *
 *      19-Nov-1989 : Fixed bug causing protection violation when the       *
 *                    pszSearchSpec string is located in a read-only        *
 *                    segment.                                              *
 *      11-Oct-1990 : Added long filename support.                          *
 *                    Eliminated ResetDefaultButton function as OS/2 1.21   *
 *                    fixed the bug that rendered it necessary.             *
 *      02-Nov-1990 : Added workaround to eliminate the listing of the      *
 *                    ".." directory in the directory list box when the     *
 *                    current directory is the root. This is needed         *
 *                    because on HPFS drives, ".." is returned as one of    *
 *                    the subdirectories of the root.                       *
 *      14-Nov-1990 : Eliminated bug causing internally stored current      *
 *                      directory to sometimes get out of synch with        *  *
 *                      actual value when an error occurred while opening   *  *
 *                      a file.                                             *
 *                    Disabled hard-error processing while executing        *
 *                      FileOpenDlg functions.                              *
 *                    Added ability to suppress display of error messages   *
 *                      for errors that occur in FillListBoxes().           *
 *                                                                          *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WIN
    #define INCL_DOS
    #define INCL_DOSERRORS
    #define INCL_GPIBITMAPS
    #define INCL_GPIPRIMITIVES
    #include <os2.h>

    #include <string.h>
    #include <errmsg.h>

    #include "filedlg.h"
    #include "dialog.h"
    #define INCL_ARROWS
    #include "tools.h"
    #include "static.h"
    #include "opendata.h"   // definition of structure holding dialog box
                            //  static data (DATA & PDATA types).

/****************************************************************************
 *  Procedure declarations                                                  *
 ****************************************************************************/
    /* Open file dialog box procedures */
    MRESULT EXPENTRY _OpenDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 );
    static MRESULT NEAR OpenInit( HWND hwnd,MPARAM mp2 );
    static MRESULT NEAR DriveListbox( HWND hwnd,USHORT msg,
                                      MPARAM mp1,MPARAM mp2 );
    static MRESULT NEAR DirListbox( HWND hwnd,USHORT msg,
                                    MPARAM mp1,MPARAM mp2 );
    static MRESULT NEAR FileListbox( HWND hwnd,USHORT msg,
                                     MPARAM mp1,MPARAM mp2 );
    static MRESULT NEAR FnameEditCtrl( HWND hwnd,USHORT msg,
                                       MPARAM mp1,MPARAM mp2 );
    static MRESULT NEAR OpenButton( HWND hwnd,PDATA pData );
    static VOID NEAR FillListBoxes( HWND hDlg,PDATA pData,BOOL fError );
    static USHORT NEAR OpenFile( HWND hDlg,PDATA pData );

    /* Find file dialog box procedures */
    extern MRESULT EXPENTRY _FindDlgProc( HWND hwnd,USHORT msg,
                                          MPARAM mp1,MPARAM mp2 );
/****************************************************************************/


/****************************************************************************
 *  FileOpenDlg()                                                           *
 ****************************************************************************/
    USHORT EXPENTRY FileOpenDlg( HWND hwndOwner,
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
                                 ULONG ulReserved )
    {
        USHORT  usMaxPathLen;
        SEL     sel;
        PDATA   pData;
        USHORT  rc;
        HMODULE hmod = 0;

    /* Disable hard-error processing */
        DosError( HARDERROR_DISABLE );

    /* Set pszTitle and pszIns to default if NULL */
        if ( pszTitle == NULL ) pszTitle = szDefOpenTitle;
        if ( pszIns == NULL ) pszIns = szDefOpenIns;

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

    /* Allocate memory for search spec */
        if ( (rc = DosAllocSeg(usMaxPathLen,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData->pszShowSpec = MAKEP(sel,0);

    /* Allocate scratch data area */
        if ( (rc = DosAllocSeg(usMaxPathLen+3,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData->pszShowSpec) );
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData->pszScratch = MAKEP(sel,0);

    /* Allocate storage for current directory string */
        if ( (rc = DosAllocSeg(usMaxPathLen+3,&sel,SEG_NONSHARED)) )
            {
            DosFreeSeg( SELECTOROF(pData->pszShowSpec) );
            DosFreeSeg( SELECTOROF(pData->pszScratch) );
            DosFreeSeg( SELECTOROF(pData) );
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            rc = FDLG_CANCEL;
            goto EXIT;
            }
        pData->pszCurDir = MAKEP(sel,0);

    /* Set current drive and directory to drive and directory listed   */
    /* in show file specification, and store filename portion of spec. */
        strcpy( pData->pszShowSpec,pszShowSpec );
        if ( rc = ParseFileName(pData->pszShowSpec,
                                pData->pszScratch,
                                szStarDotStar) )
            {
            ErrMessageBox( hwndOwner,pszTitle,rc,NULL,0 );
            strcpy( pData->pszShowSpec,szStarDotStar );
            }
        else
            strcpy( pData->pszShowSpec,strrchr(pData->pszScratch,'\\')+1 );

    /* Initialize contents of dialog box data structure */
        pData->pszTitle         = pszTitle;
        pData->pszIns           = pszIns;
        pData->pfnHelpProc      = pfnHelpProc;
        pData->pszFile          = pszFile;
        pData->phf              = phf;
        pData->ulFileSize       = ulFileSize;
        pData->pusAction        = pusAction;
        pData->usAttribute      = usAttribute;
        pData->fsOpenFlags      = fsOpenFlags;
        pData->fsOpenMode       = fsOpenMode;
        pData->ulReserved       = ulReserved;
        pData->usShowAttr       = usShowAttr;
        pData->usMaxPathLen     = usMaxPathLen;
        pData->hmod             = hmod;
        pData->usFocus          = OPEN_FNAME;
        pData->iFirstChar       = 0;
        pData->hbmLeft          = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROW);
        pData->hbmLeftPressed   = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROWDEP);
        pData->hbmLeftDisabled  = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBLFARROWDIS);
        pData->hbmRight         = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROW);
        pData->hbmRightPressed  = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROWDEP);
        pData->hbmRightDisabled = WinGetSysBitmap(HWND_DESKTOP,SBMP_SBRGARROWDIS);

    /* Activate open file dialog box */
        rc = WinDlgBox( HWND_DESKTOP,hwndOwner,_OpenDlgProc,
                        hmod,IDD_OPEN,pData );

    /* Free resources */
        GpiDeleteBitmap( pData->hbmLeft );
        GpiDeleteBitmap( pData->hbmLeftPressed );
        GpiDeleteBitmap( pData->hbmLeftDisabled );
        GpiDeleteBitmap( pData->hbmRight );
        GpiDeleteBitmap( pData->hbmRightPressed );
        GpiDeleteBitmap( pData->hbmRightDisabled );
        DosFreeSeg( SELECTOROF(pData->pszShowSpec) );
        DosFreeSeg( SELECTOROF(pData->pszScratch) );
        DosFreeSeg( SELECTOROF(pData) );

    /* Re-enable hard error processing and return */
EXIT:   DosError( HARDERROR_ENABLE );
        return rc;
    }
/****************************************************************************/


/****************************************************************************
 * _OpenDlgProc()                                                           *
 ****************************************************************************/
    MRESULT EXPENTRY _OpenDlgProc( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;

        switch ( msg ) {
            case WM_INITDLG:
                return OpenInit( hwnd,mp2 );

            case WM_CONTROL:
                switch ( SHORT1FROMMP(mp1) ) {
                    case OPEN_DRIVES:
                        return DriveListbox( hwnd,msg,mp1,mp2 );

                    case OPEN_DIRLIST:
                        return DirListbox( hwnd,msg,mp1,mp2 );

                    case OPEN_FILELIST:
                        return FileListbox( hwnd,msg,mp1,mp2 );

                    case OPEN_FNAME:
                        return FnameEditCtrl( hwnd,msg,mp1,mp2 );

                    case OPEN_LEFTARROW:
                        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                        return LeftArrow( hwnd,msg,mp1,mp2,
                                          pData->hbmLeft,
                                          pData->hbmLeftPressed,
                                          pData->hbmLeftDisabled );

                    case OPEN_RIGHTARROW:
                        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                        return RightArrow( hwnd,msg,mp1,mp2,
                                           pData->hbmRight,
                                           pData->hbmRightPressed,
                                           pData->hbmRightDisabled );
                    }
                break;

            case WM_COMMAND:
                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                switch (COMMANDMSG(&msg)->cmd) {
                    case OPEN_OK:
                        return OpenButton( hwnd,pData );

                    case DID_CANCEL:
                    case OPEN_CANCEL:
                        WinDismissDlg( hwnd,FDLG_CANCEL );
                        return 0L;

                    case OPEN_FIND:
                        if ( FDLG_OK == WinDlgBox(HWND_DESKTOP,hwnd,
                                                  _FindDlgProc,pData->hmod,
                                                  IDD_FIND,pData) )
                            WinDismissDlg( hwnd,FDLG_OK );
                        else
                            WinSetFocus( HWND_DESKTOP,
                                         WinWindowFromID(hwnd,OPEN_FNAME) );
                        return 0L;

                    case OPEN_LEFTARROW:
                        if ( pData->iFirstChar == 0 )
                            WinAlarm( HWND_DESKTOP,WA_ERROR );
                        else
                            {
                            pData->iFirstChar--;
                            WinSetDlgItemText( hwnd,OPEN_CURDIR,
                                               &pData->pszCurDir[pData->iFirstChar] );
                            }
                        return 0L;

                    case OPEN_RIGHTARROW:
                        {
                        size_t len = strlen(pData->pszCurDir);

                        if ( len == 0 || pData->iFirstChar == len-1 )
                            WinAlarm( HWND_DESKTOP,WA_ERROR );
                        else
                            {
                            pData->iFirstChar++;
                            WinSetDlgItemText( hwnd,OPEN_CURDIR,
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
 * OpenInit() -- Process WM_INITDLG message for the open file dialog        *
 *               box.                                                       *
 ****************************************************************************/
    static MRESULT near OpenInit( HWND hwnd,MPARAM mp2 )
    {
        PDATA pData;

        pData = PVOIDFROMMP( mp2 );
        WinSetWindowULong( hwnd,QWL_USER,(ULONG)pData );
        if ( pData->pfnHelpProc == NULL )
            WinDestroyWindow( WinWindowFromID(hwnd,OPEN_HELP) );
        WinSetWindowText( WinWindowFromID(hwnd,FID_TITLEBAR),pData->pszTitle );
        WinSetDlgItemText( hwnd,OPEN_HLPTEXT,pData->pszIns );
        WinSendDlgItemMsg( hwnd,OPEN_FNAME,EM_SETTEXTLIMIT,
                           MPFROMSHORT(pData->usMaxPathLen),NULL );
        FillListBoxes( hwnd,pData,TRUE );
        return 0L;
    }
/****************************************************************************/


/****************************************************************************
 * DriveListbox() -- Handle messages sent by the disk drive list box to     *
 *                   the open file dialog.                                  *
 ****************************************************************************/
    static MRESULT near DriveListbox( HWND hwnd,USHORT msg,
                                      MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        USHORT  usResult;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        switch ( SHORT2FROMMP(mp1) ) {
            case LN_ENTER:
                pData->usFocus = OPEN_DRIVES;
                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(pData->usSelectDrive,pData->usMaxPathLen),
                                   MPFROMP(pData->pszScratch) );
                usResult = DosSelectDisk(pData->pszScratch[0]-'@');
                if ( usResult )
                    ErrMessageBox( hwnd,pData->pszTitle,usResult,NULL,0 );
                else
                    WinSendDlgItemMsg( hwnd,OPEN_OK,BM_CLICK,0L,0L );
                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                   MPFROMSHORT(pData->usSelectDrive),
                                   MPFROMSHORT(TRUE) );
                return 0L;

            case LN_SETFOCUS:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,OPEN_DRIVES,
                                                           LM_QUERYTOPINDEX,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE )
                    {
                    if ( pData->usSelectDrive < usResult )
                        pData->usSelectDrive = usResult;
                    else if (pData->usSelectDrive > usResult+5)
                        pData->usSelectDrive = usResult+5;

                    WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                       MPFROMSHORT(pData->usSelectDrive),
                                       MPFROMSHORT(TRUE) );
                    }
                return 0L;

            case LN_KILLFOCUS:
                WinSendDlgItemMsg( hwnd,OPEN_DRIVES,LM_SELECTITEM,
                                   MPFROMSHORT(pData->usSelectDrive),
                                   MPFROMSHORT(FALSE) );
                return 0L;

            case LN_SELECT:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,OPEN_DRIVES,
                                                           LM_QUERYSELECTION,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE ) pData->usSelectDrive = usResult;
                return 0L;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * DirListbox() -- Handle messages sent by directory list box to the        *
 *                 open file dialog.                                        *
 ****************************************************************************/
    static MRESULT near DirListbox( HWND hwnd,USHORT msg,
                                    MPARAM mp1,MPARAM mp2 )
    {
        PDATA  pData;
        USHORT usResult;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        switch ( SHORT2FROMMP(mp1) ) {
            case LN_ENTER:
                pData->usFocus = OPEN_DIRLIST;
                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(pData->usSelectDir,pData->usMaxPathLen),
                                   MPFROMP(pData->pszScratch) );
                usResult = DosChDir(pData->pszScratch,0L);
                if ( usResult )
                    ErrMessageBox( hwnd,pData->pszTitle,usResult,NULL,0 );
                else
                    WinSendDlgItemMsg( hwnd,OPEN_OK,BM_CLICK,0L,0L );
                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                   MPFROMSHORT(pData->usSelectDir),
                                   MPFROMSHORT(TRUE) );
                return 0L;

            case LN_SETFOCUS:
                usResult = SHORT1FROMMR(WinSendDlgItemMsg(hwnd,OPEN_DIRLIST,
                                                          LM_QUERYTOPINDEX,
                                                          0L,0L)
                                       );
                if ( usResult != LIT_NONE )
                    {
                    if ( pData->usSelectDir < usResult )
                        pData->usSelectDir = usResult;
                    else if (pData->usSelectDir > usResult+4)
                        pData->usSelectDir = usResult+4;

                    WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                       MPFROMSHORT(pData->usSelectDir),
                                       MPFROMSHORT(TRUE) );
                    }
                return 0L;

            case LN_KILLFOCUS:
                WinSendDlgItemMsg( hwnd,OPEN_DIRLIST,LM_SELECTITEM,
                                   MPFROMSHORT(pData->usSelectDir),
                                   MPFROMSHORT(FALSE) );
                return 0L;

            case LN_SELECT:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,OPEN_DIRLIST,
                                                           LM_QUERYSELECTION,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE ) pData->usSelectDir = usResult;
                return 0L;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * FileListbox() -- Handle messages sent by file list box the open dialog.  *
 ****************************************************************************/
    static MRESULT near FileListbox( HWND hwnd,USHORT msg,
                                     MPARAM mp1,MPARAM mp2 )
    {
        PDATA  pData;
        USHORT usResult;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        switch ( SHORT2FROMMP(mp1) ) {
            case LN_SELECT:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,OPEN_FILELIST,
                                                           LM_QUERYSELECTION,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE ) pData->usSelectFile = usResult;
                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(pData->usSelectFile,pData->usMaxPathLen),
                                   MPFROMP(pData->pszScratch) );
                WinSetDlgItemText( hwnd,OPEN_FNAME,
                                   pData->pszScratch );
                return 0L;

            case LN_ENTER:
                pData->usFocus = OPEN_FILELIST;
                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(pData->usSelectFile,pData->usMaxPathLen),
                                   MPFROMP(pData->pszScratch) );
                WinSetDlgItemText( hwnd,OPEN_FNAME,pData->pszScratch );
                WinSendDlgItemMsg( hwnd,OPEN_OK,BM_CLICK,0L,0L );
                return 0L;

            case LN_SETFOCUS:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,OPEN_FILELIST,
                                                           LM_QUERYTOPINDEX,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE )
                    {
                    if ( pData->usSelectFile < usResult )
                        pData->usSelectFile = usResult;
                    else if (pData->usSelectFile > usResult+7)
                        pData->usSelectFile = usResult+7;

                    WinSendDlgItemMsg( hwnd,OPEN_FILELIST,LM_SELECTITEM,
                                       MPFROMSHORT(pData->usSelectFile),
                                       MPFROMSHORT(TRUE) );
                    }
                return 0L;

            case LN_KILLFOCUS:
                WinSendDlgItemMsg( hwnd,OPEN_FILELIST,LM_SELECTITEM,
                                   MPFROMSHORT(pData->usSelectFile),
                                   MPFROMSHORT(FALSE) );
                return 0L;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * FnameEditCtrl() -- Handles messages sent by OPEN_FNAME edit control      *
 *                    to open file dialog box.                              *
 ****************************************************************************/
    static MRESULT near FnameEditCtrl( HWND hwnd,USHORT msg,
                                       MPARAM mp1,MPARAM mp2 )
    {
        USHORT usResult;
        PDATA  pData;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );

        if ( SHORT2FROMMP(mp1) == EN_SETFOCUS )
            {
            usResult = WinQueryDlgItemTextLength( hwnd,OPEN_FNAME );
            WinSendDlgItemMsg( hwnd,OPEN_FNAME,EM_SETSEL,
                               MPFROM2SHORT(0,usResult),0L );
            return 0L;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * OpenButton() - Procedure to executed when OPEN button is clicked.        *
 ****************************************************************************/
    static MRESULT near OpenButton( HWND hwnd,PDATA pData )
    {
        USHORT      usDriveNum;
        ULONG       ulMap;
        USHORT      usCount;
        HWND        hwndButton;

        hwndButton = WinWindowFromID( hwnd,OPEN_OK );

        if ( hwndButton == WinQueryFocus(HWND_DESKTOP,FALSE) )
            {
            switch ( pData->usFocus ) {
                case OPEN_FNAME:
                case OPEN_FILELIST:
                    WinQueryDlgItemText( hwnd,OPEN_FNAME,
                                         pData->usMaxPathLen,
                                         pData->pszScratch );
                    if ( OpenFile(hwnd,pData) )
                        {
                        DosQCurDisk( &usDriveNum,&ulMap );
                        pData->pszScratch[0] = (CHAR)( usDriveNum + '@' );
                        pData->pszScratch[1] = ':';
                        pData->pszScratch[2] = '\\';
                        pData->pszScratch[3] = 0;
                        usCount = pData->usMaxPathLen-3;
                        DosQCurDir(0,pData->pszScratch+3,&usCount);
                        if ( 0 != stricmp(pData->pszScratch,pData->pszCurDir) )
                            FillListBoxes( hwnd,pData,FALSE );
                        }
                    else
                        WinDismissDlg( hwnd,FDLG_OK );
                    break;

                case OPEN_DRIVES:
                case OPEN_DIRLIST:
                    FillListBoxes( hwnd,pData,TRUE );
                    break;
                }
            WinSetFocus( HWND_DESKTOP,WinWindowFromID(hwnd,pData->usFocus) );
            pData->usFocus = OPEN_FNAME;
            }

        return 0L;
    }
/****************************************************************************/


/****************************************************************************
 * FillListBoxes() - Fill drive, directory, and file list boxes, and        *
 *                   display the name of the current drive and directory    *
 *                   in the OPEN_CURDIR control.                            *
 *                                                                          *
 *                   This function uses the scratch data area.              *
 ****************************************************************************/
    static void near FillListBoxes( HWND hDlg,PDATA pData,BOOL fError )
    {
        USHORT      usDriveNum;
        ULONG       ulMap;
        USHORT      usResult;
        HDIR        hdir;
        FILEFINDBUF findbuf;
        USHORT      usCount;

    /* Clear current contents of list boxes and text controls */
        WinSendDlgItemMsg( hDlg,OPEN_DRIVES,LM_DELETEALL,NULL,NULL );
        WinSendDlgItemMsg( hDlg,OPEN_DIRLIST,LM_DELETEALL,NULL,NULL );
        WinSendDlgItemMsg( hDlg,OPEN_FILELIST,LM_DELETEALL,NULL,NULL );
        WinSetDlgItemText( hDlg,OPEN_CURDIR,"" );
        WinSetDlgItemText( hDlg,OPEN_FNAME,pData->pszShowSpec );

        pData->usSelectDrive = 0;
        pData->usSelectDir   = 0;
        pData->usSelectFile  = 0;
        pData->pszCurDir[0]  = 0;
        pData->iFirstChar    = 0;

    /* Fill in disk drive list box */
        if ( usResult = DosQCurDisk(&usDriveNum,&ulMap) )
            {
            if ( fError ) ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return;
            }

        pData->pszScratch[1] = ':';
        pData->pszScratch[2] = '\0';
        for ( usCount = 0; usCount < 26; usCount++ )
            if ( ulMap & 1L << usCount )
                {
                pData->pszScratch[0] = (CHAR)( usCount + 'A' );

                usResult = SHORT1FROMMR(
                            WinSendDlgItemMsg( hDlg,OPEN_DRIVES,
                                               LM_INSERTITEM,
                                               MPFROMSHORT(LIT_END),
                                               MPFROMP(pData->pszScratch) )
                                       );
                if ( usCount == usDriveNum-1 )
                    {
                    WinSendDlgItemMsg( hDlg,OPEN_DRIVES,
                                       LM_SETTOPINDEX,
                                       MPFROMSHORT(usResult),
                                       0L );
                    pData->usSelectDrive = usResult;
                    }
                }

    /* Set OPEN_CURDIR static text control to current drive/directory */
        pData->pszCurDir[0] = (CHAR)( usDriveNum + '@' );
        pData->pszCurDir[1] = ':';
        pData->pszCurDir[2] = '\\';
        pData->pszCurDir[3] = 0;
        usCount = pData->usMaxPathLen-3;
        usResult = DosQCurDir(0,pData->pszCurDir+3,&usCount);
        WinSetDlgItemText( hDlg,OPEN_CURDIR,pData->pszCurDir );
        if ( usResult )
            {
            if ( fError ) ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return;
            }

    /* Fill list box with subdirectories of current directory */
        hdir    = HDIR_CREATE;
        usCount = 1;
        usResult = DosFindFirst( szStarDotStar,&hdir,FILE_DIRECTORY,&findbuf,
                                  sizeof(findbuf),&usCount,0L );

        while ( !usResult )
            {
            if (   (findbuf.attrFile & FILE_DIRECTORY)
                && !(findbuf.achName[0] == '.' && (   findbuf.achName[1]==0
                                                   || pData->pszCurDir[3]==0)
                    )
               )
                {
                WinSendDlgItemMsg( hDlg,OPEN_DIRLIST,LM_INSERTITEM,
                                   MPFROMSHORT(LIT_END),
                                   MPFROMP(findbuf.achName) );
                }
            usResult = DosFindNext( hdir,&findbuf,sizeof(findbuf),&usCount );
            }

        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if ( usResult && usResult != ERROR_NO_MORE_FILES )
            {
            if ( fError ) ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return;
            }

    /* Fill file list box with list of files that match search specs. */
        hdir    = HDIR_CREATE;
        usCount = 1;
        usResult = DosFindFirst( pData->pszShowSpec,&hdir,
                                 pData->usShowAttr,&findbuf,
                                 sizeof(findbuf),&usCount,0L );

        while ( !usResult )
            {
            WinSendDlgItemMsg( hDlg,OPEN_FILELIST,LM_INSERTITEM,
                               MPFROMSHORT(LIT_END),
                               MPFROMP(findbuf.achName) );

            usResult = DosFindNext( hdir,&findbuf,sizeof(findbuf),&usCount );
            }

        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if ( usResult && usResult != ERROR_NO_MORE_FILES )
            if ( fError ) ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );

    /* Done. Return to caller. */
        return;
    }
/****************************************************************************/


/****************************************************************************
 * OpenFile() - This function attempts to open the file specified           *
 *              in the scratch data area, or if the file is a search        *
 *              specification, updates the contents of the list boxes.      *
 *                                                                          *
 *              This function returns a non-zero value if an error occured  *
 *              or the input string was a search specification.             *
 ****************************************************************************/
    static USHORT near OpenFile( HWND hDlg,PDATA pData )
    {
        USHORT  usResult;

        usResult = ParseFileName( pData->pszScratch,
                                  pData->pszFile,
                                  pData->pszShowSpec );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return 1;
            }

        if ( NULL != strpbrk(pData->pszFile,szWildCardChars) )
            {
            strcpy( pData->pszShowSpec,strrchr(pData->pszFile,'\\')+1 );
            FillListBoxes( hDlg,pData,TRUE );
            return 1;
            }
        else
            {
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
    }
/****************************************************************************/
