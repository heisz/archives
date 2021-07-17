/******************************************************************
                          sPLOTch!

  Spawn - special option systems that 'spawn' the rest of the
      graphical operations (ok, so I'm pushing it).
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

extern char argbuff[], inbuff[], tmpbuff[];
extern double m_pi, deg_to_rad;

static char *grids[]={"either", "grid", "triangle"};

/*  option - parse the option command line for graphics options
	   - needs the address of the options structure         */

option(opt) 
struct opt_def *opt;
{
   int rc, com[4], tmp, l, f, k, t_fl, i;
   double val, ll;
   COORD crd_sz, x_c, y_c, x_w, y_w;
   char *ptr, *copy_buff(), ch, next_non_white();
   struct coord3d temp;
   struct united jnk_vu;
   struct sp_linefill lp;
   FILE *fp, *search_path();

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
         t_fl=0;
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
	    case 33 : ptr=copy_buff(argbuff);
                      line_fill(ptr, &(opt->borderline), &lp, tmp, 0, 1, 1);
                      opt->border_set=1;
                      xfree(ptr);
		      break;
            case 74 : if ((rc!=0)&&(l_comp(argbuff,"off")!=0)) {
                        log_off();
                      }else if ((rc!=0)&&(l_comp(argbuff, "on")!=0)) {
                        error_on();
                      }else{
                        sp_err(BADLOG, tmp, l);
                      }
		      break;
	    case 39 : if (rc==0) opt->fourtk=1;
                      else opt->fourtk=atoi(argbuff);
		      break;
	    case 3  : opt->rotate=atoi(argbuff);
                      if ((opt->rotate<0)||(opt->rotate>3)) opt->rotate=0;
		      set_rotate(opt->rotate);
		      break;
	    case 40 : opt->fourtk=0;
		      break;
	    case 50 : rc=set_unit(argbuff);
		      if (rc<0) {
			sp_err(BADSIZE, tmp, l);
		      }else{
                        opt->def_unit=rc;
                      }
		      break;
            case 160: for (k=0;k<3;k++) {
                        if (l_comp(argbuff, grids[k])!=0) break;
                      }
                      if (k==3) {
                         sp_err(BADSURF, tmp, l);
                      }else{
                         opt->use_gridtri=k;
                      }
                      break;
            case 161: if (opt->colourlims!=(struct sp_colour *) NULL)
                           xfree((char *) opt->colourlims);
                      opt->colourlims=(struct sp_colour *) NULL;
                      opt->ncolours=0;
                      get_colourset(argbuff, tmp, &(opt->colourlims),
                          &(opt->ncolours));
                      break;
            case 169: t_fl++;
	    case 170: rc=scsize(argbuff, 'y', &crd_sz, &jnk_vu, 1, 0);
		      if (rc<0) {
			 sp_err(BADSIZE, tmp, l);
		      }else{
                         if (t_fl==0) {
                           opt->margin=jnk_vu;
                         }else{
                           opt->curv_res=crd_sz;
                         }
                      }
                      break;
            case 70 : t_fl++;
            case 48 : t_fl++;
	    case 138: rc=scsize(argbuff, 'x', &crd_sz, &jnk_vu, 1, 0);
		      if (rc<0) {
			 sp_err(BADSIZE, tmp, l);
		      }else{
			 if (t_fl==2) opt->dev_xmax=crd_sz;
			 if ((t_fl==1)||(t_fl==2)) opt->cp_xmax=crd_sz;
			 if ((t_fl==0)||(t_fl==2)) opt->gp_xmax=crd_sz;
			 if (opt->aspect>0) {
                           crd_sz=crd_sz*opt->aspect;
			   if (t_fl==2) opt->dev_ymax=crd_sz;
			   if ((t_fl==1)||(t_fl==2)) opt->cp_ymax=crd_sz;
			   if ((t_fl==0)||(t_fl==2)) opt->gp_ymax=crd_sz;
			 }
		      }
		      break;
            case 71 : t_fl++;
            case 49 : t_fl++;
	    case 137: rc=scsize(argbuff, 'y', &crd_sz, &jnk_vu, 1, 0);
		      if (rc<0) {
			 sp_err(BADSIZE, tmp, l);
		      }else{
			 if (t_fl==2) opt->dev_ymax=crd_sz;
			 if ((t_fl==1)||(t_fl==2)) opt->cp_ymax=crd_sz;
			 if ((t_fl==0)||(t_fl==2)) opt->gp_ymax=crd_sz;
			 if (opt->aspect>0) {
                           crd_sz=crd_sz/opt->aspect;
			   if (t_fl==2) opt->dev_xmax=crd_sz;
			   if ((t_fl==1)||(t_fl==2)) opt->cp_xmax=crd_sz;
			   if ((t_fl==0)||(t_fl==2)) opt->gp_xmax=crd_sz;
			 }
		      }
		      break;
            case 112: if (is_empty(argbuff)!=0) {
                        opt->sht=0;
                      }else{
                        rc=scsize(argbuff, 'y', &crd_sz, &jnk_vu, 1, 0);
                        if (rc<0) sp_err(BADSIZE, tmp, l);
                        else opt->sht=crd_sz;
                      }
                      break;
            case 156: k=0;
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (is_empty(inbuff)==0) {
                        rc=scsize(inbuff, 'y', &crd_sz, &jnk_vu, 1, 0);
                        if (rc<0) sp_err(BADSIZE, tmp, l);
                        else opt->arrow=crd_sz;
                      }
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (is_empty(inbuff)==0) {
                        opt->arr_ang=atof(inbuff);
                      }
                      break;
            case 113:
            case 123: rc=scsize(argbuff, 'y', &crd_sz, &jnk_vu, 1, 0);
                      if (rc<0) {
                        sp_err(BADSIZE, tmp, l);
                      }else{
                        if (jnk_vu.unit==-2) crd_sz=opt->linewidth*jnk_vu.val;
                        if (crd_sz<=0.0) {
                          sp_err(BADSIZE, tmp, l);
                        }else{
                          opt->linewidth=crd_sz;
                        }
                      }
                      break;
            case 167:
            case 124: (void) get_colour(argbuff, &(opt->linecolour), &t_fl,
                         &t_fl, tmp, l, 0);
                      break;
	    case 47 : close_screen();
                      dereg_sdvi();
                      i=0;
                      ch=next_non_white(argbuff, &i);
                      if (ch=='`') {
                        for (k=strlen(argbuff);k>i;k--) {  
                          if (argbuff[k]=='`') break;
                        }
                        argbuff[k]='\0';
                        fp=popen((argbuff+i+1), "w");
                        if (fp==(FILE *) NULL) {
                          sp_err(BADPIPE, tmp, l);
                        }else{
                          reg_sdvi(fp, 1);
                        }
                      }else{
                        fp=search_path((argbuff+i+1), "w", 0, 0);
                        if (fp==(FILE *) NULL) {
                          sp_err(NOFILE, tmp, l);
                        }else{
                          reg_sdvi(fp, 0);
                        }
                      }
		      break;
	    case 34 : val=atof(argbuff);
		      opt->edge=val;
		      break;
	    case 69 : val=atof(argbuff);
		      opt->aspect=val;
		      break;
            case 5  :
	    case 68 : f=get_font(argbuff);
		      if (f==-1) {
		 	 sp_err(BADFONT, tmp, l);
		      }else{ 
			 opt->dfont=f;
		      }
		      break;
            case 136: t_fl++;
	    case 37 : if (t_fl==0) {
                        x_c=opt->origin_x; y_c=opt->origin_y;
                        x_w=opt->cp_xmax; y_w=opt->cp_ymax;
                      }else{
                        x_c=opt->gp_orig_x; y_c=opt->gp_orig_y;
                        x_w=opt->gp_xmax; y_w=opt->gp_ymax;
                      }
                      k=0;
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (rc<0) k= -1;
                      if (is_empty(inbuff)==0) {
                        rc=scsize(inbuff, 'x', &crd_sz, &jnk_vu, 0, 0);
                        if (rc<0) {
                          sp_err(BADSIZE, tmp, l);
                        }else{
                          if (rc>MID_UNIT) x_c+=crd_sz;
                          else x_c=crd_sz;
                        }
                      }
                      if (k>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (rc<0) k= -1;
                        if (is_empty(inbuff)==0) {
                          rc=scsize(inbuff, 'y', &crd_sz, &jnk_vu, 0, 0);
                          if (rc<0) {
                            sp_err(BADSIZE, tmp, l);
                          }else{
                            if (rc>MID_UNIT) y_c+=crd_sz;
                            else y_c=crd_sz;
                          }
                        }
                      }
                      if (k>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (rc<0) k= -1;
                        if (is_empty(inbuff)==0) {
                          if (l_comp(inbuff,"center")!=0) {
                            x_c-=(x_w/2);
                          }else if (l_comp(inbuff,"right")!=0) {
                            x_c-=x_w;
                          }else if (l_comp(inbuff,"left")==0) {
                            sp_err(BADXALIGN, tmp, l);
                          }
                        }
                      }
                      if (k>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (is_empty(inbuff)==0) {
                          if (l_comp(inbuff,"center")!=0) {
                            y_c-=(y_w/2);
                          }else if (l_comp(inbuff,"top")!=0) {
                            y_c-=y_w;
                          }else if (l_comp(inbuff,"bottom")==0) {
                            sp_err(BADYALIGN, tmp, l);
                          }
                        }
                      }
	              if (t_fl==0) {
                        opt->origin_x=x_c; opt->origin_y=y_c;
                      }else{
                        opt->gp_orig_x=x_c; opt->gp_orig_y=y_c;
                      }
		      break;
            case 176: k=0;  
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (is_empty(inbuff)==0) {
                        opt->prj_x=atof(inbuff);
                      }
                      if (rc>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (is_empty(inbuff)==0) {
                          opt->prj_y=atof(inbuff);
                        }
                      }
                      break;
            case 93 : opt->dim_mode=3;
                      break;
            case 94 : opt->dim_mode=2;
                      break;
            case 111: val=atof(argbuff);
                      if (val>0.0) {
                        opt->winfract=val;
                      }else{
                        opt->winfract=1.0;
                      }
                      break;
            case 96 : val=atof(argbuff);
                      opt->screen=val;
                      opt->screen_set=1;
                      if (opt->screen<0) {
                        ll=opt->eyeball.x*opt->eyeball.x
                            +opt->eyeball.y*opt->eyeball.y
                            +opt->eyeball.z*opt->eyeball.z;
                        opt->screen=sqrt((double) ll)/2.0;
                        opt->screen_set=0;
                      }
                      break;
            case 108: t_fl++;
            case 101: t_fl++;
            case 95 : if (t_fl==0) {
                        (void) pull_coord(tmp, argbuff, l, &opt->eyeball, 3);
                      }else if (t_fl==1) {
                        (void) pull_coord(tmp, argbuff, l, &opt->vertical, 3);
                      }else{
                        (void) pull_coord(tmp, argbuff, l, &temp, 3);
                        opt->eyeball.x=temp.x*sin(deg_to_rad*temp.y);
                        opt->eyeball.x*=cos(deg_to_rad*temp.z);
                        opt->eyeball.y=temp.x*sin(deg_to_rad*temp.y);
                        opt->eyeball.y*=sin(deg_to_rad*temp.z);
                        opt->eyeball.z=temp.x*cos(deg_to_rad*temp.y);
                      }
                      if (opt->screen_set==0) {
                        ll=opt->eyeball.x*opt->eyeball.x
                            +opt->eyeball.y*opt->eyeball.y
                            +opt->eyeball.z*opt->eyeball.z;
                        opt->screen=sqrt((double) ll)/2.0;
                      }
                      comp_tvecs(opt);
                      break;
            case 97 : (void) pull_coord(tmp, argbuff, l, &opt->orig_2d, 3);
                      break;
            case 104: (void) pull_coord(tmp, argbuff, l, &opt->orig_3d, 3);
                      break;
            case 98 : (void) pull_coord(tmp, argbuff, l, &opt->vec_2dx, 3);
                      renormalize(&opt->vec_2dx);
                      break;
            case 99 : (void) pull_coord(tmp, argbuff, l, &opt->vec_2dy, 3);
                      renormalize(&opt->vec_2dy);
                      break;
            case 100: (void) pull_coord(tmp, argbuff, l, &opt->size_2d, 2);
                      break;
            case 103: (void) pull_coord(tmp, argbuff, l, &opt->size_3d, 3);
                      break;
	    default : com[1]=com[3];
		      what (ILLCOM, (char *) NULL, com, 0, 1); 
		      break;
	 }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);

#ifdef EBUG
  if (debug_level>1) {
    (void) fprintf(deb_log,"screen %g\n",opt->screen);
    (void) fprintf(deb_log,"eye %g %g %g\n",
               opt->eyeball.x,opt->eyeball.y,opt->eyeball.z);
    (void) fprintf(deb_log,"orig2d %g %g %g\n", 
               opt->orig_2d.x,opt->orig_2d.y,opt->orig_2d.z);
    (void) fprintf(deb_log,"orig3d %g %g %g\n", 
               opt->orig_3d.x,opt->orig_3d.y,opt->orig_3d.z);
    (void) fprintf(deb_log,"vecx %g %g %g\n", 
               opt->vec_2dx.x,opt->vec_2dx.y,opt->vec_2dx.z);
    (void) fprintf(deb_log,"vecy %g %g %g\n", 
               opt->vec_2dy.x,opt->vec_2dy.y,opt->vec_2dy.z);
    (void) fprintf(deb_log,"size2d %g %g %g\n", 
               opt->size_2d.x,opt->size_2d.y,opt->size_2d.z);
    (void) fprintf(deb_log,"size3d %g %g %g\n", 
               opt->size_3d.x,opt->size_3d.y,opt->size_3d.z);
    (void) fprintf(deb_log,"vertical %g %g %g\n", 
               opt->vertical.x,opt->vertical.y,opt->vertical.z);
    (void) fprintf(deb_log,"tvecx %g %g %g\n", 
               opt->tvecx.x,opt->tvecx.y,opt->tvecx.z);
    (void) fprintf(deb_log,"tvecy %g %g %g\n", 
               opt->tvecy.x,opt->tvecy.y,opt->tvecy.z);
    (void) fprintf(deb_log,"tvecz %g %g %g\n", 
               opt->tvecz.x,opt->tvecz.y,opt->tvecz.z);
  }
#endif
}

/* init_opt - initializes the options structure to defaults */

init_opt(opt)
struct opt_def *opt;
{
   static struct opt_def first_opt={
    PCT, 360, 900, 10.0, {0.0, -1},
    {{0.0, -1}, {0.0,0.0,0.0}, 0, {0.0, -1}, 0, 0, 0, 0, 0},
    0, 5.0, 0, 0, 0, 0, 0, -1.0, 1.0, 0, 0, 0, 0, 0, 

    40,{0.0,0.0,0.0}, 0, 0, 0, 0,

    2, 250.0, 0,
    {418.0,112.0,250.0},{0.0,0.0,0.0},{1.0,0.0,0.0},
    {0.0,1.0,0.0},{100.0,100.0,-1.0},{0.0,0.0,1.0},
    {1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0},
    {100.0,100.0,100.0},{0.0,0.0,0.0}, 50.0, 50.0,
    0, (struct sp_colour *) NULL, 0
    };

   *opt=first_opt;
   init_lf_def(&(opt->borderline));
   set_rotate(opt->rotate);
   comp_tvecs(opt);
}

/* comp_tvecs - precalculate the viewpoint-aligned projection vectors */

comp_tvecs(opt)
struct opt_def *opt;
{
   struct coord3d temp;

   cross_product(opt->eyeball, opt->vertical, &temp);
   cross_product(temp, opt->eyeball, &opt->tvecy);
   opt->tvecz=opt->eyeball;
   cross_product(opt->tvecy, opt->tvecz, &opt->tvecx);
   renormalize(&opt->tvecx);
   renormalize(&opt->tvecy);
   renormalize(&opt->tvecz);
}
