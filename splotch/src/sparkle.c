/******************************************************************
                          sPLOTch!

  Sparkle.c - the doodle command processor, meant for drawing
   non-graphical figures and the like.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"
#include "sparkle.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

#define PNT_BLK 200

extern struct opt_def options;
extern char argbuff[], inbuff[], tmpbuff[];
extern double m_pi, deg_to_rad;
extern int polar_fl;

/* doodle - just like it sounds, doodle a set of objects on screen */

doodle() 
{
  int rc, nerrs, dtype, com[4], t_unit, t_fl, i, nlim, dir, arrow;
  COORD xp0, yp0, xp1, yp1, xp2, yp2, perm_ht, x_c, y_c;
  char *ptr, *copy_buff();
  float r, rx, ry, theta;
  float theta_i, theta_f;
  struct lnfill_def ln_def, fl_def;
  struct sp_linefill line, fill;
  struct sp_poly polygon, t_poly;
  struct united jnk_vu;
  struct coordinate cpt;
  struct inst_key *inst;
  struct sp_text txt;

  error_off();
  nerrs=0;
  t_unit=options.def_unit;
  options.def_unit=GPCT;
  nerrs=0;
  while (end_of_prog()==0) {
    read_data_line(argbuff, 1000);
    if ((end_comms(argbuff,"enddoodle")!=0)||(is_empty(argbuff)!=0)) break;
    com[1]=0;
    yank(argbuff, com, inbuff);
    dtype=atoi(inbuff);
    if (dtype==D_END) break;
    t_fl=0;
    switch(dtype) {
      case D_NO_OUTCLIP:
          del_clip(0);
          break;
      case D_NO_INCLIP:
          del_clip(1);
          break;
      case D_BOX_OUTCLIP: t_fl++;
      case D_ELL_OUTCLIP: t_fl++;
      case D_BOX_INCLIP: t_fl++;
      case D_ELL_INCLIP: t_fl++;
      case D_BOX : t_fl++;
      case D_ELLIPSE : 
          if (grab_val(argbuff, com, 'x', 0, &xp0, &nerrs)<0) break;
          if (grab_val(argbuff, com, 'y', 0, &yp0, &nerrs)<0) break;
          if (grab_val(argbuff, com, 'x', 1, &xp1, &nerrs)<0) break;
          yank(argbuff, com, inbuff);
          if (strcmp(inbuff, "sq")==0) {
            yp1=xp1;
          }else{
            rc=scsize(inbuff, 'y', &yp1, &jnk_vu, 1, 0);
            if (rc<0) {nerrs++; break;}
          }
          yank(argbuff, com, inbuff);
          theta_i=atof(inbuff)*deg_to_rad;
          if (grab_style(argbuff, com, &ln_def, &nerrs)<0) break;
          init_linefill(&line, -1, 0);
          line_fill((char *) NULL, &ln_def, &line, 0, 0, 1, 0);
          if (t_fl<2) {
            if (grab_style(argbuff, com, &fl_def, &nerrs)<0) break;
            init_linefill(&fill, -1, 1);
            line_fill((char *) NULL, &fl_def, &fill, 0, 1, 1, 0);
          }
          if ((t_fl&1)==0) {
            init_poly(&polygon, -1);
            theta=0.0;
            while (theta<(2.0*m_pi)) {
              rx=xp1*cos((double) theta)/2.0;
              ry=yp1*sin((double) theta)/2.0;
              cpt.x=xp0+rx;
              cpt.y=yp0+ry;
              poly_add_point(&polygon, cpt);
              r=hypot((double) rx, (double) ry);
              theta+=options.curv_res/r;
            }
            (polygon.pts+polygon.n_points)->x=polygon.pts->x;
            (polygon.pts+polygon.n_points)->y=polygon.pts->y;
          }else{
            init_poly(&polygon, 10);
            make_polybox(&polygon, (COORD) (xp0-xp1/2), (COORD) (yp0-yp1/2),
                         (COORD) (xp0+xp1/2), (COORD) (yp0+yp1/2));
          }
          if (theta_i!=0.0) {
            for (i=0;i<=polygon.n_points;i++) {
              rx=(polygon.pts+i)->x-xp0;
              ry=(polygon.pts+i)->y-yp0;
              cpt.x=rx*cos((double) theta_i)-ry*sin((double) theta_i)+xp0;
              cpt.y=rx*sin((double) theta_i)+ry*cos((double) theta_i)+yp0;
              *(polygon.pts+i)=cpt;
            }
          }
          cent_poly(&polygon);
          if (t_fl<2) {
            if (fl_def.none==0) tn_fill(polygon,&fill);
            if (ln_def.none==0) draw_polyline(polygon, 1, 0, &line, 0);
          }
          if ((t_fl==2)||(t_fl==3)) {
            init_clip(polygon, line, ln_def.none, 1, 1);
          }
          if ((t_fl==4)||(t_fl==5)) {
            init_clip(polygon, line, ln_def.none, 0, 1);
          }
          if (t_fl<2) xfree((char *) polygon.pts);
          break;
      case D_TEXT :
          perm_ht=options.sht;
          if (grab_val(argbuff, com, 'x', 0, &xp0, &nerrs)<0) break;
          if (grab_val(argbuff, com, 'y', 0, &yp0, &nerrs)<0) break;
          yank(argbuff, com, inbuff);
          if (l_comp(inbuff, "none")==0) {
            rc=scsize(inbuff, 'y', &yp1, &jnk_vu, 1, 0);
            if (rc<0) {nerrs++; break;}
            if (jnk_vu.unit==-2) options.sht=options.sht*jnk_vu.val;
            else options.sht=yp1;
          }
          ptr=copy_buff(argbuff+com[1]);
          init_text(&txt);
          inst=(struct inst_key *) NULL;
          speak(ptr, &txt, 0, 1, 0, &inst, 1, 0);
          xfree(ptr);
          yp2=0;
          halign_text(inst, xp0, xp0, yp0, &yp2, 1, 0, T_LEFT);
          del_inst_tree(inst);
          options.sht=perm_ht;
          break;
      case D_SPLINE_OUTCLIP: t_fl++;
      case D_SPLINE_INCLIP: t_fl++;
      case D_POLY_SPLINE: t_fl++;
      case D_LINE_SPLINE: t_fl++;
      case D_POLY_OUTCLIP: t_fl++;
      case D_POLY_INCLIP: t_fl++;
      case D_POLYGON: t_fl++;
      case D_POLYLINE:
          if ((t_fl%4)<2) {
            yank(argbuff, com, inbuff);
            arrow=atoi(inbuff);
          }
          if (grab_style(argbuff, com, &ln_def, &nerrs)<0) break;
          init_linefill(&line, -1, 0);
          line_fill((char *) NULL, &ln_def, &line, 0, 0, 1, 0);
          if ((t_fl==1)||(t_fl==5)) {
            if (grab_style(argbuff, com, &fl_def, &nerrs)<0) break;
            init_linefill(&fill, -1, 1);
            line_fill((char *) NULL, &fl_def, &fill, 0, 1, 1, 0);
          }
          if (grab_polygon(argbuff, com, &polygon, &nerrs)<0) break;
          if (t_fl>=4) {
            if (t_fl==4) b_norm_spline(polygon, &t_poly, 0);
            else b_norm_spline(polygon, &t_poly, 1);
            xfree((char *) polygon.pts);
            polygon=t_poly;
            cent_poly(&polygon);
            t_fl-=4;
          }
          if (t_fl<2) {
            if ((t_fl==1)&&(fl_def.none==0)) tn_fill(polygon, &fill);
            if (ln_def.none==0) draw_polyline(polygon, t_fl, arrow, &line, 0);
          }
          if (t_fl==2) {
            init_clip(polygon, line, ln_def.none, 1, 1);
          }else if (t_fl==3) {
            init_clip(polygon, line, ln_def.none, 0, 1);
          }
          if (t_fl<2) xfree((char *) polygon.pts);
          break;
      case D_ARC_CENT: t_fl++;
          if (grab_val(argbuff, com, 'x', 0, &x_c, &nerrs)<0) break;
          if (grab_val(argbuff, com, 'y', 0, &y_c, &nerrs)<0) break;
          if (grab_val(argbuff, com, 'y', 1, &perm_ht, &nerrs)<0) break;
          r=perm_ht;
          yank(argbuff, com, inbuff);
          theta_i=atof(inbuff)*deg_to_rad;
          yank(argbuff, com, inbuff);
          theta_f=atof(inbuff)*deg_to_rad;
          yank(argbuff, com, inbuff);
          dir=atoi(inbuff);
      case D_ARC_PTS:
          if (dtype==D_ARC_PTS) {
            if (grab_val(argbuff, com, 'x', 0, &xp0, &nerrs)<0) break;
            if (grab_val(argbuff, com, 'y', 0, &yp0, &nerrs)<0) break;
            if (grab_val(argbuff, com, 'x', 0, &xp1, &nerrs)<0) break;
            if (grab_val(argbuff, com, 'y', 0, &yp1, &nerrs)<0) break;
            if (grab_val(argbuff, com, 'x', 0, &xp2, &nerrs)<0) break;
            if (grab_val(argbuff, com, 'y', 0, &yp2, &nerrs)<0) break;
            if (arc_center((float) xp0, (float) yp0, (float) xp1, (float) yp1,
                 (float) xp2, (float) yp2, &x_c, &y_c)<0) break;
            r=hypot((double) (yp0-y_c), (double) (xp0-x_c));
            theta_i=atan2((double) (yp0-y_c), (double) (xp0-x_c));
            theta_f=atan2((double) (yp2-y_c), (double) (xp2-x_c));
            theta=atan2((double) (yp1-y_c), (double) (xp1-x_c));
            if (theta_i>theta_f) {
              if ((theta<theta_i)&&(theta>theta_f)) dir= -1;
              else dir=1;
            }else{
              if ((theta>theta_i)&&(theta<theta_f)) dir=1;
              else dir= -1;
            }
          }
          yank(argbuff, com, inbuff);
          arrow=atoi(inbuff);
          if (grab_style(argbuff, com, &ln_def, &nerrs)<0) break;
          init_linefill(&line, -1, 0);
          line_fill((char *) NULL, &ln_def, &line, 0, 0, 1, 0);
          if (dir>0) {
            if (theta_f<theta_i) theta_i=theta_i-2.0*m_pi;
          }else{
            if (theta_i<theta_f) theta_f=theta_f-2.0*m_pi;
          }
          nlim=dir*(theta_f-theta_i)*r/options.curv_res;
          rx=(theta_f-theta_i)/nlim;
          init_poly(&polygon, (nlim+10));
          polygon.n_points=nlim+1;
          for (i=0;i<(nlim+1);i++) {
            (polygon.pts+i)->x=x_c+r*cos((double) theta_i+i*rx);
            (polygon.pts+i)->y=y_c+r*sin((double) theta_i+i*rx);
          }
          draw_polyline(polygon, 0, arrow, &line, 0);
          break;
      default : 
          nerrs++;
          break;
    }
  }
  error_on();
  options.def_unit=t_unit;

  (void) sprintf(tmpbuff,"   Doodling complete. %i error(s) encountered.",
                 nerrs);
  add_note(tmpbuff);
}

/* grab_polygon - grabs a series of point definitions from program file
                - first entry in argbuff is number of points (junk after)
                - returns negative if error occurs */

int grab_polygon(ptr, com, polygon, nerrs)
char *ptr;
int com[4], *nerrs;
struct sp_poly *polygon;
{
   struct united jnk_vu;
   int np, rc, i, err_fl;
   COORD crd_sz;

   yank(ptr, com, inbuff);
   np=atoi(inbuff);
   if ((is_empty(inbuff)!=0)||(np<=0)) {
     *nerrs= *nerrs+1;
     return(-1);
   }

   init_poly(polygon, (np+10));
   polygon->n_points=np;

   err_fl=0;
   inbuff[0]='\0';
   for (i=0;i<np;i++) {
     yank(argbuff, com, inbuff);
     while (is_empty(inbuff)!=0) {
       if (end_of_prog()!=0) return(-1);
       read_data_line(argbuff, 1000);
       com[1]=0;
       yank(argbuff, com, inbuff);
     }
     rc=scsize(inbuff, 'x', &crd_sz, &jnk_vu, 0, 0);
     if (rc<0) err_fl++;
     else{
       if ((rc>MID_UNIT)&&(i!=0)) {
         (polygon->pts+i)->x=(polygon->pts+i-1)->x+crd_sz;
       }else{
         (polygon->pts+i)->x=crd_sz;
       }
     }
     yank(argbuff, com, inbuff);
     rc=scsize(inbuff, 'y', &crd_sz, &jnk_vu, 0, 0);
     if ((is_empty(inbuff)!=0)||(rc<0)) err_fl++;
     else{
       if ((rc>MID_UNIT)&&(i!=0)) {
         (polygon->pts+i)->y=(polygon->pts+i-1)->y+crd_sz;
       }else{
         (polygon->pts+i)->y=crd_sz;
       }
     }
   }

   if (err_fl!=0){
     *nerrs= *nerrs+err_fl; 
     return(-1);
   }

   *(polygon->pts+polygon->n_points)= *polygon->pts;
   cent_poly(polygon);
   return(0);
}

/* draw_polyline - draws a line (fl==0) or polygon (fl==1) 
                 - arrow is 1 for open, 2 for close, 3 for both 
                 - fl_3d is non-zero if 3d calling routine*/

draw_polyline(poly, fl, arrow, style, fl_3d)
struct sp_poly poly;
int fl, arrow, fl_3d;
struct sp_linefill *style;
{
   int i;
   struct coordinate crd;

   if ((arrow&1)!=0) {
     i=0;
     crd= *poly.pts;
     while(((poly.pts+i)->x==crd.x)&&((poly.pts+i)->y==crd.y)&&
           (i<poly.n_points)) i++;
     draw_arrow(style, crd.x, crd.y, (COORD) ((poly.pts+i)->x-poly.pts->x), 
                (COORD) ((poly.pts+i)->y-poly.pts->y), 0, fl_3d);
   }

   tn_move(poly.pts->x, poly.pts->y, style);
   for (i=1;i<(poly.n_points+fl);i++) {
     if (fl_3d==0) tn_draw((poly.pts+i)->x, (poly.pts+i)->y, style);
     else t3_draw((poly.pts+i)->x, (poly.pts+i)->y, style);
   }

   if ((arrow&2)!=0) {
     i=poly.n_points+fl-1;
     crd= *(poly.pts+i);
     while(((poly.pts+i)->x==crd.x)&&((poly.pts+i)->y==crd.y)&&(i>0)) i--;
     draw_arrow(style, crd.x, crd.y, (COORD) (crd.x-(poly.pts+i)->x), 
                (COORD) (crd.y-(poly.pts+i)->y), 1, fl_3d);
   } 
}

/* draw_arrow - draws an open/close arrow along the indicated line */

draw_arrow(style, xp0, yp0, del_x, del_y, open_fl, fl_3d)
COORD xp0, yp0, del_x, del_y;
struct sp_linefill *style;
int open_fl, fl_3d;
{
   double ang;

   ang=atan2((double) del_y, (double) del_x)+open_fl*m_pi;

   ang+=options.arr_ang*deg_to_rad;
   tn_move((COORD) (xp0+options.arrow*cos(ang)), 
           (COORD) (yp0+options.arrow*sin(ang)), style);
   if (fl_3d==0) tn_draw(xp0, yp0, style);
   else t3_draw(xp0, yp0, style);

   ang-=2.0*options.arr_ang*deg_to_rad;
   tn_move((COORD) (xp0+options.arrow*cos(ang)), 
           (COORD) (yp0+options.arrow*sin(ang)), style);
   if (fl_3d==0) tn_draw(xp0, yp0, style);
   else t3_draw(xp0, yp0, style);
}

/* grab_style - grabs the line/fill style from the buffer */

int grab_style(ptr, com, style, nerrs)
char *ptr;
int com[4], *nerrs;
struct lnfill_def *style;
{
   int rc;
   COORD crd_sz;

   init_lf_def(style);
   yank(ptr, com, inbuff);
   if (strcmp(inbuff,"none")==0) {
     style->none=1;
     return(0);
   }
   rc=scsize(inbuff, 'y', &crd_sz, &(style->width), 1, 0);
   if (rc<0) {*nerrs= *nerrs+1; return(-1);};
   yank(ptr, com, inbuff);
   rc=scsize(inbuff, 'y', &crd_sz, &(style->repeat), 1, 0);
   if (rc<0) {*nerrs= *nerrs+1; return(-1);};
   yank(ptr, com, inbuff);
   rc=get_colour(inbuff, &(style->colour), &(style->colour_set), 
                 &(style->colour_sp_fl), 0, 0, 0);
   if (rc<0) {*nerrs= *nerrs+1; return(-1);};
   yank(ptr, com, inbuff);
   style->pattern=atoi(inbuff);
   style->pattern_set=1;
   style->none=0;
 
#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
     (void) fprintf(deb_log,"width %g %i repeat %g %i colour %i pattern %i\n",
        style->width.val, style->width.unit, style->repeat.val,
        style->repeat.unit, style->colour_set, style->pattern);
     (void) fflush(deb_log);
  }
#endif

   return(0);
}

/* grab_val - grabs a value from the buffer */

int grab_val(ptr, com, dir, rel, val, nerrs)
char *ptr, dir;
int com[4], *nerrs, rel;
COORD *val;
{
   struct united jnk_vu;
   int rc;

   yank(ptr, com, inbuff);
   rc=scsize(inbuff, dir, val, &jnk_vu, rel, 0);
   if (rc<0) *nerrs= *nerrs+1;
   return(rc);
}

/* make_polybox - creates a box polygon from the corner information 
                - points must already be allocated! */

make_polybox(poly, ll_x, ll_y, ur_x, ur_y)
struct sp_poly *poly;
COORD ll_x, ll_y, ur_x, ur_y;
{
  poly->n_points=4;
  (poly->pts+0)->x=(poly->pts+3)->x=ll_x;
  (poly->pts+0)->y=(poly->pts+1)->y=ll_y;
  (poly->pts+1)->x=(poly->pts+2)->x=ur_x;
  (poly->pts+2)->y=(poly->pts+3)->y=ur_y;
  *(poly->pts+4)= *(poly->pts+0);
  cent_poly(poly);
}

/* arc_center - find the center of the arc given the three points
              - returns negative if bad points */

arc_center(xp0, yp0, xp1, yp1, xp2, yp2, cx, cy)
float xp0, yp0, xp1, yp1, xp2, yp2; 
COORD *cx, *cy;
{
   float l0, l1, l2, d0x, d1x, d0y, d1y, den;

   l0=xp0*xp0+yp0*yp0;
   l1=xp1*xp1+yp1*yp1;
   l2=xp2*xp2+yp2*yp2;
   d0x=xp0-xp1;
   d0y=yp0-yp1;
   d1x=xp1-xp2;
   d1y=yp1-yp2;

   den=2.0*(d0x*d1y-d1x*d0y);
   if (den==0.0) return(-1);

   *cx=(d1y*(l0-l1)-d0y*(l1-l2))/den;
   if (d0y!=0.0) {
     *cy=(l0-l1-(*cx)*2.0*d0x)/(2.0*d0y);
   }else if (d1y!=0.0) {
     *cy=(l1-l2-(*cx)*2.0*d1x)/(2.0*d1y);
   }else return(-1);

   return(0);
}

/* cent_poly - find the max/min limits for a polygon */

cent_poly(polygon)
struct sp_poly *polygon;
{
   int i;
   COORD xmin, xmax, ymin, ymax;

   xmin=xmax=(polygon->pts)->x;
   ymin=ymax=(polygon->pts)->y;

   for (i=1;i<(polygon->n_points);i++) {
     if ((polygon->pts+i)->x>xmax) xmax=(polygon->pts+i)->x;
     if ((polygon->pts+i)->y>ymax) ymax=(polygon->pts+i)->y;
     if ((polygon->pts+i)->x<xmin) xmin=(polygon->pts+i)->x;
     if ((polygon->pts+i)->y<ymin) ymin=(polygon->pts+i)->y;
   }
   polygon->xmin=xmin;
   polygon->xmax=xmax;
   polygon->ymin=ymin;
   polygon->ymax=ymax;
}

/* end_comms - scans line buff for a specific end indicator in end */

int end_comms(buff, end)
char *buff, *end;
{
   int i,c,ok;
   char ch;

   i=ok=0;
   while((buff[i]==' ')|(buff[i]=='\t')) i++;
   for (c=0;c<strlen(end);c++) {
      ch=clower(*(buff+c+i));
      if (ch!=*(end+c)) break;
   }
   if (c==strlen(end)) ok=1;
   return(ok);
}

struct sp_fourpoint {struct coordinate c1, c2, c3, c4; } four_map[50];

c_cent(c, c1,c2)
struct coordinate *c, c1, c2;
{

  c->x=(c1.x+c2.x)/2;
  c->y=(c1.y+c2.y)/2;
}

/* b_norm_spline - build a normal spline from input polygon points
                 - closes if close non-zero 
                 - IMPORTANT - calling polygon must have 3 more points
                         allocated than indicated*/

b_norm_spline(poly_in, poly_out, close)
struct sp_poly poly_in, *poly_out;
int close;
{
   int i, end;
   struct coordinate c, c1, c2;
   struct sp_fourpoint t_pt;

   if (options.curv_res<=0) {
     copy_poly(poly_in, poly_out);
     return;
   }

   init_poly(poly_out, -1);

   if (close==0) end=poly_in.n_points;
   else{
     end=poly_in.n_points+2;
     c= *(poly_in.pts+1);
     *(poly_in.pts+end-1)=c;
   }

   for (i=0;i<end;i++) {
     c= *(poly_in.pts+i);
     if (i==0) {
       c1=c;
       if (close==0) poly_add_point(poly_out, *(poly_in.pts));
       continue;
     }
     if (i==1) {
       c2=c;
       c_cent(&(t_pt.c1), c1, c2);
       if (close==0) {
         c_cent(&(t_pt.c2), t_pt.c1, c2);
       }else{
         t_pt.c2.x=(c1.x+3*c2.x)/4; t_pt.c2.y=(c1.y+3*c2.y)/4;
       }
       continue;
     }
     c1=c2; c2=c;
     c_cent(&(t_pt.c4), c1, c2);
     c_cent(&(t_pt.c3), c1, t_pt.c4);

     b_smooth(t_pt, poly_out);

     c=t_pt.c4;
     t_pt.c1=c;
     c_cent(&(t_pt.c2), t_pt.c1, c2);
   }

   if (close==0) {
     poly_add_point(poly_out, t_pt.c1);
     poly_add_point(poly_out, c2);
   }else{
     *(poly_out->pts+poly_out->n_points)= *(poly_out->pts);
   }
}

/*  b_smooth - quadratic B spline smoothing routine   
             - inverted tree bifurcation to allow continuous
                drawing from (left) to (right)
             - based on Chakin's algorithm.......

        George Merrill Chakin, Computer Graphics and 
            Image Processing, 3 (1974), 346.

 */

b_smooth(pt_orig, poly)
struct sp_fourpoint pt_orig;
struct sp_poly *poly;
{
   struct sp_fourpoint t_pt, x_pt, *ptr;
   struct coordinate cp;
   int nstore, limit;

   limit=options.curv_res;

   ptr=four_map;
   nstore=0;

   *(ptr++)=pt_orig; nstore++;

   while (nstore>0) {
      t_pt= *(--ptr); nstore--;

      c_cent(&cp, t_pt.c2, t_pt.c3);

      if (((abs((int) (cp.x-t_pt.c1.x))<limit)&&
           (abs((int) (cp.y-t_pt.c1.y))<limit)&&
            (abs((int) (cp.x-t_pt.c4.x))<limit)&&
             (abs((int) (cp.y-t_pt.c4.y))<limit))||
              (nstore>46)) {
        poly_add_point(poly, t_pt.c1);
        poly_add_point(poly, cp);
      }else{
        x_pt=t_pt;
        t_pt.c1=cp;
        c_cent(&(t_pt.c2), cp, t_pt.c3);
        c_cent(&(t_pt.c3), t_pt.c3, t_pt.c4);
        x_pt.c4=cp;
        c_cent(&(x_pt.c3), cp, x_pt.c2);
        c_cent(&(x_pt.c2), x_pt.c1, x_pt.c2);
        *(ptr++)=t_pt; nstore++;
        *(ptr++)=x_pt; nstore++;
      }
   }
}

/* conv_poly - convolute a polygon from hax/vax to real space 
             - pays attention to 180 degree swing
             - close polygon if flag 1, open if flag 0 */

conv_poly(poly_in, poly_out, flag)
struct sp_poly poly_in, *poly_out;
int flag;
{
  float rmax, delth;
  int i, nstore;
  struct sp_fourpoint tc, tp;

  if ((polar_fl==0)||(options.curv_res<=0)) {
    copy_poly(poly_in, poly_out);
    if (polar_fl!=0) {
      for (i=0;i<(poly_in.n_points+flag);i++) { 
        pol_crd(&((poly_out->pts+i)->x), &((poly_out->pts+i)->y), 0);
      }
    }
  }else{
    init_poly(poly_out, -1);

    poly_add_point(poly_out, *(poly_in.pts));
    for (i=0;i<(poly_in.n_points+flag-1);i++) {
      tc.c1= *(poly_in.pts+i);
      tc.c2= *(poly_in.pts+i+1);
      delth=(tc.c2.x-tc.c1.x)/1000.0;
      delth=fmod((double) delth, (double) 360.0);
      if (delth>180.0) delth=delth-360.0;
      if (delth<-180.0) delth=delth+360.0;
      tc.c2.x=tc.c1.x+delth*1000.0;

      nstore=0;
      *(four_map+nstore++)=tc;
      while (nstore>0) {
        tc= *(four_map+(--nstore));
        rmax=tc.c1.y;
        if (tc.c2.y>rmax) rmax=tc.c2.y;
        rmax=fabs(rmax*deg_to_rad*(tc.c1.x-tc.c2.x)/1000.0);
        if ((rmax<options.curv_res)||(nstore>46)) {
          poly_add_point(poly_out, tc.c2);
        }else{
          c_cent(&(tp.c1), tc.c1, tc.c2);
          tp.c2=tc.c2;
          *(four_map+nstore++)=tp;
          tp.c1=tc.c1;
          c_cent(&(tp.c2), tc.c1, tc.c2);
          *(four_map+nstore++)=tp;
        }
      }
    }

    for (i=0;i<poly_out->n_points;i++) {
      pol_crd(&((poly_out->pts+i)->x), &((poly_out->pts+i)->y), 0);
    }
    if (flag!=0) {
      poly_out->n_points=poly_out->n_points-1;
      (poly_out->pts+poly_out->n_points)->x=(poly_out->pts)->x;
      (poly_out->pts+poly_out->n_points)->y=(poly_out->pts)->y;
    }
  }
}

/* init_poly - create a polygon point structure
             - if n_points<0 use PNT_BLK */

init_poly(poly, n_points)
struct sp_poly *poly;
int n_points;
{
   if (n_points<=0) n_points=PNT_BLK;

   poly->pts=(struct coordinate *) xalloc((unsigned int)
                (n_points*sizeof(struct coordinate)),
                "Unable to allocate polygon point storage.");
   poly->nlim=n_points;
   poly->n_points=0;
}

/* poly_add_point - add the specifed point to the polygon
                  - automatically expands storage if exceeded and nlim
                       positive                    */

poly_add_point(poly, point)
struct sp_poly *poly;
struct coordinate point;
{
   if (poly->nlim<0) return;

   if (poly->n_points>(poly->nlim-10)) {
     poly->nlim=poly->nlim+PNT_BLK;
     poly->pts=(struct coordinate *) xrealloc( 
           (char *) poly->pts, 
           (unsigned int) (poly->nlim*sizeof(struct coordinate)),
           "Unable to reallocate memory for polygon expansion.");
   }
   *(poly->pts+poly->n_points)=point;
   poly->n_points=poly->n_points+1;
}

/* copy_poly - copy a polygon allocation */

copy_poly(poly_in, poly_out)
struct sp_poly poly_in, *poly_out;
{
   int i;
   
   i=poly_in.nlim;
   if (i<=0) i=poly_in.n_points+10;
   init_poly(poly_out, i);
   poly_out->n_points=poly_in.n_points;
   for (i=0;i<=poly_in.n_points;i++) {
     *(poly_out->pts+i)= *(poly_in.pts+i);
   }
}
