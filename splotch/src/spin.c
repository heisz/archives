/******************************************************************
                          sPLOTch!

  Spin - manages data tables for plotting data.  Inputs and
     updates memory tables, also handles retrieval parsing.
     And processes data output (mainly for test purposes).
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "spastic.h"

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

struct loc_var { int nvar;
                 int *vars;
                 int n_lines;
                 int n_filter;
                 struct loc_var *next;
               };

#define D_BLOCK 1024  /* data block chunk size */
                      /* make sure this jives with the shifting routines!*/

extern char argbuff[],inbuff[],tmpbuff[];

static char *v_type[]={"What?", "Integer", "Float"};
char *prec_types[]={"single", "double"};
int num_fl=1;

/*  data_p - main data line parser
	   - controls file handles for input
	   - fills data tables while allocating storage space, in a set
	       of smaller bunches for more efficient (?) operation  */

data_p(datas) 
struct sp_data *datas;
{
   int rc, com[4], tmp, l, file_open, file_sp, com2[4], c, i, j;
   int nnums, nvarn[30], n_lines, p_flag, l_flag, var_prec, n_filter;
   char ch, *tptr, next_non_white();
   FILE *fp, *search_path();
   struct loc_var *init_loc, *curr_loc;

   fp=(FILE *) NULL;
   if (datas->set==1) {
      while (datas->nvars>0) del_var(datas,0);
   }

   for (i=0;i<100;i++) datas->vars[i]=(struct var_t *) NULL;
   file_open=file_sp=p_flag=l_flag=0;
   datas->nvars=0;
   datas->set=1;

   init_loc=(struct loc_var *) xalloc((unsigned int) sizeof(struct loc_var),
              "Unable to allocate local variable storage packet.");
   curr_loc=init_loc;
   curr_loc->next=(struct loc_var *) NULL;

   n_filter=var_prec=0;

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
            case 182: n_filter=atoi(argbuff);
                      break;
            case 165: j= -1;
                      for (i=0;i<2;i++) {
                        if (l_comp(prec_types[i], argbuff)!=0) j=i;
                      }
                      if (j==-1) sp_err(BADPREC, tmp, l);
                      else var_prec=j;
                      break;
            case 24 : if ((file_open==1)&&(file_sp==0)) {
                        if (p_flag==0) (void) fclose(fp);
                        else{
                          if ((rc=pclose(fp))!=0) {
                            (void) sprintf(tmpbuff,
                 "*** Warning: Shell returned non-zero status %i. ***\n",rc);
                            add_note(tmpbuff);
                          }
                        }
                      }
                      file_sp=p_flag=l_flag=i=0;
                      ch=next_non_white(argbuff, &i);
                      if (ch=='`') {
                        for (j=strlen(argbuff);j>i;j--) {
                          if (argbuff[j]=='`') break;
                        }
                        argbuff[j]='\0';
                        fp=popen((argbuff+i+1), "r");
                        if (fp==(FILE *) NULL) {
                           sp_err(BADPIPE,tmp,l);
                           file_open=0;
                        }else{
                           file_open=p_flag=num_fl=1;
                        }
                      }else{
		        if (l_comp((argbuff+i), "stdin")!=0) {
			   fp=stdin;
			   file_sp=1;
		        }else if (l_comp((argbuff+i), "local")!=0) {
                           fp=stdin;
                           file_sp=l_flag=1;
		        }else{
		           fp=search_path(argbuff, "r", 0, 1);
			   file_sp=0;
		        }
		        if (fp==(FILE *) NULL) {
			   sp_err(NOFILE, tmp, l);
			   file_open=file_sp=0;
		        }else{
			   file_open=num_fl=1;
		        }
                      }
		      break;
	    case 59 : if (file_open==0) {
			 sp_err(NOINFILE, -com[3], com[2]);
			 break;
		      }
	              tmp=atoi(argbuff);
                      if (l_flag==0) {
		        for(c=0;c<tmp;c++) {
                          data_line(fp, inbuff, 1000);
                          if (end_comp(inbuff)||(feof(fp)!=0)) break;
                        }
                      }else{
                        curr_loc->next=(struct loc_var *) xalloc(
                           (unsigned int) sizeof(struct loc_var),
                           "Unable to allocate next local var packet.");
                        curr_loc=curr_loc->next;
                        curr_loc->nvar= -1;
                        curr_loc->n_lines=tmp;
                        curr_loc->next=(struct loc_var *) NULL;
                      }
		      break;
	    case 25 : if (file_open==0) {
			 sp_err(NOINFILE, -com[3], com[2]);
			 break;
		      }

                      n_lines= -1;
                      for (i=0;i<strlen(argbuff);i++) {
                        if (argbuff[i]==',') {
                          argbuff[i]='\0';
                          n_lines=atoi(argbuff+i+1);
                          break;
                        }
                      }
                      
		      com2[1]=0;
		      nnums=0;
		      yank(argbuff, com2, inbuff);
		      while (is_empty(inbuff)==0) {
                         tptr=inbuff;
                         if (*tptr=='-') tptr++;
                         if (*tptr!='.') {
			   if (strlen(tptr)>15) {
                             (void) sprintf(tmpbuff, 
        "*** Warning: Variable %s truncated to 15 characters. ***\n", tptr);
                             add_note(tmpbuff);
			     *(tptr+15)='\0';
			   }
                           rc=find_var(datas, tptr);
                           if (rc>=0) {
                             empty_var(datas, rc);
                             (void) sprintf(tmpbuff, 
        "*** Warning: Variable %s already exists (emptied). ***\n", tptr);
                             add_note(tmpbuff);
                             del_loc_var(init_loc->next, rc);
                             nvarn[nnums]=rc;
                           }else nvarn[nnums]=add_var(datas, tptr, var_prec);
		           if (nvarn[nnums]<0) sev_err(OVERDATA);
                           if (inbuff[0]=='-') {
                             nvarn[nnums]= -nvarn[nnums]-2;
                           }
                         }else{
                           nvarn[nnums]= -1;
                         }
                         nnums++;
			 yank(argbuff, com2, inbuff);
		      } 
		      if (nnums==0) {
			 sp_err(NOVARS, tmp, l);
			 break;
		      }

                      if (l_flag!=0) {
                        curr_loc->next=(struct loc_var *) xalloc(
                           (unsigned int) sizeof(struct loc_var),
                           "Unable to allocate next local var packet.");
                        curr_loc=curr_loc->next;
                        curr_loc->nvar=nnums;
                        curr_loc->n_lines=n_lines;
                        curr_loc->n_filter=n_filter;
                        curr_loc->vars=(int *) xalloc((unsigned int)
                           (nnums*sizeof(int)), "Unable to get var list.");
                        for (i=0;i<nnums;i++) *(curr_loc->vars+i)=nvarn[i];
                        curr_loc->next=(struct loc_var *) NULL;
                      }else{
                        read_data(fp, datas, nnums, nvarn, n_lines, n_filter);
                      }
		      break;
	    default : com[1]=com[3];
		      what (ILLCOM, (char *) NULL ,com, 0, 1); 
		      break;
	 }
      }
      line_buff_flush(com[1], 1);
   } while(com[0]!=0);

   if ((file_open==1)&&(file_sp==0)) {
     if (p_flag==0) (void) fclose(fp);
     else{
       if ((rc=pclose(fp))!=0) {
         (void) sprintf(tmpbuff,
             "*** Warning: Shell returned non-zero status %i. ***\n",rc);
         add_note(tmpbuff);
       }
     }
   }

   num_fl=1;
   curr_loc=init_loc->next;
   xfree((char *) init_loc);
   while (curr_loc!=(struct loc_var *) NULL) {
     if (curr_loc->nvar>0) {
       read_data((FILE *) NULL, datas, curr_loc->nvar, curr_loc->vars,
         curr_loc->n_lines, curr_loc->n_filter);
       xfree((char *) curr_loc->vars);
     }else{
       for(c=0;c<(curr_loc->n_lines);c++) {
         data_line((FILE *) NULL, inbuff, 1000);
         if (end_comp(inbuff)||(end_of_prog()!=0)) break;
       }
     }
     init_loc=curr_loc;
     curr_loc=init_loc->next;
     xfree((char *) init_loc);
   }

#ifdef EBUG
 if (debug_level>0) {
  (void) fprintf(deb_log,"Block size %i\n", D_BLOCK);
  (void) fprintf(deb_log,"Nvars: %i\n",datas->nvars);    
  for (c=0;c<datas->nvars;c++){
     (void) fprintf(deb_log,"Var %i:%s type %i prec %i nb %i nrows %i\n",c,
	  (datas->vars[c])->name,(datas->vars[c])->type,
          (datas->vars[c])->prec,
          (datas->vars[c])->nblocks,(datas->vars[c])->nrows);
  }
  (void) fflush(deb_log);
 } 
#endif
   (void) sprintf(tmpbuff,"   %i variable(s) defined.\n",datas->nvars);
   add_note(tmpbuff);
   tmp=0;
   for (c=0;c<datas->nvars;c++) {
     (void) sprintf(tmpbuff,
             "   Variable %s (%s, %s precision) contains %i row(s) of data\n",
	     (datas->vars[c])->name, v_type[(datas->vars[c])->type],
             prec_types[(datas->vars[c])->prec-1], (datas->vars[c])->nrows);
     add_note(tmpbuff);
     if ((datas->vars[c])->prec==PRC_SING) {
       tmp=tmp+(datas->vars[c])->nblocks*D_BLOCK*sizeof(union data_t_s);
     }else{
       tmp=tmp+(datas->vars[c])->nblocks*D_BLOCK*sizeof(union data_t_d);
     }
   }
   (void) sprintf(tmpbuff,"   %i byte(s) of storage used.",tmp);
   add_note(tmpbuff);
}

/* del_loc_var - null (-1) all occurences of the given variable in
                 the local variable structure */

del_loc_var(curr_loc, var)
struct loc_var *curr_loc;
int var;
{
   int i;

   while (curr_loc!=(struct loc_var *) NULL) {
     if (curr_loc->nvar>0) {
       for (i=0;i<curr_loc->nvar;i++) {
         if (*(curr_loc->vars+i)==var) *(curr_loc->vars+i)= -1;
       }
     }
     curr_loc=curr_loc->next;
   }
}

/* read_data - read in data entries for variables in nvarn
             - if fp NULL, read from local input 
             - if n_filter non-zero, only read in every nth line */

read_data(fp, datas, nvar, vars, n_lines, n_filter)
FILE *fp;
struct sp_data *datas;
int nvar, n_lines, vars[], n_filter;
{
   int rc, c, com[4], i_val, ntype[30], v, isgn, count;
   double sgn, f_val;

   for (c=0;c<nvar;c++) {
     v=vars[c];
     if (v==-1) continue;
     if (v<-1) v= -v-2;
     if ((ntype[c]=var_type(datas, v))<0) {
       (void) fprintf(stderr,"Fatal error.  Typing nonexistent variable.\n");
       (void) exit(1);
     }
   }

   count=0;
   while (m_eof(fp)==0) {
     data_line(fp, inbuff, 1000);
     if ((end_comp(inbuff))||(is_empty(inbuff)!=0)) break;
     com[1]=0;
     if ((n_filter==0)||(count==0)) {
       for (c=0;c<nvar;c++) {
         yank(inbuff, com, tmpbuff);
         if (is_empty(tmpbuff)!=0) break;
         if (vars[c]==-1) continue;
         sgn=isgn=1;
         if ((v=vars[c])<-1) {
           v= -v-2;
           sgn=isgn= -1;
         }
         if (ntype[c]==TYPE_INT) {
           rc=type_num(tmpbuff);
           if (rc==TYPE_FLT) {
             change_var(datas, v, TYPE_FLT);
             ntype[c]=TYPE_FLT;
           }else{
             i_val=isgn*atoi(tmpbuff);
             f_val=(double) i_val;
           }
         }
         if (ntype[c]==TYPE_FLT) {
           f_val=sgn*atof(tmpbuff);
           i_val=(int) f_val;
         }
         add_num(datas, v, i_val, f_val);
       }
     }
     count++;
     if (count==n_filter) count=0;
     if (n_lines>0) {
       n_lines--;
       if (n_lines==0) break;
     }
   }
}

/* output - outputs data sets for whatever purpose */

output(datas) 
struct sp_data *datas;
{
   int i, j, rc, com[4], com2[4], l, tmp, varn[30], t_fl;
   int file_open, file_sp, p_flag, nrows, nnums, i_val;
   char ch, dir[3], next_non_white();
   FILE *fp, *search_path();
   double f_val;

   fp=(FILE *) NULL;
   file_open=file_sp=p_flag=0;
   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM, (char *) NULL, com, 0, 1);
      }else{
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
            case 24 : if ((file_open==1)&&(file_sp==0)) {
                        if (p_flag==0) (void) fclose(fp);
                        else{
                          if ((rc=pclose(fp))!=0) {
                            (void) sprintf(tmpbuff,
             "*** Warning: Shell returned non-zero status %i. ***\n",rc);
                            add_note(tmpbuff);
                          }
                        }
                      }
                      file_sp=p_flag=i=0;
                      ch=next_non_white(argbuff, &i);
                      if (ch=='`') {
                        for (j=strlen(argbuff);j>i;j--) {
                          if (argbuff[j]=='`') break;
                        }
                        argbuff[j]='\0';
                        fp=popen((argbuff+i+1), "w");
		        if (fp==(FILE *) NULL) {
			   sp_err(BADPIPE,tmp,l);
			   file_open=0;
		        }else{
			   file_open=p_flag=1;
		        }
                      }else{
                        for (j=i;j<strlen(argbuff);j++) {
                          if (argbuff[j]==',') break;
                        }
                        dir[0]='w';
                        dir[1]='\0';
                        if (argbuff[j]==',') {
                          argbuff[j++]='\0';
                          ch=next_non_white(argbuff, &j);
                          if ((ch=='a')||(ch=='A')) dir[0]='a';
                        }
		        if (l_comp((argbuff+i), "stdout")!=0) {
			   fp=stdout;
			   file_sp=1;
		        }else if (l_comp((argbuff+i), "stderr")!=0) {
			   fp=stderr;
			   file_sp=1;
		        }else{
		           fp=search_path((argbuff+i), dir, 0, 0);
			   file_sp=0;
		        }
		        if (fp==(FILE *) NULL) {
			   sp_err(NOFILE, tmp, l);
			   file_open=file_sp=0;
		        }else{
			   file_open=1;
		        }
                      }
		      break;
	    case 41 : if (file_open==0) {
			 sp_err(NOINFILE, -com[3], com[2]);
			 break;
		      }
                      (void) fprintf(fp, "%s\n", argbuff);
                      break;
	    case 81 : if (file_open==0) {
			 sp_err(NOINFILE, -com[3], com[2]);
			 break;
		      }
		      com2[1]=0;
		      nnums=0;
                      nrows= -1;
		      yank(argbuff, com2, inbuff);
		      while ((l=strlen(inbuff))!=0) {
                        t_fl=0;
                        if (inbuff[0]=='#') t_fl=1;
			inbuff[(15+t_fl)]='\0';
                        if ((inbuff[1]=='\0')&&(t_fl!=0)) {
                          varn[nnums++]= -200;
                        }else{
                          rc=find_var(datas, (inbuff+t_fl));
                          if (rc<0) {
                            sp_err(UNKVAR, (tmp+com2[3]+t_fl), (l-t_fl));
                          }else{
                            if (t_fl!=0) varn[nnums++]= -1-rc;
                            else varn[nnums++]=rc;
                          }
                        }
                        if (nrows==-1) {
                          if (varn[nnums-1]>=0) 
                            nrows=get_rows(datas, varn[nnums-1]);
                        }else{
                          if (varn[nnums-1]>=0) 
                                rc=get_rows(datas, varn[nnums-1]);
                          if (rc<nrows) nrows=rc;
                        }
			yank(argbuff,com2,inbuff);
                        if (((l=strlen(inbuff))!=0)&&(nnums==30)) {
                          sp_err(INOUT, (tmp+com2[3]), l);
                        }
		      }
                      if (nnums==0) break;
                      if (nrows==-1) nrows=1;
                      for (i=0;i<nrows;i++) {
                        for (j=0;j<nnums;j++) {
                          if (varn[j]<0) {
                            if (varn[j]==-200) (void) fprintf(fp, "%i", (i+1));
                            else{
                              rc=get_rows(datas, (-varn[j]-1));
                              (void) fprintf(fp, "%i", rc);
                            }
                          }else{
                            get_num(datas, varn[j], i, &i_val, &f_val);
                            if ((datas->vars[varn[j]])->type==TYPE_INT) {
                              (void) fprintf(fp, "%i", i_val);
                            }else{
                              (void) fprintf(fp, "%g", f_val);
                            }
                          }
                          if (j==(nnums-1)) (void) fprintf(fp, "\n");
                          else (void) fprintf(fp, " ");
                        }
                      }
                      break;
	    default : com[1]=com[3];
		      what (ILLCOM, (char *) NULL, com, 0, 1); 
		      break;
	 }
      }
      line_buff_flush(com[1], 1);
   } while (com[0]!=0);

   if ((file_open==1)&&(file_sp==0)) {
     if (p_flag==0) (void) fclose(fp);
     else{
       if ((rc=pclose(fp))!=0) {
         (void) sprintf(tmpbuff,
             "*** Warning: Shell returned non-zero status %i. ***\n",rc);
         add_note(tmpbuff);
       }
     }
   }
}

/*   piperr - catch the pipe dump error */

void piperr()
{
   sev_err(PIPERR);
}

/*   data_line - pulls in one line of information from file fp
                   (NULL indicates local mode reader)
	       - tosses out lines with non-numerics (except end, e or #)
	       - also tosses out blank lines
               - any lines beginning with # represent breaks
               - following lines with # will be ignored
	       - changes commas to spaces to allow comma delimitation
	       - removes double precision indicators to allow
		   atof input */

data_line(fp, buff, lim)
FILE *fp;
char *buff;
int lim;
{
   int i,fl;
   char ch;

   fl=0;
   while((fl==0)&&(m_eof(fp)==0)) {
     if (fp==(FILE *) NULL) read_data_line(buff, lim);
     else{
       i=0;
       while(((ch=fgetc(fp))!='\n')&&(feof(fp)==0)) {
           if (ch=='\\') {
           ch=fgetc(fp);
           if (feof(fp)!=0) break;
           if (ch=='\n') {
             *(buff+i++)=' ';
             ch=fgetc(fp);
             if (feof(fp)!=0) break;
           }else{
             *(buff+i++)='\\';
           }
         }
         *(buff+i++)=ch;
         if (i==(lim-3)) sev_err(BUFLOAD);
       }
       *(buff+i)='\0';
     }

     if ((*buff=='#')&&(num_fl==0)) break;

     fl=1;
     for (i=0;i<strlen(buff);i++) {
       ch= *(buff+i);
       if ((iscntrl(ch))||(ch==',')) ch=' ';
       if ((ch=='E')||(ch=='D')||(ch=='d')) ch='e';
       *(buff+i)=ch;
       if (((ch<'0')||(ch>'9'))&&(ch!='+')&&(ch!='-')&&(ch!='.')&&
            (ch!=' ')&&(ch!='e')&&(ch!='n')&&(ch!='N')) *buff='\0';
     }
     if (is_empty(buff)!=0) fl=0;
   }

#ifdef EBUG
 if (debug_level&DBG_DATA) {
     (void) fprintf(deb_log,"Data line:%s:\n",buff);
     (void) fflush(deb_log);
 } 
#endif

  num_fl=0;
  if (*buff=='#') num_fl=1;
}

/*  m_eof - my end of file comparator
          - returns non-zero if end of file, or end of prog if fp==NULL */

int m_eof(fp)
FILE *fp;
{
   if (fp==(FILE *) NULL) return(end_of_prog());
   return(feof(fp));
}

/*  type_num - returns the type of number contained in buff */

int type_num(buff)
char *buff;
{
   int type,i;

   type=TYPE_INT;
   for (i=0;i<strlen(buff);i++) {
      if ((*(buff+i)=='.')||(*(buff+i)=='e')) type=TYPE_FLT;
   }

#ifdef EBUG
 if (debug_level&DBG_DATA) {
   (void) fprintf(deb_log,"Value type->%i\n",type);
   (void) fflush(deb_log);
 }
#endif

   return (type);
}

/* end_comp - scans line buff for an 'END' statement 
	    - used to cut sections of data  */

int end_comp(buff)
char *buff;
{
    static char comp[5]="ene";
    int i,c,ok;
    char ch, next_non_white();

    i=ok=0;
    ch=next_non_white(buff, &i);
    for (c=0;c<strlen(comp);c++) {
       ch=clower(*(buff+c+i));
       if (ch!=comp[c]) break;
    }
    if (c==strlen(comp)) ok=1;
    if (*buff=='#') ok=1;
    if (ok!=0) num_fl=1;
    return(ok);
}

/* add_var - add a variable name to the data list (lowered)
           - if prec non-zero, double precision
           - always TYPE_INT by default
	   - initialize the various data structures
	   - returns -1 if out of room (for variables)
	   - returns variable number otherwise */

int add_var(datas, name, prec)
struct sp_data *datas;
char *name;
int prec;
{
   int i;
   struct var_t *ptr;

   if (datas->nvars>=100) return(-1);

   ptr=(struct var_t *) xalloc((unsigned int) sizeof(struct var_t),
                        "Unable to allocate variable storage table.");

   datas->vars[datas->nvars++]=ptr;
   for (i=0;i<20;i++) {
      ptr->name[i]=clower(*(name+i));
   }
   ptr->name[19]='\0';
   ptr->nrows=0;
   ptr->type=TYPE_INT;
   ptr->prec=prec+1;
   ptr->nblocks=1;
   ptr->maxtab=50;
   if (ptr->prec==PRC_SING) {
     ptr->sgle=(union data_t_s **) xalloc((unsigned int)
                        (ptr->maxtab*sizeof(union data_t_s *)),
                        "Unable to allocate data reference block.");
     *(ptr->sgle+0)=(union data_t_s *) xalloc((unsigned int)
                        (D_BLOCK*sizeof(union data_t_s)),
                        "Unable to allocate data storage block.");
   }else{
     ptr->dble=(union data_t_d **) xalloc((unsigned int)
                        (ptr->maxtab*sizeof(union data_t_d *)),
                        "Unable to allocate data reference block.");
     *(ptr->dble+0)=(union data_t_d *) xalloc((unsigned int)
                        (D_BLOCK*sizeof(union data_t_d)),
                        "Unable to allocate data storage block.");
   }

   return((datas->nvars-1));
}

/* del_var - delete variable indicated by num 
           - returns doing nothing if num is invalid */

del_var(datas, num)
struct sp_data *datas;
int num;
{
   int i;

   if (check_var(datas, num)!=0) return;

   empty_var(datas, num);
   
   if ((datas->vars[num])->prec==PRC_SING) {
     if ((datas->vars[num])->sgle!=(union data_t_s **) NULL) {
        xfree((char *) *((datas->vars[num])->sgle));
        xfree((char *) (datas->vars[num])->sgle);
     }
   }else{
     if ((datas->vars[num])->dble!=(union data_t_d **) NULL) {
        xfree((char *) *((datas->vars[num])->dble));
        xfree((char *) (datas->vars[num])->dble);
     }
   }
   xfree ((char *) datas->vars[num]);

   datas->nvars=datas->nvars-1;
   for (i=num;i<datas->nvars;i++) {
      datas->vars[i]=datas->vars[i+1];
   }
}

/* empty_var - empty out all data contained in the indicated variable */

empty_var(datas, num)
struct sp_data *datas;
int num;
{
   int i;

   if (check_var(datas, num)!=0) return;

   if ((datas->vars[num])->prec==PRC_SING) {
     if ((datas->vars[num])->sgle!=(union data_t_s **) NULL) {
       for (i=1;i<(datas->vars[num])->nblocks;i++) {
         xfree((char *) *((datas->vars[num])->sgle+i));
       }
     }
   }else{
     if ((datas->vars[num])->dble!=(union data_t_d **) NULL) {
       for (i=1;i<(datas->vars[num])->nblocks;i++) {
         xfree((char *) *((datas->vars[num])->dble+i));
       }
     }
   }
   (datas->vars[num])->nrows=0;
   (datas->vars[num])->nblocks=1;
   (datas->vars[num])->type=TYPE_INT;
}

/* change_var - changes the variable type to type
              - converts all data currently in the storage table
              - returns doing nothing if num is invalid */

change_var(datas, num, type)
struct sp_data *datas;
int num, type;
{
   union data_t_s *ptr_s;
   union data_t_d *ptr_d;
   int i, j;

   if (check_var(datas, num)!=0) return;
   if (type==(datas->vars[num])->type) return;

   if ((datas->vars[num])->prec==PRC_SING) {
     for (i=0;i<(datas->vars[num])->nblocks;i++) {
       for (j=0;j<D_BLOCK;j++) {
         ptr_s= *((datas->vars[num])->sgle+i)+j;
         if (type==TYPE_INT) {
           ptr_s->i=(int) ptr_s->f;
         }else{
           ptr_s->f=(float) ptr_s->i;
         }
       }
     }
   }else{
     for (i=0;i<(datas->vars[num])->nblocks;i++) {
       for (j=0;j<D_BLOCK;j++) {
         ptr_d= *((datas->vars[num])->dble+i)+j;
         if (type==TYPE_INT) {
           ptr_d->i=(int) ptr_d->f;
         }else{
           ptr_d->f=(double) ptr_d->i;
         }
       }
     }
   }

   (datas->vars[num])->type=type;
}

/* var_type - returns the type of variable num
            - returns -1 if variable undefined */

var_type(datas, num)
struct sp_data *datas;
int num;
{
   if (check_var(datas,num)!=0) return(-1);
   return((datas->vars[num])->type);
}

/*  find_var - return the variable number of name
             - returns -1 if variable unfound     */

int find_var(datas, name)
struct sp_data *datas;
char *name;
{
   int k;

#ifdef EBUG
   if (debug_level&DBG_DATA) {
      (void) fprintf(deb_log,"Seaching for variable %s\n",name);
      (void) fflush(deb_log);
   }
#endif

   for (k=0;k<datas->nvars;k++) {
     if (l_comp((datas->vars[k])->name, name)!=0) return(k);
   }
   return(-1);
}

/*  add_num - add the given number (in either format) to
		the indicated variable           */

add_num(datas, num, i_val, f_val)
struct sp_data *datas;
int num, i_val;
double f_val;
{
   struct var_t *ptr;

#ifdef EBUG
  if (debug_level&DBG_DATA) {
   (void) fprintf(deb_log,"Adding %g to variable %s\n",f_val,
   	       (datas->vars[num])->name);
   (void) fflush(deb_log);
  }
#endif

   if (check_var(datas, num)!=0) return;

   ptr=datas->vars[num];
/*   if ((ptr->nrows-(ptr->nblocks-1)*D_BLOCK)>=D_BLOCK) { */
   if ((ptr->nrows-((ptr->nblocks-1)<<10))>=D_BLOCK) {
     ptr->nblocks++;
     if (ptr->nblocks==ptr->maxtab) {
       ptr->maxtab=ptr->maxtab+50;
       if (ptr->prec==PRC_SING) {
         ptr->sgle=(union data_t_s **) xrealloc((char *) ptr->sgle,
                  (unsigned int) (ptr->maxtab*sizeof(union data_t_s *)),
                  "Unable to reallocate data reference block.");
       }else{
         ptr->dble=(union data_t_d **) xrealloc((char *) ptr->dble,
                  (unsigned int) (ptr->maxtab*sizeof(union data_t_d *)),
                  "Unable to reallocate data reference block.");
       }
     }
     if (ptr->prec==PRC_SING) {
       *(ptr->sgle+ptr->nblocks-1)=(union data_t_s *) xalloc((unsigned int)
                        (D_BLOCK*sizeof(union data_t_s)),
                        "Unable to allocate data storage block.");
     }else{
       *(ptr->dble+ptr->nblocks-1)=(union data_t_d *) xalloc((unsigned int)
                        (D_BLOCK*sizeof(union data_t_d)),
                        "Unable to allocate data storage block.");
     }
   }

   ex_num(datas, num, ptr->nrows, &i_val, &f_val, 1);
   ptr->nrows++;
}

/* ex_num - puts/retrives a specific number from the data list
            (internal function)
          - puts data in if dir non-zero */

ex_num(datas, num, rown, i_val, f_val, dir)
struct sp_data *datas;
int num, rown, *i_val, dir;
double *f_val;
{
   struct var_t *ptr;
   union data_t_s *ptr_s;
   union data_t_d *ptr_d;
   int bl_n, r_n;

   ptr=datas->vars[num];
/*   bl_n=rown/D_BLOCK;
   r_n=rown-bl_n*D_BLOCK; */
   bl_n=rown>>10;
   r_n=(rown&(0x3FF));

   if (ptr->prec==PRC_SING) {
     ptr_s= *(ptr->sgle+bl_n)+r_n;
     if (ptr->type==TYPE_INT) {
       if (dir==0) {
         *i_val=ptr_s->i;
         *f_val=(double) *i_val;
       }else ptr_s->i= *i_val;
     }else{
       if (dir==0) {
         *f_val=(double) ptr_s->f;
         *i_val=(int) *f_val;
       }else ptr_s->f=(float) *f_val;
     }
   }else{
     ptr_d= *(ptr->dble+bl_n)+r_n;
     if (ptr->type==TYPE_INT) {
       if (dir==0) {
         *i_val=ptr_d->i;
         *f_val=(double) *i_val;
       }else ptr_d->i= *i_val;
     }else{
       if (dir==0) {
         *f_val=ptr_d->f;
         *i_val=(int) *f_val;
       }else ptr_d->f= *f_val;
     }
   }
}


/* get_rows - retrieve the number of data values in variable num */

int get_rows(datas,num)
struct sp_data *datas;
int num;
{
   if (check_var(datas, num)!=0) return(0);
   return((datas->vars[num])->nrows);
}

/* get_num - retrieve the number from the indicated variable and row
           - returns doing nothing if bad num or rown */

get_num(datas, num, rown, i_val, f_val)
struct sp_data *datas;
int num,rown,*i_val;
double *f_val;
{

   if (check_var(datas, num)!=0) return;
   if ((rown<0)||(rown>=(datas->vars[num])->nrows)) return;

   ex_num(datas, num, rown, i_val, f_val, 0);
    
#ifdef EBUG
  if (debug_level&DBG_DATA) {
     (void) fprintf(deb_log,"Finding %i of %s: %g\n",rown,
	 (datas->vars[num])->name,*f_val);
     (void) fflush(deb_log);
  }
#endif
}

/* put_num - insert the number into the indicated variable and row
           - returns doing nothing if bad num or rown<0
           - fills out storage structure if rown>nrows */

put_num(datas, num, rown, i_val, f_val)
struct sp_data *datas;
int num, rown, i_val;
double f_val;
{
   int i;

   if (check_var(datas, num)!=0) return;
   if (rown<0) return;

   if (rown>=(datas->vars[num])->nrows) {
     i=rown-(datas->vars[num])->nrows+1;
     for (;i>0;i--) add_num(datas, num, 0, 0.0);
   }

   ex_num(datas, num, rown, &i_val, &f_val, 1);
    
#ifdef EBUG
  if (debug_level&DBG_DATA) {
     (void) fprintf(deb_log,"Putting %i of %s: %g\n",rown,
	 (datas->vars[num])->name,f_val);
     (void) fflush(deb_log);
  }
#endif
}

/* check_var - check for valid var number */

int check_var(datas,num)
struct sp_data *datas;
int num;
{
   if ((num<0)||(num>=datas->nvars)) return(1);
   if (datas->vars[num]==(struct var_t *) NULL) return(1);
   return(0);
}

/*  add_tab - add plotting entry to the table
	    - searches for plot variable vn, and updates
	       plot info table accordingly 
            - returns the error value from find_var   */

int add_tab(varn,datas,crd,vn)
char *varn;
struct sp_data *datas;
int vn;
struct var_set *crd;
{
   int rc,j;

#ifdef EBUG
   if (debug_level&DBG_DATA) {
     (void) fprintf(deb_log,"Adding %s to variable table\n",varn);
     (void) fflush(deb_log);
   }
#endif

   rc=find_var(datas,varn);
   if (rc!=-1) {
     if (vn>=0) {
       for (j=0;j<10;j++) crd->name[vn][j]= *(varn+j);
       crd->name[vn][9]='\0';
       crd->var_n[vn]=rc;
       j=(datas->vars[rc])->nrows;
       if ((crd->nrows<0)||(crd->nrows>j)) crd->nrows=j;
     }else{
       crd->extra_n[-1-vn]=rc;
       crd->extra_t[-1-vn]=(datas->vars[rc])->type;
     }
     rc=0;
   }
   return (rc);
}
