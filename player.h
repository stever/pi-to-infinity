#ifndef MX_PLAYER_H
#define MX_PLAYER_H

#include <stdbool.h> // bool

bool mxPlayerSetup();
void mxPlayerMoveToStartPosition();
void mxPlayerUpdate(unsigned char moveKeys, float mouseDeltaX, float mouseDeltaY, float timeSinceLastUpdate);

#endif /* MX_PLAYER_H */
AYER_H */
