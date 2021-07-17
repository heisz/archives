/******************************************************************
                          sPLOTch!

  Sparse - math processing routine
 
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
extern char *prec_types[];
struct sp_data *loc_data;

struct pst_c {int val_t;
              union data_t_d val;
              int index;
             } op_stack[100];
int n_ops;
int abort_flag, mat_row;

union data_t_d sing_val[27];
int sing_type[27];

double m_tol=1.0e-8;
extern double m_pi;

int math_err;

/* calc - perform calculations on a data set */

calc(datas) 
struct sp_data *datas;
{
   int i, j, k, rc, com[4], l, tmp, com2[4], var_prec, n_parsed, max_l;
   int ranges[3], k_o;
   struct parse_stack parsed_stack[100];
   double calc_row();

   if (datas->set==0) {
     for (i=0;i<100;i++) datas->vars[i]=(struct var_t *) NULL;
     datas->nvars=0;
     datas->set=1;
   }

   var_prec=math_err=0;

   do { 
      com[1]=0;
      rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
      if (rc<0) {
	 what(BADCOM,(char *) NULL, com, 0, 1);
      }else{
	 switch (com[0]) {
	    case 0  : break;
	    case -2 : break;
            case 165: j= -1;
                      for (i=0;i<2;i++) {
                        if (l_comp(prec_types[i], argbuff)!=0) j=i;
                      }
                      if (j==-1) sp_err(BADPREC, tmp, l);
                      else var_prec=j;
                      break;
            case 131: for (k=0;k<strlen(argbuff);k++) {
                        if (argbuff[k]==':') break;
                      }
                      rc=0;
                      ranges[0]=ranges[1]= -1;
                      ranges[2]=1;
                      if (argbuff[k]==':') {
                        argbuff[k++]='\0';
                        for (i=0;i<3;i++) {
                          k_o=k;
                          (void) getc_buff(argbuff, &k, inbuff, 1000, ',');
                          if (is_empty(inbuff)==0) {
                            l=parse_it(datas, inbuff, (tmp+k_o), 
                                 parsed_stack, &n_parsed);
                            if (l<0) {rc--; continue;}
                            ranges[i]=(int) calc_row(datas, 1,
                                 parsed_stack, n_parsed);
                          }
                        }
                      }
                      max_l=parse_it(datas, argbuff, tmp, 
                        parsed_stack, &n_parsed);
                      if (max_l<0) rc--;
                      if (rc<0) {
                        math_err++;
                        (void) sprintf(tmpbuff,
                            "   %i -> Calculation aborted.\n", math_err);
                        add_note(tmpbuff);
                      }else{
                        if (ranges[0]<=0) ranges[0]=1;
                        if (ranges[1]<=0) ranges[1]=max_l;
                        compute_it(datas, ranges[0], ranges[1], ranges[2], 
                            parsed_stack, n_parsed);
                      }
                      break;
            case 117: com2[1]=0;
                      yank(argbuff, com2, inbuff);
                      while (is_empty(inbuff)==0) {
                         inbuff[15]='\0';
                         rc=find_var(datas, inbuff);
                         if (rc<0) {
                           sp_err(UNKVAR, (tmp+com2[3]), (int) strlen(inbuff));
                         }else{
                           del_var(datas, rc);
                         }
                         yank(argbuff, com2, inbuff);
                      }
                      break;
            case 119: com2[1]=0;
                      yank(argbuff, com2, inbuff);
                      while (is_empty(inbuff)==0) {
                         for (i=0;i<strlen(inbuff);i++) 
                                      if (inbuff[i]=='[') break;
                         if (i==strlen(inbuff)) i= -1;
                         else{ 
                           for (j=strlen(inbuff);j>i;j--) {
                             if (inbuff[j]==']') break;
                           }
                           inbuff[j]='\0';
                           inbuff[i]='\0';
                         }

                         if (strlen(inbuff)>15) {
                           (void) sprintf(tmpbuff,
              "*** Warning: Variable %s truncated to 15 characters. ***\n",
                                  inbuff);
                           add_note(tmpbuff);
                           inbuff[15]='\0';
                         }
                         rc=find_var(datas, inbuff);
                         if (rc>=0) {
                           (void) sprintf(tmpbuff,
              "*** Warning: Variable %s already exists (deleted). ***\n",
                                  inbuff);
                           add_note(tmpbuff);
                           del_var(datas, rc);
                         }
                         rc=add_var(datas, inbuff, var_prec);
                         if (rc<0) sev_err(OVERDATA);
                         if (i>=0) {
                           j=atoi(inbuff+i+1)-1;
                           put_num(datas, rc, j, 0, 0.0);
                         }
                         yank(argbuff, com2, inbuff);
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

char *op_key[]={";","=","|","&",">","<","==","<=",">=","!=",
                 "~=","+","-","*","/","%","^","(","[","-","!"};

int op_prec[]={0,1,2,2,3,3,3,3,3,3,
                 3,4,4,5,5,6,7,10,10,8,9};

char *fn_names[]={"sum","prod","avg","median","stddev","maxr",
            "minr","max","min","if","ln","log","exp","sqrt",
            "int","rnd","abs","sin","cos","tan","asin",
            "acos","atan2","atan","rand","exy","pi","hypot",
            "num","sgn","row","float","xmesh","ymesh"};

int fn_args[]={1,1,1,1,1,1,
               1,2,2,3,1,1,1,1,
               1,2,1,1,1,1,1,
               1,2,1,0,2,0,2,
               1,1,1,1,2,2};

#define N_FN 34
#define N_OP 21


char *op_types[]={"Colon","Equal", "Or", "And", "Greater", "Lesser",
   "Full Equal", "Lesser or Equal", "Greater or Equal", "Not Equal",
   "Almost Equal", "Add", "Subtract", "Multiply", "Divide", "Modulus", 
   "Exponent", "lbr", "lsqb", "Negate", "Not", "Function", "Start", "Stop"};

/* parse_it - parses the process string into an execute stack
            - returns negative if error occurs
            - returns the largest number of rows from variable list */

int parse_it(datas, buff, lpos, parsed_stack, n_p)
struct sp_data *datas;
struct parse_stack parsed_stack[];
char *buff;
int lpos, *n_p;
{
   char *ptr, ch;
   int i, j, k, t_op, t_op_ptr, i_flag, s_fl, t_pt[5], nt_pt;
   int var_n, n_rows, t_prec, n_to_parse, n_parsed;
   struct pst_a {char *ptr;
             } to_parse_stack[100];

   n_to_parse=1;
   to_parse_stack[0].ptr=buff;
   n_parsed=1;
   parsed_stack[0].op=23;
   n_rows= -1;

/* parse loop (to build parse stack) */

   while (n_to_parse!=0) {
     ptr=to_parse_stack[n_to_parse-1].ptr;
     t_prec= -1;
     s_fl=1;
     for (i=0;i<(strlen(ptr)-1);i++) {
       ch= *(ptr+i);
       if ((ch==' ')||iswht(ch)) continue;
       if ((ch=='-')&&(s_fl==1)) {j=19; s_fl=0;}
       else if ((ch=='!')&&(s_fl==1)&&(*(ptr+i+1)!='=')) {j=20; s_fl=0;}
       else{
         s_fl=0;
         for (j=0;j<19;j++) {
           for (k=0;k<strlen(op_key[j]);k++) {
             if(*(ptr+i+k)!=*(op_key[j]+k)) break;
           }
           if ((k==strlen(op_key[j]))&&(*(ptr+i+k)!='=')) break;
         }
         if (j==19) j+=2;  /* skip previously scanned leftwise operators */
       }
       if (j!=21) {
         if (j==18) {
           if (skip_bra(ptr, &i, '[', ']')<0) {
             sp_err(BRAMISS, (lpos+i+ptr-buff+1), 0);
             return(-1);
           }
         }else if (j==17) {
           if (skip_bra(ptr, &i, '(', ')')<0) {
             sp_err(BRAMISS, (lpos+i+ptr-buff+1), 0);
             return(-1);
           }
         }else if (j==12) {
           ch= *(ptr+i-1);
           if ((ch!='*')&&(ch!='/')&&(ch!='%')&&(ch!='^')&&(ch!='!')) {
             if ((op_prec[j]<t_prec)||(t_prec==-1)) {
               t_prec=op_prec[j];
               t_op=j;
               t_op_ptr=i;
             }else if ((op_prec[j]==t_prec)&&(j!=1)) {
               t_op=j;
               t_op_ptr=i;
             }
           }
         }else{
           if ((op_prec[j]<t_prec)||(t_prec==-1)) {
             t_prec=op_prec[j];
             t_op=j;
             t_op_ptr=i;
           }else if ((op_prec[j]==t_prec)&&(j!=1)) {
             t_op=j;
             t_op_ptr=i;
           }
         }
         if (strlen(op_key[j])==2) i++;
       }
     }

     if (t_prec==-1) {  /* variable, number, bracket or function*/
       i=0;
       while(((ch= *(ptr+i))==' ')||iswht(ch)) i++;
       ptr=ptr+i;
       if (ch=='\0') {
         sp_err(MISSVAR, (lpos+ptr-buff), 0);
         return(-1);
       }else if (ch=='(') {
         for (i=(strlen(ptr)-1);i>=0;i--) {
           if (*(ptr+i)==')') break;
         }
         if (i<0) {
           sp_err(BRAMISS, (lpos+ptr-buff), 0);
           return(-1);
         }else{
           *(ptr+i)='\0';
         }
         to_parse_stack[n_to_parse-1].ptr=ptr+1;

       }else if (ch=='@') {
         if (*(ptr+1)=='@') {
           ch=clower(*(ptr+2));
           if (((ch<'a')||(ch>'z'))&&(ch!='@')) {
             sp_err(BADSING, (lpos+ptr-buff), 3);
             return(-1);
           }else{
             parsed_stack[n_parsed].op= -4;
             if (ch!='@') {
              parsed_stack[n_parsed++].num.i=ch-'a';
             }else{
              parsed_stack[n_parsed++].num.i=26;
             }
             n_to_parse--;
           }
         }else{
           for (i=0;i<N_FN;i++) {
             for(j=0;j<strlen(fn_names[i]);j++) {
               ch=clower(*(ptr+j+1));
               if (ch!=*(fn_names[i]+j)) break;
             }
             if (j==strlen(fn_names[i])) break;
           }
           if (i==N_FN) {
             sp_err(BADFUNC, (lpos+ptr-buff), (int) strlen(ptr));
             return(-1);
           }else{
             if (fn_args[i]!=0) {
               j=strlen(fn_names[i]);
               while(((ch= *(ptr+j))!='(')&&(ch!='\0')) j++;
               if (ch=='\0') {
                 sp_err(BADARG, (lpos+ptr-buff), (int) strlen(ptr));
                 return(-1);
               }else{
                 ptr=ptr+j+1;
                 for (j=(strlen(ptr)-1);j>=0;j--) {
                   if (*(ptr+j)==')') break;
                 }
                 if (j<0) {
                   sp_err(BRAMISS, (lpos+ptr-buff-1), 0);
                   return(-1);
                 }else{
                   *(ptr+j)='\0';
                   t_pt[0]=0;
                   nt_pt=1;
                   for (j=0;j<strlen(ptr);j++) {
                     ch= *(ptr+j);
                     if (ch==',') {
                       t_pt[nt_pt++]=j+1;
                     }else if (ch=='(') {
                       if (skip_bra(ptr, &j, '(', ')')<0) {
                         sp_err(BRAMISS, (lpos+j+ptr-buff), 0);
                         return(-1);
                       }
                     }else if (ch=='[') {
                       if (skip_bra(ptr, &j, '[', ']')<0) {
                         sp_err(BRAMISS, (lpos+j+ptr-buff), 0);
                         return(-1);
                       }
                     }
                   }
                   if (nt_pt!=fn_args[i]) {
                     sp_err(BADARG, (lpos+ptr-buff), (int) strlen(ptr));
                     return(-1);
                   }else{
                     parsed_stack[n_parsed].op= -7;
                     parsed_stack[n_parsed++].num.i=i;
                     for (j=0;j<nt_pt;j++) {
                       to_parse_stack[n_to_parse+j-1].ptr=ptr+t_pt[j];
                       *(ptr+t_pt[j]-1)='\0';
                     }
                     n_to_parse=n_to_parse+j-1;
                   }
                 }
               }
             }else{
               parsed_stack[n_parsed].op= -7;
               parsed_stack[n_parsed++].num.i=i;
               n_to_parse--;
             }
           }
         }

       }else{
         i_flag=1;
         for (i=0;i<strlen(ptr);i++) {
           ch= *(ptr+i);
           if (((ch<'0')||(ch>'9'))&&(ch!=' ')&&(!iswht(ch))&&(ch!='.')) {
             break;
           }
           if (ch=='.') i_flag=0;
         }
         if (i>=strlen(ptr)) {
           if (i_flag!=0) {
             parsed_stack[n_parsed].op= -6;
             parsed_stack[n_parsed++].num.i=atoi(ptr);
           }else{
             parsed_stack[n_parsed].op= -5;
             parsed_stack[n_parsed++].num.f=atof(ptr);
           }
           n_to_parse--;
         }else{
           for (i=0;i<strlen(ptr);i++) {
             ch= *(ptr+i);
             if ((ch=='[')||(ch==' ')||iswht(ch)) break;
           }
           if (i!=strlen(ptr)) *(ptr+i)='\0';
           if (*ptr=='#') {
             var_n= -1;
           }else{
             if (strlen(ptr)>15) *(ptr+15)='\0';
             var_n=find_var(datas,ptr);
             if (var_n<0) {
               sp_err(UNKVAR,(lpos+ptr-buff), (int) strlen(ptr));
               return(-1);
             }
             k=get_rows(datas, var_n);
             if (k>n_rows) n_rows=k;
           }
           parsed_stack[n_parsed].op= -1;
           parsed_stack[n_parsed++].num.i=var_n;
           n_to_parse--;
           if (ch=='[') {
             ptr=ptr+i+1;
             for (i=(strlen(ptr)-1);i>=0;i--) {
               if (*(ptr+i)==']') break;
             }
             if (i<0) {
               sp_err(BRAMISS, (lpos+ptr-buff-1), 0);
               return(-1);
             }else{
               *(ptr+i)='\0';
               nt_pt=0;
               for (i=0;i<strlen(ptr);i++) {
                 ch= *(ptr+i);
                 if (ch==',') {
                   t_pt[nt_pt++]=i+1;
                 }else if (ch=='(') {
                   if (skip_bra(ptr, &i, '(', ')')<0) {
                     sp_err(BRAMISS, (lpos+i+ptr-buff), 0);
                     return(-1);
                   }
                 }else if (ch=='[') {
                   if (skip_bra(ptr, &i, '[', ']')<0) {
                     sp_err(BRAMISS, (lpos+i+ptr-buff), 0);
                     return(-1);
                   }
                 }
               }
               if (nt_pt>1) {
                 sp_err(BADRANGE, (lpos+ptr-buff), (int) strlen(ptr));
                 return(-1);
               }else{
                 if (nt_pt==0) {
                   parsed_stack[n_parsed-1].op= -2;
                   to_parse_stack[n_to_parse++].ptr=ptr;
                 }else{
                   parsed_stack[n_parsed-1].op= -3;
                   to_parse_stack[n_to_parse++].ptr=ptr;
                   *(ptr+t_pt[0]-1)='\0';
                   to_parse_stack[n_to_parse++].ptr=ptr+t_pt[0];
                 }
               }
             }
           }
         }
       }
     }else{   /* operation */
       if ((t_op==19)||(t_op==20)) {
         parsed_stack[n_parsed++].op=t_op;
         to_parse_stack[n_to_parse-1].ptr=
            to_parse_stack[n_to_parse-1].ptr+t_op_ptr+1;
       }else{
         if ((t_op>6)&&(t_op<10)) {
           parsed_stack[n_parsed++].op=20;
           parsed_stack[n_parsed++].op=t_op-3;
         }else{
           parsed_stack[n_parsed++].op=t_op;
         }
         *(ptr+t_op_ptr)='\0';
         to_parse_stack[n_to_parse++].ptr=ptr+t_op_ptr+strlen(op_key[t_op]);
       }
     }
   }

   parsed_stack[n_parsed++].op=22;

#ifdef EBUG
   if (debug_level&DBG_CALC) {
     (void) fprintf(deb_log,"CALC -> op_stack parsing result\n");
     for (i=0;i<n_parsed;i++) {
       if (parsed_stack[i].op==-7) {
         (void) fprintf(deb_log,"  function %s\n",
                        fn_names[parsed_stack[i].num.i]);
       }else if (parsed_stack[i].op==-6) {
         (void) fprintf(deb_log,"  integer value %i\n",parsed_stack[i].num.i);
       }else if (parsed_stack[i].op==-5) {
         (void) fprintf(deb_log,"  float value %g\n",parsed_stack[i].num.f);
       }else if (parsed_stack[i].op==-4) {
         (void) fprintf(deb_log,"  single variable %i\n",parsed_stack[i].num.i);
       }else if (parsed_stack[i].op==-3) {
         (void) fprintf(deb_log,"  variable %i (range)\n",parsed_stack[i].num.i);
       }else if (parsed_stack[i].op==-2) {
         (void) fprintf(deb_log,"  variable (set) %i\n",parsed_stack[i].num.i);
       }else if (parsed_stack[i].op==-1) {
         (void) fprintf(deb_log,"  variable %i\n",parsed_stack[i].num.i);
       }else (void) fprintf(deb_log,"  %s\n",op_types[parsed_stack[i].op]);
     }
   }
#endif

   if (n_rows<=0) n_rows=1;
   *n_p=n_parsed;
   return(n_rows);
}

/* skip_bra - skips material enclosed between brackets
            - returns -1 and j pointing at open if error
            - returns 1 and j point at close otherwise */

skip_bra(ptr,j,open,close)
char *ptr, open, close;
int *j;
{
   int nbr,k;
   char ch;

   nbr=1;
   k= *j+1;
   while (nbr!=0) {
     ch= *(ptr+k++);
     if (ch==open) nbr++;
     else if (ch==close) nbr--;
     else if (ch=='\0') {
       return(-1);
     }
   }
   *j=k-1;
   return(1);
}

/** various math functions **/

/* get_val_type - returns type of nth variable down stack
                - 0 for integer, 1 for float */
                
int get_val_type(nth)
int nth;
{
   if ((n_ops-nth-1)<0) {
     (void) fprintf(stderr,"Fatal stack failure (get_type). %i\n",n_ops);
     (void) exit(1);
   }
   if (op_stack[n_ops-nth-1].val_t==-1) return(1);
   else if (op_stack[n_ops-nth-1].val_t==-2) return(0);
   else if (op_stack[n_ops-nth-1].val_t==-3)
      return(sing_type[op_stack[n_ops-nth-1].val.i]);

   if(var_type(loc_data, op_stack[n_ops-nth-1].val_t)==TYPE_INT) return(0);
   return(1);
}

/* get_act_val - returns value from stack
               - decrements stack pile if dec non-zero
               - sets abort_flag if mat_row not in variable range */

get_act_val(i_val, f_val, dec)
int *i_val,dec;
double *f_val;
{
   if ((dec!=0)&&(n_ops==0)) {
     (void) fprintf(stderr,"Fatal stack failure (get_act_val).\n");
     (void) exit(1);
   }

   if (op_stack[n_ops-1].val_t==-1) {
     *f_val=op_stack[n_ops-1].val.f;
     *i_val=(int) *f_val;
   }else if (op_stack[n_ops-1].val_t==-2) {
     *i_val=op_stack[n_ops-1].val.i;
     *f_val=(double) *i_val;
   }else if (op_stack[n_ops-1].val_t==-3) {
     if (sing_type[op_stack[n_ops-1].val.i]!=0) {
       *f_val=sing_val[op_stack[n_ops-1].val.i].f;
       *i_val=(int) *f_val;
     }else{
       *i_val=sing_val[op_stack[n_ops-1].val.i].i;
       *f_val=(double) *i_val;
     }
   }else{
     if (op_stack[n_ops-1].index==-2) {
       if (mat_row>get_rows(loc_data, op_stack[n_ops-1].val_t)) {
         abort_flag=1;
       }else{
         get_num(loc_data, op_stack[n_ops-1].val_t, (mat_row-1), 
                i_val, f_val);
       }
     }else if (op_stack[n_ops-1].index==-1) {
       if ((op_stack[n_ops-1].val.i>
               get_rows(loc_data, op_stack[n_ops-1].val_t)||
                  (op_stack[n_ops-1].val.i)<0)) {
         abort_flag=1;
       }else{
         get_num(loc_data, op_stack[n_ops-1].val_t, 
             (op_stack[n_ops-1].val.i-1), i_val, f_val);
       }
     }else{
       if ((mat_row>op_stack[n_ops-1].index)||
                    (mat_row<op_stack[n_ops-1].val.i)) {
          abort_flag=1;
       }else{
         if (mat_row>get_rows(loc_data, op_stack[n_ops-1].val_t)) {
           abort_flag=1;
         }else{
           get_num(loc_data, op_stack[n_ops-1].val_t, 
                (mat_row-1), i_val, f_val);
         }
       }
     }
   }

   if (dec==1) n_ops--;
}

/* get_var_range - return the variable number and range for functions
                 - if var_n -1, f_val contains value (non-range)
                 - if var_n -2, begin contains integer value (non-range)
                 - if var_n -3, out of range  
                 - if var_n -4, special variable indicated by begin
                          (only possible if fl non-zero)
                 - do not check upper bound if fl non-zero, and return
                      end negative if undefined */

get_var_range(var_n, begin, end, f_val, dec, fl)
int *var_n, *begin, *end, dec, fl;
double *f_val;
{
   if ((dec!=0)&&(n_ops==0)) {
     (void) fprintf(stderr,"Fatal stack failure (get_range).\n");
     (void) exit(1);
   }

   if (op_stack[n_ops-1].val_t==-1) {
     *var_n= -1;
     *f_val=op_stack[n_ops-1].val.f;
   }else if (op_stack[n_ops-1].val_t==-2) {
     *var_n= -2;
     *begin=op_stack[n_ops-1].val.i;
   }else if (op_stack[n_ops-1].val_t==-3) {
     if (fl==0) {
       if (sing_type[op_stack[n_ops-1].val.i]!=0) {
         *var_n= -1;
         *f_val=sing_val[op_stack[n_ops-1].val.i].f;
       }else{
         *var_n= -2;
         *begin=sing_val[op_stack[n_ops-1].val.i].i;
       }
     }else{
       *var_n= -4;
       *begin=op_stack[n_ops-1].val.i;
     }
   }else{
     if (op_stack[n_ops-1].index==-2) {
       *var_n=op_stack[n_ops-1].val_t;
       if (fl==0) {
         *begin=0;
         *end=get_rows(loc_data, *var_n)-1;
       }else{
         *begin=0;
         *end=mat_row-1;
       }
     }else{ 
       *var_n=op_stack[n_ops-1].val_t;
       if ((op_stack[n_ops-1].val.i>get_rows(loc_data, *var_n))&&(fl==0)) {
         *var_n= -3;
       }else{
         if (op_stack[n_ops-1].index!=-1) {
           *begin=op_stack[n_ops-1].val.i-1;
           if (*begin<0) *begin=0;
           *end= *begin;
           if (op_stack[n_ops-1].index<0) {
             *var_n= -3;
           }else{
             *end=op_stack[n_ops-1].index-1;
             if ((*end>=get_rows(loc_data, *var_n))&&(fl==0))
                      *end=get_rows(loc_data, *var_n)-1;
           }
         }else{
           *begin=op_stack[n_ops-1].val.i-1;
           if (fl==0) *end= *begin;
           else *end= -1;
           if (*begin<0) *var_n= -3;
         }
       }
     }
   }

   if (dec==1) n_ops--;
}

/* push_val - push a numerical result onto the stack 
            - type is zero for integer, non-zero for float */

push_val(i_val, f_val, type)
int i_val, type;
double f_val;
{
   if (type==0) {
     op_stack[n_ops].val_t= -2;
     op_stack[n_ops++].val.i=i_val;
   }else{
     op_stack[n_ops].val_t= -1;
     op_stack[n_ops++].val.f=f_val;
   }
}

/* push_var - push a variable onto the stack */

push_var(var_n, begin, end)
int var_n, begin, end;
{
   op_stack[n_ops].val_t=var_n;
   op_stack[n_ops].val.i=begin;
   op_stack[n_ops++].index=end;
}

int i_opval, i_opa, i_opb;
double f_opval, f_opa, f_opb;
 
m_double(oper)
int oper;
{
   if ((get_val_type(0)+get_val_type(1))!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     switch(oper) {
        case 1: f_opval=f_opb+f_opa; break;
        case 2: f_opval=f_opb-f_opa; break;
        case 3: f_opval=f_opb*f_opa; break;
        case 4: f_opval=f_opb;
                if (f_opa>f_opb) f_opval=f_opa;
                break;
        case 5: f_opval=f_opb;
                if (f_opa<f_opb) f_opval=f_opa;
                break;
        default: break;
     }
     push_val(0, f_opval, 1);
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     switch(oper) {
        case 1: i_opval=i_opb+i_opa; break;
        case 2: i_opval=i_opb-i_opa; break;
        case 3: i_opval=i_opb*i_opa; break;
        case 4: i_opval=i_opb;
                if (i_opa>i_opb) i_opval=i_opa;
                break;
        case 5: i_opval=i_opb;
                if (i_opa<i_opb) i_opval=i_opa;
                break;
        default: break;
     }
     push_val(i_opval, (double) 0.0, 0);
   }
}

m_null()
{
}

m_eql()
{
   int var_n, beg, end, i_val, type;
   double f_val, fv_t;

   type=get_val_type(0);
   get_act_val(&i_val, &f_val, 1);
   get_var_range(&var_n, &beg, &end, &fv_t, 1, 1);

   if (type==0) push_val(i_val, (double) 0.0, 0);
   else push_val(0, f_val, 1);

   if (var_n==-4) {
      sing_type[beg]=type;
      if (type==0) sing_val[beg].i=i_val;
      else sing_val[beg].f=f_val;
      return;
   }

   if (var_n<0) {
     abort_flag=1;
   }else{
     if (end>=0) {
       if (((mat_row-1)<beg)||((mat_row-1)>end)) beg= -1;
       else beg=mat_row-1;
     }
     if (beg<0) {
       abort_flag=1;
     }else{
       if (abort_flag==0) {
         if (type==0) {
           put_num(loc_data, var_n, beg, i_val, f_val);
         }else{
           if (var_type(loc_data, var_n)==TYPE_INT) {
             change_var(loc_data, var_n, TYPE_FLT);
           }
           put_num(loc_data, var_n, beg, i_val, f_val);
         }
       }
     }
   }
}

m_l_or()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   i_opval=i_opb | i_opa;
   push_val(i_opval, (double) 0.0, 0);
}

m_l_and()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   i_opval=i_opb & i_opa;
   push_val(i_opval, (double) 0.0, 0);
}

m_cmp(oper)
int oper;
{
   if ((get_val_type(0)+get_val_type(1))!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     i_opval=0;
     switch(oper) {
        case 1: if (f_opb>f_opa) i_opval=1; break;
        case 2: if (f_opb<f_opa) i_opval=1; break;
        case 3: if (f_opb==f_opa) i_opval=1; break;
        default: break;
     }
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     i_opval=0;
     switch(oper) {
        case 1: if (i_opb>i_opa) i_opval=1; break;
        case 2: if (i_opb<i_opa) i_opval=1; break;
        case 3: if (i_opb==i_opa) i_opval=1; break;
        default: break;
     }
   }
   push_val(i_opval, (double) 0.0, 0);
}

m_gt_tn()
{
   m_cmp(1);
}

m_ls_tn()
{
   m_cmp(2);
}

m_c_eql()
{
   m_cmp(3);
}

m_c_almeql()
{
   if ((get_val_type(0)+get_val_type(1))!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     i_opval=0;
     if (((f_opa-m_tol)<f_opb)&&((f_opa+m_tol)>f_opb)) i_opval=1;
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     i_opval=0;
     if (i_opa==i_opb) i_opval=1;
   }
   push_val(i_opval, (double) 0.0, 0);
}

m_add()
{
   m_double(1);
}

m_sub()
{
   m_double(2);
}

m_tms()
{
   m_double(3);
}

m_div()
{
   if ((get_val_type(0)+get_val_type(1))!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     f_opval=f_opb/f_opa;
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     f_opval=((double) i_opb)/i_opa;
   }
   push_val(0, f_opval, 1);
}

m_mod()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   i_opval=i_opb % i_opa;
   push_val(i_opval, (double) 0.0, 0);
}

m_exp()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   f_opval=pow(f_opb, f_opa);
   push_val(0, f_opval, 1);
}

m_neg()
{
   if (get_val_type(0)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     f_opval= -f_opa;
     push_val(0, f_opval, 1);
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     i_opval= -i_opa;
     push_val(i_opval, (double) 0.0, 0);
   }
}

m_not()
{
   i_opval=0;
   if (get_val_type(0)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     if (f_opa==0.0) i_opval=1;
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     if (i_opa==0) i_opval=1;
   }
   push_val(i_opval, (double) 0.0, 0);
}

range_op(oper)
int oper;
{
   int i, type, var_n, begin, end;

   type=get_val_type(0);
   get_var_range(&var_n, &begin, &end, &f_opa, 1, 0);
   if (var_n==-3) {
     abort_flag=1;
   }else if (var_n==-2) {
     push_val(begin, (double) 0.0, 0);
   }else if (var_n==-1) {
     push_val(0, f_opa, 1);
   }else{
     if (type!=0) {
       if (oper==1) f_opval=0.0;
       else if (oper==2) f_opval=1.0;
       else{
          get_num(loc_data, var_n, begin, &i_opa, &f_opval);
       }
       for (i=begin;i<=end;i++) {
         get_num(loc_data, var_n, i, &i_opa, &f_opa);
         if (oper==1) f_opval=f_opval+f_opa;
         else if (oper==2) f_opval=f_opval*f_opa;
         else if (oper==3) {
            if (f_opa>f_opval) f_opval=f_opa;
         }else{
            if (f_opa<f_opval) f_opval=f_opa;
         }
       }
       push_val(0, f_opval, 1);
     }else{
       if (oper==1) i_opval=0;
       else if (oper==2) i_opval=1;
       else{
          get_num(loc_data, var_n, begin, &i_opa, &f_opa);
          i_opval=i_opa;
       }
       for (i=begin;i<=end;i++) {
         get_num(loc_data, var_n, i, &i_opa, &f_opa);
         if (oper==1) i_opval=i_opval+i_opa;
         else if (oper==2) i_opval=i_opval*i_opa;
         else if (oper==3) {
            if (i_opa>i_opval) i_opval=i_opa;
         }else{
            if (i_opa<i_opval) i_opval=i_opa;
         }
       }
       push_val(i_opval, (double) 0.0, 0);
     }
   }
}

f_sum()
{
   range_op(1);
}

f_prod()
{
   range_op(2);
}

f_avg()
{
   int var_n, begin, end;

   get_var_range(&var_n, &begin, &end, &f_opa, 0, 0);
   if (var_n==-3) {
     abort_flag=1;
   }else{
     range_op(1);
     get_act_val(&i_opa, &f_opa, 1);
     if (var_n<0) {
       push_val(0, f_opa, 1);
     }else{
       f_opval=f_opa/(end-begin+1.0);
       push_val(0, f_opval, 1);
     }
   }
}

f_med()
{
   int i, index, type, var_n, begin, end, cnta, cntb, cntc, inda;
   double fva, fvb, fv_eq;

   type=get_val_type(0);
   get_var_range(&var_n, &begin, &end, &f_opa, 1, 0);
   if (var_n==-3) {
     abort_flag=1;
   }else if (var_n==-2) {
     push_val(begin, (double) 0.0, 0);
   }else if (var_n==-1) {
     push_val(0, f_opa, 1);
   }else{
     inda= -1;
     for (index=begin;index<=end;index++) {
       get_num(loc_data, var_n, index, &i_opa, &fva);
       cnta=cntb=cntc=0;
       for (i=begin;i<=end;i++) {
         get_num(loc_data, var_n, i, &i_opb, &fvb);
         if (type!=0) {
           if (fvb<fva) cnta++;
           else if (fvb>fva) cntc++;
           else cntb++;
         }else{
           if (i_opb<i_opa) cnta++;
           else if (i_opb>i_opa) cntc++;
           else cntb++;
         }
       }
       if (abs(cntc-cnta)==cntb) {
         if (inda==-1) {
           inda=index;
           fv_eq=fva;
         }else{ 
           if (fva!=fv_eq) break;
         }
       }else if (abs(cntc-cnta)<cntb) {
         break;
       }
     }
     if (index>end) {
       (void) fprintf(stderr,"Fatal error in median routine.\n");
       (void) exit(1);
     }
     if (inda==-1) {
       if (type!=0) {
         push_val(0, fva, 1);
       }else{
         push_val(i_opa, (double) 0.0, 0);
       }
     }else{
       get_num(loc_data, var_n, inda, &i_opb, &fvb);
       if (type!=0) {
         f_opval=(fva+fvb)/(double) 2.0;
         push_val(0, f_opval, 1);
       }else{
         i_opval=(i_opa+i_opb)/2.0;
         f_opval=(fva+fvb)/2.0;
         if (f_opval==(double) i_opval) {
           push_val(i_opval, (double) 0.0, 0);
         }else{
           push_val(0, f_opval, 1);
         }
       }
     }
   }
}

f_stddev()
{
   int i, var_n, begin, end;
   double fv;

   get_var_range(&var_n, &begin, &end, &f_opa, 0, 0);
   if (var_n==-3) {
     abort_flag=1;
   }else{
     f_avg();
     get_act_val(&i_opa, &f_opa, 1);
     if (var_n<0) {
       push_val(0, 0.0, 0);
     }else{
       f_opval=0.0;
       for (i=begin;i<=end;i++) {
         get_num(loc_data, var_n, i, &i_opb, &fv);
         f_opval=f_opval+(fv-f_opa)*(fv-f_opa);
       }
       if (end==begin) {
         f_opval=0.0;
       }else{
         f_opval=sqrt(f_opval/(end-begin));
       } 
       push_val(0, f_opval, 1);
     }
   }
}

f_maxr()
{
   range_op(3);
}

f_minr()
{
   range_op(4);
}

f_max()
{
   m_double(4);
}

f_min()
{
   m_double(5);
}

f_if()
{
   int crit, typea, typeb;
   
   typea=get_val_type(0);
   typeb=get_val_type(1);
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   get_act_val(&crit, &f_opval, 1);
   if (crit!=0) {
     if (typeb!=0) {
        push_val(0, f_opb, 1);
     }else{
        push_val(i_opb, (double) 0.0, 0);
     }
   }else{
     if (typea!=0) {
        push_val(0, f_opa, 1);
     }else{
        push_val(i_opa, (double) 0.0, 0);
     }
   }
}

f_ln()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=log(f_opa);
   push_val(0, f_opval, 1);
}

f_log()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=log10(f_opa);
   push_val(0, f_opval, 1);
}

f_exp()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=exp(f_opa);
   push_val(0, f_opval, 1);
}

f_sqrt()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=sqrt(f_opa);
   push_val(0, f_opval, 1);
}

f_int()
{
   if (get_val_type(0)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     i_opval=(int) f_opa;
     push_val(i_opval, (double) 0.0, 0);
   }
}

f_float()
{
   if (get_val_type(0)==0) {
     get_act_val(&i_opa, &f_opa, 1);
     f_opval=(double) i_opa;
     push_val(0, f_opval, 1);
   }
}

f_rnd()
{
   if (get_val_type(1)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     i_opval=fint(f_opb, i_opa);
     push_val(i_opval, (double) 0.0, 0);
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     push_val(i_opb, (double) 0.0, 0);
   }
}

f_abs()
{
   if (get_val_type(0)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     if (f_opa<0.0) f_opa= -f_opa;
     push_val(0, f_opa, 1);
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     if (i_opa<0) i_opa= -i_opa;
     push_val(i_opa, (double) 0.0, 0);
   }
}

f_sin()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=sin(f_opa);
   push_val(0, f_opval, 1);
}

f_cos()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=cos(f_opa);
   push_val(0, f_opval, 1);
}

f_tan()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=tan(f_opa);
   push_val(0, f_opval, 1);
}

f_asin()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=asin(f_opa);
   push_val(0, f_opval, 1);
}

f_acos()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=acos(f_opa);
   push_val(0, f_opval, 1);
}

f_atan()
{
   get_act_val(&i_opa, &f_opa, 1);
   f_opval=atan(f_opa);
   push_val(0, f_opval, 1);
}

f_atan2()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   f_opval=atan2(f_opb, f_opa);
   push_val(0, f_opval, 1);
}

f_rand()
{
   float ran2();

   f_opval=ran2();
   push_val(0, f_opval, 1);
}

f_exy()
{
   int i;

   if (get_val_type(1)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     f_opval=1.0;
     if (i_opa>0) {
       for (i=0;i<i_opa;i++) {
         f_opval=f_opval*f_opb;
       }
     }else{
       for (i=0;i<(-i_opa);i++) {
         f_opval=f_opval/f_opb;
       }
     }
     push_val(0, f_opval, 1);
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     get_act_val(&i_opb, &f_opb, 1);
     if (i_opa>0) {
       i_opval=1;
       for (i=0;i<i_opa;i++) {
         i_opval=i_opval*i_opb;
       }
       push_val(i_opval, (double) 0.0, 0);
     }else{
       f_opval=1.0;
       for (i=0;i<(-i_opa);i++) {
         f_opval=f_opval/i_opb;
       }
       push_val(0, f_opval, 1);
     }
   }
}

f_pi()
{
   push_val(0, m_pi, 1);
}

f_hypot()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   f_opval=hypot(f_opb, f_opa);
   push_val(0, f_opval, 1);
}

f_num()
{
   int var_n, begin, end;

   get_var_range(&var_n, &begin, &end, &f_opa, 1, 0);
   if (var_n==-3) {
     abort_flag=1;
   }else{
     if (var_n<0) i_opval=1;
     else i_opval=end-begin+1;
     push_val(i_opval, (double) 0.0, 0);
   }
}

f_sgn()
{
   if (get_val_type(0)!=0) {
     get_act_val(&i_opa, &f_opa, 1);
     if (f_opa<0.0) i_opval= -1;
     else i_opval=1;
   }else{
     get_act_val(&i_opa, &f_opa, 1);
     if (i_opa<0) i_opval= -1;
     else i_opval=1;
   }
   push_val(i_opval, (double) 0.0 ,0);
}

f_row()
{
   get_act_val(&i_opa, &f_opa, 1);
   if (i_opa<1) {
     abort_flag=1;
   }else{
     mat_row=i_opa;
   }
   push_val(i_opval, (double) 0.0 ,0);
}

f_xmesh()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   i_opval=(mat_row-1)%i_opb;
   f_opval=i_opval/((double) i_opb-1.0);
   push_val(0, f_opval, 1);
}

f_ymesh()
{
   get_act_val(&i_opa, &f_opa, 1);
   get_act_val(&i_opb, &f_opb, 1);
   i_opval=(mat_row-1)/i_opb;
   f_opval=i_opval/((double) i_opa-1.0);
   push_val(0, f_opval, 1);
}

int (*math_op[])()={m_null, m_eql, m_l_or, m_l_and, m_gt_tn, m_ls_tn, m_c_eql,
                    m_null, m_null, m_null, m_c_almeql, m_add,
                    m_sub, m_tms, m_div, m_mod, m_exp, m_null,
                    m_null, m_neg, m_not};

int (*func_op[])()={f_sum, f_prod, f_avg, f_med, f_stddev, f_maxr,
                    f_minr, f_max, f_min, f_if, f_ln, f_log, f_exp,
                    f_sqrt, f_int, f_rnd, f_abs, f_sin, f_cos, f_tan,
                    f_asin, f_acos, f_atan2, f_atan, f_rand, f_exy,
                    f_pi, f_hypot, f_num, f_sgn, f_row, f_float,
                    f_xmesh, f_ymesh};

#ifdef MATHERR
int ne_dom, ne_sing, ne_over, ne_under;
#endif

compute_it(datas, start, end, step, parsed_stack, n_parsed)
struct sp_data *datas;
struct parse_stack parsed_stack[];
int start, end, step, n_parsed;
{
   int max, min;
   double calc_row();

#ifdef MATHERR
   char *ptr;
   ne_dom=ne_sing=ne_over=ne_under=0;
#endif

   min=start; max=end;
   if (end<start) {min=end; max=start;}
   for (mat_row=start;((mat_row>=min)&&(mat_row<=max));mat_row+=step) {
     (void) calc_row(datas, mat_row, parsed_stack, n_parsed);
   }

   math_err++;
#ifdef MATHERR
   if ((ne_dom==0)&&(ne_sing==0)&&(ne_over==0)&&(ne_under==0)) {
     (void) sprintf(tmpbuff,"   %i -> No calculation errors.\n",
                    math_err);
   }else{
     (void) sprintf(tmpbuff,"   %i -> Calculation errors:", math_err);
     ptr=tmpbuff+strlen(tmpbuff);
     if (ne_dom!=0) {
       (void) sprintf(ptr," %i domain,", ne_dom);
       ptr=ptr+strlen(ptr);
     }
     if (ne_sing!=0) {
       (void) sprintf(ptr," %i singularity,", ne_sing);
       ptr=ptr+strlen(ptr);
     }
     if (ne_over!=0) {
       (void) sprintf(ptr," %i overflow,", ne_over);
       ptr=ptr+strlen(ptr);
     }
     if (ne_under!=0) {
       (void) sprintf(ptr," %i underflow", ne_under);
       ptr=ptr+strlen(ptr);
     }
     if (*(ptr-1)==',') ptr--;
     (void) sprintf(ptr,".\n");
   }
#else
   (void) sprintf(tmpbuff,"   %i -> Calculation complete.\n", math_err);
#endif
   add_note(tmpbuff);
}

/* calc_row - calculate one row of the calculation */

double calc_row(datas, row_n, parsed_stack, n_parsed)
struct sp_data *datas;
struct parse_stack parsed_stack[];
int row_n, n_parsed;
{
   int i, tmp;
   double val;

   n_ops=0;
   loc_data=datas;
   mat_row=row_n;
   abort_flag=0;
   for (i=(n_parsed-1);i>=0;i--) {
     switch(parsed_stack[i].op) {
       case -1: if (parsed_stack[i].num.i==-1) {
                  push_val(mat_row, (double) 0.0, 0);
                }else{
                  push_var(parsed_stack[i].num.i, 0, -2);
                }
                break;
       case -2: get_act_val(&i_opa, &f_opa, 1);
                if (parsed_stack[i].num.i==-1) {
                  if (mat_row!=i_opa) {
                    abort_flag=1;
                  }else{
                    push_val(mat_row, (double) 0.0, 0);
                  }
                }else{
                  push_var(parsed_stack[i].num.i, i_opa, -1);
                }
                break;
       case -3: get_act_val(&i_opa, &f_opa, 1);
                get_act_val(&i_opb, &f_opb, 1);
                if (i_opa>i_opb) {
                  tmp=i_opa; i_opa=i_opb; i_opb=tmp;
                }
                if (parsed_stack[i].num.i==-1) {
                  if ((mat_row<i_opa)||(mat_row>i_opb)) {
                    abort_flag=1;
                  }else{
                    push_val(mat_row, (double) 0.0, 0);
                  }
                }else{
                  if (i_opb<0) {
                    abort_flag=1;
                  }else{
                    push_var(parsed_stack[i].num.i, i_opa, i_opb);
                  }
                }
                break;
       case -4: push_var(-3, parsed_stack[i].num.i, 0);
                break;
       case -5: push_val(0, parsed_stack[i].num.f, 1);
                break;
       case -6: push_val(parsed_stack[i].num.i, (double) 0.0, 0);
                break;
       case -7: if ((parsed_stack[i].num.i<0)||
                                  (parsed_stack[i].num.i>=N_FN)) {
                  (void) fprintf(stderr,"Fatal stack parsing error.\n");
                  (void) exit(1);
                }else{
                  (*func_op[parsed_stack[i].num.i])();
                }
                break;
       default: if ((parsed_stack[i].op<0)||(parsed_stack[i].op>=N_OP)) {
                   /* future redirection expansions here */
                }else{
                  (*math_op[parsed_stack[i].op])();
                } 
                break;
     }
     if (abort_flag!=0) break;
   }
   val=0.0;
   if (n_ops!=0) get_act_val(&tmp, &val, 0);
   return(val);
}

#ifdef MATHERR
/* matherr - included to access the ieeelib error routines
           - keeps track of occurences of numerical errors */

matherr(exc)
struct exception *exc;
{
   switch(exc->type) {
     case DOMAIN    : ne_dom++; 
                      break;
     case SING      : ne_sing++; 
                      break;
     case OVERFLOW  : ne_over++; 
                      break;
     case UNDERFLOW : ne_under++; 
                      break;
     default        : break;
   }
}
#endif
