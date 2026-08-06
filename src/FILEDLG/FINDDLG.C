/****************************************************************************
 * FINDDLG.C - Find file dialog box routines.                               *
 *                                                                          *
 *  Modifications -                                                         *
 *      21-Sep-1989 : Moved find file functions into separate file.         *
 *                    Made default button context sensitive.                *
 *                    Prevented buttons from retaining focus after being    *
 *                      clicked.                                            *
 *                    Fixed bug that caused duplicate error messages to     *
 *                      be displayed when enter key used to select list     *
 *                      box items.                                          *
 *      11-Oct-1989 : Changed to DLL version of ErrMessageBox function.     *
 *      11-Oct-1990 : Added long filename support.                          *
 *                    Eliminated ResetDefaultButtons function.              *
 *                    Fixed bug that caused contents of open file dialog    *
 *                    to be invalid if an unsuccessful attempt was made     *
 *                    to open a file located in a directory other than      *
 *                    the current directory.                                *
 *      15-Oct-1990 : Moved search function into a background thread.       *
 *      16-Oct-1990 : Added ability to search for files that match one      *
 *                    of a list of search patterns.                         *
 *      06-Nov-1990 : Increased speed of search function by causing         *
 *                    DosFindFirst/DosFindNext to return the list of        *
 *                    found files in blocks of 10.                          *
 *      12-Nov-1990 : Added help support.                                   *
 *      Ported to 32-bit OS/2: replaced 16-bit APIs (DosAllocSeg, MAKEP,   *
 *      DosFreeSeg, DosSemSet/Clear/Wait, DosQCurDisk) with 32-bit         *
 *      equivalents. Removed near/far/SEL modifiers. Updated FILEFINDBUF   *
 *      to FILEFINDBUF3 and buffer traversal to use oNextEntryOffset.      *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WIN
    #define INCL_DOS
    #define INCL_DOSERRORS
    #include <os2.h>

    #include <string.h>
    #include <process.h>
    #include <errmsg.h>

    #include "filedlg.h"
    #include "dialog.h"
    #include "tools.h"
    #include "static.h"
    #include "opendata.h"   // definition of structure holding dialog box
                            //  static data (DATA & PDATA types).

/****************************************************************************
 *  Procedure declarations                                                  *
 ****************************************************************************/
    static MRESULT FindInit( HWND hwnd,MPARAM mp2 );
    static MRESULT PatternEditCtrl( HWND hwnd,ULONG msg,
                                    MPARAM mp1,MPARAM mp2 );
    static MRESULT FindListbox( HWND hwnd,ULONG msg,
                                MPARAM mp1,MPARAM mp2 );
    static MRESULT SearchButton( HWND hwnd );
    static MRESULT OpenButton( HWND hwnd );
    static BOOL Directory( PSZ pszFileSpec,PDATA pData );
    static USHORT OpenFile( HWND hDlg,PDATA pData );
    static VOID SearchThread( VOID *pArg );
/****************************************************************************/


/****************************************************************************
 *  Definitions of messages sent from search thread to dialog box.          *
 ****************************************************************************/
    #define SEARCH_DONE         (WM_USER)
    /* if mp1 == 0 then mp2 contains OS/2 error code.
     */

    #define SEARCH_ADDDIR       (WM_USER+1)
    /* mp1 contains a pointer to the directory name to be added
     * to the list box.
     */

    #define SEARCH_ADDFILES     (WM_USER+2)
    /* mp1 contains a pointer to an array of FILEFINDBUF3 structures.
     * mp2 contains the count of entries in the array.
     */
/****************************************************************************/


/****************************************************************************
 * _FindDlgProc()                                                           *
 ****************************************************************************/
    MRESULT EXPENTRY _FindDlgProc( HWND hwnd,ULONG msg,MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;

        switch ( msg ) {
            case WM_INITDLG:
                return FindInit( hwnd,mp2 );

            case WM_CONTROL:
                switch ( SHORT1FROMMP(mp1) ) {
                    case FIND_PATTERN:
                        return PatternEditCtrl( hwnd,msg,mp1,mp2 );

                    case FIND_FLIST:
                        return FindListbox( hwnd,msg,mp1,mp2 );

                    case FIND_DRIVES:
                        if ( SHORT2FROMMP(mp1) == LN_SETFOCUS )
                            {
                            WinSendDlgItemMsg( hwnd,FIND_SEARCH,BM_SETDEFAULT,
                                               MPFROMSHORT(TRUE),0L );
                            WinSendDlgItemMsg( hwnd,FIND_OPEN,BM_SETDEFAULT,
                                               MPFROMSHORT(FALSE),0L );
                            WinSendDlgItemMsg( hwnd,FIND_CANCEL,BM_SETDEFAULT,
                                               MPFROMSHORT(FALSE),0L );
                            return 0L;
                            }
                        break;
                    }
                break;

            case WM_COMMAND:
                switch (COMMANDMSG(&msg)->cmd) {
                    case FIND_SEARCH:
                        return SearchButton( hwnd );

                    case FIND_OPEN:
                        return OpenButton( hwnd );

                    case DID_CANCEL:
                    case FIND_CANCEL:
                        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                        DosEnterCritSec();
                        switch ( pData->fControl ) {
                            case CANCEL:
                                pData->fControl = TERMINATE;
                                DosPostEventSem( pData->hevTrigger );
                                DosExitCritSec();
                                DosWaitEventSem( pData->hevTerminate,SEM_INDEFINITE_WAIT );
                                DosFreeMem( pData->pszPattern );
                                DosFreeMem( pData->pszScratch2 );
                                DosCloseEventSem( pData->hevTrigger );
                                DosCloseEventSem( pData->hevTerminate );
                                DosCloseEventSem( pData->hevAdd );
                                WinDismissDlg( hwnd,FDLG_CANCEL );
                                break;

                            case CONTINUE:
                                DosPostEventSem( pData->hevAdd );
                                pData->fControl = CANCEL;
                                DosExitCritSec();
                                break;

                            default:
                                DosExitCritSec();
                                break;
                            }
                        return 0L;
                    }
                break;

            case SEARCH_DONE:
                {
                SHORT sCount;

                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                WinSetWindowText( hwnd,szSearchTitle );
                WinEnableWindow( WinWindowFromID(hwnd,FIND_SEARCH),TRUE );
                sCount = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_FLIST,
                                                         LM_QUERYITEMCOUNT,
                                                         0L,0L)
                                     );
                if ( SHORT1FROMMP(mp1) != 0 )
                    ErrMessageBox( hwnd,szSearchTitle,SHORT1FROMMP(mp1),
                                   appMsgList,appMsgCount,pData->usFindCount );
                else
                    ErrMessageBox( hwnd,szSearchTitle,SHORT1FROMMP(mp2),
                                   NULL,0 );
                return 0L;
                }

            case SEARCH_ADDDIR:
                {
                SHORT   rc;

                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                if ( pData->fControl == CONTINUE )
                    {
                    rc = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_FLIST,
                                                         LM_INSERTITEM,
                                                         MPFROMSHORT(LIT_END),
                                                         mp1)
                                     );
                    if ( rc == LIT_MEMERROR || rc == LIT_ERROR )
                        {
                        pData->fControl = ERROR;
                        WinPostMsg( hwnd,SEARCH_DONE,MPFROMSHORT(LISTBOX_FULL),0L );
                        }
                    }
                DosPostEventSem( pData->hevAdd );
                return 0L;
                }

            case SEARCH_ADDFILES:
                {
                SHORT         rc;
                ULONG         n;
                PFILEFINDBUF3 pFindbuf;

                pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
                if ( pData->fControl == CONTINUE )
                    {
                    pData->pszScratch2[0] = ' ';
                    pData->pszScratch2[1] = ' ';
                    pData->pszScratch2[2] = ' ';
                    pFindbuf = PVOIDFROMMP( mp1 );
                    for ( n = 0; n < (ULONG)mp2; n++ )
                        {
                        strcpy( pData->pszScratch2+3,pFindbuf->achName );
                        rc = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_FLIST,
                                                             LM_INSERTITEM,
                                                             MPFROMSHORT(LIT_END),
                                                             pData->pszScratch2)
                                         );
                        if ( rc == LIT_MEMERROR || rc == LIT_ERROR )
                            {
                            pData->fControl = ERROR;
                            WinPostMsg( hwnd,SEARCH_DONE,MPFROMSHORT(LISTBOX_FULL),0L );
                            break;
                            }
                        pFindbuf = (PFILEFINDBUF3)((PBYTE)pFindbuf + pFindbuf->oNextEntryOffset);
                        }
                    }
                DosPostEventSem( pData->hevAdd );
                return 0L;
                }

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
 * FindInit() -- Process WM_INITDLG message sent to find file dialog box.   *
 ****************************************************************************/
    static MRESULT FindInit( HWND hwnd,MPARAM mp2 )
    {
        #define STACK_SIZE  (8*1024)

        PDATA   pData;
        APIRET  usResult;
        USHORT  usCount,usIndex;
        ULONG   usDriveNum;
        ULONG   ulMap;

        pData = PVOIDFROMMP( mp2 );

        if ( pData->pfnHelpProc == NULL )
            WinDestroyWindow( WinWindowFromID(hwnd,FIND_HELP) );

        if ( !WinSendDlgItemMsg(hwnd,FIND_PATTERN,EM_SETTEXTLIMIT,
                                MPFROMSHORT(pData->usMaxPathLen),0L) )
            {
            ErrMessageBox( hwnd,szSearchTitle,ERROR_NOT_ENOUGH_MEMORY,NULL,0 );
            WinDismissDlg( hwnd,FDLG_CANCEL );
            return 0L;
            }

        if ( usResult = DosAllocMem((PPVOID)&pData->pszPattern,pData->usMaxPathLen,
                                    PAG_COMMIT|PAG_READ|PAG_WRITE) )
            {
            ErrMessageBox( hwnd,szSearchTitle,(USHORT)usResult,NULL,0 );
            WinDismissDlg( hwnd,FDLG_CANCEL );
            return 0L;
            }
        if ( usResult = DosAllocMem((PPVOID)&pData->pszScratch2,pData->usMaxPathLen,
                                    PAG_COMMIT|PAG_READ|PAG_WRITE) )
            {
            ErrMessageBox( hwnd,szSearchTitle,(USHORT)usResult,NULL,0 );
            DosFreeMem( pData->pszPattern );
            WinDismissDlg( hwnd,FDLG_CANCEL );
            return 0L;
            }

        DosCreateEventSem( NULL,&pData->hevTrigger,0,FALSE );
        DosCreateEventSem( NULL,&pData->hevTerminate,0,FALSE );
        DosCreateEventSem( NULL,&pData->hevAdd,0,FALSE );
        pData->hDlg     = hwnd;
        pData->fControl = CANCEL;

        pData->tid = _beginthread( SearchThread,NULL,STACK_SIZE,pData );
        if ( -1 == (int)pData->tid )
            {
            ErrMessageBox( hwnd,szSearchTitle,SEARCH_THREAD_ERROR,
                           appMsgList,appMsgCount );
            DosFreeMem( pData->pszPattern );
            DosFreeMem( pData->pszScratch2 );
            DosCloseEventSem( pData->hevTrigger );
            DosCloseEventSem( pData->hevTerminate );
            DosCloseEventSem( pData->hevAdd );
            WinDismissDlg( hwnd,FDLG_CANCEL );
            return 0L;
            }

        WinSetWindowULong( hwnd,QWL_USER,(ULONG)pData );
        WinSendDlgItemMsg( hwnd,FIND_FLIST,LM_DELETEALL,NULL,NULL );
        WinSendDlgItemMsg( hwnd,FIND_DRIVES,LM_DELETEALL,NULL,NULL );
        WinSetDlgItemText( hwnd,FIND_PATTERN,pData->pszShowSpec );
        WinEnableWindow( WinWindowFromID(hwnd,FIND_OPEN),FALSE );
        pData->usSelectFind = 0;

        /* Fill in disk drive list box */
        if ( usResult = DosQueryCurrentDisk(&usDriveNum,&ulMap) )
            {
            ErrMessageBox( hwnd,szSearchTitle,(USHORT)usResult,NULL,0 );
            WinDismissDlg( hwnd,FDLG_CANCEL );
            return 0L;
            }
        pData->pszScratch[1] = ':';
        pData->pszScratch[2] = '\0';
        for ( usCount = 0; usCount < 26; usCount++ )
            if ( ulMap & 1L << usCount )
                {
                pData->pszScratch[0] = (CHAR)( usCount + 'A' );

                usIndex = SHORT1FROMMR(
                            WinSendDlgItemMsg( hwnd,FIND_DRIVES,
                                               LM_INSERTITEM,
                                               MPFROMSHORT(LIT_END),
                                               MPFROMP(pData->pszScratch) )
                                      );

                if ( usCount == usDriveNum-1 )
                    {
                    WinSendDlgItemMsg( hwnd,FIND_DRIVES,
                                       LM_SELECTITEM,
                                       MPFROMSHORT(usIndex),
                                       MPFROMSHORT(TRUE) );
                    }
                }

        return 0L;
    }
/****************************************************************************/


/****************************************************************************
 * PatternEditCtrl() -- Handles messages sent by FIND_PATTERN edit control  *
 *                      to find file dialog box.                            *
 ****************************************************************************/
    static MRESULT PatternEditCtrl( HWND hwnd,ULONG msg,
                                    MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        USHORT  usLen;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        switch ( SHORT2FROMMP(mp1) ) {
            case EN_CHANGE:
                WinEnableWindow( WinWindowFromID(hwnd,FIND_PATTERN),
                                 WinQueryDlgItemTextLength(hwnd,FIND_PATTERN)
                               );
                break;

            case EN_SETFOCUS:
                usLen = WinQueryDlgItemTextLength( hwnd,FIND_PATTERN );
                WinSendDlgItemMsg( hwnd,FIND_PATTERN,EM_SETSEL,
                                   MPFROM2SHORT(0,usLen),0L );
                WinSendDlgItemMsg( hwnd,FIND_SEARCH,BM_SETDEFAULT,
                                   MPFROMSHORT(TRUE),0L );
                WinSendDlgItemMsg( hwnd,FIND_OPEN,BM_SETDEFAULT,
                                   MPFROMSHORT(FALSE),0L );
                WinSendDlgItemMsg( hwnd,FIND_CANCEL,BM_SETDEFAULT,
                                   MPFROMSHORT(FALSE),0L );
                break;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * FindListbox() -- Handle messages sent by FIND_FLIST list box to the      *
 *                  find file dialog box.                                   *
 ****************************************************************************/
    static MRESULT FindListbox( HWND hwnd,ULONG msg,
                                MPARAM mp1,MPARAM mp2 )
    {
        PDATA   pData;
        SHORT   usResult;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        switch ( SHORT2FROMMP(mp1) ) {
            case LN_SELECT:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_FLIST,
                                                           LM_QUERYSELECTION,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE )
                    pData->usSelectFind = usResult;
                WinSendDlgItemMsg( hwnd,FIND_FLIST,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(pData->usSelectFind,pData->usMaxPathLen+2),
                                   MPFROMP(pData->pszScratch) );
                WinEnableWindow( WinWindowFromID(hwnd,FIND_OPEN),
                                 (*pData->pszScratch == ' ') );
                return 0L;

            case LN_ENTER:  /* equivalent to open button clicked */
                WinSendDlgItemMsg( hwnd,FIND_OPEN,BM_CLICK,0L,0L );
                return 0L;

            case LN_SETFOCUS:
                usResult = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_FLIST,
                                                           LM_QUERYTOPINDEX,
                                                           0L,0L)
                                       );
                if ( usResult != LIT_NONE )
                    {
                    if ( pData->usSelectFind < usResult )
                        pData->usSelectFind = usResult;
                    else if (pData->usSelectFind > usResult+10)
                        pData->usSelectFind = usResult+10;

                    WinSendDlgItemMsg( hwnd,FIND_FLIST,
                                       LM_SELECTITEM,
                                       MPFROMSHORT(pData->usSelectFind),
                                       MPFROMSHORT(TRUE) );
                    }

                /* Make OPEN button the current default button */
                WinSendDlgItemMsg( hwnd,FIND_SEARCH,BM_SETDEFAULT,
                                   MPFROMSHORT(FALSE),0L );
                WinSendDlgItemMsg( hwnd,FIND_OPEN,BM_SETDEFAULT,
                                   MPFROMSHORT(TRUE),0L );
                WinSendDlgItemMsg( hwnd,FIND_CANCEL,BM_SETDEFAULT,
                                   MPFROMSHORT(FALSE),0L );
                return 0L;
            }

        return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * OpenButton() -- Executed whenever OPEN button in find file dialog        *
 *                 box is clicked.                                          *
 ****************************************************************************/
    static MRESULT OpenButton( HWND hwnd )
    {
        PDATA   pData;
        SHORT   iItem;
        HWND    hwndButton;

    /* Skip procedure when button does not have the focus.      *
     * This prevents the duplicate error message problem that   *
     * occurs when the enter key is used to select a list box   *
     * item.                                                    */
        hwndButton = WinWindowFromID( hwnd,FIND_OPEN );
        if ( hwndButton != WinQueryFocus(HWND_DESKTOP) ) return 0L;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );
        iItem = pData->usSelectFind;

        if ( WinSendDlgItemMsg( hwnd,FIND_FLIST,
                                LM_QUERYITEMTEXT,
                                MPFROM2SHORT(iItem,pData->usMaxPathLen),
                                MPFROMP(pData->pszFile) )
           )
            {
            /* Get path name */
            do  {
                WinSendDlgItemMsg( hwnd,FIND_FLIST,
                                   LM_QUERYITEMTEXT,
                                   MPFROM2SHORT(--iItem,pData->usMaxPathLen),
                                   MPFROMP(pData->pszScratch) );
                } while ( *pData->pszScratch == ' ' );
            strcat( pData->pszScratch,szSlash );
            strcat( pData->pszScratch,pData->pszFile+3 );

            if ( !OpenFile(hwnd,pData) )
                {
                DosEnterCritSec();
                pData->fControl = TERMINATE;
                DosPostEventSem( pData->hevTrigger );
                DosPostEventSem( pData->hevAdd );
                DosExitCritSec();
                DosWaitEventSem( pData->hevTerminate,SEM_INDEFINITE_WAIT );
                DosFreeMem( pData->pszPattern );
                DosFreeMem( pData->pszScratch2 );
                DosCloseEventSem( pData->hevTrigger );
                DosCloseEventSem( pData->hevTerminate );
                DosCloseEventSem( pData->hevAdd );
                WinDismissDlg( hwnd,FDLG_OK );
                }
            else
                {
                /* Resynch current drive/directory with open dialog box */
                ParseFileName( pData->pszCurDir,pData->pszScratch,pData->pszShowSpec );
                WinSetFocus( HWND_DESKTOP,WinWindowFromID(hwnd,FIND_FLIST) );
                }
            }

        return 0L;
    }
/****************************************************************************/


/****************************************************************************
 * OpenFile() - This function attempts to open the file specified           *
 *              in the scratch data area.                                   *
 *                                                                          *
 *              This function returns a non-zero value if an error occured  *
 *              or the input string was a search specification.             *
 ****************************************************************************/
    static USHORT OpenFile( HWND hDlg,PDATA pData )
    {
        USHORT  usResult;
        ULONG   ulAction;

        usResult = ParseFileName( pData->pszScratch,
                                  pData->pszFile,
                                  pData->pszShowSpec );
        if ( usResult )
            {
            ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
            return 1;
            }

        if ( NULL != strpbrk(pData->pszFile,szWildCardChars) )
            return 1;
        else
            {
            usResult = (USHORT)DosOpen( pData->pszFile,
                                pData->phf,
                                &ulAction,
                                pData->ulFileSize,
                                pData->usAttribute,
                                pData->fsOpenFlags,
                                pData->fsOpenMode,
                                (PEAOP2)NULL );
            if ( usResult )
                {
                ErrMessageBox( hDlg,pData->pszTitle,usResult,NULL,0 );
                return 1;
                }
            else
                {
                *pData->pusAction = (USHORT)ulAction;
                return 0;
                }
            }
    }
/****************************************************************************/


/****************************************************************************
 * SearchButton() -- Executed whenever SEARCH button in find file dialog    *
 *                   box is clicked.                                        *
 ****************************************************************************/
    static MRESULT SearchButton( HWND hwnd )
    {
        PDATA       pData;
        SHORT       iItem;

        pData = (PDATA)WinQueryWindowULong( hwnd,QWL_USER );

    /* Retrieve current search pattern */
        WinQueryDlgItemText( hwnd,FIND_PATTERN,
                             pData->usMaxPathLen,
                             pData->pszPattern );
        if (   NULL != strchr(pData->pszPattern,':')
            || NULL != strchr(pData->pszPattern,'\\') )
            {
            ErrMessageBox( hwnd,szSearchTitle,BAD_SEARCH,
                           appMsgList,appMsgCount );
            WinSetFocus( HWND_DESKTOP,WinWindowFromID(hwnd,FIND_PATTERN) );
            return 0L;
            }

    /* Disable open and search buttons */
        WinEnableWindow( WinWindowFromID(hwnd,FIND_OPEN),FALSE );
        WinEnableWindow( WinWindowFromID(hwnd,FIND_SEARCH),FALSE );

    /* Set dialog box title to "Searching ..." */
        WinSetWindowText( hwnd,szSearching );

    /* Erase current file list */
        WinSendDlgItemMsg( hwnd,FIND_FLIST,LM_DELETEALL,0L,0L );
        pData->usSelectFind = 0;

    /* Generate list of drives to be searched */
        pData->ulDriveList = 0L;
        iItem = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_DRIVES,
                                                LM_QUERYSELECTION,
                                                MPFROMSHORT(LIT_FIRST),0L)
                            );
        while ( iItem != LIT_NONE )
            {
            pData->ulDriveList |= (1L << iItem);
            iItem = SHORT1FROMMR( WinSendDlgItemMsg(hwnd,FIND_DRIVES,
                                                    LM_QUERYSELECTION,
                                                    MPFROMSHORT(iItem),0L)
                                );
            }

    /* Start search thread */
        pData->fControl = CONTINUE;
        DosPostEventSem( pData->hevTrigger );

    /* Set focus to list of files found */
        WinSetFocus( HWND_DESKTOP,WinWindowFromID(hwnd,FIND_FLIST) );
        return 0L;
    }
/****************************************************************************/


/****************************************************************************
 * SearchThread() - This function searches for the specified files in       *
 *                  a background thread.                                    *
 ****************************************************************************/
    static VOID SearchThread( VOID *pArg )
    {
        PDATA   pData = (PDATA)pArg;
        ULONG   usCount;
        PSZ     pszCurrent;
        PSZ     pszFileSpec;
        ULONG   ulPost;
        APIRET  rc;

        while (TRUE)
            {
            DosWaitEventSem( pData->hevTrigger,SEM_INDEFINITE_WAIT );
            DosResetEventSem( pData->hevTrigger,&ulPost );

            if ( pData->fControl == TERMINATE )
                {
                DosEnterCritSec();
                DosPostEventSem( pData->hevTerminate );
                _endthread();
                }

            if ( rc = DosAllocMem((PPVOID)&pszFileSpec,pData->usMaxPathLen,
                                   PAG_COMMIT|PAG_READ|PAG_WRITE) )
                {
                DosEnterCritSec();
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)rc) );
                pData->fControl = CANCEL;
                DosExitCritSec();
                continue;
                }

            pData->usFindCount = 0;
            pszCurrent = strtok( pData->pszPattern,szDelimiters );
            while ( pszCurrent != NULL )
                {
                strcpy( pszFileSpec+3,pszCurrent );
                for ( usCount = 0; usCount < 26; usCount++ )
                    {
                    if ( pData->ulDriveList & (1L << usCount) )
                        {
                        pszFileSpec[0] = (CHAR)( usCount + 'A' );
                        pszFileSpec[1] = ':';
                        pszFileSpec[2] = '\\';
                        if ( !Directory(pszFileSpec,pData) ) switch (pData->fControl) {
                            case CANCEL:
                                WinPostMsg( pData->hDlg,SEARCH_DONE,
                                            MPFROMSHORT(SEARCH_CANCELLED),0L );
                                goto RESTART_LOOP;

                            case TERMINATE:
                                DosFreeMem( pszFileSpec );
                                DosPostEventSem( pData->hevTerminate );
                                _endthread();

                            case ERROR:
                                goto RESTART_LOOP;
                            }
                        }
                    DosEnterCritSec();
                    switch ( pData->fControl ) {
                        case CANCEL:
                            WinPostMsg( pData->hDlg,SEARCH_DONE,
                                        MPFROMSHORT(SEARCH_CANCELLED),0L );
                            goto RESTART_LOOP;

                        case TERMINATE:
                            DosFreeMem( pszFileSpec );
                            DosPostEventSem( pData->hevTerminate );
                            _endthread();

                        case CONTINUE:
                            DosExitCritSec();
                            break;
                        }

                    } /* search next drive */

                pszCurrent = strtok( NULL,szDelimiters );
                } /* look for next search pattern */

            DosEnterCritSec();
            WinPostMsg( pData->hDlg,SEARCH_DONE,MPFROMSHORT(FILES_FOUND),0L );

RESTART_LOOP:
            DosFreeMem( pszFileSpec );
            pData->fControl = CANCEL;
            DosExitCritSec();
            } /* end while */
    }
/****************************************************************************/


/****************************************************************************
 * Directory() - This function is called recursively, successively          *
 *               searching each subdirectory for files that match           *
 *               the search criteria until the entire drive has been        *
 *               traversed.                                                 *
 *                                                                          *
 *               This function returns a value of FALSE if an error         *
 *               occurs, otherwise it returns a value of TRUE.              *
 ****************************************************************************/
    static BOOL Directory( PSZ pszFileSpec,PDATA pData )
    {
        #define FINDCOUNT       10
        #define FILEFINDSIZE    (FINDCOUNT*sizeof(FILEFINDBUF3))
        #define ABORT_CHK()     if ( pData->fControl != CONTINUE ) goto ABORT_EXIT

        PSZ            pszSpec;
        PSZ            pszSub;
        PFILEFINDBUF3  pFindbuf,pFindbuf2;
        PFILEFINDBUF3  pCurrent;
        HDIR           hdir = HDIR_CREATE;
        ULONG          usSearchCount;
        ULONG          n;
        ULONG          ulPost;
        APIRET         usResult,rc;

        /*********************************************************
         * Allocate memory for file find buffers.               *
         * DosAllocMem keeps them off the stack so deeply        *
         * recursive calls on large trees don't overflow it.    *
         *********************************************************/
        if ( rc = DosAllocMem((PPVOID)&pFindbuf,FILEFINDSIZE,
                               PAG_COMMIT|PAG_READ|PAG_WRITE) )
            {
            DosEnterCritSec();
            if ( pData->fControl == CONTINUE )
                {
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)rc) );
                pData->fControl = ERROR;
                }
            return FALSE;
            }
        if ( rc = DosAllocMem((PPVOID)&pFindbuf2,FILEFINDSIZE,
                               PAG_COMMIT|PAG_READ|PAG_WRITE) )
            {
            DosEnterCritSec();
            if ( pData->fControl == CONTINUE )
                {
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)rc) );
                pData->fControl = ERROR;
                }
            DosFreeMem( pFindbuf );
            return FALSE;
            }

        /* Split input file spec into directory name and file name */
        strcpy( pData->pszScratch2,pszFileSpec );
        pszSpec = strrchr( pszFileSpec,'\\' );
        *pszSpec++ = '\0';

        /* Find first file that matches criteria */
        usSearchCount = FINDCOUNT;
        usResult = DosFindFirst( pData->pszScratch2,&hdir,pData->usShowAttr,
                                 pFindbuf2,FILEFINDSIZE,
                                 &usSearchCount,FIL_STANDARD );

        /* Add directory name to list box if a file was found */
        if ( !usResult )
            {
            ABORT_CHK();
            DosResetEventSem( pData->hevAdd,&ulPost );
            while ( !WinPostMsg(pData->hDlg,SEARCH_ADDDIR,MPFROMP(pszFileSpec),0L) )
                ABORT_CHK();
            DosWaitEventSem( pData->hevAdd,SEM_INDEFINITE_WAIT );
            }

        /* Add names of found files to list box */
        while ( !usResult )
            {
            ABORT_CHK();
            memcpy( pFindbuf,pFindbuf2,FILEFINDSIZE );
            pData->usFindCount += usSearchCount;
            DosResetEventSem( pData->hevAdd,&ulPost );
            while ( !WinPostMsg(pData->hDlg,SEARCH_ADDFILES,
                                MPFROMP(pFindbuf),
                                (MPARAM)usSearchCount) ) ABORT_CHK();

            /* Get next set of matching files while list is being updated */
            usSearchCount = FINDCOUNT;
            usResult = DosFindNext( hdir,pFindbuf2,FILEFINDSIZE,&usSearchCount );

            /* Wait until listbox is finished being updated */
            DosWaitEventSem( pData->hevAdd,SEM_INDEFINITE_WAIT );
            }
        DosFreeMem( pFindbuf2 );
        pFindbuf2 = NULL;
        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if (   (usResult && usResult != ERROR_NO_MORE_FILES)
            || pData->fControl != CONTINUE )
            {
            DosEnterCritSec();
            if ( pData->fControl == CONTINUE )
                {
                pData->fControl = ERROR;
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)usResult) );
                }
            DosFreeMem( pFindbuf );
            return FALSE;
            }

        /* Allocate memory for subdirectory search spec. */
        if ( rc = DosAllocMem((PPVOID)&pszSub,pData->usMaxPathLen,
                               PAG_COMMIT|PAG_READ|PAG_WRITE) )
            {
            DosEnterCritSec();
            if ( pData->fControl == CONTINUE )
                {
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)rc) );
                pData->fControl = ERROR;
                }
            DosFreeMem( pFindbuf );
            return FALSE;
            }

        /* Search subdirectories for matching files */
        hdir            = HDIR_CREATE;
        usSearchCount   = FINDCOUNT;
        strcpy( pData->pszScratch2,pszFileSpec );
        strcat( pData->pszScratch2,szSlash );
        strcat( pData->pszScratch2,szStarDotStar );
        usResult = DosFindFirst( pData->pszScratch2,&hdir,FILE_DIRECTORY,
                                 pFindbuf,FILEFINDSIZE,
                                 &usSearchCount,FIL_STANDARD );
        while ( !usResult )
            {
            for ( pCurrent = pFindbuf,n = 0; n < usSearchCount; n++ )
                {
                if ( pData->fControl != CONTINUE )
                    {
                    DosEnterCritSec();
                    DosFindClose( hdir );
                    DosFreeMem( pFindbuf );
                    DosFreeMem( pszSub );
                    return FALSE;
                    }

                if (   (pCurrent->attrFile & FILE_DIRECTORY)
                    && (pCurrent->achName[0] != '.') )
                    {
                    strcpy( pszSub,pszFileSpec );
                    strcat( pszSub,szSlash );
                    strcat( pszSub,pCurrent->achName );
                    strcat( pszSub,szSlash );
                    strcat( pszSub,pszSpec );
                    if ( !Directory(pszSub,pData) )
                        {
                        DosFindClose( hdir );
                        DosFreeMem( pFindbuf );
                        DosFreeMem( pszSub );
                        return FALSE;
                        }
                    }
                pCurrent = (PFILEFINDBUF3)((PBYTE)pCurrent + pCurrent->oNextEntryOffset);
                }
            usSearchCount = FINDCOUNT;
            usResult = DosFindNext( hdir,pFindbuf,FILEFINDSIZE,&usSearchCount );
            }
        if ( usResult != ERROR_NO_MORE_SEARCH_HANDLES ) DosFindClose(hdir);
        if (   (usResult && usResult != ERROR_NO_MORE_FILES)
            || pData->fControl != CONTINUE )
            {
            DosEnterCritSec();
            if ( pData->fControl == CONTINUE )
                {
                pData->fControl = ERROR;
                WinPostMsg( pData->hDlg,SEARCH_DONE,0L,MPFROMSHORT((USHORT)usResult) );
                }
            DosFreeMem( pFindbuf );
            DosFreeMem( pszSub );
            return FALSE;
            }

        /* Done. Return to caller */
        DosFreeMem( pFindbuf );
        DosFreeMem( pszSub );
        return TRUE;

        /* Executed by ABORT_CHK() macro if search is aborted */
ABORT_EXIT:
        DosEnterCritSec();
        DosFindClose( hdir );
        DosFreeMem( pFindbuf );
        if ( pFindbuf2 ) DosFreeMem( pFindbuf2 );
        return FALSE;

        #undef FINDCOUNT
        #undef FILEFINDSIZE
        #undef ABORT_CHK
    }
/****************************************************************************/
