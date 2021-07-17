/*
*  Vector output driver for sPLOTch! Version 2.1
*/

#include "../localdef.h"
#include <stdio.h>
#include "../sdvi.h"

#define PLOT_PER_INCH 1016  /* number of hp plotting points per inch */

#ifndef PRINTER
#define PRINTER "/usr/ucb/lpr"
#endif
#ifndef DEF_DEV
#define DEF_DEV 0
#endif

int sideways_fl = 0, scr_open = 0, timeout, dev_type, use_stdin, all_esc;
int oxl, oxr, oyl, oyr, no_colour, eavesdrop, curr_colour;
COORD oxp, oyp;
char *names[] = {"tek", "hpgl"}, *pr_comm = PRINTER;
float t_fact, x_off, y_off;
FILE *out_file;

main(argc, argv)
int argc;
char *argv[];
{
   int c, comm, i, n;
   COORD x, y, xr, yr;
   int maj_ver, min_ver;
   float hue, sat, brt;
   char *in_filename, *out_filename, *pr_name, *ptr, *init_str;
   char com_buff[1000];
   FILE *sdvi_file;

   no_colour = eavesdrop = all_esc = use_stdin = 0;
   init_str = in_filename = out_filename = pr_name = (char *) NULL;
   x_off = y_off = 0.0;
   timeout = 30;
   dev_type = DEF_DEV;

   for (c = 1; c < argc; c++) {
     ptr = argv[c] + 2;
     if (*(argv[c]) == '-') {
       switch(*(argv[c]+1)) {
         case 'd' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    for (i = 0; i < 2; i++) {
                      if (strcmp(ptr, names[i]) == 0) {
                        dev_type = i;
                        break;
                      }
                    }
                    if (i == 2) (void) fprintf(stderr,
                       "sp_vec: invalid device specification :%s:\n", ptr);
                    break;
         case 'e' : all_esc = 1;
                    break;
         case 'h' : help();
                    break;
         case 'i' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    init_str = ptr;
                    break;
         case 'n' : no_colour = 1;
                    break;
         case 'o' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    out_filename = ptr;
                    break;
         case 'P' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    pr_name = ptr;
                    break;
         case 's' : sideways_fl = 1;
                    break;
         case 't' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    timeout = atoi(ptr);
                    break;
         case 'x' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    x_off = atof(ptr);
                    break;
         case 'y' : if ((*ptr == '\0')&&(argv[c+1] != (char *) NULL))
                      ptr = argv[++c];
                    y_off = atof(ptr);
                    break;
         case 'Y' : eavesdrop = 1;
                    break;
         default  : (void) fprintf(stderr,
                      "sp_vec: option not one of dehinoPstxyY.\n");
                    break;
       }
     }else{
       in_filename = argv[c];
     }
   }

   if (in_filename == (char *) NULL) {
     sdvi_file = stdin;
     use_stdin = 1;
   }else{
     sdvi_file = fopen(in_filename, "r");
     if (sdvi_file == (FILE *) NULL) {
       (void) fprintf(stderr,
           "sp_vec: unable to open sdvi file %s.\n", in_filename);
       (void) exit(1);
     }
   }

   x = readCoord(sdvi_file);
   if (feof(sdvi_file) != 0) (void) exit(0);

   if (x != SDVI_MAGIC) {
     (void) fprintf(stderr, "sp_vec: not a sPLOTch! sdvi file.\n");
     (void) fprintf(stderr, "Magic number %i instead of %i.\n", x, SDVI_MAGIC);
     (void) exit(1);
   }
   maj_ver = readCoord(sdvi_file);
   min_ver = readCoord(sdvi_file);
   (void) readCoord(sdvi_file);

   if (out_filename != (char *) NULL) {
     if (*out_filename == '-') {
       out_file = stdout;
     }else{
       out_file = fopen(out_filename, "w");
       if (out_file == (FILE *) NULL) {
         (void) fprintf(stderr,
            "sp_vec: cannot open output file %s.\n", out_filename);
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
             "sp_vec: cannot open printer pipe via %s.\n", com_buff);
         }
       }
     }else{   
       out_file = stdout;
     }
   }

   if ((dev_type == 1) && (eavesdrop != 0)) {
     (void) fprintf(out_file, "\033.Y");
   }
   if ((dev_type == 1) && (init_str != (char *) NULL)) {
     for (i = 0; i < strlen(init_str); i++) 
       if (*(init_str+i) == '#') *(init_str+i) = '\033';
     (void) fprintf(out_file, init_str);
   }
   curr_colour = -1;
   if ((dev_type == 1) && (no_colour != 0)) hpgl_change_colour(0.0, 0.0, 0.0);
  
   while (((comm = readCoord(sdvi_file)) != END_OF_FILE) && (feof(sdvi_file) == 0)) {
     switch(comm) {
       case OPEN_SCR:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           (void) readFloat(sdvi_file);
           if (dev_type == 0) tek_open_screen(x, y);
           else if (dev_type == 1) hpgl_open_screen(x, y);
           break;
       case UPD_SCR:
           if (dev_type == 0) (void) fflush(stdout);
           else if (dev_type == 1) (void) fflush(out_file);
           break;
       case CLOSE_SCR:
           break;
       case MOVE_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           if (dev_type == 0) tek_move(x, y);
           else if (dev_type == 1) hpgl_move(x, y);
           break;
       case DRAW_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           if (dev_type == 0) tek_draw(x, y);
           else if (dev_type == 1) hpgl_draw(x, y);
           break;
       case CH_WDTH:
           x = readCoord(sdvi_file);
           break;
       case CH_COL:
           hue = readFloat(sdvi_file);
           sat = readFloat(sdvi_file);
           brt = readFloat(sdvi_file);
           if ((dev_type == 1) && (no_colour == 0)) hpgl_change_colour(hue, sat, brt);
           break;
       case FILL_P:
           x = readCoord(sdvi_file);
           for (i = 0; i <= x; i++) {
             (void) readCoord(sdvi_file);
             (void) readCoord(sdvi_file);
           }
           break;
       case DIAGRAM:
           (void) readCoord(sdvi_file);
           (void) readCoord(sdvi_file);
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           xr = readCoord(sdvi_file);
           yr = readCoord(sdvi_file);
           n = readCoord(sdvi_file);
           for (i = 0; i < n; i++) (void) fgetc(sdvi_file);
           break;
       default: 
           (void) fprintf(stderr, "sp_vec: bad sdvi command %i.\n", comm);
           (void) exit(1);
           break;
     }
   }

   if (dev_type == 0) tek_close_screen();
   else if (dev_type == 1) hpgl_close_screen();

   if ((dev_type == 1) && (eavesdrop != 0)) {
     (void) fprintf(out_file, "\033.Z");
   }

   if (in_filename != (char *) NULL) (void) fclose(sdvi_file);

   if (out_filename != (char *) NULL) {
     if (*out_filename != '-') (void) fclose(out_file);
   }else{
     if ((pr_name != (char *) NULL) && (*pr_name != '-')) (void) pclose(out_file);
   }
   return(0);
}

help()
{
   (void) fprintf(stderr, "Usage: sp_vec [options] [filename]\n");
   (void) fprintf(stderr, "\nOptions:\n--------\n");
   (void) fprintf(stderr,
     "  -d <device>  = > selects output <device> type (tek[default] or hpgl)\n");
   (void) fprintf(stderr,
       "  -e           = > escapes all TEK control commands\n");
   (void) fprintf(stderr,
       "  -h           = > displays this help list\n");
   (void) fprintf(stderr,
       "  -i <string>  = > initializes HPGL device using given <string>\n");
   (void) fprintf(stderr,
       "  -n           = > requests no colour changes for HPGL device\n");
   (void) fprintf(stderr,
       "  -o <file>    = > outputs into the specified <file>\n");
   (void) fprintf(stderr,
       "                 ( - outputs to standard output)\n");
   (void) fprintf(stderr,
       "  -P <name>    = > forwards output to printer <name>\n");
   (void) fprintf(stderr,
       "                 ( - outputs to standard error)\n");
   (void) fprintf(stderr,
       "  -s           = > displays sdvi file sideways (TEK only)\n");
   (void) fprintf(stderr,
       "  -t <time>    = > pauses <time> seconds for user actions\n");
   (void) fprintf(stderr,
 "  -x <xdim>    = > horizontally shifts output by <xdim> inches (HPGL only)\n");
   (void) fprintf(stderr,
 "  -y <ydim>    = > vertically shifts output by <ydim> inches (HPGL only)\n");
   (void) fprintf(stderr,
       "  -Y           = > accesses HPGL device in eavesdrop mode\n");
   (void) fprintf(stderr,
       "\nIf no <filename> is given, sdvi commands are read from stdin.\n");
   (void) fprintf(stderr,
       "By default, all output is directed to standard output.\n\n");
   (void) exit(0);
}

test_screen()
{
   if (scr_open == 0) {
     (void) fprintf(stderr, "sp_vec: defective sdvi file format.\n");
     (void) exit(1);
   }
}

tek_open_screen(xw, yw)
COORD xw, yw;
{
  float facta, factb;

  if (scr_open == 1) {
    tek_close_screen();
  }
  oxl = oxr = oyl = oyr =  -1;
  scr_open = 1;

  if (sideways_fl == 0) {
    facta = 1024.0/xw;
    factb = 768.0/yw;
    if (facta<factb) t_fact = facta;
    else t_fact = factb;
  }else{
    facta = 1024.0/yw;
    factb = 768.0/xw;
    if (facta<factb) t_fact = facta;
    else t_fact = factb;
  }

  (void) putchar('\033');
  (void) putchar('\014');
  tek_move((COORD) 1, (COORD) 1);
  tek_draw((COORD) xw, (COORD) 1);
  tek_draw((COORD) xw, (COORD) yw);
  tek_draw((COORD) 1, (COORD) yw);
  tek_draw((COORD) 1, (COORD) 1);
}

hpgl_open_screen(xw, yw)
COORD xw, yw;
{
   oxp = xw; oyp = yw;
   hpgl_wait_func(scr_open);
   (void) fprintf(out_file, "IN;SP0;");
   scr_open = 1;
}

tek_move(xp, yp)
COORD xp, yp;
{
   test_screen();
   if ((oxp != xp) || (oyp != yp)) {
     if (all_esc != 0) (void) putchar('\033');
     (void) putchar('\035');
     tek_draw(xp, yp);
   }
}

hpgl_move(xp, yp)
COORD xp, yp;
{
   test_screen();
   (void) fprintf(out_file, "PU;");
   hpgl_coord(xp, yp);
}

tek_draw(xp, yp)
COORD xp, yp;
{
   int xl, xr, yl, yr, xpt, ypt;

   test_screen();
   if (sideways_fl == 0) {
     xpt = t_fact*xp;
     ypt = t_fact*yp;
   }else{
     xpt = 1023-t_fact*yp;
     ypt = t_fact*xp;
   }

   xr =  0x20 | ((xpt>>5)&0x1f);
   xl =  0x40 | (xpt&0x1f);
   yr =  0x20 | ((ypt>>5)&0x1f);
   yl =  0x60 | (ypt&0x1f);

   if (yr != oyr) (void) putchar(yr);
   if ((yl != oyl) || (xr != oxr)) (void) putchar(yl);
   if (xr != oxr) (void) putchar(xr);
   (void) putchar(xl);

   oyr = yr; oyl = yl; oxr = xr; oxl = xl;
   oxp = xp;
   oyp = yp;
}

hpgl_draw(xp, yp)
COORD xp, yp;
{
   test_screen();
   hpgl_coord(xp, yp);
   (void) fprintf(out_file, "PD;");
   hpgl_coord(xp, yp);
}

hpgl_coord(xp, yp)
COORD xp, yp;
{
   float x, y;

   x = yp/3600.0*PLOT_PER_INCH + y_off*PLOT_PER_INCH;
   y = (oxp-xp)/3600.0*PLOT_PER_INCH + x_off*PLOT_PER_INCH;
   (void) fprintf(out_file, "PA, %.2f, %.2f;", x, y);
}

tek_close_screen()
{
   if (scr_open != 0) {
     tek_move((COORD) 0, (COORD) 0);
     if (all_esc == 0) (void) putchar('\033');
     (void) putchar('\037');
     tek_wait_func();
     scr_open = 0;
     (void) putchar('\033');
     (void) putchar('\014');
   }
}

hpgl_close_screen()
{
   (void) fprintf(out_file, "PU;SP0;");
   (void) fflush(out_file);
}

hpgl_change_colour(hue, sat, brt)
float hue, sat, brt;
{
   int colour;

   if (brt < 0.33) colour = 0;
   else if (brt > 0.66) colour = 7;
   else {
     colour = hue/60.0 + 1.5;
     if (colour == 7) colour = 1;
   }
   if (colour != curr_colour) {
     curr_colour = colour;
     (void) fprintf(out_file, "PU;SP0;");
     hpgl_wait_func(-colour-1);
     (void) fprintf(out_file, "SP1;");
   }
}

tek_wait_func()
{
   if (use_stdin != 0) {
     (void) printf("Waiting %i seconds.\n", timeout);
     sleep((unsigned) timeout);
   }else{
     (void) printf("Press <return> to continue.\n");
     while(getchar() != '\n');
   }
}

static char *plot_msgs[] = {"Please insert paper into printer.\n",
                            "Please insert new paper into the printer.\n"},
            *colours[] = {"black", "red", "yellow", "green", "cyan",
                          "blue", "magenta", "white (or no)"};

hpgl_wait_func(type)
int type;
{
   if (eavesdrop != 0) {
     (void) fprintf(out_file, "\033.Z");
   }
   (void) fflush(out_file);

   if (type >= 0) (void) fprintf(stderr, "%s", plot_msgs[type]);
   else (void) fprintf(stderr, "Please insert a %s pen into stall 1.\n",
           colours[-type-1]);

   if (use_stdin != 0) {
     (void) fprintf(stderr, "Restarting in %i seconds.\n", timeout);
     sleep((unsigned) timeout);
   }else{
     (void) fprintf(stderr, "Press <return> when ready.\n");
     while(getchar() != '\n');
   }

   (void) fflush(stderr);
   if (eavesdrop != 0) {
     (void) fprintf(out_file, "\033.Y");
   }
}
