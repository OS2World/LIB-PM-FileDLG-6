/****************************************************************************
 * ERRMSG.H -- Header file containing the data structure and function       *
 *             declarations of the standard error handler function.         *
 *                                                                          *
 *                                                                          *
 *  Modifications --                                                        *
 *      09-Oct-1989 : Initial version.                                      *
 *      12-Nov-1990 : Added help manager error list.                        *
 ****************************************************************************/

#if !defined(_ERRMSG_INCLUDED)
#define _ERRMSG_INCLUDED

#if !defined(_QC)
    #pragma comment( lib,"errmsg.lib" )
#endif

/****************************************************************************
 * Data type definitions                                                    *
 ****************************************************************************/
    typedef struct {
        USHORT  usCode;         // error code
        USHORT  flStyle;        // message box style (see explanation below)
        PSZ     pszMsg;         // ptr to error message text
        } ERRORMSG;

    typedef ERRORMSG FAR * PERRORMSG;

/****************************************************************************\
flStyle     Specifies the type of message-box window created. This parameter
            consists of a button flag, an icon flag, a default button flag,
            and any number of special flags. The following four lists
            describe the available flags which can be combined using the OR
            operator together for this parameter:

            Buttons              Meaning
            ----------------------------------------------------------------
            MB_ABORTRETRYIGNORE  Message box contains Abort, Retry, and
                                 Ignore push buttons.

            MB_ENTER             Message box contains an Enter push button.

            MB_ENTERCANCEL       Message box contains Enter and Cancel push
                                 buttons.

            MB_OK                Message box contains an OK push button.

            MB_OKCANCEL          Message box contains OK and Cancel push
                                 buttons.

            MB_RETRYCANCEL       Message box contains Retry and Cancel push
                                 buttons.

            MB_YESNO             Message box contains Yes and No push
                                 buttons.

            MB_YESNOCANCEL       Message box contains Yes, No, and Cancel
                                 push buttons.

            Icon                Meaning
            ----------------------------------------------------------------
            MB_ICONASTERISK     Message box contains asterisk icon.

            MB_ICONEXCLAMATION  Message box contains exclamation-point
                                icon.

            MB_ICONHAND         Message box contains hand icon.

            MB_ICONQUESTION     Message box contains question-mark icon.

            MB_NOICON           Message box does not contain an icon.

            Default button  Meaning
            ----------------------------------------------------------------
            MB_DEFBUTTON1   First button is the default (first button is
                            always the default unless MB_DEFBUTTON2 or
                            MB_DEFBUTTON3 is specified).

            MB_DEFBUTTON2   Second button is the default.

            MB_DEFBUTTON3   Third button is the default.

            Special flags   Meaning
            ----------------------------------------------------------------
            MB_APPLMODAL    Message box is application modal.

            MB_SYSTEMMODAL  Message box is system modal.

            MB_HELP         Message box contains Help push button.

            MB_MOVEABLE     Message box is movable.
\****************************************************************************/


/****************************************************************************
 * Predefined error lists                                                   *
 ****************************************************************************/
    extern ERRORMSG SysErrMsg[];        /* system error list */
    extern USHORT   SysErrCount;        /* (errors < 0x1000 && > 0x8000) */

    extern ERRORMSG PMErrMsg[];         /* PM error list */
    extern USHORT   PMErrCount;         /* (errors between 0x1000 && 0x8000 */

    extern ERRORMSG HelpMgrErrors[];    /* help manager error list */
    extern USHORT   HelpMgrCount;
/****************************************************************************/


/****************************************************************************/
extern USHORT cdecl far _loadds _export ErrMessageBox( HWND hwndOwner,
                                                       PSZ pszCaption,
                                                       USHORT usErrorCode,
                                                       PERRORMSG perrormsg,
                                                       USHORT cErrMsgCnt,
                                                       ... );
/****************************************************************************\
The ErrMessageBox function creates, displays, and operates a message-box
window that displays the error message associated with a given error
code.

Parameters  Description
----------------------------------------------------------------------------
hwndOwner   Identifies the owner window of the message-box window. The owner
            window is activated when ErrMessageBox returns.

pszCaption  Points to the title of the message-box window. If this parameter
            is NULL, "Error" (the default title) is displayed. The maximum
            length of the text is device-dependent. If the text is too long,
            it will be clipped.

usErrorCode Contains the error code.

perrormsg   Points to the user defined error message list. If this
            parameter is NULL then the error message list appropriate
            to the error code will be used (SysErrMsg or PMErrMsg).

            The following error message lists are predefined:

                SysErrMsg    : system error messages for error codes
                               less that 0x1000 or above or equal to
                               0x8000.

                PMErrMsg     : PM error messages for error codes between
                               0x1000 and 0x8000.

                HlpMgrErrors : Help manager error messages.

cErrMsgCnt  Contains the number of items in the user defined error message
            list. This parameter is ignored if perrormsg is NULL.

arguments   Optional list of arguments that are passed to the vsprintf
            function in order to allow error messages to have variable
            contents. NOTE THAT THIS FUNCTION REQUIRES THAT ALL POINTER
            ARGUMENTS (SUCH AS POINTERS TO STRINGS) MUST BE FAR POINTERS.


Return Value

The return value indicates the user's response to the message. It can be one
of the following values:

    Value        Meaning
    ------------------------------------------------------------------------
    MBID_ABORT   Abort button was selected.

    MBID_CANCEL  Cancel button was selected.

    MBID_ENTER   Enter button was selected.

    MBID_IGNORE  Ignore button was selected.

    MBID_NO      No button was selected.

    MBID_OK      OK button was selected.

    MBID_RETRY   Retry button was selected.

    MBID_YES     Yes button was selected.

    MDID_ERROR   The WinMessageBox function failed--an error occurred.

If a message box has a Cancel button, MBID_CANCEL is returned if the ESCAPE
key is pressed or if the Cancel button is selected. If the message box has
no Cancel button, pressing the ESCAPE key has no effect.

Comments

If a message-box window is created as part of the processing of a dialog
window, the dialog window should be made the owner of the message-box
window.

If a system modal message-box window is created to tell the user that the
system is running out of memory, the strings passed into this function
should not be taken from a resource file because an attempt to load the
resource file may fail due to lack of memory. Such a message-box window can
safely use the hand icon (MB_ICONHAND), however, because this icon is always
memory-resident.

If the message box contains a help pushbutton, the message box window
identifier will be set to the error code value.

The total length of the error message string after it has been formatted
and stored by vsprintf must not exceed 512 characters (including the
terminating NULL character).
\****************************************************************************/
#endif /* _ERRMSG_INCLUDED */
