#ifndef MX_GFX_H
#define MX_GFX_H

#include <stdbool.h> // bool

bool mxGraphicsSetup(unsigned int screen_width, unsigned int screen_height);
void mxGraphicsLookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ);
void mxGraphicsUpdate(float timeSinceLastUpdate);
void mxGraphicsPaint();
void mxGraphicsCleanup();

#endif /* MX_GFX_H */
int();
void mxGraphicsCleanup();

#endif /* MX_GFX_H */
