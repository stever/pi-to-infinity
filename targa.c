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

#include <stdio.h>
#if !defined(__APPLE__)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

#include "targa.h"

/* uncomment this line if you're compiling on a big-endian machine */
/* #define WORDS_BIGENDIAN */

#define TGA_IMG_NODATA        (0)
#define TGA_IMG_UNC_PALETTED  (1)
#define TGA_IMG_UNC_TRUECOLOR (2)
#define TGA_IMG_UNC_GRAYSCALE (3)
#define TGA_IMG_RLE_PALETTED  (9)
#define TGA_IMG_RLE_TRUECOLOR (10)
#define TGA_IMG_RLE_GRAYSCALE (11)

#define TGA_LOWER_LEFT  (0)
#define TGA_LOWER_RIGHT (1)
#define TGA_UPPER_LEFT  (2)
#define TGA_UPPER_RIGHT (3)

#define HDR_IDLEN              (0)
#define HDR_CMAP_TYPE          (1)
#define HDR_IMAGE_TYPE         (2)
#define HDR_CMAP_FIRST         (3)
#define HDR_CMAP_LENGTH        (5)
#define HDR_CMAP_ENTRY_SIZE    (7)
#define HDR_IMG_SPEC_WIDTH     (12)
#define HDR_IMG_SPEC_HEIGHT    (14)
#define HDR_IMG_SPEC_PIX_DEPTH (16)
#define HDR_IMG_SPEC_IMG_DESC  (17)
#define HDR_LENGTH             (18)

#define TGA_ERR_NONE                    (0)
#define TGA_ERR_BAD_HEADER              (1)
#define TGA_ERR_OPEN_FAILS              (2)
#define TGA_ERR_BAD_FORMAT              (3)
#define TGA_ERR_UNEXPECTED_EOF          (4)
#define TGA_ERR_NODATA_IMAGE            (5)
#define TGA_ERR_COLORMAP_FOR_GRAY       (6)
#define TGA_ERR_BAD_COLORMAP_ENTRY_SIZE (7)
#define TGA_ERR_BAD_COLORMAP            (8)
#define TGA_ERR_READ_FAILS              (9)
#define TGA_ERR_BAD_IMAGE_TYPE          (10)
#define TGA_ERR_BAD_DIMENSIONS          (11)

static uint32 TargaError;

static int16 ttohs(int16 val);
static int32 ttohl(int32 val);
static uint32 tga_get_pixel(FILE* tga, ubyte bytes_per_pix, ubyte* colormap, ubyte cmap_bytes_entry);
static uint32 tga_convert_color(uint32 pixel, uint32 bpp_in, ubyte alphabits, uint32 format_out);
static void tga_write_pixel_to_mem(ubyte* dat, ubyte img_spec, uint32 number, uint32 w, uint32 h, uint32 pixel, uint32 format);

/* returns the last error encountered */
uint32 tga_get_last_error()
{
    return TargaError;
}

/* returns a pointer to the string for an error code */
const char* tga_error_string(int error_code)
{
    switch (error_code)
    {
        case TGA_ERR_NONE: return "no error";
        case TGA_ERR_BAD_HEADER: return "bad image header";
        case TGA_ERR_OPEN_FAILS: return "cannot open file";
        case TGA_ERR_BAD_FORMAT: return "bad format argument";
        case TGA_ERR_UNEXPECTED_EOF: return "unexpected end-of-file";
        case TGA_ERR_NODATA_IMAGE: return "image contains no data";
        case TGA_ERR_COLORMAP_FOR_GRAY: return "found colormap for a grayscale image";
        case TGA_ERR_BAD_COLORMAP_ENTRY_SIZE: return "unsupported colormap entry size";
        case TGA_ERR_BAD_COLORMAP: return "bad colormap";
        case TGA_ERR_READ_FAILS: return "cannot read from file";
        case TGA_ERR_BAD_IMAGE_TYPE: return "unknown image type";
        case TGA_ERR_BAD_DIMENSIONS: return "image has size 0 width or height (or both)";
        default: return "unknown error";
    }
}

/* loads and converts a targa from disk */
void* tga_load(const char* filename, int* width, int* height, unsigned int format)
{
    switch (format)
    {
        case TGA_TRUECOLOR_24:
        case TGA_TRUECOLOR_32:
            break;

        default:
            TargaError = TGA_ERR_BAD_FORMAT;
            return NULL;
    }

    /* open binary image file */
    FILE* targafile = fopen(filename, "rb");
    if (targafile == NULL) {
        TargaError = TGA_ERR_OPEN_FAILS;
        return NULL;
    }

    /* allocate memory for the header */
    ubyte* tga_hdr = (ubyte*) malloc(HDR_LENGTH);

    /* read the header in. */
    if (fread((void*) tga_hdr, 1, HDR_LENGTH, targafile) != HDR_LENGTH)
    {
        free(tga_hdr);
        TargaError = TGA_ERR_BAD_HEADER;
        return NULL;
    }

    /* byte order is important here. */

    // length of the image_id string below.
    ubyte idlen = tga_hdr[HDR_IDLEN];

    // can be any of the IMG_TYPE constants above.
    ubyte image_type = tga_hdr[HDR_IMAGE_TYPE];

    // paletted image <=> cmap_type.
    ubyte cmap_type = tga_hdr[HDR_CMAP_TYPE];

    uint16 cmap_first = ttohs(*(uint16*) (&tga_hdr[HDR_CMAP_FIRST]));

    // how long the colormap is.
    uint16 cmap_length = ttohs(*(uint16*) (&tga_hdr[HDR_CMAP_LENGTH]));

    // how big a palette entry is.
    ubyte cmap_entry_size = tga_hdr[HDR_CMAP_ENTRY_SIZE];

    // the width of the image.
    uint16 img_spec_width = ttohs(*(uint16*) (&tga_hdr[HDR_IMG_SPEC_WIDTH]));

    // the height of the image.
    uint16 img_spec_height = ttohs(*(uint16*) (&tga_hdr[HDR_IMG_SPEC_HEIGHT]));

    // the depth of a pixel in the image.
    ubyte img_spec_pix_depth = tga_hdr[HDR_IMG_SPEC_PIX_DEPTH];

    // the image descriptor.
    ubyte img_spec_img_desc = tga_hdr[HDR_IMG_SPEC_IMG_DESC];

    free(tga_hdr);

    uint32 num_pixels = img_spec_width * img_spec_height;
    if (num_pixels == 0) {
        TargaError = TGA_ERR_BAD_DIMENSIONS;
        return NULL;
    }

    ubyte alphabits = img_spec_img_desc & 0x0F;

    /* seek past the image id, if there is one */
    if (idlen)
    {
        if (fseek( targafile, idlen, SEEK_CUR))
        {
            TargaError = TGA_ERR_UNEXPECTED_EOF;
            return NULL;
        }
    }

    /* if this is a 'nodata' image, just jump out. */
    if (image_type == TGA_IMG_NODATA)
    {
        TargaError = TGA_ERR_NODATA_IMAGE;
        return NULL;
    }

    /* now we're starting to get into the meat of the matter. */

    /* deal with the colormap, if there is one. */
    ubyte cmap_bytes_entry = 0;
    ubyte* colormap = NULL;
    if (cmap_type)
    {
        switch (image_type)
        {
            case TGA_IMG_UNC_PALETTED:
            case TGA_IMG_RLE_PALETTED:
                break;

            case TGA_IMG_UNC_TRUECOLOR:
            case TGA_IMG_RLE_TRUECOLOR:
                // this should really be an error, but some really old
                // crusty targas might actually be like this (created by TrueVision, no less!)
                // so, we'll hack our way through it.
                break;

            case TGA_IMG_UNC_GRAYSCALE:
            case TGA_IMG_RLE_GRAYSCALE:
                TargaError = TGA_ERR_COLORMAP_FOR_GRAY;
                return NULL;
        }

        /* ensure colormap entry size is something we support */
        if (!(cmap_entry_size == 15 ||
                cmap_entry_size == 16 ||
                cmap_entry_size == 24 ||
                cmap_entry_size == 32))
        {
            TargaError = TGA_ERR_BAD_COLORMAP_ENTRY_SIZE;
            return NULL;
        }

        /* allocate memory for a colormap */
        if (cmap_entry_size & 0x07)
        {
            cmap_bytes_entry = (((8 - (cmap_entry_size & 0x07)) + cmap_entry_size) >> 3);
        }
        else
        {
            cmap_bytes_entry = (cmap_entry_size >> 3);
        }

        uint32 cmap_bytes = cmap_bytes_entry*  cmap_length;
        colormap = (ubyte*) malloc(cmap_bytes);

        for (int i = 0; i < cmap_length; i++)
        {
            /* seek ahead to first entry used */
            if (cmap_first != 0)
            {
                fseek(targafile, cmap_first*  cmap_bytes_entry, SEEK_CUR);
            }

            uint32 tmp_int32 = 0;
            for (int j = 0; j < cmap_bytes_entry; j++)
            {
                ubyte tmp_byte = 0;
                if (!fread(&tmp_byte, 1, 1, targafile))
                {
                    free(colormap);
                    TargaError = TGA_ERR_BAD_COLORMAP;
                    return NULL;
                }
                tmp_int32 += tmp_byte << (j * 8);
            }

            // byte order correct.
            tmp_int32 = ttohl(tmp_int32);

            for (int j = 0; j < cmap_bytes_entry; j++)
            {
                colormap[i * cmap_bytes_entry + j] = (tmp_int32 >> (8 * j)) & 0xFF;
            }
        }
    }

    // compute number of bytes in an image data unit (either index or BGR triple)
    ubyte bytes_per_pix;
    if (img_spec_pix_depth & 0x07)
    {
        bytes_per_pix = (((8 - (img_spec_pix_depth & 0x07)) + img_spec_pix_depth) >> 3);
    }
    else
    {
        bytes_per_pix = (img_spec_pix_depth >> 3);
    }

    /* assume that there's one byte per pixel */
    if (bytes_per_pix == 0)
    {
        bytes_per_pix = 1;
    }

    /* compute how many bytes of storage we need for the image */
    uint32 bytes_total = img_spec_width * img_spec_height * format;

    ubyte* image_data = (ubyte*) malloc(bytes_total);

    /* compute the true number of bits per pixel */
    ubyte true_bits_per_pixel = cmap_type ? cmap_entry_size : img_spec_pix_depth;

    switch (image_type)
    {
        case TGA_IMG_UNC_TRUECOLOR:
        case TGA_IMG_UNC_GRAYSCALE:
        case TGA_IMG_UNC_PALETTED:

            /* FIXME: support grayscale */

            for (int i = 0; i < num_pixels; i++)
            {
                // get the color value.
                uint32 tmp_col;
                tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap, cmap_bytes_entry);
                tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel, alphabits, format);

                // now write the data out.
                tga_write_pixel_to_mem(image_data, img_spec_img_desc,
                        i, img_spec_width, img_spec_height, tmp_col, format);
            }
            break;

        case TGA_IMG_RLE_TRUECOLOR:
        case TGA_IMG_RLE_GRAYSCALE:
        case TGA_IMG_RLE_PALETTED:

            // FIXME: handle grayscale..

            for (int i = 0; i < num_pixels;)
            {

                /* a bit of work to do to read the data.. */
                ubyte packet_header = 0;
                if (fread(&packet_header, 1, 1, targafile) < 1)
                {
                    // well, just let them fill the rest with null pixels then...
                    packet_header = 1;
                }

                ubyte repcount;
                if (packet_header & 0x80)
                {
                    /* run length packet */

                    uint32 tmp_col;
                    tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap, cmap_bytes_entry);
                    tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel, alphabits, format);

                    repcount = (packet_header & 0x7F) + 1;

                    /* write all the data out */
                    for (int j = 0; j < repcount; j++)
                    {
                        tga_write_pixel_to_mem(image_data, img_spec_img_desc,
                                i + j, img_spec_width, img_spec_height, tmp_col, format);
                    }

                    i += repcount;

                } else {
                    /* raw packet */
                    /* get pixel from file */

                    repcount = (packet_header & 0x7F) + 1;

                    for (int j = 0; j < repcount; j++)
                    {
                        uint32 tmp_col;
                        tmp_col = tga_get_pixel(targafile, bytes_per_pix, colormap, cmap_bytes_entry);
                        tmp_col = tga_convert_color(tmp_col, true_bits_per_pixel, alphabits, format);

                        tga_write_pixel_to_mem(image_data, img_spec_img_desc,
                                i + j, img_spec_width, img_spec_height, tmp_col, format);
                    }

                    i += repcount;
                }
            }

            break;

        default:
            TargaError = TGA_ERR_BAD_IMAGE_TYPE;
            return NULL;
    }

    fclose(targafile);

    *width = img_spec_width;
    *height = img_spec_height;

    return (void*) image_data;
}

static void tga_write_pixel_to_mem(ubyte* dat, ubyte img_spec, uint32 number,
        uint32 w, uint32 h, uint32 pixel, uint32 format)
{
    // write the pixel to the data regarding how the
    // header says the data is ordered.

    uint32 j;
    uint32 x, y;
    uint32 addy;

    switch ((img_spec & 0x30) >> 4)
    {
        case TGA_LOWER_RIGHT:
            x = w - 1 - (number % w);
            y = number / h;
            break;

        case TGA_UPPER_LEFT:
            x = number % w;
            y = h - 1 - (number / w);
            break;

        case TGA_UPPER_RIGHT:
            x = w - 1 - (number % w);
            y = h - 1 - (number / w);
            break;

        case TGA_LOWER_LEFT:
        default:
            x = number % w;
            y = number / w;
    }

    addy = (y * w + x) * format;
    for (j = 0; j < format; j++)
    {
        dat[addy + j] = (ubyte) ((pixel >> (j * 8)) & 0xFF);
    }
}

static uint32 tga_get_pixel(FILE* tga, ubyte bytes_per_pix, ubyte* colormap, ubyte cmap_bytes_entry)
{
    /* get the image data value out */

    uint32 tmp_int32 = 0;
    for (uint32 j = 0; j < bytes_per_pix; j++)
    {
        ubyte tmp_byte;
        if (fread(&tmp_byte, 1, 1, tga) < 1)
        {
            tmp_int32 = 0;
        }
        else
        {
            tmp_int32 += tmp_byte << (j * 8);
        }
    }

    /* byte-order correct the thing */
    switch (bytes_per_pix)
    {
        case 2:
            tmp_int32 = ttohs((uint16) tmp_int32);
            break;
        case 3:
        case 4:
            tmp_int32 = ttohl(tmp_int32);
            break;
    }

    uint32 tmp_col;
    if (colormap != NULL)
    {
        /* need to look up value to get real color */
        tmp_col = 0;
        for (uint32 j = 0; j < cmap_bytes_entry; j++)
        {
            tmp_col += colormap[cmap_bytes_entry * tmp_int32 + j] << (8 * j);
        }
    }
    else
    {
        tmp_col = tmp_int32;
    }

    return tmp_col;
}

static uint32 tga_convert_color(uint32 pixel, uint32 bpp_in, ubyte alphabits, uint32 format_out)
{
    // this is not only responsible for converting from different depths
    // to other depths, it also switches BGR to RGB.

    // this thing will also premultiply alpha, on a pixel by pixel basis.

    ubyte r, g, b, a;

    switch (bpp_in)
    {
        case 32:
            if (alphabits == 0)
            {
                goto is_24_bit_in_disguise;
            }
            // 32-bit to 32-bit -- nop.
            break;

        case 24:
            is_24_bit_in_disguise:
                    // 24-bit to 32-bit; (only force alpha to full)
                    pixel |= 0xFF000000;
            break;

        case 15:
            is_15_bit_in_disguise:
                    r = (ubyte)(((float)((pixel & 0x7C00) >> 10)) * 8.2258f);
            g = (ubyte)(((float)((pixel & 0x03E0) >> 5 )) * 8.2258f);
            b = (ubyte)(((float)(pixel & 0x001F)) * 8.2258f);
            // 15-bit to 32-bit; (force alpha to full)
            pixel = 0xFF000000 + (r << 16) + (g << 8) + b;
            break;

        case 16:
            if (alphabits == 1)
            {
                goto is_15_bit_in_disguise;
            }
            // 16-bit to 32-bit; (force alpha to full)
            r = (ubyte) (((float) ((pixel & 0xF800) >> 11)) * 8.2258f);
            g = (ubyte) (((float) ((pixel & 0x07E0) >> 5 )) * 4.0476f);
            b = (ubyte) (((float)  (pixel & 0x001F)) * 8.2258f);
            pixel = 0xFF000000 + (r << 16) + (g << 8) + b;
            break;
    }

    // convert the 32-bit pixel from BGR to RGB.
    pixel = (pixel & 0xFF00FF00) + ((pixel & 0xFF) << 16) + ((pixel & 0xFF0000) >> 16);

    r =  pixel & 0x000000FF;
    g = (pixel & 0x0000FF00) >> 8;
    b = (pixel & 0x00FF0000) >> 16;
    a = (pixel & 0xFF000000) >> 24;

    // not premultiplied alpha -- multiply.
    r = (ubyte) (((float) r / 255.0f) * ((float) a / 255.0f) * 255.0f);
    g = (ubyte) (((float) g / 255.0f) * ((float) a / 255.0f) * 255.0f);
    b = (ubyte) (((float) b / 255.0f) * ((float) a / 255.0f) * 255.0f);

    pixel = r + (g << 8) + (b << 16) + (a << 24);

    /* now convert from 32-bit to whatever they want. */

    switch (format_out)
    {
        case TGA_TRUECOLOR_32:
            // 32 to 32 -- nop.
            break;

        case TGA_TRUECOLOR_24:
            // 32 to 24 -- discard alpha.
            pixel &= 0x00FFFFFF;
            break;
    }

    return pixel;
}

static int16 ttohs(int16 val)
{
#ifdef WORDS_BIGENDIAN
    return ((val & 0xFF) << 8) + (val >> 8);
#else
    return val;
#endif
}

static int32 ttohl(int32 val)
{
#ifdef WORDS_BIGENDIAN
    return ((val & 0x000000FF) << 24) +
           ((val & 0x0000FF00) << 8)  +
           ((val & 0x00FF0000) >> 8)  +
           ((val & 0xFF000000) >> 24);
#else
    return val;
#endif
}
