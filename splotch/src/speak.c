/******************************************************************
                          sPLOTch!
 
  Speak.c - Text output information.  Takes an input text command
            string (like from label) and compiles it into graphics
            coding.  Can return size information for layout purposes.

*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "fonts.h"
#include "spastic.h"

extern struct opt_def options;

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

extern char argbuff[], inbuff[], tmpbuff[];
extern double m_pi, deg_to_rad;

extern struct pair_v ax_val;
extern struct inst_key *ax_key;
extern int ax_fl;

static int x_sp[2][40]={{57, 71, 88, 97, 88, 74, 68, 57, 57},
                        {82, 74, 64, 57, 56, 49, 48, 53, 49, 36, 29,
                        27, 31, 32, 32, 25, 18, 6, 0, 4, 12, 25, 27, 29, 
                        27, 17, 14, 25, 36, 43, 47, 57, 68, 78, 95, 
                        100, 97, 94, 87, 82}};
       
static int y_sp[2][40]={{43, 45, 38, 21, 8, 13, 26, 42, 43},
                        {55, 57, 56, 53, 52, 44, 32, 19, 6, 0, 3, 12,
                        22, 30, 40, 38, 27, 30, 43, 57, 62, 65, 61, 65, 71,
                        82, 90, 100, 100, 86, 73, 75, 86, 91, 94, 75,
                        62, 57, 53, 55}};

static int n_sp[2]={8,39};
static char *arr_opt[]={"open", "close"};

extern struct pair_v sym_vals[];

static char fonsp_text[FONTSZ];

/*  speak - processes a command string into an instruction sequence (if
              strg non-NULL), or works with the sequence inst
          - txt is the text attribute description structure
          - drw is non-zero for text output to occur
          - lbl specifies label type (all relative occurences)
          - lpos gives string location for error indication
          - inst is the instruction sequence lead pointer
          - make_fl is non-zero if string -> instruction list
          - syval_fl indicates origin (0)other (1)symbol (2)value */
              
speak(strg, txt, drw, lbl, lpos, inst, make_fl, syval_fl)
char *strg;
struct sp_text *txt;
int drw, lbl, lpos, make_fl, syval_fl;
struct inst_key **inst;
{
  int com[4], i, j, rc, t_com, spec;
  int l, tmp, md_fl, xp, yp, rel, last_d_fl;
  int st_tab_n, tp_fl, supb_fl, no_j;
  char *t_ptr, *copy_buff();
  COORD crd_sz, xp0, yp0, supb_ox, supb_oy, del_x, del_y;
  double a, diff, ll;
  struct sp_text txt_ss, *st_tab[20];
  struct coordinate sp_crd[50];
  struct united sp_vu, sp_vub;
  struct sp_poly poly_a, poly_b;
  struct inst_key *c_inst, *leader, *make_inst();
  struct sp_linefill fill;

  if (make_fl!=0) {
    leader=make_inst(*inst, 1, 0, 0);
  }else{
    if (*inst==(struct inst_key *) NULL) return;
    leader= *inst;
  }
  c_inst=leader;

  txt->uline.toggle=com[1]=st_tab_n=last_d_fl=supb_fl=0;
  txt->style.curr_ln=0.0;
  del_y=supb_ox=supb_oy=0;
  del_x=1;
  no_j=1;

  do {
     if (c_inst!=(struct inst_key *) NULL) {
       c_inst->xp=txt->x0;
       c_inst->yp=txt->y0;
     }

     if (make_fl==0) {
       rc=1;
       if (c_inst==(struct inst_key *) NULL) {
         com[0]=0;
       }else{
         com[0]=c_inst->number;
       }
       com[1]=com[2]=com[3]=l=tmp=0;
       argbuff[0]='\0';
     }else{
       if (strg==(char *) NULL) com[1]=0;
       rc=scan_cmd(strg, com, lpos, &l, &tmp, argbuff, 1);
       if (strg==(char *) NULL) t_com= -com[3];
       else t_com=lpos+com[3];
     }

     if (rc<0) {
	what(BADCOM, strg, com, lpos, 1);
     }else{
        tp_fl=0;
	md_fl=0;
	switch(com[0]) {
	   case  0 : break;
	   case -2 : break;
	   case 14 : txt->none=1;
                     if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 14, AT_NONE);
                     }
		     break;
           case 157: if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 157, AT_NONE);
                     }
		     break;
	   case 56 : txt->lrc=T_LEFT;
                     no_j=0;
		     break;
	   case 57 : txt->lrc=T_RIGHT;
                     no_j=0;
		     break;
	   case 58 : txt->lrc=T_CENTER;
                     no_j=0;
		     break;
           case 155: if (make_fl!=0) {
                       for (i=0;i<2;i++) {
                         if (l_comp(argbuff, arr_opt[i])!=0) break;
                       }
                       if (i==2) {
                         sp_err(BADARRW, tmp, l);
                         break;
                       }else{
                         c_inst=make_inst(c_inst, 1, 155, AT_STR);
                         c_inst->arg.str=copy_buff(argbuff);
                       }
                     }
                     if (l_comp(c_inst->arg.str, arr_opt[0])!=0) {
                       txt->arrow_fl=1;
                     }else{
                       txt_arrow(txt, drw, txt->x0, txt->y0, del_x, del_y, 1);
                     }
                     break;
           case 154: if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 154, AT_NONE);
                     }
                     a=txt->r_angle;
                     txt->x0=txt->x0+11.0*txt->hfact*cos(a);
                     txt->y0=txt->y0+11.0*txt->hfact*sin(a);
                     for (j=0;j<2;j++) {
                       for (i=0;i<(n_sp[j]+1);i++) {
                         xp=(x_sp[j][i]-50)*0.18*txt->hfact;
                         yp=(y_sp[j][i]-50)*0.18*txt->hfact;
                         sp_crd[i].x=txt->x0+xp*cos(a)-yp*sin(a);
                         sp_crd[i].y=txt->y0+xp*sin(a)+yp*cos(a);
                       }
                       poly_a.n_points=n_sp[j];
                       poly_a.nlim= -1;
                       poly_a.pts=sp_crd;
                       b_norm_spline(poly_a, &poly_b, 1);
                       for (i=0;i<poly_b.n_points;i++) {
                         md_size(txt,0,(poly_b.pts+i)->x,(poly_b.pts+i)->y,0);
                       }
                       if ((drw-txt->noprint)>0) {
                         fill=txt->style;
                         fill.pattern=7;
                         tn_fill(poly_b, &fill);
                       }
                       xfree((char *) poly_b.pts);
                     }
                     txt->x0=txt->x0+11.0*txt->hfact*cos(a);
                     txt->y0=txt->y0+11.0*txt->hfact*sin(a);
                     break;
           case 128:
           case 115: if (st_tab_n>=19) sev_err(TOOBRA);
                     st_tab[st_tab_n]=(struct sp_text *)
                           xalloc((unsigned int) sizeof(struct sp_text),
                           "Unable to allocate text marker space.");
                     *(st_tab[st_tab_n++])= *txt;
                     txt->uline.toggle=0;
                     if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, com[0], AT_NONE);
                     }
		     break;
           case 129: tp_fl++;
           case 116: if (st_tab_n<1) {
                       if (make_fl!=0) sp_err(NOBRA, t_com, 1);
                     }else{
                       pull_tab(txt, st_tab, &st_tab_n, tp_fl);
                       if (make_fl!=0) {
                         c_inst=make_inst(c_inst, 1, com[0], AT_NONE);
                       }
                     }
		     break;
           case 66 : txt->noprint=0;
                     if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 66, AT_NONE);
                     }
		     break;
           case 67 : txt->noprint=1;
                     if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 67, AT_NONE);
                     }
		     break;
           case -12: tp_fl++;
           case -11: tp_fl++;
           case -10: if (syval_fl==0) {
                       if (make_fl!=0) sp_err(ILLCOM, t_com, 2);
                     }else{
                       if (make_fl!=0) {
                         if (rc==0) {
                           c_inst=make_inst(c_inst, 1, com[0], AT_NONE);
                         }else{
                           c_inst=make_inst(c_inst, 1, com[0], AT_STR);
                           c_inst->arg.str=copy_buff(argbuff);
                         }
                       }
                       if (syval_fl==2) {
                         if (ax_fl==2) {
                           if (c_inst->arg_type==AT_NONE) {
                             print_val(inbuff, (char *) NULL, tp_fl, ax_val);
                           }else{
                             (void) strcpy(argbuff, c_inst->arg.str);
                             print_val(inbuff, argbuff, tp_fl, ax_val);
                           }
                           if (strlen(inbuff)!=0) {
                             do_text(inbuff, txt, (drw-txt->noprint));
                             last_d_fl=0;
                           }
                         }else if (ax_fl==1) {
		           speak((char *) NULL, txt, drw, lbl, tmp,
                              &(ax_key), 0, syval_fl); 
                         }
                         txt->val_fl=1;
                       }else{
                         if (c_inst->arg_type==AT_NONE) {
                           print_val(inbuff, (char *) NULL, 3,
                                 sym_vals[tp_fl]);
                         }else{
                           (void) strcpy(argbuff, c_inst->arg.str);
                           print_val(inbuff, argbuff, 3,
                                 sym_vals[tp_fl]);
                         }
                         if (strlen(inbuff)!=0) {
                           do_text(inbuff, txt, (drw-txt->noprint));
                           last_d_fl=0;
                         }
                       }
                     }
#ifdef EBUG
  if (debug_level&DBG_UTIL) {
    (void) fprintf(deb_log,"Special command %i\n",com[0]);
    (void) fflush(deb_log);
  }
#endif
                     break;
           case -1 : if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, -1, AT_STR);
                       c_inst->arg.str=copy_buff(argbuff);
                     }
                     (void) strcpy(argbuff, c_inst->arg.str);
	             if (strlen(argbuff)!=0) {
                       do_text(argbuff, txt, (drw-txt->noprint));
                       last_d_fl=0;
                     }
		     break;
	   case 60 : md_fl++;
	   case 61 : md_fl++;
                     if (supb_fl!=0) {
                       txt->x0=supb_ox;
                       txt->y0=supb_oy;
                     }
		     txt_ss= *txt;
                     supb_ox=txt->x0;
                     supb_oy=txt->y0;
		     if (md_fl==1) {
	                yp= -10.5*txt->hfact;
		     }else{
			yp=10.5*txt->hfact;
		     }
	             xp=yp*tan((double) (txt->slant+txt->rotate));
	             a=txt->r_angle;
	             txt_ss.x0=txt_ss.x0+(xp*cos(a)-yp*sin(a));
	             txt_ss.y0=txt_ss.y0+(xp*sin(a)+yp*cos(a));
		     txt_ss.hfact=0.6*txt->hfact;
                     txt_ss.sub_fl=1;
		     xp0=txt_ss.x0;
		     yp0=txt_ss.y0;

                     if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, com[0], AT_COMM);
                       t_ptr=copy_buff(argbuff);
                       c_inst->arg.comm=(struct inst_key *) NULL;
                       speak(t_ptr, &txt_ss, drw, lbl, tmp,
                              &(c_inst->arg.comm), 1, syval_fl);
                       xfree(t_ptr);
                     }else{
		       speak((char *) NULL, &txt_ss, drw, lbl, tmp,
                              &(c_inst->arg.comm), 0, syval_fl); 
                     }
                     last_d_fl=0;

		     ll=(txt_ss.x0-xp0)*cos(a)+(txt_ss.y0-yp0)*sin(a);
		     txt->x0=txt->x0+ll*cos(a);
		     txt->y0=txt->y0+ll*sin(a);
		     txt->limit=txt_ss.limit;
		     txt->lrc=txt_ss.lrc;
		     txt->none=txt_ss.none;
		     txt->noprint=txt_ss.noprint;
                     txt->val_fl=txt_ss.val_fl;
                     supb_fl=1;
		     break;
           case 5  : if (make_fl!=0) {
                       rc=get_font(argbuff);
		       if (rc==-1) {
		   	 sp_err(BADFONT, tmp, l);
                         break;
		       }else{ 
                         c_inst=make_inst(c_inst, 1, 5, AT_STR);
                         c_inst->arg.str=copy_buff(argbuff);
                       }
		     }
                     rc=get_font(c_inst->arg.str);
                     if (rc!=-1) txt->font=rc;
		     break;
           case 125: tp_fl++;
           case 6  : tp_fl++;
           case 3  : if (make_fl!=0) {
                       rc=scan_num(argbuff, tmp, syval_fl);
                       if (rc<0) break;
                       c_inst=make_inst(c_inst, 1, com[0], AT_NUM);
                       c_inst->arg.num=atof(argbuff);
                       c_inst->num_sp_fl=rc;
                     }

                     if (c_inst->num_sp_fl==0) {
                       a=c_inst->arg.num;
                     }else{
                       a=sym_vals[c_inst->num_sp_fl-1].f;
                     }
                     switch(tp_fl) {
                       case 0: a=fmod(a, 360.0);
                               if (a<0.0) a+=360.0;
		               txt->rotate=(float) a*deg_to_rad;
                               break;
                       case 1: a=fmod(a, 180.0);
		               txt->slant=(float) a*deg_to_rad;
                               break;
                       default:txt->style.pattern=set_patt((int) (a+0.5));
                               break;
                     }
		     break;
           case 2  : if (make_fl!=0) {
                       rel=0;
                       t_ptr=argbuff;
		       if ((*t_ptr=='+')||(*t_ptr=='-')) rel=1;
                       if ((*t_ptr=='a')||(*t_ptr=='A')) t_ptr++;
                       rc=scan_num(t_ptr, tmp, syval_fl);
                       if (rc<0) break;
                       c_inst=make_inst(c_inst, 1, 2, AT_REL_NUM);
                       c_inst->arg.rel.val=atof(t_ptr);
                       c_inst->arg.rel.rel=rel;
                       c_inst->num_sp_fl=rc;
                     } 

                     if (c_inst->num_sp_fl==0) {
                       a=c_inst->arg.rel.val;
                     }else{
                       a=sym_vals[c_inst->num_sp_fl-1].f;
                     }
		     if (c_inst->arg.rel.rel!=0) a+=txt->angle;
                     a=fmod(a, 360.0);
                     if (a<0.0) a+=360.0;
                     txt->angle=a;
                     txt->r_angle=a*deg_to_rad;
	   case 7  : if (txt->uline.toggle==1) {
			md_size(txt, (drw-txt->noprint), txt->uline.x, 
                             txt->uline.y, 0);
			diff=fabs((double) txt->r_angle-txt->uline.a);
			if (diff>m_pi) diff-=2.0*m_pi;
			a=txt->uline.a+diff/2.0;
			ll=11.0/cos(txt->uline.a-a);
			txt->uline.x=txt->x0+ll*txt->uline.h*sin(a);
			txt->uline.y=txt->y0-ll*txt->uline.h*cos(a);
			txt->uline.a=txt->r_angle;
			md_size(txt, (drw-txt->noprint), txt->uline.x, 
                             txt->uline.y, 1);
                        last_d_fl=0;
			if (com[0]==7) txt->uline.toggle=0;
		     }else{
			if (com[0]==7) txt->uline.toggle=1;
			ll=txt->uline.h=txt->hfact;
			a=txt->uline.a=txt->r_angle;
			txt->uline.x=txt->x0+11.0*ll*sin(a);
			txt->uline.y=txt->y0-11.0*ll*cos(a);
		     }
                     if ((make_fl!=0)&&(com[0]==7)) {
                       c_inst=make_inst(c_inst, 1, 7, AT_NONE);
                     }
		     break;
           case 51 : tp_fl++;
           case 123: tp_fl++;
           case 112: tp_fl++;
           case 4  : if (make_fl!=0) {
                       i=scan_num(argbuff, tmp, syval_fl);
                       if (i<0) break;
    	               rc=scsize(argbuff, 'y', &crd_sz, &sp_vu, 1, M_HPCT);
                       if (rc<0) {
			 sp_err(BADSIZE, tmp, l);
                         break;
                       }
                       c_inst=make_inst(c_inst, 1, com[0], AT_NUM_UNIT);
                       c_inst->arg.num_unit=sp_vu;
                       c_inst->num_sp_fl=i;
                     }

                     sp_vu=c_inst->arg.num_unit;
                     if (c_inst->num_sp_fl!=0) {
                       sp_vu.val=sym_vals[c_inst->num_sp_fl-1].f;
                     }
                     if ((com[0]==123)&&(sp_vu.unit==-2)) {
                       rc=PCT;
                       crd_sz=sp_vu.val*options.linewidth;
                     }else{
    	               rc=scsize((char *) NULL, 'y', &crd_sz, &sp_vu, 1, -1);
                     }
                     if ((rc==HPCT)||(rc==RHPCT)) 
                            crd_sz=crd_sz*(txt->hfact/1000.0);
                     switch (tp_fl) {
                       case 1 : options.sht=crd_sz;
                       case 0 : txt->hfact=crd_sz/32.0;
                                break;
                       case 2 : txt->style.width=crd_sz;
                                break;
                       default: txt->style.repeat=crd_sz;
                                break;
                     }
		     break;
           case 124: if (make_fl!=0) {
                       c_inst=make_inst(c_inst, 1, 124, AT_COLOUR);
                       (void) get_colour(argbuff, &(c_inst->arg.colour.col),
                         &(c_inst->arg.colour.col_set), &(c_inst->num_sp_fl),
                         tmp, l, syval_fl);
                     }
                     set_colour(&(txt->style.colour), c_inst->arg.colour.col,
                         c_inst->arg.colour.col_set, c_inst->num_sp_fl);
                     break;
           case 127: md_fl++;
           case 126: md_fl++;
	   case 46 : md_fl++;
	   case 1  : if (((md_fl&1)!=0)&&(last_d_fl==0)) 
                        md_size(txt, (drw-txt->noprint), txt->x0, txt->y0, 0);
                     xp0=txt->x0;
                     yp0=txt->y0;
                     if (make_fl!=0) {
                       sp_vu.unit=sp_vub.unit= -1;
                       rc=get_coordinate(argbuff, tmp, &sp_vu, &sp_vub,
                            &spec, (lbl|(md_fl&2)), M_HPCT, syval_fl);
	    	       if (rc<0) {
		         sp_err(BADSIZE, tmp, l);
                         break;
                       }
                       c_inst=make_inst(c_inst, 1, com[0], AT_DUAL_UNIT);
                       c_inst->arg.dual_unit.a=sp_vu;
                       c_inst->arg.dual_unit.b=sp_vub;
                       c_inst->num_sp_fl=spec;
                     }

                     sp_vu=c_inst->arg.dual_unit.a;
                     if (sp_vu.unit!=-1) {
                       if ((c_inst->num_sp_fl&0x0F)!=0) {
                         sp_vu.val=sym_vals[(c_inst->num_sp_fl&0x0F)-1].f;
                       }
		       rc=scsize((char *) NULL, 'x', &crd_sz, &sp_vu,
                                   (lbl|(md_fl&2)), -1);
                       if ((rc==HPCT)||(rc==RHPCT)) 
                              crd_sz=crd_sz*(txt->hfact/1000.0);
                       if ((md_fl&2)!=0) {
                         txt->x0+=crd_sz*cos(txt->r_angle);
                         txt->y0+=crd_sz*sin(txt->r_angle);
                       }else{
                         if (rc>MID_UNIT) txt->x0+=crd_sz;
                         else txt->x0=crd_sz;
                       }
                     }

                     sp_vu=c_inst->arg.dual_unit.b;
                     if (sp_vu.unit!=-1) {
                       if ((c_inst->num_sp_fl&0xF0)!=0) {
                         sp_vu.val=sym_vals[(c_inst->num_sp_fl&0xF0)/0x10-1].f;
                       }
		       rc=scsize((char *) NULL, 'y', &crd_sz, &sp_vu,
                                   (lbl|(md_fl&2)), -1);
                       if ((rc==HPCT)||(rc==RHPCT))
                              crd_sz=crd_sz*(txt->hfact/1000.0);
                       if ((md_fl&2)!=0) {
                         txt->x0-=crd_sz*sin(txt->r_angle);
                         txt->y0+=crd_sz*cos(txt->r_angle);
                       }else{
                         if (rc>MID_UNIT) txt->y0+=crd_sz;
                         else txt->y0=crd_sz;
                       }
                     }

		     if ((md_fl&1)!=0) {
                        if ((txt->x0!=xp0)||(txt->y0!=yp0)) {
                          del_x=txt->x0-xp0;
                          del_y=txt->y0-yp0;
                        }
                        if (txt->arrow_fl!=0) {
                          txt->arrow_fl=0;
                          txt_arrow(txt, drw, xp0, yp0, del_x, del_y, 0);
                        }
			md_size(txt, (drw-txt->noprint), txt->x0, txt->y0, 1);
                        last_d_fl=1;
		     }else{
                        last_d_fl=0;
                     }
		     break;
	   default : if (make_fl!=0) {
                       com[1]=com[3];
		       what(ILLCOM, strg, com, lpos, 1);
                     }
		     break;
	}
        if ((com[0]!=60)&&(com[0]!=61)) supb_fl=0;

        if ((c_inst!=NULL)&&(make_fl==0)) {
            c_inst=c_inst->next_inst;
        }
     } 
     if ((make_fl!=0)&&(strg==(char *) NULL)) line_buff_flush(com[1],1);
#ifdef EBUG
  if (debug_level>0) {
    (void) fprintf(deb_log,"Text Limits %i x %i %i y %i\n", txt->limit.xmin,
       txt->limit.xmax, txt->limit.ymin, txt->limit.ymax);
  }
#endif
  } while (com[0]!=0);

  while (st_tab_n>0) pull_tab(txt, st_tab, &st_tab_n, 1);

  if (make_fl!=0) {
    if ((syval_fl==2)&&(txt->val_fl==0)&&(txt->sub_fl==0)) {
       c_inst=make_inst(c_inst, 1, -10, AT_NONE);
    }
    if (no_j==0) {
      switch(txt->lrc) {
        case T_LEFT   : leader->number=56;
                        break;
        case T_RIGHT  : leader->number=57;
                        break;
        case T_CENTER : 
        default       : leader->number=58;
                        break;
      }
      leader->arg_type=AT_NONE;
      if (*inst==(struct inst_key *) NULL) *inst=leader;
    }else{
      c_inst=leader->next_inst;
      del_inst(leader, 1);
      if (*inst==(struct inst_key *) NULL) *inst=c_inst;
    }
  }
}

/* halign_text - it happens so often, this routine handles
                 horizontal alignment and vertical centering to
                 the given margin pair
               - if to a point, make the margin pair equal 
               - if height non-zero it is percent of height to adjust by
               - returns the height for default stacking */

halign_text(inst, l_x, r_x, y, height, lbl, syval_fl, dft_align)
struct inst_key *inst;
COORD l_x, r_x, y, *height;
int lbl, syval_fl, dft_align;
{
   struct sp_text txt;
   COORD tpx, tpy;

   init_text(&txt);
   speak((char *) NULL, &txt, 0, lbl, 0, &inst, 0, syval_fl);
   tpy=y-(txt.limit.ymax+txt.limit.ymin)/2;
   if (height!=0) {
     tpy=tpy+(txt.limit.ymax-txt.limit.ymin)/100.0*(float) *height;
   }
   *height=txt.limit.ymax-txt.limit.ymin;
   if (txt.lrc<0) txt.lrc=dft_align;
   switch (txt.lrc) {
     case T_LEFT   : tpx=l_x-txt.limit.xmin;
                     break;
     case T_RIGHT  : tpx=r_x-txt.limit.xmax;
                     break;
     case T_CENTER : tpx=(l_x+r_x)/2-(txt.limit.xmax+txt.limit.xmin)/2;
                     break;
   }
   init_text(&txt);
   txt.x0=tpx;
   txt.y0=tpy;
   speak((char *) NULL, &txt, 1, lbl, 0, &inst, 0, syval_fl);
}

/* get_coordinate - compiles the definition of a unit coordinate 
                  - returns negative if error occurs
                  - will not alter sp_vu values if bad unit */

get_coordinate(buff, lpos, sp_vua, sp_vub, spec, rel, mask, syval_fl)
char *buff;
int lpos, *spec, rel, mask;
struct united *sp_vua, *sp_vub;
{
   int rc, k, ret_val, s1, s2;
   struct united sp_vu;
   COORD crd_sz;

   k=s1=s2=ret_val=0;

   rc=getc_buff(buff, &k, inbuff, 1000, ',');
   if (rc<0) k= -1;
   if (is_empty(inbuff)==0) {
     if ((s1=scan_num(inbuff, lpos, syval_fl))<0) {
       s1=0;
     }else{
       rc=scsize(inbuff, 'x', &crd_sz, &sp_vu, rel, mask);
       if (rc<0) ret_val= -1;
       else *sp_vua=sp_vu;
     }
   }

   if (k>=0) {
     lpos+=k;
     rc=getc_buff(buff, &k, inbuff, 1000, ',');
     if (is_empty(inbuff)==0) {
       if ((s2=scan_num(inbuff, lpos, syval_fl))<0) {
         s2=0;
       }else{
         rc=scsize(inbuff, 'y', &crd_sz, &sp_vu, rel, mask);
         if (rc<0) ret_val= -1;
         else *sp_vub=sp_vu;
       }
     }
   }

   *spec=s1|(s2*16);
   return(ret_val);
}

/*    pull_tab - no, not open a coke, but grab last storage in table
               - keeps xmin/max, ymin/max, but restores remainder
               - also keeps current position if k_pos non-zero */

pull_tab(txt, st_tab, st_tab_n, k_pos)
struct sp_text *txt, *st_tab[];
int *st_tab_n, k_pos;
{
   struct limits t_lim;
   COORD xp0, yp0;
   int t_set;

   t_lim=txt->limit;
   t_set=txt->max_set;
   xp0=txt->x0;
   yp0=txt->y0;

   *st_tab_n= *st_tab_n-1;
   *txt= *st_tab[*st_tab_n];
   txt->limit=t_lim;
   txt->max_set=t_set;
   if (k_pos!=0) {
     txt->x0=xp0;
     txt->y0=yp0;
   }
   xfree((char *) st_tab[*st_tab_n]);
}

/*  init_text - initialize character type descriptor (sp_text)   */

init_text(txt)
struct sp_text *txt;
{
  static struct sp_text def_t={0, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 0,
         {0, 0, 0, 0}, 1, 0, 0, -1, 0, 0, 0, {0.0, 0.0, 0.0, 0.0, 0},
         {1, 1, {0.0, 0.0, 0.0}, 0.0, 0.0}};
  COORD get_ymax();

  *txt=def_t;

  txt->font=options.dfont;
  if (options.sht==0.0) {
    txt->hfact=0.05*get_ymax()/32.0;
  }else{
    txt->hfact=options.sht/32.0;
  }
  init_linefill(&(txt->style), -1, 0);
}

/* get_font - finds font number according to name in buff
            - returns -1 if invalid
            - returns default font for dfont           */

int get_font(buff)
char buff[];
{
  int i, f;
  static char sp_font[]="dfont";

  f= -1;
  for (i=0;i<nfonts;i++) {
    if (l_comp(buff,fontnm[i])!=0) {
      f=i; 
      break;
    }
  }

  if (f==-1) {
    if (l_comp(buff,sp_font)!=0) {
      f=options.dfont;
    }
  }

  return(f);
}

/*      rdfont - read in the complete font motion data set     */

rdfont()
{
  FILE *fp;
  int i;
  char *font_file;
  static char dft_font[]=FONTFILE;
 
  if ((font_file=getenv("SPLOTCH_FONT"))==NULL) {
    font_file=dft_font;
  }
 
  if((fp=fopen(font_file,"r"))==NULL) {
    (void) fprintf(stderr,"Unable to load font data file:%s.\n",font_file);
    (void) fprintf(stderr,"Check or define the SPLOTCH_FONT environment");
    (void) fprintf(stderr," variable.\n");
    (void) exit(1);
  }

  for (i=0;i<FONTSZ;i++) {
    fonsp_text[i]=fgetc(fp);
  }
  
  (void) fclose(fp);
}

/*        do_text - text output routine
                  - takes in string, font info and aspect ratio
	          - outputs commands or simply sizes depending on drw
	          - handles rotate,angle and slant together  */

do_text(strg, type, drw)
char *strg;
struct sp_text *type;
int drw;
{
  int xl, xr, ptr, i, chr, pos, dr, np, dxr, dyr;
  double xp, yp, a;
  COORD xout, yout;
  char ch;

  a=type->r_angle-type->rotate;
  ptr=0;
  while((ch= *(strg+ptr++))!='\0') {
     
     chr=(int) ch-' ';
     if ((chr<0)||(chr>95)) chr=0;
     pos=fontpos[type->font][chr]+fontorig[type->font];
     np=fontsz[type->font][chr]-1;
     xl=(int) (fonsp_text[pos++]-'R');
     xr=(int) (fonsp_text[pos++]-'R');

     slip(type, xl, xr, 0);

     dr=0;
     if (np>0) {
	for(i=0;i<np;i++) {

	   if(fonsp_text[pos]==' ') {
	      dr=0; pos+=2; i++;
	   }

           dxr=fonsp_text[pos++]-'R';
           dyr=fonsp_text[pos++]-'R';
	   xp=dxr*fabs((double) type->hfact);
	   yp= -dyr*type->hfact;
	   xp=xp+yp*tan(type->slant);

	   xout=type->x0+(xp*cos(a)-yp*sin(a));
	   yout=type->y0+(xp*sin(a)+yp*cos(a));

           md_size(type, drw, xout, yout, dr);
           dr=1;
	}
     }

     slip(type, xl, xr, 1);
  }
}

/* txt_arrow - draws an open/close arrow along the indicated line,
               but using the text mode specifier */

txt_arrow(txt, drw, xp0, yp0, del_x, del_y, open_fl)
COORD xp0, yp0, del_x, del_y;
struct sp_text *txt;
int drw, open_fl;
{
   double ang;

   ang=atan2((double) del_y, (double) del_x)+open_fl*m_pi;

   ang+=options.arr_ang*deg_to_rad;
   md_size(txt, drw, (COORD) (xp0+options.arrow*cos(ang)),
     (COORD) (yp0+options.arrow*sin(ang)), 0);
   md_size(txt, drw, xp0, yp0, 1);

   ang-=2.0*options.arr_ang*deg_to_rad;
   md_size(txt, drw, (COORD) (xp0+options.arrow*cos(ang)),
     (COORD) (yp0+options.arrow*sin(ang)), 0);
   md_size(txt, drw, xp0, yp0, 1);
}

/*   md_size - move or with drw check and maximum determination 
             - md_fl non-zero for draw operation */

md_size(txt, drw, x, y, md_fl)
struct sp_text *txt;
int drw, md_fl;
COORD x, y;
{
   if ((x>txt->limit.xmax)||(txt->max_set==1)) txt->limit.xmax=x;
   if ((x<txt->limit.xmin)||(txt->max_set==1)) txt->limit.xmin=x;
   if ((y>txt->limit.ymax)||(txt->max_set==1)) txt->limit.ymax=y;
   if ((y<txt->limit.ymin)||(txt->max_set==1)) txt->limit.ymin=y;
   if (txt->max_set==1) txt->max_set=0;

   if (drw>0) {
#ifdef PCS
     if (md_fl==0) move_point(x, y);
     else draw_point(x, y);
#else
     if (md_fl==0) tn_move(x, y, &(txt->style));
     else tn_draw(x, y, &(txt->style));
#endif
   }
}

/*       slip - calculates the intercharacter spacing
	      - considers the rotation to determine length
	      - may need adjustment from flip criterion of 45 deg */

slip(type, xl, xr, side)
struct sp_text *type;
int xl, xr, side;
{
  int shift;
  double rot, testa, angle;

  angle=type->r_angle;
  rot=type->rotate/deg_to_rad;

  testa=fmod((rot+45.0+180.0*side), 360.0);
  if (testa<0.0) testa+=360.0;

  if (testa<90.0) shift= -xl;
  else if (testa<180.0) shift=10;
  else if (testa<270.0) shift=xr;
  else shift=14;

  testa=fmod(rot, 90.0);
  if (testa<0.0) testa+=90.0;
  if (testa>45.0) testa=90.0-testa;
  testa=testa*deg_to_rad;

  type->x0+=fabs((double) type->hfact)*cos(angle)/cos(testa)*shift;
  type->y0+=fabs((double) type->hfact)*sin(angle)/cos(testa)*shift;
}

/* make_inst - create a new instruction, linked to inst
             - if inst is NULL, create a new instruction chain
             - preset with command num and argument type
             - place after inst if append=1, before otherwise */

struct inst_key *make_inst(inst, append, num, type)
struct inst_key *inst;
int append, num, type;
{
   struct inst_key *curr_inst, *make_key_inst();

   curr_inst=(struct inst_key *) xalloc(sizeof(struct inst_key),
                    "Unable to allocate memory for text storage.");

   curr_inst->number=num;
   curr_inst->arg_type=type;
   curr_inst->num_sp_fl=0;

   if (inst==(struct inst_key *) NULL) {
     curr_inst->prev_inst=(struct inst_key *) NULL;
     curr_inst->next_inst=(struct inst_key *) NULL;
     return(curr_inst);
   }

   insert_inst(inst, curr_inst, append);

   return(curr_inst);
}

/* insert_inst - inserts curr_inst after inst if append=1
               - before inst if append=0                  */

insert_inst(inst, curr_inst, append)
struct inst_key *inst, *curr_inst;
int append;
{
   if (append==1) {
     if (inst->next_inst==(struct inst_key *) NULL) {
       inst->next_inst=curr_inst;
       curr_inst->prev_inst=inst;
       curr_inst->next_inst=(struct inst_key *) NULL;
     }else{
       curr_inst->next_inst=inst->next_inst;
       curr_inst->prev_inst=inst;
       (inst->next_inst)->prev_inst=curr_inst;
       inst->next_inst=curr_inst;
     }
   }else{
     if (inst->prev_inst==(struct inst_key *) NULL) {
       inst->prev_inst=curr_inst;
       curr_inst->prev_inst=(struct inst_key *) NULL;
       curr_inst->next_inst=inst;
     }else{
       curr_inst->prev_inst=inst->prev_inst;
       curr_inst->next_inst=inst;
       (inst->prev_inst)->next_inst=curr_inst;
       inst->prev_inst=curr_inst;
     }
   }
}

/* del_inst - delete appropriate instruction
            - connect surrounding instruction sequences
            - saves inst until next delete for undo commands
                (but only if dest_fl zero!!!!)        */

struct inst_key *old_inst=(struct inst_key *) NULL;

del_inst(inst, dest_fl)
struct inst_key *inst;
int dest_fl;
{
   struct inst_key *t_inst;

   if (dest_fl!=0) {
     t_inst=old_inst;
     old_inst=inst;
   }

   if (old_inst!=(struct inst_key *) NULL) {
      switch(old_inst->arg_type) {
        case AT_STR:
          xfree((char *) old_inst->arg.str);
          break;
        case AT_COMM:
          del_inst_tree(old_inst->arg.comm);
          break;
        default:
          break;
      }
   }

   if (inst!=(struct inst_key *) NULL) {
     if (inst->next_inst==(struct inst_key *) NULL) {
       if (inst->prev_inst!=(struct inst_key *) NULL) 
            (inst->prev_inst)->next_inst=(struct inst_key *) NULL;
     }else{
       if (inst->prev_inst==(struct inst_key *) NULL) {
         (inst->next_inst)->prev_inst=(struct inst_key *) NULL;
       }else{
         (inst->prev_inst)->next_inst=inst->next_inst;
         (inst->next_inst)->prev_inst=inst->prev_inst;
       }
     }
   }

   if (old_inst!=(struct inst_key *) NULL)  xfree((char *) old_inst);
   old_inst=inst;
   if (dest_fl!=0) old_inst=t_inst;
}

/* del_inst_tree - delete instructions from inst onward
                 - continues down tree branches of comm arguments */

del_inst_tree(inst)
struct inst_key *inst;
{
   struct inst_key *curr_inst, *t_inst;

   if (inst==(struct inst_key *) NULL) return;
   if (inst->prev_inst!=(struct inst_key *) NULL) 
       (inst->prev_inst)->next_inst=(struct inst_key *) NULL;
   curr_inst=inst;
   while (curr_inst!=(struct inst_key *) NULL) {
     t_inst=curr_inst->next_inst;
     del_inst(curr_inst, 1);
     curr_inst=t_inst;
   }
}

/* last_inst - returns the last instruction of a series */

struct inst_key *last_inst(inst)
struct inst_key *inst;
{
   struct inst_key *curr_inst;

   curr_inst=inst;
   if (curr_inst==(struct inst_key *) NULL) return(curr_inst);
   while (curr_inst->next_inst!=(struct inst_key *) NULL) 
       curr_inst=curr_inst->next_inst;

   return(curr_inst);
}

/* copy_inst - makes a copy of the single instruction inst */

struct inst_key *copy_inst(inst)
struct inst_key *inst;
{
   struct inst_key *t_inst, *make_inst(), *copy_inst_tree();
   char *copy_buff();

   t_inst=make_inst((struct inst_key *) NULL, 0, inst->number, inst->arg_type);
   *t_inst= *inst;
   t_inst->prev_inst=(struct inst_key *) NULL;
   t_inst->next_inst=(struct inst_key *) NULL;
   switch(t_inst->arg_type) {
     case AT_STR :
       t_inst->arg.str=copy_buff(inst->arg.str);
       break;
     case AT_COMM :
         t_inst->arg.comm=copy_inst_tree(inst->arg.comm);
       break;
     default :
       break;
   }

   return(t_inst);
}

/* copy_inst_tree - copy complete instruction tree of inst */

struct inst_key *copy_inst_tree(inst)
struct inst_key *inst;
{
   struct inst_key *t_inst, *c_inst, *f_inst, *copy_inst();

   f_inst=(struct inst_key *) NULL;
   c_inst=inst;
   while (c_inst!=(struct inst_key *) NULL) {
     t_inst=copy_inst(c_inst);
     if (c_inst==inst) {
       f_inst=t_inst;
     }else{
       t_inst->prev_inst=inst;
       inst->next_inst=t_inst;
     }
     inst=t_inst;
     c_inst=c_inst->next_inst;
   }

   return(f_inst);
}

/* copy_buff - returns the pointer to a copy of buff */

char *copy_buff(buff)
char *buff;
{
   char *ptr;

   ptr=(char *) xalloc((unsigned int) ((strlen(buff)+10)*sizeof(char)),
              "Unable to create buffer copy space.");
   (void) strcpy(ptr, buff);
   return(ptr);
}

/* scan_num - scans the string buff for #1, #2, etc. specifiers 
            - only legal if syval_fl==1
            - if found, replaced with 0.0 and returns specifier
            - if not found, returns 0
            - returns negative if error (bad value, etc.)  */

int scan_num(buff, lpos, syval_fl)
char *buff;
int syval_fl, lpos;
{
  int i,rc;

  for (i=0;i<strlen(buff);i++) {
    if (*(buff+i)=='#') {
      if ((*(buff+i+1)<'0')||(*(buff+i+1)>'9')||(syval_fl!=1)) {
        sp_err(BADSPEC, (lpos+i), 2);
        *(buff+i)='0';
        *(buff+i+1)='0';
        return(-1);
      }else{
        rc= *(buff+i+1)-'0'+1;
        *(buff+i)='0';
        *(buff+i+1)='0';
        return(rc);
      }
    }
  }
  return(0);
}

#ifdef EBUG
#define NUNITS 19 /* matches value in specific.c */
extern int ncomm;

extern char *coms[], *typen[];
extern int types[];
extern short comm[];

/* print_inst - prints the instruction set beginning at inst into buff
              - does num instructions, to end if negative 
              - if rec zero, do not continue down command trees */

print_inst(buff, inst, num, rec)
struct inst_key *inst;
char *buff;
int num, rec;
{
   char *ptr;
   struct inst_key *curr_inst;
   int i, j;

   ptr=buff;
   curr_inst=inst;
   while((curr_inst!=NULL)&&(num!=0)) {
     if (curr_inst->number==-1) {
       (void) sprintf(ptr,"'");
       ptr++;
       pr_argument(&ptr, curr_inst, rec);
       (void) sprintf(ptr,"'");
     }else if (curr_inst->number==-10) {
       (void) sprintf(ptr,"$#");
     }else if (curr_inst->number==-11) {
       (void) sprintf(ptr,"$$");
     }else if (curr_inst->number==-12) {
       (void) sprintf(ptr,"$%%");
     }else{
       for (i=0;i<ncomm;i++) {
         if (curr_inst->number==comm[i]) break;
       }
       if (i!=ncomm) {
         for (j=0;j<(strlen(coms[i])+1);j++) {
           *(ptr+j)=clower(coms[i][j]);
         }
         ptr=ptr+strlen(ptr);
         if (curr_inst->arg_type!=AT_NONE) {
           (void) sprintf(ptr,"=(");
           ptr=ptr+2;
           pr_argument(&ptr, curr_inst, rec);
           (void) sprintf(ptr,")");
         }
       }
     }
     ptr=ptr+strlen(ptr);
     (void) sprintf(ptr," ");
     ptr++;
     curr_inst=curr_inst->next_inst;
     num--;
   }
   ptr--;
   *ptr='\0';
}

/* pr_argument - print argument of inst into buff (incremented)
               - do the recursive thing if rec non-zero */

pr_argument(buff, inst, rec)
char **buff;
struct inst_key *inst;
int rec;
{
   switch(inst->arg_type) {
     case AT_NONE:
        break;
     case AT_NUM_INT:
        (void) sprintf(*buff,"%i",inst->arg.num_int);
        break;
     case AT_NUM:
        (void) sprintf(*buff,"%g",inst->arg.num);
        break;
     case AT_REL_NUM:
        if (inst->arg.rel.rel==1) {
          (void) sprintf(*buff,"%+g",inst->arg.rel.val);
        }else{ 
          if (inst->arg.rel.val<0) {
            (void) sprintf(*buff,"a%g",inst->arg.rel.val);
          }else{
            (void) sprintf(*buff,"%g",inst->arg.rel.val);
          }
        }
        break;
     case AT_NUM_UNIT:
        pr_unit(buff, inst->arg.num_unit.val, inst->arg.num_unit.unit);
        break;
     case AT_DUAL_UNIT:
        if (inst->arg.dual_unit.a.unit!=-1) {
          pr_unit(buff, inst->arg.dual_unit.a.val, 
              inst->arg.dual_unit.a.unit);
        }
        (void) sprintf(*buff,", ");
        *buff= *buff+2;
        if (inst->arg.dual_unit.b.unit!=-1) {
          pr_unit(buff, inst->arg.dual_unit.b.val, 
              inst->arg.dual_unit.b.unit);
        }
        break;
     case AT_STR:
        (void) sprintf(*buff,"%s",inst->arg.str);
        break;
     case AT_COMM:
        if (rec!=0) {
          print_inst(*buff, inst->arg.comm, -1, rec);
        }else{
          (void) sprintf(*buff,"...");
        }
        *buff= *buff+strlen(*buff);
        break;
     case AT_COLOUR:
        if (inst->arg.colour.col_set&1) {
          (void) sprintf(*buff,"%g",inst->arg.colour.col.hue);
          *buff= *buff+strlen(*buff);
        }
        (void) sprintf(*buff,", ");
        *buff= *buff+2;
        if (inst->arg.colour.col_set&2) {
          (void) sprintf(*buff,"%g",inst->arg.colour.col.sat);
          *buff= *buff+strlen(*buff);
        }
        (void) sprintf(*buff,", ");
        *buff= *buff+2;
        if (inst->arg.colour.col_set&4) {
          (void) sprintf(*buff,"%g",inst->arg.colour.col.bright);
          *buff= *buff+strlen(*buff);
        }
        break;
     default :
        break;
   }
   *buff= *buff+strlen(*buff);
   **buff='\0';
}

/* pr_unit - print val with unit into buff (incremented) */

pr_unit(buff, val, unit)
char **buff;
float val;
int unit;
{
   if ((unit<NUNITS)&&(unit>=0)) {
     (void) sprintf(*buff,"%g%s",val,typen[unit]);
   }else{
     (void) sprintf(*buff,"%g",val);
   }
   *buff= *buff+strlen(*buff);
}
#endif
