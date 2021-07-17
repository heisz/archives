/******************************************************************
                          sPLOTch!

  Sprocket - the command buffer controller...all command processing,
             argument loading and line reading is done through here,
             including macro expansion and file buffering
 
*******************************************************************/      

#define NEEDTIME
#include "splotch.h"
#include <stdio.h>
#include <ctype.h>
#include "spastic.h"
#include "version.h"
#include "time.h"

#ifdef EBUG
extern FILE *deb_log;
extern int debug_level;
#endif

extern union data_t_d sing_val[];
extern int sing_type[];

int flow_nest[40], flow_loop[40], flow_file[40], curr_flow;
char *flow_strs[40];

#define BUFF_CHUNK 20

char *line_buffer;  /* the address of the input buffer */
int lpos_no; /* file position of the first character in line_buffer*/
int lb_ln, lb_size; /* no of chars, maximum length in line_buffer */
static char *bf_msg="Unable to reallocate line buffer memory.";
static char *lp_msg="Unable to reallocate temporary loop storage.";
static char *loop_msg="Unable to allocate temporary loop storage.";

extern char **s_argv, argbuff[], tmpbuff[];
extern int s_argc;

FILE *in_file;
FILE *file_tree[20];
int n_file_tree=0, the_end=0;
char *buff_tree[20], *rb_tree[20];

/* readback buffer stuff - for looping feedback */

char *rb_ptr;
int rb_curr=0, rb_end=0, rb_max= -1;

/* f_getc, f_eof - custom file readers to handle multiple data entry
                   points (ie. interactive and readback data buffers) */

static char f_getc(file)
FILE *file;
{
   char ch;

   if (rb_end!=0) {
     ch= *(rb_ptr+rb_curr++);
     if (ch!='\0') return(ch);
     else {
       rb_curr=rb_end=0;
       *(rb_ptr)='\0';
     }
   }

   if (file==(FILE *) NULL) return(EOF);
   return(fgetc(file));
}

static int f_eof(file)
FILE *file;
{
   if (rb_end!=0) return(0);
   if (file==(FILE *) NULL) return(1);
   return(feof(file));
}


/* init_line_buff - initialize the input buffer to BUFF_CHUNK chars
                       at position pos */

init_line_buff(pos)
int pos;
{
   lpos_no=pos;
   lb_ln=0;
   lb_size=BUFF_CHUNK;
   line_buffer=(char *) xalloc((unsigned int) (lb_size*sizeof(char)),
                      "Unable to allocate storage for line buffer.");
   *line_buffer='\0';
}

/* add_line_buff - adds lines of text to the line buff an end-of-line or a
                     semi-colon is encountered
                 - increases line buffer storage if required
                 - returns line_buffer in case of move ...
                 - automatically handles previous line buffers from inputs */

char *add_line_buff()
{
   char ch;
   int i, l, old_ln, exit_fl;
  
   if (the_end!=0) return(line_buffer);

   old_ln=lb_ln;
   exit_fl=0;    /* superfluous */
   while (exit_fl==0) {
     while ((((ch=f_getc(in_file))!='\n')&&(ch!=';')&&(f_eof(in_file)==0))) {
       while (lb_ln>(lb_size-5)) inc_line_buff();
       *(line_buffer+lb_ln++)=ch;
     }
     if (f_eof(in_file)!=0) {
#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"End of file encountered\n");
     (void) fflush(deb_log);
   }
#endif
       if (n_file_tree>1) {
         n_file_tree--;
         *(line_buffer+lb_ln)='\0';
         parse_macros(old_ln);
         while ((strlen(buff_tree[n_file_tree-1])+lb_ln)>(lb_size-5)) {
           inc_line_buff();
         }
         (void) strcat(line_buffer, buff_tree[n_file_tree-1]);
         xfree(buff_tree[n_file_tree-1]);
         if (rb_tree[n_file_tree-1]!=(char *) NULL) {
           (void) strcat(rb_ptr, rb_tree[n_file_tree-1]);
           rb_curr=0;
           rb_end=strlen(rb_ptr);
           xfree(rb_tree[n_file_tree-1]);
         }
         old_ln=lb_ln=strlen(line_buffer);
         if (in_file!=(FILE *) NULL) (void) fclose(in_file);
         in_file=file_tree[n_file_tree-1];
         exit_fl=1;
       }else{
         the_end=1;
         exit_fl=1;
       }
     }else{
       if (ch=='\n') *(line_buffer+lb_ln++)='\n';
       else if (ch==';') *(line_buffer+lb_ln++)=';';
       else (void) fprintf(stderr, "Input buffer malfunction...tell Jeff.\n");
       exit_fl=1;
     }
   }
   *(line_buffer+lb_ln)='\0';
#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"Add_line_buff:%s:\n",line_buffer);
     (void) fflush(deb_log);
   }
#endif

   for (i=(curr_flow-1);i>=0;i--) {
     if (flow_loop[i]!=0) {
       if (flow_loop[i]==-1) break;
       if (flow_file[i]!=n_file_tree) continue;
       l=strlen(flow_strs[i]);
       if ((l+lb_ln-old_ln+5)>flow_loop[i]) {
         flow_loop[i]=l+lb_ln-old_ln+5;
         flow_strs[i]=(char *) xrealloc((char *) flow_strs[i],
            (unsigned int) (flow_loop[i]*sizeof(char)), lp_msg);
       }
       (void) strcat(flow_strs[i], (line_buffer+old_ln));
     }
   }

   parse_macros(old_ln);
   return(line_buffer);
}

/* init_loop - initializes the buffer storage table for loop copying */

init_loop(num)
int num;
{
   flow_loop[num]=BUFF_CHUNK;
   flow_file[num]=n_file_tree;
   flow_strs[num]=(char *) xalloc(
        (unsigned int) (BUFF_CHUNK*sizeof(char)), loop_msg);
   *flow_strs[num]='\n';
   *(flow_strs[num]+1)='\0';
}

/*  add_readback - adds a command string to the readback buffer
                     (from looping routines) */

add_readback(buff)
char *buff;
{
   int i;

   if (rb_max<0) {
     rb_max=BUFF_CHUNK;
     rb_ptr=(char *) xalloc((unsigned int) (BUFF_CHUNK*sizeof(char)),
                        "Unable to allocate readback buffer storage.");
     rb_end=rb_curr=0;
     *rb_ptr='\0';
   }

   if (rb_end!=0) {
     for (i=0;i<=(rb_end-rb_curr);i++) {
       *(rb_ptr+i)= *(rb_ptr+i+rb_curr);
     }
   }else *(rb_ptr)='\0';
   rb_end-=rb_curr;

   rb_curr=strlen(buff);
   if ((rb_end+rb_curr+5)>rb_max) {
     rb_max=rb_end+rb_curr+5;
     rb_ptr=(char *) xrealloc((char *) rb_ptr,
               (unsigned int) (rb_max*sizeof(char)),
               "Unable to reallocate readback buffer storage.");
   }

   for (i=rb_end;i>=0;i--) {
     *(rb_ptr+i+rb_curr)= *(rb_ptr+i);
   }
   for (i=0;i<rb_curr;i++) {
     *(rb_ptr+i)= *(buff+i);
   }
   rb_curr=0;
   rb_end=strlen(rb_ptr);
}

/*  direct_add_buff - directly adds the indicated string to the input buffer
                    - used for interactive mode inputs 
                    - note that all macros in the line are processed at once
                       (so subsequent redefinitions will fail in same line)*/

direct_add_buff(buff)
char *buff;
{
   int old_ln;

   old_ln=lb_ln;
   if ((lb_ln+strlen(buff))>(lb_size-5)) {
     lb_size=lb_ln+strlen(buff)+5;
     line_buffer=(char *) xrealloc((char *) line_buffer, 
                      (unsigned int) (lb_size*sizeof(char)), bf_msg);
   }
   (void) strcat(line_buffer, buff);
   lb_ln=strlen(line_buffer);
   parse_macros(old_ln);
   the_end=0;
}

/* inc_line_buff - expand the input buffer storage by BUFF_CHUNK */

inc_line_buff()
{
  lb_size=lb_size+BUFF_CHUNK;
  line_buffer=(char *) xrealloc((char *) line_buffer, 
                      (unsigned int) (lb_size*sizeof(char)), bf_msg);
}

/* num_files - returns the file tree size (for external overflows) */

int num_files()
{
   return(n_file_tree);
}

/*  new_file - adds a new input file to the tree
             - buffer must be flushed to end of input command
             - returns doing nothing if tree too deep */

new_file(fp)
FILE *fp;
{
   char *add_line_buff(), *copy_buff();

   if (n_file_tree==19) return;
   if (n_file_tree!=0) {
     buff_tree[n_file_tree-1]=line_buffer;
     if (rb_max<0) rb_tree[n_file_tree-1]=(char *) NULL;
     else {
       rb_tree[n_file_tree-1]=copy_buff(rb_ptr+rb_curr);
       rb_end=rb_curr=0;
       *rb_ptr='\0';
     }
   }
   init_line_buff(lpos_no);
   file_tree[n_file_tree++]=fp;
   in_file=fp;
   the_end=0;
   (void) add_line_buff();
}

/* line_buff_flush - flushes pos characters from the front of the line
                         buffer
                   - updates lpos_no and handles error channeling 
                         (if err_fl non-zero)*/

line_buff_flush(pos, err_fl)
int pos, err_fl;
{
   int i;

#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"Line_buff_flush: %i\n",pos);
     (void) fflush(deb_log);
   }
#endif
   if (pos>lb_ln) {
     (void) fprintf(stderr,"Flush error! Please tell Jeff.%i %i\n",pos,lb_ln);
     pos=lb_ln;
   }

   if (err_fl!=0) {
     err_flush(pos);
     lpos_no=lpos_no+pos;
   }

   for (i=0;i<(lb_ln-pos+2);i++) {
     *(line_buffer+i)= *(line_buffer+pos+i);
   }
   lb_ln=lb_ln-pos;
}

/* read_data_line - reads a data line from the line_buffer
                  - delimited line feeds are dropped
                  - discards blank lines
                  - meant for doodle and local readers */

read_data_line(buff,lim)
char *buff;
int lim;
{
   int pos,i;
   char *add_line_buff();

   *buff='\0';
   while ((is_empty(buff)!=0)&&(end_of_prog()==0)) {
     i=pos=0;
     while (i<lim) {
       if ((pos>=(lb_ln-3))&&(the_end==0)) {
         line_buff_flush(pos,0);
         pos=0;
         (void) add_line_buff();
         continue;
       }
       if ((*(line_buffer+pos)=='\\')&&(*(line_buffer+pos+1)=='\n')) {
         *(buff+i++)=' ';
         pos+=2;
         continue;
       }
       if ((*(line_buffer+pos)=='\n')||(*(line_buffer+pos)=='\0')) break;
       *(buff+i++)= *(line_buffer+pos++);
     }
     if (i==lim) {
       (void) fprintf(stderr,"Data line overflow (limit %i char).\n",lim);
       (void) exit(1);
     }
     if (*(line_buffer+pos)=='\n') pos++;
     line_buff_flush(pos,0);
     *(buff+i)='\0';
   }
#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"Read_data_line :%s:\n",buff);
     (void) fflush(deb_log);
   }
#endif
}

/* end_of_prog - returns non-zero if all inout streams/buffers finished */

int end_of_prog()
{
   if ((the_end!=0)&&(lb_ln==0)) return(1);
   return(0);
}

/* next_non_white - gets the next non-white character from the string
                  - returns character and position of it ( in k) */

char next_non_white(ptr, k)
char *ptr;
int *k;
{
  char n_non_white_str();

  return(n_non_white_str(&ptr, 0, k));
}

/* n_non_white_str - gets the next non-white character from the stream/string
                   - adds data if in_fl non-zero (and possible adjusts str)
                   - returns character and position of it ( in k) */

char n_non_white_str(ptr, in_fl, k)
char **ptr;
int in_fl, *k;
{
  int i;
  char ch, *add_line_buff();

  i= *k;

  while (((ch= *(*ptr+i))==' ')||iswht(ch)||(ch=='\0')) {
    if (ch=='\0') {
      if ((in_fl!=0)&&(the_end==0)) {
        *ptr=add_line_buff();
      }else break;
    }else i++;
  }
  
  *k=i;
  return(ch);
}

/*        what - error handling routine for unrecognizable or
		 incorrect command                   
               - scans string for space, = or ( 
	       - if = or (, retrieve argument (but do nothing)  
               - reports error if err_rpt non-zero */

what(errt,ptr,com,lpos,err_rpt)
char *ptr;
int com[4],errt,lpos; 
{
   int i,j,l,nc,pp,in_fl;
   char ch, n_non_white_str(), *add_line_buff();

   nc=in_fl=0;
   if (ptr==(char *) NULL) {
     ptr=line_buffer;
     lpos=lpos_no;
     in_fl=1;
   }

   i=j=l=0;
   while(j==0) {
      ch= *(ptr+com[1]+(i++));
      if ((ch=='=')||(ch=='(')) j=1;
      if ((ch==' ')||iswht(ch)||(ch==';')) j=2; 
      if (ch=='\0') {
        if ((in_fl!=0)&&(the_end==0)) {
          ptr=add_line_buff();
          i--;
        }else j=3;
      }
   }
   i-=1;
   if (j==2) {
      pp=com[1]+i;
      ch=n_non_white_str(&ptr, in_fl, &pp);
      if ((ch=='=')||(ch=='(')) j=1;
   }
   if (in_fl!=0) ptr=(char *) NULL;
   if (j==1) nc=getarg(ptr,(com[1]+i),lpos,argbuff,1,&l);
   if (err_rpt!=0) sp_err(errt,(com[1]+lpos),i);
   com[1]=com[1]+i+nc;
#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"What -> type %i, %i %i\n",j,nc,com[1]);
     (void) fflush(deb_log);
   }
#endif
}

/*      getarg - get an argument for font, angle, etc...
	       - feed in string ptr, and location of input
	       - returns a null term string and number of char used 
               - returns negative if bad argument
	       - negative length given if bracketed
	       - takes one continuos string after = or
	       - all enclosed char between (), spaces removed when
		   flag is non-zero (all chars if not) */

int getarg(ptr,loc,lpos,buff,flag,l)
char *ptr,*buff;
int loc,lpos,flag,*l;
{ 
  int i,j,bracnt,qu,in_fl,nc;
  char ch, n_non_white_str();

  in_fl=0;
  if (ptr==(char *) NULL) {
    ptr=line_buffer;
    lpos=lpos_no;
    in_fl=1;
  }

  *buff='\0';
  i=loc;
  ch=n_non_white_str(&ptr, in_fl, &i);

  nc=j=0;
  switch (ch) {
     case '=' : i++;
                ch=n_non_white_str(&ptr, in_fl, &i);
		if (ch!='(') {
                   qu=0;
		   while ((((ch= *(ptr+i))!=' ')&&(!iswht(ch)))||(qu!=0)) {
                     if ((ch==';')&&(qu==0)) break;
                     if (ch=='\'') qu=1-qu;
                     if (ch=='\\') {
                       nc++;
                       *(buff+(j++))=ch;
                       ch= *(ptr+(++i));
                     }
		     if (ch=='\0') {
                       if ((the_end==0)&&(in_fl!=0)) {
                         ptr=add_line_buff();
                       }else break;
                     }else{
                       nc++;
		       if (!iscntrl(ch)) *(buff+(j++))=ch;
                       i++;
                       
	               if (j==999) {
                         line_buff_flush((lpos+i-1), 1);
                         sev_err(BUFLOAD);
                       }
                     }
		   }
   		   break;
		}
     case '(' : bracnt=1;
                qu=0;
		while (bracnt!=0) {
                   ch= *(ptr+(++i));
		   if ((ch=='(')&&(qu==0)) bracnt+=1;
		   if ((ch==')')&&(qu==0)) bracnt-=1;
                   if (ch=='\'') qu=1-qu;
                   if (ch=='\\') {
                     nc--;
                     *(buff+(j++))=ch;
                     ch= *(ptr+(++i));
                   }
		   if (ch=='\0') {
                     if ((the_end==0)&&(in_fl!=0)) {
                       ptr=add_line_buff();
                       i--;
                     }else {nc++; break;}
                   }else{
                     nc--;
		     if (!((ch==' ')||iswht(ch))||(flag==0)) {
		       if (!iscntrl(ch)||iswht(ch)) *(buff+(j++))=ch;
                       *(buff+j)='\0';
		     }
                   }
	           if (j==999) {
                     line_buff_flush((loc+i-1), 1);
                     sev_err(BUFLOAD);
                   }
		}
		if (ch==')') { 
		   i++; j--; nc++;
                }
		break;
     default  : i= -1;
		break;
  }
  *(buff+j)='\0';
#ifdef EBUG 
 if (debug_level&DBG_INPT) {
   (void) fprintf(deb_log,"Get_arg-> :%s: Length: %i Nchar: %i\n",
             buff,nc,(i-loc));
   (void) fflush(deb_log);
 }
#endif
  *l=nc;
  return(i-loc);
}

/*        scan_cmd - scan for a command at the location of ptr
               - returns negative if unsuccessful, 0 if no argument,
                   positive otherwise
               - com[0] gives command sequence number
               - com[1] returns new command line position
	       - com[2] is number of characters used 
               - com[3] is start position (basically com[1]-com[2])
	       - automatically scans the argument, returning buff,
		   l and tmp, the beginning of the argument in buff
               - NOTE: more than just the text commands may be here
               - argument errors reported only if err_rpt non-zero */

/* i.e. if command were move=(3 in), com[0] would be one, com[1]
     would point to the comma, com[2] would be 11, tmp would point 
     at the 3, l would be 4 and buff would contain '3 in'   */

char *coms[]= {"Move","HULL","Angle","Rotate","Height","Font",
        "Slant","Underline","AXIS","MAJor","MINor","IN","OUT",
	"Number","NOne","VALue","LABel","ORDer","FROM","TO","STEP",
	"BY","BEGin","END","HI","NOTE","DATA","FILE","INPut","PLOT",
	"FRAME","HAXIS","VAXIS","OFFset","TITLE","OPTions","BORDer",
	"EDGE","FOOTnote","LENgth","ORIGin","VARs","4Tick","NO4tick",
	"ECHO","SYMbol","CHar","Line","Inter","Draw","DEVice",
	"HSIZE","VSIZE","UNIT","REPeat","VREF","HREF","HMAJGRID",
	"HMINGRID","LEFT","RIGHT","CENTer","CENTre","SKIP","SUP","SUB",
  /**/  "CONTour","XXX","RVARs","RVAXIS","PRint","NOPRint","DFONT",
	"ASPECT","HASIZE","VASIZE","OPENSCREEN","PLOTOVER","LOG",
	"GAPA","GAPB","CLIST","VLIST","HISTogram","RANge","OUTPUT",
	"POSTSCRIPT","SIZE","EXP","SORT","PROCess","TYPE","ERRbar",
        "RVREF","VMAJGRID","VMINGRID","DEFINE","MODE3D","MODE2D","EYEball",
        "SCREEN","2DORIGin","2DVECX","2DVECY","2DSIZE","VERTICAL",
        "PLOT3D","3DSIZE","3DORIGin","XAXis","YAXis","ZAXis",
        "EULEReye","LAYout","SIDES","WINFRACT","STRht","LINEWIDTH",
        "DOODLE","[","]","DELete","CALCulate","NEWvar","CLOSEscreen",
        "BYE","QUIT","Width","COLour","PATtern","RMove","RDraw","{","}",
  /**/  "DUMP", "COMPute","EXTEND","START","FILL","SPECial","GORIGin",
        "VGSIZE", "HGSIZE","$#","$$","$%","CLEAR","CLIP","BOUNDary",
        "HLENgth", "VLENgth", "RVLENgth", "HORIGin", "VORIGin", "RVORIGin",
        "POLAR","ECCentricity","HALIGN","VALIGN","RVALIGN","SPLOTCH",
        "ARRow","ARROWSIZE","NULL","DATACLIP","SCATTER","SURFTYPE",
        "COLOURSET", "IF", "ELSE", "ENDIF", "SYSTEM", "PRECision",
        "LINECOLour","DOODLE3D", "RESolution", "MARGin", "DO", "WHILE",
        "FOREACH", "FOR", "ENDFOR","PROJection","SURFace", "4DSURFace",
        "HIDden","4DCONTour","GRID", "FILTER"};

/* in the following array, combine 1 if argument needed and 2 if
        argument can be shrunk, 4 if argument possible */

char arg_fl[]= {3,1,3,3,3,3,
	3,0,3,1,1,0,0,
	3,0,1,1,1,6,6,6,
	6,6,6,0,6,6,1,1,6,
	4,3,3,3,6,0,4,
	3,6,3,3,1,6,0,
	1,6,1,1,3,3,1,
	3,3,3,3,1,1,1,
	1,0,0,0,0,3,1,1,
  /**/  1,1,1,3,0,0,3,
	3,3,3,0,6,6,
	3,3,1,3,6,3,1,
	0,3,6,3,1,3,6,
        1,1,1,0,0,0,3,
        3,3,3,3,3,3,
        6,3,3,3,3,3,
        3,3,3,3,3,3,
        0,0,0,1,6,1,0,
        0,0,3,3,3,3,3,0,0,
  /**/  6,1,6,6,1,3,3,
        3,3,4,4,4,0,3,3,
        3,3,3,3,3,3,
        6,3,3,3,3,0,
        3,3,0,3,1,3,
        1,1,0,0,1,3,
        3,0,3,3,0,1,
        1,3,0,3,1,1,
        3,1,3,3};

short comm[]= {1,140,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	20,20,18,19,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
	36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,
	56,57,58,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,
	74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,
        94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,
        110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
        125,126,127,128,129,130,131,132,133,134,135,136,137,138,
        -10,-11,-12,139,141,142,143,144,145,146,147,148,149,150,
        151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,
        166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,
        181,182};

int ncomm=189;

#ifdef EBUG
marktab()
{
   int i;
   FILE *fp;

   (void) fprintf(stderr,"Dumping command table...");
   fp=fopen("TABLE","w");
   if (fp==(FILE *) NULL) {
      (void) fprintf(stderr,"\nUnable to open debug TABLE file.\n");
      (void) exit(1);
   }
   for (i=0;i<ncomm;i++) {
     (void) fprintf(fp,"%i:%i:%s:-> ",i,comm[i],coms[i]);
     if (arg_fl[i]!=0) {
       if (arg_fl[i]&1) (void) fprintf(fp,"Needs argument");
       else (void) fprintf(fp,"May have argument");
       if ((arg_fl[i]&2)!=0) (void) fprintf(fp," (shrinkable)");
       else (void) fprintf(fp," (fixed)");
     }else (void) fprintf(fp,"No explicit argument");
     (void) fprintf(fp,"\n");
     (void) fflush(fp);
   }
   (void) fclose(fp);
   (void) fprintf(stderr,"Done\n");
}
#endif

int scan_cmd(ptr,com,lpos,l,tmp,buff,err_rpt)
char *ptr,*buff;
int com[4],lpos,*l,*tmp,err_rpt;
{
  int i, j, cpos2, inc, in_fl, nc;
  char ch, ch2, ch3, *ptemp, n_non_white_str(), *add_line_buff();

  in_fl=0;
  if (ptr==(char *) NULL) {
    ptr=line_buffer;
    lpos=lpos_no;
    in_fl=1;
  }

  com[3]=com[1];
  cpos2=0;

  ch=n_non_white_str(&ptr, in_fl, &(com[3]));

  if (ch=='\0') {
     com[0]=0;
     com[1]=com[3];
#ifdef EBUG
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"EOF MARKER\n");
    (void) fflush(deb_log);
  }
#endif
     return(0);
  }
  if (ch==';') {
     com[0]=0;
     com[1]=com[3]+1;
#ifdef EBUG
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"TERMINATION MARKER\n");
    (void) fflush(deb_log);
  }
#endif
     return(0);
  }

  if (ch=='\'') {
     com[0]= -1;
     i=0;
     j=1;
     while (ch!='\0'){
        ch= *(ptr+com[3]+j++);
	if (ch=='\''){
	   *(buff+i)='\0';
	   com[1]=com[3]+j;
           com[2]=strlen(buff)+2;
           *l=j-1;
           *tmp=lpos+com[3]+1;
	   j=0;
	   break;
	}
	if (ch=='\\'){
	   ch= *(ptr+com[3]+(j++));
	}
        if (ch=='\0') {
           if ((the_end==0)&&(in_fl!=0)) {
             ptr=add_line_buff();
             j--;
             ch=' ';
           }
        }else{
  	  *(buff+i++)=ch;
	  if (i==999) {
            line_buff_flush((lpos+i-1), 1);
            sev_err(BUFLOAD);
          }
        }
     }
     if (j!=0) {
        *(buff+i)='\0';
	if (err_rpt!=0) sp_err(BADEND,(com[3]+lpos+j-3),1);
	com[1]=com[3]+j-1;
        com[2]=strlen(buff)+2;
        *l=j-2;
        *tmp=lpos+com[3]+1;
     }
#ifdef EBUG
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"Text :%s:\n",buff);
    (void) fflush(deb_log);
  }
#endif
     return(1);
  }

  if ((ch=='/')&&(*(ptr+com[3]+1)=='*')) {
    com[0]= -2;
    cpos2=com[3];
    do {
      ptemp=ptr+cpos2++;
      if ((*ptemp=='\0')&&(the_end==0)&&(in_fl!=0)) {
        ptr=add_line_buff();
        ptemp=ptr+(cpos2-1);
      }
    }while (((*ptemp!='*')||(*(ptemp+1)!='/'))&&(*ptemp!='\0'));
    com[1]=cpos2+1;
    if(*ptemp=='\0') com[1]=cpos2-1;
    return(0);
  }

#ifdef EBUG   
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"Looking at :%s: -> %i:%i:%i:%i\n",(ptr+com[3]),
        com[0],com[1],com[2],com[3]);
    (void) fflush(deb_log);
  }
#endif
  for(i=0;i<ncomm;i++) {
     inc=0;
     for(cpos2=0;cpos2<20;cpos2++) {
        ch2=clower(*(ptr+com[3]+cpos2+inc));
	if ((ch2==' ')||(ch2=='=')||(ch2=='(')||(ch2=='\0')||
           iswht(ch2)||(ch2==';')) {
	   if (islower(*(coms[i]+cpos2))||(*(coms[i]+cpos2)=='\0')) {
	      com[0]=comm[i];
	      com[1]=com[3]+cpos2+inc;
	      com[2]=cpos2+inc;
#ifdef EBUG   
  if (debug_level&DBG_INPT) {
    (void) fprintf(deb_log,"Command: %s -> %i:%i:%i:%i\n",coms[i],
             com[0],com[1],com[2],com[3]);
    (void) fflush(deb_log);
  }
#endif
              nc=0;
	      if (arg_fl[i]!=0) {
                 if (in_fl!=0) ptr=(char *) NULL;
   	         nc=getarg(ptr,com[1],lpos,buff,(int) (arg_fl[i]&2),l);
                 if (in_fl!=0) ptr=line_buffer;
		 if (nc<0) {
		   *tmp=lpos+com[3];
                   *l=com[2];
                   if ((arg_fl[i]&4)!=0) {
                     nc=0;
                   }else{
		     if (err_rpt!=0) sp_err(MISSARG,*tmp,*l);
		     com[0]= -2;
                     nc=0;
                   }
		 }else{
	           com[1]=com[1]+nc;
                   com[2]=com[2]+nc;
		   *tmp=lpos+com[1]-*l;
		   if (*l<=0) {
                     *l= -*l;
		     *tmp=lpos+com[1]-*l-1;
                   }
		 }
	      }

	      if (com[0]==21) {
		 if (err_rpt!=0) sp_err(HI,(lpos+com[3]),2);
		 com[0]= -2;
	      }
	      return(nc);
	   }
	}
	if (iswht(ch2)) {
	   inc++;
	   cpos2--;
	}else{
	   ch3=clower(*(coms[i]+cpos2));
	   if (ch2!=ch3) break;
	}
     }
  }

  com[0]= -1;
  com[1]=com[3];
  return(-1);

}

/* init_macros - initializes the macro table
               - defines $name, $date, $time and $version variables */

#define N_PRE 5
char *mac_name[100], *mac_def[100], time_buff[20], date_buff[50],
     vers_buff[10], arc_buff[20];
int n_macs;
char *mac_spec[N_PRE]={"name", "date", "time", "version", "argc"};
char *months[12]={"January", "February", "March", "April", "May",
                  "June", "July", "August", "September", "October",
                  "November", "December"};
char *weekday[7]={"Sunday", "Monday", "Tuesday", "Wednesday", 
                  "Thursday", "Friday", "Saturday"};

init_macros()
{
   int i;
   time_t t_clock;
   struct tm *tmedat;

   n_macs=N_PRE;
   for (i=0;i<n_macs;i++) mac_name[i]=mac_spec[i];
   
   mac_def[1]=date_buff;
   mac_def[2]=time_buff;
   mac_def[3]=vers_buff;
   mac_def[4]=arc_buff;

   t_clock=time((time_t *) 0);
   tmedat=localtime(&t_clock); 

   (void) sprintf(time_buff,"%i:%02i:%02i",tmedat->tm_hour,tmedat->tm_min,
         tmedat->tm_sec);
   (void) sprintf(date_buff,"%s %s %i, 19%i", weekday[tmedat->tm_wday],
         months[tmedat->tm_mon], tmedat->tm_mday, tmedat->tm_year);
   (void) sprintf(vers_buff,"%i.%i",MAJ_VERS_NO, MIN_VERS_NO);
}

more_macros(name)
char *name;
{
   mac_def[0]=name;
   (void) sprintf(arc_buff,"%i",s_argc);
}

/* parse_macros - scans the input buffer from position start,
                    substituting macros*/

parse_macros(start)
int start;
{
   int i, j, st_pt, num_fl, del, n_subst, count;
   char ch, *ptr;

#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"Parse_macros :%s:\n",(line_buffer+start));
     (void) fflush(deb_log);
   }
#endif
   count=0;
   n_subst=1;
   while (n_subst!=0) {
     count++;
     if (count==20) break;
     n_subst=0;
     i=start;
     while (*(line_buffer+i)!='\0') {

       if (*(line_buffer+i)=='\\') i++;
       else if (*(line_buffer+i)=='$') {
         ch= *(line_buffer+i+1);
         if ((ch=='#')||(ch=='$')||(ch=='%')||(ch=='<')) {
           i+=2;
           continue;
         }
         st_pt=i++;
         j=0;
         if ((ch= *(line_buffer+i++))=='(') {
           ch= *(line_buffer+i++);
           while ((ch!=')')&&(ch!='\0')) {
             if ((j<40)&&(ch!=' ')&&(ch!='\t')) tmpbuff[j++]=ch;
             ch= *(line_buffer+i++);
           }
           tmpbuff[j]='\0';
           i--;
         }else{
           while (((ch>='0')&&(ch<='9')) || ((ch>='a')&&(ch<='z')) ||
                  ((ch>='A')&&(ch<='Z')) || (ch=='@')) {
             if (j<40) tmpbuff[j++]=ch;
             ch= *(line_buffer+i++);
           }
           tmpbuff[j]='\0';
           i=i-2;
         }
 
         num_fl=1;
         for (j=0;j<strlen(tmpbuff);j++) {
           if ((tmpbuff[j]<'0')||(tmpbuff[j]>'9')) {
             num_fl=0;
           }
         }
 
#ifdef EBUG
  if (debug_level>0) {
    (void) fprintf(deb_log,"Macro type:%s\n",tmpbuff);
  }
#endif
         ptr=(char *) NULL;
         if (tmpbuff[0]!='\0') {
           if (num_fl!=0) {
             j=atoi(tmpbuff);
             if (j<s_argc) {
               ptr= *(s_argv+j);
             }
           }else if (tmpbuff[0]=='@') {
             ch=clower(tmpbuff[1]);
             if ((ch>='a')&&(ch<='z')) {
               j=ch-'a';
               if (sing_type[j]==0) {
                 (void) sprintf(tmpbuff,"%i",sing_val[j].i);
               }else{
                 (void) sprintf(tmpbuff,"%f",sing_val[j].f);
               }
               ptr=tmpbuff;
             }
           }else{
             for (j=0;j<n_macs;j++) {
               if (strcmp(mac_name[j], tmpbuff)==0) ptr=mac_def[j];
             }
             if (ptr==(char *) NULL) {
               ptr=getenv(tmpbuff);
             }
           }
         }

         if (ptr!=(char *) NULL) {
           del=strlen(ptr)-(i-st_pt+1);
           while ((lb_ln+del)>(lb_size-3)) inc_line_buff();

           if (del<0) {
             for (j=(i+1);j<(lb_ln+1);j++) {
               *(line_buffer+j+del)= *(line_buffer+j);
             }
           }else{
             for (j=(lb_ln+1);j>=i;j--) {
               *(line_buffer+j+del)= *(line_buffer+j);
             }
           }
           for (j=0;j<strlen(ptr);j++) {
             *(line_buffer+st_pt+j)= *(ptr+j);
           }
           i=st_pt-1;
           n_subst++;
           lb_ln=strlen(line_buffer);
         }
       }
  
       if (*(line_buffer+i)!='\0') i++;
     }
   }
}

/* add_macro_name, add_macro - adds a macro definition to the list
                             - no checking performed on macro definition
                                (assumes direct call)
                             - strips special characters from macro
                                definition before storage */

int add_macro_name(name)
char *name;
{
   int i;
   char *copy_buff();

   for (i=0;i<n_macs;i++) {
     if (strcmp(mac_name[i],name)==0) break;
   }

   if (i==n_macs) {
     n_macs++;
     if (n_macs==100) sev_err(MACFLOOD);
     mac_name[i]=copy_buff(name);
   }else{
     if ((i>=N_PRE)&&(mac_def[i]!=(char *) NULL)) xfree(mac_def[i]);
   }
   mac_def[i]=(char *) NULL;

   return(i);
}

add_macro(num, str)
char *str;
int num;
{
   int i,j;
   char ch, chb;

   mac_def[num]=(char *) xalloc((unsigned int) (strlen(str)+10),
          "Unable to allocate macro definition storage.");

   j=0;
   for (i=0;i<strlen(str);i++) {
     ch= *(str+i);
     if (ch=='\\') {
       chb= *(str+i+1);
       if ((chb=='$')||(chb=='(')||(chb==')')||(chb==';')) continue;
     }
     *(mac_def[num]+j++)=ch;
   }
   *(mac_def[num]+j)='\0';
}

del_macro(num)
{
   if ((num>N_PRE)&&(mac_def[num]!=(char *) NULL)) xfree(mac_def[num]);
   mac_def[num]=(char *) NULL;
}
