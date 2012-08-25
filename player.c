#include "player.h"
#include "keyboard.h"
#include "gfx_engine.h"

#include <math.h>

#define MOUSE_LOOK_SPEED 0.075f
#define MOVEMENT_SPEED 0.075f
#define TURN_SPEED 0.1f
#define MAX_PITCH 89.99f
#define MIN_PITCH -MAX_PITCH

#ifndef M_PI
#define M_PI 3.141592654
#endif

static float _upX;
static float _upY;
static float _upZ;
static float _posX;
static float _posY;
static float _posZ;
static float _pitch;
static float _yaw;

///////////////////////////////////////////////////////////////////////////////
bool mxPlayerSetup()
{
    _upX = 0.f;
    _upY = 1.f;
    _upZ = 0.f;
    mxPlayerMoveToStartPosition();
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void mxPlayerMoveToStartPosition()
{
    _posX = 0.f;
    _posY = 0.f;
    _posZ = -100.f;
    _pitch = 0.f;
    _yaw = 90.f;
}

///////////////////////////////////////////////////////////////////////////////
static float limit_pitch(float pitch)
{
    if (pitch > MAX_PITCH) pitch = MAX_PITCH;
    else if (pitch < MIN_PITCH) pitch = MIN_PITCH;
    return pitch;
}

///////////////////////////////////////////////////////////////////////////////
static float normal_yaw(float yaw)
{
    while (yaw >= 360.0f) yaw -= 360.0f;
    while (yaw < 0.0f) yaw += 360.0f;
    return yaw;
}

///////////////////////////////////////////////////////////////////////////////
static float degrees_to_radians(float degrees)
{
	return degrees * (M_PI / 180.f);
}

///////////////////////////////////////////////////////////////////////////////
static void move(float x, float y, float z)
{
    _posX += x;
    _posY += y;
    _posZ += z;
}

///////////////////////////////////////////////////////////////////////////////
static void move_forward(float amount)
{
    float cosYaw = cos(degrees_to_radians(_yaw));
    float sinYaw = sin(degrees_to_radians(_yaw));
    float x = cosYaw * amount;
    float z = sinYaw * amount;
    move(x, 0, z);
}

///////////////////////////////////////////////////////////////////////////////
static void move_sideways(float amount)
{
    float angle = _yaw + 90;
    double cosYaw = cos(degrees_to_radians(normal_yaw(angle)));
    double sinYaw = sin(degrees_to_radians(normal_yaw(angle)));
    float x = cosYaw * amount;
    float z = sinYaw * amount;
    move(x, 0, z);
}

///////////////////////////////////////////////////////////////////////////////
void mxPlayerUpdate(unsigned char moveKeys, 
					float mouseDeltaX, float mouseDeltaY, 
					float timeSinceLastUpdate)
{
    float moveAmount = MOVEMENT_SPEED * timeSinceLastUpdate;
    float mouseMoveAmount = MOUSE_LOOK_SPEED * timeSinceLastUpdate;

    // Move forward and backward.
    if (moveKeys & MOVE_FORWARD && moveKeys & MOVE_BACK)
    {
        if (moveKeys & MOVE_FORWARD_OVER_BACK) move_forward(moveAmount);
        else move_forward(-moveAmount);
    }
    else if (moveKeys & MOVE_FORWARD) move_forward(moveAmount);
    else if (moveKeys & MOVE_BACK) move_forward(-moveAmount);

    // Move left and right.
    if (moveKeys & MOVE_LEFT && moveKeys & MOVE_RIGHT)
    {
        if (moveKeys & MOVE_LEFT_OVER_RIGHT) move_sideways(-moveAmount);
        else move_sideways(moveAmount);
    }
    else if (moveKeys & MOVE_LEFT) move_sideways(-moveAmount);
    else if (moveKeys & MOVE_RIGHT) move_sideways(moveAmount);
    
    // Move up and down.
    // TODO: This control is for testing purposes only.
    // NOTE: There's no MOVE_UP_OVER_DOWN due to not having available bits, and this will be removed later anyway.
    if (moveKeys & MOVE_UP) move(0, moveAmount, 0);
    else if (moveKeys & MOVE_DOWN) move(0, -moveAmount, 0);

    // Mouse control.
    _yaw = normal_yaw(_yaw + (mouseDeltaX * mouseMoveAmount));
    _pitch = limit_pitch(_pitch + (mouseDeltaY * -mouseMoveAmount));

    // Calculate view target based on new position.
    float viewTargetX = _posX + cos(degrees_to_radians(_yaw));
    float viewTargetY = _posY + tan(degrees_to_radians(_pitch));
    float viewTargetZ = _posZ + sin(degrees_to_radians(_yaw));

    // Update view position.
    mxGraphicsLookAt(_posX, _posY, _posZ, viewTargetX, viewTargetY, viewTargetZ);
}
X, _upY, _upZ);
}
//////////////////////////////////////////////////////
void mxPlayerCleanup()
{
    // Nothing to do currently.
}
