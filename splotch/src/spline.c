/******************************************************************
                          sPLOTch!

  Spline - no, this is not the routine which does the spline
     interpolations, but rather it is the line drawing routines.
     Handles multiple clipping boundaries, polygon filling and 
     hatching, line and polygon attribute control, and colour!
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

extern struct opt_def options;
extern char inbuff[],tmpbuff[];
static int exc_flag=0, inc_flag=0, exc_none=0, inc_none=0;
static struct sp_poly exc_poly, inc_poly;
static struct sp_linefill exc_bord, inc_bord;
extern struct pair_v sym_vals[];

/* linestyle definition list...-1 terminated list of inversion  
    positions in percent of repeat length  */

int lines[31][10]={
   {1000,-1},{250,500,750,1000,-1},{375,500,875,1000,-1},
   {250,330,410,500,750,830,910,1000,-1},
   {435,500,935,1000,-1},{500,600,700,800,900,1000,-1},
   {350,400,450,500,850,900,950,1000,-1},
   {930,1000,-1},{850,900,950,1000,-1},
   {300,400,500,600,900,1000,-1},
   {750,1000,-1},{875,1000,-1},{700,800,900,1000,-1},
   {500,1000,-1},{500,660,820,1000,-1}, {0,1000,-1}
   };

/* linedefs start at line 0 (ie. exact pattern match) */

int linedefs[32]={1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
                  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#define NUMLINES 29
#define RPTLN 1000.0   /* fractional increment in the linedefs */
#define RPT_LIM 10.0   /* smallest allowable line sizing */

struct poly_int { struct coordinate pt;
                  float mu; 
                  int poly_ln; 
                };

/* init_lf_def - initializes the lnfill_def storage structure */

init_lf_def(lp_t)
struct lnfill_def *lp_t;
{
   lp_t->width.unit= -1;
   lp_t->colour_set=0;
   lp_t->repeat.unit= -1;
   lp_t->pattern_set=0;
   lp_t->none=0;
   lp_t->sp_fl=0;
   lp_t->colour_sp_fl=0;
}

/* init_linefill - initializes the linefill description structure 
                 - lp_fl non-zero if polygon */

init_linefill(lp, patt, lp_fl)
struct sp_linefill *lp;
{
   COORD get_ymax();

   lp->width=options.linewidth;
   if (patt<=0) lp->pattern= -patt;
   else lp->pattern=set_sym_patt(patt, lp_fl);
#ifndef SPACE
   set_colour(&(lp->colour), options.linecolour, 7, 0);
#else
   lp->colour.hue = lp->colour.sat = lp->colour.bright = 0.0;
#endif
   if (lp_fl==0) lp->repeat=0.1*get_ymax();
   else lp->repeat=0.05*get_ymax();
   lp->curr_ln=0;
}

/* set_patt - returns the pattern number if defined, else 1 (solid) */

int set_patt(patt)
int patt;
{
   if (linedefs[patt]==0) patt=1;
   return(patt);
}

/* set_sym_patt - returns the pattern given by sym */

int set_sym_patt(sym, lp_fl)
int sym, lp_fl;
{
  int i, patt;

  if (lp_fl==0) {
    patt=1;
    while (sym>0) {
       for (i=1;i<=31;i++) {
         if (linedefs[i]!=0) {
           sym--;
           if (sym==0) patt=i;
         }
       }
     }
   }else{
     patt=sym%7+1;
   }

   return(patt);
}

#ifndef SPACE
/*  line_fill - interprets the line/fill attribute commands, either
                  from ptr (if non-NULL) or from the attribute 
                  storage structure
              - lp_fl non-zero if fill definition
              - dft is the pattern replacement value (if pattern -1)
              - returns the storage and description structures
             - syval_fl is the origin indicator (see speak.c) */

line_fill(ptr, lp_t, lp, lpos, lp_fl, dft, syval_fl)
char *ptr;
int lpos, lp_fl, dft, syval_fl;
struct sp_linefill *lp;
struct lnfill_def *lp_t;
{
   float val;
   int i, rc, com[4], t_com[4], l, tmp, t_fl;
   struct united sp_vu;
   COORD jnk, get_ymax();


   if (ptr!=(char *) NULL) {
     init_lf_def(lp_t);

     com[1]=0;
     do {
       rc=scan_cmd(ptr, com, lpos, &l, &tmp, inbuff, 1);
       if (rc<0) {
         for (i=0;i<4;i++) t_com[i]=com[i];
         yank(ptr, com, inbuff);
         if (is_numeric(inbuff,1)==0) {
           for (i=0;i<4;i++) com[i]=t_com[i];
         }else{
           rc=strlen(inbuff);
           com[0]=125;
           tmp=com[3]+lpos;
           l=com[2];
         }
       }
       if (rc<0) {
         what(BADCOM, ptr, com, lpos, 1);
       }else{
         switch(com[0]) {
           case 0  : break;
           case -2 : break;
           case 14 : lp_t->none=1;
                     break;
           case 51 : t_fl=scan_num(inbuff, tmp, syval_fl);
                     if (t_fl<0) break;
                     rc=scsize(inbuff, 'y', &jnk, &sp_vu, 1, 0);
                     if (rc<0) {
                       sp_err(BADSIZE, tmp, l);
                     }else{
                       lp_t->repeat=sp_vu;
                       lp_t->sp_fl=(lp_t->sp_fl&0xF0F)|(t_fl*0x010);
                     }
                     break;
           case 123: t_fl=scan_num(inbuff, tmp, syval_fl);
                     if (t_fl<0) break;
                     rc=scsize(inbuff, 'y', &jnk, &sp_vu, 1, 0);
                     if (rc<0) {
                       sp_err(BADSIZE, tmp, l);
                     }else{
                       lp_t->width=sp_vu;
                       lp_t->sp_fl=(lp_t->sp_fl&0xFF0)|(t_fl*0x001);
                     }
                     break;
           case 124: (void) get_colour(inbuff, &(lp_t->colour),
                          &(lp_t->colour_set), &(lp_t->colour_sp_fl), tmp,
                          l, syval_fl);
                     break;
           case 125: t_fl=scan_num(inbuff, tmp, syval_fl);
                     if (t_fl<0) break;
                     rc=atoi(inbuff);
                     if (((((rc<0)||(rc>30)||(linedefs[rc]==0))&&(lp_fl==0))||
                          (((rc<0)||(rc>7))&&(lp_fl!=0)))&&(rc!=-1)) {
                       sp_err(BADPATT, tmp, l);
                     }else{
                       lp_t->pattern_set=1;
                       lp_t->pattern=rc;
                       lp_t->sp_fl=(lp_t->sp_fl&0x0FF)|(t_fl*0x100);
                     }
                     break;
           default : com[1]=com[3];
                     what (ILLCOM, ptr, com, lpos, 1);
                     break;
         }
       }
     } while (com[0]!=0);
   }

   if (lp_t->width.unit!=-1) {
     sp_vu=lp_t->width;
     if ((lp_t->sp_fl&0x00F)!=0) {
       sp_vu.val=sym_vals[(lp_t->sp_fl&0x00F)/0x001-1].f;
     }
     if (lp_t->width.unit==-2) {
       lp->width=options.linewidth*sp_vu.val;
     }else{
       rc=scsize((char *) NULL, 'y', &(lp->width), &sp_vu, 1, -1);
     }
   }

   if (lp_t->repeat.unit!=-1) {
     sp_vu=lp_t->repeat;
     if ((lp_t->sp_fl&0x0F0)!=0) {
       sp_vu.val=sym_vals[(lp_t->sp_fl&0x0F0)/0x010-1].f;
     }
     rc=scsize((char *) NULL, 'y', &(lp->repeat), &sp_vu, 1, -1);
   }
   if (lp->repeat==0){
     if (lp_fl==0) lp->repeat=0.1*get_ymax();
     else lp->repeat=0.05*get_ymax();
   }

   if (lp_t->pattern_set!=0) {
     if ((lp_t->sp_fl&0xF00)==0) {
       lp->pattern=lp_t->pattern;
     }else{
       val=sym_vals[(lp_t->sp_fl&0xF00)/0x100-1].f;
       lp->pattern=set_patt((int) (val+0.5));
     }
     if (lp->pattern==-1) {
       lp->pattern=set_sym_patt(dft, lp_fl);
     }
   } 
   if (lp_t->none!=0) lp->pattern=0;

   set_colour(&(lp->colour), lp_t->colour, lp_t->colour_set, 
              lp_t->colour_sp_fl);
}

/* get_colour - processes the colour string
              - returns the color structure and the set members 
              - returns negative if error occurs */

char *st_colours[6]={"red", "yellow", "green", "cyan", "blue", "magenta"};

int get_colour(ptr, colour, colour_set, colour_sp_fl, lpos, len, syval_fl)
char *ptr;
int lpos, *colour_set, len, *colour_sp_fl, syval_fl;
struct sp_colour *colour;
{
   int i, k, ok, t_fl;
   float val;

   ok=0;
   i=k=0;
   (void) getc_buff(ptr, &k, tmpbuff, 1000, ':');
   if (is_empty(tmpbuff)==0) {
     t_fl=scan_num(tmpbuff, lpos, syval_fl);
     if (t_fl>=0) {
       if (is_numeric(tmpbuff,0)!=0) {
         val=atof(tmpbuff);
       }else{
         for (i=0;i<6;i++) {
           if (l_comp(tmpbuff, st_colours[i])!=0) {
             val=60.0*i;
             break;
           }
         }
       }
       if (i==6) {
         sp_err(BADCOL, lpos, len);
         ok--;
       }else{
         colour->hue=val;
         *colour_set= *colour_set|1;
         *colour_sp_fl=(*colour_sp_fl&0xFF0)|(t_fl*0x001);
       }
     }
   }
  
   (void) getc_buff(ptr, &k, tmpbuff, 1000, ':');
   if (is_empty(tmpbuff)==0) {
     t_fl=scan_num(tmpbuff, lpos, syval_fl);
     if (t_fl>=0) {
       colour->sat=atof(tmpbuff);
       *colour_set= *colour_set|2;
       *colour_sp_fl=(*colour_sp_fl&0xF0F)|(t_fl*0x010);
     }
   }
   
   (void) getc_buff(ptr, &k, tmpbuff, 1000, ':');
   if (is_empty(tmpbuff)==0) {
     t_fl=scan_num(tmpbuff, lpos, syval_fl);
     if (t_fl>=0) {
       colour->bright=atof(tmpbuff);
       *colour_set= *colour_set|4;
       *colour_sp_fl=(*colour_sp_fl&0x0FF)|(t_fl*0x100);
     }
   }

   return(ok);
}

/* set_colour - sets the colour information structure according to the
                   definition
              - includes external values, if requested (illegal values
                   are not used) */

set_colour(colour, col_def, col_set, col_sp_fl)
struct sp_colour *colour, col_def;
int col_set, col_sp_fl;
{
   float val;

   if ((col_set&1)!=0) {
     if ((col_sp_fl&0x00F)==0) {
       val=col_def.hue;
     }else{
       val=sym_vals[(col_sp_fl&0x00F)/0x001-1].f;
     }
     colour->hue=val;
   }

   if ((col_set&2)!=0) {
     if ((col_sp_fl&0x0F0)==0) {
       val=col_def.sat;
     }else{
       val=sym_vals[(col_sp_fl&0x0F0)/0x010-1].f;
     }
     if (val<0.0) val=0.0;
     if (val>1.0) val=1.0;
     colour->sat=val;
   }

   if ((col_set&4)!=0) {
     if ((col_sp_fl&0xF00)==0) {
       val=col_def.bright;
     }else{
       val=sym_vals[(col_sp_fl&0xF00)/0x100-1].f;
     }
     if (val<0.0) val=0.0;
     if (val>1.0) val=1.0;
     colour->bright=val;
   }
}
#endif

int max_lines() 
{
   return (NUMLINES);
}

/* init_clip - initializes the clipping region and its boundary
             - flag is one if you really mean it
             - NOTE: the points are NOT copied - do not free */

init_clip(poly, border, bord_none, inout, flag)
struct sp_poly poly;
struct sp_linefill border;
int bord_none, inout;
{
   if (inout!=0) {
     if (inc_flag!=0) del_clip(1);
     inc_poly=poly;
     inc_bord=border;
     inc_none=bord_none;
     inc_flag=flag;
   }else{
     if (exc_flag!=0) del_clip(0);
     exc_poly=poly;
     exc_bord=border;
     exc_none=bord_none;
     exc_flag=flag;
   }
}

struct sp_poly tpx_poly;
struct sp_linefill tpx_border;
int tpx_flag, tpx_none;

/* save_clip - saves current clip pattern and turns off clip if fl non-zero
             - restores saved clip pattern if fl zero*/

save_clip(inout, fl)
int inout, fl;
{
   if (fl==0) {
     init_clip(tpx_poly, tpx_border, tpx_none, inout, tpx_flag);
   }else{
     if (inout!=0) {
       tpx_poly=inc_poly;
       tpx_border=inc_bord;
       tpx_none=inc_none;
       tpx_flag=inc_flag;
       inc_flag=0;
     }else{
       tpx_poly=exc_poly;
       tpx_border=exc_bord;
       tpx_none=exc_none;
       tpx_flag=exc_flag;
       exc_flag=0;
     }
   }
}

/* del_clip - deletes the clipping boundary, while outlining it
            - deletes inside clipping if inout non-zero  */

del_clip(inout)
int inout;
{
   if (inout!=0) {
     if ((inc_flag!=0)&&(inc_poly.n_points>0)) {
       inc_flag=0;
#ifndef SPACE
       if (inc_none==0) draw_polyline(inc_poly, 1, 0, &inc_bord, 0);
#endif
       xfree((char *) inc_poly.pts);
     }
     inc_poly.n_points=0;
   }else{
     if ((exc_flag!=0)&&(exc_poly.n_points>0)) {
       exc_flag=0;
#ifndef SPACE
       if (exc_none==0) draw_polyline(exc_poly, 1, 0, &exc_bord, 0);
#endif
       xfree((char *) exc_poly.pts);
     }
     exc_poly.n_points=0;
   }
}

/* tn_move - moves to the position (x,y) while resetting the 
	    line segment parameters                 */

COORD z_t_oldx, z_t_oldy;
int z_t_oldw;
struct sp_colour t_old_colour={-1.0, -1.0, -1.0};

tn_move(x, y, style)
COORD x, y;
struct sp_linefill *style;
{
   z_t_oldx=x;
   z_t_oldy=y;
   style->curr_ln=0.0;
}

/* tn_draw - draws to the position (x,y), clipping to internal
               and external poly's                  */

tn_draw(x, y, style)
COORD x, y;
struct sp_linefill *style;
{
   float val;

   if (style->pattern==0) return;
   check_style(style);

   val=style->curr_ln;
   line_branch(z_t_oldx, z_t_oldy, x, y, style->pattern, style->repeat,
               &val, 2);
   style->curr_ln=val;

   z_t_oldx=x;
   z_t_oldy=y;
}

/* t3_draw - draws to the position (x,y), bypassing 3d conversions and
               only clipping to the external polygon */

t3_draw(x, y, style)
COORD x, y;
struct sp_linefill *style;
{
   float draw_line();

   if (style->pattern==0) return;
   check_style(style);

   style->curr_ln=draw_line(z_t_oldx, z_t_oldy, x, y, style->pattern, 
       style->repeat, style->curr_ln, 0);

   z_t_oldx=x;
   z_t_oldy=y;
}

/* tn_fill - fill the polygon poly according to the style information */

tn_fill(poly, style)
struct sp_poly poly;
struct sp_linefill *style;
{
   float vec;

   if (style->pattern==0) return;
   check_style(style);

   vec=1.0/sqrt((double) 2.0);

   switch (style->pattern) {
     case 7 : poly_2d(poly, options.dim_mode);
              break;
     case 1 : hash_poly(poly, 1.0, 0.0, style->repeat);
              break;
     case 2 : hash_poly(poly, vec, vec, style->repeat);
              break;
     case 3 : hash_poly(poly, 0.0, 1.0, style->repeat);
              break;
     case 4 : hash_poly(poly, vec, -vec, style->repeat);
              break;
     case 5 : hash_poly(poly, 1.0, 0.0, style->repeat);
              hash_poly(poly, 0.0, 1.0, style->repeat);
              break;
     case 6 : hash_poly(poly, vec, vec, style->repeat);
              hash_poly(poly, vec, -vec, style->repeat);
              break;
     default: break;
   }
}

/* t3_fill - solid fill the polygon without transformation */

t3_fill(poly, style)
struct sp_poly poly;
struct sp_linefill *style;
{
   if (style->pattern==0) return;
   check_style(style);

   poly_2d(poly, 2);
}

/* check_style - check the style parameters and change if necessary */

check_style(style)
struct sp_linefill *style;
{
   if (style->width!=z_t_oldw) {
#ifndef SPACE
      change_width(style->width);
#else
      space_change_width(style->width);
#endif
      z_t_oldw=style->width;
   }
   if ((style->colour.hue!=t_old_colour.hue)||
            (style->colour.sat!=t_old_colour.sat)||
                   (style->colour.bright!=t_old_colour.bright)) {
#ifndef SPACE
      change_colour(style->colour);
#else
      space_change_colour(style->colour);
#endif
      t_old_colour=style->colour;
   }
}

hash_poly(poly, vec_x, vec_y, repeat)
struct sp_poly poly;
float vec_x, vec_y;
COORD repeat;
{
   float c, cmin, cmax, cmid, perp_x, perp_y, l, lmid, lmin, lmax;
   struct coordinate p1, p2;
   int i, nmin, nmax;
   float c_x, c_y;

   c_x=(poly.xmax+poly.xmin)/2.0;
   c_y=(poly.ymax+poly.ymin)/2.0;
   cmid=vec_x*c_y-vec_y*c_x;
   lmid=vec_x*c_x+vec_y*c_y;
   cmin=cmax=vec_x*poly.pts->y-vec_y*poly.pts->x;
   lmin=lmax=vec_x*poly.pts->x+vec_y*poly.pts->y;
   for (i=0;i<poly.n_points;i++) {
     c=vec_x*(poly.pts+i)->y-vec_y*(poly.pts+i)->x;
     if (c<cmin) cmin=c;
     else if (c>cmax) cmax=c;
     l=vec_x*(poly.pts+i)->x+vec_y*(poly.pts+i)->y;
     if (l<lmin) lmin=l;
     else if (l>lmax) lmax=l;
   }

   if (lmin==lmax) { lmin=lmid-2; lmax=lmid+2; }

   perp_x= -repeat*vec_y;
   perp_y=repeat*vec_x;
   nmin=(int) ((cmin-cmid)/repeat);
   nmax=(int) ((cmax-cmid)/repeat);

   for (i=nmin;i<=nmax;i++) {
     p1.x=c_x+1.2*(lmin-lmid)*vec_x+perp_x*i;
     p1.y=c_y+1.2*(lmin-lmid)*vec_y+perp_y*i;
     p2.x=c_x+1.2*(lmax-lmid)*vec_x+perp_x*i;
     p2.y=c_y+1.2*(lmax-lmid)*vec_y+perp_y*i;

     poly_inter(poly, p1, p2, p1, p2, 2, 1, 1, (COORD) 0, &c);
   }
}

/* NOTE: in both poly_line and poly_inter, the d_fl indicates the
         next step in the line output process
           0 - move_pt, draw_pt
           1 - draw_line
           2 - poly_line (with inc_poly)     */

/* poly_line - nicer front for poly_inter
             - draws the line from (xp1,yp1) to (xp2,yp2), clipping with poly
             - remaining flags are same as poly_inter */

poly_line(poly, xp1, yp1, xp2, yp2, d_fl, inout, line_t, rpt, lent)
struct sp_poly poly;
COORD xp1, xp2, yp1, yp2, rpt;
int d_fl, inout, line_t;
float *lent;
{
   struct coordinate p1, p2, p, q, tc;
   COORD dpx, dpy;
   int rv_fl;

   rv_fl=0;
   if (xp2>=xp1) {
     p.x=xp1; p.y=yp1;
     q.x=xp2; q.y=yp2;
   }else{
     q.x=xp1; q.y=yp1;
     p.x=xp2; p.y=yp2;
     xp1=p.x; yp1=p.y;
     xp2=q.x; yp2=q.y;
     rv_fl=1;
   }
   dpx=q.x-p.x;
   dpy=q.y-p.y;

   if (dpy>=0) {
     if ((dpx==0)&&(dpy==0)) dpx=100;
     while ((xp1>=poly.xmin)&&(yp1>=poly.ymin)) { xp1-=dpx; yp1-=dpy; }
     while ((xp2<=poly.xmax)&&(yp2<=poly.ymax)) { xp2+=dpx; yp2+=dpy; }
   }else{
     while ((xp1>=poly.xmin)&&(yp1<=poly.ymax)) { xp1-=dpx; yp1-=dpy; }
     while ((xp2<=poly.xmax)&&(yp2>=poly.ymin)) { xp2+=dpx; yp2+=dpy; }
   }

   p1.x=xp1-dpx; p2.x=xp2+dpx;
   p1.y=yp1-dpy; p2.y=yp2+dpy;
  
   if (rv_fl==1) {
     tc=p1; p1=p2; p2=tc;
     tc=p; p=q; q=tc;
   }

   poly_inter(poly, p1, p2, p, q, d_fl, inout, line_t, rpt, lent);
}

/* poly_inter - draws the intersecting line p1->p2, between the points
                  p and q (must be properly ordered, p closer to p1)
              - p1->p2 must encompass polygon and have non-zero length
              - inout non-zero for inside, zero for outside intersection
              - line_t, rpt and lent describe dashing patterns */

poly_inter(poly, p1, p2, p, q, d_fl, inout, line_t, rpt, lent)
struct sp_poly poly;
struct coordinate p1, p2, p, q;
int inout, d_fl, line_t;
float *lent;
COORD rpt;
{
   struct poly_int *ints;
   float mod_fact(), umin, umax, dx, dy;
   int i, cnt;
  
   dx=p.x-p1.x;
   dy=p.y-p1.y;
   umin=sqrt((double) (dx*dx+dy*dy));
   dx=q.x-p1.x;
   dy=q.y-p1.y;
   umax=sqrt((double) (dx*dx+dy*dy));

   poly_cut(poly, p1, p2, &ints, &cnt, 0);

   if ((cnt==0)||(umax<ints->mu)||(umin>(ints+cnt-1)->mu)) {
     if (inout==0) {
       line_branch(p.x, p.y, q.x, q.y, line_t, rpt, lent, d_fl);
     }
     xfree((char *) ints);
     return;
   }

   if (umin<ints->mu) {
     if (inout==0) {
       line_branch(p.x, p.y, ints->pt.x, ints->pt.y, line_t, rpt, lent, d_fl);
       i=1;
     }else{
       *lent=mod_fact((ints->mu-umin), rpt, *lent);
       i=0;
     }
   }else{
     i=0;
     if (inout==0) i=1;
     for (;i<(cnt-1);i+=2) {
       if (umin<(ints+i)->mu) {
         *lent=mod_fact(((ints+i)->mu-umin), rpt, *lent);
         break;
       }
       if (umin<(ints+i+1)->mu) {
         if (umax<(ints+i+1)->mu) {
           line_branch(p.x, p.y, q.x, q.y, line_t, rpt, lent, d_fl);
         }else{
           line_branch(p.x, p.y, (ints+i+1)->pt.x, (ints+i+1)->pt.y, 
                  line_t, rpt, lent, d_fl);
         }
         i+=2;
         break;
       }
     }
   }

   for (;i<(cnt-1);i+=2) {
     if (umax<(ints+i)->mu) {
       if (i!=0) *lent=mod_fact((umax-(ints+i-1)->mu), rpt, *lent);
       break;
     }
     if (umax<(ints+i+1)->mu) {
       line_branch((ints+i)->pt.x, (ints+i)->pt.y, q.x, q.y, 
              line_t, rpt, lent, d_fl);
       break;
     }else{
       line_branch((ints+i)->pt.x, (ints+i)->pt.y, (ints+i+1)->pt.x, 
               (ints+i+1)->pt.y, line_t, rpt, lent, d_fl);
       if (((i+2)<cnt)&&((ints+i+2)->mu<umax)) {
         *lent=mod_fact(((ints+i+2)->mu-(ints+i+1)->mu), rpt, *lent);
       }
     }
   }

   if ((umax>(ints+cnt-1)->mu)&&(inout==0)) {
     line_branch((ints+cnt-1)->pt.x, (ints+cnt-1)->pt.y, q.x, q.y, 
              line_t, rpt, lent, d_fl);
   }

   xfree((char *) ints);
}

/* line_branch - diverts to next draw procedure according to d_fl */

line_branch(xp1, yp1, xp2, yp2, line_t, rpt, lent, d_fl)
COORD xp1, yp1, xp2, yp2, rpt;
int line_t, d_fl;
float *lent;
{
   float draw_line();

   if (d_fl==2) {
     if (inc_flag==0) {
       *lent=draw_line(xp1, yp1, xp2, yp2, line_t, rpt, *lent, 1);
     }else{
       poly_line(inc_poly, xp1, yp1, xp2, yp2, 1, 1, line_t, rpt, lent);
     }
   }else if (d_fl==1) {
     *lent=draw_line(xp1, yp1, xp2, yp2, line_t, rpt, *lent, 1);
   }else if (d_fl==0) {
#ifndef SPACE
     move_pt(xp1, yp1);
     draw_pt(xp2, yp2);
#else
     space_move_pt(xp1, yp1);
     space_draw_pt(xp2, yp2);
#endif
   }
}

/* mod_fact - returns the new line distance after skipping over
               a distance d */

float mod_fact(d, rpt, lent)
float d, lent;
COORD rpt;
{
   int tt;

   tt=(int) (d/rpt+lent);
   lent=(lent+d/rpt)-tt;
   if ((lent>1.0)||(lent<0.0)) lent=0.0;
   return(lent);
}

/* draw_line - draws a line between (xp1,yp1) and (xp2,yp2)
             - follows the linestyle given by line_t, repeat length of rpt
             - starts at pattern fraction xi      
             - returns the pattern fraction at the end of the line 
             - fl_2d indicates 2d conversions to take place */

float draw_line(xp1, yp1, xp2, yp2, line_t, rpt, xi, fl_2d)
COORD xp1, yp1, xp2, yp2, rpt;
int line_t, fl_2d;
float xi;
{
   float tlen, sn, csn, dx, dy, pce, patts;
   COORD move_x, move_y;
   int drmv, t_pos;

   move_x=xp1;
   move_y=yp1;
   if (line_t==1) {
      line_2d(xp1, yp1, xp2, yp2, fl_2d);
   }
   if (line_t>1) {
      dx=xp2-xp1;
      dy=yp2-yp1;
      tlen=(float) sqrt((double) (dx*dx+dy*dy));
      patts=tlen/rpt;
      sn=dy/tlen;
      csn=dx/tlen;

      if ((xi>=1.0)||(xi<0.0)) xi=0.0;
      t_pos=0;
      drmv=1;
      while (lines[line_t-1][t_pos]!=-1) {
        if ((lines[line_t-1][t_pos]/RPTLN)>xi) break;
        drmv=1-drmv;
        t_pos++;
      }

      pce=0.0;
      while ((patts+xi)>(lines[line_t-1][t_pos]/RPTLN)) {
         pce=pce+(lines[line_t-1][t_pos]/RPTLN-xi)*rpt;
         patts=patts-(lines[line_t-1][t_pos]/RPTLN-xi);
         xi=lines[line_t-1][t_pos]/RPTLN;
         if (drmv==1) {
	    line_2d(move_x, move_y, (COORD) (xp1+pce*csn),
               (COORD) (yp1+pce*sn), fl_2d);
         }else{
            move_x=xp1+pce*csn;
            move_y=yp1+pce*sn;
         }
         drmv=1-drmv;
         if (lines[line_t-1][++t_pos]==-1) {
	    t_pos=0;
	    xi=0.0;
	    drmv=1;
         }
      }
      if (drmv==1) {
         line_2d(move_x, move_y, xp2, yp2, fl_2d);
      }
      xi=xi+patts;
   }
   return(xi);
}

/* point_in_poly - returns non-zero if point is inside poly(gon)
                 - all edge points are considered to be inside! */

int point_in_poly(poly, point)
struct sp_poly poly;
struct coordinate point;
{
   struct coordinate end_point;
   int count;

   end_point.y=point.y;
   end_point.x=MAX_CRD;

   poly_cut(poly, point, end_point, (struct poly_int **) NULL, &count, 1);

   return(count%2);
}

/* poly_cut - returns the intersection points between the line p->q
               (encompasses polygon) and the polygon poly 
            - points are sorted in order of increasing mu
            - does not obtain intersections if flag!=0 (counts only),
                also specially handles coincidence (point_in_poly) */

poly_cut(poly, p, q, inters, count, flag)
struct sp_poly poly;
struct coordinate p, q;
struct poly_int **inters;
int *count, flag;
{
   int rc, i, tx, cnt, coin_fl, coin_p, test, testb, f_ln_coin;
   struct big_int b_mul();
   struct poly_int *ints, tmp;
   struct coordinate tpt;
   COORD dpx, dpy;
   float dx, dy;

   if (flag==0) {
     ints=(struct poly_int *) xalloc((unsigned int) 
             (sizeof(struct poly_int)*(poly.n_points+10)),
             "Out of memory for polygon intercepts.");
   }
  
   dpx=q.x-p.x;
   dpy=q.y-p.y;
   f_ln_coin=coin_fl=cnt=0;
   for (i=0;i<poly.n_points;i++) {
     
     rc=inter_2(*(poly.pts+i), *(poly.pts+i+1), p, q, &tpt, 1);
     if (rc>0) continue;

     if (flag!=0) {         /* special coincidence tests */
       if (rc==-3) {
         *count=1; 
         return;
       }
       if (rc==-1) {
         if ((poly.pts+i)->x<(poly.pts+i+1)->x) {
           if ((p.x>=(poly.pts+i)->x)&&(p.x<=(poly.pts+i+1)->x)) {
             *count=1; 
             return;
           }
         }else{
           if ((p.x<=(poly.pts+i)->x)&&(p.x>=(poly.pts+i+1)->x)) {
             *count=1; 
             return;
           }
         }
       }
       if (rc==-2) {
         if ((tpt.x==p.x)&&(tpt.y==p.y)) {
           *count=1; 
           return;
         }
       }
     }

     if (rc==-2) {
       if (f_ln_coin>0) {
         f_ln_coin= -i; continue;
       }
       if (coin_fl==0) coin_p=i;

       test=b_diff(b_mul((long) dpx, (long) ((poly.pts+i+1)->y-p.y)),
                   b_mul((long) dpy, (long) ((poly.pts+i+1)->x-p.x)));
       if (coin_p==0) {
         testb=test*b_diff(b_mul((long) dpx, (long) 
                  ((poly.pts+poly.n_points-1)->y-p.y)),
                  b_mul((long) dpy, (long) 
                  ((poly.pts+poly.n_points-1)->x-p.x)));
       }else{
         testb=test*b_diff(b_mul((long) dpx, (long) 
                   ((poly.pts+coin_p-1)->y-p.y)),
                   b_mul((long) dpy, (long) 
                   ((poly.pts+coin_p-1)->x-p.x)));
       }
       if ((i==0)&&(testb==0)) continue;
       if ((coin_fl==0)&&(testb>0)) continue;
       else if ((coin_fl!=0)&&(testb<0)) { coin_fl=0; continue; }
       if (coin_fl!=0) coin_fl=0;
     }else if (rc==-1) {
       if ((coin_fl!=0)||(f_ln_coin>0)) continue;
       if (i==0) {
         f_ln_coin=1; continue;
       }
       if (f_ln_coin<=0) {
         coin_fl=1; coin_p=i;
       }
     }else{
       if ((coin_fl!=0)||(f_ln_coin>0)) (void) fprintf(stderr,
                          "Point algorithm breakdown %i\n",rc);
     }
     if (flag==0) {
       (ints+cnt)->pt=tpt;
       dx=tpt.x-p.x;
       dy=tpt.y-p.y;
       (ints+cnt)->mu=sqrt((double) (dx*dx+dy*dy));
       (ints+cnt)->poly_ln=i;
     }
     cnt++;
   }

   if ((coin_fl!=0)||(f_ln_coin<0)) {
     if (coin_fl==0) { 
       coin_p=poly.n_points;
       if (flag==0) {
         (ints+cnt)->pt= *(poly.pts);
         dx=(poly.pts)->x-p.x;
         dy=(poly.pts)->y-p.y;
         (ints+cnt)->mu=sqrt((double) (dx*dx+dy*dy));
         (ints+cnt)->poly_ln=0;
       }
       cnt++;
     }
     test=b_diff(b_mul((long) dpx, (long) ((poly.pts+coin_p-1)->y-p.y)),
                 b_mul((long) dpy, (long) ((poly.pts+coin_p-1)->x-p.x)));
     testb=test*b_diff(b_mul((long) dpx, (long) 
                  ((poly.pts-f_ln_coin+1)->y-p.y)),
                  b_mul((long) dpy, (long) 
                  ((poly.pts-f_ln_coin+1)->x-p.x)));
     if (testb>0) {
       if (flag==0) {
         (ints+cnt)->pt= *(poly.pts-f_ln_coin);
         dx=(poly.pts-f_ln_coin)->x-p.x;
         dy=(poly.pts-f_ln_coin)->y-p.y;
         (ints+cnt)->mu=sqrt((double) (dx*dx+dy*dy));
         (ints+cnt)->poly_ln= -f_ln_coin;
       }
       cnt++;
     }
   }

   if (flag!=0) {
     *count=cnt;
     return;
   }

   if (cnt!=0) {
     tx=1;
     while (tx>0) {
       tx=0;
       for (i=0;i<(cnt-1);i++) {
         if ((ints+i)->mu>(ints+i+1)->mu) {
           tmp= *(ints+i);
           *(ints+i)= *(ints+i+1);
           *(ints+i+1)=tmp;
           tx++;
         }
       }
     }
   }
   
   tx=cnt/2.0;
   if ((tx*2)!=cnt) {
     (void) fprintf(stderr,"Internal error in polygon clipping routine.");
     (void) fprintf(stderr,"  %i intersection(s) found.\n", cnt);
     (void) fprintf(stderr,"p (%i,%i) q (%i,%i)\n",p.x,p.y,q.x,q.y);
     for (i=0;i<cnt;i++) {
       (void) fprintf(stderr,"%i: (%i,%i) # %i\n", i, (ints+i)->pt.x, 
                (ints+i)->pt.y, (ints+i)->poly_ln);
     }
     (void) fprintf(stderr,"bounds %i %i -> %i %i\n", poly.xmin, poly.ymin, 
         poly.xmax, poly.ymax);
     for (i=0;i<poly.n_points;i++) {
       tpt.x=tpt.y= -1;
       rc=inter_2(*(poly.pts+i), *(poly.pts+i+1), p, q, &tpt, 1);
       (void) fprintf(stderr,"POLY %i %i -> rc %i, (%i, %i)\n", (poly.pts+i)->x,
         (poly.pts+i)->y, rc, tpt.x, tpt.y);
     }
     (void) fprintf(stderr,"POLY %i %i\n", (poly.pts+i)->x,
         (poly.pts+i)->y);
     cnt=0;
   }

   *inters=ints;
   *count=cnt;
}

/* inter_2 - finds the intersection of the lines p1--p2 and q1--q2
           - p2 is NOT a valid intersection point if flag!=0
           - returns intersection pt or a positive value
               if no intersection occurs 
           - returns -1 if coincident lines (and p1) 
           - returns -2 if exact end-point intersection on p1 or p2
           - returns -3 if exact end-point intersection on q1 or q2
           - returns zero otherwise */

inter_2(p1, p2, qi1, qi2, pt, flag)
struct coordinate p1, p2, qi1, qi2, *pt;
int flag;
{
   struct big_int b_mul();
   struct coordinate q1,q2;
   COORD dpx, dpy, dqx, dqy;
   int test, testb, testc, testd, teste;
   double mu,z;

   dpx=p2.x-p1.x; dpy=p2.y-p1.y;
   dqx=qi2.x-qi1.x; dqy=qi2.y-qi1.y;

   q1.x=qi1.x-p1.x; q1.y=qi1.y-p1.y;
   q2.x=qi2.x-p1.x; q2.y=qi2.y-p1.y;

   test=b_diff(b_mul((long) dpx,(long) q1.y),
               b_mul((long) dpy,(long) q1.x));
   testb=b_diff(b_mul((long) dpx,(long) q2.y),
                b_mul((long) dpy,(long) q2.x));
   if ((test*testb)>0) return(1);

   testc=b_diff(b_mul((long) dqy,(long) q1.x),
                b_mul((long) dqx,(long) q1.y));
   testd=b_diff(b_mul((long) dqx,(long) (dpy-q1.y)),
                b_mul((long) dqy,(long) (dpx-q1.x)));
   if ((testc*testd)>0) return(1);

   teste=b_diff(b_mul((long) dpx,(long) dqy), 
                b_mul((long) dpy,(long) dqx));
   if (teste==0) { 
     if ((dqx==0)&&(dqy==0)) {
       if ((dpx==0)&&(dpy==0)) {
         if ((q1.x!=0)||(q2.x!=0)) return(1);
         /* else - see testd */
       }else{
         if ((qi1.x!=p2.x)||(qi1.y!=p2.y)) { *pt=qi1; return(-3); }
         /* else - see testd */
       }
     }else{
       if ((dpx!=0)||(dpy!=0)) { *pt=p1; return(-1); }
       /* else - see testd */
     }
   }

   if (testd==0) { 
      if (flag==0) { *pt=p2; return(-2); }
      else return(1);
   }
   if (testc==0) { *pt=p1; return(-2); }
   if (test==0) { *pt=qi1; return(-3); }
   if (testb==0) { *pt=qi2; return(-3); }

   mu=(((double) q1.x)*dqy-((double) q1.y)*dqx)/
               (((double) dpx)*dqy-((double) dpy)*dqx);

   z=p1.x+mu*dpx;
   if (z>0) pt->x=z+0.5;
   else pt->x=z-0.5;
   z=p1.y+mu*dpy;
   if (z>0) pt->y=z+0.5;
   else pt->y=z-0.5;

   return(0);
}

/* same_side - returns non-zero if q1 and q2 are on the same side of p1-p2
             - points on p1-p2 are not counted (ie MUST cross) */

int same_side(p1, p2, q1, q2)
struct coordinate p1, p2, q1, q2;
{
   struct big_int b_mul();
   COORD dpx, dpy;
   int test, testb;

   dpx=p2.x-p1.x; dpy=p2.y-p1.y;

   q1.x=q1.x-p1.x; q1.y=q1.y-p1.y;
   q2.x=q2.x-p1.x; q2.y=q2.y-p1.y;

   test=b_diff(b_mul((long) dpx,(long) q1.y),
               b_mul((long) dpy,(long) q1.x));
   testb=b_diff(b_mul((long) dpx,(long) q2.y),
                b_mul((long) dpy,(long) q2.x));
   if ((test*testb)>0) return(1);
   return(0);
}

/* go_left - returns non-zero if point q is to the left of vector p1-p2
           - q inline also returns true */

int go_left(p1, p2, q)
struct coordinate p1, p2, q;
{
   int res;
   float dot_prod;
   struct big_int b_mul();

   res=b_diff(b_mul((long) (q.x-p2.x), (long) (p1.y-p2.y)),
              b_mul((long) (p1.x-p2.x), (long) (q.y-p2.y)));

   if (res>0) return(1);
   if (res<0) return(0);

   dot_prod=((float) p2.x-p1.x)*((float) q.x-p2.x)+
                 ((float) p2.y-p1.y)*((float) q.y-p2.y);
   if (dot_prod>0) return(1);
   return(0);
}

#define BMASK 0x7FFF

/* b_mul - multiply together two longs into a big integer */

struct big_int b_mul(xp1,xp2)
long xp1, xp2;
{
   struct big_int out;
   unsigned long a1, a2, t1, t2;

   if ((xp1==0)||(xp2==0)) {
     out.sign=out.upper=out.lower=0;
     return(out);
   }else if (xp1<0) {
      a1= -xp1;
      if (xp2<0) { a2= -xp2; out.sign=1;}
      else { a2=xp2; out.sign= -1;}
   }else{
      a1=xp1;
      if (xp2<0) { a2= -xp2; out.sign= -1;}
      else { a2=xp2; out.sign=1;}
   }

   out.upper=(a1>>15)*(a2>>15);
   t1=(a1>>15)*(a2&BMASK);
   t2=(a1&BMASK)*(a2>>15);
   out.lower=(a1&BMASK)*(a2&BMASK)+((t1&BMASK)<<15)+((t2&BMASK)<<15);
   out.upper=(a1>>15)*(a2>>15)+(t1>>15)+(t2>>15);
   out.upper=out.upper+(out.lower>>30);
   out.lower=out.lower&(BMASK<<15|BMASK);

   return(out);
}

/* b_diff - returns sign of difference a-b of two big_ints */

int b_diff(a,b)
struct big_int a,b;
{
   if (a.sign<0) {
     if (b.sign>=0) return(-1);
     if (a.upper>b.upper) return(-1);
     if (b.upper>a.upper) return(1);
     if (a.lower>b.lower) return(-1);
     if (b.lower>a.lower) return(1);
   }else if (a.sign==0) return(-b.sign);
   else{
     if (b.sign<=0) return(1);
     if (a.upper>b.upper) return(1);
     if (b.upper>a.upper) return(-1);
     if (a.lower>b.lower) return(1);
     if (b.lower>a.lower) return(-1);
   }
   return(0);
}
   
/* line_2d - draws a line from the location (xp1,yp1) to (xp2,yp2), either
               directly (2dmode) or on the specified plane (3dmode) 
           - bypasses 2d conversion if fl_2d zero */

line_2d(xp1, yp1, xp2, yp2, fl_2d)
COORD xp1, yp1, xp2, yp2;
int fl_2d;
{
   struct coordinate pt1, pt2, scale2d();
   float jk;

   if ((options.dim_mode==2)||(fl_2d==0)) {
     pt1.x=xp1;
     pt1.y=yp1;
     pt2.x=xp2;
     pt2.y=yp2;
   }else{
     pt1=scale2d(xp1, yp1);
     pt2=scale2d(xp2, yp2);
   }

   if (exc_flag==0) {
#ifndef SPACE
     move_pt(pt1.x, pt1.y);
     draw_pt(pt2.x, pt2.y);
#else
     space_move_pt(pt1.x, pt1.y);
     space_draw_pt(pt2.x, pt2.y);
#endif
   }else{
     poly_line(exc_poly, pt1.x ,pt1.y, pt2.x, pt2.y, 0, 0, 1, (COORD) 0, &jk);
   }
}

/* poly_2d - fills the given poly directly (2dmode) or on the specified
                 plane (3dmode) */

poly_2d(poly, mode)
struct sp_poly poly;
int mode;
{
   struct sp_poly newpoly;
   struct coordinate *pt, scale2d();
   int i;

   pt=(struct coordinate *) xalloc ((unsigned int)
                    (sizeof(struct coordinate)*(poly.n_points+10)),
                    "Unable to allocate memory for polygon scaling.");

   newpoly=poly;
   newpoly.pts=pt;
 
   if (mode==3) {
     for (i=0;i<=newpoly.n_points;i++) {
       *(newpoly.pts+i)=scale2d((poly.pts+i)->x, (poly.pts+i)->y);
     }
   }else{
     for (i=0;i<=newpoly.n_points;i++) {
       *(newpoly.pts+i)= *(poly.pts+i);
     }
   }

#ifndef SPACE
   fill_poly(newpoly);
#else
   space_fill_poly(newpoly);
#endif
   xfree((char *) pt);
}

/* scale2d - translate a 2d normal point to the 3d display plane, and
               return the perspective screen coordinate */

struct coordinate scale2d(x, y)
COORD x, y;
{
   struct coord3d point;
   struct coordinate md_3d();
   COORD get_xmax(), get_ymax();
   double xp,yp;

   xp=((float) x)/get_xmax()*options.size_2d.x;
   yp=((float) y)/get_ymax()*options.size_2d.y;
   point.x=options.orig_2d.x+xp*options.vec_2dx.x+yp*options.vec_2dy.x;
   point.y=options.orig_2d.y+xp*options.vec_2dx.y+yp*options.vec_2dy.y;
   point.z=options.orig_2d.z+xp*options.vec_2dx.z+yp*options.vec_2dy.z;
   return(md_3d(point, 0));
}

/* md_3d - returns the 2d screen coordinate corresponding to the
             3d location point (using perspective transform)
         - adds the 3d origin if org_fl non-zero */

struct coordinate md_3d(point, org_fl)
struct coord3d point;
int org_fl;
{
   COORD xp, yp;
   float r, xc, yc, zc, dot_product();
   struct coordinate out;

   if (org_fl!=0) {
      point.x=point.x+options.orig_3d.x;
      point.y=point.y+options.orig_3d.y;
      point.z=point.z+options.orig_3d.z;
   }

   xc=dot_product(point, options.tvecx);
   yc=dot_product(point, options.tvecy);
   zc=dot_product(point, options.tvecz);
   r=dot_product(options.tvecz, options.eyeball);

   xp=xc/(r-zc)*options.screen/100.0*options.dev_xmax+
          options.dev_xmax*options.prj_x/100.0;
   yp=yc/(r-zc)*options.screen/100.0*options.dev_xmax+
          options.dev_ymax*options.prj_y/100.0;

   out.x=xp;
   out.y=yp;
   return(out);
}

/* cross_product - returns the cross product of vec1 and vec2 in result */

cross_product(vec1, vec2, result)
struct coord3d vec1, vec2, *result;
{
   result->x=vec1.y*vec2.z-vec1.z*vec2.y;
   result->y= -1.0*(vec1.x*vec2.z-vec1.z*vec2.x);
   result->z=vec1.x*vec2.y-vec1.y*vec2.x;
}

/* renormalize - normalizes the 3d vector vec */

renormalize(vec)
struct coord3d *vec;
{
   double length;

   length=vec->x*vec->x+vec->y*vec->y+vec->z*vec->z;
   length=sqrt(length);
   if (length!=0.0) {
     vec->x=vec->x/length;
     vec->y=vec->y/length;
     vec->z=vec->z/length;
   }
}

/* dot_product - returns the dot product of the 3d vectors vec1 and vec2 */

float dot_product(vec1, vec2)
struct coord3d vec1, vec2;
{
   float val;
   
   val=vec1.x*vec2.x+vec1.y*vec2.y+vec1.z*vec2.z;
   return(val);
}
