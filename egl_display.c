#include <stdlib.h> // NULL
#include <stdbool.h> // bool, true, false

#include <EGL/egl.h>
#include <EGL/eglext.h>

static EGLDisplay _egl_display;
static EGLSurface _egl_surface;
static EGLContext _egl_context;

///////////////////////////////////////////////////////////////////////////////
bool mxDisplaySetup(unsigned int* screen_width, unsigned int* screen_height)
{
    // NOTE: The content of this function is largely based on the triangle.c
    //       source file that was provided as an example for Raspberry Pi.
    
    // https://github.com/raspberrypi/firmware/blob/master/opt/vc/src/hello_pi/hello_triangle/triangle.c
    
    /* 
     * Copyright (c) 2012 Broadcom Europe Ltd
     *
     * Licensed to the Apache Software Foundation (ASF) under one or more
     * contributor license agreements.  See the NOTICE file distributed with
     * this work for additional information regarding copyright ownership.
     * The ASF licenses this file to You under the Apache License, Version 2.0
     * (the "License"); you may not use this file except in compliance with
     * the License.  You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
     * See the License for the specific language governing permissions and
     * limitations under the License.
     */
    
	// Get an EGL display connection.
	_egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (_egl_display == EGL_NO_DISPLAY) return false;

	// Initialise the EGL display connection.
    EGLBoolean result = eglInitialize(_egl_display, NULL, NULL);
    if (result == EGL_FALSE) return false;

	// EGL frame buffer configuration.
	static const EGLint attribute_list[] =
	{
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE
	};
	EGLConfig config;
    EGLint num_config;
	result = eglChooseConfig(_egl_display, attribute_list, &config, 1, &num_config);
    if (result == EGL_FALSE) return false;

	// Create an EGL rendering context.
    static const EGLint context_attributes[] = 
    {
       // EGL_CONTEXT_CLIENT_VERSION, 2,
       EGL_NONE
    };
	_egl_context = eglCreateContext(_egl_display, config, EGL_NO_CONTEXT, context_attributes);
    if (_egl_context == EGL_NO_CONTEXT) return false;

	// Create an EGL window surface.
    int success = graphics_get_display_size(0/* LCD */, screen_width, screen_height);
    if (success < 0) return false;

	VC_RECT_T dst_rect;
	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = *screen_width;
	dst_rect.height = *screen_height;
	  
	VC_RECT_T src_rect;
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = *screen_width << 16;
	src_rect.height = *screen_height << 16;        

	DISPMANX_DISPLAY_HANDLE_T dispman_display = vc_dispmanx_display_open(0/* LCD */);
	DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
	DISPMANX_ELEMENT_HANDLE_T dispman_element = vc_dispmanx_element_add(
        dispman_update, dispman_display, 0/*layer*/, &dst_rect, 0/*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, 0/*alpha*/, 0/*clamp*/, 0/*transform*/);
	  
	static EGL_DISPMANX_WINDOW_T nativewindow;
	nativewindow.element = dispman_element;
	nativewindow.width = *screen_width;
	nativewindow.height = *screen_height;
	vc_dispmanx_update_submit_sync(dispman_update);
	  
	_egl_surface = eglCreateWindowSurface(_egl_display, config, &nativewindow, NULL);
    if (_egl_surface == EGL_NO_SURFACE) return false;

	// Connect the context to the surface.
	result = eglMakeCurrent(_egl_display, _egl_surface, _egl_surface, _egl_context);
    if (result == EGL_FALSE) return false;
    
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void mxDisplaySwapBuffers()
{
    eglSwapBuffers(_egl_display, _egl_surface);
}

///////////////////////////////////////////////////////////////////////////////
void mxDisplayCleanup()
{
    eglMakeCurrent(_egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroySurface(_egl_display, _egl_surface);
    eglDestroyContext(_egl_display, _egl_context);
    eglTerminate(_egl_display);
}
