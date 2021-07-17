/******************************************************************
                          sPLOTch!

  Specific.c - specific routines related to unit handling and
               sdvi output
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"
#include "version.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

FILE *sdvi_out=(FILE *) NULL;              /* output file for sdvi codes */
extern COORD llc_x,llc_y,urc_x,urc_y; /* frame def for fpct */
extern struct axis_def p_axes[3];       /* vert and hor axes for abs */
extern struct opt_def options;
extern char tmpbuff[];
COORD x_DRAW, y_DRAW;
int fl_DRAW, scr_open=0, sdvi_pipe= -1;
extern int on_the_fly, interactive_mode;

/* new for sPLOTch!-2.0 consistent page description
        -default dpi=3600
        -page size=8 1/2 by 11 inches or 210mmx297mm (A4) (unless scaled)
        -external drivers scale output to fit device proper */

/*  scsize - converts length parameter to absolute coordinate units
	   - input percent is in Absolute page, Current page or Frame
	   - input string contains value of PerCenT, INches or CMs
	   - now also handles Absolute units for graphs
           - and String pct as well as General pct
	   - defaults to units set by options statement

           - returns specific value in sp_v and specific unit in sp_u
	   - units in terms of direction char dc ( x or y)
           - uses sp_v and sp_u values if mask<0

	   - relative specs are started with R
	   - rel input forces the relative issue (has different
	       effects for things like ABS)
	   - returns negative value if invalid argument (sz=0),
		unit type otherwise                            
           - test for a relative request by device type being > MID_UNIT
           - mask indicates DISALLOWED unit types, except for HPCT
				     */
#define NUNITS 21

char *typen[]={"fpct","apct","pct","in","cm","abs","spct",
      "gpct","hpct","pt","r","rfpct","rapct","rpct","rin","rcm","rabs",
      "rspct","rgpct","rhpct","rpt"};

int types[]={FPCT,APCT,PCT,INCH,CM,ABS,SPCT,GPCT,HPCT,PT,100,
       RFPCT,RAPCT,RPCT,RINCH,RCM,RABS,RSPCT,RGPCT,RHPCT,RPT};

int scsize(ptr, dc, crd_sz, sp_vu, rel, mask)
char *ptr, dc;
COORD *crd_sz;
int rel, mask;
struct united *sp_vu;
{
    COORD xpos, t_sz;
    int unit, c, i;
    char ch;

    unit=options.def_unit;

    if (mask<0) {
      if (sp_vu->unit==-1) {
        *crd_sz=0;
        return(-1);
      }
      if ((sp_vu->unit>=0)&&(sp_vu->unit<NUNITS)) {
        unit=types[sp_vu->unit];
      }
      mask=M_HPCT;
    }else{
      for (c=0;c<strlen(ptr);c++) {
        ch=clower(*(ptr+c));
        if((ch!=' ')&&(ch!='\t')&&(ch!='.')&&(ch!='-')&&(ch!='+')&&
           ((ch<'0')||(ch>'9'))&&(ch!='e')) break;
      }
      ch= *(ptr+c);
      *(ptr+c)='\0';
      sp_vu->val=atof(ptr);
      sp_vu->unit= -2;
      *(ptr+c)=ch;

      if (c!=strlen(ptr)) {
        unit=sp_vu->unit= -1;
        for (i=0;i<NUNITS;i++) {
          if (l_comp((ptr+c),typen[i])!=0) {
            unit=types[i];
            sp_vu->unit=i;
          }
        }
        if (unit==-1) {
	  *crd_sz=0;
          sp_vu->val=0.0;
	  return(-1);
        }
      }
    }

    if (unit==100) {
      if (options.def_unit<=MID_UNIT) {
        unit=options.def_unit+MID_UNIT;
      }else{
        unit=options.def_unit;
      }
    }

    switch (unit) {
      case CM : case RCM :
              if ((mask&M_CM)!=0) unit= -1;
              t_sz=sp_vu->val*1417.3228;
              break;
      case INCH : case RINCH :
              if ((mask&M_INCH)!=0) unit= -1;
	      t_sz=sp_vu->val*3600.0;
	      break;
      case PT: case RPT:
              if ((mask&M_PT)!=0) unit= -1;
              t_sz=sp_vu->val*49.8132;
              break;
      case RPCT : rel=1;
      case PCT :
              if ((mask&M_PCT)!=0) unit= -1;
              if (dc=='x') {
	         t_sz=sp_vu->val*options.cp_xmax/100.0;
                 if (rel==0) t_sz+=options.origin_x;
              }else{
	         t_sz=sp_vu->val*options.cp_ymax/100.0;
                 if (rel==0) t_sz+=options.origin_y;
              }
	      break;
      case RAPCT : case APCT :
              if ((mask&M_APCT)!=0) unit= -1;
              if (dc=='y') {
	         t_sz=sp_vu->val*options.dev_ymax/100.0;
              }else{
	         t_sz=sp_vu->val*options.dev_xmax/100.0;
              }
	      break;
      case RFPCT : rel=1;
      case FPCT :
              if ((mask&M_FPCT)!=0) unit= -1;
              if (dc=='y') {
	         t_sz=sp_vu->val*(urc_y-llc_y)/100.0;
	         if (rel==0) t_sz+=llc_y;
              }else{
	         t_sz=sp_vu->val*(urc_x-llc_x)/100.0;
       	         if (rel==0) t_sz+=llc_x;
              }
	      break;
#ifndef PCS
      case RABS : rel=1;
      case ABS :
              if ((mask&M_ABS)!=0) unit= -1;
              else{
                i=0;
	        if (dc=='x') {
		   pos_inter(p_axes, (double) sp_vu->val, 'x', &t_sz, 0);
                   if (t_sz==MAX_CRD) i=1;
		   if (rel!=0) {
		      pos_inter(p_axes, (double) 0.0, 'x', &xpos, 0);
                      if (xpos==MAX_CRD) i=1;
		      t_sz-=xpos;
		   }
	        }else{
		   pos_inter((p_axes+1), (double) sp_vu->val, 'y', &t_sz, 0);
                   if (t_sz==MAX_CRD) i=1;
		   if (rel!=0) {
		      pos_inter((p_axes+1), (double) 0.0, 'y', &xpos, 0);
                      if (xpos==MAX_CRD) i=1;
		      t_sz-=xpos;
		   }
	        }
                if (i!=0) t_sz=MAX_CRD;
              }
	      break;
#endif
      case RSPCT : case SPCT :
              if ((mask&M_SPCT)!=0) unit= -1;
              t_sz=options.sht*sp_vu->val/100.0;
	      break;
      case RGPCT : rel=1;
      case GPCT :
              if ((mask&M_GPCT)!=0) unit= -1;
              if (dc=='y') {
	         t_sz=sp_vu->val*options.gp_ymax/100.0;
	         if (rel==0) t_sz+=options.gp_orig_y;
              }else{
	         t_sz=sp_vu->val*options.gp_xmax/100.0;
	         if (rel==0) t_sz+=options.gp_orig_x;
              }
	      break;
      case RHPCT : case HPCT :
              if ((mask&M_HPCT)==0) unit= -1;
              t_sz=sp_vu->val*320.0;
	      break;
    }

    if ((rel!=0)&&(unit>0)&&(unit<=MID_UNIT)) {
      unit=unit+MID_UNIT;
    }
    if (unit==-1) {
      *crd_sz=0;
      sp_vu->val=0.0;
      sp_vu->unit= -1;
    }else{
      *crd_sz=t_sz;
    }

#ifdef EBUG
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"Unit source %s unit %i height %i\n",ptr,unit,t_sz);
    (void) fprintf(deb_log,"Decomp val %g unit %i\n", sp_vu->val, sp_vu->unit);
    (void) fflush(deb_log);
  }
#endif

    return(unit);
}

/* set_unit - set default unit type (option statement)   */

int set_unit(ptr)
char *ptr;
{
   int i, unit;

   unit= -1;
   for (i=0;i<NUNITS;i++) {
     if (l_comp(ptr, typen[i])!=0) unit=types[i];
   }
   if (unit==100) {
      if (options.def_unit<=MID_UNIT) {
        unit=options.def_unit+MID_UNIT;
      }else{
        unit=options.def_unit;
      }
   }
   return(unit);
}

/*   get_ymax,get_xmax - returns current page x,y sizes   */

COORD get_ymax()
{
   return (options.cp_ymax);
}
COORD get_xmax()
{
   return (options.cp_xmax);
}

/* getmaca_xmax, getmaca_ymax - returns actual page x and y sizes
			      - considers rotate flag for portrait
                                  rotation */

COORD getmaca_xmax()
{
   if ((options.rotate==0)||(options.rotate==2)) {
     return(options.dev_xmax);
   } else {
     return(options.dev_ymax);
   }
}

COORD getmaca_ymax()
{
   if ((options.rotate==0)||(options.rotate==2)) {
     return(options.dev_ymax);
   } else {
     return(options.dev_xmax);
   }
}

/* set_rotate - initialize the appropriate sizes according
		to the rotate parameter
              - new for sPLOTch!-2.0...8 1/2 by 11 inches or A4, period */

set_rotate(rotate)
int rotate;
{
   if ((rotate==1)||(rotate==3)) {
#ifdef A4
      options.gp_ymax=options.cp_ymax=options.dev_ymax=29764;
      options.gp_xmax=options.cp_xmax=options.dev_xmax=42095;
#else
      options.gp_ymax=options.cp_ymax=options.dev_ymax=30600;
      options.gp_xmax=options.cp_xmax=options.dev_xmax=39600;
#endif
   } else {
#ifdef A4
      options.gp_ymax=options.cp_ymax=options.dev_ymax=42095;
      options.gp_xmax=options.cp_xmax=options.dev_xmax=29764;
#else
      options.gp_xmax=options.cp_xmax=options.dev_xmax=30600;
      options.gp_ymax=options.cp_ymax=options.dev_ymax=39600;
#endif
   }
   options.aspect= -1;
   options.gp_orig_x=options.origin_x=0;
   options.gp_orig_y=options.origin_y=0;
   fl_DRAW=1;
}

#ifndef PCS
/* outputs for sdvi output generation

   init_sdvi - writes the sdvi header sequence
   open_screen - open the device, or finish page and start new one
   close_screen - finish page
   update_screen - refresh page to present status
   move - move to the point xp, yp (null if last move/draw was to there)
   draw - draw to the point xp, yp
   change_width - set the linewidth to width
   change_colour - change to the colour color
   fill_poly - fill the polygon poly 
   dg_include - include diagram of sorts */

#define SDVI_MAGIC 745252188

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

int first_open;

reg_sdvi(file, pipe)
FILE *file;
int pipe;
{
   sdvi_out=file;
   sdvi_pipe=pipe;
   first_open=0;
}

dereg_sdvi()
{
   if ((sdvi_out==(FILE *) NULL)||(first_open==0)) return;

   write_crd((COORD) END_OF_FILE);
   (void) fflush(sdvi_out);
   if (sdvi_pipe==1) (void) pclose(sdvi_out);
   else if (sdvi_pipe==0) (void) fclose(sdvi_out);
   sdvi_pipe= -1;
   sdvi_out=(FILE *) NULL;
}

init_sdvi()
{
   write_crd((COORD) SDVI_MAGIC);
   write_crd((COORD) MAJ_VERS_NO);
   write_crd((COORD) MIN_VERS_NO);
   write_crd((COORD) interactive_mode);
}

open_screen()
{
   if (sdvi_out==(FILE *) NULL) abort_file();
  
   if (first_open==0) {
     init_sdvi();
     first_open=1;
   }

   del_clip(1);
   del_clip(0);

   write_crd((COORD) OPEN_SCR);
   write_crd(getmaca_xmax());
   write_crd(getmaca_ymax());
   write_flt((float) options.winfract);
   scr_open=1;
}

close_screen()
{
   if (sdvi_out==(FILE *) NULL) abort_file();

   del_clip(1);
   del_clip(0);

   if (scr_open!=0) {
     write_crd((COORD) CLOSE_SCR);
     (void) fflush(sdvi_out);
     scr_open=0;
   }
}

update_screen()
{
   if (sdvi_out==(FILE *) NULL) abort_file();
   if (scr_open==0) return;

   write_crd((COORD) UPD_SCR);
   (void) fflush(sdvi_out);
}

move_pt(xp,yp)
COORD xp,yp;
{
   COORD x,y;

   test_screen();

   if (fl_DRAW!=0) fl_DRAW=0;
   else if ((xp==x_DRAW)&&(yp==y_DRAW)) return;
   x_DRAW=xp;
   y_DRAW=yp;

   conv_point(xp, yp, &x, &y);

   write_crd((COORD) MOVE_PT);
   write_crd(x);
   write_crd(y);
}

draw_pt(xp,yp)
COORD xp,yp;
{
   COORD x,y;

   test_screen();

   if (on_the_fly>0) {
     (void) fprintf(stderr,"Line %i %i->%i %i\n", x_DRAW, y_DRAW, xp, yp);
   }
   if (fl_DRAW!=0) fl_DRAW=0;
   x_DRAW=xp;
   y_DRAW=yp;

   conv_point(xp, yp, &x, &y);

   write_crd((COORD) DRAW_PT);
   write_crd(x);
   write_crd(y);

   if (on_the_fly!=0) {
     update_screen();
   }
   if (on_the_fly>0) {
     while(getchar()!='\n');
   }
}

conv_point(xp,yp,x,y)
COORD xp,yp,*x,*y;
{
   switch(options.rotate) {
     case 1 : *x=options.dev_ymax-yp;
	      *y=xp;
	      break;
     case 2 : *x=options.dev_xmax-xp;
	      *y=options.dev_ymax-yp;
	      break;
     case 3 : *x=yp;
	      *y=options.dev_xmax-xp;
	      break;
     default: *x=xp;
	      *y=yp;
	      break;
   }
}

change_width(width)
COORD width;
{
   test_screen();
   write_crd((COORD) CH_WDTH);
   write_crd(width);
}

change_colour(colour)
struct sp_colour colour;
{
   test_screen();
   write_crd((COORD) CH_COL);
   colour.hue=fmod(colour.hue, 360.0);
   if (colour.hue<0.0) colour.hue=colour.hue+360.0;
   write_flt((float) colour.hue);
   write_flt((float) colour.sat);
   write_flt((float) colour.bright);
}
 
fill_poly(poly)
struct sp_poly poly;
{
   int i;
   COORD x,y;

   test_screen();

   if (on_the_fly>0) {
     (void) fprintf(stderr,"Poly %i %i->%i %i->%i %i...\n",
        poly.pts->x, poly.pts->y, (poly.pts+1)->x, (poly.pts+1)->y,
        (poly.pts+2)->x, (poly.pts+2)->y);
   }

   write_crd((COORD) FILL_P);
   write_crd((COORD) poly.n_points);
   for (i=0;i<=poly.n_points;i++) {
     conv_point((poly.pts+i)->x, (poly.pts+i)->y, &x, &y);
     write_crd(x);
     write_crd(y);
   }

   if (on_the_fly!=0) {
     update_screen();
   }
   if (on_the_fly>0) {
     while(getchar()!='\n');
   }
}

dg_include(type,xl,yl,xr,yr,name)
int type;
COORD xl,yl,xr,yr;
char *name;
{
   int i;
   COORD x,y;

   test_screen();
   write_crd((COORD) DIAGRAM);
   write_crd((COORD) type);
   write_crd((COORD) options.rotate);
   conv_point(xl, yl, &x, &y);
   write_crd(x);
   write_crd(y);
   conv_point(xr, yr, &x, &y);
   write_crd(x);
   write_crd(y);
   write_crd((COORD) strlen(name));
   for (i=0;i<strlen(name);i++) (void) fputc(*(name+i), sdvi_out);
}

test_screen()
{
   if (sdvi_out==(FILE *) NULL) abort_file();
   if (scr_open==0) open_screen();
}

abort_file()
{
   (void) fprintf(stderr,"Fatal error:  Nowhere to send the output.\n");
   (void) fprintf(stderr,"Dying.....\n");
   (void) exit(1);
}

write_flt(val)
float val;
{
   write_crd((COORD) (val*1000000.0));
}

write_crd(val)
COORD val;
{
   int flag;
   int s1, s2, s3, s4;

   flag=0;
   if (val<0) {flag=1; val= -val;}

   s2=val/65536; s4=val-s2*65536;
   s1=s2/256; s2-=s1*256;
   s3=s4/256; s4-=s3*256;

   if (flag!=0) s1+=128;
  
   (void) fputc(s1, sdvi_out);
   (void) fputc(s2, sdvi_out);
   (void) fputc(s3, sdvi_out);
   (void) fputc(s4, sdvi_out);
}

#endif
