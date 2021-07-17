/******************************************************************
                          sPLOTch!

  Spuds - sPLOTch! utility devices, such as command parsers,
    data routines, and all other coding which is not specific
    to one routine.
 
*******************************************************************/      

#define LOCALMEM
#include "splotch.h"
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include "spastic.h"
#include <time.h>

#ifdef EBUG
   extern FILE *deb_log;
   extern int debug_level;
#endif

extern char argbuff[], inbuff[], tmpbuff[];
extern struct axis_def *axes[20];
extern struct opt_def options;

/*   xalloc - allocates a memory segment, with error message and
              exit if malloc fails                                  */

double *xalloc(size, msg)
unsigned int size;
char *msg;
{
  double *ptr;

  /* (void) fprintf(stderr,"%s %i\n",msg,size); */
  ptr=(double *) malloc(size);

  if (ptr==(double *) NULL) {
    (void) fprintf(stderr, 
      "Memory allocation fault - %i bytes requested.\n", size);
    (void) fprintf(stderr, "%s\n",msg);
    (void) exit(1);
  }
 
  return(ptr);
}

/*   xrealloc - reallocates a memory segment, with error message */

double *xrealloc(ptr, size, msg)
unsigned int size;
char *ptr, *msg;
{
  double *n_ptr;

  n_ptr=(double *) realloc(ptr, size);

  if (n_ptr==(double *) NULL) {
    (void) fprintf(stderr, 
       "Memory Reallocation fault - %i bytes requested.\n", size);
    (void) fprintf(stderr, "%s\n",msg);
    (void) exit(1);
  }

  return(n_ptr);
}

/*  xfree - frees a memory block allocated by xalloc or xrealloc */

xfree(ptr)
char *ptr;
{
   if (ptr!=(char *) NULL) (void) free(ptr);
}

/*      yank - pull first continous set of letters from string
	     - updates com position                          */

yank(ptr,com,buff)
char *ptr,*buff;
int com[4];
{
   int i,j;
   char ch, next_non_white();

   i=com[1];
   j=0;
   ch=next_non_white(ptr, &i);
   com[3]=i;
   while (((*(buff+j++)=ch)!=' ')&&!iswht(ch)&&(ch!='\0')) {
      ch= *(ptr+(++i));
      if (j==999) sev_err(BUFLOAD);
   }
   *(buff+j-1)='\0';
   com[1]=i;
   com[2]=com[1]-com[3];
#ifdef EBUG
 if (debug_level&DBG_UTIL) {
   (void) fprintf(deb_log,"Yanked string :%s: %i-> %i\n",buff,com[3],com[1]);
   (void) fflush(deb_log);
 }
#endif
}

/*      getc_buff - pulls string component into buff from ptr, stopping
                      at specified character
                  - limited to length lim
		  - starts at position indicated by beg
		  - beg returned at end of line or after comma
                  - returns negative if no character found */

int getc_buff(ptr,beg,buff,lim,ch)
char *ptr,*buff,ch;
int *beg;
{
   int i,qu,k;

   *buff='\0';
   i=qu=0;
   k= *beg;
   while (((*(ptr+k+i)!=ch)||(qu!=0))&&(*(ptr+k+i)!='\0')) {
     if (i<lim) *(buff+i)= *(ptr+k+i);
     if (*(buff+i)=='\'') qu=1-qu;
     if (*(buff+i)=='\\') {
       i++;
       if (i<lim) *(buff+i)= *(ptr+k+i);
       if (*(ptr+k+i)=='\0') break;
     }
     i++;
   }	
   if (i<lim) *(buff+i)='\0';
   else *(buff+lim-1)='\0';
#ifdef EBUG
 if (debug_level&DBG_UTIL) {
   (void) fprintf(deb_log,"Character yanked string :%s:\n",buff);
   (void) fflush(deb_log);
 }
#endif
   if (*(ptr+k+i)==ch) {*beg=i+k+1; return(0);}
   *beg=i+k;
   return(-1);
}

/*   is_empty - returns true if string is empty */

int is_empty(ptr)
char *ptr;
{
   int i,fl;
   char next_non_white();

   i=fl=0;
   if (next_non_white(ptr, &i)=='\0') fl=1;
   return(fl);
}

/* is_numeric - returns true if string is numeric (can be fooled by e)
              - includes # as a character if fl non-zero */

int is_numeric(ptr, fl)
char *ptr;
int fl;
{
   int i, out;
   char ch;

   i=out=1;

   for (i=0;i<strlen(ptr);i++) {
     ch=clower(*(ptr+i));
     if ((ch!='-')&&(ch!='+')&&((ch<'0')||(ch>'9'))&&(ch!='e')&&
        (ch!='.')&&(!iswht(ch))&&(ch!=' ')) if ((ch!='#')||(fl==0)) out=0;
   }

   return(out);
}

/*  l_comp - returns non-zero if the lower case strings equal */

int l_comp(a,b)
char *a, *b;
{
   int i;
   char ch1, ch2;

   for (i=0;;i++) {
      ch1=clower(*(a+i));
      ch2=clower(*(b+i));
      if (ch1!=ch2) break;
      if (ch1=='\0') return(1);
   }
   return(0);
}

/* clean_string - removes all delimited characters from a string */

clean_string(buff)
char *buff;
{
   int i, j ;
   char ch;

   j=0;
   ch='a';
   for (i=0;ch!='\0';i++) {
     ch= *(buff+i);
     if (ch=='\\') {
       ch= *(buff+(++i));
     }
     *(buff+j++)=ch;
   }
}

/*  pull_coord - obtains a 3 dimensional coordinate from ptr
               - l is the error length
               - can skip coordinates above ncs       */

int pull_coord(lpos,ptr,l,coord,ncs)
int l,lpos,ncs;
char *ptr;
struct coord3d *coord;
{
   int i,rc,k;
   struct coord3d tcrd;

   k=0;
   tcrd= *coord;
   for (i=0;i<3;i++) {
     rc=getc_buff(ptr, &k, tmpbuff, 1000, ',');
     if (is_empty(tmpbuff)==0) {
       if (i==0) tcrd.x=atof(tmpbuff);
       else {
         if (i==1) tcrd.y=atof(tmpbuff);
         else tcrd.z=atof(tmpbuff);
       }
     }
     if (rc<0) {
       if (i<(ncs-1)) i= -1;
       break;
     }
   }

   if (i>0) {
     *coord=tcrd;
   }else{
     sp_err(THREEC,lpos,l);
   }

   return(i);
}

/* crdabs - takes the absolute value of a coordinate */

COORD crdabs(num)
COORD num;
{
   if (num<0) return(-num);
   else return(num);
}

/*  fint - returns the integer value associated with a
         - rounding values depends on dir value
	     -negative - always rounds down to smaller integer
	     -positive - always rounds up to larger integer
	     -zero - always rounds towards zero (normal int) */

int fint(a,dir)
double a;
int dir;
{
   int res;

   res=(int) a;
   if ((a<0.0)&&(a!=(float) res)&&(dir<0)) res=res-1;
   if ((a>0.0)&&(a!=(float) res)&&(dir>0)) res=res+1;
   return(res);
}

/* ran2 - a portable random number generator, as ripped straight from
            Numerical Recipes in C (sort of) */

#define M_R 714025
#define IA_R 1366
#define IC_R 150889

float ran2()
{
   static long iy, ir[98];
   static int idum, iff=0;
   time_t t_clock, time();
   int j;

   if (iff==0) {
     iff=1;
     t_clock=time((time_t *) 0);
     idum= -((int) t_clock&0xffffff);
     if ((idum=(IC_R-idum)%M_R)<0) idum= -idum;
     for (j=1;j<97;j++) {
       idum=(IA_R*idum+IC_R)%M_R;
       ir[j]=idum;
     }
     idum=(IA_R*idum+IC_R)%M_R;
     iy=idum;
   }
   j=1+97.0*iy/M_R;
   if ((j>97)||(j<1)) {
     (void) fprintf(stderr, "Internal error in random numbers...tell Jeff.\n");
     (void) exit(1);
   }
   iy=ir[j];
   idum=(IA_R*idum+IC_R)%M_R;
   ir[j]=idum;
   return(((float) iy)/M_R);
}



/* bisort - bubble sort the integer items in table
 	  - nent is the number of entries in the table
	  - does not check for equivalent values   */

bisort(table,nent)
int nent,*table;
{
   int fl,j,tmp;

   fl=1;
   while (fl==1) {
      fl=0;
      for (j=0;j<(nent-1);j++) {
         if (*(table+j)>*(table+j+1)) {
	   tmp= *(table+j);
	   *(table+j)= *(table+j+1);
	   *(table+j+1)=tmp;
	   fl=1;
         }
      }
   }
}

/* bfsort - bubble sort the float items in table
 	  - nent is the number of entries in the table
	  - does not check for equivalent values   */

bfsort(table,nent)
int nent;
double *table;
{
   int fl,j;
   double tmp;

   fl=1;
   while (fl==1) {
      fl=0;
      for (j=0;j<(nent-1);j++) {
         if (*(table+j)>*(table+j+1)) {
	   tmp= *(table+j);
	   *(table+j)= *(table+j+1);
	   *(table+j+1)=tmp;
	   fl=1;
         }
      }
   }
}

#ifdef SFE
#else
/* variables - processes the y(x) or y(x,y) plot point definition
	     - returns variable coords in crd
             - returns -1 if plot bad or else number of vars
		   (either 2 or 3) 
             - also determines existence of modifier variables
             - key determines numbering order of coordinates
                  (0 is lines, 1 is surfaces)

e.g. y(x)   -> 2d line [1,0]
     y(x,c) -> set of 2d lines [1,0,2]
     z(x,y) -> 3d surface, contour, or 3d line [2,0,1]

 all can be followed by [x1,...,x10] for modifiers         */

int variables(ptr, datas, crd, lpos, key)
int lpos, key;
struct var_set *crd;
struct sp_data *datas;
char *ptr;
{
   int p_open, p_close, p_comma, p_sq;
   int i,rc,rc2,ert,k,cnt,ok;
   char *ex_ptr;

   ert=0;
   crd->n_extra=0;
   crd->nrows= -1;
   crd->var_n[2]= -1;
   p_open=p_close=p_comma=p_sq= -1;
   for (i=0;((i<=strlen(ptr))&&(p_sq<0));i++) {
      if (*(ptr+i)=='(') p_open=i;
      if (*(ptr+i)==')') p_close=i;
      if (*(ptr+i)==',') p_comma=i;
      if (*(ptr+i)=='[') p_sq=i;
   }

   if ((p_open==-1)||(p_close<p_open)) {
      sp_err(NOFORM,lpos,(int) strlen(ptr));
      return(-1);
   }
   if ((p_comma!=-1)&&((p_comma<p_open)||(p_comma>p_close))) {
      sp_err(NOFORM,lpos,(int) strlen(ptr));
      return(-1);
   }
   if ((p_sq!=-1)&&(p_sq<p_close)) {
      sp_err(NOFORM,lpos,(int) strlen(ptr));
      return(-1);
   }

   *(ptr+p_open)='\0';
   if (p_open>15) *(ptr+15)='\0';
   rc=add_tab(ptr, datas, crd, (1+key));
   if (rc<0) {
     ert= -1;
     sp_err(UNKVAR,lpos,(int) strlen(ptr));
   }

   if (p_comma!=-1) {
     *(ptr+p_close)='\0';
     if ((p_close-p_comma)>16) *(ptr+p_comma+16)='\0';
     rc=add_tab((ptr+p_comma+1), datas, crd, (2-key));
     if (rc<0) {
       ert= -1;
       sp_err(UNKVAR,lpos,(int) strlen(ptr+p_comma+1));
     }
     p_close=p_comma;
   }

   *(ptr+p_close)='\0';
   if ((p_close-p_open)>16) *(ptr+p_open+16)='\0';
   rc=add_tab((ptr+p_open+1), datas, crd, 0);
   if (rc<0) {
     ert= -1;
     sp_err(UNKVAR,lpos,(int) strlen(ptr+p_open+1));
   }

   if (ert!=-1) {
     ert=2;
     if (p_comma!=-1) ert=3;
   }else{
     crd->nrows= -1;
   }

   if (p_sq!=-1) {
     ex_ptr=ptr+p_sq+1;
     for (i=0;i<strlen(ex_ptr);i++) {
       if (*(ex_ptr+i)==']') *(ex_ptr+i)='\0';
     }
     k=0;
     for (cnt=0;cnt<10;) {
       ok=k;
       rc=getc_buff(ex_ptr, &k, tmpbuff, 1000, ',');
       if (is_empty(tmpbuff)!=0) crd->extra_n[cnt]= -1;
       else{
         tmpbuff[15]='\0';
         rc2=add_tab(tmpbuff,datas,crd,(-cnt-1));
         if (rc2<0) {
           sp_err(UNKVAR,(lpos+p_sq+1+ok),(int) strlen(tmpbuff));
         }else{
           cnt++;
         }
       }
       if (rc<0) break;
     }
     crd->n_extra=cnt;
   }

   return(ert);
}

/* get_surface - obtains a surface definition, including the surface vars,
                 the hull definition (with holes!)
               - need_aux indicates a 4d surface definition
               - returns negative if bad definition */

int get_surface(datas, ptr, lpos, surface, need_aux)
struct sp_data *datas;
char *ptr;
int lpos;
struct surf_def *surface;
int need_aux;
{
   int com[4], rc, l, tmp, surf_done, tmp2;

   surf_done=com[1]=surface->hull_set=0;
   surface->ax_n= -1;
   surface->cl_flag=0;

   surface->colourlims=(struct sp_colour *) NULL;
   surface->ncolours=0;

   do {
     rc=scan_cmd(ptr, com, lpos, &l, &tmp, argbuff, 1);
     if ((rc<0)||((com[0]!=8)&&(com[0]!=63)&&(com[0]!=140)&&
            (com[0]!=158)&&(com[0]!=161)&&(com[0]!=0)&&(com[0]!=-2))) {
       if (rc>=0) com[1]=com[3];
       yank(ptr, com, argbuff);
       l=strlen(argbuff);
       if (surf_done==0) {
       rc=variables(argbuff, datas, &(surface->crd), (lpos+com[3]), 1);
       if (rc!=3) sp_err(NOTHREE, (lpos+com[3]), l);
       else {
         if ((need_aux!=0)&&(surface->crd.n_extra==0))
           sp_err(BADFOUR, (lpos+com[3]), l);
         else surf_done=1;
       }
       }else{
         sp_err(TWOSURF, (lpos+com[3]), l);
       }
     }else{
       switch(com[0]) {
         case 0:   break;
         case -2:  break;
         case 8:   tmp2=atoi(argbuff)-1;
                   if ((tmp2<0)||(tmp2>19)) {
                     sp_err(BADAXIS, tmp, l);
                     break;
                   }
                   if ((axes[tmp2]==(struct axis_def *) NULL)||
                        (axes[tmp2]->set_flag==0)) {
                      sp_err(NOAXIS, tmp, l);
                      break;
                   }else{
                      surface->ax_n=tmp2;
                   }
                   break;
         case 140: surface->hull_set=0;
                   rc=variables(argbuff, datas, &(surface->hull), tmp, 0);
                   if (rc!=-1) {
                     if (rc==3) {
                       sp_err(NOTHREE, tmp, l);
                     }else{
                       surface->hull_set=1;
                     }
                   }
                   break;
         case 158: get_clip(argbuff, surface->d_clips,
                     &(surface->cl_flag));
                   break;
         case 161: get_colourset(argbuff, tmp, &(surface->colourlims),
                       &(surface->ncolours));
                   break;
         default:  com[1]=com[3];
                   what(ILLCOM, ptr, com, lpos, 1);
                   break;
       }
     }
   }while (com[0]!=0);
   return(surf_done-1);
}

/* get_colourset - obtains the colourset list */

get_colourset(buff, lpos, list, nlist)
char *buff;
struct sp_colour **list;
int lpos, *nlist;
{
   int i,k,ct,rc,ok;

   ct=1;
   for (i=0;i<strlen(buff);i++) if (*(buff+i)==',') ct++;

   *list=(struct sp_colour *) xalloc((unsigned int)
           ((ct+1)*sizeof(struct sp_colour)),
           "Unable to allocate colourset storage area.");

   k=0;
   for (i=0;i<ct;i++) {
     ok=k;
     (void) getc_buff(buff, &k, inbuff, 1000, ',');
     set_colour((*list+i), options.linecolour, 7, 0);
     if (is_empty(inbuff)==0) {
       (void) get_colour(inbuff, (*list+i), &rc, &rc, (lpos+ok), 
                   strlen(inbuff), 0);
     }
   }

   *nlist=ct;
}

/* get_clip - returns the 4 clipping points, as well as the clip mask */

get_clip(buff, d_clips, cl_flag)
char *buff;
double d_clips[4];
int *cl_flag;
{
   int i, k, rc;
   static int clips[4]={CL_XMIN, CL_XMAX, CL_YMIN, CL_YMAX};

   k=0;
   for (i=0;i<4;i++) {
     rc=getc_buff(buff, &k, tmpbuff, 1000, ',');
     if (is_empty(tmpbuff)==0) {
       d_clips[i]=atof(tmpbuff);
       *cl_flag= *cl_flag|clips[i];
     }
     if (rc<0) break;
   }
}

/* search_path - searches the path set to find the file <name>
	       - opens file in direction dir, passing file pointer
	       - if keep is true, place final directory into dir_buff
	       - if search is true, check search path (start dir_buff)
	       - if search is false, go to dir_buff  
	       - returns file pointer to opended file */

static char dir_buff[1000]="";

FILE *search_path(name,dir,keep,search)
char *name,*dir;
int keep,search;
{
   char *t_buff,*paths;
   int i,j;
   FILE *fp;

   paths=getenv("SPLOTCH_PATHS");
   if (paths==NULL) {
     t_buff=(char *) xalloc((unsigned int) 
                   ((strlen(dir_buff)+10)*sizeof(char)),
                   "Unable to allocate storage for directory scan.");
     if (dir_buff[0]=='\0') {
       (void) sprintf(t_buff,":.:~");
     }else{
       (void) sprintf(t_buff,"%s::.:~",dir_buff);
     }
   }else{
     t_buff=(char *) xalloc((unsigned int) 
                   ((strlen(dir_buff)+strlen(paths)+10)*sizeof(char)),
                   "Unable to allocate storage for directory scan.");
     if (dir_buff[0]=='\0') {
       (void) sprintf(t_buff,":.:~:%s",paths);
     }else{
       (void) sprintf(t_buff,"%s::.:~:%s",dir_buff,paths);
     }
   }
   paths=t_buff;
   *(paths+strlen(t_buff)+1)='\0';

#ifdef EBUG
  if (debug_level&DBG_UTIL) {
     (void) fprintf(deb_log,"Search Path->%s\n",paths);
     (void) fflush(deb_log);
  }
#endif

   if (search!=0) {
      i=j=0;
      while (i<=strlen(paths)) {
         if ((*(paths+i)==':')||(*(paths+i)=='\0')) {
           *(paths+i)='\0';
	   if (i!=0) {
	     (void) sprintf(tmpbuff,"%s/%s",paths,name);
	   }else{
	     (void) sprintf(tmpbuff,"%s",name);
	   }
           expand_home(tmpbuff);

#ifdef EBUG
   if (debug_level&DBG_UTIL) {
      (void) fprintf(deb_log,"Trying:%s:\n",tmpbuff);
      (void) fflush(deb_log);
   }
#endif

	   fp=fopen(tmpbuff,dir);
	   if (fp!=NULL) break;
           else {
             paths=paths+i+1;
             i=0;
           }
         }
         i++;
      }

#ifdef EBUG
      if (debug_level&DBG_UTIL) {
	 if (fp==NULL) {
	   (void) fprintf(deb_log,"File unfound\n");
	 }else{
           (void) fprintf(deb_log,"Found file on path:%s:\n",tmpbuff);
	   (void) fflush(deb_log);
	 }
      }
#endif

      if ((keep!=0)&&(fp!=NULL)) {
	 for(i=strlen(tmpbuff);i>-1;i--) {
	    if (tmpbuff[i]=='/') break;
	 }
	 for(j=0;j<i;j++) dir_buff[j]=tmpbuff[j];
	 dir_buff[j]='\0';

#ifdef EBUG
   if (debug_level&DBG_UTIL) {
      (void) fprintf(deb_log,"Kept directory :%s:\n",dir_buff);
      (void) fflush(deb_log);
   }
#endif

      }
   }else{
      if (dir_buff[0]=='\0')  (void) sprintf(tmpbuff,"%s",name);
      else (void) sprintf(tmpbuff,"%s/%s",dir_buff,name);
      if (*dir!='r') {
        for (i=0;i<strlen(name);i++) {
          if (*(name+i)!=' ') break;
        }
        if ((*(name+i)=='/')||(*(name+i)=='~')) {
          (void) sprintf(tmpbuff,"%s",(name+i));
          expand_home(tmpbuff);
        }
      }

#ifdef EBUG
   if (debug_level&DBG_UTIL) {
      (void) fprintf(deb_log,"Directly searching :%s:, %s\n",tmpbuff,dir);
      (void) fflush(deb_log);
   }
#endif

      fp=fopen(tmpbuff,dir);
   }

   return(fp);
}

/* expand_home - replaces a leading tilde with the HOME directory */

static char *null_str="";

expand_home(buff)
char buff[];
{
   int k,l;
   char *home;

   home=getenv("HOME");
   if (home==(char *) NULL) home=null_str;

   for (k=0;k<strlen(buff);k++) {
     if ((buff[k]!=' ')&&(!iswht(buff[k]))) {
       if (buff[k]=='~') {
         for (l=strlen(buff);l>k;l--) {
           buff[l+strlen(home)-1]=buff[l];
         }
         for (l=0;l<strlen(home);l++) {
           buff[k+l]= *(home+l);
         }
       }
       break;
     }
   }
}

/*   print_path - outputs the currently stored directory
		- used for information about log writing errors */

print_path(fp)
FILE *fp;
{
   (void) fprintf(fp,"%s",dir_buff);
}
#endif

unsigned char fast_rot[8]={0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

/* init_bit_map, set_bit, get_bit - handle data "bitmaps" */

init_bit_map(bits, n_bits)
unsigned char **bits;
unsigned int n_bits;
{
   *bits=(unsigned char *) xalloc((unsigned int) 
                    ((n_bits/8+10)*sizeof(unsigned char)),
                    "Unable to allocate data bitmap.");
   clear_bit_map(*bits, n_bits);
}

/* clear_bit_map - clear it */

clear_bit_map(bits, n_bits)
unsigned char *bits;
unsigned int n_bits;
{
   int i;

   for (i=0;i<(n_bits/8+10);i++) {
     *(bits+i)=(unsigned char) 0;
   }
}

/* set_bit - set a specific bit of the map to the data value */

set_bit(bits, n_bits, i, data)
unsigned char *bits;
unsigned int n_bits;
int i, data;
{
   int index;
   unsigned char mask;

   if ((i<0)||(i>=n_bits)) return;
   index=((unsigned int) i)>>3;
   /* mask=1<<(i-index*8); */
   mask=fast_rot[((unsigned int) i)&0x07];

   if (data==0) {
     *(bits+index)= *(bits+index)&(~mask);
   }else{
     *(bits+index)= *(bits+index)|mask;
   }
}

/* get_bit - returns the setting of the specified bit */

int get_bit(bits, n_bits, i)
unsigned char *bits;
unsigned int n_bits;
int i;
{
   int index;
   unsigned char mask;

   if ((i<0)||(i>=n_bits)) return(0);
   index=((unsigned int) i)>>3;
   /* mask=1<<(i-index*8); */
   mask=fast_rot[((unsigned int) i)&0x07];

   if ((*(bits+index)&mask)!=0) return(1);
   else return(0);
}
