/******************************************************************
                          sPLOTch!

  Special - special splotch commands which don't necessarily
      fall into the other categories, such as histograms,
      diagram input, error bar calculators, macros, etc.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include "spastic.h"

extern struct opt_def options;
extern char argbuff[], inbuff[], tmpbuff[];

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

/*  define_macros - defines the set of macros listed with the
                       corresponding values
                  - will do commands and standard input as well */

define_macros()
{
   int rc, com[4], tmp, l, macro_num, j, k;
   char ch, next_non_white();
   FILE *fp;

   do {
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if ((rc<0)||((com[0]!=0)&&(com[0]!=-2))) {
        com[0]= -2;
        if (rc>=0) com[1]=com[3];
        rc=getarg((char *) NULL, com[1], 0, argbuff, 1, &l);
        if (rc<0) {
          what(BADDEF, (char *) NULL, com, 0, 1);
        }else{
          macro_num=add_macro_name(argbuff);
          com[1]=com[1]+rc;
          com[2]=com[2]+rc;
          rc=getarg((char *) NULL, com[1], 0, argbuff, 0, &l);
          if (rc<0) {
            add_macro(macro_num, "");
          }else{
            k=0;
            ch=next_non_white(argbuff, &k);
            if ((ch=='$')&&(argbuff[k+1]=='<')) {
              k=0;
              while(((ch=getchar())!='\n')&&(feof(stdin)==0)&&(k!=999))
                 argbuff[k++]=ch;
              argbuff[k]='\0';
            }else if (ch=='`') {
              for (j=strlen(argbuff);j>k;j--) if (argbuff[j]=='`') break;
              argbuff[j]='\0';
              fp=popen((argbuff+k+1), "r");
              if (fp==(FILE *) NULL) {
                argbuff[0]='\0';
              }else{
                k=0;
                while(((ch=fgetc(fp))!=EOF)&&(feof(fp)==0)&&(k!=999))
                  argbuff[k++]=ch;
                argbuff[k]='\0';
                (void) pclose(fp);
              }
            }
            add_macro(macro_num, argbuff);
            com[1]=com[1]+rc;
            com[2]=com[2]+rc;
          }
        }
      }
      line_buff_flush(com[1], 1);
      parse_macros(0);
   } while(com[0]!=0);
}

/*   error_bar - puts together the error bar constructs */

error_bar(datas)
struct sp_data *datas;
{
   int i, rc, com[4], tmp, l, out_varn[2], com2[4], nout, err_type;
   int in_varn[10], n_in, nrows, iv;
   double xl_val, xm_val, xr_val, yl_val, ym_val, yr_val, del;

   do {
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
         what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
         switch (com[0]) {
            case 0  : break;
            case -2 : break;
            case 87 : err_type=atoi(argbuff);
                      break;
            case 81 : nout=0;
                      com2[1]=0;
                      yank(argbuff, com2, inbuff);
                      while ((is_empty(inbuff)==0)&&(nout<2)) {
                        if (strlen(inbuff)>15) {
                          (void) sprintf(tmpbuff,
        "*** Warning: Variable %s truncated to 15 characters. ***\n", inbuff);
                          add_note(tmpbuff);
                          inbuff[15]='\0';
                        }
                        rc=find_var(datas, inbuff);
                        if (rc>=0) {
                          empty_var(datas, rc);
                          (void) sprintf(tmpbuff,
        "*** Warning: Variable %s already exists (emptied). ***\n", inbuff);
                          add_note(tmpbuff);
                          out_varn[nout]=rc;
                        }else out_varn[nout]=add_var(datas, inbuff, PRC_SING);
                        if (out_varn[nout]<0) sev_err(OVERDATA);
                        change_var(datas, out_varn[nout], TYPE_FLT);
                        nout++;
                        yank(argbuff, com2, inbuff);
                      }
                      if (nout!=2) {
                        sp_err(NOOUT, tmp, l);
                        for (i=0;i<nout;i++) {
                          del_var(datas, i);
                        }
                      }
                      break;
            case 86 : if (nout!=2) {
                        sp_err(UNDOUT, -com[3], com[2]);
                        break;
                      }
                      n_in=0;
                      com2[1]=0;
                      nrows= -1;
                      yank(argbuff, com2, inbuff);
                      while ((is_empty(inbuff)==0)&&(n_in<6)) {
                        if (strlen(inbuff)>15) inbuff[15]='\0';
                        rc=find_var(datas, inbuff);
                        if (rc==-1) {
                          sp_err(UNKVAR, (tmp+com2[3]), (int) strlen(inbuff));
                        }else{
                          in_varn[n_in++]=rc;
                          rc=get_rows(datas, rc);
                          if (nrows<0) nrows=rc;
                          else if (rc<nrows) nrows=rc;
                        }
                        yank(argbuff, com2, inbuff);
                      }
                      if ((n_in!=3)&&(n_in!=4)&&(n_in!=6)) {
                        sp_err(NOBAR, tmp, l);
                        break;
                      }
                      for (i=0;i<nrows;i++) {
                        switch(n_in) {
                          case 3:  
                            get_num(datas, in_varn[0], i, &iv, &xm_val);
                            xl_val=xr_val=xm_val;
                            if (err_type==0) {
                              get_num(datas, in_varn[1], i, &iv, &yr_val);
                              get_num(datas, in_varn[2], i, &iv, &yl_val);
                              ym_val=(yl_val+yr_val)/2.0;
                            }else{
                              get_num(datas, in_varn[1], i, &iv, &ym_val);
                              get_num(datas, in_varn[2], i, &iv, &del);
                              yl_val=ym_val-del;
                              yr_val=ym_val+del;
                            }
                            break;
                          case 4:
                            if (err_type==0) {
                              get_num(datas, in_varn[0], i, &iv, &xm_val);
                              xl_val=xr_val=xm_val;
                              get_num(datas, in_varn[1], i, &iv, &ym_val);
                              get_num(datas, in_varn[2], i, &iv, &yr_val);
                              get_num(datas, in_varn[3], i, &iv, &yl_val);
                            }else if (err_type==1) {
                              get_num(datas, in_varn[0], i, &iv, &xm_val);
                              xl_val=xr_val=xm_val;
                              get_num(datas, in_varn[1], i, &iv, &ym_val);
                              get_num(datas, in_varn[2], i, &iv, &del);
                              yr_val=ym_val+del;
                              get_num(datas, in_varn[3], i, &iv, &del);
                              yl_val=ym_val-del;
                            }else if (err_type==2) {
                              get_num(datas, in_varn[0], i, &iv, &xr_val);
                              get_num(datas, in_varn[1], i, &iv, &xl_val);
                              xm_val=(xl_val+xr_val)/2.0;
                              get_num(datas, in_varn[2], i, &iv, &yr_val);
                              get_num(datas, in_varn[3], i, &iv, &yl_val);
                              ym_val=(yl_val+yr_val)/2.0;
                            }else if (err_type==3) {
                              get_num(datas, in_varn[0], i, &iv, &xm_val);
                              get_num(datas, in_varn[1], i, &iv, &del);
                              xl_val=xm_val-del;
                              xr_val=xm_val+del;
                              get_num(datas, in_varn[2], i, &iv, &ym_val);
                              get_num(datas, in_varn[3], i, &iv, &del);
                              yl_val=ym_val-del;
                              yr_val=ym_val+del;
                            }
                            break;
                          case 6:
                            if (err_type==0) {
                              get_num(datas, in_varn[0], i, &iv, &xm_val);
                              get_num(datas, in_varn[1], i, &iv, &xr_val);
                              get_num(datas, in_varn[2], i, &iv, &xl_val);
                              get_num(datas, in_varn[3], i, &iv, &ym_val);
                              get_num(datas, in_varn[4], i, &iv, &yr_val);
                              get_num(datas, in_varn[5], i, &iv, &yl_val);
                            }else{
                              get_num(datas, in_varn[0], i, &iv, &xm_val);
                              get_num(datas, in_varn[1], i, &iv, &del);
                              xl_val=xm_val+del;
                              get_num(datas, in_varn[2], i, &iv, &del);
                              xl_val=xm_val-del;
                              get_num(datas, in_varn[3], i, &iv, &ym_val);
                              get_num(datas, in_varn[4], i, &iv, &del);
                              yl_val=ym_val+del;
                              get_num(datas, in_varn[5], i, &iv, &del);
                              yl_val=ym_val-del;
                            }
                            break;
                          default: break;
                        }
                        add_num(datas, out_varn[0], iv, xm_val);
                        add_num(datas, out_varn[0], iv, xl_val);
                        add_num(datas, out_varn[0], iv, xr_val);
                        add_num(datas, out_varn[1], iv, ym_val);
                        add_num(datas, out_varn[1], iv, yl_val);
                        add_num(datas, out_varn[1], iv, yr_val);
                      }
                      break;
            default : com[1]=com[3];
                      what (ILLCOM, (char *) NULL ,com, 0, 1);
                      break;
        }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);
}

/*   histo_gram - does the histogram bin counting */

histo_gram(datas)
struct sp_data *datas;
{
   int i, j, rc, com[4], tmp, l, out_varn[2], com2[4], nout, nrange, k;
   int in_varn[100], n_in, *count, top, bot, iv;
   double *range, vmin, vmax, f_val;

   range=(double *) NULL;
   nrange= -1;

   do {
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
         what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
         switch (com[0]) {
            case 0  : break;
            case -2 : break;
            case 81 : nout=0;
                      com2[1]=0;
                      yank(argbuff, com2, inbuff);
                      while ((is_empty(inbuff)==0)&&(nout<2)) {
                        if (strlen(inbuff)>15) {
                          (void) sprintf(tmpbuff,
        "*** Warning: Variable %s truncated to 15 characters. ***\n", inbuff);
                          add_note(tmpbuff);
                          inbuff[15]='\0';
                        }
                        rc=find_var(datas, inbuff);
                        if (rc>=0) {
                          empty_var(datas, rc);
                          (void) sprintf(tmpbuff,
        "*** Warning: Variable %s already exists (emptied). ***\n", inbuff);
                          add_note(tmpbuff);
                          out_varn[nout]=rc;
                        }else out_varn[nout]=add_var(datas, inbuff, PRC_SING);
                        if (out_varn[nout]<0) sev_err(OVERDATA);
                        if (nout==0) change_var(datas,out_varn[nout],TYPE_FLT);
                        nout++;
                        yank(argbuff, com2, inbuff);
                      }
                      if (nout!=2) {
                        sp_err(NOOUT, tmp, l);
                        for (i=0;i<nout;i++) {
                          del_var(datas, i);
                        }
                      }
                      break;
            case 80 : if (range!=(double *) NULL) xfree((char *) range);
                      nrange=0;
                      if (is_empty(argbuff)!=0) {
                        range=(double *) NULL;
                        break;
                      }
                      for (i=0;i<strlen(argbuff);i++) {
                        if (argbuff[i]==',') nrange++;
                      }
                      range=(double *) xalloc((unsigned int)
                             ((nrange+10)*sizeof(double)),
                             "Unable to allocate range storage.");
                      nrange=0;
                      k=0;
                      while(1) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (is_empty(inbuff)==0) {
                          *(range+(nrange++))=atof(inbuff);
                        }
                        if (rc<0) break;
                      }
                      if (nrange<2) {
                        sp_err(BADHIST, tmp, l);
                        xfree((char *)  range);
                        nrange=0;
                      }else{
                        bfsort(range, nrange);
                      }
                      break;
            case 86 : if (nout!=2) {
                        sp_err(UNDOUT, -com[3], com[2]);
                        break;
                      }
                      n_in=0;
                      com2[1]=0;
                      yank(argbuff, com2, inbuff);
                      while (is_empty(inbuff)==0) {
                        if (strlen(inbuff)>15) inbuff[15]='\0';
                        rc=find_var(datas, inbuff);
                        if (rc==-1) {
                          sp_err(UNKVAR, (tmp+com2[3]), (int) strlen(inbuff));
                        }else{
                          in_varn[n_in++]=rc;
                          if (n_in==100) {
                            add_note(
     "*** Warning: Only 100 variables allowed in the PROCESS argument. ***\n");
                            break;
                          }
                        }
                        yank(argbuff, com2, inbuff);
                      }
                      if (n_in==0) break;

                      if (range==(double *) NULL) {
                        vmin=5.0e300;
                        vmax= -5.0e300;
                        for (i=0;i<n_in;i++) {
                          for (j=0;j<get_rows(datas, in_varn[i]);j++) {
                            get_num(datas, in_varn[i], j, &iv, &f_val);
                            if (f_val<vmin) vmin=f_val;
                            if (f_val>vmax) vmax=f_val;
                          }
                        }
                        top=fint((vmax+0.5), -1);
                        bot=fint((vmin+0.5), -1);
                        nrange=top-bot+1;
                      }

                      count=(int *) xalloc((unsigned int)
                          ((nrange+10)*sizeof(int)),
                          "Unable to allocate histogram storage bins.\n");
                      for (i=0;i<nrange;i++) *(count+i)=0;

                      for (i=0;i<n_in;i++) {
                        for (j=0;j<get_rows(datas, in_varn[i]);j++) {
                          get_num(datas, in_varn[i], j, &iv, &f_val);
                          if (range==(double *) NULL) {
                            iv=fint((f_val+0.5), -1)-bot;
                            *(count+iv)= *(count+iv)+1;
                          }else{
                            for (k=0;k<(nrange-1);k++) {
                              if ((f_val>*(range+k))&&(f_val<*(range+k+1))) {
                                *(count+k)= *(count+k)+1;
                                break;
                              }
                            }
                          }
                        }
                      }

                      for (k=0;k<(nrange-1);k++) {
                        if (range==(double *) NULL) {
                          f_val=(double) k+bot;
                        }else{
                          f_val=(*(range+k)+*(range+k+1))/2.0;
                        }
                        add_num(datas, out_varn[0], iv, f_val);
                        iv= *(count+k);
                        f_val=(double) iv;
                        add_num(datas, out_varn[1], iv, f_val);
                      }
                      break;
            default : com[1]=com[3];
                      what (ILLCOM, (char *) NULL ,com, 0, 1);
                      break;
        }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);

   if (range!=(double *) NULL) xfree((char *) range);
}
