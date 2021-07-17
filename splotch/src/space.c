/******************************************************************
                          sPLOTch!

  Space - what handles all of the surface gridding/triangulation
          etc.

*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

#define DAT_CHUNK 100
#undef DEB_DEL
#undef DEB_INTER

extern char argbuff[],tmpbuff[];
extern struct opt_def options;
extern double m_pi;

static char mem_msg[]="Unable to allocate memory for grid construction.";

int dblcompare(i,j)
double *i,*j;
{
   return((*i<*j)?(-1):((*i>*j)?1:0));
}

/* build_grid - constructs the grid structure from the surface def 
              - returns negative if unsuccessful (or grid non-existent) 
              - auxil requests auxiliary data grid be constructed */

int build_grid(datas, x_axis, y_axis, grid, surface, auxil)
struct sp_data *datas;
struct axis_def *x_axis, *y_axis;
struct surf_def surface;
struct grid_def *grid;
int auxil;
{
   int i, j, k, nx_lim, ny_lim, nx, ny, i_val, grid_bad, cnt, ind, e_f, n_p;
   int *edge, n_edge, edge_lim, del_cnt, *connect_pt, start, curr, curr_ntri;
   int *edgept, *new_edge, *new_edgept, new_n_edge, first_cut;
   double *xmap, *ymap, f_x, f_y, z, xmin, xmax, ymin, ymax, aux_z;
   COORD crd_sz;
   struct sp_poly hull_poly, add_poly;
   struct coordinate point, *pptr;
   struct sp_linefill fill;

   init_linefill(&fill, -7, 1);

   if ((auxil!=0)&&(surface.crd.n_extra==0)) {
     add_note(
       "Error: auxiliary surface requested without auxiliary variable.\n");
     return(-1);
   }

   grid->type_flag=nx=ny=nx_lim=ny_lim=grid_bad=0;
   grid->zmin=5.0e300;
   grid->zmax= -5.0e300;

   if (options.use_gridtri!=2) {
   nx_lim=ny_lim=DAT_CHUNK;
   xmap=(double *) xalloc((unsigned int) (DAT_CHUNK*sizeof(double)), mem_msg);
   ymap=(double *) xalloc((unsigned int) (DAT_CHUNK*sizeof(double)), mem_msg);

   for (i=0;i<surface.crd.nrows;i++) {
     get_num(datas, surface.crd.var_n[0], i, &i_val, &f_x);
     for (j=0;j<nx;j++) if (f_x==*(xmap+j)) break;
     if (j==nx) *(xmap+nx++)=f_x;
     get_num(datas, surface.crd.var_n[1], i, &i_val, &f_y);
     for (j=0;j<ny;j++) if (f_y==*(ymap+j)) break;
     if (j==ny) *(ymap+ny++)=f_y;
     if (nx>=(nx_lim-2)) {
       nx_lim=nx_lim+DAT_CHUNK;
       xmap=(double *) xrealloc((char *) xmap, (unsigned int) 
              (nx_lim*sizeof(double)), mem_msg);
     }
     if (ny>=(ny_lim-2)) {
       ny_lim=ny_lim+DAT_CHUNK;
       ymap=(double *) xrealloc((char *) ymap, (unsigned int) 
              (ny_lim*sizeof(double)), mem_msg);
     }
     if (((nx*ny)>surface.crd.nrows)&&(options.use_gridtri!=1)) {
       xfree((char *) xmap);
       xfree((char *) ymap);
       grid_bad=1;
       break;
     }
   }
   }else{
     grid_bad=1;
   }

   if (grid_bad==0) {
     qsort((char *) xmap, nx, sizeof(double), dblcompare);
     qsort((char *) ymap, ny, sizeof(double), dblcompare);

     grid->xmap=(COORD *) xalloc((unsigned int) 
               ((nx+10)*sizeof(COORD)), mem_msg);
     grid->ymap=(COORD *) xalloc((unsigned int) 
               ((ny+10)*sizeof(COORD)), mem_msg);
     cnt=0;
     for (i=0;i<nx;i++) {
       if (((surface.cl_flag&CL_XMIN)!=0)&&
               (*(xmap+i)<surface.d_clips[0])) continue;
       if (((surface.cl_flag&CL_XMAX)!=0)&&
               (*(xmap+i)>surface.d_clips[1])) continue;
       pos_inter(x_axis, *(xmap+i), 'x', &crd_sz, 0);
       if (crd_sz==MAX_CRD) continue;
       *(grid->xmap+cnt)=crd_sz;
       *(xmap+cnt++)= *(xmap+i);
     }
     nx=cnt;

     cnt=0;
     for (i=0;i<ny;i++) {
       if (((surface.cl_flag&CL_YMIN)!=0)&&
               (*(ymap+i)<surface.d_clips[2])) continue;
       if (((surface.cl_flag&CL_YMAX)!=0)&&
               (*(ymap+i)>surface.d_clips[3])) continue;
       pos_inter(y_axis, *(ymap+i), 'y', &crd_sz, 0);
       if (crd_sz==MAX_CRD) continue;
       *(grid->ymap+cnt)=crd_sz;
       *(ymap+cnt++)= *(ymap+i);
     }
     ny=cnt;

     if ((nx<2)||(ny<2)) {
       add_note("Error: requested surface is actually a line.\n");
       xfree((char *) xmap);
       xfree((char *) ymap);
       xfree((char *) grid->xmap);
       xfree((char *) grid->ymap);
       return(-1);
     }
     xmin= *xmap; xmax= *(xmap+nx-1);
     ymin= *ymap; ymax= *(ymap+ny-1);

     grid->n_map=(nx+4)*(ny+4);
     grid->prec=datas->vars[surface.crd.var_n[2]]->prec;
     if (grid->prec==PRC_SING) {
       grid->z_sing=(float *) xalloc((unsigned int) 
         ((grid->n_map+10)*sizeof(float)), mem_msg);
       grid->z_doub=(double *) NULL;
     }else{
       grid->z_doub=(double *) xalloc((unsigned int) 
         ((grid->n_map+10)*sizeof(double)), mem_msg);
       grid->z_sing=(float *) NULL;
     }
     if (auxil!=0) {
       grid->aux_prec=datas->vars[surface.crd.extra_n[0]]->prec;
       if (grid->aux_prec==PRC_SING) {
         grid->aux_z_sing=(float *) xalloc((unsigned int) 
           ((grid->n_map+10)*sizeof(float)), mem_msg);
         grid->aux_z_doub=(double *) NULL;
       }else{
         grid->aux_z_doub=(double *) xalloc((unsigned int) 
           ((grid->n_map+10)*sizeof(double)), mem_msg);
         grid->aux_z_sing=(float *) NULL;
       }
     }else{
       grid->aux_prec=0;
     }
     init_bit_map(&(grid->map), grid->n_map);
     init_bit_map(&(grid->block_map), grid->n_map);

     cnt=0;
     for (i=0;i<surface.crd.nrows;i++) {
       get_num(datas, surface.crd.var_n[0], i, &i_val, &f_x);
       if ((f_x<xmin)||(f_x>xmax)) continue;
       get_num(datas, surface.crd.var_n[1], i, &i_val, &f_y);
       if ((f_y<ymin)||(f_y>ymax)) continue;
       for (j=0;j<nx;j++) if (f_x==*(xmap+j)) break;
       for (k=0;k<ny;k++) if (f_y==*(ymap+k)) break;
       if ((j==nx)||(k==ny)) continue;

       get_num(datas, surface.crd.var_n[2], i, &i_val, &z);
       if (z>grid->zmax) grid->zmax=z;
       if (z<grid->zmin) grid->zmin=z;
       if (auxil!=0) {
         get_num(datas, surface.crd.extra_n[0], i, &i_val, &aux_z);
         if (aux_z>grid->aux_zmax) grid->aux_zmax=aux_z;
         if (aux_z<grid->aux_zmin) grid->aux_zmin=aux_z;
       }
       ind=k*nx+j;
       if (grid->prec==PRC_SING)  *(grid->z_sing+ind)=(float) z;
       else *(grid->z_doub+ind)=z;
       if (auxil!=0) {
         if (grid->aux_prec==PRC_SING)  *(grid->aux_z_sing+ind)=(float) aux_z;
         else *(grid->aux_z_doub+ind)=aux_z;
       }
       set_bit(grid->map, grid->n_map, ind, 1);
       cnt++;
     }

     xfree((char *) xmap);
     xfree((char *) ymap);
     if ((cnt<(nx*ny))&&(options.use_gridtri!=1)) {
       xfree((char *) grid->xmap);
       xfree((char *) grid->ymap);
       if (grid->prec=PRC_SING) xfree((char *) grid->z_sing);
       else xfree((char *) grid->z_doub);
       if (auxil!=0) {
         if (grid->aux_prec=PRC_SING) xfree((char *) grid->aux_z_sing);
         else xfree((char *) grid->aux_z_doub);
       }
       grid_bad=1;
     }else{
       edge_lim=DAT_CHUNK;
       n_edge=0;
       edge=(int *) xalloc((unsigned int) (edge_lim*sizeof(int)), mem_msg);
        
       for (i=0;i<nx;i++) {
         for (j=0;j<ny;j++) {
           ind=j*nx+i;
           if (get_bit(grid->map, grid->n_map, ind)==0) {
             set_bit(grid->block_map, grid->n_map, ind, 1);
             if (i!=0) set_bit(grid->block_map, grid->n_map, (ind-1), 1);
             if (j!=0) {
               set_bit(grid->block_map, grid->n_map, (ind-nx), 1);
               if (i!=0) set_bit(grid->block_map, grid->n_map, (ind-nx-1), 1);
             }
           }
         }
       }

       for (i=0;i<(nx-1);i++) {
         e_f=0;
         for (j=0;j<(ny-1);j++) {
           if (get_bit(grid->block_map, grid->n_map, (j*nx+i))!=0) {
             if (e_f==1) 
                add_edge(&edge, &n_edge, &edge_lim, 2*((j-1)*(nx-1)+i)+1);
             e_f=0;
           }else{
             if (e_f==0) 
                add_edge(&edge, &n_edge, &edge_lim, 2*(j*(nx-1)+i));
             e_f=1;
           }
         }
         if (e_f==1)
           add_edge(&edge, &n_edge, &edge_lim, 2*((ny-2)*(nx-1)+i)+1);
       }
       for (j=0;j<(ny-1);j++) {
         e_f=0;
         for (i=0;i<(ny-1);i++) {
           if (get_bit(grid->block_map, grid->n_map, (j*nx+i))!=0) {
             if (e_f==1) 
                add_edge(&edge, &n_edge, &edge_lim, 2*(j*(nx-1)+i-1)+1);
             e_f=0;
           }else{
             if (e_f==0) 
                add_edge(&edge, &n_edge, &edge_lim, 2*(j*(nx-1)+i));
             e_f=1;
           }
         }
         if (e_f==1)
           add_edge(&edge, &n_edge, &edge_lim, 2*(j*(nx-1)+nx-2)+1);
       }

       grid->type_flag=1;
       grid->close_flag=0;
       grid->nx=nx;
       grid->ny=ny;
       grid->edgelist=edge;
       grid->nedge=n_edge;
       grid->n_tri=2*(nx-1)*(ny-1);
       grid->n_points=nx*ny;
     }
   }

   if (grid_bad==1) { /* DELAUNEY TRIANGULATION */
     if (surface.hull_set!=0) {
       make_polydat(datas, &hull_poly, x_axis, y_axis, &(surface.hull),
                     (int **) NULL, -2, surface.d_clips, surface.cl_flag);
     } 
     grid->zmin=5.0e300;
     grid->zmax= -5.0e300;
     grid->xmap=(COORD *) xalloc((unsigned int)
            ((surface.crd.nrows+10)*sizeof(COORD)), mem_msg);
     grid->ymap=(COORD *) xalloc((unsigned int)
            ((surface.crd.nrows+10)*sizeof(COORD)), mem_msg);
     grid->prec=datas->vars[surface.crd.var_n[2]]->prec;
     if (grid->prec=PRC_SING) {
       grid->z_sing=(float *) xalloc((unsigned int)
            ((surface.crd.nrows+10)*sizeof(float)), mem_msg);
       grid->z_doub=(double *) NULL;
     }else{
       grid->z_doub=(double *) xalloc((unsigned int)
            ((surface.crd.nrows+10)*sizeof(double)), mem_msg);
       grid->z_sing=(float *) NULL;
     }
     if (auxil!=0) {
       grid->aux_prec=datas->vars[surface.crd.extra_n[0]]->prec;
       if (grid->aux_prec=PRC_SING) {
         grid->aux_z_sing=(float *) xalloc((unsigned int)
              ((surface.crd.nrows+10)*sizeof(float)), mem_msg);
         grid->aux_z_doub=(double *) NULL;
       }else{
         grid->aux_z_doub=(double *) xalloc((unsigned int)
              ((surface.crd.nrows+10)*sizeof(double)), mem_msg);
         grid->aux_z_sing=(float *) NULL;
       }
     }else{
       grid->aux_prec=0;
     }

     del_cnt=cnt=0;
     for (i=0;i<surface.crd.nrows;i++) {
       get_num(datas, surface.crd.var_n[0], i, &i_val, &f_x);
       if (((surface.cl_flag&CL_XMIN)!=0)&&(f_x<surface.d_clips[0])) continue;
       if (((surface.cl_flag&CL_XMAX)!=0)&&(f_x>surface.d_clips[1])) continue;
       pos_inter(x_axis, f_x, 'x', &crd_sz, 0);
       if (crd_sz==MAX_CRD) continue;
       *(grid->xmap+cnt)=crd_sz;
       point.x=crd_sz;
       get_num(datas, surface.crd.var_n[1], i, &i_val, &f_y);
       if (((surface.cl_flag&CL_YMIN)!=0)&&(f_y<surface.d_clips[2])) continue;
       if (((surface.cl_flag&CL_YMAX)!=0)&&(f_y>surface.d_clips[3])) continue;
       pos_inter(y_axis, f_y, 'y', &crd_sz, 0);
       if (crd_sz==MAX_CRD) continue;
       *(grid->ymap+cnt)=crd_sz;
       point.y=crd_sz;

       if (surface.hull_set!=0) {
         if (point_in_poly(hull_poly, point)==0) {
           del_cnt++;
           continue;
         }
       }

       get_num(datas, surface.crd.var_n[2], i, &i_val, &z);
       if (z>grid->zmax) grid->zmax=z;
       if (z<grid->zmin) grid->zmin=z;
       if (grid->prec=PRC_SING) *(grid->z_sing+cnt)=(float) z;
       else *(grid->z_doub+cnt)=z;
       if (auxil!=0) {
         get_num(datas, surface.crd.extra_n[0], i, &i_val, &aux_z);
         if (aux_z>grid->aux_zmax) grid->aux_zmax=aux_z;
         if (aux_z<grid->aux_zmin) grid->aux_zmin=aux_z;
         if (grid->aux_prec=PRC_SING) *(grid->aux_z_sing+cnt)=(float) aux_z;
         else *(grid->aux_z_doub+cnt)=aux_z;
       }
       cnt++;
     }

     if (del_cnt!=0) {
       (void) sprintf(tmpbuff,
          "Warning: %i surface points outside of triangulation hull.\n", 
          del_cnt);
       add_note(tmpbuff);
     }
     if (cnt<3) {
       add_note(
         "Error: Cannot construct a surface from less than three points.\n");
       return(-1);
     }

     grid->n_points=cnt;
     grid->connect=(int *) xalloc((unsigned int)
                    ((2*cnt+10)*6*sizeof(int)), mem_msg);
     connect_pt=(int *) xalloc((unsigned int)
                    ((grid->n_points+10)*sizeof(int)), mem_msg);

     if (surface.hull_set==0) {
       for (i=0;i<grid->n_points;i++) {
         *(connect_pt+i)=i;
       }
       cnt=inter_connect(grid, grid->connect, 0, connect_pt, grid->n_points,
             &new_edge, &new_edgept, &new_n_edge);
       grid->edgelist=new_edge;
       xfree((char *) new_edgept);
       grid->nedge=new_n_edge;
       grid->n_tri=cnt;
       xfree((char *) connect_pt);
       return(1);
     }

     i=0;
     pptr=hull_poly.pts;
     for (j=1;j<hull_poly.n_points;j++) {
      if (((pptr+j)->x!=(pptr+i)->x)||((pptr+j)->y!=(pptr+i)->y)) {
        *(pptr+(++i))=*(pptr+j);
      }
     }
     *(pptr+i+1)=*(pptr);
     hull_poly.n_points=i+1;
     if (hull_poly.n_points<3) {
       add_note("Error: hull must have at least three distinct points.\n");
       return(-1);
     }

     /* clockwise check here !!!! */

     /* for (i=0;i<=hull_poly.n_points;i++) {
       fprintf(stderr,"Hull pts->%i %i\n",(pptr+i)->x,(pptr+i)->y);
     } */
     init_poly(&add_poly, -1);
     curr_ntri=0;
     first_cut=1;

     while(1) {
       start=0;

       curr=start;
       while (curr!=(hull_poly.n_points)) {
         if (go_left(*(pptr+curr), *(pptr+curr+1), *(pptr+curr+2))==0) {
           if (curr==start) {
             start++;
             curr++;
             continue;
           }else{
             curr--;
             break;
           }
         }
         if (go_left(*(pptr+start), *(pptr+start+1), *(pptr+curr+2))==0) {
           curr--;
           break;
         }
         if (go_left(*(pptr+curr+1), *(pptr+curr+2), *(pptr+start))==0) {
           while(start<curr) {
             start++;
             if (go_left(*(pptr+curr+1), *(pptr+curr+2),
                  *(pptr+start))!=0) break;
           }
           break;
         }
         if (curr==(hull_poly.n_points-2)) break;
         curr++;
       }
       if (start>curr) {
         (void) fprintf(stderr,"Triangulation slice error...Tell Jeff\n");
         (void) exit(1);
       }
       /* fprintf(stderr,"start %i curr %i\n",start,curr); */

       add_poly.n_points=0;
       for (i=0;i<(curr-start+3);i++) {
         poly_add_point(&add_poly, *(pptr+start+i));
       }
       (add_poly.pts+add_poly.n_points)->x=(add_poly.pts)->x;
       (add_poly.pts+add_poly.n_points)->y=(add_poly.pts)->y;

       n_p=0;
       for (i=0;i<grid->n_points;i++) {
         point.x=*(grid->xmap+i);
         point.y=*(grid->ymap+i);
         if (point_in_poly(add_poly, point)!=0) {
          /*  fprintf(stderr,"inside %i %i\n",point.x,point.y); */
           *(connect_pt+(n_p++))=i;
         }
       }
      /*  fprintf(stderr,"n_p %i curr_ntri %i\n",n_p,curr_ntri); */
       if (n_p>=3) {
         cnt=inter_connect(grid, grid->connect, curr_ntri, connect_pt, n_p,
             &new_edge, &new_edgept, &new_n_edge);
/*
         for (i=0;i<new_n_edge;i++) {
           fprintf(stderr,"new edge %i %i\n",*(new_edge+i), *(new_edgept+i));
         }
*/
         if (first_cut!=0) {
           edge=new_edge;
           edgept=new_edgept;
           *(edge+new_n_edge)=-1;
           *(edge+new_n_edge+1)=*(edgept+new_n_edge+1)=-1;
           edge_lim=n_edge=new_n_edge+2;
           first_cut=0;
         }else{
           for (i=0;i<(n_edge-2);i++) {
             for (j=1;j<new_n_edge;j++) {
               if ((cmp_point(grid, *(edgept+i), *(new_edgept+j))!=0)&&
                   (cmp_point(grid,*(edgept+i+1),*(new_edgept+j-1))!=0)) break;
             }
             if (j==new_n_edge) continue;
             if ((*(edge+i)<0)||(*(new_edge+j-1)<0)) continue;
             fix_tri(grid->connect, *(edge+i) , -(i+1), *(new_edge+j-1), edge);
             fix_tri(grid->connect, *(new_edge+j-1) , -j, *(edge+i), new_edge);
             *(edge+i)=*(new_edge+j-1)=-1;
           }
           if ((n_edge+new_n_edge+5)>edge_lim) {
             edge_lim=n_edge+new_n_edge+DAT_CHUNK;
             edge=(int *) xrealloc((char *) edge, (unsigned int)
                     (edge_lim*sizeof(int)), mem_msg);
             edgept=(int *) xrealloc((char *) edgept, (unsigned int)
                     (edge_lim*sizeof(int)), mem_msg);
           }
           for (i=0;i<=new_n_edge;i++) {
             if (*(new_edge+i)>=0) fix_tri(grid->connect, *(new_edge+i), 
                -(i+1), -(n_edge+i+1), new_edge);
             *(edge+n_edge+i)=*(new_edge+i);
             *(edgept+n_edge+i)=*(new_edgept+i);
           }
           xfree((char *) new_edge);
           xfree((char *) new_edgept);
           n_edge+=new_n_edge;
           *(edge+n_edge)=-1;
           *(edge+n_edge+1)=*(edgept+n_edge+1)=-1;
           n_edge+=2;
/*
           for (i=0;i<n_edge;i++) {
             fprintf(stderr,":: %i %i\n",*(edge+i), *(edgept+i));
           }
*/
         }
         /* fprintf(stderr,"%i triangles added\n",cnt); */
     /* draw_grid(grid, grid->connect, curr_ntri, cnt); */
         curr_ntri+=cnt;
       }

       for (i=start+1;i<(hull_poly.n_points-(curr-start));i++) {
         *(pptr+i)=*(pptr+i+curr-start+1);
       }
       hull_poly.n_points=hull_poly.n_points-(curr-start+1);
       if (hull_poly.n_points<=2) break;
     }
     /* draw_grid(grid, grid->connect, 0, curr_ntri); */
     relax_grid(grid, grid->connect, 0, curr_ntri, edge);
     /* draw_grid(grid, grid->connect, 0, curr_ntri); */

     i=0;
     for (j=0;j<n_edge;j++) {
       if (*(edge+j)==-1) continue;
       *(edge+(i++))=*(edge+j);
     }
     n_edge=i;

     /* draw_edge(grid, edge, n_edge); */

     grid->edgelist=edge;
     xfree((char *) edgept);
     grid->nedge=n_edge;
     grid->n_tri=curr_ntri;
     xfree((char *) connect_pt);
   }
   return(1);
}

add_edge(edge, n_edge, edge_lim, num)
int **edge, *n_edge, *edge_lim, num;
{
   if (*n_edge>=(*edge_lim-5)) {
     *edge_lim= *edge_lim+DAT_CHUNK;
     *edge=(int *) xrealloc((char *) *edge, 
             (unsigned int) (*edge_lim*sizeof(int)), mem_msg);
   }
   *(*edge+*n_edge)=num;
   *n_edge= *n_edge+1;
}

/*  destroy_grid - releases the storage space assigned to the grid
                   construction */

destroy_grid(grid)
struct grid_def *grid;
{
   xfree((char *) grid->xmap);
   xfree((char *) grid->ymap);
   if (grid->prec==PRC_SING) xfree((char *) grid->z_sing);
   else xfree((char *) grid->z_doub);
   if (grid->aux_prec!=0) {
     if (grid->prec==PRC_SING) xfree((char *) grid->aux_z_sing);
     else xfree((char *) grid->aux_z_doub);
   }

   if (grid->type_flag==1) {
     xfree((char *) grid->edgelist);
   }else{
     xfree((char *) grid->edgelist);
     xfree((char *) grid->connect);
   }

   grid->nx=grid->ny=grid->n_tri=grid->nedge=grid->n_points=0;
}

/*  inter_connect - using the points contained in the grid definition
                    and pointed to by the connect_pt list, build the
                    convex triangulation of the point set 
                  - starts at the indicated offset triangle */

int inter_connect(grid, connect, offset, connect_pt, n_conn, edgelist, 
           edgepts, nedge)
struct grid_def *grid;
int *connect, *connect_pt, n_conn, **edgelist, **edgepts, *nedge, offset;
{
   COORD rrx, px, py;
   int i,j,k,lp,rp,rrindex,count,*ptr,not_max,start,end,start_fl;
   int *edge_pts, *edge_tris, n_edge, edge_lim, last_tri, first_tri;
   int min_slinf, max_slinf, slope_inf, *tri_ptr;
   float min_sl, max_sl, slope;

#ifdef DEB_INTER
   fprintf(stderr,"# starting inter_connect : connect points \n");
   for (i=0;i<n_conn;i++) fprintf(stderr,"%i: %i %i\n",
    *(connect_pt+i),
    *(grid->xmap+*(connect_pt+i)),
    *(grid->ymap+*(connect_pt+i)));
   fprintf(stderr,"\n"); 
#endif

   lp=n_conn/2+1;   /* heap sort the indices by x-coordinate */
   rp=n_conn;
   for (;;) {
     if (lp>1) {
       lp--;
       rrindex=*(connect_pt+lp-1);
       rrx=*(grid->xmap+rrindex);
     }else{
       rrindex=*(connect_pt+rp-1);
       rrx=*(grid->xmap+rrindex);
       *(connect_pt+rp-1)=*(connect_pt);
       rp--;
       if (rp==1) {
         *(connect_pt)=rrindex;
         break;
       }
     }
     i=lp;
     j=lp*2;
     while(j<=rp) {
       if ((j<rp)&&
         (*(grid->xmap+*(connect_pt+j-1))<*(grid->xmap+*(connect_pt+j)))) j++;
       if (rrx<*(grid->xmap+*(connect_pt+j-1))) {
         *(connect_pt+i-1)=*(connect_pt+j-1);
         i=j;
         j=2*j;
       }
       else j=rp+1;
     }
     *(connect_pt+i-1)=rrindex;
   }

   count=1;             /* bubble sort the sequential y (for equal x) */
   while(count!=0) {
     count=0;
     for (i=0;i<(n_conn-1);i++) {
       ptr=connect_pt+i;
       if ((*(grid->xmap+*ptr)==*(grid->xmap+*(ptr+1)))&&
                (*(grid->ymap+*ptr)>*(grid->ymap+*(ptr+1)))) {
         rrindex=*ptr; *ptr=*(ptr+1); *(ptr+1)=rrindex;
         count++;
       }
     }
   }

   i=0;                   /* watch out for identical point sets */
   for (j=1;j<n_conn;j++) {
    if (cmp_point(grid, *(connect_pt+i), *(connect_pt+j))==0) {
      *(connect_pt+(++i))=*(connect_pt+j);
    }
   }
   n_conn=i+1;

#ifdef DEB_INTER
   fprintf(stderr,"# sorted point set \n");
   for (i=0;i<n_conn;i++) fprintf(stderr,"%i: %i %i\n",
    *(connect_pt+i),
    *(grid->xmap+*(connect_pt+i)),
    *(grid->ymap+*(connect_pt+i)));
   fprintf(stderr,"\n"); 
#endif

   count=0;   /* make the first edge pairing */
   edge_pts=(int *) xalloc((unsigned int) DAT_CHUNK*sizeof(int), mem_msg);
   edge_tris=(int *) xalloc((unsigned int) DAT_CHUNK*sizeof(int), mem_msg);
   edge_lim=DAT_CHUNK;
   n_edge=0;

   if (*(grid->xmap+*(connect_pt))==*(grid->xmap+*(connect_pt+1))) {
     *(grid->xmap+*connect_pt)=*(grid->xmap+*connect_pt)-1;
   }
 
   *(edge_pts)=*(edge_pts+2)=*connect_pt;
   *(edge_pts+1)=*(connect_pt+1);
   *(edge_tris)=*(edge_tris+1)=*(edge_tris+2)=-1;
   n_edge=3;
   count=0;

   start_fl=1;
   for (i=2;i<n_conn;i++) {   /* add the points!!!! */
     px=*(grid->xmap+*(connect_pt+i));
     py=*(grid->ymap+*(connect_pt+i));
     start=end=-1;
     min_sl=max_sl=0.0;
     min_slinf=max_slinf=not_max=0;
     for (j=0;j<n_edge;j++) {
       rrindex=*(edge_pts+j);
       if (px==*(grid->xmap+rrindex)) {
         if (py<*(grid->ymap+rrindex)) {
           slope=5.e30;
           slope_inf=1;
         }else{
           slope=-5.e30;
           slope_inf=-1;
         }
       }else{
         slope_inf=0;
         slope=((float) *(grid->ymap+rrindex)-py)/
                    ((float) px-*(grid->xmap+rrindex));
       }

       if ((start==-1)||(slope_inf<min_slinf)||
             ((slope_inf==min_slinf)&&(slope<min_sl))||
               ((slope_inf==min_slinf)&&(slope==min_sl)&&(start==(j-1)))) {
         start=j;
         min_sl=slope;
         min_slinf=slope_inf;
       }
       if ((end==-1)||(slope_inf>max_slinf)||
             ((slope_inf==max_slinf)&&(slope>max_sl))||
               ((slope_inf==max_slinf)&&(slope==max_sl)&&(not_max!=0))) {
         end=j;
         max_sl=slope;
         max_slinf=slope_inf;
         not_max=0;
       }else{
         if ((slope_inf!=max_slinf)||(slope!=max_sl)) not_max=1;
       }
     }
#ifdef DEB_INTER
     fprintf(stderr,"adding point %i %i\n",px,py);
     fprintf(stderr,"start %i end %i\n",start,end);
#endif
     if (start>=end) {
       (void) fprintf(stderr,"Triangulation error!  Tell Jeff.\n");
       (void) exit(1);
     }

     if (((start+1)+(n_edge-end)+1)>(edge_lim-5)) {
       edge_lim=edge_lim+DAT_CHUNK;
       edge_pts=(int *) xrealloc((char *) edge_pts,
              (unsigned int) edge_lim*sizeof(int), mem_msg);
       edge_tris=(int *) xrealloc((char *) edge_tris,
              (unsigned int) edge_lim*sizeof(int), mem_msg);
     }
     if ((end-start)==1) {
       for (j=(n_edge-1);j>=end;j--) {
         *(edge_pts+j+1)=*(edge_pts+j);
         if (start_fl==0) fix_tri(connect, *(edge_tris+j) , -(j+1), -(j+2), 
              edge_tris);
         *(edge_tris+j+1)=*(edge_tris+j);
       }
     }
  
     last_tri=-(start+1);
     first_tri=count+offset;
     for (j=start;j<end;j++) {
       rrindex=*(edge_tris+j);
       tri_ptr=connect+6*(count+offset);
       *(tri_ptr)=*(edge_pts+j);
       *(tri_ptr+3)=last_tri;
       *(tri_ptr+1)=*(connect_pt+i);
       *(tri_ptr+4)=-(start+2);
       *(tri_ptr+2)=*(edge_pts+j+1);
       *(tri_ptr+5)=rrindex;

       if (last_tri>=0) {
         *(connect+6*last_tri+4)=count+offset;
       }
       if (start_fl==0) {
         for (k=0;k<3;k++) {
           if (*(connect+6*rrindex+k)==*(edge_pts+j)) break;
         }
         if ((k==3)||(*(connect+6*rrindex+3+k)!=-(j+1))) {
           (void) fprintf(stderr,"tri %i\n",rrindex);
           (void) fprintf(stderr,"kpoint %i\n",k);
           (void) fprintf(stderr,"tri_n %i\n",*(connect+6*rrindex+3+k));
           (void) fprintf(stderr,"Triangulation error (D). Tell Jeff!\n");
           (void) exit(1);
         }
         *(connect+6*rrindex+3+k)=count+offset;
       }

       last_tri=count+offset;
       count++;
     }
     *(edge_tris+start)=first_tri;
     *(edge_pts+start+1)=*(connect_pt+i);
     *(edge_tris+start+1)=last_tri;

     if ((end-start)!=1) {
       for (j=0;j<(n_edge-end);j++) {
         *(edge_pts+start+2+j)=*(edge_pts+end+j);
         fix_tri(connect, *(edge_tris+end+j) , -(end+j+1), -(start+j+3), 
            edge_tris);
         *(edge_tris+start+2+j)=*(edge_tris+end+j);
       }
       n_edge=n_edge-(end-start)+2;
     }else n_edge++;

     if (start_fl!=0) {
       for (j=0;j<n_edge;j++) *(edge_tris+j)=offset;
       for (j=0;j<3;j++) *(connect+6*offset+3+j)=-(((j+start)%3)+1);
       start_fl=0;
     }

#ifdef DEB_INTER
     fprintf(stderr,"edgelist (pt:tri)\n");
     for (j=0;j<n_edge;j++) {
      fprintf(stderr,"%i %i %i : %i: %i\n",j, *(grid->xmap+*(edge_pts+j)),
               *(grid->ymap+*(edge_pts+j)),
               *(edge_pts+j), *(edge_tris+j));
     }
     for (j=0;j<count;j++) { 
       fprintf(stderr,"%i\n",j);
       for (k=0;k<3;k++) {
         fprintf(stderr,"# pt %i tri %i\n",*(connect+6*(j+offset)+k),*(connect+6*(j+offset)+3+k));
       }
     }
     /* draw_grid(grid, connect, offset, count); */
#endif
   }
   n_edge--;

   /* draw_grid(grid, connect, offset, count); */
   relax_grid(grid, connect, offset, count, edge_tris);
   /* draw_grid(grid, connect, offset, count); */
#ifdef DEB_DEL
     fprintf(stderr,"RELAXED edgelist (pt:tri)\n");
     for (j=0;j<n_edge;j++) {
      fprintf(stderr,"%i %i %i : %i: %i\n",j, *(grid->xmap+*(edge_pts+j)),
               *(grid->ymap+*(edge_pts+j)),
               *(edge_pts+j), *(edge_tris+j));
     }
     for (j=0;j<count;j++) { 
       fprintf(stderr,"%i\n",(j+offset));
       for (k=0;k<3;k++) {
         fprintf(stderr,"# pt %i tri %i\n",*(connect+6*(j+offset)+k),*(connect+6*(j+offset)+3+k));
       }
     }
     /* draw_grid(grid, connect, offset, count); */
#endif

   *edgelist=edge_tris;
   *edgepts=edge_pts;
   *nedge=n_edge;
   return(count);
}

/* cmp_point - returns non-zero if two point indices are same point */

cmp_point(grid, inda, indb)
struct grid_def *grid;
int inda, indb;
{
   if (*(grid->xmap+inda)!=*(grid->xmap+indb)) return(0);
   if (*(grid->ymap+inda)!=*(grid->ymap+indb)) return(0);
   return(1);
}

/*  Note: this is for test purposes only!!! */

#ifdef DEB_DEL
draw_edge(grid, edge, n_edge)
struct grid_def *grid;
int *edge, n_edge;
{
   int i,j,k,pt;
   struct sp_linefill line;

   init_linefill(&line, -1, 0);
   open_screen();
   for (i=0;i<n_edge;i++) {
     for (j=0;j<3;j++) {
       k=(j+1)%3;
       pt=*(grid->connect+6*(*(edge+i))+j);
       tn_move(*(grid->xmap+pt), *(grid->ymap+pt), &line);
       pt=*(grid->connect+6*(*(edge+i))+k);
       tn_draw(*(grid->xmap+pt), *(grid->ymap+pt), &line);
     }
   }

}

draw_grid(grid, connect, offset, n_tri)
struct grid_def *grid;
int *connect, n_tri;
{
   int i,j,k,pt;
   struct sp_linefill line;

   init_linefill(&line, -1, 0);
   open_screen();
   for (i=0;i<n_tri;i++) {
     for (j=0;j<3;j++) {
       k=(j+1)%3;
       pt=*(connect+6*(i+offset)+j);
       tn_move(*(grid->xmap+pt), *(grid->ymap+pt), &line);
       pt=*(connect+6*(i+offset)+k);
       tn_draw(*(grid->xmap+pt), *(grid->ymap+pt), &line);
     }
   }

   update_screen();
}
#endif

/* relax_grid - using the supplied connectivity list, swap triangle
                pairs according to Lawsons criterion, to relax the
                grid connections to the Delauney triangulation */

static int spd_mod[6]={0, 1, 2, 0, 1, 2};

relax_grid(grid, connect, offset, n_tri, edge_tri)
struct grid_def *grid;
int *connect, n_tri, offset, *edge_tri;
{
   int count, index, tr_index, i, j, comp, p1, p2, q1, q2;
   int *i_ptr, *c_ptr;

   do {
     count=0;
     for (index=0;index<n_tri;index++) {
       tr_index=index+offset;
       i_ptr=connect+6*tr_index;
       for (i=0;i<3;i++) {
         if ((comp= *(i_ptr+3+i))<tr_index) continue;
         c_ptr=connect+6*comp;
         for (j=0;j<3;j++) {
           if (*(c_ptr+3+j)==tr_index) break;
         }
         if (j==3) {
           for (j=0;j<n_tri;j++) { 
             (void) fprintf(stderr,"%i\n",j);
             for (i=0;i<3;i++) {
               (void) fprintf(stderr,"# %i %i\n",*(connect+6*(j+offset)+i),
                *(connect+6*(j+offset)+3+i));
             }
           }
           (void) fprintf(stderr,"ind %i comp %i\n",tr_index,comp);
           (void) fprintf(stderr,"Triangulation error(A)....Tell Jeff.\n");
           (void) exit(1);
         }
#ifdef DEB_DEL
           fprintf(stderr,"comparing %i and %i\n",tr_index,comp);
#endif
         p1= *(i_ptr+i);
         p2= *(i_ptr+spd_mod[i+1]);
         q1= *(i_ptr+spd_mod[i+2]);
         q2= *(c_ptr+spd_mod[j+2]);
         if (test_swap(grid, p1, p2, q1, q2)!=0) {
           if (*(i_ptr+i)==*(c_ptr+j)) {
             *(i_ptr+spd_mod[i+1])=q2;
             *(c_ptr+j)=q1;
             fix_tri(connect, *(c_ptr+3+spd_mod[j+2]), comp, tr_index,
                     edge_tri);
             fix_tri(connect, *(i_ptr+3+spd_mod[i+1]), tr_index, comp,
                     edge_tri);
             *(i_ptr+3+i)= *(c_ptr+3+spd_mod[j+2]);
             *(c_ptr+3+spd_mod[j+2])=tr_index;
             *(c_ptr+3+j)= *(i_ptr+3+spd_mod[i+1]);
             *(i_ptr+3+spd_mod[i+1])=comp;
           }else{
             *(i_ptr+spd_mod[i+1])=q2;
             *(c_ptr+spd_mod[j+1])=q1;
             fix_tri(connect, *(c_ptr+3+spd_mod[j+1]), comp, tr_index,
                     edge_tri);
             fix_tri(connect, *(i_ptr+3+spd_mod[i+1]), tr_index, comp,
                     edge_tri);
             *(i_ptr+3+i)= *(c_ptr+3+spd_mod[j+1]);
             *(c_ptr+3+spd_mod[j+1])=tr_index;
             *(c_ptr+3+j)= *(i_ptr+3+spd_mod[i+1]);
             *(i_ptr+3+spd_mod[i+1])=comp;
           }
#ifdef DEB_DEL
           fprintf(stderr,"swapped %i and %i\n",tr_index,comp);
#endif
           count++;
         }
       }
     }
     /* draw_grid(grid, connect, offset, n_tri); */
   }while (count!=0);
}

/* fix_tri - in triangle tri_n, swap indexing from old_tri to new_tri
           - if tri_n<0, swap edge index from old_tri to new_tri */

fix_tri(connect, tri_n, old_tri, new_tri, edge_tri)
int *connect, tri_n, old_tri, new_tri, *edge_tri;
{
   int k;

#ifdef DEB_DEL
   fprintf(stderr,"Fixing %i: %i->%i\n",tri_n, old_tri, new_tri);
#endif

   if (tri_n<0) {
     if (*(edge_tri+(-tri_n-1))!=old_tri) {
       fprintf(stderr,"Edge # %i\n", (-tri_n-1));
       fprintf(stderr,"old %i new %i\n", old_tri, new_tri);
       fprintf(stderr,"Triangulation error (E)....Tell Jeff\n");
       (void) exit(1);
     }
     *(edge_tri+(-tri_n-1))=new_tri;
   }else{
     for (k=0;k<3;k++) {
       if (*(connect+6*tri_n+3+k)==old_tri) *(connect+6*tri_n+3+k)=new_tri;
     }
   }
}

/* test_swap - test if the diagonal qt1->qt2 should be swapped with
               diagonal pt1->pt2 (returns non-zero), according to
               Lawson's criteria 
             - Note: uses double precision to avoid deadly! roundoff
               errors  */

#define SQ(c) (((double) (c))*((double) (c)))

int test_swap(grid, pt1, pt2, qt1, qt2)
struct grid_def *grid;
int pt1, pt2, qt1, qt2;
{
   struct coordinate p1, p2, q1, q2;
   double s1, s2, s3, s4, d1, d2;
   double numa, dena, numb, denb, numc, denc;

   p1.x=*(grid->xmap+pt1); p1.y=*(grid->ymap+pt1);
   p2.x=*(grid->xmap+pt2); p2.y=*(grid->ymap+pt2);
   q1.x=*(grid->xmap+qt1); q1.y=*(grid->ymap+qt1);
   q2.x=*(grid->xmap+qt2); q2.y=*(grid->ymap+qt2);

   if (same_side(q1,q2,p1,p2)!=0) return(0);

   s1=SQ(q2.x-p1.x)+SQ(q2.y-p1.y);
   s2=SQ(p2.x-q2.x)+SQ(p2.y-q2.y);
   s3=SQ(q1.x-p2.x)+SQ(q1.y-p2.y);
   s4=SQ(p1.x-q1.x)+SQ(p1.y-q1.y);
   d1=SQ(p1.x-p2.x)+SQ(p1.y-p2.y);
   d2=SQ(q1.x-q2.x)+SQ(q1.y-q2.y);

   min_ang(s1, s2, d1, &numa, &dena);
   min_ang(s3, s4, d1, &numb, &denb);
   if ((numb*dena)>(numa*denb)) {
     numa=numb;
     dena=denb;
   }

   min_ang(s1, s4, d2, &numb, &denb);
   min_ang(s2, s3, d2, &numc, &denc);
   if ((numc*denb)>(numb*denc)) {
     numb=numc;
     denb=denc;
   }

   if ((numa*denb)>(numb*dena)) return(1);
   return(0);
}

/* min_ang - returns the num/den pair for the cos factor of the smallest angle 
             in the triangle defined by sides asq-bsq-csq */

min_ang(asq, bsq, csq, num, den)
double asq, bsq, csq;
double *num, *den;
{
   double temp;

   if (csq>asq) {
     temp=csq; csq=asq; asq=temp;
   }
   if (csq>bsq) {
     temp=csq; csq=bsq; bsq=temp;
   }
   *num=(csq-asq-bsq)*(csq-asq-bsq);
   *den=asq*bsq;
}

/*  get_grid_point - returns the specified data point 
                   - returns negative if point bad */

int get_grid_point(grid, pnt, c, z)
struct grid_def grid;
int pnt;
struct coordinate *c;
double *z;
{
   int xp, yp;

   if (grid.type_flag==1) {
     if ((pnt<0)||(pnt>=grid.n_points)) return(-1);
     if (get_bit(grid.map, grid.n_map, pnt)==0) return(-1);
     yp=pnt/grid.nx;
     xp=pnt-yp*grid.nx;
     c->x= *(grid.xmap+xp);
     c->y= *(grid.ymap+yp);
   }else{
     if ((pnt<0)||(pnt>=grid.n_points)) return(-1);
     c->x= *(grid.xmap+pnt);
     c->y= *(grid.ymap+pnt);
   }
   if (grid.prec==PRC_SING) *z= *(grid.z_sing+pnt);
   else *z= *(grid.z_doub+pnt);
   return(0);
}

/*  get_triangle - determines the definition of triangle num
                 - neighbours are given by the nt matrix
                 - returns negative if triangle invalid (for holed grids) */

static int tr_ind_x[2][3]={{0, 1, 0}, {1, 0, 1}},
           tr_ind_y[2][3]={{0, 0, 1}, {1, 1, 0}};

int get_triangle(grid, num, triangle, nt)
struct grid_def grid;
struct mesh_tri *triangle;
int num, nt[3];
{
   int i, index, evodd, xp, yp, nxb, fx, fy, tp;

   if (grid.type_flag==1) {
     nxb=grid.nx-1;
     if ((num<0)||(num>=grid.n_tri)) return(-1);
     index=num/2;
     yp=index/nxb;
     xp=index-yp*nxb;
     evodd=num-2*index;
     index=yp*grid.nx+xp;
     if (get_bit(grid.block_map, grid.n_map, index)!=0) return(-1);
     for (i=0;i<3;i++) {
       fx=xp+tr_ind_x[evodd][i];
       fy=yp+tr_ind_y[evodd][i];
       tp=fy*grid.nx+fx;
       triangle->c[i].x= *(grid.xmap+fx);
       triangle->c[i].y= *(grid.ymap+fy);
       if (grid.prec==PRC_SING) triangle->z[i]= *(grid.z_sing+tp);
       else triangle->z[i]= *(grid.z_doub+tp);
       triangle->pnt[i]=tp;
       if (grid.aux_prec!=0) {
         if (grid.aux_prec==PRC_SING) triangle->aux_z[i]= *(grid.aux_z_sing+tp);
         else triangle->aux_z[i]= *(grid.aux_z_doub+tp);
       }else{
         triangle->aux_z[i]=0.0;
       }
     }
     if (evodd==0) {
       if ((yp<=0)||
         (get_bit(grid.block_map, grid.n_map, (index-grid.nx))!=0)) nt[0]= -1;
       else nt[0]=num-2*nxb+1;
       nt[1]=num+1;
       if ((xp<=0)||
         (get_bit(grid.block_map, grid.n_map, (index-1))!=0)) nt[2]= -1;
       else nt[2]=num-1;
     }else{
       if ((yp>=(grid.ny-2))||
         (get_bit(grid.block_map, grid.n_map, (index+grid.nx))!=0)) nt[0]= -1;
       else nt[0]=num+2*nxb-1;
       nt[1]=num-1;
       if ((xp>=(grid.nx-2))||
         (get_bit(grid.block_map, grid.n_map, (index+1))!=0)) nt[2]= -1;
       else nt[2]=num+1;
     }
   }else{
     if ((num<0)||(num>=grid.n_tri)) return(-1);
     for (i=0;i<3;i++) {
       tp= *(grid.connect+6*num+i);
       triangle->c[i].x= *(grid.xmap+tp);
       triangle->c[i].y= *(grid.ymap+tp);
       if (grid.prec==PRC_SING) triangle->z[i]= *(grid.z_sing+tp);
       else triangle->z[i]= *(grid.z_doub+tp);
       triangle->pnt[i]=tp;
       if (grid.aux_prec!=0) {
         if (grid.aux_prec==PRC_SING) triangle->aux_z[i]= *(grid.aux_z_sing+tp);
         else triangle->aux_z[i]= *(grid.aux_z_doub+tp);
       }else{
         triangle->aux_z[i]=0.0;
       }
       nt[i]= *(grid.connect+6*num+3+i);
       if (nt[i]<0) nt[i]=-1;
     }
   }
   return(0);
}

/* get_tri_slice - slice a triangle with the given value 
                 - returns negative if no intersection
                 - returns 3rd edge if edge cut (see notes) 
                 - doesn't actually calc if no_z true 
                 - uses aux_z values if auxil set */

int get_tri_slice(triangle, z, c1, c2, e1, e2, no_z, auxil)
struct mesh_tri triangle;
double z;
struct coordinate *c1, *c2;
int *e1, *e2, no_z, auxil;
{
   int i, cent, nz;
   double d[3];

   nz=0;
   if (auxil==0) {
     for (i=0;i<3;i++) if ((d[i]=z-triangle.z[i])==0.0) nz++;
   }else{
     for (i=0;i<3;i++) if ((d[i]=z-triangle.aux_z[i])==0.0) nz++;
   }
   if (nz==3) return(-1);
 
   if (nz==2) {
     for (i=0;i<3;i++) if (d[i]!=0.0) break;
     *e1=(i+2)%3;
     *c1=triangle.c[*e1];
     *e2=i;
     *c2=triangle.c[(i+1)%3];
     return((i+1)%3);
   }

   if ((d[0]*d[1])<=0.0) {
     if (no_z!=0) return(0);
     if ((d[1]*d[2])<=0.0) cent=1;
     else cent=0;
   }else if ((d[1]*d[2])<=0.0) {
     if (no_z!=0) return(0);
     cent=2;
   }else return(-1);

   i= *e1=(cent+2)%3;
   inter_sect(triangle.c[i], d[i], triangle.c[cent], d[cent], c1);
   i=(cent+1)%3;
   inter_sect(triangle.c[cent], d[cent], triangle.c[i], d[i], c2);
   *e2=cent;

   return(0);
}

inter_sect(c1, d1, c2, d2, c)
struct coordinate c1, c2, *c;
double d1, d2;
{
  c->x=(d1*c2.x-d2*c1.x)/(d1-d2);
  c->y=(d1*c2.y-d2*c1.y)/(d1-d2);
}
