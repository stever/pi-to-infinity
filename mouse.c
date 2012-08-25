// Enable or disable debugging in this file.
//#define DEBUG_THIS

#ifdef DEBUG_THIS
#include "debug.h"
#endif

#include <stdbool.h> // bool, true, false
#include <sys/time.h> // FD_ZERO, FD_SET, FD_ISSET, select
#include <gpm.h> // Gpm_GetEvent

///////////////////////////////////////////////////////////////////////////////
// Callback function for gpm_handler. This function is just used for logging.
///////////////////////////////////////////////////////////////////////////////
static int mouse_handler(Gpm_Event *event, void *data)
{
#ifdef DEBUG_THIS
    mxDebug("%d at x=%d y=%d", event->type, event->dx, event->dy);
#endif
    return 0;       
}

///////////////////////////////////////////////////////////////////////////////
// Returns false if connection with the mouse server could not be established.
///////////////////////////////////////////////////////////////////////////////
bool mxMouseSetup()
{
    // Try to connect to the mouse server.
    static Gpm_Connect conn;
    conn.eventMask = ~0;
    conn.defaultMask = ~GPM_HARD;
    conn.minMod = 0;
    conn.maxMod = ~0;
    if (Gpm_Open(&conn, 0) == -1) return false;

    // Register the mouse_handler callback function.
    gpm_handler = mouse_handler;    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// This function is called by the main event loop to fetch state of the mouse.
///////////////////////////////////////////////////////////////////////////////
void mxMouseUpdate(float *deltaX, float *deltaY, 
                   unsigned char *buttons, unsigned char *eventType)
{
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(gpm_fd, &readset);

    // Zero timevalue to not block on select for Gpm_GetEvent.
    static struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    while (select(gpm_fd + 1, &readset, 0, 0, &tv) > 0)
    {
        if (FD_ISSET(gpm_fd, &readset))
        {
            Gpm_Event event;
            if (Gpm_GetEvent(&event) > 0)
            {
#ifdef DEBUG_THIS
                mxDebug("type 0x%02X, at %3i %3i (delta %3i %3i), buttons 0x%02X, modifiers 0x%02X",
                        event.type,
                        event.x, event.y,
                        event.dx, event.dy,
                        event.buttons,
                        event.modifiers
                );
#endif
                
                *deltaX += event.dx;
                *deltaY += event.dy;
                *buttons = event.buttons;
                *eventType = event.type;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void mxMouseCleanup()
{
    Gpm_Close();
}