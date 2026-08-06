/****************************************************************************
 * HELPERR.C -- Help Manager error messages.                                *
 *                                                                          *
 * Modifications -                                                          *
 *      12-Nov-1990 : Initial version.                                      *
 ****************************************************************************/

#define INCL_WINDIALOGS
#include <os2.h>
#include "errmsg.h"

#define MB_STD  MB_OK|MB_ICONEXCLAMATION

ERRORMSG HelpMgrErrors[] = {
    0x1001, MB_STD, "HMERR_NO_FRAME_WND_IN_CHAIN - There is no frame window in the window chain from which to find or set the associated help instance.",
    0x1002, MB_STD, "HMERR_INVALID_ASSOC_APP_WND - The application window handle specified on the WinAssociateHelpInstance() call is not a valid window handle.",
    0x1003, MB_STD, "HMERR_INVALID_ASSOC_HELP_INST - The help instance handle specified on the WinAssociateHelpInstance() call is not a valid window handle.",
    0x1004, MB_STD, "HMERR_INVALID_DESTROY_HELP_INST - The window handle specified as the help instance to destroy is not of the help instance class.",
    0x1005, MB_STD, "HMERR_NO_HELP_INST_IN_CHAIN - The parent or owner chain of the application window specified does not have a help instance associated with it.",
    0x1006, MB_STD, "HMERR_INVALID_HEAP_INSTANCE_HDL - The handle specified to be a help instance does not have the class name of a IPF help instance.",
    0x1007, MB_STD, "HMERR_INVALID_QUERY_APP_WND - The application window specified on a WinQueryHelpInstance() call is not a valid window handle.",
    0x1008, MB_STD, "HMERR_HELPINST_CALLED_INVALID - The handle of the help instance specified on an API call to the IPF does not have the class name of an IPF help instance.",
    0x1009, MB_STD, "HMERR_HELPTABLE_UNDEFINE",
    0x100A, MB_STD, "HMERR_HELP_INSTANCE_UNDEFINE",
    0x100B, MB_STD, "HMERR_HELPITEM_NOT_FOUND",
    0x100C, MB_STD, "HMERR_INVALID_HELPSUBITEM_SIZE",
    0x100D, MB_STD, "HMERR_HELPSUBITEM_NOT_FOUND",
    0x2001, MB_STD, "HMERR_INDEX_NOT_FOUND - No index in help file.",
    0x2002, MB_STD, "HMERR_CONTENT_NOT_FOUND - Library file does not have a table of contents.",
    0x2003, MB_STD, "HMERR_OPEN_LIB_FILE - Cannot open help file.",
    0x2004, MB_STD, "HMERR_READ_LIB_FILE - Cannot read help file.",
    0x2005, MB_STD, "HMERR_CLOSE_LIB_FILE - Cannot close help file.",
    0x2006, MB_STD, "HMERR_INVALID_LIB_FILE - Improper help file provided.",
    0x2007, MB_STD, "HMERR_NO_MEMORY - Unable to allocate the requested amount of memory.",
    0x2008, MB_STD, "HMERR_ALLOCATE_SEGMENT - Unable to allocate a segment of memory for memory allocation requested from the IPF.",
    0x2009, MB_STD, "HMERR_FREE_MEMORY - Unable to free allocated memory.",
    0x2010, MB_STD, "HMERR_PANEL_NOT_FOUND - Unable to find a requested help panel.",
    0x2011, MB_STD, "HMERR_DATABASE_NOT_OPEN"
    };

USHORT HelpMgrCount = sizeof(HelpMgrErrors) / sizeof(ERRORMSG);
