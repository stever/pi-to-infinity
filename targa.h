/**
 ** Copyright (c) 2005 Michael L. Gleicher
 **
 ** Permission is hereby granted, free of charge, to any person
 ** obtaining a copy of this software and associated documentation
 ** files (the "Software"), to deal in the Software without
 ** restriction, including without limitation the rights to use, copy,
 ** modify, merge, publish, distribute, sublicense, and/or sell copies
 ** of the Software, and to permit persons to whom the Software is
 ** furnished to do so, subject to the following conditions:
 **
 ** The above copyright notice and this permission notice shall be
 ** included in all copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 ** HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 ** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 ** DEALINGS IN THE SOFTWARE.
 **/

/*
NOTE: The original contents of this file have been stripped to load only.
http://research.cs.wisc.edu/graphics/Gallery/LibTarga/
*/

#ifndef MX_TARGA_H
#define MX_TARGA_H

#define byte char
#define int32 int
#define int16 short

#define ubyte unsigned byte
#define uint32 unsigned int32
#define uint16 unsigned int16

/*
Truecolor images supported:

bits            breakdown   components
--------------------------------------
32              8-8-8-8     RGBA
24              8-8-8       RGB
16              5-6-5       RGB
15              5-5-5-1     RGB (ignore extra bit)

Paletted images supported:

index size      palette entry   breakdown   components
------------------------------------------------------
8               <any of above>  <same as above> ..
16              <any of above>  <same as above> ..
24              <any of above>  <same as above> ..
*/

// The 'format' argument to tga_create:
#define TGA_TRUECOLOR_32 (4)
#define TGA_TRUECOLOR_24 (3)
// NOTE: Only TGA_TRUECOLOR_32 supports an alpha channel.

/* Error handling routines: */
uint32 tga_get_last_error();
const char* tga_error_string(int error_code);

/* Loading image  --  a return of NULL indicates a fatal error. */
void* tga_load(const char* file, int* width, int* height, unsigned int format);
/* Image data will start in the low-left corner of the image. */

#endif /* MX_TARGA_H */
