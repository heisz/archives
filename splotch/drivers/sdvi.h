/**
 * Common definitions of the sPLOTch! device independent (sdvi) data format.
 * Copyright (C) 1991-2005 J.M. Heisz 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * See the file named COPYRIGHT in the root directory of the source
 * distribution for specific references to the GNU General Public License,
 * as well as further clarification on your rights to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef SDVI_H
#define SDVI_H 1

/* Leading integer/magic number to identify sdvi contents */
#define SDVI_MAGIC 745252188

/* Command definitions for sdvi operations */
#define OPEN_SCR  10
#define CLOSE_SCR 11
#define UPD_SCR   12
#define MOVE_PT   20
#define DRAW_PT   21
#define CH_WDTH   30
#define CH_COL    31
#define FILL_P    32
#define DIAGRAM   40
#define END_OF_FILE 100

/*
 * Read an 4-byte coordinate value from the provided input stream.
 */
extern COORD read_coord(FILE *fp);

/*
 * Read an 4-byte floating point value from the input stream.
 */
extern float read_float(FILE *fp);


#endif
