/****************************************************************************
 * USHORT ErrMessageBox( HWND hwndOwner,PSZ pszCaption,USHORT usErrorCode,  *                       *
 *                       PERRORMSG perrormsg,USHORT cErrMsgCnt, ... )       *
 * Purpose          The ErrMessageBox function displays in a message        *
 *                  box the error message associated with the given         *
 *                  error code.                                             *
 *                                                                          *
 * Parameters       hwndOwner identifies the owner window of the            *
 *                  message-box window. The owner window is activated       *
 *                  when ErrMessageBox returns.                             *
 *                                                                          *
 *                  pszCaption points to the title of the message-box       *
 *                  window. If this parameter is NULL, "Error" (the         *
 *                  default title) is displayed. The maximum length         *
 *                  of the text is device-dependent. If the text is         *
 *                  too long, it will be clipped.                           *
 *                                                                          *
 *                  usErrorCode specifies the error value returned          *
 *                  by an MS OS/2 function.                                 *
 *                                                                          *
 *                  perrormsg points to the user defined error message      *
 *                  list. If this parameter is NULL then the default        *
 *                  system error message list will be used.                 *
 *                                                                          *
 *                  cErrMsgCnt contains the number of items in the user     *
 *                  defined error message list. This parameter is           *
 *                  ignored if perrormsg is NULL.                           *
 *                                                                          *
 *                  The optional arguments are passed to the vsprintf       *
 *                  function in order to allow error messages to have       *
 *                  variable contents. NOTE THAT THIS FUNCTION REQUIRES     *
 *                  THAT ALL POINTER ARGUMENTS (SUCH AS POINTERS TO         *
 *                  STRINGS) MUST BE FAR POINTERS.                          *
 *                                                                          *
 * Return Value     The return value indicates the user's response to       *
 *                  the message. It can be one of the following values:     *
 *                                                                          *
 *                  MBID_ABORT   Abort button was selected.                 *
 *                  MBID_CANCEL  Cancel button was selected.                *
 *                  MBID_ENTER   Enter button was selected.                 *
 *                  MBID_IGNORE  Ignore button was selected.                *
 *                  MBID_NO      No button was selected.                    *
 *                  MBID_OK      OK button was selected.                    *
 *                  MBID_RETRY   Retry button was selected.                 *
 *                  MBID_YES     Yes button was selected.                   *
 *                  MDID_ERROR   The WinMessageBox function failed--        *
 *                                  an error occurred.                      *
 *                                                                          *
 *                  If a message box has a Cancel button, MBID_CANCEL       *
 *                  is return if the ESCAPE key is pressed or if the        *
 *                  Cancel button is selected. If the message box has       *
 *                  no Cancel button, pressing the ESCAPE key has no        *
 *                  effect.                                                 *
 *                                                                          *
 * Notes            If a message-box window is created as part of the       *
 *                  processing of a dialog window, the dialog window        *
 *                  should be made the owner of the message-box             *
 *                  window.                                                 *
 *                                                                          *
 *                  If a system modal message-box window is created to      *
 *                  tell the user that the system is running out of         *
 *                  memory, the strings passed into this function           *
 *                  should not be taken from a resource file because        *
 *                  an attempt to load the resource file may fail due       *
 *                  to lack of memory. Such a message-box window can        *
 *                  safely use the hand icon (MB_ICONHAND), however,        *
 *                  because this icon is always memory-resident.            *
 *                                                                          *
 *                  If the message box contains a help pushbutton, the      *
 *                  message box window identifier will be set to the        *
 *                  error code value.                                       *
 *                                                                          *
 *                  The total length of the error message string after      *
 *                  it has been formatted and stored by vsprintf must       *
 *                  not exceed 512 characters (including the terminating    *
 *                  NULL character).                                        *
 *                                                                          *
 *                                                                          *
 * Modifications -                                                          *
 *      09-Oct-1989 : Initial version.                                      *
 *      27-Nov-1989 : Added variable arguments.                             *
 ****************************************************************************/

    #define INCL_WINDIALOGS
    #define INCL_DOSSEMAPHORES
    #include <os2.h>
    #include <stdio.h>
    #include <stdarg.h>

    #include "errmsg.h"
    #include "errconst.h"

/****************************************************************************/
    USHORT cdecl far _loadds _export ErrMessageBox( HWND hwndOwner,
                                                    PSZ pszCaption,
                                                    USHORT usErrorCode,
                                                    PERRORMSG perrormsg,
                                                    USHORT cErrMsgCnt,
                                                    ... )
    {
        static ULONG  hsem = 0L;  // semaphore to control access to function
        static CHAR   szBuf[512]; // buffer for formatted error message

        PERRORMSG   pList;      // ptr to list to be searched
        USHORT      usCount;    // number of items in list
        USHORT      i;          // counter
        PSZ         pszMsg;     // ptr to error message
        USHORT      flStyle;    // message box style
        va_list     arglist;    // pointer to variable length argument list
        USHORT      usResult;   // result code

    /* Make sure this is the only thread accessing this function */
        DosSemRequest( &hsem,SEM_INDEFINITE_WAIT );

    /* Select error list that is to be searched */
        if ( perrormsg == NULL )
            { /* Use system error list */
            if ( usErrorCode < 0x1000 || usErrorCode >= 0x8000 )
                {
                pList   = SysErrMsg;
                usCount = SysErrCount;
                }
            else
                {
                pList   = PMErrMsg;
                usCount = PMErrCount;
                }
            }
        else
            { /* Use user supplied error list */
            pList   = perrormsg;
            usCount = cErrMsgCnt;
            }

    /* Search selected list for error code */
        for ( i = 0; i < usCount; i++ )
            if ( pList[i].usCode == usErrorCode )
                {
                pszMsg  = pList[i].pszMsg;
                flStyle = pList[i].flStyle;
                break;
                }

    /* If error code not in list then display default error message */
        if ( i == usCount )
            {
            sprintf( szBuf,
                     (perrormsg == NULL) ? szUnkSysError : szUnkAppError,
                     usErrorCode,usErrorCode );
            flStyle = MB_OK | MB_ICONEXCLAMATION;
            }

    /* Error code found in list so format error message */
        else
            {
            va_start( arglist,cErrMsgCnt );
            vsprintf( szBuf,pszMsg,arglist );
            va_end( arglist );
            }

    /* Display message box */
        usResult = WinMessageBox( HWND_DESKTOP,hwndOwner,szBuf,
                                  pszCaption,usErrorCode,flStyle );

    /* Clear semaphore and return to caller */
        DosSemClear( &hsem );
        return usResult;
    }
/****************************************************************************/
