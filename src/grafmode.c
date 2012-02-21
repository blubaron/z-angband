/*
 * File: grafmode.c
 * Purpose: load a list of possible graphics modes.
 *
 * Copyright (c) 2011 Brett Reid
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

#include "angband.h"
#include "grafmode.h"
//#include "parser.h"

graphics_mode *graphics_modes;
graphics_mode *current_graphics_mode = NULL;
int graphics_mode_high_id;


bool init_graphics_modes(const char *filename) {
  int i = 0;
  int line = 0;
  int previd;
	char *zz[16];
	char buf[1024];
  FILE *fp;
  graphics_mode *mode,*next;

  /* Build the filename */
	path_make(buf, ANGBAND_DIR_XTRA_GRAF, filename);

  fp = my_fopen(buf, "r");
  if (!fp) {
    return FALSE;
  }
	while (0 == my_fgets(fp, buf, 1024))
	{
		/* Count lines */
		line++;


		/* Skip "empty" lines */
		if (!buf[0]) continue;

		/* Skip "blank" lines */
		if (isspace(buf[0])) continue;

		/* Skip comments */
		if (buf[0] == '#') continue;

    
  	/* Require "?:*" format */
	  if (buf[1] != ':') return FALSE;//(1);

    /* process the line */
	  /* Process "N:<id>:<menuname>" -- start a new record */
	  if (buf[0] == 'N') {
		  if (tokenize(buf + 2, 2, zz, TOKENIZE_CHECKQUOTE) == 2) {
        mode = ZNEW(graphics_mode);
        if (mode) {
          mode->grafID = strtol(zz[0], NULL, 0);
          strncpy(mode->menuname, zz[1], 32);
          mode->menuname[31] = 0;
          if (graphics_mode_high_id < mode->grafID) {
            graphics_mode_high_id = mode->grafID;
          }
          mode->pNext = graphics_modes;
          graphics_modes = mode;
        } else {
			    return FALSE;//(PARSE_ERROR_OUT_OF_MEMORY);
        }
      } else {
        return FALSE;//(PARSE_ERROR_GENERIC);
      }

    }
	  /* Process "P:<prefname>:<preffile>" -- set the name used with pref files */
	  else if (buf[0] == 'P') {
      byte count = tokenize(buf + 2, 2, zz, TOKENIZE_CHECKQUOTE);
		  if (count >= 1) {
        if (mode) {
          strncpy(mode->pref, zz[0], 32);
          if (count == 2) {
            /* overwrite the pref name with the pref filename */
            strncpy(mode->pref, zz[1], 32);
          }
          mode->pref[31] = 0;
        } else {
      		return FALSE;//(PARSE_ERROR_MISSING_RECORD_HEADER);
        }
      } else {
        return FALSE;//(PARSE_ERROR_GENERIC);
      }
    }
	  /* Process "I:<cell width>:<cell height>:<filename>" -- read the tileset info */
	  else if (buf[0] == 'I') {
		  if (tokenize(buf + 2, 3, zz, TOKENIZE_CHECKQUOTE) == 3) {
        if (mode) {
          mode->cell_width = strtol(zz[0], NULL, 0);
          mode->cell_height = strtol(zz[1], NULL, 0);
          strncpy(mode->file, zz[2], 32);
          mode->file[31] = 0;
        } else {
      		return FALSE;//(PARSE_ERROR_MISSING_RECORD_HEADER);
        }
      } else {
        return FALSE;//(PARSE_ERROR_GENERIC);
      }
    }
	  /* Process "X:<alpha>:<overdraw_min>:<overdraw_max>" -- read some extra info */
	  else if (buf[0] == 'X') {
		  if (tokenize(buf + 2, 3, zz, TOKENIZE_CHECKQUOTE) == 3) {
        if (mode) {
          mode->alphablend = strtol(zz[0], NULL, 0);
          mode->overdrawRow = strtol(zz[1], NULL, 0);
          mode->overdrawMax = strtol(zz[2], NULL, 0);
        } else {
      		return FALSE;//(PARSE_ERROR_MISSING_RECORD_HEADER);
        }
      } else {
        return FALSE;//(PARSE_ERROR_GENERIC);
      }
    }
	  else {
		  /* Oops */
		  return FALSE;//(PARSE_ERROR_UNDEFINED_DIRECTIVE);
	  }
	}

  mode = graphics_modes;
  graphics_modes = C_RNEW(graphics_mode_high_id+1,graphics_mode);

  /* set the default mode */
  graphics_modes[0].grafID = 0;
  graphics_modes[0].alphablend = 0;
  graphics_modes[0].cell_height = 1;
  graphics_modes[0].cell_width = 1;
  graphics_modes[0].overdrawRow = 0;
  graphics_modes[0].overdrawMax = 0;
  strncpy(graphics_modes[0].menuname, "None", 32);
  strncpy(graphics_modes[0].pref, "font.prf", 32);
  strncpy(graphics_modes[0].file, "", 32);
  if (mode) {
    next = mode;
    while (next->pNext) next = next->pNext;
    graphics_modes[0].pNext = &(graphics_modes[next->grafID]);
  } else {
    graphics_modes[0].pNext = NULL;
  }
  previd = 0;
  while (mode) {
    next = mode->pNext;
    i = mode->grafID;
    graphics_modes[i].grafID = mode->grafID;
    graphics_modes[i].cell_height = mode->cell_height;
    graphics_modes[i].cell_width = mode->cell_width;
    graphics_modes[i].alphablend = mode->alphablend;
    graphics_modes[i].overdrawRow = mode->overdrawRow;
    graphics_modes[i].overdrawMax = mode->alphablend;
    strncpy(graphics_modes[i].menuname, mode->menuname, 32);
    strncpy(graphics_modes[i].pref, mode->pref, 32);
    strncpy(graphics_modes[i].file, mode->file, 32);
    if (previd) {
      graphics_modes[i].pNext = &(graphics_modes[previd]);
    } else {
      graphics_modes[i].pNext = NULL;
    }
    previd = i;
    FREE(mode);
    mode = next;
  }
  return TRUE;
}

void close_graphics_modes(void) {
	if (graphics_modes) {
		FREE(graphics_modes);
		graphics_modes = NULL;
	}
}

graphics_mode* get_graphics_mode(byte id) {
  if ((id <= graphics_mode_high_id) && (id >= 0)
    && graphics_modes[id].file[0]) {
    return &(graphics_modes[id]);
  }
	return NULL;
}
