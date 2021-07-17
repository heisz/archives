/**
  * sPLOTch! X11-based display driver.
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
 *   As time has evolved, this driver has become complicated enough
 *   to warrant the use of Xt to handle the objects...but as the old
 *   saying goes: if it ain't broken, don't fix it!!
 */

#include <stdio.h>
#include <stdlib.h>
#include "localdefs.h"
#include <sys/select.h>
#include <sys/time.h>
#include <sys/select.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Xmu/StdCmap.h>

#define BUFFER_SDVI 1
void add_to_buffer(int s1, int s2, int s3, int s4);
#include "../sdvi.h"

#include "splotch.xbm"
#include "splotch.xcurs"
#include "shades.xbm"

#ifndef DEF_PRCOMM
#define DEF_PRCOMM "sp_ps -Psparc"
#endif

typedef struct point {COORD x, y;} point;

/* Forward declarations */
void init_print_buffer();
void rem_print_buffer();
void help();
void init_display();
void close_display();
void x_open_screen(COORD xw, COORD yw, float winfract);
void x_close_screen();
void x_update();
void x_move(COORD xp, COORD yp);
void x_draw(COORD xp, COORD yp);
void x_poly(point *poly, int nPoints);
void x_width(COORD x);
void x_colour(float hue, float sat, float bright);
void x_proc_events(int status);
void x_out_text(char *str, Window win, int width);
void x_abort();
void x_update();
void x_close_screen();
void doEvents(int *quitFlag, int status);
void draw_frame();
void draw_button(int status);
void inc_work(int flag);
void resizeGraphWin();
void drawPage(int x, int y, int flag);
void closeWins();
void convPoint(COORD *x, COORD *y, COORD xp, COORD yp);
void draw_status();
void nextFrame();
void draw_cross(int x, int y);
void drawPosition(int x, int y, int flag);
void draw_on_off(int onoff);
void do_print(char *print_comm);

extern void hsb_to_rgb(float hue, float sat, float brt,
                       float *rd, float *gr, float *bl);

/**
  *  The following are the Xdefaults/command line options
  */

#define PROGRAM_NAME "splotch"

#define N_OPTIONS 14
struct {
          char *cl_name, *id_name, *dflt;
        } options_list[N_OPTIONS] = {
#define O_DISP 0
          {"display", (char *) NULL,  (char *) NULL},
#define O_NAME 1
          {"name", (char *) NULL,  (char *) NULL},
#define O_BD 2
          {"bd", "borderColor", "black"},
#define O_BG 3
          {"bg", "background", "LightGrey"},
#define O_BW 4
          {"bw", "borderWidth", "1"},
#define O_CC 5
          {"cc", "cursorColor", "MediumSlateBlue"},
#define O_FG 6
          {"fg", "foreground", "MidnightBlue"},
#define O_FN 7
          {"fn", "font", "7x13"},
#define O_GEOM 8
          {"geometry", "geometry", (char *) NULL},
#define O_SCRN 9
          {"screen", "screen", (char *) NULL},
#define O_TITLE 10
          {"title", "title", "sPLOTch!"},
#define O_VT 11
          {"vt", "visualType", (char *) NULL},
#define O_WIN 12
          {"w", "winFract", (char *) NULL},
#define O_DT 13
          {"dt", "deskTop", (char *) NULL}
        };

char *opt_names[N_OPTIONS];

char *visualtypes[] = {"StaticColor", "PseudoColor", "StaticGray",
                       "GrayScale", "TrueColor", "DirectColor"};
int visualclass[] = {StaticColor, PseudoColor, StaticGray, GrayScale,
                     TrueColor, DirectColor};
int col_inds[4] = {O_BG, O_BD, O_CC, O_FG};

/* Screen size layout parameters */

#define CHARWIDTH 30
#define CHARHEIGHT 23
#define ANIMHEIGHT 10
#define PRINTWIDTH 40
#define PRINTHEIGHT 7 

unsigned char patterns[65][8];
unsigned char rotates[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

int sideways_fl, scr_open = 0, maj_ver, min_ver, interactive;
int grey_fl, mono_fl, install_fl, best_fl, quiet_fl, anim_fl;
int num_dither_col, num_dither_grey, n_animate;
int reverse_fl, X_main_fd;
#ifdef DEBUG
int dbg_fl;
#endif
char *my_name = "sPLOTch!", f_name[300], ver_name[100], fract[100];

/**
  *  3...2...1...Launch program!
  */
int main(int argc, char *argv[]) {
    int c, comm, i, n, exit_fl, q_fl, nfds;
    COORD x, y, xr, yr;
    float va, vb, vc;
    char *in_filename;
    FILE *sdvi_file;
    fd_set fds;
    struct point *poly;

    /* Initialize options flags/tables */
    anim_fl = n_animate = 0;
    quiet_fl = grey_fl = mono_fl = install_fl = 0;
    sideways_fl = reverse_fl = best_fl = 0;
    num_dither_col = num_dither_grey = -1;
    in_filename = (char *) NULL;
    for (i = 0; i < N_OPTIONS; i++) opt_names[i] = (char *) NULL;

    /* Parse command line option set */
    for (c = 1; c < argc; c++) {
        if (*(argv[c]) == '-') {
            for (i = 0; i < N_OPTIONS; i++) {
                if (strcmp((argv[c]+1), options_list[i].cl_name) == 0) {
                    opt_names[i] = argv[++c];
                    break;
                }
            }
            if (i == N_OPTIONS) switch(*(argv[c]+1)) {
                case 'a' : anim_fl = 1;
                           break;
                case 'b' : best_fl = 1;
                           break;
                case 'd' : 
#ifdef DEBUG
                           dbg_fl = 1;
#else
                (void) fprintf(stderr,
                     "sp_x: debugging (-d option) disabled at compilation.\n");
#endif
                           break;
                case 'g' : grey_fl = 1;
                           break;
                case 'h' : help();
                           break;
                case 'i' : 
#ifndef NOXMU
                           install_fl = 1;
#else
               (void) fprintf(stderr,
       "sp_x: standard map installation (-i option) disabled at compilation.\n");
#endif
                           break;
                case 'm' : mono_fl = 1;
                           break;
                case 'n' : if ((*(argv[c] + 2) == '\0') && 
                               (argv[c + 1] != (char *) NULL)) {
                               n = atoi(argv[++c]);
                           } else {
                               n = atoi(argv[c]+2);
                           }
                           if ((n != 0) && (n != 16) && (n != 64)) {
                               (void) fprintf(stderr,
                                              "sp_x: bad dither resolution.\n");
                           } else {
                               num_dither_grey = num_dither_col = n;
                           }
                           break;
                case 'q' : quiet_fl = 1;
                           break;
                case 'r' : reverse_fl = 1;
                           break;
                case 's' : sideways_fl = 1;
                           break;
                default :  (void) fprintf(stderr,
                                     "sp_x: option not one of abdghimnqrs.\n");
                           break;
            }
        } else {
            in_filename = argv[c];
        }
    }

    /* Determine sdvi command sequence origin */
    if (in_filename == (char *) NULL) {
        sdvi_file = stdin;
        (void) sprintf(f_name, "Filename: <stdin>");
    } else {
        sdvi_file = fopen(in_filename, "r");
        if (sdvi_file == (FILE *) NULL) {
            (void) fprintf(stderr, "sp_x: unable to open sdvi file %s.\n",
                                   in_filename);
            (void) exit(1);
        }
        (void) sprintf(f_name, "Filename: %s", in_filename);
    }

    /* Check file initializations */
    init_print_buffer();
    x = read_coord(sdvi_file);
    if (feof(sdvi_file) != 0) (void) exit(0);

    if (x != SDVI_MAGIC) {
        (void) fprintf(stderr,"sp_x: not a sPLOTch! sdvi file.\n");
        (void) fprintf(stderr,"Magic number %d not %d.\n", (int) x, SDVI_MAGIC);
        exit(1);
    }
    maj_ver = read_coord(sdvi_file);
    min_ver = read_coord(sdvi_file);
    (void) sprintf(ver_name, "Version: %i.%i", maj_ver, min_ver);
    interactive = read_coord(sdvi_file);

    init_display();

    /* process sdvi commands - simultaneously, handle X events (abort) */
    comm = exit_fl = 0;
    while (exit_fl == 0) {
        c = fileno(sdvi_file);
        nfds = ((X_main_fd > c) ? (X_main_fd) : (c))+1;
        FD_ZERO(&fds);
        FD_SET(X_main_fd, &fds);
        FD_SET(fileno(sdvi_file), &fds);
        if ((interactive != 0) && (comm == UPD_SCR)) {
            (void) select(nfds, &fds, (fd_set *) NULL,
                          (fd_set *) NULL, (struct timeval *) NULL);
            if ((FD_ISSET(X_main_fd, &fds)) && (interactive != 0)) {
                q_fl = 0;
                doEvents(&q_fl, 2);
                if (q_fl != 0) x_abort();
            }
        }
        if (FD_ISSET(fileno(sdvi_file), &fds)) { 
            comm = read_coord(sdvi_file);
            if ((comm != END_OF_FILE) && (feof(sdvi_file) == 0)) {
                switch(comm) {
                    case OPEN_SCR:
                        if (scr_open != 0) {
                            if (anim_fl == 0) {
                                rem_print_buffer();
                                x_proc_events(0);
                            }
                        }
                        init_print_buffer();
                        x = read_coord(sdvi_file);
                        y = read_coord(sdvi_file);
                        va = read_float(sdvi_file);
                        if (opt_names[O_WIN] != (char *) NULL) {
                            va = atof(opt_names[O_WIN]);
                        }
                        x_open_screen(x, y, va);
                        break;
                    case UPD_SCR:
                        x_update();
                        break;
                    case CLOSE_SCR:
                        x_close_screen();
                        break;
                    case MOVE_PT:
                        x = read_coord(sdvi_file);
                        y = read_coord(sdvi_file);
                        x_move(x, y);
                        break;
                    case DRAW_PT:
                        x = read_coord(sdvi_file);
                        y = read_coord(sdvi_file);
                        x_draw(x, y);
                        break;
                    case CH_WDTH:
                        x = read_coord(sdvi_file);
                        x_width(x);
                        break;
                    case CH_COL:
                        va = read_float(sdvi_file);
                        vb = read_float(sdvi_file);
                        vc = read_float(sdvi_file);
                        x_colour(va, vb, vc);
                        break;
                    case FILL_P:
                        n = read_coord(sdvi_file);
                        poly = (struct point *) malloc((unsigned int)
                                                 (n+2)*sizeof(struct point));
                        if (poly == (struct point *) NULL) {
                            (void) fprintf(stderr,
                                 "sp_x: unable to allocate polygon memory.\n");
                            (void) exit(1);
                        }
                        for (i = 0; i <= n; i++) {
                            (poly+i)->x = read_coord(sdvi_file);
                            (poly+i)->y = read_coord(sdvi_file);
                        }
                        x_poly(poly, n);
                        free((char *) poly);
                        break;
                    case DIAGRAM:
                        (void) read_coord(sdvi_file);
                        (void) read_coord(sdvi_file);
                        x = read_coord(sdvi_file);
                        y = read_coord(sdvi_file);
                        xr = read_coord(sdvi_file);
                        yr = read_coord(sdvi_file);
                        n = read_coord(sdvi_file);
                        for (i = 0; i < n; i++) (void) fgetc(sdvi_file);
                        x_move(x, y);
                        x_draw(xr, y);
                        x_draw(xr, yr);
                        x_draw(x, yr);
                        x_draw(x, y);
                        break;
                    default: 
                        (void) fprintf(stderr,
                                       "sp_x: bad sdvi command %i.\n",comm);
                        (void) exit(1);
                        break;
                 }
             } else exit_fl = 1;
         }
    }

    /* All done - shut down cleanly */
    if (scr_open != 0) x_close_screen();

    close_display();
    return(0);
}

void help() {
    (void) fprintf(stderr,"\nUsage: sp_x [options] [filename]\n");
    (void) fprintf(stderr,"\nOptions:\n--------\n");
    (void) fprintf(stderr,
            "  -a        = > initializes animation mode\n");
    (void) fprintf(stderr,
            "  -b        = > requests the \"best\" colourmap be accessed\n");
#ifdef DEBUG
    (void) fprintf(stderr,
            "  -d        = > turns on debugging output\n");
#endif
    (void) fprintf(stderr,
            "  -g        = > forces colour to greyscale conversion\n");
    (void) fprintf(stderr,
            "  -h        = > displays this help list\n");
#ifndef NOXMU
    (void) fprintf(stderr,
            "  -i        = > requests that a standard colourmap be installed\n");
#endif
    (void) fprintf(stderr,
            "  -m        = > forces output to be in monochrome halfshading\n");
    (void) fprintf(stderr,
            "  -n <num>  = > uses a dithering pattern of resolution <num>\n");
    (void) fprintf(stderr,
            "  -q        = > work quietly\n");
    (void) fprintf(stderr,
            "  -s        = > displays output in landscape orientation\n");
    (void) fprintf(stderr,
            "\nPress <return> to continue.\n");
    while(getchar() != '\n');
    
    (void) fprintf(stderr,
         "\nX Window Options:\n----------------\n");
    (void) fprintf(stderr,
         "  -display <name>  = > connects to the specified server <name>\n");
    (void) fprintf(stderr,
         "  -name <name>     = > use <name> to search Xresources database\n");
    (void) fprintf(stderr,
         "  -bd <colour>     = > specifies the window border <colour>\n");
    (void) fprintf(stderr,
         "  -bg <colour>     = > specifies the background <colour>\n");
    (void) fprintf(stderr,
         "  -bw <width>      = > sets the border thickness to <width>\n");
    (void) fprintf(stderr,
         "  -cc <colour>     = > specifies the cursor <colour>\n");
    (void) fprintf(stderr,
         "  -dt <bitmap>     = > specifies the desktop backing <bitmap>\n");
    (void) fprintf(stderr,
         "  -fg <colour>     = > specifies the foreground <colour>\n");
    (void) fprintf(stderr,
         "  -fn <font>       = > specifies the window character <font>\n");
    (void) fprintf(stderr,
         "  -geometry <val>  = > determines the main window size/position\n");
    (void) fprintf(stderr,
         "  -rv              = > requests that reverse video be used\n");
    (void) fprintf(stderr,
         "  -screen <num>    = > outputs to the given display screen <num>\n");
    (void) fprintf(stderr,
         "  -title <title>   = > passes the <title> to the window manager\n");
    (void) fprintf(stderr,
         "  -vt <type>       = > attempts to use the given visual <type>\n");
    (void) fprintf(stderr,
         "  -w <num>         = > specifies an override WINFRACT value <num>\n");
    (void) fprintf(stderr,
           "\nIf no <filename> is given, sdvi commands read from stdin.\n\n");

    (void) exit(0);
}

void testScreen()
{
    if (scr_open == 0) {
      (void) fprintf(stderr,"sp_x: defective sdvi file format.\n");
      (void) exit(1);
    }
}

Display *X_display;
Screen *X_screen;
Window X_root;
XVisualInfo *X_info, vinfo_data;
Colormap X_cmap;
XFontStruct *X_font;
XStandardColormap *X_stdcmap, *stdcmapret;
Pixmap X_icon, X_shades[17], X_shade_grey, X_shade_col, X_desk;
XImage *X_col_image;
Cursor X_gr_curs, X_pnl_curs, X_watch_curs;
int X_sc_num, has_colour, has_std_cmap, sc_ht_pix, sc_wd_pix;
int bord_w, txt_ht, txt_wd, txt_offset, desk_flag, grey_max;
float x_dpi, y_dpi;
unsigned long black_pixel, white_pixel;
unsigned long colours[4], primaries[8];

void init_display() {
    int i, j, n, class, maxdepth, col_fl, dt_w, dt_h, jkx, jky;
    int h_mm, w_mm, tile, rem;
    XStandardColormap *cptr;
    XVisualInfo *vinfo, *vptr, tv_info;
    XColor exact_col, scr_col;
    XGCValues gcv;
    unsigned long gcv_mask;
    GC t_gc;
    Atom prop;
    Status rc;
    Pixmap tpxmap;
    char *prg_name;
#ifdef DEBUG
    char *print_type(), *print_class();
#endif

    if (opt_names[O_DISP] == (char *) NULL) opt_names[0] = getenv("DISPLAY");
    X_display = XOpenDisplay(opt_names[O_DISP]);
    if (X_display == (Display *) NULL) {
      if (opt_names[O_DISP] == (char *) NULL) 
       (void) fprintf(stderr,"sp_x: cannot open X display (null).\n");
      else
       (void) fprintf(stderr,"sp_x: cannot open X display %s.\n",
           opt_names[O_DISP]);
      (void) exit(1);
    }

    prg_name = opt_names[O_NAME];
    if (prg_name == (char *) NULL) prg_name = PROGRAM_NAME;

    for (i = 2; i < N_OPTIONS; i++) {
      if (opt_names[i] == (char *) NULL) {
        opt_names[i] = XGetDefault(X_display, prg_name, options_list[i].id_name);
      }
#ifdef DEBUG
if (dbg_fl != 0) {
    (void) fprintf(stderr,"sp_x: option: %s -> %s\n", options_list[i].id_name,
                (opt_names[i] == (char *) NULL) ? "(null)" : opt_names[i]);
}
#endif
    }

    if (opt_names[O_SCRN] == (char *) NULL) {
      X_sc_num = DefaultScreen(X_display);
    } else {
      X_sc_num = atoi(opt_names[O_SCRN]);
      if ((X_sc_num < 0) || (X_sc_num >= XScreenCount(X_display))) {
        (void) fprintf(stderr,"sp_x: invalid screen number %i.\n", X_sc_num);
        (void) exit(1);
      }
    }
    X_screen = ScreenOfDisplay(X_display, X_sc_num);
    X_main_fd = ConnectionNumber(X_display);
    X_root = RootWindow(X_display, X_sc_num);

    if (opt_names[O_VT] == (char *) NULL) {
      tv_info.visual = DefaultVisual(X_display, X_sc_num);
      tv_info.visualid = XVisualIDFromVisual(tv_info.visual);
      vinfo = XGetVisualInfo(X_display, VisualIDMask, &tv_info, &n);
      if (vinfo == (XVisualInfo *) NULL) {
        (void) fprintf(stderr,"sp_x: unable to load default visual info.\n");
        (void) exit(1);
      }
      class = vinfo->class;
      XFree((char *) vinfo);
    } else {
      class = -1;
      for (i = 0; i < 6; i++) 
        if (strcmp(visualtypes[i], opt_names[O_VT]) == 0) class = visualclass[i];
      if (class == -1) {
        (void) fprintf(stderr,"sp_x: bad Visual Type %s.\n",opt_names[O_VT]);
        (void) exit(1);
      }
    }
    maxdepth = -1;
    tv_info.screen = X_sc_num;
    vptr = vinfo = XGetVisualInfo(X_display, VisualScreenMask, &tv_info, &n);
    for (i = 0; i < n; i++) {
      if ((vptr->class == class) && (vptr->depth > maxdepth)) {
        maxdepth = vptr->depth;
        vinfo_data =  *vptr;
      }
      vptr++;
    }
    XFree((char *) vinfo);
    if (maxdepth == -1) {
      if (opt_names[O_VT] != (char *) NULL) 
        (void) fprintf(stderr,"sp_x: visual Type %s is not appropriate.\n",
           opt_names[O_VT]);
      else
        (void) fprintf(stderr,
                 "sp_x internal error: default visual does not exist.\n");
      (void) exit(1);
    }

    X_info = &vinfo_data; /* kludge to guarantee visual depth */

    if (X_info->colormap_size <= 2) {
      mono_fl = 1;
      has_colour = 0;
      has_std_cmap = 0;
    } else {
      has_std_cmap = 0;
      has_colour = 1;
      if ((X_info->class == StaticGray) || (X_info->class == GrayScale)) {
        has_colour = 0;
        grey_fl = 1;
      }

      if (has_colour != 0) {
        if (best_fl == 0) prop = XA_RGB_DEFAULT_MAP;
        else prop = XA_RGB_BEST_MAP;
      } else {
        prop = XA_RGB_GRAY_MAP;
      }
#ifdef DEBUG
if (dbg_fl != 0) {
   (void) fprintf(stderr, "sp_x: Looking for %s on %s with depth %i.\n",
             print_type(prop), print_class(X_info->class), X_info->depth);
}
#endif

      rc = XGetRGBColormaps(X_display, X_root, &stdcmapret, &n, prop);
      if (rc) {
        cptr = stdcmapret;
        for (i = 0; i < n; i++) {
          if (X_info->visualid == cptr->visualid) {
            X_stdcmap = cptr;
            has_std_cmap = 1;
            break;
          }
          cptr++;
        }
      }

#ifndef NOXMU
      if ((has_std_cmap == 0) && (install_fl != 0)) {
#ifdef DEBUG
if (dbg_fl != 0) {
   (void) fprintf(stderr, 
       "sp_x: No standard colourmap available...attempting to install.\n");
}
#endif
        rc = XmuLookupStandardColormap(X_display, X_sc_num,
             X_info->visualid, X_info->depth, prop, 0, 1);
        if (rc) {
          rc = XGetRGBColormaps(X_display, X_root, &stdcmapret, &n, prop);
          if (rc) {
            cptr = stdcmapret;
            for (i = 0; i < n; i++) {
              if (X_info->visualid == cptr->visualid) {
                X_stdcmap = cptr;
                has_std_cmap = 1;
                break;
              }
              cptr++;
            }
          }
        } else {
#ifdef DEBUG
if (dbg_fl != 0) {
   (void) fprintf(stderr, "sp_X: XmuLookup failure.  No colourmap installed.\n");
}
#endif
        }
      }
#endif
#ifdef DEBUG
if (dbg_fl != 0) {
   if (has_std_cmap == 0) {
     (void) fprintf(stderr,"sp_x: No standard colourmap available.\n");
   } else {
     (void) fprintf(stderr,"sp_x: Standard colourmap available.\n");
     (void) fprintf(stderr,"Red_max %i Blue_max %i Green_max %i\n",
        X_stdcmap->red_max, X_stdcmap->blue_max, X_stdcmap->green_max);
   }
}
#endif
    }

    grey_max = 0;
    if (has_std_cmap != 0) {
      X_cmap = X_stdcmap->colormap;
      if (has_colour != 0) {
        black_pixel = X_stdcmap->base_pixel;
        white_pixel = X_stdcmap->base_pixel+
             X_stdcmap->red_max*X_stdcmap->red_mult+
             X_stdcmap->green_max*X_stdcmap->green_mult+
             X_stdcmap->blue_max*X_stdcmap->blue_mult;
        if (num_dither_col == -1) num_dither_col = 16;
        if ((X_stdcmap->red_max == X_stdcmap->green_max) &&
              (X_stdcmap->blue_max == X_stdcmap->green_max)) { /* true grey */
          grey_max = X_stdcmap->red_max;
          if (num_dither_grey == -1) num_dither_grey = 16;
        } else {
          if (num_dither_grey == -1) num_dither_grey = 64;
        }
      } else {
        black_pixel = X_stdcmap->base_pixel;
        white_pixel = X_stdcmap->base_pixel +
             X_stdcmap->red_max*X_stdcmap->red_mult;
        grey_max = X_stdcmap->red_max;
        if (num_dither_grey == -1) num_dither_grey = 16;
      }
    } else {
      if (X_info->visual == DefaultVisual(X_display, X_sc_num)) {
        X_cmap = DefaultColormap(X_display, X_sc_num);
        white_pixel = XWhitePixel(X_display, X_sc_num);
        black_pixel = XBlackPixel(X_display, X_sc_num);
      } else {
        X_cmap = XCreateColormap(X_display, X_root, X_info->visual, AllocNone);
        scr_col.red = scr_col.blue = scr_col.green = ~(unsigned short) 0;
        (void) XAllocColor(X_display, X_cmap, &scr_col);
        white_pixel = scr_col.pixel;
        scr_col.red = scr_col.blue = scr_col.green = 0;
        (void) XAllocColor(X_display, X_cmap, &scr_col);
        black_pixel = scr_col.pixel;
      }
      if (mono_fl == 0) {
        if (grey_fl != 0) {
          rem = 65535;
          for (i = 0; i < 8; i++) {
            tile = rem/7.0*i;
            scr_col.red = scr_col.blue = scr_col.green = tile;
            (void) XAllocColor(X_display, X_cmap, &scr_col);
            primaries[i] = scr_col.pixel;
          }
          grey_max = 7;
          if (num_dither_grey == -1) num_dither_grey = 16;
        } else {
          for (i = 0; i < 8; i++) {
            scr_col.red = (((i & 1) != 0) ? ~(unsigned short) 0 : 0);
            scr_col.blue = (((i & 2) != 0) ? ~(unsigned short) 0 : 0);
            scr_col.green = (((i & 4) != 0) ? ~(unsigned short) 0 : 0);
            (void) XAllocColor(X_display, X_cmap, &scr_col);
            primaries[i] = scr_col.pixel;
          }
          if (num_dither_grey == -1) num_dither_grey = 64;
          if (num_dither_col == -1) num_dither_col = 64;
        }
      } else {
        if (num_dither_grey == -1) num_dither_grey = 64;
      }
    }
  
    for (i = 0; i <= 64; i++) {
      tile = i/4;
      rem = i-tile*4;
      for (j = 0; j < 4; j++) 
              patterns[i][j] = shade_bits[tile+((rem>2) ? 1 : 0)][j];
      for (j = 0; j < 4; j++) 
              patterns[i][j] |= (shade_bits[tile+((rem>0)? 1 : 0)][j] << 4);
      for (j = 0; j < 4; j++) 
              patterns[i][j+4] = shade_bits[tile+((rem>1)? 1 : 0)][j];
      for (j = 0; j < 4; j++) 
              patterns[i][j+4] |= (shade_bits[tile+((rem>3)? 1 : 0)][j] << 4);
    }

#ifdef DEBUG
if (dbg_fl != 0) {
    (void) fprintf(stderr,"sp_x: Dither resolutions: grey %i colour %i.\n",
                   num_dither_grey, num_dither_col);
}
#endif

    for (i = 0; i < 4; i++) {
      col_fl = 0;
      if (reverse_fl == 0) {
        if (i < 1) colours[i] = white_pixel;
        else colours[i] = black_pixel;
      } else {
        if (i < 1) colours[i] = black_pixel;
        else colours[i] = white_pixel;
      }
      if (opt_names[col_inds[i]] != (char *) NULL) {
        rc = XParseColor(X_display, X_cmap, opt_names[col_inds[i]], &scr_col);
        if (rc) {
          rc = XAllocColor(X_display, X_cmap, &scr_col);
          if (rc) {
            colours[i] = scr_col.pixel;
            col_fl = 1;
          }
        } else {
          (void) fprintf(stderr,"sp_x: bad colour specification for %s:%s.\n",
             options_list[col_inds[i]].id_name, opt_names[col_inds[i]]);
        }
      }
      if ((col_fl == 0) && (has_colour != 0)) {
        rc = XAllocNamedColor(X_display, X_cmap, 
            options_list[col_inds[i]].dflt, &scr_col, &exact_col);
        if (rc) colours[i] = scr_col.pixel;
      }
    }

    X_icon = XCreateBitmapFromData(X_display, X_root,
            splotch_xbm_bits, splotch_xbm_width, splotch_xbm_height);
    if (X_icon == None) {
      (void) fprintf(stderr,"sp_x: unable to create icon pixmap.\n");
      (void) exit(1);
    }
    desk_flag = 0;
    if (opt_names[O_DT] == (char *) NULL) {
      tpxmap = X_icon;
      dt_w = splotch_xbm_width;
      dt_h = splotch_xbm_height;
    } else {
      if (strcmp(opt_names[O_DT],"none") == 0) {
        desk_flag = -1;
      } else {
        desk_flag = 1;
        rc = XReadBitmapFile(X_display, X_root, opt_names[O_DT],
                  &dt_w, &dt_h, &tpxmap, &jkx, &jky);
        if (rc != BitmapSuccess) {
          (void) fprintf(stderr,"sp_x: unable to access desktop bitmap %s.\n", 
                    opt_names[O_DT]);
          (void) exit(1);
        }
      }
    }
    if (desk_flag >= 0) {
      X_desk = XCreatePixmap(X_display, X_root, dt_w, dt_h, X_info->depth);
      gcv.foreground = colours[3];
      gcv.background = colours[0];
      gcv_mask = GCForeground|GCBackground;
      t_gc = XCreateGC(X_display, X_desk, gcv_mask, &gcv);
      XCopyPlane(X_display, tpxmap, X_desk, t_gc, 0, 0, dt_w, dt_h,
          0, 0, (unsigned long) 1);
      XFreeGC(X_display, t_gc);
      if (desk_flag>0) XFreePixmap(X_display, tpxmap);
    }

    scr_col.pixel = colours[2];
    XQueryColor(X_display, X_cmap, &scr_col);

    tpxmap = XCreateBitmapFromData(X_display, X_root,
            splcurs_xbm_bits, splcurs_xbm_width, splcurs_xbm_height);
    if (tpxmap == None) {
      (void) fprintf(stderr,"sp_x: unable to create cursor map.\n");
      (void) exit(1);
    }
    X_pnl_curs = XCreatePixmapCursor(X_display, tpxmap, tpxmap,
                  &scr_col, &scr_col, 16, 0);
    XFreePixmap(X_display, tpxmap);
    tpxmap = XCreateBitmapFromData(X_display, X_root,
            splbl_xbm_bits, splbl_xbm_width, splbl_xbm_height);
    if (tpxmap == None) {
      (void) fprintf(stderr,"sp_x: unable to create cursor map.\n");
      (void) exit(1);
    }
    X_gr_curs = XCreatePixmapCursor(X_display, tpxmap, tpxmap,
                  &scr_col, &scr_col, 16, 0);
    XFreePixmap(X_display, tpxmap);
/*   tpxmap = XCreateBitmapFromData(X_display, X_root,
  *          watch_xbm_bits, watch_xbm_width, watch_xbm_height);
  *  if (tpxmap == None) {
  *    (void) fprintf(stderr,"sp_x: unable to create cursor map.\n");
  *    (void) exit(1);
  *  }
  *  X_watch_curs = XCreatePixmapCursor(X_display, tpxmap, tpxmap,
  *                &scr_col, &scr_col, 16, 0);
  *  XFreePixmap(X_display, tpxmap);
  */
    X_watch_curs = XCreateFontCursor(X_display, XC_watch);
    
    for (i = 0; i < 17; i++) {
      X_shades[i] = XCreateBitmapFromData(X_display, X_root,
         shade_bits[i], shade_width, shade_height);
      if (X_shades[i] == None) {
        (void) fprintf(stderr,"sp_x: cannot allocate shading maps.\n");
        (void) exit(1);
      }
    }
    if (num_dither_grey == 64) 
      X_shade_grey = XCreatePixmap(X_display, X_root, 8, 8, 1);
    else
      X_shade_grey = XCreatePixmap(X_display, X_root, 4, 4, 1);
    if (X_shade_grey == None) {
      (void) fprintf(stderr,"sp_x: cannot allocate master shading map.\n");
      (void) exit(1);
    }
    if (num_dither_col == 64) {
      X_shade_col = XCreatePixmap(X_display, X_root, 8, 8, X_info->depth);
      X_col_image = XGetImage(X_display, X_shade_col, 0, 0, 8, 8,
         AllPlanes, ZPixmap);
    } else {
      X_shade_col = XCreatePixmap(X_display, X_root, 4, 4, X_info->depth);
      X_col_image = XGetImage(X_display, X_shade_col, 0, 0, 4, 4,
         AllPlanes, ZPixmap);
    }
    if (X_shade_col == None) {
      (void) fprintf(stderr,"sp_x: cannot allocate master shading map.\n");
      (void) exit(1);
    }

    h_mm = XDisplayHeightMM(X_display, X_sc_num);
    w_mm = XDisplayWidthMM(X_display, X_sc_num);
    sc_ht_pix = XDisplayHeight(X_display, X_sc_num);
    sc_wd_pix = XDisplayWidth(X_display, X_sc_num);
    x_dpi = sc_wd_pix*(25.4/w_mm);
    y_dpi = sc_ht_pix*(25.4/h_mm);

    if (opt_names[O_FN] == (char *) NULL) {
      opt_names[O_FN] = options_list[O_FN].dflt;
    }
    X_font = XLoadQueryFont(X_display, opt_names[O_FN]);
    if (X_font == (XFontStruct *) NULL) {
      (void) fprintf(stderr,"sp_x: cannot load font %s - using fixed.\n",
        opt_names[O_FN]);
      X_font = XLoadQueryFont(X_display, "fixed");
      if (X_font == (XFontStruct *) NULL) {
        (void) fprintf(stderr,"sp_x: unable to load fixed font either.\n");
        (void) exit(1);
      }
    }
    txt_ht = X_font->max_bounds.ascent+X_font->max_bounds.descent;
    txt_wd = X_font->max_bounds.rbearing-X_font->min_bounds.lbearing;
    txt_offset = (X_font->max_bounds.ascent-X_font->max_bounds.descent)/2;

    bord_w = 2;
    if (opt_names[O_BW] != (char *) NULL) bord_w = atoi(opt_names[O_BW]);
}

void close_display() {
    int i;

    XFreeFont(X_display, X_font);
    for (i = 0; i < 17; i++) XFreePixmap(X_display, X_shades[i]);
    XFreeCursor(X_display, X_watch_curs);
    XFreeCursor(X_display, X_gr_curs);
    XFreeCursor(X_display, X_pnl_curs);
    XFreePixmap(X_display, X_icon);
    if (desk_flag >= 0) XFreePixmap(X_display, X_desk);
    XFreePixmap(X_display, X_shade_grey);
    XFreePixmap(X_display, X_shade_col);

    XFreeColors(X_display, X_cmap, colours, 4, 0L);
}

Window X_main_wind, X_graph_wind, X_pnl_wind, X_pos_wind, X_but_wind;
Window X_work_wind; 
Window X_anim_wind, X_onoff_wind, X_fast_wind, X_slow_wind, X_frame_wind;
Window X_print_wind, X_prbutt_wind, X_prOK_wind, X_prCanc_wind, X_prText_wind;
GC X_gc_shade_grey, X_gc_shade_col, X_gc_colour, X_gc_text, X_gc_ch;
Pixmap X_pixmap, X_animaps[100];
/* Atom X_protocol, X_kill; */
int graph_wd, graph_ht, pix_wd, pix_ht;
COORD x_max, y_max, x_ox, x_oy;
int work_status, work_level, m_ox =  -1, m_oy =  -1, del_x, del_y;
int grab_x, grab_y, oversize, main_wd, main_ht;
float fact_x, fact_y;

void x_open_screen(COORD xw, COORD yw, float winfract) {
    char *ptr;
    int x0, win_wd, win_ht, x_win, y_win, temp;
    unsigned long gcv_mask;
    XSetWindowAttributes xswa;
    XWindowAttributes xwa_main;
    XGCValues gcv;
    XWMHints xwmhints;

    (void) sprintf(fract,"Winfract: %.3f", winfract);
    x_max = xw;
    y_max = yw;
    if (sideways_fl != 0) {
      temp = xw; xw = yw; yw = temp;
    }

    fact_x = x_dpi*winfract/3600.0;
    fact_y = y_dpi*winfract/3600.0;
    pix_wd = xw*fact_x;
    pix_ht = yw*fact_y;
    if ((pix_wd <= 1) || (pix_ht <= 1)) {
      (void) fprintf(stderr,
          "sp_x: output size is (way!) too small to display.\n");
      (void) fprintf(stderr,
          "      Check the WINFRACT or HASIZE/VASIZE settings.\n");
      exit(1);
    }

    if (scr_open == 0) {
      win_wd = pix_wd+CHARWIDTH*txt_wd+bord_w;
      win_ht = pix_ht;
      if (win_ht < (CHARHEIGHT*txt_ht)) win_ht = CHARHEIGHT*txt_ht;
      if (win_wd > (sc_wd_pix-20)) win_wd = sc_wd_pix-20;
      if (win_ht > (sc_ht_pix-20)) win_ht = sc_ht_pix-20;
      x_win = sc_wd_pix/2-win_wd/2;
      y_win = sc_ht_pix/2-win_ht/2;
      if (opt_names[O_GEOM] != (char *) NULL) {
        temp = XParseGeometry(opt_names[O_GEOM], &x_win, &y_win, &win_wd,
                 &win_ht);
        if (temp & XNegative) x_win += sc_wd_pix-win_wd;
        if (temp & YNegative) y_win += sc_ht_pix-win_ht;
      }

      xswa.background_pixel = colours[0];
      xswa.border_pixel = colours[1];
      xswa.colormap = X_cmap;

      X_main_wind = XCreateWindow(X_display, X_root, x_win, y_win, win_wd,
           win_ht, bord_w, X_info->depth, InputOutput, X_info->visual,
           CWBackPixel|CWBorderPixel|CWColormap, &xswa);
      if (opt_names[O_TITLE] == (char *) NULL) {
        ptr = my_name;
      } else {
        ptr = opt_names[O_TITLE];
      }
      XChangeProperty(X_display, X_main_wind, XA_WM_NAME, XA_STRING, 8,
	  PropModeReplace, ptr, strlen(ptr));
      xwmhints.flags = IconPixmapHint;
      xwmhints.icon_pixmap = X_icon;
      XSetWMHints(X_display, X_main_wind, &xwmhints);

      if (anim_fl == 0)
      {
        X_print_wind = XCreateWindow(X_display, X_root, x_win, y_win, 
           PRINTWIDTH*txt_wd, PRINTHEIGHT*txt_ht, 
           bord_w, X_info->depth, InputOutput, X_info->visual,
           CWBackPixel|CWBorderPixel|CWColormap, &xswa);
        ptr = "sPLOTch! Print";
        XChangeProperty(X_display, X_print_wind, XA_WM_NAME, XA_STRING, 8,
	    PropModeReplace, ptr, strlen(ptr));
        xwmhints.flags = IconPixmapHint;
        xwmhints.icon_pixmap = X_icon;
        XSetWMHints(X_display, X_print_wind, &xwmhints);
        XSelectInput(X_display, X_print_wind, ExposureMask);

        X_prOK_wind = XCreateSimpleWindow(X_display, X_print_wind,
               (int) ((PRINTWIDTH/3-3.5)*txt_wd-bord_w),
               (int) ((PRINTHEIGHT-2)*txt_ht-bord_w),
               (unsigned int) (7*txt_wd), (unsigned int) (1.5*txt_ht),
               bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_prOK_wind, ExposureMask|ButtonPressMask);
        X_prCanc_wind = XCreateSimpleWindow(X_display, X_print_wind,
               (int) ((2*PRINTWIDTH/3-3.5)*txt_wd-bord_w),
               (int) ((PRINTHEIGHT-2)*txt_ht-bord_w),
               (unsigned int) (7*txt_wd), (unsigned int) (1.5*txt_ht),
               bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_prCanc_wind, ExposureMask|ButtonPressMask);

        X_prText_wind = XCreateSimpleWindow(X_display, X_print_wind,
               (int) bord_w, (int) ((PRINTHEIGHT-4.75)*txt_ht-bord_w),
               (unsigned int) (PRINTWIDTH*txt_wd - 3*bord_w), 
               (unsigned int) (1.5*txt_ht),
               bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_prText_wind, ExposureMask|KeyPressMask);
      }

      /* X_protocol = XInternAtom(X_display, "WM_PROTOCOLS", False);
      X_kill = XInternAtom(X_display, "WM_DELETE_WINDOW", False);
      XSetWMProtocols(X_display, X_main_wind, &X_kill, 1); */

      XSelectInput(X_display, X_main_wind, StructureNotifyMask);
      XDefineCursor(X_display, X_main_wind, X_pnl_curs);
      XMapRaised(X_display, X_main_wind);
      XFlush(X_display);
      XGetWindowAttributes(X_display, X_main_wind, &xwa_main);
      main_wd = xwa_main.width;
      main_ht = xwa_main.height;

      X_graph_wind = XCreateSimpleWindow(X_display, X_main_wind,
           CHARWIDTH*txt_wd, -bord_w, (unsigned int) 10, (unsigned int) 10, 
           bord_w, colours[1], colours[0]);
      if (desk_flag >= 0) {
        xswa.background_pixmap = X_desk;
        XChangeWindowAttributes(X_display, X_graph_wind, CWBackPixmap, &xswa);
      }
      resizeGraphWin();
      XSelectInput(X_display, X_graph_wind,
          ExposureMask|PointerMotionMask|ButtonMotionMask|ButtonPressMask|
          EnterWindowMask|LeaveWindowMask|ButtonReleaseMask);

      X_pnl_wind = XCreateSimpleWindow(X_display, X_main_wind,
             -bord_w, -bord_w, (unsigned int) CHARWIDTH*txt_wd, 
             (unsigned int) CHARHEIGHT*txt_ht,
             bord_w, colours[1], colours[0]);
      XSelectInput(X_display, X_pnl_wind, ExposureMask);
      XDefineCursor(X_display, X_pnl_wind, X_pnl_curs);

      X_pos_wind = XCreateSimpleWindow(X_display, X_pnl_wind,
             (int) (1.5*txt_wd-bord_w), (int) (9.5*txt_ht-bord_w), 
             (unsigned int) ((CHARWIDTH-3)*txt_wd), (unsigned int) (8*txt_ht),
              bord_w, colours[1], colours[0]);
      XSelectInput(X_display, X_pos_wind, ExposureMask);
      XDefineCursor(X_display, X_pos_wind, X_pnl_curs);
      
      x0 = (CHARWIDTH/2-3)*txt_wd-bord_w;
      if (anim_fl == 0)
      {
        x0 = (CHARWIDTH/2-7)*txt_wd-bord_w;
        X_prbutt_wind = XCreateSimpleWindow(X_display, X_pnl_wind,
               (int) ((CHARWIDTH/2+1)*txt_wd),
               (int) ((CHARHEIGHT-3)*txt_ht-bord_w),
               (unsigned int) (6*txt_wd), (unsigned int) (1.5*txt_ht),
               bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_prbutt_wind, ExposureMask|ButtonPressMask);
        XDefineCursor(X_display, X_prbutt_wind, X_pnl_curs);
      }
      X_but_wind = XCreateSimpleWindow(X_display, X_pnl_wind,
             x0, (int) ((CHARHEIGHT-3)*txt_ht-bord_w),
             (unsigned int) (6*txt_wd), (unsigned int) (1.5*txt_ht),
              bord_w, colours[1], colours[0]);
      XSelectInput(X_display, X_but_wind, ExposureMask|ButtonPressMask);
      XDefineCursor(X_display, X_but_wind, X_pnl_curs);

      X_work_wind = XCreateSimpleWindow(X_display, X_pnl_wind,
             (int) ((CHARWIDTH/2-6)*txt_wd-1), 
             (int) (6*txt_ht-1),
             (unsigned int) (12*txt_wd), (unsigned int) (1.5*txt_ht),
              1, colours[1], colours[0]);
      XSelectInput(X_display, X_work_wind, ExposureMask);
      XDefineCursor(X_display, X_work_wind, X_pnl_curs);

      if (anim_fl != 0) {
        X_anim_wind = XCreateSimpleWindow(X_display, X_main_wind,
             -bord_w, ((CHARHEIGHT+2)*txt_ht-bord_w), 
             (unsigned int) CHARWIDTH*txt_wd, 
             (unsigned int) ANIMHEIGHT*txt_ht,
             bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_anim_wind, ExposureMask);
        XDefineCursor(X_display, X_anim_wind, X_pnl_curs);

        X_onoff_wind = XCreateSimpleWindow(X_display, X_anim_wind,
               (int) (2*txt_wd-bord_w), (int) (3.5*txt_ht-bord_w),
               (unsigned int) (7*txt_wd), (unsigned int) (1.5*txt_ht),
                bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_onoff_wind, ExposureMask|ButtonPressMask);
        XDefineCursor(X_display, X_onoff_wind, X_pnl_curs);

        X_frame_wind = XCreateSimpleWindow(X_display, X_anim_wind,
               (int) (13*txt_wd-2*bord_w), (int) (3.5*txt_ht-bord_w),
               (unsigned int) (15*txt_wd), (unsigned int) (1.5*txt_ht),
                bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_frame_wind, ExposureMask|ButtonPressMask);
        XDefineCursor(X_display, X_frame_wind, X_pnl_curs);

        X_fast_wind = XCreateSimpleWindow(X_display, X_anim_wind,
               (int) (2*txt_wd-bord_w), (int) (7*txt_ht-bord_w),
               (unsigned int) (11*txt_wd), (unsigned int) (1.5*txt_ht),
                bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_fast_wind, ExposureMask|ButtonPressMask);
        XDefineCursor(X_display, X_fast_wind, X_pnl_curs);

        X_slow_wind = XCreateSimpleWindow(X_display, X_anim_wind,
               (int) (17*txt_wd-2*bord_w), (int) (7*txt_ht-bord_w),
               (unsigned int) (11*txt_wd), (unsigned int) (1.5*txt_ht),
                bord_w, colours[1], colours[0]);
        XSelectInput(X_display, X_slow_wind, ExposureMask|ButtonPressMask);
        XDefineCursor(X_display, X_slow_wind, X_pnl_curs);
      }

      gcv.foreground = colours[3];
      gcv.background = colours[0];
      gcv_mask = GCForeground|GCBackground;

      X_gc_ch = XCreateGC(X_display, X_graph_wind, gcv_mask, &gcv);
      X_gc_colour = XCreateGC(X_display, X_graph_wind, gcv_mask, &gcv);
      X_gc_shade_grey = XCreateGC(X_display, X_shade_grey, gcv_mask, &gcv);
      X_gc_shade_col = XCreateGC(X_display, X_shade_col, gcv_mask, &gcv);

      X_gc_text = XCreateGC(X_display, X_pnl_wind, gcv_mask, &gcv);
      XSetFont(X_display, X_gc_text, X_font->fid);

      XMapRaised(X_display, X_graph_wind);
      XMapRaised(X_display, X_pnl_wind);
      XMapSubwindows(X_display, X_pnl_wind);
      if (anim_fl != 0) {
        XMapRaised(X_display, X_anim_wind);
        XMapSubwindows(X_display, X_anim_wind);
      }
    } else {
      if (anim_fl == 0) {
        XFreePixmap(X_display, X_pixmap);
      }
    }

    if (interactive == 0) XDefineCursor(X_display, X_graph_wind, X_watch_curs);
    else XDefineCursor(X_display, X_graph_wind, X_gr_curs);

    X_pixmap = XCreatePixmap(X_display, X_graph_wind, pix_wd, 
          pix_ht, X_info->depth);
    if (anim_fl != 0) {
      X_animaps[n_animate++] = X_pixmap;
      draw_frame();
      if (n_animate == 100) {
        (void) fprintf(stderr,"sp_x: too many animation frames (100 max).\n");
        (void) exit(1);
      }
    }

    oversize = 0;
    if (pix_wd < graph_wd) oversize |= 1;
    if (pix_ht < graph_ht) oversize |= 2;

    XSetState(X_display, X_gc_ch, white_pixel, white_pixel, GXcopy, AllPlanes);
    XFillRectangle(X_display, X_pixmap, X_gc_ch, 0, 0, pix_wd, pix_ht);
    XSetState(X_display, X_gc_ch, (unsigned long) ~0L,  0L, GXxor, AllPlanes);

    draw_button(2);
    scr_open = 1;
    work_status =  -2;
    XFlush(X_display);
    inc_work(0);
}

void resizeGraphWin() {
    graph_wd = main_wd-CHARWIDTH*txt_wd-bord_w;
    graph_ht = main_ht;
    if ((graph_wd < 1) || (graph_ht < (CHARHEIGHT*txt_ht))) {
      (void) fprintf(stderr,
             "sp_x : Warning - main window size should be larger - please resize it.\n");
    }

    XResizeWindow(X_display, X_graph_wind, 
           (unsigned int) ((graph_wd<10) ? 10 : graph_wd), 
           (unsigned int) graph_ht);

    oversize = 0;
    if (pix_wd < graph_wd) oversize |= 1;
    if (pix_ht < graph_ht) oversize |= 2;

    if (scr_open != 0) drawPage(del_x, del_y, 0);
}

void closeWins() {
    int i;

    if (scr_open == 0) return;

    if (anim_fl == 0) {
      XFreePixmap(X_display, X_pixmap);
    } else {
      for (i = 0; i < n_animate; i++) XFreePixmap(X_display, X_animaps[i]);
      n_animate = 0;
    }
    XFreeGC(X_display, X_gc_text);
    XFreeGC(X_display, X_gc_colour);
    XFreeGC(X_display, X_gc_shade_grey);
    XFreeGC(X_display, X_gc_shade_col);
    XFreeGC(X_display, X_gc_ch);
    XDestroyWindow(X_display, X_work_wind);
    XDestroyWindow(X_display, X_but_wind);
    XDestroyWindow(X_display, X_pos_wind);
    XDestroyWindow(X_display, X_pnl_wind);
    XDestroyWindow(X_display, X_graph_wind);
    XDestroyWindow(X_display, X_main_wind);
}

void x_close_screen() {
    x_proc_events(1);
    closeWins();
    XFlush(X_display);
    scr_open = 0;
}

void x_update() {
    if (scr_open != 0) {
        drawPage(0, 0, 1);
        inc_work(1);
    }
}

void x_move(COORD xp, COORD yp) {
    testScreen();
    convPoint(&x_ox, &x_oy, xp, yp);
}

void x_draw(COORD xp, COORD yp) {
    COORD x,y;
    XSegment line;

    testScreen();
    convPoint(&x, &y, xp, yp);

    line.x1 = x_ox; line.y1 = x_oy;
    line.x2 = x; line.y2 = y;
    
    XDrawSegments(X_display, X_pixmap, X_gc_colour, &line, 1);

    x_ox = x;
    x_oy = y;

    inc_work(0);
}

void x_poly(point *poly, int nPoints) {
    XPoint *X_poly;
    COORD x, y;
    int i; 

    X_poly = (XPoint *) malloc((unsigned int) (nPoints+2)*sizeof(XPoint));
    if (X_poly == (XPoint *) NULL) {
      (void) fprintf(stderr,"sp_x: unable to allocate XPoint structure.\n");
      (void) exit(1);
    }

    for (i = 0; i <= nPoints; i++) {
      convPoint(&x, &y, (poly+i)->x, (poly+i)->y);
      (X_poly+i)->x = x;
      (X_poly+i)->y = y;
    }

    XFillPolygon(X_display, X_pixmap, X_gc_colour, X_poly, nPoints,
           Complex,CoordModeOrigin);

    inc_work(0);
}

void convPoint(COORD *x, COORD *y, COORD xp, COORD yp) {
    if (sideways_fl == 0) {
        *x = xp*fact_x;
        *y = (y_max-yp)*fact_y;
    } else { 
        *x = (y_max-yp)*fact_x;
        *y = (x_max-xp)*fact_y;
    }
}

void x_width(COORD x) {
    int wd;

    testScreen();
    wd = x*fact_y;
    XSetLineAttributes(X_display, X_gc_colour, wd, LineSolid, 
       CapRound, JoinRound);
}

void x_colour(float hue, float sat, float bright) {
   float red, green, blue, grey_scale();
   int i, j, tile_num, sh_main, rem, ri, bi, gi, t_grey;
   unsigned long colour, fore_col, back_col, cube[8];
   
   testScreen();

   if ((mono_fl != 0) || (grey_fl != 0)) {
      hsb_to_rgb(hue, sat, bright, &red, &green, &blue);
      hue = sat = 0.0;
      bright = grey_scale(red, green, blue);
   }

   if (bright == 0.0) {
     XSetState(X_display, X_gc_colour, black_pixel, white_pixel, GXcopy,
        AllPlanes);
     XSetFillStyle(X_display, X_gc_colour, FillSolid);
   }else if (bright == 1.0) {
     XSetState(X_display, X_gc_colour, white_pixel, white_pixel, GXcopy,
        AllPlanes);
     XSetFillStyle(X_display, X_gc_colour, FillSolid);
   }else if (sat == 0.0) {
     tile_num = bright*num_dither_grey+0.5;
     fore_col = black_pixel;
     back_col = white_pixel;
     t_grey = bright*grey_max;
     if (t_grey == grey_max) t_grey--;
     if (mono_fl == 0) {
       if (has_std_cmap != 0) {
         if (has_colour == 0) {
           tile_num = (bright*grey_max-t_grey)*num_dither_grey+0.5;
           fore_col = X_stdcmap->base_pixel+t_grey*X_stdcmap->red_mult;
           back_col = fore_col+X_stdcmap->red_mult;
         } else {
           if (grey_max != 0) {
             tile_num = (bright*grey_max-t_grey)*num_dither_grey+0.5;
             fore_col = X_stdcmap->base_pixel+t_grey*X_stdcmap->red_mult+
                      t_grey*X_stdcmap->blue_mult+t_grey*X_stdcmap->green_mult;
             back_col = fore_col+X_stdcmap->red_mult+X_stdcmap->blue_mult+
                      X_stdcmap->green_mult;
           }
         }
       } else {
         if (grey_fl != 0) {
           tile_num = (bright*grey_max-t_grey)*num_dither_grey+0.5;
           fore_col = primaries[t_grey];
           back_col = primaries[t_grey+1];
         }
       }
     }
     if ((tile_num < 0) || (tile_num > num_dither_grey)) {
       (void) fprintf(stderr,"sp_x: bad colour specs..tell Jeff.\n");
       (void) fprintf(stderr,"Hue %f Sat %f Bright %f\n", hue, sat, bright);
       (void) exit(1);
     }
     if (num_dither_grey == 0) {
       XSetState(X_display, X_gc_colour, fore_col, fore_col, GXcopy,
          AllPlanes);
       XSetFillStyle(X_display, X_gc_colour, FillSolid);
     }else if (num_dither_grey == 16) {
       XCopyArea(X_display, X_shades[tile_num], X_shade_grey,
          X_gc_shade_grey, 0, 0, 4, 4, 0, 0);
       XSetState(X_display, X_gc_colour, fore_col, back_col, GXcopy,
           AllPlanes);
       XSetFillStyle(X_display, X_gc_colour, FillOpaqueStippled);
       XSetStipple(X_display, X_gc_colour, X_shade_grey); 
     } else {
       sh_main = tile_num/4;
       rem = tile_num-sh_main*4;

       XCopyArea(X_display, X_shades[sh_main+((rem>0) ? 1 : 0)], X_shade_grey,
          X_gc_shade_grey, 0, 0, 4, 4, 0, 0);
       XCopyArea(X_display, X_shades[sh_main+((rem>1) ? 1 : 0)], X_shade_grey,
          X_gc_shade_grey, 0, 0, 4, 4, 4, 4);
       XCopyArea(X_display, X_shades[sh_main+((rem>2) ? 1 : 0)], X_shade_grey,
          X_gc_shade_grey, 0, 0, 4, 4, 4, 0);
       XCopyArea(X_display, X_shades[sh_main+((rem>3) ? 1 : 0)], X_shade_grey,
          X_gc_shade_grey, 0, 0, 4, 4, 0, 4); 
       XSetState(X_display, X_gc_colour, fore_col, back_col, GXcopy,
           AllPlanes);
       XSetFillStyle(X_display, X_gc_colour, FillOpaqueStippled);
       XSetStipple(X_display, X_gc_colour, X_shade_grey); 
     }
   } else {
     hsb_to_rgb(hue, sat, bright, &red, &green, &blue);
     if (has_std_cmap != 0) {
       red = red*X_stdcmap->red_max;
       ri = red; if (ri == X_stdcmap->red_max) ri--;
       red = red-ri;
       blue = blue*X_stdcmap->blue_max;
       bi = blue; if (bi == X_stdcmap->blue_max) bi--;
       blue = blue-bi;
       green = green*X_stdcmap->green_max;
       gi = green; if (gi == X_stdcmap->green_max) gi--;
       green = green-gi;

       for (i = 0; i < 8; i++) {
         colour = X_stdcmap->base_pixel+
           (ri+(((i & 1) != 0) ? 1 : 0))*X_stdcmap->red_mult+
	  (bi+(((i & 2) != 0) ? 1 : 0))*X_stdcmap->blue_mult+
           (gi+(((i & 4) != 0) ? 1 : 0))*X_stdcmap->green_mult;
         cube[i] = colour;
       }
     } else {
       for (i = 0; i < 8; i++) {
         cube[i] = primaries[i];
       }
     }
       
     if (num_dither_col == 64) {
       ri = red*64+0.5; bi = blue*64+0.5; gi = green*64+0.5;
       for (i = 0; i < 8; i++) {
         for (j = 0; j < 8; j++) {
           colour = cube[(((patterns[ri][i]&rotates[j]) != 0) ? 0 : 1)+
                       (((patterns[bi][i]&rotates[j]) != 0) ? 0 : 2)+
                       (((patterns[gi][i]&rotates[j]) != 0) ? 0 : 4)];
           XPutPixel(X_col_image, j, i, colour);
         }
       }
       XPutImage(X_display, X_shade_col, X_gc_shade_col, X_col_image, 0, 0,
          0, 0, 8, 8);
       XSetFillStyle(X_display, X_gc_colour, FillTiled);
       XSetTile(X_display, X_gc_colour, X_shade_col); 
     }else if (num_dither_col == 16) {
       ri = red*16+0.5; bi = blue*16+0.5; gi = green*16+0.5;
       for (i = 0; i < 4; i++) {
         for (j = 0; j < 4; j++) {
           colour = cube[(((shade_bits[ri][i]&rotates[j]) != 0) ? 0 : 1)+
                       (((shade_bits[bi][i]&rotates[j]) != 0) ? 0 : 2)+
                       (((shade_bits[gi][i]&rotates[j]) != 0) ? 0 : 4)];
           XPutPixel(X_col_image, j, i, colour);
         }
       }
       XPutImage(X_display, X_shade_col, X_gc_shade_col, X_col_image, 0, 0,
          0, 0, 4, 4);
       XSetFillStyle(X_display, X_gc_colour, FillTiled);
       XSetTile(X_display, X_gc_colour, X_shade_col); 
     } else {
       ri = red+0.5; bi = blue+0.5; gi = green+0.5;
       colour = cube[ri+bi*2+gi*4];
       XSetState(X_display, X_gc_colour, colour, white_pixel, GXcopy,
         AllPlanes);
       XSetFillStyle(X_display, X_gc_colour, FillSolid);
     }
   }
}

#define RGB
#include "../common.c"

int frame_no, on_off = 1;
unsigned int anim_speed = 100000;
char print_comm[50] = DEF_PRCOMM;
int cursor_pos;

void x_proc_events(int status) {
    int quitFlag, rc;
    fd_set fds;
    struct timeval *time_ptr, delay;

    frame_no = 0;
    XDefineCursor(X_display, X_graph_wind, X_gr_curs);
    XBell(X_display, 100);
    draw_button(status);
    work_status = 0;
    draw_status();
    x_update();
    XFlush(X_display);

    if (scr_open != 0) {
       quitFlag = 0;

       while (quitFlag == 0) {
         while (XEventsQueued(X_display, QueuedAfterReading) == 0) {
           FD_ZERO(&fds);
           FD_SET(X_main_fd, &fds);
           if (anim_fl == 0) time_ptr = (struct timeval *) NULL;
           else{
             time_ptr = &delay;
             delay.tv_sec = 0;
             delay.tv_usec = anim_speed;
           }

           rc = select((X_main_fd+1), (fd_set *) &fds, (fd_set *) NULL, 
               (fd_set *) NULL, (struct timeval *) time_ptr);

           if ((rc == 0) && (anim_fl != 0) && (on_off != 0)) nextFrame();
         }
         doEvents(&quitFlag, status);
       }
    }
}

void nextFrame() {
   frame_no++;
   if (frame_no == n_animate) frame_no = 0;
   drawPage(0, 0, 1);
   draw_frame();
   XSync(X_display, False);
}

void doEvents(int *quitFlag, int status) {
    XEvent X_evr;
    static char *label = "Animation Control";
    static char *prlabel = "Printer command:";
    int i, x, y;
    KeySym x_key_sym;
    char key_buff[3];

    if (scr_open != 0) {
      while(XEventsQueued(X_display, QueuedAfterReading) != 0) {
          XNextEvent(X_display, &X_evr);
          if ((status == 2) && (X_evr.type != Expose) && 
              (X_evr.type != ButtonPress) &&(X_evr.type != ConfigureNotify) &&
              (interactive == 0)) continue;
          switch (X_evr.type) {
           case Expose:       
               while(XCheckTypedWindowEvent(X_display, X_evr.xexpose.window,
                  Expose, &X_evr));
               if (X_evr.xexpose.window == X_but_wind) {
                 draw_button(status);
               }else if (X_evr.xexpose.window == X_work_wind) {
                 draw_status();
               }else if (X_evr.xexpose.window == X_pos_wind) {
                 drawPosition(0, 0, 1);
               }else if (X_evr.xexpose.window == X_pnl_wind) {
                 XClearWindow(X_display, X_pnl_wind);
                 XDrawString(X_display, X_pnl_wind, X_gc_text,
                   (int) (2*txt_wd), (int) (2*txt_ht+txt_offset),
                   f_name, strlen(f_name));
                 XDrawString(X_display, X_pnl_wind, X_gc_text,
                   (int) (2*txt_wd), (int) (3*txt_ht+txt_offset),
                   ver_name, strlen(ver_name));
                 XDrawString(X_display, X_pnl_wind, X_gc_text,
                   (int) (2*txt_wd), (int) (4*txt_ht+txt_offset),
                   fract, strlen(fract));
               }else if (X_evr.xexpose.window == X_graph_wind) {
                 if ((status != 2) || (interactive != 0)) drawPage(0, 0, 1);
               }else if (X_evr.xexpose.window == X_anim_wind) {
                 XClearWindow(X_display, X_anim_wind);
                 XDrawString(X_display, X_anim_wind, X_gc_text,
                       (int) (2*txt_wd), (int) (1.5*txt_ht+txt_offset),
                       label, strlen(label));
               }else if (X_evr.xexpose.window == X_onoff_wind) {
                 draw_on_off(on_off);
               }else if (X_evr.xexpose.window == X_frame_wind) {
                 draw_frame();
               }else if (X_evr.xexpose.window == X_fast_wind) {
                 x_out_text("Faster", X_fast_wind, 11);
               }else if (X_evr.xexpose.window == X_slow_wind) {
                 x_out_text("Slower", X_slow_wind, 11);
               }else if (X_evr.xexpose.window == X_prbutt_wind) {
                 x_out_text("Print", X_prbutt_wind, 6);
               }else if (X_evr.xexpose.window == X_prOK_wind) {
                 x_out_text("OK", X_prOK_wind, 7);
               }else if (X_evr.xexpose.window == X_prCanc_wind) {
                 x_out_text("Cancel", X_prCanc_wind, 7);
               }else if (X_evr.xexpose.window == X_prText_wind) {
                 XClearWindow(X_display, X_prText_wind);
                 XDrawString(X_display, X_prText_wind, X_gc_text,
                    (int) (txt_wd/2), (int) (0.75*txt_ht+txt_offset),
                    print_comm, strlen(print_comm));
                 x = XTextWidth(X_font, print_comm, cursor_pos);
                 XDrawRectangle(X_display, X_prText_wind, X_gc_text,
                    (int) (x+txt_wd/2-1), (int) (txt_ht/4), txt_wd, txt_ht);
               }else if (X_evr.xexpose.window == X_print_wind) {
                 XClearWindow(X_display, X_print_wind);
                 XDrawString(X_display, X_print_wind, X_gc_text,
                       (int) (1*txt_wd), (int) (1*txt_ht+txt_offset),
                       prlabel, strlen(prlabel));
               }
               break;
          case KeyPress:
               if (X_evr.xkey.window == X_prText_wind) {
                 XLookupString((XKeyEvent *) &X_evr, key_buff, 1, 
                               &x_key_sym, NULL);
                 if (!IsModifierKey(x_key_sym)) {
                   if ((x_key_sym == XK_Return) || (x_key_sym == XK_KP_Enter)||
                       (x_key_sym == XK_Linefeed)) {
                     do_print(print_comm);
                     XUnmapWindow(X_display, X_print_wind);
                     break;
                   }
                   if ((x_key_sym == XK_Left) && 
                       (cursor_pos > 0)) {
                     cursor_pos--;
                   } else if ((x_key_sym == XK_Right) &&
                            (cursor_pos < strlen(print_comm))) {
                     cursor_pos++;
                   } else if ((x_key_sym == XK_BackSpace) &&
                            (cursor_pos > 0)) {
                     cursor_pos--;
                     for (i = cursor_pos; i < strlen(print_comm); i++)
                     {
                       print_comm[i] = print_comm[i+1];
                     }
                   } else if ((x_key_sym == XK_Delete) &&
                            (cursor_pos < strlen(print_comm))) {
                     for (i = cursor_pos; i < strlen(print_comm); i++)
                     {
                       print_comm[i] = print_comm[i+1];
                     }
                   } else if ((x_key_sym == XK_Delete) &&
                            (cursor_pos == strlen(print_comm)) &&
                            (cursor_pos != 0)) {
                     cursor_pos--;
                     print_comm[cursor_pos] = '\0';
                   } else if ((((x_key_sym >= XK_KP_Space) &&
                                (x_key_sym <= XK_KP_9)) ||
                               ((x_key_sym >= XK_space) &&
                                (x_key_sym <= XK_asciitilde))) &&
                              (strlen(print_comm) < PRINTWIDTH)) {
                     for (i = strlen(print_comm); i >= cursor_pos; i--)
                     {
                       print_comm[i+1] = print_comm[i];
                     }
                     print_comm[cursor_pos++] = key_buff[0];
                   } else {
                     XBell(X_display, 100);
                   }
                 }

                 XClearWindow(X_display, X_prText_wind);
                 XDrawString(X_display, X_prText_wind, X_gc_text,
                    (int) (txt_wd/2), (int) (0.75*txt_ht+txt_offset),
                    print_comm, strlen(print_comm));
                 x = XTextWidth(X_font, print_comm, cursor_pos);
                 XDrawRectangle(X_display, X_prText_wind, X_gc_text,
                    (int) (x+txt_wd/2-1), (int) (txt_ht/4), txt_wd, txt_ht);
               }
               break;
           case ButtonPress:
               if (X_evr.xbutton.window == X_but_wind) {
                 if ((interactive == 0) || (status != 2)) *quitFlag = 1;
                 else XBell(X_display, 100);
               }else if ((status != 2) && 
                         (X_evr.xbutton.window == X_graph_wind)) {
                 work_status = 1;
                 draw_status();
                 grab_x = X_evr.xbutton.x-del_x;
                 grab_y = X_evr.xbutton.y-del_y;
               }else if ((status != 2) && (anim_fl != 0)) {
                 if (X_evr.xbutton.window == X_onoff_wind) {
                   on_off = 1-on_off;
                   draw_on_off(on_off);
                 }else if (X_evr.xbutton.window == X_frame_wind) {
                   if (X_evr.xbutton.button == Button1) {
                     frame_no -= 2;
                     if (frame_no < 0) frame_no += n_animate;
                     nextFrame();
                   }else if (X_evr.xbutton.button == Button3) {
                     nextFrame();
                   }
                 }else if (X_evr.xbutton.window == X_fast_wind) {
                   anim_speed = anim_speed/1.2;
                   if (anim_speed < 10) anim_speed = 10;
                 }else if (X_evr.xbutton.window == X_slow_wind) {
                   anim_speed = anim_speed*1.2;
                 }
               }else if (status !=2) {
                 if (X_evr.xbutton.window == X_prbutt_wind) {
                   XMapRaised(X_display, X_print_wind);
                   XMapSubwindows(X_display, X_print_wind);
                   cursor_pos = strlen(print_comm);
                 }else if (X_evr.xbutton.window == X_prCanc_wind) {
                   XUnmapWindow(X_display, X_print_wind);
                 }else if (X_evr.xbutton.window == X_prOK_wind) {
                   do_print(print_comm);
                   XUnmapWindow(X_display, X_print_wind);
                 }
               }
               break;
           case ButtonRelease:
               if (X_evr.xbutton.window == X_graph_wind) {
                 work_status = 0;
                 draw_status();
               }
               break;
           case EnterNotify:
               if (X_evr.xcrossing.window != X_graph_wind) break;
               m_ox = X_evr.xmotion.x;
               m_oy = X_evr.xmotion.y;
               draw_cross(m_ox, m_oy);
               drawPosition(m_ox, m_oy, 0);
               break;
           case MotionNotify: 
               while(XCheckTypedWindowEvent(X_display, X_evr.xconfigure.window,
                  MotionNotify, &X_evr));
               if (X_evr.xmotion.window != X_graph_wind) break;
               if (m_ox != -1) draw_cross(m_ox,m_oy);
               m_ox = X_evr.xmotion.x;
               m_oy = X_evr.xmotion.y;
               if (work_status == 1) {
                 x = X_evr.xmotion.x-grab_x;
                 y = X_evr.xmotion.y-grab_y;
                 drawPage(x, y, 0);
               } else {
                 draw_cross(m_ox,m_oy);
                 drawPosition(m_ox,m_oy,0);
               }
               break;
           case LeaveNotify:
               if (X_evr.xcrossing.window != X_graph_wind) break;
               draw_cross(m_ox,m_oy);
               m_ox = m_oy =  -1;
               drawPosition(m_ox,m_oy,0);
               if (work_status == 1) {
                 work_status = 0;
                 draw_status();
               }
               break;
           case DestroyNotify:
               if ((X_evr.xdestroywindow.window == X_main_wind)) {
                 closeWins();
                 close_display();
                 (void) exit(0);
               }
               break;
           case ConfigureNotify:
               while(XCheckTypedWindowEvent(X_display, X_evr.xconfigure.window,
                  ConfigureNotify, &X_evr));
               if ((X_evr.xconfigure.window != X_main_wind)||
                   ((X_evr.xconfigure.width == main_wd) &&
                     ((X_evr.xconfigure.height == main_ht)))) break;
               main_wd = X_evr.xconfigure.width;
               main_ht = X_evr.xconfigure.height;
               resizeGraphWin();
               break;
        /* case ClientMessage:
         *      if ((X_evr.xclient.message_type == X_protocol)&&
         *           (X_evr.xclient.data.l[0] == X_kill)) {
         *        closeWins();
         *        close_display();
         *        (void) exit(0);
         *      }
         *      break; */
          }
          XFlush(X_display);
          if (*quitFlag != 0) break;
       }
    } else {
      while(XEventsQueued(X_display, QueuedAfterReading) != 0) {
          XNextEvent(X_display, &X_evr);
          XFlush(X_display);
      }
    }
}

void drawPage(int x, int y, int flag) {
     if ((oversize & 1) != 0) {
       if (x < 0) x = 0;
       if (x > (graph_wd-pix_wd)) x = graph_wd-pix_wd;
     } else {
       if (x > 0) x = 0;
       if (x < (graph_wd-pix_wd)) x = graph_wd-pix_wd;
     }
     if ((oversize & 2) != 0) {
       if (y < 0) y = 0;
       if (y > (graph_ht-pix_ht)) y = graph_ht-pix_ht;
     } else {
       if (y > 0) y = 0;
       if (y < (graph_ht-pix_ht)) y = graph_ht-pix_ht;
     }

     if ((x != del_x) || (y != del_y) || (flag != 0)) {
       if (flag == 0) {del_x = x; del_y = y;}
       if (anim_fl != 0) X_pixmap = X_animaps[frame_no];
       if (oversize != 0) {
         XClearWindow(X_display, X_graph_wind);
         XCopyArea(X_display, X_pixmap, X_graph_wind, X_gc_colour, 
             0, 0, pix_wd, pix_ht, del_x, del_y);
       } else {
         XCopyArea(X_display, X_pixmap, X_graph_wind, X_gc_colour, 
             -del_x, -del_y, graph_wd, graph_ht, 0, 0);
       }
     }
     draw_cross(m_ox, m_oy);
}

float x_pos = -1.0, y_pos = -1.0;

void drawPosition(int x, int y, int flag) {
    static char *pos_title = "Cursor position (apct)";
    char xbuff[100], ybuff[100];

    if (flag == 0) {
      if ((x < 0) || (y < 0) || (x > graph_wd) || (y > graph_ht)) x_pos =  -1.0;
      else{
        x -= del_x;
        y -= del_y;
        if ((x < 0) || (y < 0) || (x > pix_wd) || (y > pix_ht)) x_pos =  -1.0;
        else {
          if (sideways_fl == 0) {
            x_pos = x*100.0/pix_wd;
            y_pos = (pix_ht-y)*100.0/pix_ht;
          } else {
            x_pos = (pix_ht-y)*100.0/pix_ht;
            y_pos = (pix_wd-x)*100.0/pix_wd;
          }
        }
      }
    }

    XClearWindow(X_display, X_pos_wind);
    XDrawString(X_display, X_pos_wind, X_gc_text,
         (int) (2*txt_wd), (int) (2*txt_ht+txt_offset),
         pos_title, strlen(pos_title));

    if (x_pos < 0.0) {
      (void) sprintf(xbuff, "X: off page");
      (void) sprintf(ybuff, "Y: off page");
    } else {
      (void) sprintf(xbuff, "X: %.4f", x_pos);
      (void) sprintf(ybuff, "Y: %.4f", y_pos);
    }
    XDrawString(X_display, X_pos_wind, X_gc_text,
         (int) (4*txt_wd), (int) (4*txt_ht+txt_offset),
         xbuff, strlen(xbuff));
    XDrawString(X_display, X_pos_wind, X_gc_text,
         (int) (4*txt_wd), (int) (6*txt_ht+txt_offset),
         ybuff, strlen(ybuff));
}

void draw_status() {
    static char *msg_a = "Working", *msg_b = "Complete", *msg_c = "Dragging";
    char *ptr;
    int wd;

    XClearWindow(X_display, X_work_wind);

    if (work_status == -1) ptr = msg_a;
    else if (work_status == 0) ptr = msg_b;
    else ptr = msg_c;
    wd = XTextWidth(X_font, ptr, strlen(ptr));
    XDrawString(X_display, X_work_wind, X_gc_text,
      (int) (6*txt_wd-wd/2), (int) (0.75*txt_ht+txt_offset),
      ptr, strlen(ptr));

    if (work_status == -1) {
      XFillRectangle(X_display, X_work_wind, X_gc_text, 0, 0, 
         work_level, (int) (1.5*txt_ht));
    }
}

void draw_button(int status) {
    static char *msgs[] = {"Next", "Bye!", "Quit"};

    XClearWindow(X_display, X_but_wind);
    if ((interactive != 0) && (status == 2)) return;
    x_out_text(msgs[status], X_but_wind, 6);
}

void draw_on_off(int onoff) {
    static char *labels[] = {"Start", "Stop"};

    x_out_text(labels[onoff], X_onoff_wind, 7);
}

void draw_frame() {
    char buff[40];

    (void) sprintf(buff,"Frame: %i/%i", (frame_no+1), n_animate);
    XClearWindow(X_display, X_frame_wind);
    XDrawString(X_display, X_frame_wind, X_gc_text,
                (int) (1*txt_wd), (int) (0.75*txt_ht+txt_offset),
                buff, strlen(buff));
}

void x_out_text(char *str, Window win, int width) {
    int wd;

    XClearWindow(X_display, win);
    wd = XTextWidth(X_font, str, strlen(str));
    XDrawString(X_display, win, X_gc_text,
          (int) (width*txt_wd/2-wd/2), (int) (0.75*txt_ht+txt_offset),
          str, strlen(str));
}

int t_ct;

void inc_work(int flag) {
    int quitFlag;

    if (work_status == -2) {work_status = -1; work_level = 0; t_ct = -1;}
    if (((quiet_fl != 0) || (work_status != -1)) && (flag == 0)) return;

    t_ct++;
    if (((t_ct%25) != 0) && (flag == 0)) return;
    t_ct = 0;
    work_level++;
    if (work_level == (12*txt_wd)) work_level = 0;
    draw_status();

    quitFlag = 0;
    doEvents(&quitFlag, 2);
    if (quitFlag != 0) x_abort();

    XSync(X_display, False);
}

void draw_cross(int x, int y) {
    if (x < 0) return;

    XDrawLine(X_display, X_graph_wind, X_gc_ch, 0, y, graph_wd, y);
    XDrawLine(X_display, X_graph_wind, X_gc_ch, x, 0, x, graph_ht);
}

void x_abort() {
   (void) fprintf(stderr,"\nsp_x: Sorry.  Maybe next time will be better.\n\n");

   closeWins();
   close_display();
   exit(0);
}

/*
  *  Subroutines/Storage for buffering display pages for printing.
  */
int pr_buffer_size = 0, pr_buffer_pos = 0;
char *pr_buffer = (char *) NULL;

void init_print_buffer() {
    if (pr_buffer_size == 0) {
        pr_buffer = (char *) malloc((unsigned int) 2048);
        pr_buffer_size = 2048;
        pr_buffer_pos = 0;
    } else {
        pr_buffer_pos = 20;
    }
}

void rem_print_buffer() {
    if (pr_buffer_pos > 3) pr_buffer_pos -= 4;
}

void add_to_buffer(int codeA, int codeB, int codeC, int codeD) {
   if (pr_buffer_size != 0) {
     if (pr_buffer_pos >= (pr_buffer_size - 5)) {
       pr_buffer_size += 2048;
       pr_buffer = (char *) realloc(pr_buffer,
                                    (unsigned int) pr_buffer_size);
     }
     *(pr_buffer + pr_buffer_pos++) = codeA;
     *(pr_buffer + pr_buffer_pos++) = codeB;
     *(pr_buffer + pr_buffer_pos++) = codeC;
     *(pr_buffer + pr_buffer_pos++) = codeD;
   }
}

void do_print(char *print_comm) {
   FILE *fp;
   int i;

   fp = popen(print_comm, "w");
   if (fp == (FILE *) NULL) {
     (void) fprintf(stderr, "Unable to open print command: %s", print_comm);
     return;
   }

   if (pr_buffer != (char *) NULL) {
     for (i = 0; i < pr_buffer_pos; i++) {
       (void) fputc(*(pr_buffer + i), fp);
     }
   }

   (void) pclose(fp);
}

/*
  *  Debugging stuff
  */
#ifdef DEBUG
char *print_type(Atom prop) {
    char *ptr;

    switch(prop) {
        case XA_RGB_DEFAULT_MAP: ptr = "XA_RGB_DEFAULT_MAP";
                                 break;
        case XA_RGB_BEST_MAP   : ptr = "XA_RGB_BEST_MAP";
                                 break;
        case XA_RGB_GRAY_MAP   : ptr = "XA_RGB_GRAY_MAP";
                                 break;
        default                : ptr = "Hey....whats going on here?";
                                 break;
    }

    return ptr;
}

char *print_class(int class) {
    char *ptr;

    switch (class) {
        case PseudoColor: ptr = "PseudoColour";
                          break;
        case DirectColor: ptr = "DirectColour";
                          break;
        case GrayScale  : ptr = "GrayScale";
                          break;
        case StaticColor: ptr = "StaticColour";
                          break;
        case TrueColor  : ptr = "TrueColour";
                          break;
        case StaticGray : ptr = "StaticGray";
                          break;
        default         : ptr = "Hey....whats going on here?";
                          break;
    }

    return ptr;
}
#endif
