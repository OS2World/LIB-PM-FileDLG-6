/****************************************************************************
 * STATIC.H --  Header file containing declarations of static data used     *
 *              by the file open/save dialog box procedures.                *
 *                                                                          *
 * Modifications --                                                         *
 *      08-Sep-1989 : Initial version.                                      *
 *      11-Oct-1989 : Changed to DLL version of ErrMessageBox function.     *
 *      19-Nov-1989 : Added szUnnamed.                                      *
 *      11-Oct-1990 : Eliminated szFitReplace                               *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/


/****************************************************************************
 *  Constant data common to all functions                                   *
 ****************************************************************************/
    extern CHAR     szSlash[];
    extern CHAR     szStarDotStar[];
    extern CHAR     szDLLName[];
/****************************************************************************/


/****************************************************************************
 *  Constant data used in the file save dialog box functions                *
 ****************************************************************************/
    extern CHAR     szDefSaveTitle[];
    extern CHAR     szDefSaveIns[];
    extern CHAR     szOverwriteMsg[];
    extern CHAR     szUnnamed[];
/****************************************************************************/


/****************************************************************************
 *  Constant data used in the file open dialog box functions.               *
 ****************************************************************************/
    extern CHAR     szDefOpenTitle[];
    extern CHAR     szDefOpenIns[];
    extern CHAR     szSearchTitle[];
    extern CHAR     szWildCardChars[];
    extern CHAR     szListBoxFull[];
    extern CHAR     szFilesFnd[];
    extern CHAR     szSearching[];
    extern CHAR     szSearchCancelled[];
    extern CHAR     szSearchThreadError[];
    extern CHAR     szIllegalSearch[];
    extern CHAR     szDelimiters[];
/****************************************************************************/


/****************************************************************************
 *  Application defined error/information message data                      *
 ****************************************************************************/
#ifdef INCL_WINDIALOGS
    #define OVERWRITE_MSG       0
    #define LISTBOX_FULL        1
    #define FILES_FOUND         2
    #define SEARCH_CANCELLED    3
    #define SEARCH_THREAD_ERROR 4
    #define BAD_SEARCH          5

    extern ERRORMSG appMsgList[];
    extern USHORT   appMsgCount;
#endif
/****************************************************************************/
