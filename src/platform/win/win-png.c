/*
 * File: win-png.c
 * Purpose: load and save png files in x11, using libpng.
 *
 * Copyright (c) 2012 Brett Reid
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband license":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#include "z-config.h"
#include "h-basic.h"
#include "z-file.h"

#ifdef USE_GRAPHICS

#include "png.h"

/* file io routines for the png library */
static void png_read_ang_file(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ang_file *fff = (ang_file*) png_get_io_ptr(png_ptr);
	(void) file_read(fff, (char*)data, length);
}
static void png_write_ang_file(png_structp png_ptr, png_bytep data, png_size_t length)
{
	ang_file *fff = (ang_file*) png_get_io_ptr(png_ptr);
	(void) file_write(fff, (const char*)data, length);
}
static void png_write_flush_ang_file(png_structp png_ptr)
{
	ang_file *fff = (ang_file*) png_get_io_ptr(png_ptr);
	(void)file_flush(fff);
}

int LoadPNG(Display *dpy, char *filename, XImage **ret_color, XImage **ret_mask,
	int *ret_width, int *ret_height, bool premultiply)
{
	png_structp png_ptr;
	png_infop info_ptr;
	byte header[8];
	png_bytep *row_pointers;
	png_byte *color_data;
	bool noerror = TRUE;

	XImage *color;

	png_byte color_type;
	png_byte bit_depth;
	int width, height;
	int y, number_of_passes;
	int row_bytes;

	bool update = FALSE;

	Visual *visual = DefaultVisual(dpy, DefaultScreen(dpy));
	int scr_depth = DefaultDepth(dpy, DefaultScreen(dpy));
	
	/* open the file and test it for being a png */
	ang_file *fp = file_open(filename, MODE_READ, FTYPE_RAW);
	if (!fp)
	{
		/*plog_fmt("Unable to open PNG file.");*/
		return -4;
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) {
		plog_fmt("Unable to open PNG file - not a PNG file. %s", filename);
		file_close(fp);
		return -2;
	}
	
	/* Create the png structure */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png_ptr)
	{
		plog("Unable to initialize PNG library");
		file_close(fp);
		return -3;
	}
	
	/* create the info structure */
	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		plog("Failed to create PNG info structure.");
		return -3;
	}
	
	/* setup error handling for init */
	png_set_read_fn(png_ptr, fp, png_read_ang_file);
	png_set_sig_bytes(png_ptr, 8);
	
	png_read_info(png_ptr, info_ptr);
	
	width = png_get_image_width(png_ptr, info_ptr);
	height = png_get_image_height(png_ptr, info_ptr);
	color_type = png_get_color_type(png_ptr, info_ptr);
	bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	
	number_of_passes = png_set_interlace_handling(png_ptr);
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_palette_to_rgb(png_ptr);
		update = TRUE;
	}

	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png_ptr);
		update = TRUE;
	}

	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
		update = TRUE;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY ||
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
		png_set_gray_to_rgb(png_ptr);
		update = TRUE;
	}

	if (update) {
		png_read_update_info(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		update = FALSE;
	}

	if (color_type == PNG_COLOR_TYPE_RGB) {
		png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
		/* add the alpha channel, without updating the color type,
		 * so we can use the color type when we process the mask */
		/*update = TRUE;*/
	}

	/*if (update) {
		png_read_update_info(png_ptr, info_ptr);
		color_type = png_get_color_type(png_ptr, info_ptr);
		bit_depth = png_get_bit_depth(png_ptr, info_ptr);
		update = FALSE;
	}*/

	png_set_bgr(png_ptr);
	/* after these requests, the data should always be RGB or ARGB */

	/* initialize row_pointers */
	row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	color_data = (png_byte*) malloc(sizeof(png_byte) * height * row_bytes);
	if (!color_data) {
		/* release all the the PNG Structures */
		if (info_ptr) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			info_ptr = NULL;
			png_ptr = NULL;
		}
		else if (png_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			png_ptr = NULL;
		}
		return -3;
	}
	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers) {
		free(color_data);
		/* release all the the PNG Structures */
		if (info_ptr) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			info_ptr = NULL;
			png_ptr = NULL;
		}
		else if (png_ptr) {
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			png_ptr = NULL;
		}
		return -3;
	}
	for (y = 0; y < height; ++y) {
		row_pointers[y] = color_data + row_bytes*y;
	}

	/* read the image data into row_pointers */
	png_read_image(png_ptr, row_pointers);

	/* we are done with the file pointer, so close it */
	file_close(fp);

	/* release the row_pointer memory */
	if (row_pointers) {
		free(row_pointers);
	}
	/* release all the the PNG Structures */
	if (info_ptr) {
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		info_ptr = NULL;
		png_ptr = NULL;
	}
	else if (png_ptr) {
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		png_ptr = NULL;
	}

	/* premultiply the alpha data if we need to */
	/*if (premultiply && (color_type == PNG_COLOR_TYPE_RGBA)) {
		int x;
		u32b *srcrow;
		for (y = 0; y < height; ++y) {
			srcrow = (u32b*) (color_data + row_bytes*y);
			for (x = 0; x < width; ++x) {
			}
		}
	}*/

	/* create the Image structure */
	color = XCreateImage(dpy, visual, scr_depth, ZPixmap, 0,
			(char*)color_data, width, height, 32, 0);
	if (!color) {
		if (color_data)
			free(color_data);
		noerror = FALSE;
	} else {
		if (ret_color)
			*ret_color = color;
		if (ret_width)
			*ret_width = width;
		if (ret_height)
			*ret_height = height;
	}
	if (noerror && ret_mask && (color_type == PNG_COLOR_TYPE_RGB_ALPHA)) {
		byte *pBits, v;
		int x, w;
		u32b *srcrow;
		png_byte *mask_data;
		XImage *mask = NULL;
		bool have_alpha = FALSE;
		
		/* allocate the storage space */
		w = width*4;
		mask_data = (png_byte*)malloc(sizeof(png_byte)*height*w);
		if (!mask_data) {
			noerror = FALSE;
		}

		if (noerror) {
			for (y = 0; y < height; ++y) {
				srcrow = (u32b*) (color_data + row_bytes*y);
				for (x = 0; x < width; ++x) {
					/* get the alpha byte from the source */
					v = (*(srcrow + x)>>24);
					v = 255 - v;
					if (v==255) 
					{
						have_alpha = TRUE;
					}
					/* write the alpha byte to the three colors of the storage space */
					*(mask_data + (y*w) + (x*4)) = v;
					*(mask_data + (y*w) + (x*4)+1) = v;
					*(mask_data + (y*w) + (x*4)+2) = v;
					*(mask_data + (y*w) + (x*4)+3) = 255;

				}
			}
			/* create the bitmap from the storage space */
			if (have_alpha)
			{
				mask = XCreateImage(dpy, visual, scr_depth, ZPixmap, 0,
					(char*)mask_data, width, height, 32, 0);
			}
		}
		if (!mask) {
			if (mask_data)
				free(mask_data);
			noerror = FALSE;
		} else {
			if (ret_mask)
				*ret_mask = mask;
		}
	}

	if ((color_type == PNG_COLOR_TYPE_RGB_ALPHA) && noerror) {
		return 1;
	}
	return 0;
}
int SavePNG(Display *dpy, char *filename, XImage *color, XImage *mask,
	int wid, int hgt, bool unpremultiply)
{
	return -1;
#if 0
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;

	HRESULT result;
  BOOL noerror = TRUE;
  BOOL masksaved = FALSE;

  HBITMAP hbmOld, hbmOldMask;
	HDC dcColor, dcMask;

  png_byte color_type;
  png_byte bit_depth;
  png_byte channels;

  int x,y;
  {
    int i,j;
    COLORREF c,a;
    HBITMAP hbmOld;
	  HDC dcColor;

	  dcColor = CreateCompatibleDC(NULL);
    hbmOld = (HBITMAP)SelectObject(dcColor, color);
    for (j = 0; j < height; ++j) {
      for (i = 0; i < width; ++i) {
        c = GetPixel(dcColor, i, j);
        if (c != 0)
        c=c;
      }
    }
    SelectObject(dcColor, hbmOld);
    DeleteDC(dcColor);
  }

  if (mask) {
    color_type = PNG_COLOR_TYPE_RGB_ALPHA;
    bit_depth = 8;
    channels = 4;
  } else
  {
    color_type = PNG_COLOR_TYPE_RGB;
    bit_depth = 8;
    channels = 3;
  }

  // open the file and test it for being a png
	ang_file *fp = file_open(filename, MODE_WRITE, FTYPE_RAW);
  if (!fp)
  {
    //plog_fmt("Unable to open PNG file.");
    return E_FAIL;
  }

  // Create the png structure
  png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if(!png_ptr) {
    //plog_fmt("Unable to initialize PNG library");
    file_close(fp);
    return E_FAIL;
  }

  // create the info structure
  if (noerror) {
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
			//plog_fmt("Failed to create PNG info structure.");
      noerror = FALSE;
      result = E_FAIL;
    }
  }

  if (noerror) {
    // setup error handling for init
    png_set_write_fn(png_ptr, fp, png_write_ang_file, png_write_flush_ang_file);

    png_set_IHDR(png_ptr, info_ptr, width, height,
      bit_depth, color_type, PNG_INTERLACE_NONE,
      PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    if (bit_depth < 8) {
      png_set_packing(png_ptr);
    }
    png_write_info(png_ptr, info_ptr);
  }

  // copy the allocate the memory libpng can access
  if (noerror) {
    // setup error handling for read
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep)*height);
    if (!row_pointers)
    {
			//plog_fmt("Failed to alloc temporary memory for PNG data.");
      noerror = FALSE;
      result = E_OUTOFMEMORY;
    }
    if (noerror) {
      for (y = 0; y < height; ++y)
      {
        row_pointers[y] = (png_bytep) malloc(sizeof(png_bytep)*width*channels);
        if (!row_pointers[y])
        {
			    //plog_fmt("Failed to alloc temporary memory for PNG data.");
          noerror = FALSE;
          result = E_OUTOFMEMORY;
          break;
        }
      }
    }
  }

  // copy the data to it
  if (noerror) {
    COLORREF bgr, a;
	  dcColor = CreateCompatibleDC(NULL);
    if (!dcColor) {
    }
    hbmOld = (HBITMAP)SelectObject(dcColor, color);
    
    if (color_type == PNG_COLOR_TYPE_GRAY) {
      // copy just the mask bitmap data
      byte *data;

      if (mask) {
        dcMask = CreateCompatibleDC(NULL);
        if (!dcMask) {
        }
        hbmOldMask = (HBITMAP)SelectObject(dcMask, mask);
        for (y = 0; y < height; ++y) {
          data = row_pointers[y];
          for (x = 0; x < width; ++x) {
            a = GetPixel(dcMask, x,y);
            data[x] = ((a&0x0000FF00) >> 8);
          }
        }
        png_set_packing(png_ptr);
        masksaved = TRUE;

        SelectObject(dcMask, hbmOldMask);
        DeleteDC(dcMask);
      } else {
        for (y = 0; y < height; ++y) {
          data = row_pointers[y];
          for (x = 0; x < width; ++x) {
            a = GetPixel(dcColor, x,y);
            data[x] = ((a&0x0000FF00) >> 8);
          }
        }
        png_set_packing(png_ptr);
      }
    } else
    if (color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
      // copy both sets of data
      DWORD  argb, *data;

      dcMask = CreateCompatibleDC(NULL);
      if (!dcMask) {
      }
      hbmOldMask = (HBITMAP)SelectObject(dcMask, mask);

      for (y = 0; y < height; ++y) {
        data = (DWORD*) row_pointers[y];
        for (x = 0; x < width; ++x) {
          bgr = GetPixel(dcColor, x,y);
          a = GetPixel(dcMask, x,y);
          //argb = ((bgr&0x00FF0000)>>16) | ((bgr&0x0000FF00)) | ((bgr&0x000000FF)<<16) | (0xFF000000 - ((a&0x0000FF00)<<16));
          argb = (bgr&0x00FFFFFF) | (0xFF000000 - ((a&0x0000FF00)<<16));
          *(data+x) = argb;
        }
      }
      masksaved = TRUE;

      SelectObject(dcMask, hbmOldMask);
      DeleteDC(dcMask);
    } else
    {
      // copy just the color data
      byte b[3], *data;
      for (y = 0; y < height; ++y) {
        data = row_pointers[y];
        for (x = 0; x < width; ++x) {
          bgr = GetPixel(dcColor, x,y);
          b[0] = ((bgr&0x000000FF));
          b[1] = ((bgr&0x0000FF00)>>8);
          b[2] = ((bgr&0x00FF0000)>>16);
          *(data++) = b[2];
          *(data++) = b[1];
          *(data++) = b[0];
        }
      }
    }

    SelectObject(dcColor, hbmOld);
    DeleteDC(dcColor);
  }  

  // write the file
  if (noerror)
  {
    //png_set_bgr(png_ptr);
    png_write_image(png_ptr, row_pointers);
    png_write_end(png_ptr, NULL);
  }

  // we are done with the file pointer, so
  // release all the the PNG Structures
  if (info_ptr) {
    png_destroy_write_struct(&png_ptr, &info_ptr);//, (png_infopp)NULL);
    info_ptr = NULL;
    png_ptr = NULL;
  }
  else if (png_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);//, (png_infopp)NULL);
    png_ptr = NULL;
  }
  
  // we are done with the file pointer, so close it
  if (fp) {
    file_close(fp);
    fp = NULL;
  }
  if (row_pointers) {
    for (y = 0; y < height; ++y) {
      free(row_pointers[y]);
    }
    free(row_pointers);
  }

  if (!noerror)
  {
    return result;
  }
  // return S_OK if both were saved, S_FALSE if just color bitmap
  // S_OK also if just mask was saved and that's all we wanted
  if (masksaved) {
    return S_OK;
  } else {
    return S_FALSE;
  }
#endif
}

#endif
