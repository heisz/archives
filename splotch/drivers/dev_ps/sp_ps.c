/*
 *  PostScript printer driver for sPLOTch! Version 2.1
 */

#define NEEDTIME
#include "../localdef.h"
#include <stdio.h>
#include "../sdvi.h"

#ifndef DEF_PRNAME
#define DEF_PRNAME "laser"
#endif
#ifndef PRINTER
#define PRINTER "/usr/ucb/lpr"
#endif

FILE *ps_file;
int lps, drfl, eps_flag, min_ver, maj_ver, clip_fl;
int sideways_fl, scr_open = 0, mv_fl, num_copies;
int line_count, page_no, grey_fl, quiet_fl;
float curr_hue, curr_sat, curr_brt, curr_width, x_off, y_off;
static char *d_pr = DEF_PRNAME, *pr_comm = PRINTER;
COORD x_last, y_last;

main(argc, argv)
int argc;
char *argv[];
{
   int c, comm, i, n;
   COORD x, y, xr, yr;
   float hue, sat, brt;
   char *pr_name, *in_filename, *ptr, *out_filename;
   FILE *sdvi_file;
   char com_buff[1000], diag_buff[1000];

   clip_fl = line_count = page_no = grey_fl = eps_flag = sideways_fl = quiet_fl = 0;
   x_off = y_off = curr_hue = curr_sat = curr_brt = 0.0;
   curr_width = 1.0;
   num_copies = 1;
   pr_name = d_pr;
   in_filename = out_filename = (char *) NULL;

   for (c = 1; c<argc; c++) {
     ptr = argv[c]+2;
     if (*(argv[c]) == '-') {
       switch(*(argv[c]+1)) {
         case 'c' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL)) 
                      ptr = argv[++c];
                    num_copies = atoi(ptr);
                    if (num_copies < 1) num_copies = 1;
                    break;
         case 'e' : eps_flag = 1;
                    break;
         case 'f' : clip_fl = 1;
                    break;
         case 'g' : grey_fl = 1;
                    break;
         case 'h' : help();
                    break;
         case 'o' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL)) 
                      ptr = argv[++c];
                    out_filename = ptr;
                    break;
         case 'P' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL)) 
                      ptr = argv[++c];
                    pr_name = ptr;
                    break;
         case 'q' : quiet_fl = 1;
                    break;
         case 's' : sideways_fl = 1;
                    break;
         case 'x' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL)) 
                      ptr = argv[++c];
                    x_off = atof(ptr); 
                    break;
         case 'y' : if ((*ptr == '\0') && (argv[c+1] != (char *) NULL)) 
                      ptr = argv[++c];
                    y_off = atof(ptr); 
                    break;
         default :  (void) fprintf(stderr,
                       "sp_ps: option not one of cefghoPqsxy.\n");
                    break;
       }
     }else{
       in_filename = argv[c];
     }
   }

   if (in_filename == (char *) NULL) {
     sdvi_file = stdin;
   }else{
     sdvi_file = fopen(in_filename, "r");
     if (sdvi_file == (FILE *) NULL) {
       (void) fprintf(stderr,
                "sp_ps: unable to open sdvi file %s.\n", in_filename);
       (void) exit(1);
     }
   }

   x = readCoord(sdvi_file);
   if (feof(sdvi_file) != 0) (void) exit(0);

   if (x != SDVI_MAGIC) {
     (void) fprintf(stderr, "sp_ps: not a sPLOTch! sdvi file.\n");
     (void) fprintf(stderr, "Magic number %i instead of %i.\n", x, SDVI_MAGIC);
     (void) exit(1);
   }
   maj_ver = readCoord(sdvi_file);
   min_ver = readCoord(sdvi_file);
   (void) readCoord(sdvi_file);
  
   if (out_filename == (char *) NULL) {
     if (*pr_name == '-') {
       ps_file = stderr;
     }else{
       (void) sprintf(com_buff, "%s -P%s", pr_comm, pr_name);
       ps_file = popen(com_buff, "w");
       if (ps_file == (FILE *) NULL) {
         (void) fprintf(stderr,
            "sp_ps: cannot open printer pipe via %s.\n", com_buff);
         (void) exit(1);
       }
     }
   }else{
     if (*out_filename == '-') {
       ps_file = stdout;
     }else{
       ps_file = fopen(out_filename, "w");
       if (ps_file == (FILE *) NULL) {
         (void) fprintf(stderr,
            "sp_ps: cannot open output file %s.\n", out_filename);
         (void) exit(1);
       }
     }
   }

   while (((comm = readCoord(sdvi_file)) != END_OF_FILE) && (feof(sdvi_file) == 0)) {
     switch(comm) {
       case OPEN_SCR:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           (void) readFloat(sdvi_file);
           ps_open_screen(x, y, in_filename);
           break;
       case UPD_SCR:
           ps_update();
           break;
       case CLOSE_SCR:
           break;
       case MOVE_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           ps_move(x, y);
           break;
       case DRAW_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           ps_draw(x, y);
           break;
       case CH_WDTH:
           x = readCoord(sdvi_file);
           ps_width(x);
           break;
       case CH_COL:
           hue = readFloat(sdvi_file);
           sat = readFloat(sdvi_file);
           brt = readFloat(sdvi_file);
           ps_colour(hue, sat, brt);
           break;
       case FILL_P:
           ps_open_poly();
           n = readCoord(sdvi_file);
           for (i = 0; i <= n; i++) {
             x = readCoord(sdvi_file);
             y = readCoord(sdvi_file);
             ps_poly_pt(x, y, i);
           }
           ps_close_poly();
           break;
       case DIAGRAM:
           (void) readCoord(sdvi_file);
           (void) readCoord(sdvi_file);
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           xr = readCoord(sdvi_file);
           yr = readCoord(sdvi_file);
           n = readCoord(sdvi_file);
           for (i = 0; i < n; i++) diag_buff[i] = fgetc(sdvi_file);
           ps_move(x, y);
           ps_draw(xr, y);
           ps_draw(xr, yr);
           ps_draw(x, yr);
           ps_draw(x, y);
           break;
       default: 
           (void) fprintf(stderr, "sp_ps: bad sdvi command %i.\n", comm);
           (void) exit(1);
           break;
     }
   }

   ps_close_screen();

   if (in_filename != (char *) NULL) (void) fclose(sdvi_file);

   if (out_filename == (char *) NULL) {
     if (*pr_name != '-') {
       (void) pclose(ps_file);
       if (quiet_fl == 0) (void) fprintf(stderr,
            "[sp_ps: %i pages, printer %s]\n", page_no, pr_name);
     }else{
       if (quiet_fl == 0) (void) fprintf(stderr,
            "[sp_ps: %i pages, on <stderr>]\n", page_no);
     }
   }else{
     if (*out_filename != '-') {
       (void) fclose(ps_file);
       if (quiet_fl == 0) (void) fprintf(stderr,
            "[sp_ps: %i pages, file %s]\n", page_no, out_filename);
     }else{
       if (quiet_fl == 0) (void) fprintf(stderr,
            "[sp_ps: %i pages, on <stdout>]\n", page_no);
     }
   }
   return(0);
}

help()
{

 (void) fprintf(stderr, "\nUsage: sp_ps [options] [filename]\n");
 (void) fprintf(stderr, "\nOptions:\n--------\n");
 (void) fprintf(stderr,
          "  -c <num>    = > produces <num> copies of the output\n");
 (void) fprintf(stderr,
          "  -e          = > produces PostScript without 'showpage'\n");
 (void) fprintf(stderr,
          "                (for bad Extended PostScript readers)\n");
 (void) fprintf(stderr,
          "  -f          = > enables absolute page output clipping\n");
 (void) fprintf(stderr,
          "  -g          = > force colour to greyscale conversion\n");
 (void) fprintf(stderr,
          "  -h          = > displays this help list\n");
 (void) fprintf(stderr,
          "  -o <file>   = > outputs into the specified <file>\n");
 (void) fprintf(stderr,
          "                ( - outputs to standard output)\n");
 (void) fprintf(stderr,
          "  -P <name>   = > forwards output to printer <name>\n");
 (void) fprintf(stderr,
          "                ( - outputs to standard error)\n");
 (void) fprintf(stderr,
          "  -q          = > operates quietly (no page output information)\n");
 (void) fprintf(stderr,
          "  -s          = > displays output in landscape orientation\n");
 (void) fprintf(stderr,
          "  -x <xdim>   = > horizontally shifts output by <xdim> inches\n");
 (void) fprintf(stderr,
          "  -y <ydim>   = > vertically shifts output by <ydim> inches\n");
 (void) fprintf(stderr,
          "\nIf no <filename> is given, sdvi commands are read from stdin.\n");
 (void) fprintf(stderr,
          "Current default printer name is >%s<.\n\n", d_pr);

 (void) exit(0);
}

test_screen()
{
   if (scr_open == 0) {
     (void) fprintf(stderr, "sp_ps: defective sdvi file format.\n");
     (void) exit(1);
   }
}

ps_open_screen(xw, yw, f_name)
COORD xw, yw;
char *f_name;
{
   char *userid, *d_time;
   time_t clock;
   struct tm *tmedat;

   if (scr_open == 0) {
      clock = time((time_t *) 0);
      tmedat = localtime(&clock);
      d_time = asctime(tmedat);
      userid = getenv("USER");

 (void) fprintf(ps_file, "%%!PS-Adobe-1.0\n");
 (void) fprintf(ps_file, "%%%%BoundingBox: %i %i %i %i\n", 0, 0, (xw/50), (yw/50));
      if (userid != (char *) NULL) 
 (void) fprintf(ps_file, "%%%%Creator: %s using sp_ps\n", userid);
      if (f_name != (char *) NULL)
 (void) fprintf(ps_file, "%%%%Title: sPLOTch! %i.%i output file: %s\n",
                        maj_ver, min_ver, f_name);
      else
 (void) fprintf(ps_file, "%%%%Title: sPLOTch! %i.%i output file: <stdin>\n",
                        maj_ver, min_ver);
      if (d_time != (char *) NULL) 
 (void) fprintf(ps_file, "%%%%CreationDate: %s", d_time);
 (void) fprintf(ps_file, "%%%%DocumentFonts: none\n");
 (void) fprintf(ps_file, "%%%%Pages: (atend)\n");
 (void) fprintf(ps_file, "%%%%EndComments\n\n");
 (void) fprintf(ps_file, "/xx { lineto } def\n/xz { moveto } def\n");
 (void) fprintf(ps_file, "/zx { stroke newpath } def\n");
 if (num_copies != 1) (void) fprintf(ps_file, "/#copies %i def\n", num_copies);
 (void) fprintf(ps_file, "%%%%EndProlog\n");
 (void) fprintf(ps_file, "gsave\n");
      scr_open = 1;
   }else{
      if (drfl == 1) (void) fprintf(ps_file, "zx\n");
      else (void) fprintf(ps_file, "\n");
      (void) fprintf(ps_file, "showpage\n\n");
   }
   lps = 0;
   drfl = 0;
   mv_fl = 0;
   page_no++;

 (void) fprintf(ps_file, "%%%%Page: %i %i\n", page_no, page_no);
 (void) fprintf(ps_file, "1 setlinecap 1 setlinejoin");
 (void) fprintf(ps_file, " %.2f setlinewidth\n", curr_width);
 (void) fprintf(ps_file, "%.3f %.3f %.3f sethsbcolor\n",
                curr_hue, curr_sat, curr_brt);
   if (clip_fl != 0) {
     (void) fprintf(ps_file, "newpath\n");
     ps_motion("", "xz", (COORD) 0, (COORD) 0);
     ps_motion("", "xx", xw, (COORD) 0);
     ps_motion("", "xx", xw, yw);
     ps_motion("", "xx", (COORD) 0, yw);
     (void) fprintf(ps_file, "\nclosepath clip newpath\n");
   }
}

ps_draw(xp, yp)
COORD xp, yp;
{
   test_screen();
   if (mv_fl != 0) {
     mv_fl = 0;
     ps_motion("", "xz", x_last, y_last);
     line_count = 0;
   }
   ps_motion("", "xx", xp, yp);
   drfl = 1;
   line_count += 1;
   if (line_count > 500) {
     ps_move(xp, yp);
   }
   x_last = xp;
   y_last = yp;
}

ps_move(xp, yp)
COORD xp, yp;
{
   test_screen();
   if (drfl == 1) {
      ps_motion("zx ", "xz", xp, yp);
      drfl = 0;
   }else{
      ps_motion("", "xz", xp, yp);
   }

   mv_fl = 0;
   line_count = 0;
}

ps_motion(pre_str, post_str, xp, yp)
char *pre_str, *post_str;
COORD xp, yp;
{
   float x_pt, y_pt;
   int l;

   if (sideways_fl == 0) {
     x_pt = xp/50.0;
     y_pt = yp/50.0;
   }else{
     x_pt = 612.0-yp/50.0;
     y_pt = xp/50.0;
   }
   x_pt = x_pt+x_off*72.0;
   y_pt = y_pt+y_off*72.0;

   l = fprintf(ps_file, "%s%.2f %.2f %s ", pre_str, x_pt, y_pt, post_str);
   lps += l;
   if (lps > 53) {
     (void) fprintf(ps_file, "\n");
     lps = 0;
   }
}

ps_close_screen()
{
   if (scr_open != 0) {
     if (drfl == 1) (void) fprintf(ps_file, "zx\n");
     else (void) fprintf(ps_file, "\n");
     if (eps_flag == 0) {
       if (num_copies != 1) (void) fprintf(ps_file, "/#copies %i\n", num_copies);
       (void) fprintf(ps_file, "showpage\n");
     }
     
     (void) fprintf(ps_file, "\ngrestore\n");
     (void) fprintf(ps_file, "%%%%Trailer\n");
     (void) fprintf(ps_file, "%%%%Pages: %i\n", page_no);

     scr_open = 0;
   }
}

ps_update()
{
   if (scr_open != 0) {
     (void) fflush(ps_file);
   }
}

ps_width(xp)
COORD xp;
{
   float x_pt;

   test_screen();
   x_pt = xp/50.0;
   if (drfl == 1) {
     (void) fprintf(ps_file, "zx");
     mv_fl = 1;
     drfl = 0;
   }
   (void) fprintf(ps_file, "\n%.2f setlinewidth\n", x_pt);
   lps = 0;
   curr_width = x_pt;
}

ps_colour(hue, sat, bright)
float hue, sat, bright;
{
   float red, green, blue, grey_scale(); 

   test_screen();
   if (drfl == 1) {
     (void) fprintf(ps_file, "zx");
     mv_fl = 1;
     drfl = 0;
   }

   hsb_to_rgb(hue, sat, bright, &red, &green, &blue);
   if (grey_fl != 0) {
     bright = grey_scale(red, green, blue);
     red = green = blue = bright;
   }

   (void) fprintf(ps_file, "\n%.5f %.5f %.5f setrgbcolor\n",
                  red, green, blue);
   lps = 0;
   curr_hue = hue;
   curr_sat = sat;
   curr_brt = bright;
}

ps_open_poly()
{
   test_screen();
   if (drfl == 1) {
     (void) fprintf(ps_file, "zx");
     mv_fl = 1;
     drfl = 0;
   }
   (void) fprintf(ps_file, "\nnewpath\n");
   lps = 0;
}

ps_poly_pt(xp, yp, i)
COORD xp, yp;
int i;
{
   if (i == 0) ps_motion("", "xz", xp, yp);
   else ps_motion("", "xx", xp, yp);
}

ps_close_poly()
{
   (void) fprintf(ps_file, "\nclosepath fill newpath\n");
   lps = 0;
}

#define RGB
#include "../common.c"
