/****************************************************************************
 * EXAMPLE.C -- Example program showing how to use the ErrMessageBox        *
 *              function with a user defined error list.                    *
 ****************************************************************************/
#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINDIALOGS
#include <os2.h>
#include <errmsg.h>

ERRORMSG errmsg[] = {
    1,MB_OK|MB_ICONEXCLAMATION,"Message with no arguments.",
    2,MB_OK|MB_ICONEXCLAMATION,"Test message with\n  int argument = %i\n  string argument = \"%s\".",
    3,MB_OK|MB_ICONEXCLAMATION,"Test message with\n  floating point argument = %lf.\n",
    4,MB_YESNO|MB_ICONQUESTION,"Message asking for user response."
    };
#define COUNT (sizeof(errmsg) / sizeof(ERRORMSG))

void main( void )
{
    HAB     hab;    /* handle to the anchor block  */
    HMQ     hmq;    /* handle to the message queue */

    hab = WinInitialize( 0 );
    hmq = WinCreateMsgQueue(hab, DEFAULT_QUEUE_SIZE);

    ErrMessageBox( HWND_DESKTOP,NULL,1,errmsg,COUNT );
    ErrMessageBox( HWND_DESKTOP,NULL,2,errmsg,COUNT,100,(PSZ)"INSERTED STRING" );
    ErrMessageBox( HWND_DESKTOP,NULL,3,errmsg,COUNT,3.1415926 );
    ErrMessageBox( HWND_DESKTOP,NULL,4,errmsg,COUNT );

    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
}
