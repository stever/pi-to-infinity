// Enable or disable debugging in this file.
#define DEBUG_THIS

#ifdef DEBUG_THIS
#include "debug.h"
#endif

#include "display.h" // mxDisplaySwapBuffers
#include "targa.h" // tga_load, TGA_TRUECOLOR_32, tga_error_string, tga_get_last_error

#include <stdlib.h> // free
#include <math.h> // tan, sqrt

#include <GLES/gl.h>
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>

// Textures are 16x16 pixels square.
#define TEXTURE_IMAGE_SIZE 16

// This is the number of textures to be loaded.
#define TEXTURE_COUNT 3

// The following indexes the available textures.
#define TEX_DIRT_SIDE 		(0)
#define TEX_DIRT_TOP 		(1)
#define TEX_DIRT_BOTTOM 	(2)

#ifndef M_PI
#define M_PI 3.141592654
#endif

static GLuint _tex[TEXTURE_COUNT];

static const GLbyte _voxel_vertices[6 * 4 * 3] = {

    // Front
    -10, -10,  10,
     10, -10,  10,
    -10,  10,  10,
     10,  10,  10,

    // Back
    -10, -10, -10,
    -10,  10, -10,
     10, -10, -10,
     10,  10, -10,

    // Left
    -10, -10,  10,
    -10,  10,  10,
    -10, -10, -10,
    -10,  10, -10,

    // Right
     10, -10, -10,
     10,  10, -10,
     10, -10,  10,
     10,  10,  10,

    // Top
    -10,  10,  10,
     10,  10,  10,
    -10,  10, -10,
     10,  10, -10,

    // Bottom
    -10, -10,  10,
    -10, -10, -10,
     10, -10,  10,
     10, -10, -10,
};

// Texture coordinates.
static const GLbyte _voxel_tex_coords[6 * 4 * 2] = {

    1,  0,    
    0,  0,
    1,  1,
    0,  1,
         
    0,  0,
    0,  1,
    1,  0,
    1,  1,
         
    0,  0,
    0,  1,
    1,  0,
    1,  1,
         
    0,  0,
    0,  1,
    1,  0,
    1,  1,
         
    1,  0,
    1,  1,
    0,  0,
    0,  1,

    0,  0,
    0,  1,
    1,  0,
    1,  1
};

typedef struct
{
	GLbyte pos_x;
	GLbyte pos_y;
	GLbyte pos_z;
    unsigned char type;
} MX_VOXEL_T;

static MX_VOXEL_T _voxels[1000];
static int _voxels_count;

///////////////////////////////////////////////////////////////////////////////
static unsigned char* load_texture(char* filename)
{
    int w, h;
    unsigned char* tex = (unsigned char*) tga_load(filename, &w, &h, TGA_TRUECOLOR_32);
    if (tex == NULL) mxDebugStr(tga_error_string(tga_get_last_error()));
    mxDebug("width = %d; height = %d", w, h);
    return tex;
}

///////////////////////////////////////////////////////////////////////////////
static void load_textures()
{
    // TEX_DIRT_SIDE
    unsigned char* _tex_side = load_texture("terrain/block_dirt_side.tga");
    glBindTexture(GL_TEXTURE_2D, _tex[TEX_DIRT_SIDE]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_IMAGE_SIZE, TEXTURE_IMAGE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, _tex_side);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat) GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat) GL_NEAREST);
    free(_tex_side);

    // TEX_DIRT_TOP
    unsigned char* _tex_top = load_texture("terrain/block_dirt_top.tga");
    glBindTexture(GL_TEXTURE_2D, _tex[TEX_DIRT_TOP]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_IMAGE_SIZE, TEXTURE_IMAGE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, _tex_top);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat) GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat) GL_NEAREST);
    free(_tex_top);

    // TEX_DIRT_BOTTOM
    unsigned char* _tex_bottom = load_texture("terrain/block_dirt_bottom.tga");
    glBindTexture(GL_TEXTURE_2D, _tex[TEX_DIRT_BOTTOM]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_IMAGE_SIZE, TEXTURE_IMAGE_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, _tex_bottom);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLfloat) GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLfloat) GL_NEAREST);
    free(_tex_bottom);
}

///////////////////////////////////////////////////////////////////////////////
static void paint_voxel(GLbyte pos_x, GLbyte pos_y, GLbyte pos_z, unsigned char type)
{
    glPushMatrix();
    
    unsigned char front, back, left, right, top, bottom;
    switch (type)
    {
        case 0:
            front = back = left = right = _tex[TEX_DIRT_SIDE];
            top = _tex[TEX_DIRT_TOP];
            bottom = _tex[TEX_DIRT_BOTTOM];
            break;
    }
    
    // Translate to position
    glTranslatef(pos_x, pos_y, pos_z);
        
    // Front
    glBindTexture(GL_TEXTURE_2D, front);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Back
    glBindTexture(GL_TEXTURE_2D, back);
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    // Left
    glBindTexture(GL_TEXTURE_2D, left);
    glDrawArrays(GL_TRIANGLE_STRIP, 8, 4);

    // Right
    glBindTexture(GL_TEXTURE_2D, right);
    glDrawArrays(GL_TRIANGLE_STRIP, 12, 4);

    // Top
    glBindTexture(GL_TEXTURE_2D, top);
    glDrawArrays(GL_TRIANGLE_STRIP, 16, 4);

    // Bottom
    glBindTexture(GL_TEXTURE_2D, bottom);
    glDrawArrays(GL_TRIANGLE_STRIP, 20, 4);
    
    glPopMatrix();
}

///////////////////////////////////////////////////////////////////////////////
bool mxGraphicsSetup(unsigned int screen_width, unsigned int screen_height)
{    
    // OpenGL set-up.
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_BYTE, 0, &_voxel_vertices);
    glTexCoordPointer(2, GL_BYTE, 0, &_voxel_tex_coords);
    glGenTextures(TEXTURE_COUNT, &_tex[0]);
    load_textures();

    // Configure the viewport. TODO: Screen size changes after init?
    glViewport(0, 0, (GLsizei) screen_width, (GLsizei) screen_height);

    // Set-up the view frustum.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fovy = 45.f;
    float aspect = (float) screen_width / (float) screen_height;
    float zNear = 1.0f;
    float zFar = 1000.0f;
    float yMax = zNear * (float) tan(fovy * M_PI / 360.0);
    float yMin = -yMax;
    float xMin = yMin * aspect;
    float xMax = yMax * aspect;
    glFrustumf(xMin, xMax, yMin, yMax, zNear, zFar);

    // TODO: Plot chunks instead of single cubes.
	// TODO: To replace this with Lua script.
    int n = 1;
    for (int z = -n; z <= n; z++)
    {
        for (int x = -n; x <= n; x++)
        {
            for (int y = -n; y <= n; y++)
            {
                MX_VOXEL_T *voxel = &_voxels[_voxels_count++];
                voxel->pos_x = x * 20;
                voxel->pos_z = z * 20;
                voxel->pos_y = y * 20;
                voxel->type = 0;
            }
        }
    }
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// This function is based on gluLookAt() from the Mesa3D library (MIT license).
///////////////////////////////////////////////////////////////////////////////
void mxGraphicsLookAt(float eyeX, float eyeY, float eyeZ, float centerX, float centerY, float centerZ)
{
    // NOTE: We don't ever change the up vector in this game.
    static const float upX = 0.f;
    static const float upY = 1.f;
    static const float upZ = 0.f;

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ///////////////////////////////////////////////////////////////////////////////
    // Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
    // 
    // Permission is hereby granted, free of charge, to any person obtaining a
    // copy of this software and associated documentation files (the "Software"),
    // to deal in the Software without restriction, including without limitation
    // the rights to use, copy, modify, merge, publish, distribute, sublicense,
    // and/or sell copies of the Software, and to permit persons to whom the
    // Software is furnished to do so, subject to the following conditions:
    // 
    // The above copyright notice and this permission notice shall be included
    // in all copies or substantial portions of the Software.
    //
    // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    // OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
    // BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
    // AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    // CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
    ///////////////////////////////////////////////////////////////////////////////

    // Make rotation matrix.

    // Z vector
    GLfloat z[3];
    z[0] = eyeX - centerX;
    z[1] = eyeY - centerY;
    z[2] = eyeZ - centerZ;

    GLfloat mag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag)
    {
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    // Y vector
    GLfloat y[3];
    y[0] = upX;
    y[1] = upY;
    y[2] = upZ;

    // X vector = Y cross Z
    GLfloat x[3];
    x[0] =  y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] =  y[0] * z[1] - y[1] * z[0];

    // Recompute Y = Z cross X
    y[0] =  z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] =  z[0] * x[1] - z[1] * x[0];

    // Cross product gives area of parallelogram, which is < 1.0 for
    // non-perpendicular unit-length vectors; so normalize x, y here.

    mag = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

    GLfloat m[16];
#define M(row, col) m[col * 4 + row]
    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
#undef M
    glMultMatrixf(m);

    // Translate Eye to Origin.
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

///////////////////////////////////////////////////////////////////////////////
void mxGraphicsUpdate(float timeSinceLastUpdate)
{
}

///////////////////////////////////////////////////////////////////////////////
void mxGraphicsPaint()
{
    // Set background color and clear buffers.
    // NOTE: Console text remains visible if you either use alpha transparency
    //       or don't set the colour at all.
    // Color.SkyBlue (.NET) is #87CEEB = 135, 206, 235
    glClearColor((float) 135 / 255, (float) 127 / 255, (float) 235 / 255, 0.80f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Render world.
    // TODO: Use default shader program.
    // TODO: Render chunks.

	// Paint voxels.
    for (int i = 0; i < _voxels_count; i++)
        paint_voxel(_voxels[i].pos_x, _voxels[i].pos_y, _voxels[i].pos_z, _voxels[i].type);
    
    glDisable(GL_CULL_FACE);
    
    // TODO: Use chunk alpha shader program.
    // TODO: Render chunks.

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    // Show the re-painted display.
    mxDisplaySwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////
void mxGraphicsCleanup()
{
}
