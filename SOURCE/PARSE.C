/****************************************************************************
 * USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch );         *
 * Purpose                  This function generates a fully                 *
 *                          qualified file name or search spec.             *
 *                          from the input value, and changes               *
 *                          the current drive and directory to              *
 *                          the drive/directory listed in the               *
 *                          input file name.                                *
 *                                                                          *
 * Parameters               pszSource points to the input file name         *
 *                          or search spec.                                 *
 *                                                                          *
 *                          pszDest points to the location where            *
 *                          the fully qualified file name or                *
 *                          search spec is to be stored. The memory         *
 *                          buffer pointed to by pszDest must be at         *
 *                          least the system maximum path length bytes      *
 *                          long.                                           *
 *                                                                          *
 *                          pszSearch points to the current file search     *
 *                          specification. This is used when the input      *
 *                          string is just a drive or a drive:\directory    *
 *                          without a file name.                            *
 *                                                                          *
 * Return Value             The return value is zero if the function        *
 *                          is successful, otherwise it is an               *
 *                          error value. The contents of pszDest is         *
 *                          undefined if an error has occurred.             *
 *                                                                          *
 * Notes                    This function no longer modifies the contents   *
 *                          of the string pointed to by pszSource.          *
 *                                                                          *
 *                          This function now detects illegal file          *
 *                          names and search specifications.                *
 *                                                                          *
 *                                                                          *
 * Modifications -                                                          *
 *      17-Aug-1989 : Initial version.                                      *
 *      11-Oct-1990 : Added long file name support.                         *
 *                                                                          *
 * (c)Copyright 1990 Rick Yoder                                             *
 ****************************************************************************/

    #define INCL_DOSFILEMGR
    #define INCL_DOSMISC
    #define INCL_DOSERRORS
    #include <os2.h>
    #include <string.h>
    #include <ctype.h>
    #include "tools.h"
    #include "static.h"

/****************************************************************************/
    USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch )
    {
        USHORT  usMaxPathLen;
        USHORT  cbBuf;
        USHORT  usResult;
        PCHAR   pcLastSlash;

    /* Get maximum path length */
        if ( usResult = DosQSysInfo(0,(PBYTE)&usMaxPathLen,sizeof(USHORT)) )
            return usResult;

    /* Check whether input string just contains a drive letter.
     * If true, then check whether drive is valid and create
     * a string containing the new drive, current directory, and
     * and search specification.
     */
        if ( strlen(pszSource)==2 && pszSource[1] == ':' && isalpha(pszSource[0]) )
            {
            strcpy( pszDest,pszSource );
            pszDest[0] = (CHAR)toupper(pszDest[0]);
            if ( usResult = DosSelectDisk(pszDest[0]-'@') ) return usResult;

            /* Append current directory and search spec. */
            cbBuf = usMaxPathLen - 3;
            if ( !(usResult = DosQCurDir(0,pszDest,&cbBuf)) )
                {
                if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                    strcat( pszDest,szSlash );
                strcat( pszDest,pszSearch );
                }
            return usResult;
            }

    /* Generate fully qualified file/path name */
    usResult = DosQPathInfo(pszSource,FIL_QUERYFULLNAME,pszDest,usMaxPathLen,0L);
    if ( usResult ) return usResult;

    /* Change to specified drive */
    if ( usResult = DosSelectDisk(toupper(pszDest[0])-'@') ) return usResult;

    /* Search for last backslash. */
    pcLastSlash = strrchr(pszDest,'\\');

    /* If the only backslash is at beginning, pszDest contains either */
    /* d:\filename or d:\dirname or d:\                               */
        if ( &pszDest[2] == pcLastSlash )
            {
            switch ( usResult = DosChDir(pszDest,0L) ) {
                case NO_ERROR:  // pszDest contains d:\dirname or d:\
                    if ( *(pszDest + strlen(pszDest) - 1) != '\\' )
                        strcat( pszDest,szSlash );
                    strcat( pszDest,pszSearch );
                    return usResult;

                case ERROR_FILE_NOT_FOUND:
                case ERROR_PATH_NOT_FOUND:
                case ERROR_ACCESS_DENIED:   // pszSource contains \filename
                    if ( usResult = DosChDir(szSlash,0L) ) return usResult;
                    return usResult;

                default:
                    return usResult;
                }
            }

    /* Input has d:\dir\filename or d:\dir\dir */
        switch ( usResult = DosChDir(pszDest,0L) ) {
            case NO_ERROR:      // pszSource contains d:\dir\dir
                strcat( pszDest,szSlash );
                strcat( pszDest,pszSearch );
                return 0;

            case ERROR_FILE_NOT_FOUND:
            case ERROR_PATH_NOT_FOUND:
            case ERROR_ACCESS_DENIED:   // d:\dir\filename
                *pcLastSlash = '\0';
                if ( usResult = DosChDir(pszDest,0L) ) return usResult;
                *pcLastSlash = '\\';
                return 0;

            default:
                return usResult;
            }
    }
/****************************************************************************/
