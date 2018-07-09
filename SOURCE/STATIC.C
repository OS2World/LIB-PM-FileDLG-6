/****************************************************************************
 * STATIC.C --  Static data used by file open/save dialog box               *
 *              procedures. All data defined in this module is              *
 *              stored in the data segment STATICDATA.                      *
 *                                                                          *
 *                                                                          *
 * Modifications --                                                         *
 *      08-Sep-1989 : Initial version.                                      *
 *      11-Oct-1989 : Changed to DLL version of ErrMessageBox function.     *
 *      19-Nov-1989 : Added szUnnamed.                                      *
 *      11-Oct-1990 : Changed szStarDotStar from "*.*" to "*".              *
 *                    Removed szFitReplace.                                 *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_WINDIALOGS
    #include <os2.h>
    #include <errmsg.h>
    #include "static.h"

/****************************************************************************
 *  Constant data common to all functions                                   *
 ****************************************************************************/
    CHAR szSlash[] = "\\";
    CHAR szStarDotStar[] = "*";
    CHAR szDLLName[] = "FILEDLG";
/****************************************************************************/


/****************************************************************************
 *  Constant data used in the file save dialog box functions                *
 ****************************************************************************/
    CHAR szDefSaveTitle[] = "Save File";
    CHAR szDefSaveIns[] = "Type file name.";
    CHAR szOverwriteMsg[] = "File already exists. Overwrite?";
    CHAR szUnnamed[] = "UNNAMED";
/****************************************************************************/


/****************************************************************************
 *  Constant data used in the file open dialog box functions.               *
 ****************************************************************************/
    CHAR szDefOpenTitle[] = "Open File";
    CHAR szDefOpenIns[] = "Select file or type filename.";
    CHAR szSearchTitle[] = "Search for a File";
    CHAR szWildCardChars[] = "*?";
    CHAR szListBoxFull[] = "List box is full.";
    CHAR szFilesFnd[] = "Search complete. %u files found.";
    CHAR szSearching[] = "Searching ...";
    CHAR szSearchCancelled[] = "Search cancelled.";
    CHAR szSearchThreadError[] = "Unable to start search thread.";
    CHAR szIllegalSearch[] = "Do not include a drive or path in the search pattern.";
    CHAR szDelimiters[] = ",";
/****************************************************************************/


/****************************************************************************
 *  Application defined error/information message data                      *
 ****************************************************************************/
    ERRORMSG appMsgList[] = {
        OVERWRITE_MSG, MB_YESNOCANCEL|MB_ICONQUESTION, szOverwriteMsg,
        LISTBOX_FULL, MB_OK|MB_ICONEXCLAMATION, szListBoxFull,
        FILES_FOUND, MB_OK|MB_ICONEXCLAMATION, szFilesFnd,
        SEARCH_CANCELLED, MB_OK|MB_ICONEXCLAMATION, szSearchCancelled,
        SEARCH_THREAD_ERROR, MB_OK|MB_ICONEXCLAMATION, szSearchThreadError,
        BAD_SEARCH, MB_OK|MB_ICONEXCLAMATION, szIllegalSearch
        };

    USHORT appMsgCount = sizeof(appMsgList) / sizeof(ERRORMSG);
/****************************************************************************/
