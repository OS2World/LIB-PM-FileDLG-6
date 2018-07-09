/****************************************************************************
 * OPENDATA.H                                                               *
 *                                                                          *
 *  This header file contains the definition of the data structure used     *
 *  to hold static data for the open file and find file dialog boxes.       *
 *                                                                          *
 *  Modifications --                                                        *
 *      21-Sep-1989 : Initial version.                                      *
 *      11-Oct-1990 : Added pszCurDir, iFirstChar and bitmap handles.       *
 *      15-Oct-1990 : Added vars used to control file finder thread.        *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    typedef enum { /* Constants used to control search thread */
        CANCEL,         /* Cancel search */
        CONTINUE,       /* Continue search */
        TERMINATE,      /* Terminate search thread */
        ERROR           /* Error occurred. Cancel search. */
        } THDCONTROL;

    typedef struct {
        PSZ     pszTitle;           // dialog box title
        PSZ     pszIns;             // dialog box instructions
        void (EXPENTRY *pfnHelpProc)(HWND hDlg); // ptr to help procedure
        PSZ     pszFile;            // ptr to name of opened file
        PHFILE  phf;                // ptr to file handle
        ULONG   ulFileSize;         // initial file size
        PUSHORT pusAction;          // action taken on open
        USHORT  usAttribute;        // file attribute
        USHORT  fsOpenFlags;        // open flags
        USHORT  fsOpenMode;         // open mode
        ULONG   ulReserved;         // reserved
        PSZ     pszShowSpec;        // file spec of files to be listed
        USHORT  usShowAttr;         // attributes of files to be listed
        USHORT  usMaxPathLen;       // maximum path name length
        PSZ     pszScratch;         // ptr to scratch data area
        HMODULE hmod;               // handle to resource module
        USHORT  usSelectDrive;      // current selected item in drive list box
        USHORT  usSelectDir;        // current selected item in dir list box
        USHORT  usSelectFile;       // current selected item in file list box
        USHORT  usSelectFind;       // current selected item in find list box
        USHORT  usFocus;            // control to which focus is to be returned
                                    //  after an unsuccesful open attempt.
        PSZ     pszCurDir;          // pointer to current directory string.
        USHORT  iFirstChar;         // index of first character of current
                                    //  directory string that is to be displayed.
        HBITMAP hbmLeft;            // Left arrow bitmap handle.
        HBITMAP hbmLeftPressed;     // Pressed left arrow bitmap handle.
        HBITMAP hbmLeftDisabled;    // Disabled left arrow bitmap handle.
        HBITMAP hbmRight;           // Right arrow bitmap handle.
        HBITMAP hbmRightPressed;    // Pressed right arrow bitmap handle.
        HBITMAP hbmRightDisabled;   // Disabled right arrow bitmap handle.

        /* Vars used by file find thread */
        SEL     selStack;           // Selector for search thread stack.
        ULONG   semTrigger;         // Semaphore to start search thread.
        ULONG   semTerminate;       // Signals when search thread terminated.
        ULONG   semAdd;             // Signals when item added to list box.
        HWND    hDlg;               // Handle to dialog box window.
        ULONG   ulDriveList;        // List of drives to be searched.
        PSZ     pszPattern;         // Ptr to file search pattern.
        PSZ     pszScratch2;        // Ptr to scratch data area.
        USHORT  usFindCount;        // Number of files found.
        volatile THDCONTROL fControl; // Used to control search thread.
        TID     tid;                // Thread id of search thread.
        } DATA;

    typedef DATA * PDATA;
