/******************************************************************

                          sPLOTch!

  Sp3d.c - handles the 3d constructions, including doodle3d and
      plot3d
 
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

/* #define HELPME */

extern struct opt_def options;
extern struct axis_def *axes[20];
extern char argbuff[], inbuff[], tmpbuff[];
extern double m_pi, deg_to_rad;
extern struct sp_colour d_colset[2];

extern char *polars[];
static char *transforms[]={"cartesian","polar","cylindrical","spherical"};
static int radial_ax[4][3]={{0,0,0},
                            {0,1,0},
                            {1,0,0},
                            {2,1,0}};
static char *hiddens[]={"byhorizon","byshade","none"};

/* plot_3d - draw a three dimensional plot (awfully short title for the
      most complicated routine in the program) */

char *huh3d="?", *m3d_msg="Unable to allocate spatial sorting arrays.";

plot_3d(datas)
struct sp_data *datas;
{
   int i, rc, com[4], l, tmp, hidden, frame_set, axis_n[3], axis_layout[3];
   int j, tf_fl, k, an, *ztri, tri_n, tri_p;
   char *tptr, *copy_buff(), *nms[3];
   float axis_skew[3], *zdist, *zdistri, pz, tdist, dot_product(), reye;
   struct sp_poly horizon_up, horizon_down;
   struct lnfill_def frame;
   struct inst_key *copy_inst_tree();
   struct grid_def grid;
   int n_surfs, surf_type[10], no_draw[3];
   struct surf_def surfs[10];
   double mins[3], maxs[3];
   COORD get_xmax(), crd_sz, *cut_crds;
   struct axis_def sh_axis, threed_axes[3];
   struct sp_colour *colset;
   int n_colset, trans_type, polar_unit, gridset, u_drawn;
   double *cut_set, val;
   unsigned char *tri_map;

   struct mesh_tri triangle;
   struct coordinate pt, p1, p2, md_3d(), tri_point();
   struct coord3d point;
   struct sp_linefill frame_line, line, fill;
   int nt[3],nsym,num,e1,e2,pnt;
   struct sp_poly tri_poly;
   struct coordinate tri_crd[5];
   struct sym_def loc_sym;
   int orig_mode;
   struct coord3d old_origin, old_vecx, old_vecy, old_size;

   orig_mode=options.dim_mode;
   options.dim_mode=3;
   old_origin=options.orig_2d;
   old_vecx=options.vec_2dx;
   old_vecy=options.vec_2dy;
   old_size=options.size_2d;

   hidden=0;
   trans_type=polar_unit=sh_axis.set_flag=n_surfs=frame_set=0;
   init_lf_def(&frame);
   reye=dot_product(options.tvecz, options.eyeball);
   reye=reye-dot_product(options.tvecz, options.orig_3d);

   gridset=3;

   for (i=0;i<3;i++) {
     threed_axes[i].set_flag=0;
     axis_n[i]= -100;
     axis_layout[i]=0;
     axis_skew[i]=0.0;
     no_draw[i]=0;
     nms[i]=(char *) NULL;
     mins[i]=5.0e300;
     maxs[i]= -5.0e300;
   }
   axis_layout[0]=axis_layout[2]=5;
   axis_layout[1]=1;

   do {
     com[1]=0;
     rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
     if (rc<0) {
       what(BADCOM, (char *) NULL, com, 0, 1);
     }else{
       tf_fl=0;
       switch(com[0]) {
         case 0  : break;
         case -2 : break;
         case 27 : if (rc!=0) line_fill(argbuff, &frame, &line ,tmp, 0, 1, 0);
                   frame_set=1;
                   break;
         case 181: if (l_comp(argbuff,"all")!=0) {
                    gridset=7;
                   }else{
                    gridset=0;
                    for (i=0;i<strlen(argbuff);i++) {
                      if (argbuff[i]=='x') gridset|=1;
                      if (argbuff[i]=='y') gridset|=2;
                    }
                   }
                   break;
         case 87 : k=0;
                   rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                   if (is_empty(inbuff)==0) {
                     j=-1;
                     for (i=0;i<4;i++) {
                       if (l_comp(inbuff, transforms[i])!=0) j=i;
                     }
                     if (j==-1) sp_err(BADTRANS, tmp, l);
                     else trans_type=j;
                   }
                   if (rc<0) break;
                   rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                   if (is_empty(inbuff)==0) {
                     j=-1;
                     for (i=0;i<3;i++) {
                       if (l_comp(inbuff, polars[i])!=0) j=i;
                     }
                     if (j==-1) sp_err(BADDEG, tmp, l);
                     else polar_unit=j+1;
                   }
                   break;
         case 107: tf_fl++;
         case 106: tf_fl++;
         case 105: k=0;
                   rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                   if (is_empty(inbuff)==0) {
                     an=atoi(argbuff);
                     rc=abs(an);
                     if (rc>20) {
                       sp_err(BADAXIS, tmp, l);
                       break;
                     }
                     if ((rc!=0)&&
                          ((axes[rc-1]==(struct axis_def *) NULL)||
                             (axes[rc-1]->set_flag==0))) {
                       sp_err(NOAXIS, tmp, l);
                       break;
                     }
                     axis_n[tf_fl]=an;
                   }
                   if (rc<0) break;
                   rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                   if (is_empty(inbuff)==0) {
                     an=atoi(inbuff);
                     if ((an<1)||(an>8)) an=1;
                     axis_layout[tf_fl]=an-1;
                   }
                   if (rc<0) break;
                   rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                   if (is_empty(inbuff)==0) {
                     axis_skew[tf_fl]=atof(inbuff);
                   }
                   break;
         case 179: k=-1;
                   for (i=0;i<3;i++) {
                     if (l_comp(argbuff,hiddens[i])!=0) k=i;
                   }
                   if (k==-1) {
                     sp_err(BADHIDE, tmp, l);
                   }else hidden=k;
                   break;
         case 180: tf_fl++;
         case 178: tf_fl++;
         case 62 : tf_fl++;
         case 159: tf_fl++;
         case 177: if (n_surfs==10) {
                     sp_err(TOOSURF, -com[2], com[3]);
                     break;
                   }
                   tptr=copy_buff(argbuff);
                   rc=get_surface(datas, tptr, tmp, (surfs+n_surfs),
                        (tf_fl>2)?1:0);
                   if (rc>=0) surf_type[n_surfs++]=tf_fl;
                   break;
         default : com[1]=com[3];
                   what(ILLCOM, (char *) NULL, com, 0, 1);
                   break;
       }
     }
     line_buff_flush(com[1], 1);
   } while(com[0]!=0);

#ifdef EBUG
  if (debug_level>1) {
    (void) fprintf(stderr,"3D PLOT OUTPUT\n");
    (void) fprintf(stderr,"Xaxis %i, layout %i, skew %f\n", axis_n[0],
       axis_layout[0], axis_skew[0]);
    (void) fprintf(stderr,"Yaxis %i, layout %i, skew %f\n", axis_n[1],
       axis_layout[1], axis_skew[1]);
    (void) fprintf(stderr,"Zaxis %i, layout %i, skew %f\n", axis_n[2],
       axis_layout[2], axis_skew[2]);
  }
#endif

   for (i=0;i<n_surfs;i++) {
     min_max(datas, surfs[i].crd, mins, maxs, -1, nms);
   }

   for (i=0;i<3;i++) {
     if (nms[i]==(char *) NULL) nms[i]=huh3d;
     if (axis_n[i]<=0) {
       if (axis_n[i]==-100) axis_n[i]=0;
       else {
         axis_n[i]=-axis_n[i];
         no_draw[i]=1;
       }
     }
     axis_n[i]=axis_n[i]-1;
     if (axis_n[i]!=-1) {
       threed_axes[i]= *(axes[axis_n[i]]);
       if (threed_axes[i].label.set!=0) 
         threed_axes[i].label.inst=copy_inst_tree(threed_axes[i].label.inst);
     }else init_axis(threed_axes+i);

     if (radial_ax[trans_type][i]!=0) {
       if (radial_ax[trans_type][i]==1) {
         threed_axes[i].rad_fl=polar_unit;
         threed_axes[i].length=360000;
       }else{
         threed_axes[i].rad_fl=polar_unit+3;
         threed_axes[i].length=180000;
       }
     }else threed_axes[i].length=1000000;

     threed_axes[i].origin.x=threed_axes[i].origin.y=0;
     dft_axis((threed_axes+i), "null", "null", nms[i], T_CENTER);
     dsize((threed_axes+i), mins[i], maxs[i], 4);
     size_axis((threed_axes+i), 'x');
   }

   init_linefill(&frame_line, 1, 0);
   if (frame_set!=0) line_fill((char *) NULL, &frame, &frame_line, 0, 0, 1, 1);

   /* if (hidden!=0) {
     do_sides(threed_axes, axis_layout, axis_skew, no_draw, &frame_line, 
       frame_set, trans_type, 1);
   } */
   do_sides(threed_axes, axis_layout, axis_skew, no_draw, &frame_line, 
      frame_set, trans_type, 1);

   init_linefill(&line, 1, 0);
   init_linefill(&fill, -7, 1);
   nsym=0;
   for (i=0;i<n_surfs;i++) {
     nsym++;
     fix_symbol(&loc_sym, nsym, &(surfs[i].crd), -1);
     init_linefill(&line, 1, 0);
     init_linefill(&fill, -7, 1);
     line_fill((char *) NULL, &(loc_sym.linestyle), &line, 0, 0, 1, 1);
     line_fill((char *) NULL, &(loc_sym.fillstyle), &fill, 0, 1, 7, 1);

     rc=build_grid(datas, threed_axes, (threed_axes+1), &grid, surfs[i],
          (surf_type[i]>2)?1:0);
     if (rc<0) {
       add_note("Warning: Unable to construct surface grid representation.\n");
       add_note("         Check data points and setting of DATACLIP values.\n");
     }else{
       if (surfs[i].ax_n!=-1) {
         sh_axis= *(axes[surfs[i].ax_n]);
         if (sh_axis.label.set!=0)
           sh_axis.label.inst=copy_inst_tree(sh_axis.label.inst);
       }else init_axis(&sh_axis);
       dft_axis(&sh_axis, "null", "null", "z", T_CENTER);
       if (surf_type[i]>2) dsize(&sh_axis, grid.aux_zmin, grid.aux_zmax, 10);
       else dsize(&sh_axis, grid.zmin, grid.zmax, 10);
       size_axis(&sh_axis, 'y');
       sh_axis.offset.x=sh_axis.offset.y=0;
       sh_axis.origin.x=sh_axis.origin.y=0;
       sh_axis.length=1000000;

       if (surfs[i].colourlims==(struct sp_colour *) NULL) {
         if (options.colourlims==(struct sp_colour *) NULL) {
           colset=d_colset;
           n_colset=2;
         }else{
           colset=options.colourlims;
           n_colset=options.ncolours;
         }
       }else{
         colset=surfs[i].colourlims;
         n_colset=surfs[i].ncolours;
       }

       zdist=(float *) xalloc((unsigned int) (grid.n_points*sizeof(float)),
                m3d_msg);

       for (pnt=0;pnt<grid.n_points;pnt++) {
         *(zdist+pnt)=5.0e30;
         if (get_grid_point(grid, pnt, &pt, &val)<0) continue;
         point.x=pt.x;
         point.y=pt.y;
         if (trans_type<2) point.z=0;
         else point.z=1000000;
         transform_three_d(&point, trans_type);

         point.x=point.x/1000000.0*options.size_3d.x;
         point.y=point.y/1000000.0*options.size_3d.y;
         point.z=point.z/1000000.0*options.size_3d.z;
         tdist=dot_product(point, options.tvecz);
         *(zdist+pnt)=reye-tdist;
       }

       zdistri=(float *) xalloc((unsigned int) (grid.n_tri*sizeof(float)),
                m3d_msg);
       ztri=(int *) xalloc((unsigned int) (grid.n_tri*sizeof(int)),
                m3d_msg);

       for (tri_n=0;tri_n<grid.n_tri;tri_n++) {
         *(ztri+tri_n)=tri_n;

         if (get_triangle(grid, tri_n, &triangle, nt)<0) continue;
         tdist=*(zdist+triangle.pnt[0]);
         if (tdist>*(zdist+triangle.pnt[1])) tdist=*(zdist+triangle.pnt[1]);
         if (tdist>*(zdist+triangle.pnt[2])) tdist=*(zdist+triangle.pnt[2]);
         *(zdistri+tri_n)=tdist;
       }

       horizon_sort(zdistri, ztri, (int) grid.n_tri);
       xfree((char *) zdist);
       xfree((char *) zdistri);

       if ((loc_sym.fillstyle.none==0)&&(hidden!=1)) {
         for (tri_n=grid.n_tri-1;tri_n>=0;tri_n--) {
           if (get_triangle(grid, *(ztri+tri_n), &triangle, nt)<0) continue;
  
           pz=0.0;
           if (surf_type[i]>2) for (k=0;k<3;k++) pz+=triangle.aux_z[k];
           else for (k=0;k<3;k++) pz+=triangle.z[k];
           pz=pz/3;
           pos_inter(&sh_axis, pz, 'y', &crd_sz, 0);
           int_colour(crd_sz, &fill.colour, colset, n_colset);
  
           transform_triangle(triangle, threed_axes, trans_type, tri_crd);
           tri_poly.n_points=3;
           tri_poly.pts=tri_crd;
           tri_poly.nlim= -1;
           t3_fill(tri_poly, &fill);
         }
       }

       if (loc_sym.linestyle.none==0) {
         if (hidden==0) {
           init_poly(&horizon_up, -1);
           pt.x=-MAX_CRD;
           pt.y=-MAX_CRD;
           poly_add_point(&horizon_up, pt);
           pt.x=MAX_CRD;
           poly_add_point(&horizon_up, pt);
  
           init_poly(&horizon_down, -1);
           pt.x=-MAX_CRD;
           pt.y=MAX_CRD;
           poly_add_point(&horizon_down, pt);
           pt.x=MAX_CRD;
           poly_add_point(&horizon_down, pt);
         }

         init_bit_map(&tri_map, grid.n_tri);

         u_drawn=0;
         if (hidden==1) u_drawn=1; /* byfill, draw to triangles drawn! */
        
         if ((surf_type[i]==2)||(surf_type[i]==4))
              num=build_maj_table(&sh_axis, 'y', &cut_set, &cut_crds);

         for (tri_p=0;tri_p<grid.n_tri;tri_p++) {

           if (hidden==1) tri_n=grid.n_tri-1-tri_p;
           else tri_n=tri_p;

           if (get_triangle(grid, *(ztri+tri_n), &triangle, nt)<0) continue;

           transform_triangle(triangle, threed_axes, trans_type, tri_crd);

           if (hidden==1) {
             pz=0.0;
             if (surf_type[i]>2) for (k=0;k<3;k++) pz+=triangle.aux_z[k];
             else for (k=0;k<3;k++) pz+=triangle.z[k];
             pz=pz/3;
             pos_inter(&sh_axis, pz, 'y', &crd_sz, 0);
             int_colour(crd_sz, &fill.colour, colset, n_colset);

             tri_poly.n_points=3;
             tri_poly.pts=tri_crd;
             tri_poly.nlim= -1;
             t3_fill(tri_poly, &fill);
           }

           if ((surf_type[i]==2)||(surf_type[i]==4)) {
           for (k=0;k<3;k++) {
             pt=triangle.c[k]; triangle.c[k]=tri_crd[k]; tri_crd[k]=pt;
           }
           for (k=0;k<num;k++) {
             val=*(cut_set+k);
             if (get_tri_slice(triangle, val, &p1, &p2, &e1, &e2, 
                      0, (surf_type[i]>2)?1:0)<0) continue;
             if (hidden!=0) {
               spec3_line(p1, p2, &line);
             }else{
               hor_line(p1, p2, &horizon_up, &line, 1, 0);
               hor_line(p1, p2, &horizon_down, &line, -1, 0);
             }
           }
           for (k=0;k<3;k++) {
             pt=triangle.c[k]; triangle.c[k]=tri_crd[k]; tri_crd[k]=pt;
           }
           }else{
           for(k=0;k<3;k++) {
             l=(k+1)%3;
             if ((nt[k]==-1)||(get_bit(tri_map, grid.n_tri, nt[k])==u_drawn)) {
               if ((grid.type_flag!=1)||(gridset==7)||
                 ((triangle.c[k].x==triangle.c[l].x)&&((gridset&1)!=0))||
                   ((triangle.c[k].y==triangle.c[l].y)&&((gridset&2)!=0))) {
                 if (hidden!=0) {
                   spec3_line(tri_crd[k], tri_crd[l], &line);
                 }else{
                   hor_line(tri_crd[k],tri_crd[l],&horizon_up, &line, 1, 0);
                   hor_line(tri_crd[k],tri_crd[l],&horizon_down, &line, -1, 0);
                 }
               }
             }
           }
           }
           if (hidden==0) {
             for(k=0;k<3;k++) {
               l=(k+1)%3;
               if ((nt[k]==-1)||(get_bit(tri_map, grid.n_tri, nt[k])==0)) {
                 hor_line(tri_crd[k], tri_crd[l], &horizon_up, &line, 1, 1);
                 hor_line(tri_crd[k], tri_crd[l], &horizon_down, &line, -1, 1);
               }
             }
           }
           set_bit(tri_map, grid.n_tri, *(ztri+tri_n), 1);
         }
         if (hidden==0) {
           xfree((char *) horizon_up.pts);
           xfree((char *) horizon_down.pts);
         }
         xfree((char *) tri_map);
       }
       destroy_grid(&grid);
     }
   }

   do_sides(threed_axes, axis_layout, axis_skew, no_draw, &frame_line, 
     frame_set, trans_type, 0);

   options.orig_2d=old_origin;
   options.vec_2dx=old_vecx;
   options.vec_2dy=old_vecy;
   options.size_2d=old_size;
   options.dim_mode=orig_mode;
}

/* do_sides - output the back edges of the outline (if back!=0), otherwise
               the front edges, according to the frame definition
            - draw the appropriate axes if involved */

struct coord3d edge_3d_pts[8]={
   {0.0,0.0,0.0},{1.0,0.0,0.0},{1.0,1.0,0.0},{0.0,1.0,0.0},
   {0.0,0.0,1.0},{1.0,0.0,1.0},{1.0,1.0,1.0},{0.0,1.0,1.0}},
               ax_dir_x[3]={
   {1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}},
               ax_dir_y[3]={
   {0.0,1.0,0.0},{-1.0,0.0,0.0},{1.0,0.0,0.0}};

int edges_3d[12][2]={{0,1},{3,2},{7,6},{4,5},
                     {0,3},{1,2},{5,6},{4,7},
                     {0,4},{1,5},{2,6},{3,7}},
    ax_origs[3][4]={{0,3,7,4},{0,1,5,4},{0,1,2,3}};

do_sides(axes, axis_layout, axis_skew, no_draw, frame, frame_set, 
         trans_type, back)
struct axis_def axes[3];
int axis_layout[3], frame_set, trans_type, back, no_draw[3];
struct sp_linefill *frame;
float axis_skew[3];
{
   int i, min_pt, tst, ax_n, red_layout, true_ax;
   float zdist, minz, dot_product(), lengths[3];
   struct coord3d point, origin;
   struct coordinate pt, md_3d();

   lengths[0]=options.size_3d.x;
   lengths[1]=options.size_3d.y;
   lengths[2]=options.size_3d.z;
   min_pt=-1;
   minz=0.0;
   for (i=0;i<8;i++) {
     term_mult(edge_3d_pts[i], options.size_3d, trans_type, &point);
     zdist=dot_product(point, options.tvecz);
     if ((min_pt==-1)||(zdist>minz)) {
       minz=zdist; min_pt=i;
     }
   }

   for (i=0;i<12;i++) {
     ax_n=i/4;
     tst=0;
     if ((edges_3d[i][0]==min_pt)||(edges_3d[i][1]==min_pt)) tst=1;
     if (((back==0)&&(tst==1))||((back==1)&&(tst==0))) {
       if (frame_set!=0) {
          term_mult(edge_3d_pts[edges_3d[i][0]], options.size_3d, trans_type,
            &point);
          pt=md_3d(point,1); 
          tn_move(pt.x, pt.y, frame);
          term_mult(edge_3d_pts[edges_3d[i][1]], options.size_3d, trans_type,
            &point);
          pt=md_3d(point,1); 
          t3_draw(pt.x, pt.y, frame);
       }
       red_layout=(axis_layout[ax_n])%4;
       if (((i-4*ax_n)==red_layout)&&(radial_ax[trans_type][ax_n]==0)&&
              (no_draw[ax_n]==0)) {
         origin=edge_3d_pts[ax_origs[ax_n][red_layout]];
         if (trans_type!=0) {
           if ((origin.x==0.0)&&(ax_dir_x[ax_n].x==0.0)) origin.x=-1.0;
           if ((origin.y==0.0)&&(ax_dir_x[ax_n].y==0.0)) origin.y=-1.0;
         }
         true_ax=ax_n;
         if (trans_type==2) {
           if (ax_n==2) true_ax=1;
           else true_ax=2;
         }
         if (axis_layout[ax_n]>3) {
           draw_3d_axis((axes+true_ax), ax_dir_x[ax_n], ax_dir_y[ax_n],
             origin, axis_skew[ax_n], lengths[ax_n], 1);
         }else{
           draw_3d_axis((axes+true_ax), ax_dir_x[ax_n], ax_dir_y[ax_n],
             origin, axis_skew[ax_n], lengths[ax_n], 0);
         }
       }
     }
   }
}

/* draw_3d_axis - draws the specified axis, using the given basis vectors
                  and the skew value
                - if opposite non-zero, flip origin/length settings */

draw_3d_axis(axis, xdir, ydir, origin, skew, length, opposite)
struct axis_def *axis;
struct coord3d xdir, ydir, origin;
float skew, length;
int opposite;
{
   struct coord3d true_y, zdir;
   COORD get_xmax(), get_ymax(), old_length;

   if (opposite!=0) {
     xdir.x=-xdir.x; xdir.y=-xdir.y; xdir.z=-xdir.z;
     ydir.x=-ydir.x; ydir.y=-ydir.y; ydir.z=-ydir.z;
   }
   cross_product(xdir, ydir, &zdir);
   skew=skew*deg_to_rad;
   true_y.x=sin((double) skew)*zdir.x+cos((double) skew)*ydir.x;
   true_y.y=sin((double) skew)*zdir.y+cos((double) skew)*ydir.y;
   true_y.z=sin((double) skew)*zdir.z+cos((double) skew)*ydir.z;

   term_mult(origin, options.size_3d, 0, &(options.orig_2d));
   options.orig_2d.x=options.orig_2d.x+options.orig_3d.x;
   options.orig_2d.y=options.orig_2d.y+options.orig_3d.y;
   options.orig_2d.z=options.orig_2d.z+options.orig_3d.z;
   options.vec_2dx=xdir;
   options.vec_2dy=true_y;
   options.size_2d.x=length;
   options.size_2d.y=(length/get_xmax())*get_ymax();

   old_length=axis->length;
   axis->origin.x=axis->origin.y=0;
   if (opposite==0) axis->length=get_xmax();
   else axis->length=-get_xmax();

   draw_axis(axis, 'x', 0.0, 0, 0);
   axis->length=old_length;
}

term_mult(a, b, trans_type, result)
struct coord3d a, b, *result;
int trans_type;
{
  switch(trans_type) {
    case 3: if (a.z==0.0) a.z=-1.0;
    case 2:
    case 1: if (a.x==0.0) a.x=-1.0;
            if (a.y==0.0) a.y=-1.0;
            break;
    case 0: break;
  }

  result->x=a.x*b.x; result->y=a.y*b.y; result->z=a.z*b.z;
}

/* transform_triangle - transforms the triangle coordinate set */

transform_triangle(triangle, axes, trans_type, tri_crd)
struct mesh_tri triangle;
struct axis_def *axes;
struct coordinate tri_crd[5];
int trans_type;
{
   int k;
   struct coordinate tri_point();

   for (k=0;k<3;k++) {
     tri_crd[k]=tri_point((axes+2), triangle.c[k], triangle.z[k], trans_type);
   }
   tri_crd[3]=tri_crd[0];
}

struct coordinate tri_point(axis, c, z, trans_type)
struct axis_def *axis;
struct coordinate c;
double z;
int trans_type;
{
   struct coordinate md_3d();
   COORD crd_sz;
   struct coord3d point;

   point.x=c.x;
   point.y=c.y;
   pos_inter(axis, z, 'x', &crd_sz, 0);
   point.z=crd_sz;
   transform_three_d(&point, trans_type);
   point.x=point.x/1000000.0*options.size_3d.x;
   point.y=point.y/1000000.0*options.size_3d.y;
   point.z=point.z/1000000.0*options.size_3d.z;
   return(md_3d(point,1));
}

/* transform_three_d - transform the internal coordinate from the
                       indicated system to cartesian coordinates */

transform_three_d(point, system)
struct coord3d *point;
int system;
{
   float r, theta, phi;

   switch(system) {
     case 0: return;
             break;
     case 1: r=point->x;
             phi=point->y/180000.0*m_pi;
             point->x=r*sin(phi);
             point->y=r*cos(phi);
             break;
     case 2: r=point->z;
             phi=point->x/180000.0*m_pi;
             point->z=point->y;
             point->x=r*sin(phi);
             point->y=r*cos(phi);
             break;
     case 3: r=point->z;
             theta=point->x/180000.0*m_pi;
             phi=point->y/180000.0*m_pi;
             point->x=r*sin(theta)*cos(phi);
             point->y=r*sin(theta)*sin(phi);
             point->z=r*cos(theta);
             break;
   }
}

/* hor_line - chop up the indicated line by the given horizon 
            - above is positive for points of greater y, negative for lesser 
            - if flag non-zero, modify horizon rather than draw lines */

hor_line(p1, p2, horizon, style, above, flag)
struct coordinate p1, p2;
struct sp_poly *horizon;
struct sp_linefill *style;
int above, flag;
{
   int i, j, rc, status, lp_ind, last_i;
   struct coordinate t_pt, last_pt, *ptr, test_pt;
   struct sp_poly new_horizon;
   float fact;

   if (p1.x>p2.x) {t_pt=p1; p1=p2; p2=t_pt;}

#ifdef HELPME
   if (flag==0) {
     (void) fprintf(stderr,"LINE %i %i -> %i %i, %i\n",p1.x,p1.y,p2.x,p2.y,above);
     (void) fprintf(stderr,"#\n");
     for (j=0;j<horizon->n_points;j++) {
       (void) fprintf(stderr,"%i %i\n", (horizon->pts+j)->x,
         (horizon->pts+j)->y);
     }
   }else{
     (void) fprintf(stderr,"BLOCK %i %i -> %i %i, %i\n",p1.x,p1.y,p2.x,p2.y,above);
   }
#endif

   ptr=horizon->pts;
   lp_ind=horizon->n_points-1;

   if (p1.x==p2.x) {
     if (flag!=0) return;
     if (p1.y==p2.y) return;
     if (((p1.y>p2.y)&&(above>0))||((p1.y<p2.y)&&(above<0))) {
       t_pt=p1; p1=p2; p2=t_pt;
     }

     for (i=0;i<lp_ind;i++,ptr++) if ((ptr+1)->x>=p1.x) break;
 
     if ((ptr+1)->x==p1.x) {
       lp_ind-=i;
       for (i=1;i<lp_ind;i++) if ((ptr+i+1)->x>p1.x) break;
       if ((((ptr+1)->y>(ptr+i)->y)&&(above>0))||
             (((ptr+1)->y<(ptr+i)->y)&&(above<0))) ptr+=i; 
     }

     rc=inter_2(*ptr, *(ptr+1), p1, p2, &t_pt, 0);
     if (rc==-1) {
       (void) fprintf(stderr,"3D error!  Tell Jeff (C).\n");
       (void) exit(1);
     }else if (rc>0) {
       if ((above_below(*ptr, *(ptr+1), p1)*above)>=0) {
         spec3_line(p1, p2, style);
       }
     }else{
       spec3_line(t_pt, p2, style);
     }
     return;
   }

   if (flag!=0) init_poly(&new_horizon, -1);

   status=0;
   last_i=-1;
   last_pt=p1;
   for (i=0;i<lp_ind;i++, ptr++) {
     if ((ptr+1)->x<p1.x) continue;
     if ((ptr)->x>p2.x) break;

     if ((flag!=0)&&(last_i<0)&&((ptr+1)->x>=p1.x)) {
       for (j=0;j<=i;j++) poly_add_point(&new_horizon, *(horizon->pts+j));
       if ((ptr)->x<p1.x) {
         fact=((float) p1.x-(ptr)->x)/((float) (ptr+1)->x-(ptr)->x);
         t_pt.y=(1.0-fact)*(ptr)->y+fact*(ptr+1)->y+0.5;
         t_pt.x=p1.x;
         poly_add_point(&new_horizon, t_pt);
       }
       last_i=i+1;
     }

     if ((rc=inter_2(*ptr, *(ptr+1), p1, p2, &t_pt, 0))>0) continue;

#ifdef HELPME
   fprintf(stderr,"INTER %i %i status %i\n",t_pt.x, t_pt.y, status);
#endif

     if (status==0) {   /* unknown */
       if ((last_pt.x==t_pt.x)&&(last_pt.y==t_pt.y)) continue;
       if (rc==-1) t_pt=*(ptr+1);

       if (ptr->x==(ptr+1)->x) {
         if (((ptr->y<(ptr+1)->y)&&(above>0))||
                ((ptr->y>(ptr+1)->y)&&(above<0))) {
           if (flag==0) {
             spec3_line(last_pt, t_pt, style);
           }else{
             poly_add_point(&new_horizon, last_pt);
             poly_add_point(&new_horizon, t_pt);
           }
           status=-1;
         }else{
           if (flag!=0) {
             if (last_i<0) last_i=0;
             for (j=last_i;j<=i;j++) 
                poly_add_point(&new_horizon, *(horizon->pts+j));
           }
           status=1;
         }
       }else{
         test_pt.x=(last_pt.x+t_pt.x)/2;
         test_pt.y=(last_pt.y+t_pt.y)/2;
#ifdef HELPME
         fprintf(stderr,"TEST point %i %i\n",test_pt.x,test_pt.y);
         fprintf(stderr,"        p1 %i %i\n",ptr->x,ptr->y);
         fprintf(stderr,"        p2 %i %i\n",(ptr+1)->x,(ptr+1)->y);
#endif
         if ((above_below(*ptr, *(ptr+1), test_pt)*above)>=0) {
           if (flag==0) {
             spec3_line(last_pt, t_pt, style);
           }else{
             poly_add_point(&new_horizon, last_pt);
             poly_add_point(&new_horizon, t_pt);
           }
           status=-1;
         }else{
           if (flag!=0) {
             if (last_i<0) last_i=0;
             for (j=last_i;j<=i;j++) 
                poly_add_point(&new_horizon, *(horizon->pts+j));
           }
           status=1;
         }
       }
     }else if (status>0) {  /* currently ok */
       if (rc==-1) {
         (void) fprintf(stderr,"3D error!  Tell Jeff (A).\n");
         (void) exit(1);
       }
       if (flag==0) {
         spec3_line(last_pt, t_pt, style);
       }else{
         poly_add_point(&new_horizon, last_pt);
         poly_add_point(&new_horizon, t_pt);
       }
       status=-1;
     }else if (status<0) {  /* currently not ok */
       if (rc==-1) {
         (void) fprintf(stderr,"3D error!  Tell Jeff (B).\n");
         (void) exit(1);
       }
       if (flag!=0) {
         for (j=last_i;j<=i;j++) 
           poly_add_point(&new_horizon, *(horizon->pts+j));
       }
       status=1;
     }
     if (rc<0) status=0;

     last_pt=t_pt;
     last_i=i+1;

#ifdef HELPME
     fprintf(stderr,"new status %i\n",status);
#endif
   }


   if (status==0) {
     if ((above_below(*(ptr-1), *ptr, p2)*above)>=0) {
       if (flag==0) {
         spec3_line(last_pt, p2, style);
       }else{
         poly_add_point(&new_horizon, last_pt);
         poly_add_point(&new_horizon, p2);
       }
     }else{
       if (flag!=0) {
         for (j=last_i;j<i;j++) 
           poly_add_point(&new_horizon, *(horizon->pts+j));
       }
     }
   }else if (status>0) {
     if (flag==0) {
       spec3_line(last_pt, p2, style);
     }else{
       poly_add_point(&new_horizon, last_pt);
       poly_add_point(&new_horizon, p2);
     }
   }else{
     if (flag!=0) {
       for (j=last_i;j<i;j++) 
         poly_add_point(&new_horizon, *(horizon->pts+j));
     }
   }

   if (flag!=0) {
     fact=((float) p2.x-(ptr-1)->x)/((float) (ptr)->x-(ptr-1)->x);
     t_pt.y=(1.0-fact)*(ptr-1)->y+fact*(ptr)->y+0.5;
     t_pt.x=p2.x;
     poly_add_point(&new_horizon, t_pt);
     for (j=i;j<=lp_ind;j++)
        poly_add_point(&new_horizon, *(horizon->pts+j));

     i=0;
     ptr=new_horizon.pts;
     for (j=1;j<new_horizon.n_points;j++) {
       if (((ptr+i)->x!=(ptr+j)->x)||((ptr+i)->y!=(ptr+j)->y)) {
         *(ptr+(++i))=*(ptr+j);
       }
     }
     new_horizon.n_points=i+1;

     xfree((char *) horizon->pts);
     *horizon=new_horizon;

#ifdef HELPME
     (void) fprintf(stderr,"#\n");
     for (j=0;j<horizon->n_points;j++) {
       (void) fprintf(stderr,"%i %i\n", (horizon->pts+j)->x,
         (horizon->pts+j)->y);
     }
     fprintf(stderr,"DONE\n");
#endif
   }
}

spec3_line(p1, p2, style)
struct coordinate p1, p2;
struct sp_linefill *style;
{
   if ((p1.x!=p2.x)||(p1.y!=p2.y)) {
     tn_move(p1.x, p1.y, style);
     t3_draw(p2.x, p2.y, style);
   }
}

/* above_below - returns positive if p is above line q1->q2,
                 negative if below, zero if on */

int above_below(q1, q2, pt)
struct coordinate q1, q2, pt;
{
   struct big_int b_mul();
   struct coordinate test_pt;
   COORD dqx, dqy;
   int test, testb;

   if (q1.y>q2.y) {
     test_pt.x=q2.x; test_pt.y=q1.y;
   }else if (q1.y==q2.y) {
     test_pt.x=q1.x; test_pt.y=q1.y+1000;
   }else{
     test_pt.x=q1.x; test_pt.y=q2.y;
   }

   dqx=q2.x-q1.x;
   dqy=q2.y-q1.y;

   test=b_diff(b_mul((long) dqx, (long) (pt.y-q1.y)),
               b_mul((long) dqy, (long) (pt.x-q1.x)));
   testb=b_diff(b_mul((long) dqx, (long) (test_pt.y-q1.y)),
                b_mul((long) dqy, (long) (test_pt.x-q1.x)));
   return(test*testb);
}

/* horizon_sort - sorts the distance/triangle reference matrix for horizon
              distances */

horizon_sort(dist, tri, n_tri)
float *dist;
int *tri, n_tri;
{
   float rrdist;
   int rrtri;
   int i,j,lp,rp;

   lp=n_tri/2+1;
   rp=n_tri;

   for (;;) {
     if (lp>1) {
       lp--;
       rrdist=*(dist+lp-1);
       rrtri=*(tri+lp-1);
     }else{
       rrdist=*(dist+rp-1);
       rrtri=*(tri+rp-1);
       *(dist+rp-1)=*dist;
       *(tri+rp-1)=*tri;
       rp--;
       if (rp==1) {
         *dist=rrdist;
         *tri=rrtri;
         break;
       }
     }
     i=lp;
     j=lp*2;
     while(j<=rp) {
       if ((j<rp)&&(*(dist+j-1)<*(dist+j))) j++;
       if (rrdist<*(dist+j-1)) {
         *(dist+i-1)=*(dist+j-1);
         *(tri+i-1)=*(tri+j-1);
         i=j;
         j=2*j;
       }
       else j=rp+1;
     }
     *(dist+i-1)=rrdist;
     *(tri+i-1)=rrtri;
   }
}

/*  doodle3d - does simple 3 dimensional doodling */

doodle3d()
{
   int i, com[4], dtype, arrow, nerrs, n_p, tp_nd;
   double delth, theta;
   struct lnfill_def ln_def;
   struct sp_linefill line;
   struct sp_poly polygon;
   struct coord3d tp_c, c_pt, c_vec, c_norm;
   struct coord3d tp_orig, tp_vecx, tp_vecy, tp_size;
   struct coordinate t_cd, md_3d();
   char *t_ptr, *copy_buff();
   struct sp_text txt;
   struct inst_key *inst;
   COORD jnk, get_xmax(), get_ymax(), tp_ht;

   tp_orig=options.orig_2d;
   tp_size=options.size_2d;
   tp_vecx=options.vec_2dx;
   tp_vecy=options.vec_2dy;
   tp_nd=options.dim_mode;
   tp_ht=options.sht;

   options.dim_mode=3;
   options.sht=get_ymax();
   error_off();
   nerrs=0;
   while(end_of_prog()==0) {
     read_data_line(argbuff, 1000);
     com[1]=0;
     yank(argbuff, com, inbuff);
     if ((end_comms(inbuff,"enddoodle")!=0)||(is_empty(inbuff)!=0)) break;
     dtype=atoi(inbuff);
     if (dtype==D_END) break;
     switch(dtype) {
       case 1: yank(argbuff, com, inbuff);
               arrow=atoi(inbuff);
               if (grab_style(argbuff, com, &ln_def, &nerrs)<0) break;
               init_linefill(&line, -1, 0);
               line_fill((char *) NULL, &ln_def, &line, 0, 0, 1, 0);
               yank(argbuff, com, inbuff);
               n_p=atoi(inbuff);
               if ((n_p<0)||(is_empty(inbuff)!=0)) {
                 nerrs++; break;
               }
               init_poly(&polygon, (n_p+10));
               polygon.n_points=n_p;

               for (i=0;i<n_p;i++) {
                 if (get_3d_coord(argbuff, com, &tp_c)<0) {
                   if (end_of_prog()!=0) {n_p= -1; break;}
                   read_data_line(argbuff, 1000);
                   com[1]=0;
                   i--;
                 }else{
                   t_cd=md_3d(tp_c, 1);
                   *(polygon.pts+i)=t_cd;
                 }
               }
               if (n_p<0) {
                 nerrs++; break;
               }
               draw_polyline(polygon, 0, arrow, &line, 1);
               xfree((char *) polygon.pts);
               break;
       case 2: if (get_3d_coord(argbuff, com, &c_pt)<0) {nerrs++;break;}
               if (get_3d_coord(argbuff, com, &c_vec)<0) {nerrs++;break;}
               if (get_3d_coord(argbuff, com, &c_norm)<0) {nerrs++;break;}
               yank(argbuff, com, inbuff);
               delth=atof(inbuff);
               yank(argbuff, com, inbuff);
               arrow=atoi(inbuff);
               if (grab_style(argbuff, com, &ln_def, &nerrs)<0) break;
               init_linefill(&line, -1, 0);
               line_fill((char *) NULL, &ln_def, &line, 0, 0, 1, 0);
               cross_product(c_norm, c_vec, &tp_c);
               init_poly(&polygon, 510);
               polygon.n_points=501;
               for (i=0;i<501;i++) {
                 theta=deg_to_rad*delth/500.0*i;
                 c_norm.x=c_vec.x*cos(theta)+tp_c.x*sin(theta)+c_pt.x;
                 c_norm.y=c_vec.y*cos(theta)+tp_c.y*sin(theta)+c_pt.y;
                 c_norm.z=c_vec.z*cos(theta)+tp_c.z*sin(theta)+c_pt.z;
                 t_cd=md_3d(c_norm, 1);
                 *(polygon.pts+i)=t_cd;
               }
               draw_polyline(polygon, 0, arrow, &line, 1);
               xfree((char *) polygon.pts);
               break;
       case 3: if (get_3d_coord(argbuff, com, &(options.orig_2d))<0) 
                    {nerrs++; break;}
               options.orig_2d.x+=options.orig_3d.x;
               options.orig_2d.y+=options.orig_3d.y;
               options.orig_2d.z+=options.orig_3d.z;
               if (get_3d_coord(argbuff, com, &(options.vec_2dx))<0) 
                    {nerrs++; break;}
               if (get_3d_coord(argbuff, com, &(options.vec_2dy))<0) 
                    {nerrs++; break;}
               renormalize(&(options.vec_2dx));
               renormalize(&(options.vec_2dy));
               yank(argbuff, com, inbuff);
               delth=atof(inbuff);
               options.size_2d.y=delth;
               options.size_2d.x=delth/get_ymax()*get_xmax();
               t_ptr=copy_buff(argbuff+com[1]);
               init_text(&txt);
               inst=(struct inst_key *) NULL;
               speak(t_ptr, &txt, 0, 1, 0, &inst, 1, 0);
               xfree(t_ptr);
               jnk=0;
               halign_text(inst, (COORD) 0, (COORD) 0, (COORD) 0, &jnk, 
                  1, 0, T_CENTER);
               del_inst_tree(inst);
               break;
     }
   }
   error_on();
   options.orig_2d=tp_orig;
   options.size_2d=tp_size;
   options.vec_2dx=tp_vecx;
   options.vec_2dy=tp_vecy;
   options.dim_mode=tp_nd;
   options.sht=tp_ht;

  (void) sprintf(tmpbuff,"   3D Doodling complete. %i error(s) encountered.",
                 nerrs);
  add_note(tmpbuff);
}

/* get_3d_coord - get three numbers in sequence
                - returns negative if no good (ie. reread line) */

int get_3d_coord(buff, com, coord)
char *buff;
int com[4];
struct coord3d *coord;
{
   yank(buff, com, inbuff);
   if (is_empty(inbuff)!=0) return(-1);

   coord->x=atof(inbuff);
   yank(buff, com, inbuff);
   coord->y=atof(inbuff);
   yank(buff, com, inbuff);
   coord->z=atof(inbuff);

#ifdef EBUG
  if (debug_level>0) {
    (void) fprintf(deb_log,"3D Coordinate %f %f %f\n", coord->x,
               coord->y, coord->z);
    (void) fflush(deb_log);
  }
#endif
   return(1);
}
