/******************************************************************
                          sPLOTch!

  Spindle - routines related to axis definition, layout and
    output.
 
*******************************************************************/      

#include <stdio.h>
#include "splotch.h"
#include <math.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

#define RED_FACT 0.8   /* value default reduction factor */

extern COORD llc_x, llc_y, urc_x, urc_y;
extern struct opt_def options;
extern char argbuff[],inbuff[],tmpbuff[];
extern double m_pi, deg_to_rad;

struct pair_v ax_val;
struct inst_key *ax_key;
int ax_fl, n_dec_pl;
double ax_base;

/*  init_axis - initializes axis data to defaults
	      - clears set masks for uninitializable data */

init_axis(axis)
struct axis_def *axis;
{
   int i;


   if (axis->set_flag!=0) {
     if (axis->value.set!=0) del_inst_tree(axis->value.inst);
     if (axis->label.set!=0) del_inst_tree(axis->label.inst);
     for (i=0;i<axis->n_clist;i++) {
       del_inst_tree(axis->clist[i]);
     }
   }
   axis->set_flag=1;

   axis->major_t.length_def.val=2;
   axis->major_t.length_def.unit=13;
   axis->major_t.inout=TK_OUT;
   axis->major_t.num=1;
   axis->major_t.none=0;
   axis->major_t.noprint=0;
   init_lf_def(&(axis->major_t.style));
   axis->minor_t=axis->major_t;
   axis->minor_t.num=1;
   axis->minor_t.length_def.val=1;

   axis->label.inst=(struct inst_key *) NULL;
   axis->label.none=0;
   axis->label.set=0;
   axis->label_lrc= -1;
   axis->value=axis->label;
   axis->value_lrc= -1;

   init_lf_def(&(axis->style));

   axis->nvalues= -1;
   axis->ndec_places= -1;
   axis->extend=0;
   axis->log_flag=0;
   axis->base= -1.0;
   axis->set=0;
   axis->exp=0;
   axis->exp_set=0;

   axis->offset.x_c.val=0.0;
   axis->offset.x_c.unit= -2;
   axis->offset.y_c=axis->offset.x_c;

   axis->origin.x_c.unit= -1;
   axis->origin.y_c.unit= -1;

   axis->gaps.x_c.val=4.0;
   axis->gaps.x_c.unit=13;
   axis->gaps.y_c=axis->gaps.x_c;

   axis->ln_def.unit= -1;
   axis->rad_fl=0;
   axis->n_clist=0;
   axis->n_vlist=0;
}
   
/*  size_axis - set label,value and tick sizes before plotting
              - also lengths, offsets and origins
	      - nulls the sizing information if none is set
	      - needs order information set to determine value size */

size_axis(axis, dir)
struct axis_def *axis;
char dir;
{
   struct sp_text txt;
   int max_x, max_y, x, y, i, dec, endi, begi, dr;

   max_x= -1;
   max_y= -1;

   init_text(&txt);
   if (axis->label.none==0)
        speak((char *) NULL, &txt, 0, 1, 0, &(axis->label.inst), 0, 0);
   axis->label.limit=txt.limit;

   if (axis->n_vlist>0) {
      axis->nvalues=axis->n_vlist;
   }else if(axis->log_flag==0) {
      axis->nvalues=((axis->end-axis->start)/axis->step+1.0001);
   }else{
      dr=1;
      if (axis->end<axis->begin) dr= -1;
      if (axis->base<0.0) {
        endi=fint((log10(axis->end)+0.0001),-1);
        begi=fint((log10(axis->begin)+0.0001),-1);
      }else{
        endi=fint((log(axis->end)/log(axis->base)+0.0001),-1);
        begi=fint((log(axis->begin)/log(axis->base)+0.0001),-1);
      }
      axis->nvalues=dr*(endi-begi)+1;
   }

   if (axis->value.none!=0) {
      axis->ndec_places=0;
   }else{
     for(i=0;i<axis->nvalues;i++) {
	init_text(&txt);
        txt.hfact=RED_FACT*txt.hfact;
        dec=value_put(axis, &txt, 0, i, -1);
        if (dec>axis->ndec_places) axis->ndec_places=dec;
     }
   }

   if (axis->log_flag!=0) axis->ndec_places= -1;
   for(i=0;i<axis->nvalues;i++) {
     init_text(&txt);
     txt.hfact=RED_FACT*txt.hfact;
     if (axis->value.none==0) 
            (void) value_put(axis, &txt, 0, i, axis->ndec_places);
      x=txt.limit.xmax-txt.limit.xmin;
      y=txt.limit.ymax-txt.limit.ymin;
      if (x>max_x) {
        axis->value.limit.xmin=txt.limit.xmin;
        axis->value.limit.xmax=txt.limit.xmax;
        max_x=x;
      }
      if (y>max_y) {
        axis->value.limit.ymin=txt.limit.ymin;
        axis->value.limit.ymax=txt.limit.ymax;
        max_y=y;
      }
   }

   axis->minor_t.length=axis->major_t.length=0;
   if(axis->major_t.none==0) {
      (void) scsize((char *) NULL, 'y', &(axis->major_t.length),
          &(axis->major_t.length_def), 1, -1);
   }
   if(axis->minor_t.none==0) {
      (void) scsize((char *) NULL, 'y', &(axis->minor_t.length),
          &(axis->minor_t.length_def), 1, -1);
   }

   size_crd(&(axis->offset), dir, dir, 1, axis->rad_fl, axis->rad_fl, 0);
   size_crd(&(axis->gaps), 'y', 'y', 1, 0, 0, 0);
   size_crd(&(axis->origin), 'x', 'y', ((axis->rad_fl>0)?1:0), 
            axis->rad_fl, 0, 0);

   if (axis->ln_def.unit!=-1) {
      (void) scsize((char *) NULL, dir, &(axis->length),
           &(axis->ln_def), 1, -1);
      if (axis->rad_fl>0) {
        if (axis->ln_def.unit==-2) axis->length=axis->ln_def.val*1000;
        else axis->length=axis->ln_def.val*3600;
      }
   }
}

/*  get_value - gets the nth value of the major set
	      - usually just begin+step*n
	      - this is where the log and lists come in 
	      - multiplies indicated value by 10**exp if flag=0
	      - returns value in val  */

get_value (axis, n, val, flag)
struct axis_def *axis;
int n, flag;
double *val;
{
  int dir, tmp;
  double tval;

  if (axis->n_vlist>0) {
     if (n<0) n=0;
     else if (n>=axis->n_vlist) n=axis->n_vlist-1;
     tval=axis->vlist[n];
  }else if (axis->log_flag==0) {
     tval=axis->start+n*axis->step;
  }else{
     dir=1;
     if (axis->end<axis->begin) dir= -1;
     if (axis->base<0.0) {
       tmp=fint((log10(axis->begin)+0.0001),-1)+dir*n;
       tval=exp((double) tmp*log((double) 10.0));
     }else{
       tmp=fint((log(axis->begin)/log(axis->base)+0.0001),-1)+dir*n;
       tval=(double) exp((double) (tmp*log(axis->base)));
     }
  }
  if (flag==0) {
     *val=tval*exp((double) axis->exp*log((double) 10.0));
  }else{
     *val=tval;
  }
}

/*  value_put - inserts val as a string into proper position in
		 value specifier
              - translates control string for both sizing and output
	      - uses specified decimal places, or computes it if -ve
	      - returns number of decimal places used  */

int value_put(axis, txt, drw, iii, dec)
struct axis_def *axis;
struct sp_text *txt;
int drw, dec, iii;
{
    int c, dtmp, i;
    double val;

    dtmp=dec;
    ax_fl=0;

    if ((iii<axis->n_clist)&&(iii>-1)&&
                (axis->clist[iii]!=(struct inst_key *) NULL)) {
      ax_key=axis->clist[iii];
      ax_fl=1;
      dtmp= -1;
    }else{
      ax_fl=2;
      get_value(axis, iii, &val, 1);
      ax_val.i=(int) val;
      ax_val.f=val;
      ax_val.type=TYPE_FLT;
      if (dtmp>=0) {
         n_dec_pl=dtmp;
      }else{
         (void) sprintf(tmpbuff,"%f",val);
         c=strlen(tmpbuff)-1;
         while ((c>-1)&&(tmpbuff[c]=='0')) c--;
         if (c==-1) tmpbuff[c=0]='0';
         if (tmpbuff[c]=='.') c--;
         tmpbuff[c+1]='\0';
         dtmp= -1;
         for (i=0;i<strlen(tmpbuff);i++) {
           if ((tmpbuff[i]=='.')||(dtmp!=-1)) dtmp++;
         }
         if (dtmp==-1) dtmp=0;
         n_dec_pl=dtmp;
      }
      ax_base=axis->base;
    }

    speak((char *) NULL, txt, drw, 1, 0, &(axis->value.inst), 0, 2);
    return(dtmp);
}

/* print_val - generates a formatted string for axis and symbol values
             - uses default (based on type) if format is NULL */

print_val(buff, fmt, type, value)
char *buff, *fmt;
int type;
struct pair_v value;
{
   int flag, tmpa, tmpb, tmpc;
   double val;

   switch(type) {
     case 0: if (fmt==(char *) NULL) {
               (void) sprintf(buff, "%.*f", n_dec_pl, value.f);
             }else{
               flag=is_flt_fmt(fmt); 
               if (flag==0) (void) sprintf(buff, fmt, value.i);
               else (void) sprintf(buff, fmt, value.f);
             }
             break;
     case 1: if (value.f>0.0) {
               if (ax_base<=0.0) val=log10((double) value.f);
               else val=log((double) value.f)/log((double) ax_base);
               tmpa=fint((val+0.0001), -1);
               if (fmt==(char *) NULL) {
                 (void) sprintf(buff, "%i", tmpa);
               }else{
                 flag=is_flt_fmt(fmt); 
                 if (flag==0) (void) sprintf(buff, fmt, tmpa);
                 else (void) sprintf(buff, fmt, val);
               }
             }else{
               (void) strcpy(buff, "Undef");
             }
             break;
     case 2: if (fmt==(char *) NULL) {
               tmpc=value.f*3600.0+0.5;
               tmpa=tmpc/3600;
               tmpb=tmpc/60-tmpa*60;
               tmpc=tmpc%60;
               (void) sprintf(buff,"%i:%02i:%02i", tmpa, tmpb, tmpc);
             }else{
               tmpc=value.f*3600.0;
               tmpa=value.f;
               tmpb=tmpc/60-tmpa*60;
               (void) sprintf(buff, fmt, tmpa, tmpb,
                   (((value.f-tmpa)*60.0-tmpb)*60.0));
             }
             break;
     default:if (fmt==(char *) NULL) {
               if (value.type==TYPE_INT) {
                 (void) sprintf(buff, "%i", value.i);
               }else{
                 (void) sprintf(buff, "%g", value.f);
               }
             }else{
               flag=is_flt_fmt(fmt); 
               if (flag==0) (void) sprintf(buff, fmt, value.i);
               else (void) sprintf(buff, fmt, value.f);
             }
             break;
   }
}

int is_flt_fmt(fmt)
char *fmt;
{
   int i, flag;
   char ch;

   flag=0;
   for (i=0;i<strlen(fmt);i++) {
     ch=clower(*(fmt+i));
     if ((ch=='e')||(ch=='f')||(ch=='g')) flag=1;
   }
   return(flag);
}

/*  pos_inter - interpolates a coordinate position based on value
	      - uses length, offset and origin settings  
              - returns MAX_CRD if out_fl non-zero and exceeds margins */

pos_inter(axis, val, dir, pos, out_fl)
struct axis_def *axis;
double val;
char dir;
COORD *pos;
int out_fl;
{
   int i, del, loc, d_fl;
   double tmp, vall, valr, tp;

   *pos=0;
   if ((axis==(struct axis_def *) NULL)||(axis->set_flag==0)) {
     return;
   }

   val=val/exp((double) axis->exp*log((double) 10.0));
   d_fl=1;
   if (axis->length<0.0) d_fl= -1;
   del=axis->length-d_fl*axis->offset.x-d_fl*axis->offset.y;
   if (dir=='x') {
      loc=axis->origin.x+d_fl*axis->offset.x;
   }else{ 
      loc=axis->origin.y+d_fl*axis->offset.x;
   }
   if ((axis->set!=0)||(axis->n_vlist==0)) {
     if (axis->log_flag==0) {
       tmp=(val-axis->begin)/(axis->end-axis->begin);
       if ((out_fl!=0)&&((tmp<-0.0001)||(tmp>1.0001))) {
         *pos=MAX_CRD;
       }else{
         tp=loc+tmp*del;
       }
     }else{
       if (val>0.0) {
         tmp=(log(val)-log(axis->begin));
         tmp=tmp/(log(axis->end)-log(axis->begin));
         if ((out_fl!=0)&&((tmp<-0.0001)||(tmp>1.0001))) {
           *pos=MAX_CRD;
         }else{
           tp=loc+tmp*del;
         }
       }else{
	 *pos=MAX_CRD;
       }
     }
   }else{
     for(i=0;i<(axis->nvalues-1);i++) {
	get_value(axis,i,&vall,1);
	get_value(axis,(i+1),&valr,1);
	tmp=(val-vall)/(valr-vall);
	if ((tmp>=0.0)&&(tmp<=1.0)) break;
     }
     if (i==(axis->nvalues-1)) {
	*pos=MAX_CRD;
     }else{
	if (axis->log_flag==0) {
	  loc+=del*i/(axis->nvalues-1.0);
	  loc+=tmp*del/(axis->nvalues-1.0);
          tp=loc;
	}else{
          if (val>0.0) {
             tmp=(log(val)-log(vall));
             tmp=tmp/(log(valr)-log(vall));
	     loc+=del*i/(axis->nvalues-1.0);
             tp=loc+tmp*del/(axis->nvalues-1.0);
          }else{
	     *pos=MAX_CRD;
          }
	}
     }
   }
   if (axis->rad_fl>0) tp=fmod(tp, (double) 720000.0);

   if ((fabs(tp)>MAX_CRD)||(*pos==MAX_CRD)) *pos=MAX_CRD;
   else *pos=tp;
}
 
/*    do_axis - interprets the axis command list
	      - input data begins at ptr 
	      - lpos identifies string position for errors
	      - data is a pointer to the axis number in question */

do_axis(data)
struct axis_def *data;
{
   char ch, *tptr, *copy_buff();
   int k, rc, l, com[4], type, i, tmp, lj, tp_fl, k_o;
   COORD crd_sz;
   struct sp_text txt;
   struct united sp_vu;
   struct sp_linefill lpoly;

   if (data->set_flag==0) init_axis(data);

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0 , &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
         tp_fl=0;
	 switch (com[0]) {
	    case 0:   break;
            case -2:  break;
            case 139: init_axis(data);
                      break;
	    case 9:   type=MAJOR;
	    case 10:  if (com[0]==10) type=MINOR;
                      tptr=copy_buff(argbuff);
		      if (type==MAJOR) tick(tptr, tmp, &(data->major_t));
		      if (type==MINOR) tick(tptr, tmp, &(data->minor_t));
                      xfree(tptr);
		      break;
   	    case 15:  init_text(&txt);
                      txt.hfact=RED_FACT*txt.hfact;
                      if (data->value.set!=0) del_inst_tree(data->value.inst);
                      tptr=copy_buff(argbuff);
                      data->value.inst=(struct inst_key *) NULL;
                      speak(tptr, &txt, 0, 1, tmp, &(data->value.inst), 1, 2);
                      xfree(tptr);
		      data->value.set=1;
		      data->value.none=txt.none;
		      data->value_lrc=txt.lrc;
		      break;
   	    case 16:  init_text(&txt);
                      if (data->label.set!=0) del_inst_tree(data->label.inst);
                      tptr=copy_buff(argbuff);
                      data->label.inst=(struct inst_key *) NULL;
                      speak(tptr, &txt, 0, 1, tmp, &(data->label.inst), 1, 0);
                      xfree(tptr);
		      data->label.set=1;
		      data->label.none=txt.none;
		      data->label_lrc=txt.lrc;
		      break;
            case 17:  if (is_empty(argbuff)!=0) {
                        data->set=0;
                      }else{
                        tptr=copy_buff(argbuff);
	                order(tptr, tmp, data);
                        xfree(tptr);
                      }
		      break;
            case 84:  if ((rc<0)||(is_empty(argbuff)!=0)) {
                        data->exp_set=0;
                      }else{
	                data->exp=atoi(argbuff);
		        data->exp_set=1;
                      }
		      break;
            case 74:  data->log_flag=1;
                      data->base= -1.0;
                      if (rc!=0) {
                        k=0;
                        while ((argbuff[k]==' ')||(iswht(argbuff[k]))) k++;
                        if ((argbuff[k]=='E')||(argbuff[k]=='e')) {
                          data->base=exp((double) 1.0);
                        }else{
                          data->base=atof(argbuff);
                        }
                        if (data->base<=0.0) data->log_flag=0;
                      }
		      break;
            case 36:  tp_fl++;
            case 76:  tp_fl++;
	    case 75:  rc=scsize(argbuff, 'y', &crd_sz, &sp_vu, 1, M_FPCT);
		      if (rc<0) {
		 	 sp_err(BADSIZE, tmp, l);
		      }else{
                         switch(tp_fl) {
                           case 0 : data->gaps.x_c=sp_vu;
                                    break;
                           case 1 : data->gaps.y_c=sp_vu;
                                    break;
                           case 2 : data->ln_def=sp_vu;
                           default: break;
                         }
		      }
                      break;
            case 77:  tp_fl++;
            case 78:  if (tp_fl!=0) {
                        for (i=0;i<data->n_clist;i++) {
                          del_inst_tree(data->clist[i]);
                        }
                      }
                      tptr=copy_buff(argbuff);
                      lj=strlen(tptr);
		      if (is_empty(tptr)!=0) {
		 	 if (tp_fl==0) data->n_vlist=0;
                         else data->n_clist=0;
		      }else{
                         i=k=0;
                         while(i<40) {
                           k_o=k;
                           rc=getc_buff(tptr, &k, inbuff, 1000, ',');
                           if (rc>=0) *(tptr+k-1)='\0';
                           if (tp_fl==0) data->vlist[i]=(double) atof(inbuff);
                           else {
                             data->clist[i]=(struct inst_key *) NULL;
                             if (is_empty(inbuff)==0)
                               init_text(&txt);
                               speak((tptr+k_o), &txt, 0, 1, (tmp+k_o),
                                  &(data->clist[i]), 1, 0);
                           }
                           i++;
                           if ((k>=lj)||(rc<0)) break;
                         }
		 	 if (tp_fl==0) data->n_vlist=i;
                         else data->n_clist=i;
		      }
                      xfree(tptr);
		      break;
            case 30:  tp_fl++;
	    case 37:  k=0;
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (rc<0) k= -1;
 		      if (is_empty(inbuff)==0) {
			rc=scsize(inbuff, 'x', &crd_sz, &sp_vu, tp_fl, M_FPCT);
		        if (rc<0) {
			  sp_err(BADSIZE, tmp, l);
			}else{
                          if (tp_fl==0) {
                            data->origin.x_c=sp_vu;
                          }else{
                            data->offset.x_c=sp_vu;
                          }
			}
		      }
                      if (k>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
		        if (is_empty(inbuff)==0) {
			   rc=scsize(inbuff, 'y', &crd_sz, &sp_vu, 
                                tp_fl, M_FPCT);
			   if (rc<0) {
			      sp_err(BADSIZE, tmp, l);
			   }else{
                              if (tp_fl==0) {
                                data->origin.y_c=sp_vu;
                              }else{
                                data->offset.y_c=sp_vu;
                              }
			   }
		        }
                      }
		      break;
	    case 44 : tptr=copy_buff(argbuff);
                      line_fill(tptr, &(data->style), &(lpoly), tmp,
                         0, 1, 0);
                      xfree(tptr);
		      break;
            case 132: data->extend=1;
                      if (rc!=0) {
                        if (l_comp(argbuff, "off")!=0) data->extend=0;
                      }
		      break;
            case 81 : ch=argbuff[0];
                      dsize(data, 0.0, 10.0, 4);
                      size_axis(data, ch);
#ifdef EBUG
                      if (debug_level>1) {
                        print_axis(data);
                      }
#endif
                      draw_axis(data, ch, 0.0, 0, 0);
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
   print_axis(data);
  }
#endif
}

#ifdef EBUG
/*  print_axis - outputs axis information
	       - mainly for debugging purposes */

print_axis(data)
struct axis_def *data;

{
   (void) fprintf(deb_log,"\nAXIS definition:\n");
   (void) fprintf(deb_log,"Major-length %g inout %i number %i none %i\n",
        data->major_t.length_def.val,data->major_t.inout,data->major_t.num,
	data->major_t.none);
   (void) fprintf(deb_log,"Minor-length %g inout %i number %i none %i\n",
        data->minor_t.length_def.val,data->minor_t.inout,data->minor_t.num,
	data->minor_t.none);
   (void) fprintf(deb_log,"Label-set %i none %i lrc %i\n  :%i\n",data->label.set,
	data->label.none,data->label_lrc,data->label.inst);
   (void) fprintf(deb_log,"Value-set %i none %i lrc %i\n  :%i\n",data->value.set,
	data->value.none,data->value_lrc,data->value.inst);
   (void) fprintf(deb_log,"From %g to %g by %g start %g\n",data->begin,data->end,
	data->step, data->start);
   (void) fprintf(deb_log,"Set flag %i\n",data->set);
   (void) fprintf(deb_log,"Extend %i\n",data->extend);
   (void) fprintf(deb_log,"Exponent %i set %i\n",data->exp,data->exp_set);
   (void) fprintf(deb_log,"Label, value - x,y - min,max\n");
   (void) fprintf(deb_log,"%i %i %i %i\n",data->label.limit.xmin,
      data->label.limit.xmax,data->label.limit.ymin,data->label.limit.ymax);
   (void) fprintf(deb_log,"%i %i %i %i\n",data->value.limit.xmin,
      data->value.limit.xmax,data->value.limit.ymin,data->value.limit.ymax);
   (void) fprintf(deb_log,"Origin: %g : %g\n",data->origin.x_c.val,
      data->origin.y_c.val);
   (void) fprintf(deb_log,"Origin units: %i : %i\n",data->origin.x_c.unit,
      data->origin.y_c.unit);
   (void) fprintf(deb_log,"Offsets: %g : %g\n",data->offset.x_c.val,
      data->offset.y_c.val);
   (void) fprintf(deb_log,"Gaps: %g : %g\n",data->gaps.x_c.val,
      data->gaps.y_c.val);
   (void) fprintf(deb_log,"Length: %g, %i\n",data->ln_def.val,data->ln_def.unit);
   (void) fprintf(deb_log,"Lists c->%i v->%i\n",data->n_clist, data->n_vlist);
   (void) fflush(deb_log);
}
#endif

/*    tick - interprets tick command list passed from axis
	   - defines either major or minor ticks depending
	       on tick address passed in data pointer       */

tick(ptr, lpos, t_mark)
char *ptr;
int lpos;
struct tick_m *t_mark;
{
   int rc, l, com[4], tmp; 
   char *tptr, *copy_buff();
   COORD crd_sz;
   struct united sp_vu;
   struct sp_linefill tlp;

   com[1]=0;
   do { 
      rc=scan_cmd(ptr, com, lpos, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, ptr, com, lpos, 1);
      }else{
	 switch (com[0]) {
	    case  0: break;
	    case -2: break;
	    case  4: rc=scsize(argbuff, 'y', &crd_sz, &sp_vu, 1, 0);
		     if (rc<0) {
		       sp_err(BADSIZE, tmp, l);
		     }else{
                       t_mark->length_def=sp_vu;
		     }
		     break;
	    case 11: t_mark->inout=TK_IN;
		     break;
	    case 12: t_mark->inout=TK_OUT;
		     break;
            case 58: t_mark->inout=TK_CENT;
                     break;
	    case 13: t_mark->num=atoi(argbuff);
		     break;
	    case 14: t_mark->none=1;
		     break;
            case 44: tptr=copy_buff(argbuff);
                     line_fill(tptr, &(t_mark->style), &(tlp), tmp,
                         0, 1, 0);
                     break;
	    case 67: t_mark->noprint=1;
		     break;
	    default: com[1]=com[3];
		     what (ILLCOM, ptr, com, lpos, 1);
		     break;
	 }
      }
   } while(com[0]!=0);
}

/* order - defines axis value definition limits
	 N,- unusual as arguments need not follow standard definitions
	 - values can simply follow the indicator (allows statements
	    like [ from 0 to 4 by 2 ]                            */

order(ptr, lpos, data)
char *ptr;
int lpos;
struct axis_def *data;
{
   int com[4], order_flag, rc, l, tmp;
   double val;

   order_flag=1;
   com[1]=0;
   do { 
     rc=scan_cmd(ptr, com, lpos, &l, &tmp, argbuff, 1);
     if (rc<0) {
        yank(ptr, com, argbuff);
        val=atof(argbuff);
        if (order_flag==1) {
           data->begin=val;
           data->set=data->set|BEG_MASK;
        }else if (order_flag==2) {
           data->end=val;
           data->set=data->set|END_MASK;
        }else if (order_flag==3) {
           if (val!=0.0) {
             data->step=val;
             data->set=data->set|STEP_MASK;
           }
        }else if (order_flag==4) {
           data->start=val;
           data->set=data->set|START_MASK;
        }
     }else{
        order_flag=0;
        switch (com[0]) {
	   case  0 : break;
	   case -2 : break;
           case 133: order_flag++;
           case 20 : order_flag++;
           case 19 : order_flag++;
           case 18 : order_flag++;
		     if (rc!=0) {
			val=atof(argbuff);
                        if (order_flag==1) {
                           data->begin=val;
                           data->set=data->set|BEG_MASK;
                        }else if (order_flag==2) {
                           data->end=val;
                           data->set=data->set|END_MASK;
                        }else if (order_flag==3) {
                           if (val!=0.0) {
                             data->step=val;
                             data->set=data->set|STEP_MASK;
                           }
                        }else if (order_flag==4) {
                           data->start=val;
                           data->set=data->set|START_MASK;
                        }
		     }
		     break;
	   default : com[1]=com[3];
		     what (ILLCOM, ptr, com, lpos, 1);
		     break;
        }
     }
   } while(com[0]!=0);
}

/* dsize - sets up default sizing for axes
	 - needs axis structure and data limits
	 - also passed optimum orders   */

static double trials[6]={5.0, 2.0, 1.0, 0.5, 0.2, 0.1};
					    
dsize(axis, vmin, vmax, nopt)
struct axis_def *axis;
double vmin, vmax;
int nopt;

{
   int powr, powr2, i, j, k, tmp, dir, endi, begi, tp;
   double dda, tp_e, t_base;
   char *tptr, *copy_buff();
   struct inst_key *c_inst, *last_inst();
   struct sp_text txt;

   if (axis->exp_set==0) {
     if ((axis->set==0)&&(axis->n_vlist==0)&&(axis->n_clist==0)) {
        if (vmin!=0.0) {
	   powr=fint(log10(fabs(vmin)),-1);
        }else{
	   powr= -37;
        }
        if (vmax!=0.0) {
	   powr2=fint(log10(fabs(vmax)),-1);
        }else{
	   powr2= -37;
        }
        if (powr2>powr) powr=powr2;
        tp=powr;
        if (tp<0) tp= -tp;
        if (tp>3) {
	   axis->exp=powr;
           c_inst=last_inst(axis->label.inst);
	   (void) sprintf(argbuff,"\' (10\' sup(\'%i\') \')\'",axis->exp);
           tptr=copy_buff(argbuff);
           init_text(&txt);
           speak(tptr, &txt, 0, 1, 0, &c_inst, 1, 0);
           xfree(tptr);
        }else{
           axis->exp=0;
        }
     }else{
        axis->exp=0;
     }
   }
   if (axis->exp!=0) {
     vmin=vmin/exp((double) axis->exp*log((double) 10.0));
     vmax=vmax/exp((double) axis->exp*log((double) 10.0));
   }

   if (axis->log_flag==0) {
   if (axis->set!=(BEG_MASK|END_MASK|STEP_MASK|START_MASK)) {
      if ((axis->set&BEG_MASK)==0) axis->begin=vmin;
      if ((axis->set&END_MASK)==0) axis->end=vmax;
      if ((axis->set&STEP_MASK)==0) {
	if (axis->begin!=0.0) {
          powr=fint(log10(fabs((double) axis->begin)),-1);
	}else{
   	  powr= -37;
	}
	if (axis->end!=0.0) {
          powr2=fint(log10(fabs((double) axis->end)),-1);
	}else{
	  powr2= -37;
	}
        if (powr2>powr) powr=powr2;
        j=0;
        k=400;
        tp_e=exp((double) powr*log((double) 10.0));
        for (i=0;i<6;i++) {
   tmp=abs(abs(fint((double) ((axis->end-axis->begin)/(trials[i]*tp_e)),1))
        -nopt);
	   if (tmp<k) {
	     k=tmp; j=i;
	   }
        }
	axis->step=trials[j]*tp_e;
	if (axis->end<axis->begin) axis->step= -1.0*axis->step;
      }
      dda=0.0001;
      dir=1;
      if (axis->step<0) dir= -1;
      switch(axis->set&(BEG_MASK|END_MASK)) {
        case 0:
           axis->begin=fint((double) (axis->begin/axis->step+dir*dda),
	        (-1*dir))*axis->step;
           axis->end=fint((double) (axis->end/axis->step-dir*dda),
		dir)*axis->step;
	   break;
        case BEG_MASK :
           axis->end=fint((double) ((axis->end-axis->begin)/axis->step-dda),1);
	   axis->end=axis->end*axis->step+axis->begin;
	   break;
        case END_MASK :
           axis->begin=fint((double) ((axis->begin-axis->end)/axis->step+dda),-1);
	   axis->begin=axis->begin*axis->step+axis->end;
	   break;
        default : break;
      }
      if (axis->rad_fl>0) {
        if ((axis->set&BEG_MASK)==0) axis->begin=0;
        if ((axis->set&END_MASK)==0) {
          switch(axis->rad_fl) {
            case 1: axis->end=360.0;
                    break;
            case 2: axis->end=2.0*m_pi;
                    break;
            case 3: axis->end=400.0;
                    break;
            case 4: axis->end=180.0;
                    break;
            case 5: axis->end=m_pi;
                    break;
            case 6: axis->end=200.0;
                    break;
          }
        }
        if ((axis->set&STEP_MASK)==0) {
          switch(axis->rad_fl) {
            case 4:
            case 1: axis->step=45.0;
                    break;
            case 5:
            case 2: axis->step=0.5;
                    break;
            default:axis->step=50.0;
                    break;
          }
        }
        axis->set=BEG_MASK|END_MASK|STEP_MASK;
      }
      if ((axis->set&START_MASK)==0) axis->start=axis->begin;
#ifdef EBUG
  if (debug_level>1) {
      (void) fprintf(deb_log,"DF beg %g end %g step %g exp %i\n",
              axis->begin,axis->end, axis->step, axis->exp);
      (void) fflush(deb_log);
  }
#endif
   }
   }else{
      if ((axis->set&BEG_MASK)==0) axis->begin=vmin;
      if ((axis->set&END_MASK)==0) axis->end=vmax;
      dir=1;
      if (axis->end<axis->begin) dir= -1;
      tmp=0;
      if (axis->end<=0.0) tmp+=1;
      if (axis->begin<=0.0) tmp+=2;
      t_base=axis->base;
      if (axis->base<=0.0) t_base=10.0;

      switch (tmp) {
        case 0: begi=fint((log(axis->begin)/log(t_base)+0.0001*dir),
                     (-1*dir));
                endi=fint((log(axis->end)/log(t_base)-0.0001*dir),
		     dir);
       	        break;
        case 1: begi=fint((log(axis->begin)/log(t_base)+0.0001*dir),
		     (-1*dir));
       	        endi=begi-3;
	        break;
 	case 2: endi=fint((log(axis->end)/log(t_base)-0.0001*dir),
		      dir);
	        begi=endi-3;
	        break;
	case 3: (void) fprintf(stderr,
                   "Error: log axis specified with totally negative data.\n");
	        (void) fprintf(stderr, 
                   "Using 1 to %f**3 as default.\n", t_base);
                begi=0;
                endi=3;
	        break;
        default: break;
      }
      if ((axis->set&BEG_MASK)==0) 
                 axis->begin=exp((double) (begi*log(t_base)));
      if ((axis->set&END_MASK)==0) 
                 axis->end=exp((double) (endi*log(t_base)));
   }
}

char *pis[]={"'0'", "'p/4'", "'p/2'", "'3p/4'", "'p'", "'5p/4'", "'3p/2'",
             "'7p/4'", "''"};
char *pi_def="f=greeks";

/* dft_axis - includes default axis values where none have been set
            - sets according to argument values */

dft_axis(axis, lbl, val, name, vlrc)
struct axis_def *axis;
char *lbl, *val, *name;
int vlrc;
{
   int i;
   char *tptr, *copy_buff();
   struct sp_text txt;

   if (axis->label.set==0) {
      (void) sprintf(argbuff, "%s \'%s\'", lbl, name);
      tptr=copy_buff(argbuff);
      init_text(&txt);
      speak(tptr, &txt, 0, 1, 0, &(axis->label.inst), 1, 0);
      xfree(tptr);
      axis->label.none=txt.none;
   }

   if (axis->value.set==0) {
      if ((axis->set==0)&&(axis->exp_set==0)&&(axis->n_vlist==0)&&
         (axis->n_clist==0)&&(axis->rad_fl>0)&&(axis->log_flag==0)) {
        for (i=0;i<9;i++) {
          axis->clist[i]=(struct inst_key *) NULL;
          if ((axis->rad_fl==2)||(i==8)) {
            init_text(&txt);
            speak(pis[i], &txt, 0, 1, 0, &(axis->clist[i]), 1, 0);
            if (axis->rad_fl==2) axis->vlist[i]=i*m_pi/4.0;
          }
        }
        axis->n_clist=9;
        if (axis->rad_fl==2) {
          axis->n_vlist=9;
          (void) sprintf(argbuff,"%s %s\n", val, pi_def);
          tptr=copy_buff(argbuff);
        }else{
          tptr=copy_buff(val);
        }
        init_text(&txt);
        txt.hfact=RED_FACT*txt.hfact;
        speak(tptr, &txt, 0, 1, 0, &(axis->value.inst), 1, 2);
        xfree(tptr);
      }else{
        tptr=copy_buff(val);
        init_text(&txt);
        txt.hfact=RED_FACT*txt.hfact;
        speak(tptr, &txt, 0, 1, 0, &(axis->value.inst), 1, 2);
        xfree(tptr);
      }
   }

   if (axis->label_lrc<0) axis->label_lrc=T_CENTER;
   if (axis->value_lrc<0) axis->value_lrc=vlrc;
#ifdef EBUG
  if (debug_level>1) {
    print_inst(argbuff, axis->label.inst, -1, 1);
    (void) fprintf(deb_log,"LABEL: %s\n",argbuff);
    print_inst(argbuff, axis->value.inst, -1, 1);
    (void) fprintf(deb_log,"VALUE: %s\n",argbuff);
    (void) fflush(deb_log);
  }
#endif
}

/*  draw_axis - displays the axis structure for the direction dir
              - def_ang indicates the angle deflection for in direction
              - tick2_fl controls opposing ticks (for vax)
              - sp_tik is space inclusion (for vax2)
              - gr_maj, gr_min, grid major/minor parameters   
              - grids is the grid style definition */

draw_axis(axis, dir, def_ang, tick2_fl, sp_tik)
struct axis_def *axis;
char dir;
double def_ang;
int tick2_fl, sp_tik;
{
   int t_beg, t_end, major_i, minor_i;
   COORD tk_l, tk_l2, tk_r, tk_r2, tk_v, tk_v2;
   COORD tmp_tik, tmp_val, tmp_lab, tpy, tpx, x, y, xp, yp;
   double r, ang, val;
   struct sp_text txt;
   struct sp_linefill ln_lp, mj_lp, mn_lp;

   init_linefill(&ln_lp, -1, 0);
   mj_lp=mn_lp=ln_lp;
   line_fill((char *) NULL, &(axis->style), &ln_lp, 0, 0, 1, 0);
   line_fill((char *) NULL, &(axis->major_t.style), &mj_lp, 0, 0, 1, 0);
   line_fill((char *) NULL, &(axis->minor_t.style), &mn_lp, 0, 0, 1, 0); 

   if ((dir=='x')||(axis->rad_fl!=0)) {
     tmp_val=axis->value.limit.ymax-axis->value.limit.ymin;
     tmp_lab=axis->label.limit.ymax-axis->label.limit.ymin;
     ang= -90.0;
   }else{
     tmp_val=axis->value.limit.xmax-axis->value.limit.xmin;
     tmp_lab=axis->label.limit.xmax-axis->label.limit.xmin;
     ang=180.0;
   }
   tmp_tik=0;
   if ((axis->major_t.inout&TK_OUT)!=0) tmp_tik=axis->major_t.length;
   if ((axis->minor_t.length>tmp_tik)&&((axis->minor_t.inout&TK_OUT)!=0)) {
      tmp_tik=axis->minor_t.length;
   }
   if ((tmp_tik==0)&&(options.fourtk!=0)) tmp_tik=sp_tik;
   if (axis->rad_fl<0) {
      ang=axis->origin.x/1000.0-90.0;
   }

   if (axis->label.none==0) {
     init_text(&txt);
     if ((dir=='x')||(axis->rad_fl!=0)) {
       tk_v=axis->origin.x;
       if (dir=='y') tk_v=axis->origin.y;
       switch(axis->label_lrc) {
         case T_RIGHT: tk_v=tk_v+axis->length;
                       break;
         case T_CENTER:tk_v=tk_v+axis->length/2;
                       break;
         default:      break;
       }
       if (dir=='x') {
         xp=tk_v; yp=axis->origin.y;
       }else{
         xp=axis->origin.x; yp=tk_v;
       }
       if (axis->rad_fl>0) ang=xp/1000.0;
       pol_crd(&xp, &yp, 0);
       r=tmp_tik+axis->gaps.x+tmp_val+axis->gaps.y+tmp_lab+
         axis->label.limit.ymin;
       pdisp_crd(&x, &y, xp, yp, r, (ang+def_ang));
       switch(axis->label_lrc) {
         case T_LEFT :  r=axis->label.limit.xmin;
                        break;
         case T_RIGHT : r=axis->label.limit.xmax;
  	                break;
         default : r=(axis->label.limit.xmax+axis->label.limit.xmin)/2.0;
                   break;
       }
       pdisp_crd(&xp, &yp, x, y, r, (ang+def_ang-90.0));
       txt.x0=xp; txt.y0=yp;
       if (axis->rad_fl!=0) {
         txt.angle=ang+def_ang+90.0;
         txt.r_angle=txt.angle*deg_to_rad;
       }
     }else{
       if (def_ang==0.0) {
         txt.x0=tmp_tik+tmp_val+tmp_lab+axis->gaps.x+axis->gaps.y;
         txt.x0=axis->origin.x-txt.x0-axis->label.limit.xmin;
       }else{
         txt.x0=tmp_tik+tmp_val+axis->gaps.x+axis->gaps.y;
         txt.x0=axis->origin.x+txt.x0-axis->label.limit.xmin;
       }
       switch(axis->label_lrc) {
         case T_LEFT : 
           txt.y0=axis->origin.y-axis->label.limit.ymin;
           break;
         case T_RIGHT : 
           txt.y0=axis->origin.y+axis->length-axis->label.limit.ymax;
  	   break;
         default : 
           txt.y0=axis->origin.y+axis->length/2;
           txt.y0=txt.y0-(axis->label.limit.ymax+axis->label.limit.ymin)/2.0;
           break;
       }
     }
     speak((char *) NULL, &txt, 1, 1, 0, &(axis->label.inst), 0, 0);
   }
  

   tk_l=axis->major_t.length;
   if (axis->major_t.inout==TK_IN) tk_l= -tk_l;
   tk_r=0;
   if (axis->major_t.inout==TK_CENT) tk_r= -axis->major_t.length;
   tk_l2=axis->minor_t.length;
   if (axis->minor_t.inout==TK_IN) tk_l2= -tk_l2;
   tk_r2=0;
   if (axis->minor_t.inout==TK_CENT) tk_r2= -axis->minor_t.length;

   t_beg=0;
   t_end=axis->nvalues;
   if ((axis->extend!=0)&&(axis->n_vlist==0)) {
     t_beg--; t_end++;
   }
   for (major_i=t_beg;major_i<t_end;major_i++) {
     if ((major_i!=-1)&&(major_i<axis->nvalues)) {
       get_maj_min(axis, dir, major_i, 0, &val, &tk_v);
       if ((axis->major_t.none==0)&&(axis->major_t.num!=0)&&
              (axis->major_t.noprint==0)) {
         if (dir=='y') {
           xp=axis->origin.x; yp=tk_v;
         }else{
           xp=tk_v; yp=axis->origin.y;
         }
         if (axis->rad_fl>0) ang=xp/1000.0;
         make_tick(xp, yp, tk_l, tk_r, (ang+def_ang), &(mj_lp));
         if ((tick2_fl==0)&(options.fourtk!=0)) {
           if (dir=='y') xp=urc_x;
           else yp=urc_y;
           if (options.fourtk>0) 
             make_tick(xp, yp, tk_l, tk_r, (ang+def_ang+180.0), &(mj_lp));
           else
             make_tick(xp, yp, tk_l, tk_r, (ang+def_ang), &(mj_lp));
         }
       }
     }

     if (major_i<(t_end-1)) {
       for (minor_i=1;minor_i<=axis->minor_t.num;minor_i++) {
         get_maj_min(axis, dir, major_i, minor_i, &val, &tk_v2);
         if (tk_v2!=MAX_CRD) {
           if ((axis->minor_t.none==0)&&(axis->minor_t.num!=0)&&
                 (axis->minor_t.noprint==0)) {
             if (dir=='y') {
               xp=axis->origin.x; yp=tk_v2;
             }else{
               xp=tk_v2; yp=axis->origin.y;
             }
             if (axis->rad_fl>0) ang=xp/1000.0;
             make_tick(xp, yp, tk_l2, tk_r2, (ang+def_ang), &(mn_lp));
             if ((tick2_fl==0)&(options.fourtk!=0)) {
               if (dir=='y') xp=urc_x;
               else yp=urc_y;
               if (options.fourtk>0) 
                make_tick(xp, yp, tk_l2, tk_r2, (ang+def_ang+180.0), &(mn_lp));
               else
                make_tick(xp, yp, tk_l2, tk_r2, (ang+def_ang), &(mn_lp));
             }
	   }
         }
       }
     }

     if ((axis->value.none==0)&&(major_i!=-1)&&(major_i<axis->nvalues)) {
       init_text(&txt);
       txt.hfact=RED_FACT*txt.hfact;
       (void) value_put(axis, &txt, 0, major_i, axis->ndec_places);
       if (dir=='x') {
         xp=tk_v; yp=axis->origin.y;
       }else{
         xp=axis->origin.x; yp=tk_v;
       }
       if (axis->rad_fl>0) ang=xp/1000.0;
       pol_crd(&xp, &yp, 0);
       if ((dir=='x')||(axis->rad_fl!=0)) {
         r=tmp_tik+axis->gaps.x+tmp_val/2+(txt.limit.ymax+txt.limit.ymin)/2;
         pdisp_crd(&x, &y, xp, yp, r, (ang+def_ang)); 
         switch(axis->value_lrc) {
           case T_LEFT  : r=txt.limit.xmin;
                          break;
           case T_RIGHT : r=txt.limit.xmax;
                          break;
           default      : r=(txt.limit.xmin+txt.limit.xmax)/2;
                          break;
         }
         pdisp_crd(&tpx, &tpy, x, y, r, (ang+def_ang-90.0));
       }else{
         tpy=tk_v-(txt.limit.ymax+txt.limit.ymin)/2;
         switch(axis->value_lrc) {
           case T_LEFT  : if (def_ang==0.0) {
                            tpx=axis->origin.x-tmp_tik-tmp_val;
      		            tpx=tpx-axis->gaps.x-txt.limit.xmin;
                          }else{
                            tpx=axis->origin.x+tmp_tik+axis->gaps.x;
                            tpx=tpx-txt.limit.xmin;
                          }
         	          break;
           case T_RIGHT : if (def_ang==0.0) {
                            tpx=axis->origin.x-tmp_tik-axis->gaps.x;
			    tpx=tpx-txt.limit.xmax;
                          }else{
                            tpx=axis->origin.x+tmp_tik+axis->gaps.x;
                            tpx=tpx+tmp_val-txt.limit.xmax;
                          }
		          break;
           default      : tpx=tmp_tik+axis->gaps.x+tmp_val/2;
                          tpx=axis->origin.x-cos(def_ang)*tpx;
	      	          tpx=tpx-(txt.limit.xmax+txt.limit.xmin)/2.0;
                          break;
         }
       }
       init_text(&txt);
       txt.hfact=RED_FACT*txt.hfact;
       txt.x0=tpx;
       txt.y0=tpy;
       if (axis->rad_fl!=0) {
         txt.angle=ang+def_ang+90.0;
         txt.r_angle=txt.angle*deg_to_rad;
       }
       (void) value_put(axis,&txt,1,major_i,axis->ndec_places);
     }
   }

   if (axis->style.none==0) {
     if (dir=='y') {
       tp_move(axis->origin.x, axis->origin.y, &(ln_lp));
       tp_draw(axis->origin.x, (COORD) (axis->length+axis->origin.y), 
               &(ln_lp));
     }else{
       tp_move(axis->origin.x, axis->origin.y, &(ln_lp));
       tp_draw((COORD) (axis->length+axis->origin.x), axis->origin.y, 
               &(ln_lp));
     }
   }
}

/* make_tick - make a tick mark at the point (xp,yp) */

make_tick(xp, yp, tk_l, tk_r, ang, style)
COORD xp, yp, tk_l, tk_r;
double ang;
struct sp_linefill *style;
{
   COORD x, y;

   pol_crd(&xp, &yp, 0);
   pdisp_crd(&x, &y, xp, yp, (double) tk_r, ang);
   tn_move(x, y, style);
   pdisp_crd(&x, &y, xp, yp, (double) tk_l, ang);
   tn_draw(x, y, style);
}

/* pdisp_crd - add a polar displacement to the given coordinate */

pdisp_crd(x, y, xp, yp, r, ang)
COORD *x, *y, xp, yp;
double r, ang;
{
   ang=ang*deg_to_rad;
   *x=xp+r*cos((double) ang);
   *y=yp+r*sin((double) ang);
}

/* ax_grid - draw the plot surface grid according to axis layout */

ax_grid(axis, dir, grid_maj, grid_min, gr_maj_fl, gr_min_fl)
struct axis_def *axis;
char dir;
int gr_maj_fl, gr_min_fl;
struct lnfill_def *grid_maj, *grid_min;
{
   int t_beg, t_end, major_i, minor_i;
   struct sp_linefill gmj_lp, gmn_lp;
   double val;
   COORD tk_v;

   if ((gr_maj_fl==0)&&(gr_min_fl==0)) return;

   init_linefill(&gmj_lp, -1, 0);
   init_linefill(&gmn_lp, -1, 0);
   if (gr_maj_fl!=0) line_fill((char *) NULL, grid_maj, &gmj_lp, 0, 0, 1, 0);
   if (gr_min_fl!=0) line_fill((char *) NULL, grid_min, &gmn_lp, 0, 0, 1, 0);

   t_beg=0;
   t_end=axis->nvalues;
   if ((axis->extend!=0)&&(axis->n_vlist==0)) {
     t_beg--; t_end++;
   }

   for (major_i=t_beg;major_i<t_end;major_i++) {
     if ((major_i!=-1)&&(major_i<axis->nvalues)) {
       get_maj_min(axis, dir, major_i, 0, &val, &tk_v);
       if ((gr_maj_fl!=0)&&(grid_maj->none==0)) {
         if (dir=='y') {
           tp_move(llc_x, tk_v, &(gmj_lp));
           tp_draw(urc_x, tk_v, &(gmj_lp));
         }else{
           tp_move(tk_v, llc_y, &(gmj_lp));
           tp_draw(tk_v, urc_y, &(gmj_lp));
         }
       }
     }
     if (major_i<(t_end-1)) {
       for (minor_i=1;minor_i<=axis->minor_t.num;minor_i++) {
         get_maj_min(axis, dir, major_i, minor_i, &val, &tk_v);
         if (tk_v!=MAX_CRD) {
           if ((gr_min_fl!=0)&&(grid_min->none==0)) {
             if (dir=='y') {
               tp_move(llc_x, tk_v, &(gmn_lp));
               tp_draw(urc_x, tk_v, &(gmn_lp));
             }else{
               tp_move(tk_v, llc_y, &(gmn_lp));
               tp_draw(tk_v, urc_y, &(gmn_lp));
             }
           }
         }
       }
     }
   }
}

/* get_maj_min - returns the coordinate associated with the major/minor pair
               - less efficient speed-wise, but neater code */

get_maj_min(axis, dir, major_i, minor_i, val, crd)
struct axis_def *axis;
char dir;
int major_i, minor_i;
double *val;
COORD *crd;
{
   double val_l, val_r;

   get_value(axis, major_i, &val_l, 0);
   get_value(axis, (major_i+1), &val_r, 0);
   *val=(val_r-val_l)/(axis->minor_t.num+1.0)*minor_i+val_l;
   pos_inter(axis, *val, dir, crd, (minor_i==0)?0:1);
}

/* build_maj_table - constructs a series table of the major/minor values
                   - returns number of values involved */

static char *t_msg="Unable to allocate axis level storage.";

int build_maj_table(axis, dir, vals, crds)
struct axis_def *axis;
char dir;
double **vals;
COORD **crds;
{
   int i, size, major_i, minor_i, t_beg, t_end, num;
   COORD crd_sz;
   double val;

   size=(axis->nvalues+2)*(axis->minor_t.num+1)+10;
   *vals=(double *) xalloc((unsigned int) (size*sizeof(double)), t_msg);
   *crds=(COORD *) xalloc((unsigned int) (size*sizeof(COORD)), t_msg);

   t_beg=0;
   t_end=axis->nvalues;
   if ((axis->extend!=0)&&(axis->n_vlist==0)) {
     t_beg--; t_end++;
   }

   num=0;
   for (major_i=t_beg;major_i<t_end;major_i++) {
     for (minor_i=0;minor_i<=axis->minor_t.num;minor_i++) {
       if (minor_i==0) {
         if ((major_i==-1)||(major_i==axis->nvalues)) continue;
         if ((axis->major_t.none!=0)||(axis->major_t.num==0)) continue;
       }else{
         if (major_i>=(t_end-1)) continue;
         if (axis->minor_t.none!=0) continue;
       }

       get_maj_min(axis, dir, major_i, minor_i, &val, &crd_sz);
       *(*vals+num)=val;
       *(*crds+num++)=crd_sz;
     }
   }

   if ((num>0)&&(*(*vals)>*(*vals+num-1))) {
     for (i=0;i<(num/2);i++) {
       t_end=num-i-1;
       val= *(*vals+i); *(*vals+i)= *(*vals+t_end); *(*vals+t_end)=val;
       crd_sz= *(*crds+i); *(*crds+i)= *(*crds+t_end); *(*crds+t_end)=crd_sz;
     }
   }
   return(num);
}
