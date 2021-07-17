/*******************************************************************
                          sPLOTch!

  Spatial - the core graphics routine.  (Finally!) makes the plot
    out of all the other stuff.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"

#ifdef EBUG
  extern FILE *deb_log;
  extern int debug_level;
#endif

#define DT "center f=romand h=8pct splotch { h=4pct \' sPLOTch! \' } splotch" 

extern struct axis_def *axes[20];
extern struct char_st titles[10];
extern struct char_st foots[10];
extern struct sym_def *symbols[101];
extern struct opt_def options;
extern char argbuff[], inbuff[], tmpbuff[];

extern COORD xrad_cent, yrad_cent, llc_x,llc_y,urc_x,urc_y;
extern int polar_fl, gr_drawn;
extern float eccentricity;
extern struct axis_def p_axes[3];
extern double m_pi, deg_to_rad;

char d_dir[3]={'x', 'y', 'y'};
char *d_label[3]={"null", "a=90", "a=-90"};
char *dp_label[3]={"none", "null", "null"};
char *d_val[3]={"null", "null", "null"};
char *dp_val[3]={"h=80hpct", "null", "null"};
int d_align[3]={T_CENTER, T_RIGHT, T_LEFT};

char *polars[]={"degree", "radian", "gradian"};
static char *clips[]={"normal", "none", "retain", "external", "all"};
static char *huh="?";
#define N_CLIP 5

plot_it(datas)
struct sp_data *datas;
{
   int rc,com[4],tmp,l,ind,i,j,k,com2[4],gr_end;
   int nsym,nent,fl,i_val;
   int tmp2,tick2_def;
   double f_val;
   float jfoot,jtitle;
   int set_llc,t_len;

   struct sp_text txt;
   struct united sp_vu, lens[3];
   struct coord_def origs[3];
   struct grid_def grid;
   struct axis_def zaxis;
   struct sp_poly clip_poly, tmp_poly;
   COORD gfl_x, gfl_y, gfr_x, gfr_y, tk_v, crd_sz, beg, end;
   COORD get_xmax(), get_ymax(), crdabs();
   double mins[3], maxs[3], aligns[3], d_clips[4];
   struct lnfill_def hv_gr_mm[4], frame, fill;
   int table[50], max_r, align_set[4], cl_flag;
   int hv_gr_set[4], frame_set, clip_type, n_lvref, axis_n[3], fill_set;
   int tmp_tik[3], tmp_val[3], tmp_lab[3], rv_axis;
   struct sp_linefill clip_lp, lp;
   struct var_set *crds[2], crda[50], crdb[50];
   struct var_set boundary;
   struct surf_def surfs[10];
   struct coordinate crd_tmp;
   int bound_set, ncrds[2], surf_type[10], n_surfs, no_draw[3];
   char *tptr, *copy_buff(), *nms[3];
   struct inst_key *copy_inst_tree();
   struct t_refs { struct lnfill_def line;
                   double val;
                   int type;
                 } lv_refs[40];

   zaxis.set_flag=tick2_def=n_surfs=ncrds[0]=ncrds[1]=0;
   for (i=0;i<3;i++) {
     axis_n[i]= -100; 
     no_draw[i]=0;
     align_set[i]=hv_gr_set[i]=0;
     lens[i].unit=origs[i].x_c.unit=origs[i].y_c.unit= -1;
     nms[i]=(char *) NULL;
   }
   hv_gr_set[3]=0;
   crds[0]=crda;
   crds[1]=crdb;
   bound_set=cl_flag=frame_set=fill_set=clip_type=n_lvref=polar_fl=rv_axis=0;
   eccentricity=1.0;

   init_lf_def(&frame);
   init_lf_def(&fill);

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
         ind=0;	 
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
	    case 27 : if (rc!=0) line_fill(argbuff, &frame, &lp ,tmp, 0, 1, 0);
                      frame_set=1;
		      break;
	    case 134: if (rc!=0) line_fill(argbuff, &fill, &lp ,tmp, 1, 1, 0);
                      fill_set=1;
		      break;
            case 149: polar_fl=1;
                      if (rc!=0) {
                        ind= -1;
                        for (i=0;i<3;i++) {
                          if (l_comp(argbuff, polars[i])!=0) ind=i+1;
                        }
                        if (ind==-1) {
                          sp_err(BADDEG, tmp, l);
                          polar_fl=0;
                        }else{
                          polar_fl=ind;
                        }
                      }
                      break;
            case 150: eccentricity=atof(argbuff);
                      break;
            case 153: ind++;
            case 152: ind++;
            case 151: aligns[ind]=atof(argbuff);
                      align_set[ind]=1;
                      break;
            case 91 : ind++;
            case 90 : ind++;
            case 55 : ind++;
            case 54 : line_fill(argbuff, (hv_gr_mm+ind), &lp, tmp, 0, 1, 0);
                      hv_gr_set[ind]=1;
                      break;
            case 89 : ind++;
	    case 52 : ind++;
	    case 53 : if (n_lvref==40) {
                        sp_err(TOOREF, -com[2], com[3]);
                        break;
                      }
                      k=0;
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
		      lv_refs[n_lvref].val=atof(inbuff);
                      if (rc>=0) line_fill((argbuff+k), 
                            &(lv_refs[n_lvref].line), &lp, (tmp+k), 0, 1, 0);
                      else init_lf_def(&(lv_refs[n_lvref].line));
                      lv_refs[n_lvref++].type=ind;
		      break;
            case 141: ind= -1;
                      for (i=0;i<N_CLIP;i++) {
                        if (l_comp(argbuff, clips[i])!=0) { 
                          ind=i;
                          break;
                        }
                      }
                      if (ind==-1) {
                        sp_err(BADCLIP, tmp, l);
                      }else{
                        clip_type=ind;
                      }
                      break;
            case 145: ind++;
            case 144: ind++;
            case 143: rc=scsize(argbuff, d_dir[ind], &crd_sz, &sp_vu, 1, 0);
                      if (rc<0) {
			sp_err(BADSIZE, tmp, l);
			break;
                      }else{
                        lens[ind]=sp_vu;
                      }
                      break;
            case 148: ind++;
            case 147: ind++;
            case 146: rc=get_coordinate(argbuff, tmp, &(origs[ind].x_c),
                               &(origs[ind].y_c), &i, 1, 0, 0);
                      if (rc<0) sp_err(BADSIZE, tmp, l);
                      break;
	    case 65 : ind++;
            case 106:
	    case 29 : ind++;
            case 105:
	    case 28 : tmp2=atoi(argbuff);
                      nsym=abs(tmp2);
		      if (nsym>20) {
			 sp_err(BADAXIS, tmp, l);
			 break;
		      }
		      if ((nsym!=0)&&
                           ((axes[nsym-1]==(struct axis_def *) NULL)||
                              (axes[nsym-1]->set_flag==0))) {
		        sp_err(NOAXIS, tmp, l);
		        break;
		      }else{
                        axis_n[ind]=tmp2;
                      }
		      break;
            case 64 : ind++;
	    case 38 : com2[1]=0;
		      yank(argbuff, com2, inbuff);
		      while(is_empty(inbuff)==0) {
			tmp2=tmp+com2[3];
                        if (ncrds[ind]==50) {
			  sev_err(OVERPLOT);
                          break;
                        }
		        rc=variables(inbuff, datas, (crds[ind]+ncrds[ind]), 
                                       tmp2, (ind==2)?1:0);
                        if (rc==-1) {
                          (crds[ind]+ncrds[ind])->nrows= -1;
                        }else{
                          ncrds[ind]++;
                        }
			yank(argbuff, com2, inbuff);
		      }
#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
    (void) fprintf(deb_log,"Number of %i plots %i\n", ind, ncrds[ind]);
    (void) fflush(deb_log);
  }
#endif
		      break;
            case 159: ind++;
	    case 62 : if (n_surfs==10) {
                        sp_err(TOOSURF, -com[2], com[3]);
                        break;
                      }
                      tptr=copy_buff(argbuff);
                      rc=get_surface(datas, tptr, tmp, (surfs+n_surfs), 0);
                      if (rc>=0) {
                        surf_type[n_surfs++]=ind;
                      }
                      break;
            case 142: bound_set=0;
                      rc=variables(argbuff, datas, &boundary, tmp, 0);
                      if (rc!=-1) {
                        if (rc==3) {
			  sp_err(NOTHREE, tmp, l);
                        }else{
                          bound_set=1;
                        }
                      }
                      break;
            case 158: get_clip(argbuff, d_clips, &(cl_flag));
                      break;
	    default : com[1]=com[3];
		      what (ILLCOM, (char *) NULL, com, 0, 1); 
		      break;
	 }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);

   /************** generate default definitions **********************/

#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"Plot\n");
   (void) fprintf(deb_log,"Haxis vaxis rvaxis %i %i %i\n",
      axis_n[0], axis_n[1], axis_n[2]);
   (void) fflush(deb_log);
  }
#endif
   fl=0;
   for (j=0;j<10;j++) if (titles[j].set==1) fl=1;
   if (fl==0) {
      titles[0].set=1;
      init_text(&txt);
      titles[0].inst=(struct inst_key *) NULL;
      speak(DT, &txt, 0, 1, 0, &(titles[0].inst), 1, 0);
   }

   for (i=0;i<3;i++) {
     mins[i]=5.0e300;
     maxs[i]= -5.0e300;
   }

   for (i=0;i<ncrds[0];i++) min_max(datas, *(crds[0]+i), mins, maxs, 1, nms);
   for (i=0;i<ncrds[1];i++) min_max(datas, *(crds[1]+i), mins, maxs, 2, nms);
   for (i=0;i<n_surfs;i++) min_max(datas, surfs[i].crd, mins, maxs, 1, nms);

#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"Hmin,hmax %g %g\n",mins[0],maxs[0]);
   (void) fprintf(deb_log,"Vmin,vmax %g %g\n",mins[1],maxs[1]);
   (void) fprintf(deb_log,"Vmin2,vmax2 %g %g\n",mins[2],maxs[2]);
   (void) fprintf(deb_log,"Names %s %s %s\n",nms[0], nms[1], nms[2]);
   (void) fflush(deb_log);
  }
#endif

   for (i=0;i<3;i++) {
     if (nms[i]==(char *) NULL) nms[i]=huh;
     if (axis_n[i]<=0) {
       if (axis_n[i]==-100) axis_n[i]=0;
       else {
         axis_n[i]=-axis_n[i];
         no_draw[i]=1;
       }
     }
     axis_n[i]=axis_n[i]-1;
     if (axis_n[i]!=-1) {
       p_axes[i]= *(axes[axis_n[i]]);
       if (p_axes[i].label.set!=0) 
         p_axes[i].label.inst=copy_inst_tree(p_axes[i].label.inst);
     } else init_axis(p_axes+i);
     if (polar_fl!=0) {
       if (i==0) p_axes[i].rad_fl=polar_fl;
       else p_axes[i].rad_fl= -polar_fl;
     }
     if (lens[i].unit!=-1) p_axes[i].ln_def=lens[i];
     if (origs[i].x_c.unit!=-1) p_axes[i].origin.x_c=origs[i].x_c;
     if (origs[i].y_c.unit!=-1) p_axes[i].origin.y_c=origs[i].y_c;
     if ((i<2)||(ncrds[(i-1)]>0)||(axis_n[i]!=-1)) {
       if (polar_fl==0) {
         dft_axis((p_axes+i), d_label[i], d_val[i], nms[i], 
           d_align[i]);
       }else{
         dft_axis((p_axes+i), dp_label[i], dp_val[i], nms[i], 
           T_CENTER);
       }
       dsize((p_axes+i), mins[i], maxs[i], 4);
       size_axis((p_axes+i), d_dir[i]);
       if (i==2) rv_axis=1;
#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
    (void) fprintf(deb_log, "Completed AXIS %i\n", i);
    print_axis(p_axes+i);
  }
#endif
     }
   }

#ifdef EBUG
  if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"Defaults Found rv_axis-> %i\n", rv_axis);
   (void) fflush(deb_log);
  }
#endif

   /*****************   the plot code!!!!!! *********************/

   /* A - determine final page size, subracting space for titles on
          top, footnotes on bottom, and legends wherever....   */

   nsym=0;
   error_off();

   switch (clip_type) {
     case 1: case 3: case 0: 
              save_clip(1, 1);
              break;
     case 2:  del_clip(1);
              break;
     default: break;
   }

   if ((options.border_set!=0)&&(options.borderline.none==0)) {
     init_linefill(&lp, -1, 0);
     line_fill((char *) NULL, &(options.borderline), &lp, 0, 0, 1, 1);
     init_poly(&tmp_poly, 10);
     make_polybox(&tmp_poly, options.origin_x, options.origin_y, 
        (options.origin_x+get_xmax()), (options.origin_y+get_ymax()));
     draw_polyline(tmp_poly, 1, 0, &lp, 0);
     xfree((char *) tmp_poly.pts);
   }

   if (options.margin.unit==-1) {
     gfl_x=options.origin_x+options.edge*get_xmax()/100.0; 
     gfl_y=options.origin_y+options.edge*get_ymax()/100.0;
     gfr_x=options.origin_x+(100.0-options.edge)*get_xmax()/100.0;
     gfr_y=options.origin_y+(100.0-options.edge)*get_ymax()/100.0;
   }else{
     (void) scsize((char *) NULL, 'y', &crd_sz, &(options.margin), 1, -1);
     gfl_x=options.origin_x+crd_sz; 
     gfl_y=options.origin_y+crd_sz;
     gfr_x=options.origin_x+get_xmax()-crd_sz; 
     gfr_y=options.origin_y+get_ymax()-crd_sz;
   }

   jtitle=0.0;
   for (i=0;i<10;i++) {
      if ((titles[i].set==1)&&(titles[i].none==0)) {
        crd_sz= -(50+jtitle*50);
        halign_text(titles[i].inst, gfl_x, gfr_x, gfr_y, &crd_sz, 1, 0,
            T_CENTER);
        gfr_y=gfr_y-(1.2+jtitle/2)*crd_sz;
        jtitle=0.4;
      }
   }
   jfoot=0.0;
   for (i=0;i<10;i++) {
      if((foots[i].set==1)&&(foots[i].none==0)) {
        crd_sz=50+jfoot*50;
        halign_text(foots[i].inst, gfl_x, gfr_x, gfl_y, &crd_sz, 1, 0,
            T_CENTER);
        gfl_y=gfl_y+(1.2+jfoot/2)*crd_sz;
        jfoot=0.4;
      }
   }

#ifdef EBUG
 if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"Titles and Footnotes done\n");
   (void) fprintf(deb_log,"Lower corner %i %i\n",gfl_x,gfl_y);
   (void) fprintf(deb_log,"Upper corner %i %i\n",gfr_x,gfr_y);
   (void) fflush(deb_log);
 }
#endif

  /* B - determine edge requirements for axes values, labels and tick marks */

   for (i=0;i<3;i++) {
     tmp_tik[i]=tmp_val[i]=tmp_lab[i]=0;
     tmp_lab[i]=p_axes[i].label.limit.xmax-p_axes[i].label.limit.xmin;
     tmp_val[i]=p_axes[i].value.limit.xmax-p_axes[i].value.limit.xmin;
     if ((p_axes[i].major_t.inout&TK_OUT)!=0) 
          tmp_tik[i]=p_axes[i].major_t.length;
     if ((p_axes[i].minor_t.length>tmp_tik[i])&&
                          ((p_axes[i].minor_t.inout&TK_OUT)!=0)) {
        tmp_tik[i]=p_axes[i].minor_t.length;
     }
   }
   if (rv_axis!=0) {
      if ((p_axes[2].major_t.length!=0)||(p_axes[2].minor_t.length!=0)) {
         tick2_def=1;
      }else{
         if (options.fourtk!=0) tmp_tik[2]=tmp_tik[0];
      }
   }
   tmp_lab[0]=p_axes[0].label.limit.ymax-p_axes[0].label.limit.ymin;
   tmp_val[0]=p_axes[0].value.limit.ymax-p_axes[0].value.limit.ymin;

   if (polar_fl==0) {   /****** CARTESIAN SYSTEM ******/

  /* C1 - align the axes in the y direction */

   gr_end=1;
   if (rv_axis!=0) gr_end=2;
   set_llc=0;
   t_len= -1;
   for(i=gr_end;i>=0;i--) {
      if (p_axes[i].origin.y_c.unit!=-1) {
         set_llc=1;
         if ((i>0)&&(p_axes[i].ln_def.unit!=-1)&&(p_axes[i].length<0)) {
           llc_y=p_axes[i].origin.y+p_axes[i].length;
         }else{
           llc_y=p_axes[i].origin.y;
         }
      }
      if ((i>0)&&(p_axes[i].ln_def.unit!=-1)){
        if (crdabs(p_axes[i].length)>t_len) 
           t_len=crdabs(p_axes[i].length);
      }
   }

#ifdef EBUG
 if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"set_llc t_len llc_y %i %i %i\n",set_llc,t_len, llc_y);
   for (i=0;i<3;i++) {
     (void) fprintf(deb_log, "axis %i -> l v t %i %i %i\n", i, tmp_lab[i],
                    tmp_val[i], tmp_tik[i]);
   }
   (void) fflush(deb_log);
 }
#endif

   if (set_llc==0) {
     if (t_len<0) {
       llc_y=(int) gfl_y+(1.0+jfoot)*tmp_lab[0]+tmp_val[0]+tmp_tik[0];
       llc_y=(int) llc_y+p_axes[0].gaps.x+p_axes[0].gaps.y;
     }else{
       llc_y=(int) (gfl_y+gfr_y-t_len)/2;
       llc_y+=(int) ((1.0+jfoot)*tmp_lab[0]+tmp_val[0]+tmp_tik[0])/2;
       llc_y+=(int) (p_axes[0].gaps.x+p_axes[0].gaps.y)/2;
     }
   }
   for(i=gr_end;i>=0;i--) {
     if (p_axes[i].origin.y_c.unit==-1) {
       if ((i>0)&&(p_axes[i].ln_def.unit!=-1)&&(p_axes[i].length<0)) {
         p_axes[i].origin.y=llc_y-p_axes[i].length;
       }else{
         p_axes[i].origin.y=llc_y;
       }
     }
     if ((i>0)&&(p_axes[i].ln_def.unit==-1)){
       p_axes[i].length=gfr_y-p_axes[i].origin.y;
       p_axes[i].length-=((1.0+jtitle)*tmp_tik[0]);
     }
   }
   urc_y=llc_y;
   for(i=1;i<3;i++) {
      if (p_axes[i].length<0) {
         t_len=p_axes[i].origin.y;
      }else{
         t_len=p_axes[i].origin.y+p_axes[i].length;
      }
      if (t_len>urc_y) urc_y=t_len;
   }

#ifdef EBUG
 if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"Y's set -> %i %i\n", llc_y, urc_y);
   (void) fflush(deb_log);
 }
#endif

 /* D1 - align the axes in the x-direction */

   urc_x=(int) gfr_x;
   if (rv_axis!=0) {
     if (p_axes[2].origin.x_c.unit!=-1) {
	urc_x=p_axes[2].origin.x;
     }else{
        urc_x-=(tmp_lab[2]+tmp_val[2]+tmp_tik[2]);
	urc_x-=(p_axes[2].gaps.x+p_axes[2].gaps.y);
     }
   }else{
     urc_x-=tmp_tik[0];
   }
   llc_x=gfl_x+tmp_lab[1]+tmp_val[1]+tmp_tik[1];
   llc_x+=(p_axes[1].gaps.x+p_axes[1].gaps.y);
   if (p_axes[0].origin.x_c.unit!=-1) {
     if ((p_axes[0].ln_def.unit!=-1)&&(p_axes[0].length<0)) {
       llc_x=p_axes[0].origin.x+p_axes[0].length;
     }else{
       llc_x=p_axes[0].origin.x;
     }
   }
   if (p_axes[1].origin.x_c.unit!=-1) {
     llc_x=p_axes[1].origin.x;
   }else{
     p_axes[1].origin.x=llc_x;
   }

   if (p_axes[0].ln_def.unit==-1) {
     if (p_axes[0].origin.x_c.unit==-1) p_axes[0].origin.x=llc_x;
     if (p_axes[1].origin.x_c.unit==-1) p_axes[1].origin.x=llc_x;
     if (p_axes[2].origin.x_c.unit==-1) p_axes[2].origin.x=urc_x;
     p_axes[0].length=urc_x-p_axes[0].origin.x;
   }else{
     if (p_axes[0].origin.x_c.unit!=-1) {
       if (p_axes[1].origin.x_c.unit==-1) {
          if (p_axes[0].length<0) {
            p_axes[1].origin.x=p_axes[0].origin.x+p_axes[0].length;
          }else{
            p_axes[1].origin.x=p_axes[0].origin.x;
          }
	  llc_x=p_axes[1].origin.x;
       }
       if ((rv_axis==0)|(p_axes[2].origin.x_c.unit==-1)) {
          if (p_axes[0].length<0) {
	    p_axes[2].origin.x=p_axes[0].origin.x;
          }else{
	    p_axes[2].origin.x=p_axes[0].origin.x+p_axes[0].length;
          }
	  urc_x=p_axes[2].origin.x;
       }
     }else{
       if (p_axes[1].origin.x_c.unit!=-1) {
          if (p_axes[0].length<0) {
	    p_axes[0].origin.x=llc_x-p_axes[0].length;
          }else{
	    p_axes[0].origin.x=llc_x;
          }
	  if ((rv_axis==0)|(p_axes[2].origin.x_c.unit==-1)) {
	     p_axes[2].origin.x=llc_x+crdabs(p_axes[0].length);
	     urc_x=p_axes[2].origin.x;
	  }
       }else{
	  if ((rv_axis!=0)&&(p_axes[2].origin.x_c.unit!=-1)) {
             if (p_axes[0].length<0) {
	       p_axes[0].origin.x=p_axes[2].origin.x;
	       p_axes[1].origin.x=p_axes[0].origin.x+p_axes[0].length;
             }else{
	       p_axes[0].origin.x=p_axes[2].origin.x-p_axes[0].length;
	       p_axes[1].origin.x=p_axes[0].origin.x;
             }
	     llc_x=p_axes[1].origin.x;
	  }else{
	     p_axes[0].origin.x=(int) (llc_x+urc_x-p_axes[0].length)/2;
             if (p_axes[0].length<0) {
	       urc_x=p_axes[0].origin.x;
	       llc_x=p_axes[0].origin.x+p_axes[0].length;
             }else{
	       llc_x=p_axes[0].origin.x;
	       urc_x=p_axes[0].origin.x+p_axes[0].length;
             }
	     p_axes[1].origin.x=llc_x;
	     p_axes[2].origin.x=urc_x;
	  }
       }
     }
   }

 /* E1 - handle the halign/valign specifications */
   
   for (i=0;i<3;i++) {
     if (align_set[i]!=0) {
       if (i==0) {
         pos_inter((p_axes+1), aligns[0], 'y', &tk_v, 0);
         p_axes[0].origin.y=tk_v;
       }else{
         pos_inter(p_axes, aligns[i], 'x', &tk_v, 0);
         p_axes[i].origin.x=tk_v;
       }
     }
   }

#ifdef EBUG
 if (debug_level&DBG_GRAPH) {
   (void) fprintf(deb_log,"layout done\n");
   (void) fprintf(deb_log,"llc %i %i, urc %i %i\n",llc_x,llc_y,
                 urc_x,urc_y);
   (void) fprintf(deb_log,"axis 1 (%i, %i)\n",p_axes[0].origin.x,
                  p_axes[0].origin.y);
   (void) fprintf(deb_log,"axis 2 (%i, %i)\n",p_axes[1].origin.x,
                  p_axes[1].origin.y);
   (void) fprintf(deb_log,"axis 3 (%i, %i)\n",p_axes[2].origin.x,
                  p_axes[2].origin.y);
   (void) fflush(deb_log);
 }
#endif

   }else{            /****** POLAR SYSTEM ******/

   if (p_axes[1].origin.x_c.unit==-1) {
     if ((rv_axis!=0)&&(p_axes[2].origin.x_c.unit!=-1)) {
       p_axes[1].origin.x=p_axes[2].origin.x;
     }else{
       p_axes[1].origin.x=(gfl_x+gfr_x)/2;
     }
   }
   if (p_axes[1].origin.y_c.unit==-1) {
     if ((rv_axis!=0)&&(p_axes[2].origin.y_c.unit!=-1)) {
       p_axes[1].origin.y=p_axes[2].origin.y;
     }else{
       p_axes[1].origin.y=(gfl_y+gfr_y)/2;
     }
   }
   tk_v=p_axes[0].gaps.x+p_axes[0].gaps.y+tmp_lab[0]+tmp_val[0]+tmp_tik[0];
   gfl_x+=tk_v; gfr_x-=tk_v; gfl_y+=tk_v; gfr_y-=tk_v;
   max_r=gfr_x-p_axes[1].origin.x;
   if ((p_axes[1].origin.x-gfl_x)<max_r) max_r=p_axes[1].origin.x-gfl_x;
   if (((gfr_y-p_axes[1].origin.y)/eccentricity)<max_r) 
                max_r=(gfr_y-p_axes[1].origin.y)/eccentricity;
   if (((p_axes[1].origin.y-gfl_y)/eccentricity)<max_r)
                max_r=(p_axes[1].origin.y-gfl_y)/eccentricity;
   if (p_axes[1].ln_def.unit==-1) p_axes[1].length=max_r;
   if (p_axes[2].ln_def.unit==-1) p_axes[2].length=max_r;

   if (p_axes[0].origin.x_c.unit==-1) p_axes[0].origin.x=0;
   if (p_axes[0].origin.y_c.unit==-1) {
     p_axes[0].origin.y=p_axes[1].length;
   }
   if (p_axes[0].ln_def.unit==-1) p_axes[0].length=360000;
   
   xrad_cent=p_axes[1].origin.x;
   yrad_cent=p_axes[1].origin.y;
   p_axes[1].origin.y=p_axes[2].origin.y=0;
   p_axes[1].origin.x=p_axes[0].origin.x;
   p_axes[2].origin.x=p_axes[1].origin.x+90000;

   for (i=0;i<3;i++) {
     if (align_set[i]!=0) {
       if (i==0) {
         pos_inter((p_axes+1), aligns[0], 'y', &tk_v, 0);
         p_axes[0].origin.y=tk_v;
       }else{
         pos_inter(p_axes, aligns[i], 'x', &tk_v, 0);
         p_axes[i].origin.x=tk_v;
       }
     }
   }

   llc_x=p_axes[0].origin.x;
   urc_x=p_axes[0].origin.x+p_axes[0].length;
   llc_y=0;
   urc_y=p_axes[0].origin.y;

   }
   gr_drawn=1;

   /* F - layout is complete - begin display process */

   if (bound_set==0) {
     if (polar_fl==0) {
       init_poly(&clip_poly, 10);
       make_polybox(&clip_poly, llc_x, llc_y, urc_x, urc_y);
     }else{
       init_poly(&tmp_poly, -1);
       if (urc_x>llc_x) {beg=llc_x; end=urc_x;}
       else {beg=urc_x; end=llc_x;}
       for (crd_sz=beg; crd_sz<=end; crd_sz+=50123) {
         crd_tmp.x=crd_sz; crd_tmp.y=urc_y;
         poly_add_point(&tmp_poly, crd_tmp);
       }
       crd_tmp.x=end; crd_tmp.y=urc_y;
       poly_add_point(&tmp_poly, crd_tmp);
       if (p_axes[0].length!=360000) {
         crd_tmp.x=end; crd_tmp.y=0;
         poly_add_point(&tmp_poly, crd_tmp);
         crd_tmp.x=beg; crd_tmp.y=0;
         poly_add_point(&tmp_poly, crd_tmp);
       }
       crd_tmp= *(tmp_poly.pts);
       end=tmp_poly.n_points;
       *(tmp_poly.pts+end)=crd_tmp;
       conv_poly(tmp_poly, &clip_poly, 1);
       cent_poly(&clip_poly);
       xfree((char *) tmp_poly.pts);
     }
   }else{
     make_polydat(datas, &clip_poly, p_axes, (p_axes+1), &boundary,
                  (int **) NULL, -2, d_clips, cl_flag);
     if (polar_fl!=0) {
       conv_poly(clip_poly, &tmp_poly, 1);
       xfree((char *) clip_poly.pts);
       clip_poly=tmp_poly;
       cent_poly(&clip_poly);
     }
   }

   init_linefill(&clip_lp, -1, 0);
   if (frame_set!=0) line_fill((char *) NULL, &frame, &clip_lp, 0, 0, 1, 0);
   else frame.none=1;

   switch (clip_type) {
     case 2: case 0:
             init_clip(clip_poly, clip_lp, frame.none, 1, 1);
             break;
     case 3: save_clip(1, 0);
             break;
     default:break;
   }

   if ((fill_set!=0)&&(fill.none==0)) {
     init_linefill(&lp, -1, 1);
     line_fill((char *) NULL, &fill, &lp, 0, 1, 1, 0);
     cent_poly(&clip_poly);
     tn_fill(clip_poly, &lp);
   }

   /* contours */
   for (i=0;i<n_surfs;i++) {
     rc=build_grid(datas, p_axes, (p_axes+1), &grid, surfs[i], 0);
     if (rc<0) {
       add_note("Warning: Unable to construct surface grid representation.\n");
       add_note("         Check data points and setting of DATACLIP values.\n");
     }else{
       if (surfs[i].ax_n!=-1) {
         zaxis= *(axes[surfs[i].ax_n]);
         if (zaxis.label.set!=0) 
           zaxis.label.inst=copy_inst_tree(zaxis.label.inst);
       } else init_axis(&zaxis);
       dft_axis(&zaxis, "null", "h=30hpct", "z", T_CENTER);
       dsize(&zaxis, grid.zmin, grid.zmax, 10);
       size_axis(&zaxis, 'y');
       zaxis.offset.x=zaxis.offset.y=0;
       zaxis.origin.x=zaxis.origin.y=0;
       zaxis.length=1000000;
       nsym++;
       if (surf_type[i]==1) do_scatter(grid, &zaxis, nsym, 
               surfs[i].colourlims, surfs[i].ncolours);
       else do_contour(grid, &zaxis, nsym, surfs[i].colourlims,
               surfs[i].ncolours);
       destroy_grid(&grid);
     }
   }
   /* grids */

   ax_grid(p_axes, 'x', hv_gr_mm, (hv_gr_mm+1), hv_gr_set[0], hv_gr_set[1]);
   ax_grid((p_axes+1), 'y', (hv_gr_mm+2), (hv_gr_mm+3), hv_gr_set[2], 
            hv_gr_set[3]);

   /* references */

   for (i=0;i<n_lvref;i++) {
     pos_inter((p_axes+lv_refs[i].type), lv_refs[i].val, 
                d_dir[lv_refs[i].type], &tk_v, 0);
     init_linefill(&lp, -1, 0);
     line_fill((char *) NULL, &(lv_refs[i].line), &lp, 0, 0, 1, 1);
     if (lv_refs[i].line.none==0) {
       switch(lv_refs[i].type) {
         case 0 : tp_move(tk_v, llc_y, &lp);
                  tp_draw(tk_v, urc_y, &lp);
                  break;
         default: tp_move(llc_x, tk_v, &lp);
                  tp_draw(urc_x, tk_v, &lp);
                  break;
       }
     }
   }

   /* lines */

   for (l=0;l<2;l++) {
     for (i=0;i<ncrds[l];i++) {
       if ((crds[l]+i)->var_n[2]==-1) {
         nsym++;
         plot_table(datas, p_axes, (p_axes+l+1), (crds[l]+i), nsym, -1,
           d_clips, cl_flag);
       }else{
	 nent=0;
	 for(j=0;j<(crds[l]+i)->nrows;j++) {
	   get_num(datas, (crds[l]+i)->var_n[2], j, &i_val, &f_val);
           if (i_val<0) i_val=0;
	   fl=0;
	   for (k=0;k<nent;k++) {
	     if (table[k]==i_val) fl=1;
	   }
	   if (fl==0) table[nent++]=i_val;
	 }

	 bisort(table,nent);

	 for (j=0;j<nent;j++) {
           nsym++;
           plot_table(datas, p_axes, (p_axes+l+1), (crds[l]+i), nsym, 
                      table[j], d_clips, cl_flag);
	 }
       }
     } 
   }

   switch (clip_type) {
     case 0: del_clip(1);
             break;
     case 1: case 4:
             if (frame.none==0) draw_polyline(clip_poly, 1, 0, &clip_lp, 0);
             break;
     case 2: save_clip(1, 1);
             break;
     case 3: if (frame.none==0) draw_polyline(clip_poly, 1, 0, &clip_lp, 0);
             save_clip(1, 1);
             break;
     default:break;
   }

   if (no_draw[1]==0) draw_axis((p_axes+1), 'y', 0.0, tick2_def, 0);
   if (no_draw[0]==0) draw_axis(p_axes, 'x', 0.0, 0, 0);
   if ((rv_axis!=0)&&(no_draw[2]==0)) {
     draw_axis((p_axes+2), 'y', 180.0, 1, tmp_tik[2]);
   }

   switch (clip_type) {
     case 0: case 1: case 2: case 3:
             save_clip(1, 0);
             break;
     default:break;
   }

   error_on();
}

/* min_max - finds the min,max limits for the data crd
	   - does not reset min,max values to allow recursive call
           - index points to the vertical axis - vaxis or rvaxis 
              -if negative, zaxis for three-d
           - also tries to find a name for x, y (and z)*/

min_max(datas, crd, mins, maxs, index, nms)
struct sp_data *datas;
struct var_set crd;
double mins[3], maxs[3];
int index;
char *nms[3];
{
   int fl, j, i_val;
   double f_val;
   char *copy_buff();
   
   fl=0;
   if (index<0) {fl=1; index=1;}

   if (nms[0]==(char *) NULL) nms[0]=copy_buff(crd.name[0]);
   if (nms[index]==(char *) NULL) nms[index]=copy_buff(crd.name[1]);
   if ((fl!=0)&&(nms[2]==(char *) NULL)) nms[2]=copy_buff(crd.name[2]);

   for (j=0;j<crd.nrows;j++) {
     get_num(datas, crd.var_n[0], j, &i_val, &f_val);
     if (f_val<mins[0]) mins[0]=f_val;
     if (f_val>maxs[0]) maxs[0]=f_val;
     get_num(datas, crd.var_n[1], j, &i_val, &f_val);
     if (f_val<mins[index]) mins[index]=f_val;
     if (f_val>maxs[index]) maxs[index]=f_val;
     if (fl!=0) {
       get_num(datas, crd.var_n[2], j, &i_val, &f_val);
       if (f_val<mins[2]) mins[2]=f_val;
       if (f_val>maxs[2]) maxs[2]=f_val;
     }
   }
}

/* tp_move - moves to the coordinate in hax/vax space */

COORD tpn_oldx, tpn_oldy;

tp_move(x, y, style)
COORD x, y;
struct sp_linefill *style;
{
   tpn_oldx=x;
   tpn_oldy=y;
   pol_crd(&x, &y, 0);
   tn_move(x, y, style);
}

/* tp_draw - draws to the coordinate in hax/vax space, taking into
               account the curvature of the lines
           - note: line is broken to account for possible>90 adjustments 
               (see conv_poly)                                   */

tp_draw(x, y, style)
COORD x, y;
struct sp_linefill *style;
{
   struct sp_poly poly_in, poly_out;
   struct coordinate points[10];
   COORD crdabs();
   int i, np;

   if (polar_fl==0) {
     tn_draw(x, y, style);
     tpn_oldx=x;
     tpn_oldy=y;
     return;
   }

   np=crdabs(x-tpn_oldx)/50000+1;
   points[0].x=tpn_oldx;
   points[0].y=tpn_oldy;
   if (np!=1) for (i=1;i<=np;i++) {
       points[i].x=i*(x-tpn_oldx)/(np-1)+tpn_oldx;
       points[i].y=i*(y-tpn_oldy)/(np-1)+tpn_oldy;
   }
   if ((points[np-1].x!=x)||(points[np-1].y!=y)) {
     points[np].x=x;
     points[np++].y=y;
   }
   poly_in.n_points=np;
   poly_in.pts=points;
   conv_poly(poly_in, &poly_out, 0);
   for (i=1;i<poly_out.n_points;i++) {
     tn_draw((poly_out.pts+i)->x, (poly_out.pts+i)->y, style);
   }
   xfree((char *) poly_out.pts);
   tpn_oldx=x;
   tpn_oldy=y;
}
 
/* pol_crd - converts coordinate from hax/vax space into real space
           - rel indicates if center not to be included (for offset) */

pol_crd(x, y, rel)
COORD *x, *y;
int rel;
{
   double r, theta;

   if (polar_fl!=0) {
     r= *y;
     theta= *x/180000.0*m_pi;
     if (rel==0) {
       *x=r*cos(theta)+xrad_cent;
       *y=r*sin(theta)*eccentricity+yrad_cent;
     }else{
       *x=r*cos(theta);
       *y=r*sin(theta)*eccentricity;
     }
   }
}
