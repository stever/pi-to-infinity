///////////////////////////////////////////////////////////////////////////////
// This file is responsible for handling keyboard input. The mode for the
// console is changed so that raw scancodes can be used to get the key up event
// as well as key down. This requires some care as the console is rendered more
// or less useless if the mode is not restored.
//
// Initial hints for dealing with scancodes were found in this old article:
// http://www.linuxjournal.com/article/1080
//
// The command 'showkey -s' outputs the kind of info we're interested in and
// that provided an example of how to read the console in this manner.
//
// See showkey.c in ftp://ftp.kernel.org/pub/linux/utils/kbd/kbd-1.12.tar.gz
///////////////////////////////////////////////////////////////////////////////

// Enable or disable debugging in this file.
//#define DEBUG_THIS

#ifdef DEBUG_THIS
#include "debug.h"
#endif

#include "keyboard.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> // bool, true, false
#include <signal.h>
#include <fcntl.h>
#include <termios.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h> // open, read

static int _fd;
static int _oldkbmode;
static unsigned char _buf[18]; /* divisible by 3 */
static struct termios _old;

///////////////////////////////////////////////////////////////////////////////
bool mxKeyboardSetup()
{
	_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);

	// Store the keyboard mode to be restored.
    ioctl(_fd, KDGKBMODE, &_oldkbmode);

#ifdef DEBUG_THIS
	switch (_oldkbmode)
	{
	  case K_RAW: mxDebug("Keyboard mode was %s", "RAW"); break;
	  case K_XLATE: mxDebug("Keyboard mode was %s", "XLATE"); break;
	  case K_MEDIUMRAW: mxDebug("Keyboard mode was %s", "MEDIUMRAW"); break;
	  case K_UNICODE: mxDebug("Keyboard mode was %s", "UNICODE"); break;
	  default: mxDebug("Keyboard mode was %s", "?UNKNOWN?");
	}
#endif

	struct termios new;
	tcgetattr(_fd, &_old);
	tcgetattr(_fd, &new);
	new.c_lflag &= ~ (ICANON | ECHO | ISIG);
	new.c_iflag = 0;
	new.c_cc[VMIN] = sizeof(_buf);
	new.c_cc[VTIME] = 1; /* 0.1 sec intercharacter timeout */
    tcsetattr(_fd, TCSAFLUSH, &new);

    ioctl(_fd, KDSKBMODE, K_MEDIUMRAW);
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void mxKeyboardUpdate(unsigned char *moveKeys, unsigned char *specialKeys)
{
    int keycode;
    bool press;
	int i = 0;
	int n = read(_fd, _buf, sizeof(_buf));
	while (i < n)
	{
	    // Determine if key pressed or released.
        press = _buf[i] & 0x80 ? false : true;
        
		if (i + 2 < n && (_buf[i] & 0x7f) == 0
        			  && (_buf[i + 1] & 0x80) != 0
            		  && (_buf[i + 2] & 0x80) != 0)
		{
			keycode = ((_buf[i + 1] & 0x7f) << 7) |
				       (_buf[i + 2] & 0x7f);
			i += 3;
		}
		else 
		{
			keycode = (_buf[i] & 0x7f);
			i++;
		}
		
#ifdef DEBUG_THIS
		mxDebug("Keycode: %3d %s", keycode, press ? "press" : "release");
#endif

        switch (keycode)
        {
            case 1:
                *specialKeys |= KEY_EXIT;
                break;
            case 88:
                *specialKeys |= KEY_RESET_POS;
                break;
            case 17:
            case 103:
                if (press) 
                {
                    *moveKeys |= MOVE_FORWARD;
                    *moveKeys |= MOVE_FORWARD_OVER_BACK;
                }
                else 
                {
                    *moveKeys &= (0xFF & ~MOVE_FORWARD);
                }
                break;
            case 31:
            case 108:
                if (press) 
                {
                    *moveKeys |= MOVE_BACK;
                    *moveKeys &= (0xFF & ~MOVE_FORWARD_OVER_BACK);
                }
                else 
                {
                    *moveKeys &= (0xFF & ~MOVE_BACK);
                }
                break;
            case 30:
            case 105:
                if (press) 
                {
                    *moveKeys |= MOVE_LEFT;
                    *moveKeys |= MOVE_LEFT_OVER_RIGHT;
                }
                else 
                {
                    *moveKeys &= (0xFF & ~MOVE_LEFT);
                }
                break;
            case 32:
            case 106:
                if (press) 
                {
                    *moveKeys |= MOVE_RIGHT;
                    *moveKeys &= (0xFF & ~MOVE_LEFT_OVER_RIGHT);
                }
                else 
                {
                    *moveKeys &= (0xFF & ~MOVE_RIGHT);
                }
                break;
            case 13:
                if (press)
                {
                    *moveKeys |= MOVE_UP;
                }
                else
                {
                    *moveKeys &= (0xFF & ~MOVE_UP);
                }
                break;
            case 12:
                if (press)
                {
                    *moveKeys |= MOVE_DOWN;
                }
                else
                {
                    *moveKeys &= (0xFF & ~MOVE_DOWN);
                }
                break;
        }
	}
}

///////////////////////////////////////////////////////////////////////////////
void mxKeyboardCleanup()
{
	// Restore the old keyboard mode.
    ioctl(_fd, KDSKBMODE, _oldkbmode);
	tcsetattr(_fd, 0, &_old);
	close(_fd);
}