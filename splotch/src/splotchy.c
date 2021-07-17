/******************************************************************
                          sPLOTch!

  Splotchy - puts the points on the graph, with a character label
	     and curve fitting type according to symbols.  Also
	     contains the manual linestyle code.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

extern struct sym_def *symbols[101];
extern struct opt_def options;
extern double m_pi, deg_to_rad;
extern int polar_fl;

extern COORD llc_x, llc_y, urc_x, urc_y;
extern char argbuff[];
extern struct pair_v sym_vals[11];

struct sp_colour d_colset[2]={{0.0,0.0,0.2}, {0.0,0.0,0.8}};

/* fix_symbol - fixes up a symbol given by sy_ptr, according to crd, etc. */

fix_symbol(sy_ptr, nsym, crd, index)
struct sym_def *sy_ptr;
int nsym, index;
struct var_set *crd;
{
   struct inst_key *copy_inst_tree();
   char *tptr, *copy_buff();
   struct sp_text txt;

   if ((nsym>100)||(symbols[nsym]==(struct sym_def *) NULL)||
       (symbols[nsym]->set==0)) nsym=0;
   *sy_ptr= *(symbols[nsym]);

   if (sy_ptr->pnt_set!=0) {
     sy_ptr->pnt=copy_inst_tree(sy_ptr->pnt);
   }else{
     if (symbols[0]->pnt_set!=0) {
       sy_ptr->pnt=copy_inst_tree(symbols[0]->pnt);
       sy_ptr->pnt_set=1;
     }
   }

   if (sy_ptr->label_set!=0) {
     sy_ptr->label=copy_inst_tree(sy_ptr->label);
   }else{
     if (crd!=(struct var_set *) NULL) {
     if (index<0) {
       (void) sprintf(argbuff, "'%s(%s)'", crd->name[1], crd->name[0]);
     }else{
       (void) sprintf(argbuff, "'%s(%s), %s=%i'", crd->name[1], crd->name[0],
                  crd->name[2], index);
     }
     tptr=copy_buff(argbuff);
     init_text(&txt);
     sy_ptr->label=(struct inst_key *) NULL;
     speak(tptr, &txt, 0, 1, 0, &(sy_ptr->label), 1, 0);
     xfree(tptr);
     sy_ptr->label_set=1;
     }
   }

   if (sy_ptr->line_set==0) sy_ptr->linestyle=symbols[0]->linestyle;
   if (sy_ptr->fill_set==0) sy_ptr->fillstyle=symbols[0]->fillstyle;
   if (sy_ptr->inter_set==0) {
     sy_ptr->inter=symbols[0]->inter;
     if (sy_ptr->sort_set==0) sy_ptr->sort_fl=symbols[0]->sort_fl;
   }
   if (sy_ptr->arrow_set==0) sy_ptr->arrow=symbols[0]->arrow;
   if (sy_ptr->special.unit==-1) {
     sy_ptr->special=symbols[0]->special;
     sy_ptr->spec_spec=symbols[0]->spec_spec;
   }
}

/* plot_table - converts data into coordinates, then plots it
              - uses all data points if index negative, otherwise uses
                  matching data points */

plot_table(datas, xaxis, yaxis, crd, nsym, index, d_clips, cl_flag)
struct sp_data *datas;
struct axis_def *xaxis, *yaxis;
struct var_set *crd;
double d_clips[4];
int nsym, index, cl_flag;
{
   struct sp_poly dat_poly, mid_poly, fin_poly, t_poly;
   struct sp_linefill line, fill;
   int *n_map, i, use_fl, tf_fl, cnt, offset_fl;
   double val;
   COORD xpp, ypp, xp0, yp0, crd_sz, tpx, tpy, get_xmax(), get_ymax();
   struct sym_def loc_sym;
   struct coordinate crd_t, box[10];
   struct united sp_vu;

   fix_symbol(&loc_sym, nsym, crd, index);
   make_polydat(datas, &dat_poly, xaxis, yaxis, crd, &n_map, index,
                d_clips, cl_flag);
   doublesort(dat_poly.pts, n_map, dat_poly.n_points, loc_sym.sort_fl);

   clear_specs();
   init_linefill(&line, nsym, 0);
   init_linefill(&fill, nsym, 1);
   line_fill((char *) NULL, &(loc_sym.linestyle), &line, 0, 0, nsym, 1);
   line_fill((char *) NULL, &(loc_sym.fillstyle), &fill, 0, 1, nsym, 1);
   get_offset(&loc_sym, &xp0, &yp0);

   tf_fl=use_fl=offset_fl=0;
   switch(loc_sym.inter) {
     case 7:  tf_fl++;
     case 4:  tf_fl++;
     case 1:  conv_poly(dat_poly, &mid_poly, (tf_fl>1)?1:0);
              use_fl=1;
              if (tf_fl==1) {
                val=0.0;
                if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
                pos_inter(yaxis, val, 'y', &crd_sz, 1);
                if (crd_sz==MAX_CRD) crd_sz=llc_y;
                add_under(dat_poly, &mid_poly, crd_sz);
              }
              do_fpoly(mid_poly, &loc_sym, &line, &fill, tf_fl);
              break;
     case 8:  tf_fl++;
     case 5:  tf_fl++;
     case 2:  init_poly(&mid_poly, (dat_poly.n_points+10));
              mid_poly.n_points=dat_poly.n_points;
              for (i=0;i<=dat_poly.n_points;i++) {
                crd_t= *(dat_poly.pts+i);
                if (polar_fl!=0) pol_crd(&(crd_t.x), &(crd_t.y), 0);
                *(mid_poly.pts+i)=crd_t;
              }
              b_norm_spline(mid_poly, &fin_poly, (tf_fl>1)?1:0);
              use_fl=3;
              if (tf_fl==1) {
                val=0.0;
                if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
                pos_inter(yaxis, val, 'y', &crd_sz, 1);
                if (crd_sz==MAX_CRD) crd_sz=llc_y;
                add_under(dat_poly, &fin_poly, crd_sz);
              }
              do_fpoly(fin_poly, &loc_sym, &line, &fill, tf_fl);
              break;
     case 6:  tf_fl++;
     case 3:  cub_spline(dat_poly, &mid_poly);
              conv_poly(mid_poly, &fin_poly, 0);
              use_fl=3;
              if (tf_fl==1) {
                val=0.0;
                if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
                pos_inter(yaxis, val, 'y', &crd_sz, 1);
                if (crd_sz==MAX_CRD) crd_sz=llc_y;
                add_under(mid_poly, &fin_poly, crd_sz);
              }
              do_fpoly(fin_poly, &loc_sym, &line, &fill, tf_fl);
              break;
     case 21: tf_fl++;
     case 9:  val=0.0;
              if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
              pos_inter(xaxis, val, 'x', &crd_sz, 1);
              if (crd_sz==MAX_CRD) crd_sz=llc_x;
              if (tf_fl!=0) {
                init_poly(&mid_poly, -1);
                use_fl=3;
              }
              tpx=(3*dat_poly.pts->y-(dat_poly.pts+1)->y)/2;
              for (i=0;i<dat_poly.n_points;i++) {
                if (tf_fl==0) {
                  stack_specs(datas, i, n_map, crd);
                  track_lines(&loc_sym, nsym, &line, &fill, 1);
                }
                if (i==(dat_poly.n_points-1)) {
                  tpy=(3*(dat_poly.pts+i)->y-(dat_poly.pts+i-1)->y)/2;
                }else{ 
                  tpy=((dat_poly.pts+i)->y+(dat_poly.pts+i+1)->y)/2;
                }
                if (tf_fl==0) {
                  draw_bar(crd_sz, tpx, (dat_poly.pts+i)->x, tpy,
                     &line, &fill, &loc_sym);
                }else{
                  crd_t.x=(dat_poly.pts+i)->x;
                  crd_t.y=tpx;
                  poly_add_point(&mid_poly, crd_t);
                  crd_t.y=tpy;
                  poly_add_point(&mid_poly, crd_t);
                }
                tpx=tpy;
              }
              if (tf_fl!=0) {
                crd_t.x=crd_sz;
                poly_add_point(&mid_poly, crd_t);
                crd_t.y=(mid_poly.pts)->y;
                poly_add_point(&mid_poly, crd_t);
                (mid_poly.pts+mid_poly.n_points)->x=mid_poly.pts->x;
                (mid_poly.pts+mid_poly.n_points)->y=mid_poly.pts->y;
                conv_poly(mid_poly, &fin_poly, 1);
                do_fpoly(fin_poly, &loc_sym, &line, &fill, tf_fl);
              }else offset_fl=1;
              break;
     case 20: tf_fl++;
     case 10: val=0.0;
              if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
              pos_inter(yaxis, val, 'y', &crd_sz, 1);
              if (crd_sz==MAX_CRD) crd_sz=llc_y;
              if (tf_fl!=0) {
                init_poly(&mid_poly, -1);
                use_fl=3;
              }
              tpx=(3*dat_poly.pts->x-(dat_poly.pts+1)->x)/2;
              for (i=0;i<dat_poly.n_points;i++) {
                if (tf_fl==0) {
                  stack_specs(datas, i, n_map, crd);
                  track_lines(&loc_sym, nsym, &line, &fill, 1);
                }
                if (i==(dat_poly.n_points-1)) {
                  tpy=(3*(dat_poly.pts+i)->x-(dat_poly.pts+i-1)->x)/2;
                }else{ 
                  tpy=((dat_poly.pts+i)->x+(dat_poly.pts+i+1)->x)/2;
                }
                if (tf_fl==0) {
                  draw_bar(tpx, crd_sz, tpy, (dat_poly.pts+i)->y, 
                     &line, &fill, &loc_sym);
                }else{
                  crd_t.x=tpx;
                  crd_t.y=(dat_poly.pts+i)->y;
                  poly_add_point(&mid_poly, crd_t);
                  crd_t.x=tpy;
                  poly_add_point(&mid_poly, crd_t);
                }
                tpx=tpy;
              }
              if (tf_fl!=0) {
                conv_poly(mid_poly, &fin_poly, 0);
                add_under(mid_poly, &fin_poly, crd_sz);
                do_fpoly(fin_poly, &loc_sym, &line, &fill, tf_fl);
              }
              break;
     case 12: pos_inter(yaxis, (double) 0.0, 'y', &tpx, 1);
              if (tpx==MAX_CRD) tpx=llc_y;
              for (i=0;i<dat_poly.n_points;i++) {
                if (polar_fl==0) crd_sz=0.02*get_xmax();
                else crd_sz=18000;
                stack_specs(datas, i, n_map, crd);
                if (loc_sym.special.unit!=-1) {
                  sp_vu=loc_sym.special;
                  if (loc_sym.spec_spec!=0) 
                       sp_vu.val=sym_vals[loc_sym.spec_spec-1].f;
                  if (polar_fl==0) {
                    (void) scsize((char *) NULL, 'x', &crd_sz, &sp_vu, 1, -1);
                    if (crd_sz==MAX_CRD) crd_sz=0.02*get_xmax();
                  }else{
                    if (sp_vu.unit==-2) crd_sz=sp_vu.val*1000;
                    else crd_sz=sp_vu.val*3600;
                  }
                }
                track_lines(&loc_sym, nsym, &line, &fill, 1);
                draw_bar(((dat_poly.pts+i)->x-crd_sz/2), tpx,  
                   ((dat_poly.pts+i)->x+crd_sz/2), (dat_poly.pts+i)->y,
                   &line, &fill, &loc_sym);
              }
              offset_fl=1;
              break;
     case 11: pos_inter(xaxis, (double) 0.0, 'x', &tpx, 1);
              if (tpx==MAX_CRD) tpx=llc_x;
              crd_sz=0.02*get_ymax();
              for (i=0;i<dat_poly.n_points;i++) {
                stack_specs(datas, i, n_map, crd);
                if (loc_sym.special.unit!=-1) {
                  sp_vu=loc_sym.special;
                  if (loc_sym.spec_spec!=0) 
                       sp_vu.val=sym_vals[loc_sym.spec_spec-1].f;
                  (void) scsize((char *) NULL, 'y', &crd_sz, &sp_vu, 1, -1);
                  if (crd_sz==MAX_CRD) crd_sz=0.02*get_ymax();
                }
                track_lines(&loc_sym, nsym, &line, &fill, 1);
                draw_bar(tpx, ((dat_poly.pts+i)->y-crd_sz/2),
                   (dat_poly.pts+i)->x, ((dat_poly.pts+i)->y+crd_sz/2),
                   &line, &fill, &loc_sym);
              }
              offset_fl=1;
              break;
     case 15: tf_fl++;
              fin_poly.pts=box;
              fin_poly.nlim= -1;
     case 14: tf_fl++;
     case 13: init_poly(&mid_poly, -1);
              cnt=0;
              for (i=0;i<dat_poly.n_points;i+=3) {
                if ((polar_fl==0)||(tf_fl==0)) crd_sz=0.01*get_xmax();
                else crd_sz=3600;
                stack_specs(datas, cnt, n_map, crd);
                track_lines(&loc_sym, nsym, &line, &fill, 0);
                if (loc_sym.special.unit!=-1) {
                  sp_vu=loc_sym.special;
                  if (loc_sym.spec_spec!=0) 
                       sp_vu.val=sym_vals[loc_sym.spec_spec-1].f;
                  if ((polar_fl==0)||(tf_fl==0)) {
                    (void) scsize((char *) NULL, 'y', &crd_sz, &sp_vu, 1, -1);
                    if (crd_sz==MAX_CRD) crd_sz=0.02*get_ymax();
                  }else{
                    if (sp_vu.unit==-2) crd_sz=sp_vu.val*1000;
                    else crd_sz=sp_vu.val*3600;
                  }
                  crd_sz=crd_sz/2;
                }
                crd_t= *(dat_poly.pts+i);
                poly_add_point(&mid_poly, crd_t);
                if (tf_fl==0) {
                  tpx=(dat_poly.pts+i+1)->x;
                  tpy=(dat_poly.pts+i+2)->x;
                  tp_move(tpx, (crd_t.y+crd_sz), &line);
                  tp_draw(tpx, (crd_t.y-crd_sz), &line);
                  tp_move(tpy, (crd_t.y+crd_sz), &line);
                  tp_draw(tpy, (crd_t.y-crd_sz), &line);
                  tp_move(tpx, crd_t.y, &line);
                  tp_draw(tpy, crd_t.y, &line);
                }else if (tf_fl==1) {
                  tpx=(dat_poly.pts+i+1)->y;
                  tpy=(dat_poly.pts+i+2)->y;
                  tp_move((crd_t.x+crd_sz), tpx, &line);
                  tp_draw((crd_t.x-crd_sz), tpx, &line);
                  tp_move((crd_t.x+crd_sz), tpy, &line);
                  tp_draw((crd_t.x-crd_sz), tpy, &line);
                  tp_move(crd_t.x, tpx, &line);
                  tp_draw(crd_t.x, tpy, &line);
                }else{
                  make_polybox(&fin_poly, (dat_poly.pts+i+1)->x,
                   (dat_poly.pts+i+1)->y, (dat_poly.pts+i+2)->x,
                   (dat_poly.pts+i+2)->y);
                  conv_poly(fin_poly, &t_poly, 1);
                  cent_poly(&t_poly);
                  tn_fill(t_poly, &fill);
                  draw_polyline(t_poly, 1, 0, &line, 0);
                  xfree((char *) t_poly.pts);
                }
                cnt++;
              }
              offset_fl=2;
              xfree((char *) dat_poly.pts);
              dat_poly=mid_poly;
              break;
     case 16: val=0.0;
              if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
              pos_inter(yaxis, val, 'y', &crd_sz, 1);
              if (crd_sz==MAX_CRD) crd_sz=llc_y;
              for (i=0;i<dat_poly.n_points;i++) {
                stack_specs(datas, i, n_map, crd);
                track_lines(&loc_sym, nsym, &line, &fill, 0);
                draw_needle((dat_poly.pts+i)->x, crd_sz,
                    (dat_poly.pts+i)->x, (dat_poly.pts+i)->y, &line, &loc_sym);
              }
              offset_fl=2;
              break;
     case 17: conv_poly(dat_poly, &mid_poly, 0);
              val=0.5;
              if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
              offset_poly(mid_poly, &loc_sym);
              use_fl=1;
              label_poly(mid_poly, loc_sym.label, (struct axis_def *) NULL,
                  &line, loc_sym.arrow, -1, 0, val);
              break;
     case 18: val=0.0;
              if (loc_sym.special.unit!=-1) val=loc_sym.special.val;
              pos_inter(xaxis, val, 'x', &crd_sz, 1);
              if (crd_sz==MAX_CRD) crd_sz=llc_x;
              for (i=0;i<dat_poly.n_points;i++) {
                stack_specs(datas, i, n_map, crd);
                track_lines(&loc_sym, nsym, &line, &fill, 0);
                draw_needle(crd_sz, (dat_poly.pts+i)->y,
                    (dat_poly.pts+i)->x, (dat_poly.pts+i)->y, &line, &loc_sym);
              }
              break;
     case 19: if (polar_fl!=0) {xp0=0; yp0=0;}
              tp_move(((dat_poly.pts)->x+xp0), ((dat_poly.pts->y)+yp0), &line);
              for (i=1;i<dat_poly.n_points;i++) {
                stack_specs(datas, (i-1), n_map, crd);
                track_lines(&loc_sym, nsym, &line, &fill, 0);
                tp_draw(((dat_poly.pts+i)->x+xp0), ((dat_poly.pts+i)->y+xp0), 
                  &line);
              }
              break;
     default: offset_fl=2;
              break;
   }
   if ((use_fl&1)!=0) xfree((char *) mid_poly.pts);
   if ((use_fl&2)!=0) xfree((char *) fin_poly.pts);
 
   clear_specs();
   if ((loc_sym.pnt_none==0)&&(loc_sym.pnt_set!=0)) {
     for (i=0;i<dat_poly.n_points;i++) {
       stack_specs(datas, i, n_map, crd);
       xpp=(dat_poly.pts+i)->x;
       ypp=(dat_poly.pts+i)->y;
       pol_crd(&xpp, &ypp, 0);
       if (offset_fl!=2) {
         if (offset_fl==1) get_offset(&loc_sym, &xp0, &yp0);
         xpp=xpp+xp0; ypp=ypp+yp0;
       }
       tpx=0;
       halign_text(loc_sym.pnt, xpp, xpp, ypp, &tpx, 0, 1, T_CENTER);
     }
   }

   xfree((char *) dat_poly.pts);
}

/*  do_fpoly - do a filled/outlined/labelled polygon */

do_fpoly(poly, loc_sym, line, fill, tf_fl)
struct sp_poly poly;
struct sym_def *loc_sym;
struct sp_linefill *line, *fill;
int tf_fl;
{
    COORD crd_sz;
  
    offset_poly(poly, loc_sym);
    cent_poly(&poly);
    if ((tf_fl!=0)&&(loc_sym->fillstyle.none==0)) {
      tn_fill(poly, fill);
    }
    if (loc_sym->linestyle.none==0) {
      draw_polyline(poly, (tf_fl>0)?1:0, loc_sym->arrow, line, 0);
    }
    if ((tf_fl!=0)&&(loc_sym->label_none==0)) {
      crd_sz=0;
      halign_text(loc_sym->label, poly.xmin, poly.xmax,
          (poly.ymin+poly.ymax)/2, &crd_sz, 0, 0,
          T_CENTER);
    }
}

/* add_under - add the underside filling to the polygon */

add_under(pre_poly, poly, crd_sz)
struct sp_poly pre_poly, *poly;
COORD crd_sz;
{
   struct coordinate crd, *ptr;
   COORD x0, xf, crdabs();
   int i, np;

   if (polar_fl==0) {
     ptr=poly->pts+poly->n_points;
     crd.x=(ptr-1)->x;
     crd.y=crd_sz;
     *(ptr++)=crd;
     crd.x=(poly->pts)->x;
     *(ptr++)=crd;
     crd.y=(poly->pts)->y;
     *(ptr++)=crd;
     poly->n_points=poly->n_points+2;
   }else{
     x0=(pre_poly.pts+pre_poly.n_points-1)->x;
     xf=(pre_poly.pts)->x;
     if (options.curv_res>0) {
       np=crdabs(x0-xf)/(options.curv_res*1000/(crd_sz*deg_to_rad))+1;
       if (np<2) np=2;
     }else{
       np=2;
     }
     for (i=0;i<np;i++) {
       crd.x=(xf-x0)/(np-1.0)*i+x0;
       crd.y=crd_sz;
       pol_crd(&(crd.x), &(crd.y), 0);
       poly_add_point(poly, crd);
     }
     crd= *(poly->pts);
     *(poly->pts+poly->n_points)=crd;
   }
}

/* track_lines - track variable line/fill definitions */

track_lines(sy_ptr, nsym, line, fill, f_fl)
struct sym_def *sy_ptr;
int nsym, f_fl;
struct sp_linefill *line, *fill;
{
  line_fill((char *) NULL, &(sy_ptr->linestyle), line, 0, 0, nsym, 1);
  if (f_fl!=0) 
     line_fill((char *) NULL, &(sy_ptr->fillstyle), fill, 0, 1, nsym, 1);
}

/* draw_bar - just being lazy and using the old routine */

draw_bar(bxl, byl, bxr, byr, line, fill, sy_ptr)
COORD bxl, byl, bxr, byr;
struct sym_def *sy_ptr;
struct sp_linefill *line, *fill;
{
   struct sp_poly polya, polyb;
   struct coordinate crds[10];

   polya.pts=crds;
   polya.nlim= -1;
   make_polybox(&polya, bxl, byl, bxr, byr);
   conv_poly(polya, &polyb, 1);
   offset_poly(polyb, sy_ptr);
   if (sy_ptr->fillstyle.none==0) {
     cent_poly(&polyb);
     tn_fill(polyb, fill);
   }
   if (sy_ptr->linestyle.none==0) {
     draw_polyline(polyb, 1, 0, line, 0);
   }
   xfree((char *) polyb.pts);
}

/* draw_needle - hey, why not */

draw_needle(xp1, yp1, xp2, yp2, line, sy_ptr)
COORD xp1, yp1, xp2, yp2;
struct sym_def *sy_ptr;
struct sp_linefill *line;
{
   struct sp_poly polya, polyb;
   struct coordinate crds[10];

   polya.pts=crds;
   polya.n_points=2;
   polya.nlim= -1;
   crds[0].x=xp1; crds[0].y=yp1;
   crds[1].x=xp2; crds[1].y=yp2;
   conv_poly(polya, &polyb, 0);
   offset_poly(polyb, sy_ptr);
   if (sy_ptr->linestyle.none==0) {
     draw_polyline(polyb, 0, sy_ptr->arrow, line, 0);
   }
   xfree((char *) polyb.pts);
}

/* make_polydat - creates a polygon definition from a data index 
                - also creates a point index map if index!=-2 */

make_polydat(datas, poly, xaxis, yaxis, crd, map, index, d_clips, cl_flag)
struct sp_data *datas;
struct axis_def *xaxis, *yaxis;
struct sp_poly *poly;
struct var_set *crd;
double d_clips[4];
int index, **map, cl_flag;
{
   int i, i_val, tmp;
   double f_val;
   COORD x, y;

   if (index<0) {
     tmp=crd->nrows;
   }else{
     tmp=0;
     for (i=0;i<crd->nrows;i++) {
       get_num(datas, crd->var_n[2], i, &i_val, &f_val);
       if (i_val==index) tmp++;
     }
   }

   poly->pts=(struct coordinate *) xalloc((unsigned int)
               ((tmp+10)*sizeof(struct coordinate)),
               "Cannot allocate memory for line definition.");
   if (index!=-2) *map=(int *) xalloc((unsigned int) ((tmp+10)*sizeof(int)),
                       "Cannot allocate memory for line point mapping.");
   poly->nlim= -1;

   tmp=0;
   for (i=0;i<crd->nrows;i++) {
     if (index>=0) {
       get_num(datas, crd->var_n[2], i, &i_val, &f_val);
       if (i_val!=index) continue;
     }
     get_num(datas, crd->var_n[0], i, &i_val, &f_val);
     if (((cl_flag&CL_XMIN)!=0)&&(f_val<d_clips[0])) continue;
     if (((cl_flag&CL_XMAX)!=0)&&(f_val>d_clips[1])) continue;
     pos_inter(xaxis, f_val, 'x', &x, 0);
     get_num(datas, crd->var_n[1], i, &i_val, &f_val);
     if (((cl_flag&CL_YMIN)!=0)&&(f_val<d_clips[2])) continue;
     if (((cl_flag&CL_YMAX)!=0)&&(f_val>d_clips[3])) continue;
     pos_inter(yaxis, f_val, 'y', &y, 0);
     if ((x==MAX_CRD)||(y==MAX_CRD)) continue;
     if (index!=-2) *(*map+tmp)=i;
     (poly->pts+tmp)->x=x;
     (poly->pts+tmp++)->y=y;
   }
   poly->n_points=tmp;
   *(poly->pts+tmp)= *(poly->pts);
   cent_poly(poly);
}

/* cub_spline - create the cubic spline interpolation of the input polyline 
              - uses the equivalent of routines from Numerical Recipes
                  in Fortran */

static char *c_spl_msg="Unable to allocate cubic spline storage.\n";

cub_spline(poly_in, poly_out)
struct sp_poly poly_in, *poly_out;
{
   int nn, i, j, n1, n2, t_end;
   struct coordinate *ptr, crd_t;
   float *nodes, *u, sig, p, val, a, b, h;

   if (options.curv_res<=0) {
     copy_poly(poly_in, poly_out);
     return;
   }

   nodes=(float *) xalloc((unsigned int)
            ((poly_in.n_points+10)*sizeof(float)), c_spl_msg);
   u=(float *) xalloc((unsigned int)
            ((poly_in.n_points+10)*sizeof(float)), c_spl_msg);

   nn=poly_in.n_points-1;
   ptr=poly_in.pts;

   *u=0.0;
   *nodes=0.0;
   ptr++;

   for (i=1;i<(nn-1);i++) {
     sig=(ptr->x-(ptr-1)->x)/((ptr+1)->x-(ptr-1)->x);
     p=sig*(*(nodes+i-1))+2.0;
     *(nodes+i)=(sig-1.0)/p;
     *(u+i)=(6.0*(((ptr+1)->y-ptr->y)/((ptr+1)->x-ptr->x)-
               (ptr->y-(ptr-1)->y)/(ptr->x-(ptr-1)->x))/
               ((ptr+1)->x-(ptr-1)->x)-sig*(*(u+i-1)))/p;
     ptr++;
   }
   *(nodes+nn)=0.0;
   for (i=(nn-1);i>=0;i--) {
     *(nodes+i)= *(nodes+i)*(*(nodes+i+1))+*(u+i);
   }

   xfree((char *) u);
   init_poly(poly_out, -1);

   ptr=poly_in.pts;
   u=nodes;
   for (i=0;i<nn;i++) {
     n1=abs((int) ((ptr+1)->x-ptr->x))/options.curv_res;
     n2=abs((int) ((ptr+1)->y-ptr->y))/options.curv_res;
     if (n2>n1) n1=n2;
     if (n1<3) n1=3;
     t_end=n1;
     if (i==(nn-1)) t_end++;
     for (j=0;j<t_end;j++) {
       b=j/((float) n1);
       h=(ptr+1)->x-ptr->x;
       if (j==0) val=ptr->y;
       else {
         a=1.0-b;
         val=a*ptr->y+b*(ptr+1)->y+
           ((a*a*a-a)*(*u)+(b*b*b-b)*(*(u+1)))*h*h/6.0;
       }
       crd_t.x=ptr->x+b*h;
       crd_t.y=val;
       poly_add_point(poly_out, crd_t);
     }
     ptr++;
     u++;
   }

   xfree((char *) nodes);
}

/* label_poly - draw the polygon, but delete a subsection of it
                to accomodate the needed text
              - if pt_no<0, use label
              - if pt_no>0, use axis value number 
              - tries to locate at the indicated point fract (0<<1)*/

label_poly(poly, label, axis, style, arrow, pt_no, closed, fract)
struct sp_poly poly;
struct inst_key *label;
struct axis_def *axis;
struct sp_linefill *style;
int arrow, pt_no, closed;
double fract;
{
   struct sp_text txt;
   int i, t_p, n1, n2, end;
   COORD cent_x, cent_y;
   double tpx, tpy, r, ang;
   struct coordinate c1, c2, tc, *ptr;

   init_text(&txt);
   if (pt_no<0) {
     speak((char *) NULL, &txt, 0, 1, 0, &label, 0, 0);
   }else{
     if (axis->value.none!=0) {
       draw_polyline(poly, closed, arrow, style, 0);
       return;
     }
     (void) value_put(axis, &txt, 0, pt_no, axis->ndec_places);
   }

   end=poly.n_points;
   if (closed!=0) end++;
   n1=n2=fract*(end-1);
   for (i=0;i<end;i++) {
     if ((i%2)!=0) {
        if (n1>0) n1--;
     }else{
        if (n2<(end-1)) n2++;
     }
     tpx=(poly.pts+n1)->x-(poly.pts+n2)->x;
     tpy=(poly.pts+n1)->y-(poly.pts+n2)->y;
     r=sqrt(tpx*tpx+tpy*tpy);
     if (r>(1.4*(txt.limit.xmax-txt.limit.xmin))) break;
   }
   if (i==end) {
     draw_polyline(poly, closed, arrow, style, 0);
   }else{
     cent_x=((poly.pts+n1)->x+(poly.pts+n2)->x)/2;
     cent_y=((poly.pts+n1)->y+(poly.pts+n2)->y)/2;
     if (tpx==0.0) ang=m_pi/2;
     else ang=atan(tpy/tpx);
     r=0.7*(txt.limit.xmax-txt.limit.xmin);
     pdisp_crd(&(c1.x), &(c1.y), cent_x, cent_y, r, (ang+m_pi)/deg_to_rad);
     pdisp_crd(&(c2.x), &(c2.y), cent_x, cent_y, r, ang/deg_to_rad);
     if ((poly.pts+n1)->x>cent_x) {
       tc=c1; c1=c2; c2=tc;
     }

     t_p=poly.n_points;
     if (n1!=0) {
       poly.n_points=n1+1;
       draw_polyline(poly, 0, (arrow&1), style, 0);
       tn_draw(c1.x, c1.y, style);
     }
     if (n2<(end-1)) {
       ptr=poly.pts;
       poly.pts=poly.pts+n2-1;
       *(poly.pts)=c2;
       poly.n_points=end-n2+1;
       draw_polyline(poly, 0, (arrow&2), style, 0);
       poly.pts=ptr;
     }
     poly.n_points=t_p;

     init_text(&txt);
     txt.angle=ang/deg_to_rad;
     txt.r_angle=ang;
     if (pt_no<0) {
       speak((char *) NULL, &txt, 0, 1, 0, &label, 0, 0);
     }else{
       (void) value_put(axis, &txt, 0, pt_no, axis->ndec_places);
     }
     tpx=cent_x-(txt.limit.xmax+txt.limit.xmin)/2;
     tpy=cent_y-(txt.limit.ymax+txt.limit.ymin)/2;
     init_text(&txt);
     txt.angle=ang/deg_to_rad;
     txt.r_angle=ang;
     txt.x0=tpx;
     txt.y0=tpy;
     if (pt_no<0) {
       speak((char *) NULL, &txt, 1, 1, 0, &label, 0, 0);
     }else{
       (void) value_put(axis, &txt, 1, pt_no, axis->ndec_places);
     }
   }
}

/*  do_scatter - do a rapid spatial colouring of the points in the grid,
                 according to the symbol number and the zaxis specs  */

do_scatter(grid, zaxis, nsym, colset, ncol)
struct grid_def grid;
struct axis_def *zaxis;
struct sp_colour *colset;
int nsym, ncol;
{
   struct sym_def loc_sym;
   struct sp_linefill fill;
   int i, j, nx, ny;
   COORD pt, xl, xr, yl, yr;
   struct sp_poly poly, polyb;
   struct coordinate crds[10];
   double z;

   fix_symbol(&loc_sym, nsym, (struct var_set *) NULL, -1);
   if (loc_sym.fillstyle.none!=0) {
     add_note("Warning: scatter plot requested with no fill = nothing.\n");
     return;
   }

   if (colset==(struct sp_colour *) NULL) {
     if (options.colourlims==(struct sp_colour *) NULL) {
       colset=d_colset;
       ncol=2;
     }else{
       colset=options.colourlims;
       ncol=options.ncolours;
     }
   }

   init_linefill(&fill, -7, 1);
   line_fill((char *) NULL, &(loc_sym.fillstyle), &fill, 0, 1, nsym, 1);
   poly.pts=crds;
   poly.nlim= -1;

   if (grid.type_flag==1) {
     nx=grid.nx;
     ny=grid.ny;
     xl=(*(grid.xmap)*3-*(grid.xmap+1))/2;
     for (i=0;i<nx;i++) {
       if (i==(nx-1)) {
         xr=(*(grid.xmap+i)*3-*(grid.xmap+i-1))/2;
       }else{
         xr=(*(grid.xmap+i)+*(grid.xmap+i+1))/2;
       }
       yl=(*(grid.ymap)*3-*(grid.ymap+1))/2;
       for (j=0;j<ny;j++) {
         if (j==(ny-1)) {
           yr=(*(grid.ymap+j)*3-*(grid.ymap+j-1))/2;
         }else{
           yr=(*(grid.ymap+j)+*(grid.ymap+j+1))/2;
         }
         if (get_bit(grid.map, grid.n_map, (j*nx+i))!=0) {
           make_polybox(&poly, xl, yl, xr, yr);
           if (grid.prec==PRC_SING) z= *(grid.z_sing+j*nx+i); 
           else z= *(grid.z_doub+j*nx+i); 
           pos_inter(zaxis, z, 'y', &pt, 0);
           int_colour(pt, &fill.colour, colset, ncol);
           conv_poly(poly, &polyb, 1);
           cent_poly(&polyb);
           tn_fill(polyb, &fill);
           xfree((char *) polyb.pts);
         }
         yl=yr;
       }
       xl=xr;
     }
   }else{
      /* Voronoi conversions? */
   }
}

/* do_contour - contour the grid surface, according the the specified
                 symbols and zaxis layout  */

do_contour(grid, zaxis, nsym, colset, ncol)
struct grid_def grid;
struct axis_def *zaxis;
struct sp_colour *colset;
int nsym, ncol;
{
   int i, j, k, rc, t_beg, t_end, major_i, minor_i, num, nt[3];
   int e1, e2, old_e1, c_f, n_bel, old_n_bel;
   unsigned char *tri_map;
   struct sp_linefill fill, major_l, minor_l;
   struct sym_def loc_sym;
   struct sp_poly poly, polyb;
   COORD crd_sz, *cut_crds;
   double val, *cut_set;
   struct mesh_tri triangle;
   struct coordinate c1, c2, old_c1, old_c2;
   double c_val;

   fix_symbol(&loc_sym, nsym, (struct var_set *) NULL, -1);
   c_val=0.5;
   if (loc_sym.special.unit!=-1) c_val=loc_sym.special.val;

   if (colset==(struct sp_colour *) NULL) {
     if (options.colourlims==(struct sp_colour *) NULL) {
       colset=d_colset;
       ncol=2;
     }else{
       colset=options.colourlims;
       ncol=options.ncolours;
     }
   }

   init_poly(&poly, -1);

   if ((loc_sym.fillstyle.none==0)&&(loc_sym.fill_set!=0)) {
   init_linefill(&fill, -7, 1);
   line_fill((char *) NULL, &(loc_sym.fillstyle), &fill, 0, 1, nsym, 1);
   num=build_maj_table(zaxis, 'y', &cut_set, &cut_crds);
   for (i=0;i<grid.n_tri;i++) {
     if (get_triangle(grid, i, &triangle, nt)<0) continue;
     for (j=0;j<2;j++) {
       if (triangle.z[0]>triangle.z[1]) {
         val=triangle.z[0]; triangle.z[0]=triangle.z[1]; triangle.z[1]=val;
         c1=triangle.c[0]; triangle.c[0].x=triangle.c[1].x; 
         triangle.c[0].y=triangle.c[1].y; triangle.c[1]=c1;
       }
       if (triangle.z[1]>triangle.z[2]) {
         val=triangle.z[1]; triangle.z[1]=triangle.z[2]; triangle.z[2]=val;
         c1=triangle.c[1]; triangle.c[1].x=triangle.c[2].x; 
         triangle.c[1].y=triangle.c[2].y; triangle.c[2]=c1;
       }
     }
#ifdef LINT
     old_e1=old_c1.x=old_c2.x=0;
#endif
     old_n_bel=n_bel=c_f=0;
     for (j=0;j<num;j++) {
       poly.n_points=0;
       val= *(cut_set+j);
       if (val<triangle.z[0]) continue;
       if (val>=triangle.z[2]) {
         if (j==0) c_f=2;
         if (c_f==2) continue;
         if (c_f==1) {
           if (n_bel==1) {
             if (old_e1==0) {
               *(poly.pts)=old_c2; *(poly.pts+1)=old_c1;
             }else{
               *(poly.pts)=old_c1; *(poly.pts+1)=old_c2;
             }
             *(poly.pts+2)=triangle.c[1]; *(poly.pts+3)=triangle.c[2];
             poly.n_points=4;
           }else{
             *(poly.pts)=old_c1; *(poly.pts+1)=old_c2;
             *(poly.pts+2)=triangle.c[2];
             poly.n_points=3;
           }
         }else{
           for (k=0;k<3;k++) *(poly.pts+k)=triangle.c[k];
           poly.n_points=3;
         }
         c_f=2;
       }else{
         if (get_tri_slice(triangle, val, &c1, &c2, &e1, &e2, 0, 0)<0) continue;
         if (val<triangle.z[1]) n_bel=1;
         else n_bel=2;
         if (c_f==0) {
           if (n_bel==1) {
             *(poly.pts)=c1; *(poly.pts+1)=c2;
             *(poly.pts+2)=triangle.c[0];
             poly.n_points=3;
           }else{
             if (e1==1) {
               *(poly.pts)=c2; *(poly.pts+1)=c1;
             }else{
               *(poly.pts)=c1; *(poly.pts+1)=c2;
             }
             *(poly.pts+2)=triangle.c[1]; *(poly.pts+3)=triangle.c[0];
             poly.n_points=4;
           }
         }else{
           if ((old_n_bel==1)&&(n_bel==2)) {
             if (old_e1==0) {
               *(poly.pts)=old_c2; *(poly.pts+1)=old_c1;
             }else{
               *(poly.pts)=old_c1; *(poly.pts+1)=old_c2;
             }
             *(poly.pts+2)=triangle.c[1];
             if (e1==1) {
               *(poly.pts+3)=c1; *(poly.pts+4)=c2;
             }else{
               *(poly.pts+3)=c2; *(poly.pts+4)=c1;
             }
             poly.n_points=5;
           }else{
             if (e1==old_e1) {
               *(poly.pts)=old_c2; *(poly.pts+1)=old_c1;
             }else{
               *(poly.pts)=old_c1; *(poly.pts+1)=old_c2;
             }
             *(poly.pts+2)=c1; *(poly.pts+3)=c2;
             poly.n_points=4;
           }
         }
         old_c1=c1; old_c2=c2;
         old_n_bel=n_bel;
         old_e1=e1;
         c_f=1;
       }
       if (j==0) continue;
       (poly.pts+poly.n_points)->x=(poly.pts)->x;
       (poly.pts+poly.n_points)->y=(poly.pts)->y;
       crd_sz=(*(cut_crds+j)+*(cut_crds+j-1))/2;
       int_colour(crd_sz, &fill.colour, colset, ncol);
       conv_poly(poly, &polyb, 1);
       cent_poly(&polyb);
       tn_fill(polyb, &fill);
       xfree((char *) polyb.pts);
     }
   }
   }

   if (loc_sym.linestyle.none==0) {
   init_linefill(&major_l, -1, 0);
   line_fill((char *) NULL, &(loc_sym.linestyle), &major_l, 0, 0, nsym, 1);
   minor_l=major_l;
   line_fill((char *) NULL, &(zaxis->major_t.style), &major_l, 0, 0, nsym, 0);
   line_fill((char *) NULL, &(zaxis->minor_t.style), &minor_l, 0, 0, nsym, 0);
   if (zaxis->major_t.noprint!=0) major_l.pattern=0;
   if (zaxis->minor_t.noprint!=0) minor_l.pattern=0;

   init_bit_map(&tri_map, grid.n_tri);

   t_beg=0;
   t_end=zaxis->nvalues;
   if ((zaxis->extend!=0)&&(zaxis->n_vlist==0)) {
     t_beg--; t_end++;
   }

   for (major_i=t_beg;major_i<t_end;major_i++) {
     for (minor_i=0;minor_i<=zaxis->minor_t.num;minor_i++) {
       if (minor_i==0) {
         if ((major_i==-1)||(major_i==zaxis->nvalues)) continue;
         if ((zaxis->major_t.none!=0)||(zaxis->major_t.num==0)) continue;
       }else{
         if (major_i>=(t_end-1)) continue;
         if ((zaxis->minor_t.none!=0)||(zaxis->minor_t.style.none!=0)) continue;
       }
       get_maj_min(zaxis, 'y', major_i, minor_i, &val, &crd_sz);
       clear_bit_map(tri_map, grid.n_tri);

       for (i=0;i<grid.nedge;i++) {
        rc=walk(grid, &poly, val, *(grid.edgelist+i), 1, tri_map);
        if (rc<0) continue;
        conv_poly(poly, &polyb, 0);
        if (minor_i==0) label_poly(polyb, (struct inst_key *) NULL,
              zaxis, &major_l, 0, major_i, 0, c_val);
        else draw_polyline(polyb, 0, 0, &minor_l, 0);
        xfree((char *) polyb.pts);
       }

       for (i=0;i<grid.n_tri;i++) {
        if (get_bit(tri_map, grid.n_tri, i)!=0) continue;
        rc=walk(grid, &poly, val, i, 0, tri_map);
        if (rc<0) continue;
        conv_poly(poly, &polyb, rc);
        if (minor_i==0) label_poly(polyb, (struct inst_key *) NULL,
              zaxis, &major_l, 0, major_i, rc, c_val);
        else draw_polyline(polyb, rc, 0, &minor_l, 0);
        xfree((char *) polyb.pts);
       }
     }
   }
   xfree((char *) tri_map);
   }

   xfree((char *) poly.pts);
}

/* walk - walk along the surface, starting at start_tri and building poly
        - updates the tri_map for touched triangles
        - returns negative if no walking performed */

int walk(grid, poly, z, start_tri, is_edge, tri_map)
struct grid_def grid;
struct sp_poly *poly;
double z;
int start_tri, is_edge;
unsigned char *tri_map;
{
   int nt[3], curr_tri, last_tri, e1, e2, rc;
   struct coordinate c1, c2;
   struct mesh_tri triangle;
   struct sp_linefill line;

   init_linefill(&line, -1, 0);

   poly->n_points=0;

   last_tri=curr_tri=start_tri;
   if (get_bit(tri_map, grid.n_tri, curr_tri)!=0) return(-1);
   if (get_triangle(grid, curr_tri, &triangle, nt)<0) return(-1);
   if (get_tri_slice(triangle, z, &c1, &c2, &e1, &e2, 0, 0)<0) return(-1);

   if (is_edge!=0) {
     if (nt[e1]==-1) {
        poly_add_point(poly, c1);
        poly_add_point(poly, c2);
        tn_move(c1.x,c1.y,&line);
        tn_draw(c2.x,c2.y,&line);
        curr_tri=nt[e2];
     }else if (nt[e2]==-1) {
        poly_add_point(poly, c2);
        poly_add_point(poly, c1);
        tn_move(c2.x,c2.y,&line);
        tn_draw(c1.x,c1.y,&line);
        curr_tri=nt[e1];
     }else return(-1);
   }else{
     poly_add_point(poly, c2);
     tn_move(c2.x,c2.y,&line);
     curr_tri=nt[e2];
   }
   set_bit(tri_map, grid.n_tri, start_tri, 1);

   while((curr_tri!=-1)&&(curr_tri!=start_tri)) {
     if (get_bit(tri_map, grid.n_tri, curr_tri)!=0) break;
     if (get_triangle(grid, curr_tri, &triangle, nt)<0) break;
     if (get_tri_slice(triangle, z, &c1, &c2, &e1, &e2, 0, 0)<0) break;
     set_bit(tri_map, grid.n_tri, curr_tri, 1);
     if (nt[e1]==last_tri) {
       poly_add_point(poly, c2);
     tn_draw(c2.x,c2.y,&line);
       last_tri=curr_tri;
       curr_tri=nt[e2];
     }else{
       poly_add_point(poly, c1);
     tn_draw(c1.x,c1.y,&line);
       last_tri=curr_tri;
       curr_tri=nt[e1];
     }
   }

   rc=0;
   if ((is_edge==0)&&(curr_tri==start_tri)) {
     (poly->pts+poly->n_points)->x=(poly->pts)->x;
     (poly->pts+poly->n_points)->y=(poly->pts)->y;
     rc=1;
   }

   if (poly->n_points==0) return(-1);
   return(rc);
}

/* int_colour - interpolate the colour shade with linear interpolation
                  between the colset entries at the axis limits */

int_colour(pt, colour, colset, ncol)
COORD pt;
struct sp_colour *colour, *colset;
int ncol;
{
   float del;
   int index;

   if (pt<0) pt=0;
   if (pt>1000000) pt=1000000;
   del=pt/1000000.0;

   ncol--;
   if (ncol==0) {*colour= *colset; return;}
   if (del>=1.0) {*colour= *(colset+ncol); return;}
   index=del*ncol;
   del=del*ncol-index;

   colour->hue=(1.0-del)*colset[index].hue+del*colset[index+1].hue;
   colour->sat=(1.0-del)*colset[index].sat+del*colset[index+1].sat;
   colour->bright=(1.0-del)*colset[index].bright+del*colset[index+1].bright;
}
