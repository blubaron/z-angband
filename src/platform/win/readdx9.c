/* File: readdib.c */

/*
 * This package provides a routine to read a DIB file and set up the
 * device dependent version of the image.
 *
 * This file has been modified for use with "Angband 2.9.2"
 * This file has been modified for use with "z+Angband 0.3.3"
 * This file has been copied, renamed, and changed to use DirectX 9 by Brett Reid
 *
 * COPYRIGHT:
 *
 *   (C) Copyright Microsoft Corp. 1993.  All rights reserved.
 *
 *   You have a royalty-free right to use, modify, reproduce and
 *   distribute the Sample Files (and/or any modified version) in
 *   any way you find useful, provided that you agree that
 *   Microsoft has no warranty obligations or liability for any
 *   Sample Application Files which are modified.
 */

#include <windows.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "readdib.h"


/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
#  define WIN32
# endif
#endif

/*
 * Imports a DIB from a file that can be loaded by DirectX. Once
 * the DIB is loaded, the function also creates a bitmap
 * and palette out of the DIB for a device-dependent form.
 *
 * Returns TRUE if the DIB is loaded and the bitmap/palette created, in which
 * case, the DIBINIT structure pointed to by pInfo is filled with the appropriate
 * handles, and FALSE if something went wrong.
 */

BOOL ReadDIB_DX9(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo)
{
  IDirect3D9 *pD3D;
  IDirect3DDevice9 *pD3DDevice;
  D3DXIMAGE_INFO info;
  LPDIRECT3DSURFACE9 psurf = NULL;
  D3DLOCKED_RECT surf_data;
  HRESULT result;
  BOOL noerror = TRUE;

  HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
  BITMAPINFO bi;
	HDC hDC;

  // Create the D3D object
  if((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
  {
    //plog_fmt("Unable to initialize D3D");
    return (FALSE);
  }


  // create the D3D Device
  if (noerror)
  {
    D3DPRESENT_PARAMETERS present_params;
    memset(&(present_params), 0, sizeof(D3DPRESENT_PARAMETERS));
    present_params.Windowed = TRUE;
    present_params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    present_params.hDeviceWindow = hWnd;
    present_params.BackBufferWidth = 1;
    present_params.BackBufferHeight = 1;
    present_params.BackBufferFormat = D3DFMT_X8R8G8B8;
    present_params.BackBufferCount = 2;
    present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;

    result = IDirect3D9_CreateDevice(pD3D,D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                              D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                              &present_params, &pD3DDevice);
    if (FAILED(result))
    {
			//plog_fmt("Failed to create D3D device");
      noerror = FALSE;
    }
  }

  // get the image size
  if (noerror)
  {
    result = D3DXGetImageInfoFromFile(lpFileName, &info);
    if (FAILED(result))
    {
			//plog_fmt("Failed to load image info");
      noerror = FALSE;
    }
  }
  // create the surface that the image will be loaded into
  if (noerror)
  {
    result = IDirect3DDevice9_CreateOffscreenPlainSurface(pD3DDevice,
      info.Width, info.Height, D3DFMT_R8G8B8, D3DPOOL_SCRATCH, &psurf, NULL);
    if (FAILED(result))
    {
			//plog_fmt("Failed to create D3D surface");
      noerror = FALSE;
    }
  }
  // load the image
  if (noerror)
  {
    result = D3DXLoadSurfaceFromFile(psurf, NULL, NULL, lpFileName,
      NULL, D3DX_FILTER_NONE, 0, &info);
    if (FAILED(result))
    {
			//plog_fmt("Failed to load image");
      noerror = FALSE;
    }
  }
  // create the DIB
  if (noerror)
  {
    bi.bmiHeader.biWidth = (LONG)info.Width;
    bi.bmiHeader.biHeight = -((LONG)info.Height);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biSize = 40; // the size of the structure
    bi.bmiHeader.biXPelsPerMeter = 3424; // just a number I saw in the header of a file
    bi.bmiHeader.biYPelsPerMeter = 3424; // just a number I saw in the header of a file
    bi.bmiHeader.biSizeImage = info.Width*info.Height*3;
	  hDC = GetDC(hWnd);

    hPalette = GetStockObject(DEFAULT_PALETTE);
    // Need to realize palette for converting DIB to bitmap. 
	  hOldPal = SelectPalette(hDC, hPalette, TRUE);
	  RealizePalette(hDC);
  }  
  // lock the surface and copy the data to the DIB
  if (noerror)
  {
    result = IDirect3DSurface9_LockRect(psurf,&surf_data, NULL, D3DLOCK_READONLY);
    if (FAILED(result))
    {
			//plog_fmt("Failed to lock D3D surface");
      noerror = FALSE;
		  SelectPalette(hDC, hOldPal, TRUE);
		  RealizePalette(hDC);
    }
  }
  if (noerror)
  {
		hBitmap = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, surf_data.pBits,
		                         &bi, DIB_RGB_COLORS);

		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		if (!hBitmap)
		{
			DeleteObject(hPalette);
      noerror = FALSE;
		}
		else
		{
			pInfo->hBitmap = hBitmap;
			pInfo->hPalette = hPalette;
      pInfo->hDIB = 0;
		}
  }
  // release all the the DirectX Structures
  if (psurf) {
    IDirect3DSurface9_UnlockRect(psurf);
    IDirect3DSurface9_Release(psurf);
    psurf = NULL;
  }
  if (pD3DDevice) {
    IDirect3DDevice9_Release(pD3DDevice);
    pD3DDevice = NULL;
  }
  if (pD3D) {
    IDirect3D9_Release(pD3D);
    pD3D = NULL;
  }
  
  ReleaseDC(hWnd,hDC);

  if (!noerror)
  {
    return (FALSE);
  }
  return (TRUE);
}

BOOL ReadDIB2_DX9(HWND hWnd, LPSTR lpFileName, DIBINIT *pInfo, DIBINIT *pMask)
{
  IDirect3D9 *pD3D;
  IDirect3DDevice9 *pD3DDevice;
  D3DXIMAGE_INFO info;
  LPDIRECT3DSURFACE9 psurf = NULL;
  D3DLOCKED_RECT surf_data;
  HRESULT result;
  BOOL noerror = TRUE;

  HBITMAP hBitmap;
	HPALETTE hPalette, hOldPal;
  BITMAPINFO bi;
	HDC hDC;

  // Create the D3D object
  if((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
  {
    //plog_fmt("Unable to initialize D3D");
    return (FALSE);
  }


  // create the D3D Device
  if (noerror)
  {
    D3DPRESENT_PARAMETERS present_params;
    memset(&(present_params), 0, sizeof(D3DPRESENT_PARAMETERS));
    present_params.Windowed = TRUE;
    present_params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    present_params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    present_params.hDeviceWindow = hWnd;
    present_params.BackBufferWidth = 1;
    present_params.BackBufferHeight = 1;
    present_params.BackBufferFormat = D3DFMT_X8R8G8B8;
    present_params.BackBufferCount = 2;
    present_params.SwapEffect = D3DSWAPEFFECT_DISCARD;

    result = IDirect3D9_CreateDevice(pD3D,D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
                              D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                              &present_params, &pD3DDevice);
    if (FAILED(result))
    {
			//plog_fmt("Failed to create D3D device");
      noerror = FALSE;
    }
  }

  // get the image size
  if (noerror)
  {
    result = D3DXGetImageInfoFromFile(lpFileName, &info);
    if (FAILED(result))
    {
			//plog_fmt("Failed to load image info");
      noerror = FALSE;
    }
  }
  // create the surface that the image will be loaded into
  if (noerror)
  {
    if ((info.Format == D3DFMT_A8R8G8B8) || (info.Format == D3DFMT_P8)) {
      result = IDirect3DDevice9_CreateOffscreenPlainSurface(pD3DDevice,
        info.Width, info.Height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &psurf, NULL);
    }
    else
    {
      result = IDirect3DDevice9_CreateOffscreenPlainSurface(pD3DDevice,
        info.Width, info.Height, D3DFMT_R8G8B8, D3DPOOL_SCRATCH, &psurf, NULL);
    }
    if (FAILED(result))
    {
			//plog_fmt("Failed to create D3D surface");
      noerror = FALSE;
    }
  }
  // load the image
  if (noerror)
  {
    result = D3DXLoadSurfaceFromFile(psurf, NULL, NULL, lpFileName,
      NULL, D3DX_FILTER_NONE, 0, &info);
    if (FAILED(result))
    {
			//plog_fmt("Failed to load image");
      noerror = FALSE;
    }
  }
  // create the DIB
  if (noerror)
  {
    bi.bmiHeader.biWidth = (LONG)info.Width;
    bi.bmiHeader.biHeight = -((LONG)info.Height);
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;
    bi.bmiHeader.biBitCount = 24;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biSize = 40; // the size of the structure
    bi.bmiHeader.biXPelsPerMeter = 3424; // just a number I saw in the header of a file
    bi.bmiHeader.biYPelsPerMeter = 3424; // just a number I saw in the header of a file
    bi.bmiHeader.biSizeImage = info.Width*info.Height*3;
	  hDC = GetDC(hWnd);

    hPalette = GetStockObject(DEFAULT_PALETTE);
    // Need to realize palette for converting DIB to bitmap. 
	  hOldPal = SelectPalette(hDC, hPalette, TRUE);
	  RealizePalette(hDC);
  }  
  // lock the surface and copy the data to the DIB
  if (noerror)
  {
    result = IDirect3DSurface9_LockRect(psurf,&surf_data, NULL, D3DLOCK_READONLY);
    if (FAILED(result))
    {
			//plog_fmt("Failed to lock D3D surface");
      noerror = FALSE;
		  SelectPalette(hDC, hOldPal, TRUE);
		  RealizePalette(hDC);
    }
  }
  if (noerror)
  {
    if ((info.Format == D3DFMT_A8R8G8B8) || (info.Format == D3DFMT_P8)) {
      // only copy 3 of the 4 bytes, last byte will go in the mask below
      BITMAPINFO biSrc;
      biSrc.bmiHeader.biWidth = (LONG)info.Width;
      biSrc.bmiHeader.biHeight = -((LONG)info.Height);
      biSrc.bmiHeader.biPlanes = 1;
      biSrc.bmiHeader.biClrUsed = 0;
      biSrc.bmiHeader.biClrImportant = 0;
      biSrc.bmiHeader.biBitCount = 32;
      biSrc.bmiHeader.biCompression = BI_RGB;
      biSrc.bmiHeader.biPlanes = 1;
      biSrc.bmiHeader.biSize = 40; // the size of the structure
      biSrc.bmiHeader.biXPelsPerMeter = 3424; // just a number I saw in the header of a file
      biSrc.bmiHeader.biYPelsPerMeter = 3424; // just a number I saw in the header of a file
      biSrc.bmiHeader.biSizeImage = info.Width*info.Height*4;
		  hBitmap = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, surf_data.pBits,
		                           &biSrc, DIB_RGB_COLORS);
    } else {
		  hBitmap = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, surf_data.pBits,
		                           &bi, DIB_RGB_COLORS);
    }
		SelectPalette(hDC, hOldPal, TRUE);
		RealizePalette(hDC);
		if (!hBitmap)
		{
			DeleteObject(hPalette);
      noerror = FALSE;
		}
		else
		{
			pInfo->hBitmap = hBitmap;
			pInfo->hPalette = hPalette;
      pInfo->hDIB = 0;
		}
  }

  if (noerror)
  {
    if (pMask && ((info.Format == D3DFMT_A8R8G8B8) || (info.Format == D3DFMT_P8)))
    {
      byte *pBits, v;
      int i,j;
      DWORD *srcrow;
      HBITMAP hBitmap2;
      HPALETTE hPalette2 = GetStockObject(DEFAULT_PALETTE);
      BOOL have_alpha = FALSE;

      /* Need to realize palette for converting DIB to bitmap. */
	    hOldPal = SelectPalette(hDC, hPalette2, TRUE);
	    RealizePalette(hDC);

      /* allocate the storage space */
      pBits = (byte*)malloc(sizeof(byte)*info.Width*info.Height*3);
      if (!pBits)
      {
        noerror = FALSE;
      }
      if (noerror)
      {
        for (j = 0; j < info.Height; ++j) {
          srcrow = (DWORD*)surf_data.pBits + (j*info.Width);
          for (i = 0; i < info.Width; ++i) {
            /* get the alpha byte from the source */
            v = (*((DWORD*)srcrow + i)>>24);
            v = 255 - v;
            if (v==255) 
            {
              /* check if we have a full alpha value */
              have_alpha = TRUE;
            }

            /* write the alpha byte to the three colors of the storage space */
            *(pBits + (j*info.Width*3) + (i*3)) = v;
            *(pBits + (j*info.Width*3) + (i*3)+1) = v;
            *(pBits + (j*info.Width*3) + (i*3)+2) = v;
          }
        }
        /* create the bitmap from the storage space if we have the alpha data */
        if (have_alpha)
        {
          hBitmap2 = CreateDIBitmap(hDC, &(bi.bmiHeader), CBM_INIT, pBits,
                                    &bi, DIB_RGB_COLORS);
        }
        free(pBits);
      }
		  SelectPalette(hDC, hOldPal, TRUE);
		  RealizePalette(hDC);
		  if (!hBitmap2)
		  {
			  DeleteObject(hPalette2);
        noerror = FALSE;
		  }
		  else
		  {
			  pMask->hBitmap = hBitmap2;
			  pMask->hPalette = hPalette2;
        pMask->hDIB = 0;
		  }
    }
  }
  // release all the the DirectX Structures
  if (psurf) {
    IDirect3DSurface9_UnlockRect(psurf);
    IDirect3DSurface9_Release(psurf);
    psurf = NULL;
  }
  if (pD3DDevice) {
    IDirect3DDevice9_Release(pD3DDevice);
    pD3DDevice = NULL;
  }
  if (pD3D) {
    IDirect3D9_Release(pD3D);
    pD3D = NULL;
  }
  
  ReleaseDC(hWnd,hDC);

  if (!noerror)
  {
    return (FALSE);
  }
  return (TRUE);
}

