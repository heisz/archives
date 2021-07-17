/**
 * Common routines used throughout the sPLOTch! drivers.
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

/*
 * Obtain one of the RGB leg values based on the end-points and hue setting.
 * For internal use only.
 */
float rgb_val(float n1, float n2, float hue) {
    float val;

    if (hue > 360.0) hue -= 360.0;
    if (hue < 0.0) hue += 360.0;
    if (hue < 60.0) val = n1 + (n2 - n1) * hue / 60.0;
    else if (hue < 180.0) val = n2;
    else if (hue < 240.0) val = n1 + (n2 - n1) * (240.0 - hue) / 60.0;
    else val = n1;

    return val;
}

/*
 * Convert a hue-saturation-brightness colour definition into a
 * red-green-blue format (for standard displays).
 */
void hsb_to_rgb(float hue, float sat, float brt, 
                float *rd, float *gr, float *bl) {
    float m1, m2;

    if (brt <= 0.5) m2 =brt * (1 + sat);
    else m2 = brt + sat - brt * sat;
    m1 = 2.0 * brt - m2;
 
    if (sat == 0.0) {
        *rd = *gr = *bl = brt;    /* grey scale */
    }else{
        *rd = rgb_val(m1, m2, (hue + 120.0));
        *gr = rgb_val(m1, m2, hue);
        *bl = rgb_val(m1, m2, (hue - 120.0));
    }
}

/*
 * Obtain the grey-scale value for a particular red-green-blue combination.
 * Assumes "standard" colour saturation values.
 */
float grey_scale(float rd, float gr, float bl) {
   return ((float) (0.299 * rd + 0.587 * gr + 0.114 * bl));
}

/*
 * Read an 4-byte coordinate value from the provided input stream.
 */
COORD read_coord(FILE *fp) {
   int flag;
   COORD val, s1, s2, s3, s4;

   s1 = fgetc(fp);
   s2 = fgetc(fp);
   s3 = fgetc(fp);
   s4 = fgetc(fp);

#ifdef BUFFER_SDVI
   add_to_buffer(s1, s2, s3, s4);
#endif

   flag = 0;
   if ((s1 & 0x80) != 0x0) {
       flag = 1; 
       s1 = s1 & 0x7F;
   }

   val = (s1 << 24) | (s2 << 16) | (s3 << 8) | s4;
   if (flag != 0) val = -val;

   return val;
}

/*
 * Read an 4-byte floating point value from the input stream.
 */
float read_float(FILE *fp) {
   return (float) (read_coord(fp) / 1000000.0);
}
