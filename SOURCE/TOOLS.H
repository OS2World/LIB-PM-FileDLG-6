/****************************************************************************
 * TOOLS.H                                                                  *
 *                                                                          *
 *  This header file contains the declarations of the common tool           *
 *  functions used internally by the open file and save file dialog         *
 *  box functions.                                                          *
 *                                                                          *
 *                                                                          *
 *  Modifications -                                                         *
 *      17-Aug-1989 : Initial version.                                      *
 *      11-Oct-1989 : Removed ErrMessageBox declaration.                    *
 *      11-Oct-1990 : Eliminated FitPathToBox function.                     *
 *                    Added RightArrow and LeftArrow functions.             *
 *                                                                          *
 *  (c)Copyright 1990 Rick Yoder                                            *
 ****************************************************************************/

 extern USHORT ParseFileName( PSZ pszSource,PSZ pszDest,PSZ pszSearch );
    /* Purpose                  This function generates a fully
     *                          qualified file name or search spec.
     *                          from the input value, and changes
     *                          the current drive and directory to
     *                          the drive/directory listed in the
     *                          input file name.
     *
     * Parameters               pszSource points to the input file name
     *                          or search spec.
     *
     *                          pszDest points to the location where
     *                          the fully qualified file name or
     *                          search spec is to be stored.
     *
     *                          pszSearch points to the current file search
     *                          search specification. This string is used
     *                          when the input string is just a drive or
     *                          a drive:\directory without a file name.
     *
     * Return Value             The return value is zero if the function
     *                          is successful, otherwise it is an
     *                          error value. The contents of the buffer
     *                          pointed to by pszDest is undefined if
     *                          an error occurs.
     *
     * Notes                    This function no longer modifies the contents
     *                          of the string pointed to by pszSource.
     *
     *                          This function will now detect an illegal
     *                          file name or search specification.
     */

#if defined(INCL_ARROWS)
 extern MRESULT LeftArrow( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2,
                           HBITMAP hbmLeft,HBITMAP hbmLeftPressed,
                           HBITMAP hbmLeftDisabled );
    /* Purpose                  This function handles BN_PAINT
     *                          messages sent from a left arrow button
     *                          to the dialog box.
     */

 extern MRESULT RightArrow( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2,
                            HBITMAP hbmRight,HBITMAP hbmRightPressed,
                            HBITMAP hbmRightDisabled );
    /* Purpose                  This function handles BN_PAINT messages
     *                          sent from a right arrow button to the
     *                          dialog box.
     */
#endif
/****************************************************************************/
