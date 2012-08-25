#ifndef MX_DISPLAY_H
#define MX_DISPLAY_H

#include <stdbool.h> // bool

bool mxDisplaySetup(unsigned int* screen_width, unsigned int* screen_height);
void mxDisplaySwapBuffers();
void mxDisplayCleanup();

#endif /* MX_DISPLAY_H */
