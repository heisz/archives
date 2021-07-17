/******************************************************************
                          sPLOTch!

  Sputter - definition of symbols to be sputtered onto the 
	    graph to mark data points.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

#define NFIT 22
static char *fit[]={"none","join","bspline","spline","joinunder", 
             "bsplineunder","splineunder","joinpoly","bsplinepoly",
             "hbar","vbar","hbarfix","vbarfix","herr","verr","boxerr",
             "vneedle","joinlabel","hneedle","varjoin","skyline",
             "shoreline"};
static int sortees[]={0,0,0,SORT_X,0,
              0,SORT_X,0,0,
              SORT_Y,SORT_X,0,0,SORT_Y,SORT_X,0,
              0,0,0,0,SORT_X,
              SORT_Y};

extern char argbuff[], inbuff[];
extern int polar_fl;
struct pair_v sym_vals[11];

/*  do_symbol - parses the symbol command line for specifiers
   	          defining the various graphical symbols       */

do_symbol(data, n_sym)
struct sym_def *data[];
int n_sym;
{
   int i, l, rc, com[4], tmp, ft, t_pt, t_spec, cl_fl;
   char *ptr, *copy_buff();
   struct sp_text txt;
   struct sp_linefill lp;
   struct inst_key *t_inst, *copy_inst_tree();
   struct lnfill_def t_lp_t;
   struct united sp_va, sp_vb;
   COORD crd_sz;

   for (i=0;i<n_sym;i++) if (data[i]->set==0) init_sym(data[i], 0);
   cl_fl=1;

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL , com, 0, 1);
      }else{
         t_pt=0;
         if ((cl_fl!=0)&&(com[0]!=0)&&(com[0]!=-2)) {
           for (i=0;i<n_sym;i++) data[i]->set=1;
           cl_fl=0;
         }
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
            case 139: for (i=0;i<n_sym;i++) {
                        init_sym(data[i], 0);
                        data[i]->set=0;
                      }
                      cl_fl=1;
                      break;
            case 16 : t_pt++;
	    case 43 : init_text(&txt);
                      ptr=copy_buff(argbuff);
                      t_inst=(struct inst_key *) NULL;
                      if (t_pt==0) {
		        speak(ptr, &txt, 0, 1, tmp, &t_inst, 1, 1);
                        for (i=0;i<n_sym;i++) {
                          if (data[i]->pnt_set!=0) del_inst_tree(data[i]->pnt);
                          data[i]->pnt=copy_inst_tree(t_inst);
                          data[i]->pnt_set=1;
		          data[i]->pnt_none=txt.none;
                        }
                      }else{
		        speak(ptr, &txt, 0, 1, tmp, &t_inst, 1, 0);
                        for (i=0;i<n_sym;i++) {
                          if (data[i]->label_set!=0) 
                              del_inst_tree(data[i]->label);
                          data[i]->label=copy_inst_tree(t_inst);
                          data[i]->label_set=1;
		          data[i]->label_none=txt.none;
                        }
                      }
                      del_inst_tree(t_inst);
                      xfree(ptr);
		      break;
            case 134: t_pt++;
            case 44 : ptr=copy_buff(argbuff);
                      line_fill(ptr, &t_lp_t, &lp, tmp, 0, 1, 1);
                      for (i=0;i<n_sym;i++) {
                        if (t_pt==0) {
                          data[i]->linestyle=t_lp_t;
                          data[i]->line_set=1;
                        }else{
                          data[i]->fillstyle=t_lp_t;
                          data[i]->fill_set=1;
                        }
                      }
                      xfree(ptr);
                      break;
            case 30:  sp_va.unit=sp_vb.unit= -1;
                      rc=get_coordinate(argbuff, tmp, &sp_va, &sp_vb, 
                               &t_spec, 1, 0, 1);
                      if (rc<0) {
			sp_err(BADSIZE, tmp, l);
                      }else{
                        for (i=0;i<n_sym;i++) {
                          t_pt=data[i]->offset_spec;
                          if (sp_va.unit!=-1)  {
                            data[i]->offset.x_c=sp_va;
                            t_pt=t_pt&0xF0;
                          }
                          if (sp_vb.unit!=-1) {
                            data[i]->offset.y_c=sp_vb;
                            t_pt=t_pt&0x0F;
                          }
                          data[i]->offset_spec=t_pt|t_spec;
                        }
                      }
                      break;
            case 135: t_spec=scan_num(argbuff, tmp, 1);
                      if (t_spec<0) t_spec=0;
                      else{
                        rc=scsize(argbuff, 'y', &crd_sz, &sp_va, 1, 0);
                        for (i=0;i<n_sym;i++) {
                          data[i]->special=sp_va;
                          data[i]->spec_spec=t_spec;
                        }
                      }
                      break;
	    case 85 : t_pt=0;
	              if ((clower(argbuff[0])=='x')&&
			       (clower(argbuff[1])=='y')) {
			t_pt=SORT_XY;
                      }else if ((clower(argbuff[0])=='y')&&
			       (clower(argbuff[1])=='x')) {
			t_pt=SORT_YX;
                      }else if (clower(argbuff[0])=='x') {
			t_pt=SORT_X;
                      }else if (clower(argbuff[0])=='y') {
			t_pt=SORT_Y;
		      }
                      for (i=0;i<n_sym;i++) {
                        data[i]->sort_fl=t_pt;
		        data[i]->sort_set=1;
                      }
		      break;
	    case 45 : ft= -1;
		      for (i=0;((i<NFIT)&(ft==-1));i++) {
                        if (l_comp(argbuff, fit[i])!=0) {
			  ft=i;
			  break;
			}
		      }
		      if (ft==-1) {
			sp_err(BADFIT, tmp, l);
		      }else{
                        for (i=0;i<n_sym;i++) {
			data[i]->inter=ft;
                        data[i]->inter_set=1;
			  if (data[i]->sort_set==0) {
			    data[i]->sort_fl=sortees[data[i]->inter];
			  }
                        }
		      }
		      break;
            case 155: rc=atoi(argbuff);
                      if ((rc<0)||(rc>3)) {
                        sp_err(BADPTR, tmp, l);
                      }else{
                        for (i=0;i<n_sym;i++) {
                          data[i]->arrow=rc;
                          data[i]->arrow_set=1;
                        }
                      }
                      break;
	    default : com[1]=com[3];
		      what (ILLCOM, (char *) NULL, com, 0, 1); 
		      break;
	 }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);
}

/*  init_sym - initializes symbol definition 
             - fl non-zero if working with symbol zero */

init_sym(data, fl)
struct sym_def *data;
int fl;
{
   if (data->set!=0) {
     if (data->pnt_set!=0) del_inst_tree(data->pnt);
     if (data->label_set!=0) del_inst_tree(data->label);
   }
   data->set=1;
   data->pnt_set=0;
   data->pnt_none=0;
   data->label_set=0;
   data->label_none=0;
   init_lf_def(&(data->linestyle));
   init_lf_def(&(data->fillstyle));
   if (fl==0) {
     data->inter_set=0;
     data->line_set=0;
     data->fill_set=0;
   }else{
     data->inter=1;
     data->inter_set=1;
     data->linestyle.pattern= -1;
     data->linestyle.pattern_set=1;
     data->line_set=0;
     data->fillstyle.pattern= -1;
     data->fillstyle.pattern_set=1;
     data->fill_set=0;
   }
   data->offset.x_c.unit= -1;
   data->offset.y_c.unit= -1;
   data->offset_spec=0;
   data->special.unit= -1;
   data->spec_spec=0;
   data->sort_fl=0;
   data->sort_set=0;
   data->arrow=0;
   data->arrow_set=0;
}

/* doublesort - sorts coordinate data according to the sort_fl 
	      - if SORT_X or SORT_Y, sort according to X or Y
	      - if SORT_XY or SORT_YX, subsort according to Y or X 
              - sort n_map as well for consistency (only if non-NULL) */

doublesort(d_map, n_map, ncrds, sort_fl)
struct coordinate *d_map;
int ncrds, sort_fl, *n_map;
{
   int pass, i, type, count;

   if ((sort_fl<1)||(sort_fl>5)) return;
   for (pass=0;pass<2;pass++) {
     if (pass==0) {
       type=0;
       if ((sort_fl==SORT_Y)||(sort_fl==SORT_YX)) type=1;
     }else{
       if ((sort_fl==SORT_X)||(sort_fl==SORT_Y)) continue;
       type=0;
       if (sort_fl==SORT_XY) type=1;
     }

     count=1;
     while (count!=0) {
       count=0;
       for (i=0;i<(ncrds-1);i++) {
	 if (type==0) {
	   if ((d_map+i)->x>(d_map+i+1)->x) {
	     if (!((pass==1)&&((d_map+i)->y!=(d_map+i+1)->y))) {
	       flip(d_map,n_map,i);
	       count++;
	     }
	   }
	 }else{
	   if ((d_map+i)->y>(d_map+i+1)->y) {
	     if (!((pass==1)&&((d_map+i)->x!=(d_map+i+1)->x))) {
	       flip(d_map,n_map,i);
	       count++;
	     }
	   }
	 }
       }
     }
   }
}

/* flip - trade places of two coordinate pairs at i and i+1 */

flip(d_map,n_map,i)
struct coordinate *d_map;
int i,*n_map;
{
   int t_x,t_y,t_n;

   t_x=(d_map+i)->x;
   (d_map+i)->x=(d_map+i+1)->x;
   (d_map+i+1)->x=t_x;
   t_y=(d_map+i)->y;
   (d_map+i)->y=(d_map+i+1)->y;
   (d_map+i+1)->y=t_y;
   if (n_map!=(int *) NULL) {
     t_n= *(n_map+i);
     *(n_map+i)= *(n_map+i+1);
     *(n_map+i+1)=t_n;
   }
}

/* size_crd - size a coordinate structure
            - modx, mody indicate theta space measurements 
            - spec_fl is special value inclusions */

size_crd(crd, xdir, ydir, rel, modx, mody, spec_fl)
struct coord_def *crd;
char xdir, ydir;
int modx, mody, rel, spec_fl;
{
   struct united sp_vu;

   sp_vu=crd->x_c;
   if (sp_vu.unit!=-1) {
     if ((spec_fl&0x0F)!=0) sp_vu.val=sym_vals[(spec_fl&0x0F)-1].f;
     (void) scsize((char *) NULL, xdir, &(crd->x), &(sp_vu), rel, -1);
     if (modx>0) {
       if (sp_vu.unit==-2) crd->x=sp_vu.val*1000;
       else crd->x=sp_vu.val*3600;
     }
   }
   sp_vu=crd->y_c;
   if (sp_vu.unit!=-1) {
     if ((spec_fl&0xF0)!=0) sp_vu.val=sym_vals[((spec_fl&0xF0)/0x10)-1].f;
     (void) scsize((char *) NULL, ydir, &(crd->y) , &(sp_vu), rel, -1);
     if (mody>0) {
       if (sp_vu.unit==-2) crd->y=sp_vu.val*1000;
       else crd->y=sp_vu.val*3600;
     }
   }
}


/* offset_poly - apply the spatial offset to the polygon */

offset_poly(polygon, symbol)
struct sp_poly polygon;
struct sym_def *symbol;
{
   COORD x0, y0;
   int i;

   get_offset(symbol, &x0, &y0);
   
   for (i=0;i<=polygon.n_points;i++) {
     (polygon.pts+i)->x=(polygon.pts+i)->x+x0;
     (polygon.pts+i)->y=(polygon.pts+i)->y+y0;
   }
}

/* get_offset - returns the offset term according to the symbol def */

get_offset(symbol, x0, y0)
struct sym_def *symbol;
COORD *x0, *y0;
{
   symbol->offset.x=symbol->offset.y=0;
   size_crd(&(symbol->offset), 'x', 'y', 1, polar_fl, 0, symbol->offset_spec);
   *x0=symbol->offset.x;
   *y0=symbol->offset.y;
   pol_crd(x0, y0, 1);
}

/* clear_specs - clears the special value indicators */

clear_specs()
{
   int k;

   for (k=0;k<11;k++) {
     sym_vals[k].i=0;
     sym_vals[k].f=0.0;
     sym_vals[k].type=TYPE_FLT;
   }
}

/* stack_specs - sets the special value indicators for point or
                   layout control */

stack_specs(datas, npt, n_map, crd)
int npt, *n_map;
struct sp_data *datas;
struct var_set *crd;
{
  int k, i_val;
  double f_val;

  sym_vals[0].i=(npt+1);
  sym_vals[0].f=(double) (npt+1);
  sym_vals[0].type=TYPE_INT;
  if (n_map!=(int *) NULL) {
    for (k=1;k<=crd->n_extra;k++) {
      if (crd->extra_n[k-1]<0) continue;
      get_num(datas, crd->extra_n[k-1], *(n_map+npt), &i_val, &f_val);
      sym_vals[k].i=i_val;
      sym_vals[k].f=f_val;
      sym_vals[k].type=var_type(datas, crd->extra_n[k-1]);
    }
  }
}
