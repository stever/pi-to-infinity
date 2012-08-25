#ifndef MX_MOUSE_H
#define MX_MOUSE_H

#include <stdbool.h> // bool

bool mxMouseSetup();
void mxMouseUpdate(float* deltaX, float* deltaY, 
				   unsigned char* buttons, unsigned char* eventType);
void mxMouseCleanup();

#endif /* MX_MOUSE_H */