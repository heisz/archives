/**
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

#include <stdio.h>
#include <stdlib.h>
#include "localdefs.h"

#include "../sdvi.h"
#include "sp_bm.h"
#include "patterns.h"

#ifndef PRINTER
#define PRINTER "/usr/ucb/lpr"
#endif

int sideways_fl = 0, scr_open = 0, use_stdin, n_dither, fs_dither, max_shade;
int x_resdim[2], y_resdim[2], depth, grey_fl, dev_type, true_depth;
int maj_ver, min_ver, curr_width;
char *pr_comm = PRINTER, *names[] = {"pbmraw"};
FILE *out_file;
unsigned char fast_rot[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

/* RGB data - note that grey just deals with red information */

unsigned int background[3];
unsigned char order_dither[3][8];

/* pixmap storage structures */

unsigned char *pixmap[3], n_planes, depth_planes;

#define N_DEVS sizeof(names)/sizeof(char *)

main(argc, argv)
int argc;
char *argv[];
{
   int c, comm, i, n, tf_fl, mono_fl;
   COORD x, y;
   float hue, sat, brt;
   char *in_filename, *out_filename, *pr_name, *ptr;
   char com_buff[1000];
   FILE *sdvi_file;

   grey_fl = mono_fl = use_stdin = fs_dither = 0;
   in_filename = out_filename = pr_name = (char *) NULL;
   depth = dev_type = n_dither = -1;
   x_resdim[1] = y_resdim[1] = x_resdim[2] = y_resdim[2] = -1;
   curr_width = 1;

   for (c = 1; c < argc; c++) {
     ptr = argv[c] + 2;
     if (*(argv[c]) == '-') {
       tf_fl = 0;
       switch(*(argv[c]+1)) {
         case 'D' : fs_dither = 1;
                    break;
         case 'd' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    depth = atoi(ptr);
                    if (depth <= 0) {
                      (void) fprintf(stderr,
                        "sp_bm: invalid depth specification %i.\n", depth);
                      depth = -1;
                    }
                    break;
         case 'f' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    for (i = 0; i < N_DEVS; i++) {
                      if (strcmp(ptr, names[i]) == 0) {
                        dev_type = i;
                        break;
                      }
                    }
                    if (i == N_DEVS) (void) fprintf(stderr,
                       "sp_bm: invalid format specification :%s:\n", ptr);
                    break;
         case 'g' : grey_fl = 1;
                    break;
         case 'h' : help();
                    break;
         case 'm' : mono_fl = 1;
                    break;
         case 'n' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    n_dither = atoi(ptr);
                    if ((n_dither != 0) && (n_dither != 16) && (n_dither != 64)) {
                      (void) fprintf(stderr,
                        "sp_bm: invalid depth specification %i.\n", depth);
                      n_dither = -1;
                    }
         case 'o' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    out_filename = ptr;
                    break;
         case 'P' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    pr_name = ptr;
                    break;
         case 'e' : tf_fl++;
         case 'r' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    for (i = 0; i < strlen(ptr); i++) {
                      if (*(ptr+i) == 'x') break;
                    }
                    if (i == strlen(ptr)) {
                      x_resdim[tf_fl] = y_resdim[tf_fl] = atoi(ptr);
                    }else{
                      *(ptr+i) = '\0';
                      x_resdim[tf_fl] = atoi(ptr);
                      y_resdim[tf_fl] = atoi(ptr+i+1);
                      *(ptr+i) = 'x';
                    }
                    if ((x_resdim[tf_fl] <= 0)||(y_resdim[tf_fl] <= 0)) {
                      (void) fprintf(stderr,
                        "sp_bm: invalid resolution/dimension %s.\n", ptr);
                      x_resdim[tf_fl] = y_resdim[tf_fl] = -1;
                    }
                    break;
         case 's' : sideways_fl = 1;
                    break;
         default  : (void) fprintf(stderr,
                      "sp_bm: option not one of XXX.\n");
                    break;
       }
     }else{
       in_filename = argv[c];
     }
   }

   if (dev_type<0) {
     (void) fprintf(stderr, "sp_bm: no output format specified.\n");
     (void) exit(1);
   }
   if (mono_fl != 0) {
     depth = grey_fl = 1;
   }

/* scan here for bitmap formats and default settings */

   if (fs_dither != 0) {
     n_dither = 0; 
     true_depth = 8;
   }else{
     true_depth = depth;
   }
   
   if (n_dither<0) n_dither = 64;
   max_shade = 64;

   if (in_filename == (char *) NULL) {
     sdvi_file = stdin;
     use_stdin = 1;
   }else{
     sdvi_file = fopen(in_filename, "r");
     if (sdvi_file == (FILE *) NULL) {
       (void) fprintf(stderr,
           "sp_bm: unable to open sdvi file %s.\n", in_filename);
       (void) exit(1);
     }
   }

   x = read_coord(sdvi_file);
   if (feof(sdvi_file) != 0) (void) exit(0);

   if (x != SDVI_MAGIC) {
     (void) fprintf(stderr, "sp_bm: not a sPLOTch! sdvi file.\n");
     (void) fprintf(stderr, "Magic number %i instead of %i.\n", x, SDVI_MAGIC);
     (void) exit(1);
   }
   maj_ver = read_coord(sdvi_file);
   min_ver = read_coord(sdvi_file);
   (void) read_coord(sdvi_file);

   if (out_filename != (char *) NULL) {
     if (*out_filename == '-') {
       out_file = stdout;
     }else{
       out_file = fopen(out_filename, "w");
       if (out_file == (FILE *) NULL) {
         (void) fprintf(stderr,
            "sp_bm: cannot open output file %s.\n", out_filename);
       }
     }
   }else{
     if (pr_name != (char *) NULL) {
       if (*pr_name == '-') {
         out_file = stderr;
       }else{
         (void) sprintf(com_buff, "%s -P%s", pr_comm, pr_name);
         out_file = popen(com_buff, "w");
         if (out_file == (FILE *) NULL) {
           (void) fprintf(stderr,
             "sp_bm: cannot open printer pipe via %s.\n", com_buff);
         }
       }
     }else{   
       out_file = stdout;
     }
   }

   while (((comm = read_coord(sdvi_file)) != END_OF_FILE)&&(feof(sdvi_file) == 0)) {
     switch(comm) {
       case OPEN_SCR:
           x = read_coord(sdvi_file);
           y = read_coord(sdvi_file);
           (void) read_float(sdvi_file);
           bm_open_screen(x, y);
           break;
       case UPD_SCR:
           /* nothing we can do */
           break;
       case CLOSE_SCR:
           break;
       case MOVE_PT:
           x = read_coord(sdvi_file);
           y = read_coord(sdvi_file);
           break;
       case DRAW_PT:
           x = read_coord(sdvi_file);
           y = read_coord(sdvi_file);
           break;
       case CH_WDTH:
           x = read_coord(sdvi_file);
           bm_change_width(x);
           break;
       case CH_COL:
           hue = read_float(sdvi_file);
           sat = read_float(sdvi_file);
           brt = read_float(sdvi_file);
           bm_change_colour(hue, sat, brt);
           break;
       case FILL_P:
           x = read_coord(sdvi_file);
           for (i = 0; i <= x; i++) {
             (void) read_coord(sdvi_file);
             (void) read_coord(sdvi_file);
           }
           break;
       case DIAGRAM:
           (void) read_coord(sdvi_file);
           (void) read_coord(sdvi_file);
           x = read_coord(sdvi_file);
           y = read_coord(sdvi_file);
           x = read_coord(sdvi_file);
           y = read_coord(sdvi_file);
           n = read_coord(sdvi_file);
           for (i = 0; i < n; i++) (void) fgetc(sdvi_file);
           break;
       default: 
           (void) fprintf(stderr, "sp_bm: bad sdvi command %i.\n", comm);
           (void) exit(1);
           break;
     }
   }

   if (in_filename != (char *) NULL) (void) fclose(sdvi_file);

   if (out_filename != (char *) NULL) {
     if (*out_filename != '-') (void) fclose(out_file);
   }else{
     if ((pr_name != (char *) NULL)&&(*pr_name != '-')) (void) pclose(out_file);
   }
   return(0);
}

help()
{
   int i, l;

   (void) fprintf(stderr, "Usage: sp_bm [options] [filename]\n");
   (void) fprintf(stderr, "\nOptions:\n--------\n");
   (void) fprintf(stderr,
       "  -d <depth>   = > specifies the <depth> in bits of the bit/pixmap\n");
   (void) fprintf(stderr,
       "  -D           = > uses Floyd-Steinberg error diffusion\n");
   (void) fprintf(stderr,
       "  -e <NxM>     = > sets the final size of the bit/pixmap (N by M)\n");
   (void) fprintf(stderr,
       "  -f <format>  = > determines the output <format> (see below)\n");
   (void) fprintf(stderr,
       "  -g           = > forces colour to greyscale conversion\n");
   (void) fprintf(stderr,
       "  -h           = > displays this help list\n");
   (void) fprintf(stderr,
       "  -m           = > forces monochrome output (bitmap)\n");
   (void) fprintf(stderr,
       "  -n <num>     = > uses ordered dithering shading of resolution <num>\n");
   (void) fprintf(stderr,
       "  -o <file>    = > outputs into the specified <file>\n");
   (void) fprintf(stderr,
       "                 ( - outputs to standard output)\n");
   (void) fprintf(stderr,
       "  -P <name>    = > forwards output to printer <name>\n");
   (void) fprintf(stderr,
       "                 ( - outputs to standard error)\n");
   (void) fprintf(stderr,
       "  -r <NxM>     = > specifies the bit/pixmap resolution (N by M dpi)\n");
   (void) fprintf(stderr,
       "  -s           = > displays sdvi file sideways\n");
   (void) fprintf(stderr,
       "\nIf no <filename> is given, sdvi commands are read from stdin.\n");
   (void) fprintf(stderr,
       "By default, all output is directed to standard output.\n");
   (void) fprintf(stderr,
       "\nPress <return> for a list of available output formats.\n");
   while(getchar() != '\n');
   (void) fprintf(stderr, "\nValid output formats are:\n\n");
   l = 0;
   for (i = 0; i < N_DEVS; i++) {
     (void) fprintf(stderr, "%s", names[i]);
     l += strlen(names[i]);
     if (l>60) {
       (void) fprintf(stderr, "\n");
       l = 0;
     }else (void) fprintf(stderr, " ");
   }
   (void) fprintf(stderr, "\n\n");
   (void) exit(0);
}

test_screen()
{
   if (scr_open == 0) {
     (void) fprintf(stderr, "sp_bm: defective sdvi file format.\n");
     (void) exit(1);
   }
}

bm_open_screen(xw, yw)
COORD xw, yw;
{
   if (scr_open != 0) {
     bm_close_screen();
     (void) fprintf(stderr, 
        "sp_bm: no capabilities exist for multiple page bitmaps.");
     (void) exit(1);
   }
   scr_open = 1;
}

bm_close_screen()
{
}

bm_change_width(width)
COORD width;
{
   curr_width = width;
}

bm_change_colour(hue, sat, bright)
float hue, sat, bright;
{
  float red, green, blue, grey_scale(), triad[3], error;
  int i, j, max, tile_num, sh_main, rem;
  
  test_screen();

  if (grey_fl != 0) {
     hsb_to_rgb(hue, sat, bright, &red, &green, &blue);
     hue = sat = 0.0;
     bright = grey_scale(red, green, blue);
  }
  hsb_to_rgb(hue, sat, bright, &(triad[0]), &(triad[1]), &(triad[2]));

  if (grey_fl != 0) {
    triad[0] = bright;
    max = 1;
  }else max = 3;

  for (i = 0; i < max; i++) {
    background[i] = triad[i]*max_shade;
    if (n_dither != 0) {
      error = triad[i]*max_shade - background[i];
      tile_num = error*n_dither+0.5;
      if (n_dither == 16) {
        for (j = 0; j < 4; j++) {
          order_dither[i][j] = dither_bits[tile_num][j];
          order_dither[i][j] |= (dither_bits[tile_num][j]<<4);
          order_dither[i][j+4] = order_dither[i][j];
        }
      }else{
        sh_main = tile_num/4;
        rem = tile_num-sh_main*4;
        for (j = 0; j < 4; j++) {
          order_dither[i][j] = dither_bits[sh_main+((rem>2)?1:0)][j];
          order_dither[i][j] |= (dither_bits[sh_main+((rem>0)?1:0)][j]<<4);
          order_dither[i][j+4] = dither_bits[sh_main+((rem>1)?1:0)][j];
          order_dither[i][j+4] |= (dither_bits[sh_main+((rem>3)?1:0)][j]<<4);
        }
      }
    }
  }
}

/* set_point - sets the specified point on the bitmaps, according to
               the current colour information
             - Note: for external interface, neither the bitmaps nor
               the colour parameters are passed (global variables) */

set_point(x, y)
int x, y;
{
}

#define RGB
#include "../common.c"
