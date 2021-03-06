/****************************************************************************
 * BUTTONS.C                                                                *
 *                                                                          *
 *  This file contains the functions used to draw the right arrow           *
 *  and left arrow buttons.                                                 *
 *                                                                          *
 *  Modifications -                                                         *
 *      11-Oct-1990 : Initial version.                                      *
 ****************************************************************************/

#define INCL_WINBUTTONS
#define INCL_WINMESSAGEMGR
#define INCL_GPIPRIMITIVES
#define INCL_GPIBITMAPS
#define INCL_WINDIALOGS
#include <os2.h>
#define INCL_ARROWS
#include "tools.h"

/****************************************************************************
 * LeftArrow() - Procedure to handle BN_PAINT messages generated by the     *
 *               left arrow button control.                                 *
 ****************************************************************************/
MRESULT LeftArrow( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2,
                   HBITMAP hbmLeft,HBITMAP hbmLeftPressed,
                   HBITMAP hbmLeftDisabled )

    {
        PUSERBUTTON         pUserButton;
        HBITMAP             hbm;
        POINTL              ptl;
        BITMAPINFOHEADER    bmp;
        LONG                clr;

        if ( SHORT2FROMMP(mp1) == BN_PAINT )
            {
            pUserButton = (PUSERBUTTON)PVOIDFROMMP(mp2);

            if ( pUserButton->fsState & BDS_DISABLED )
                hbm = hbmLeftDisabled;
            else if ( pUserButton->fsState & BDS_HILITED )
                hbm = hbmLeftPressed;
            else
                hbm = hbmLeft;

            clr = (pUserButton->fsState & BDS_DEFAULT) ? CLR_BLACK : CLR_DARKGRAY;

            ptl.x = 0L; ptl.y = 0L;
            WinDrawBitmap( pUserButton->hps,hbm,NULL,&ptl,
                           clr,CLR_WHITE,DBM_NORMAL );
            GpiQueryBitmapParameters( hbm,&bmp );
            GpiMove( pUserButton->hps,&ptl );
            ptl.x = bmp.cx; ptl.y = bmp.cy;
            GpiSetColor( pUserButton->hps,clr );
            GpiBox( pUserButton->hps,DRO_OUTLINE,&ptl,0L,0L );
            return 0L;
            }

        else
            return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/


/****************************************************************************
 * RightArrow() - Procedure to handle BN_PAINT messages generated by the    *
 *                right arrow button control.                               *
 ****************************************************************************/
MRESULT RightArrow( HWND hwnd,USHORT msg,MPARAM mp1,MPARAM mp2,
                    HBITMAP hbmRight,HBITMAP hbmRightPressed,
                    HBITMAP hbmRightDisabled )
    {
        PUSERBUTTON         pUserButton;
        HBITMAP             hbm;
        POINTL              ptl;
        BITMAPINFOHEADER    bmp;
        LONG                clr;

        if ( SHORT2FROMMP(mp1) == BN_PAINT )
            {
            pUserButton = (PUSERBUTTON)PVOIDFROMMP(mp2);

            if ( pUserButton->fsState & BDS_DISABLED )
                hbm = hbmRightDisabled;
            else if ( pUserButton->fsState & BDS_HILITED )
                hbm = hbmRightPressed;
            else
                hbm = hbmRight;

            clr = (pUserButton->fsState & BDS_DEFAULT) ? CLR_BLACK : CLR_DARKGRAY;

            ptl.x = 0L; ptl.y = 0L;
            WinDrawBitmap( pUserButton->hps,hbm,NULL,&ptl,
                           CLR_BLACK,CLR_WHITE,DBM_NORMAL );
            GpiQueryBitmapParameters( hbm,&bmp );
            GpiMove( pUserButton->hps,&ptl );
            ptl.x = bmp.cx; ptl.y = bmp.cy;
            GpiSetColor( pUserButton->hps,clr );
            GpiBox( pUserButton->hps,DRO_OUTLINE,&ptl,0L,0L );
            return 0L;
            }

        else
            return WinDefDlgProc( hwnd,msg,mp1,mp2 );
    }
/****************************************************************************/
