// Enable or disable debugging in this file.
#define DEBUG_THIS

#ifdef DEBUG_THIS
#include "debug.h"
#endif

#include "display.h"
#include "gfx_engine.h"
#include "keyboard.h"
#include "mouse.h"
#include "player.h"

#include <stdlib.h> // exit
#include <stdio.h> // printf
#include <signal.h> // signal, SIGINT, etc.
#include <bcm_host.h> // bcm_host_init
#include <sys/time.h> // gettimeofday
#include <gpm.h> // GPM_B_LEFT, GPM_B_RIGHT, GPM_DOWN, GPM_UP

// This variable is used to terminate the main event loop.
static volatile bool _terminate;

///////////////////////////////////////////////////////////////////////////////
// This function ends the main event loop cleanly, in response to exit signals.
///////////////////////////////////////////////////////////////////////////////
static void exit_handler(int sig)
{
#ifdef DEBUG_THIS
    mxDebug("Caught signal: %d", sig);
#endif

    // Normal loop termination.
    _terminate = true;

    // NOTE: Segmentation fault was not terminating the loop normally.
    //       Ensure that we have an operable keyboard in this case!
    if (sig == SIGSEGV)
        mxKeyboardCleanup();
    
    // Reset signal handler to default value.
    // TODO: Check what effect this has. Is it doing anything useful?
    signal(sig, SIG_DFL);
}

///////////////////////////////////////////////////////////////////////////////
// This function returns the number of milliseconds since the epoch.
///////////////////////////////////////////////////////////////////////////////
static double time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double) tv.tv_sec * 1000.0 + (double) tv.tv_usec / 1000.0;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // Exit handler setup.
    signal(SIGINT, exit_handler); // Ctrl-c
	signal(SIGHUP, exit_handler);
	signal(SIGQUIT, exit_handler);
	signal(SIGILL, exit_handler);
	signal(SIGTRAP, exit_handler);
	signal(SIGABRT, exit_handler);
	signal(SIGIOT, exit_handler);
	signal(SIGFPE, exit_handler);
	signal(SIGKILL, exit_handler);
	signal(SIGUSR1, exit_handler);
	signal(SIGSEGV, exit_handler);
	signal(SIGUSR2, exit_handler);
	signal(SIGPIPE, exit_handler);
	signal(SIGTERM, exit_handler);
#ifdef SIGSTKFLT
	signal(SIGSTKFLT, exit_handler);
#endif
	signal(SIGCHLD, exit_handler);
	signal(SIGCONT, exit_handler);
	signal(SIGSTOP, exit_handler);
	signal(SIGTSTP, exit_handler);
	signal(SIGTTIN, exit_handler);
	signal(SIGTTOU, exit_handler);
    
    // Variables used in main loop.
    double t; // current time.
    double lastTime = time();
    float timeSinceLastUpdate = 0.f;
    int frameCounter = 0;
    int frameCounterMillis = 0;
    float mouseDeltaX, mouseDeltaY = 0.f;
    unsigned char mouseButtons = 0;
    unsigned char eventType = 0;
    unsigned char moveKeys = 0;
    unsigned char specialKeys = 0;

	// Initial setup.
    double start = time();
#ifdef DEBUG_THIS
    mxDebugStr("** Started **");
#endif
    bcm_host_init();
    uint32_t screen_width, screen_height;
    if (!mxKeyboardSetup()) _terminate = true;
    if (!_terminate && !mxMouseSetup()) _terminate = true;
    if (!_terminate && !mxDisplaySetup(&screen_width, &screen_height)) _terminate = true;
    if (!_terminate && !mxGraphicsSetup(screen_width, screen_height)) _terminate = true;
    if (!_terminate && !mxPlayerSetup()) _terminate = true;

    // Loop until terminated or exit key is pressed.
    while (!_terminate)
    {
        // Measure time between each frame for smooth animation.
        timeSinceLastUpdate = (float) ((t = time()) - lastTime);
        lastTime = t;

        // Log FPS once a second.
        frameCounterMillis += (int) timeSinceLastUpdate;
        if (frameCounterMillis >= 1000)
        {
            mxDebug("FPS: %d", frameCounter);
            frameCounterMillis -= 1000;
            frameCounter = 0;
        }
        frameCounter++;

        // Handle controls.
        mouseDeltaX = mouseDeltaY = 0.f;
        mxMouseUpdate(&mouseDeltaX, &mouseDeltaY, &mouseButtons, &eventType);
        mxKeyboardUpdate(&moveKeys, &specialKeys);

        // Handle special keys.
        if (specialKeys & KEY_EXIT) break;
        if (specialKeys & KEY_RESET_POS) mxPlayerMoveToStartPosition();
        specialKeys = 0;

        // Update and paint new frame.
        mxGraphicsUpdate(timeSinceLastUpdate);
        mxPlayerUpdate(moveKeys, mouseDeltaX, mouseDeltaY, timeSinceLastUpdate);
        mxGraphicsPaint();
    }
    
    // Cleanup and shutdown gracefully.
    mxGraphicsCleanup();
    mxDisplayCleanup();
    mxMouseCleanup();
    mxKeyboardCleanup();
#ifdef DEBUG_THIS
    mxDebug("** Finished (%f seconds elapsed) **", (time() - start) / 1000.f);    
#endif
    return 0;
}
#endif
    return 0;
}
