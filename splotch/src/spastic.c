/******************************************************************
                          sPLOTch!

  Spastic.c - various error routines for the splotch parsing systems

*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include "version.h"
#include "spastic.h"
#include "spoof.h"

#ifdef EBUG
  extern FILE *deb_log;
  extern int debug_level;
#endif

extern char tmpbuff[];
int error_mode;
extern int lpos_no;
extern char *line_buffer;
struct errtb  {   /* error table for text string being processed */
	       int type;
	       int begin;
	       int length;
	      } temperr[100]; 
int nterr;                  /* and number in current string*/

int errtype[100];       /* footnote style error table for whole */
int numfoot;            /* total number of error references */

FILE *errchn;               /* log output channel */
int err_lpos;         /* line position..used for continuity of errors */
int errchn_open;    /* indicates status of errchn */
int errpt;                 /* error reporting flag */
int hush_flag;           /* stop error counting (i.e. during plots) */
int nerr=0;            /* number of errors that occurred (log or not) */

int ln_start, lflterr;             /* last end of line position, line errs*/


/*       open_err - opens log channel and initializes error keys 
                  - mode is non-zero for interactive mode */

open_err(name, mode, trace) 
char *name;
int mode, trace;
{
   char *head="*************************************";
   FILE *search_path();

   if ((mode==0)&&(trace==0)) {
     errchn=search_path(name,"w",0,0);
     if (errchn==(FILE *) NULL) {
        (void) fprintf(stderr,"Unable to open error log file.\n");
        (void) fprintf(stderr,"Check write permission for ");
        print_path(stderr);
        (void) fprintf(stderr,".\n");
        (void) exit(1);
     }
     error_mode=0;
   }else{
     errchn=stderr;
     if (mode!=0) error_mode=1;
     else error_mode=2;
   }
   ln_start=err_lpos=nterr=numfoot=lflterr=0;
   errchn_open=1;
   error_on();
   if ((mode==0)||(trace!=0)) {
     (void) fprintf(errchn,"%s*%s\n",head,head);
     (void) fprintf(errchn,"                 ");
     (void) fprintf(errchn,"sPLOTch! Version %i.%i - Routine Error Log\n",
               MAJ_VERS_NO, MIN_VERS_NO);
     (void) fprintf(errchn,"                                ");
     (void) fprintf(errchn,"Hi Sweetie!\n%s*%s\n\n",head,head);
   }else{
     (void) fprintf(errchn,"\n");
     (void) fprintf(errchn,"                This is sPLOTch! Version %i.%i\n",
               MAJ_VERS_NO, MIN_VERS_NO);
     (void) fprintf(errchn,"                      (Interactive Mode)\n\n");
     (void) fprintf(errchn,"Enter 'help;' for a simple help description,");
     (void) fprintf(errchn," 'quit;' or 'bye;' to exit.\n\n");
   }
   (void) fflush(errchn);
}

/* error_on,error_off - turn error reporting on and off
		      - meaningful only if error channel is
			 open (errchn_open=1)          */

error_on()
{
   if (errchn_open==1) {
      errpt=1;
   }
   hush_flag=0;
}
error_off()
{
   errpt=0;
   hush_flag=1;
}
log_off()
{
   errpt=0;
}

/*         sp_err - error storage routine 
		  - maintains table of individual errors found
		    during string parsing
                  - output during the err_flush routine  */

sp_err(type,begin,length)
int type,begin,length;
{
   if (begin<=0) begin=lpos_no-begin;
   if (hush_flag==0) nerr++;
   if (length<=0) length=1;
   if (errpt==1) {
      temperr[nterr].type=type;
      temperr[nterr].begin=begin;
      temperr[nterr].length=length-1;
      nterr++;
      if (nterr==99) sev_err(ERRFLOAD);
#ifdef EBUG
if (debug_level!=0) {
 (void) fprintf(deb_log,"ERROR type %i begin %i length %i\n",type,begin,length);
 (void) fflush(deb_log);
}
#endif
   }
}

/*      err_flush - error flush routine
                  - flushes pos characters from lead of line_buffer
		  - outputs command string in synch with stored error
		    table types
                  - maintains the footnote error table */

err_flush(pos)
int pos;
{
   int i,j;
   char ch;

   if (errpt==0) return;
#ifdef EBUG
   if (debug_level&DBG_INPT) {
     (void) fprintf(deb_log,"Error flushing %i from %i.\n", pos, lpos_no);
     (void) fflush(deb_log);
   }
#endif
   i=0;
   while(i<pos) {
      ch= *(line_buffer+i);
      if (ch=='\t') ch=' ';
      (void) fprintf(errchn,"%c",ch);
      err_lpos++;
      if (ch=='\n') {
        under_error();
        ln_start=lpos_no+i+1;
        err_lpos=0;
	lflterr=0;
      }

      for (j=0;j<nterr;j++) {
	 if (temperr[j].begin==(lpos_no+i)) {
           temperr[j].begin= -err_lpos;
           lflterr++;
         }
      }
      i++;
   }
   (void) fflush(errchn);
}

/* under_error - underline the error markers (at end of line) */

under_error()
{
   int j,k,l,loc,pre_n;
   char sp;

   if (lflterr!=0) {
     pre_n=numfoot+1;
     for (l=0;l<err_lpos;l++) {
        sp=' ';
	for (j=0;j<nterr;j++) {
	   if ((loc= -temperr[j].begin-1)<0) continue;
	   if((l>=loc)&&(l<=(loc+temperr[j].length))) sp='-';
	}
	(void) fprintf(errchn,"%c",sp);
     }
     (void) fprintf(errchn,"\n");
     for (l=0;l<err_lpos;l++) {
       sp='\0';
       for (j=0;j<nterr;j++) {
	 if ((loc= -temperr[j].begin-1)<0) continue;
         loc=loc+temperr[j].length/2;
         if (loc<=l) {
	   errtype[(++numfoot)]=temperr[j].type;
	   if (numfoot==99) sev_err(ERRFLOAD);
	   if (sp=='\0') (void) sprintf(tmpbuff,"%i",numfoot);
           else (void) sprintf(tmpbuff,"%c%i",sp,numfoot);
	   l=l+strlen(tmpbuff)-1;
	   (void) fprintf(errchn,"%s",tmpbuff);
           for (k=j;k<(nterr-1);k++) {
             temperr[k]=temperr[k+1];
           }
           nterr--; j--;
	   sp=',';
         }
       }
       if (sp=='\0') (void) fprintf(errchn," ");
     }
     (void) fprintf(errchn,"\n");
     if (error_mode!=0) {
       for (j=pre_n;j<=numfoot;j++) {
         (void) fprintf(errchn,"   %i: %s\n",j,errs[errtype[j]]);
       }
       if (error_mode==2) (void) fprintf(stderr,"\n");
     }
   }
}

/*         severr - major error handler
		  - called when error condition is severe
		     (usually due to fatal programming error)
		  - results in program termination          */

int severr_flag=0;
#define SEV_MSG "   sPLOTch! routine halted due to severe error exception.\n"

sev_err(type)
int type;
{
   severr_flag=1;
   (void) fprintf(stderr,SEV_MSG);
   (void) fprintf(stderr,"   %s\n",svrs[type]);
   if ((errpt!=0)&&(error_mode==0)) {
      (void) fprintf(errchn,"\n\n");
      (void) fprintf(errchn,SEV_MSG);
      (void) fprintf(errchn,"   %s\n",svrs[type]);
   }
   close_err(1);
   (void) exit(1);
}

/*         closerr - error clean up routine
                   - complains about errors only if fl==0
		   - prints complete footnote list 
		   - closes down the error log channel  */
close_err(fl)
int fl;
{
   int j;

   if (error_mode==0) {
     if (errpt!=0) {
       if (numfoot==0) {
          if (severr_flag==0) {
            (void) fprintf(errchn,"\nNo sPLOTch! errors detected.\n");
          }else{
            (void) fprintf(errchn,"\nNo other sPLOTch! errors detected.\n");
          }
       }else{
          (void) fprintf(errchn,"\n\n");
          for(j=1;j<=numfoot;j++) {
             (void) fprintf(errchn,"   %i: %s\n",j,errs[errtype[j]]);
          }
       }
       errchn_open=0;
       error_off();
     }
     (void) fclose(errchn);
     if ((nerr!=0)&&(severr_flag==0)&&(fl==0)) {
       if (nerr==1) {
         (void) fprintf(stderr,"There was one error in processing.\n");
       }else{
         (void) fprintf(stderr,"There were %i errors in processing.\n",
               nerr);
       }
       (void) fprintf(stderr,"See the log file (-l option) for more");
       (void) fprintf(stderr," information.\n");
     }
   }
}

char *note_str=(char *) NULL;

/* add_note - adds string to note storage */

add_note(note)
char *note;
{
   if (note_str==(char *) NULL) {
     note_str=(char *) xalloc((unsigned int) 
              ((strlen(note)+10)*sizeof(char)),
              "Unable to allocate storage for log notes.");
     *note_str='\0';
   }else{
     note_str=(char *) xrealloc((char *) note_str, (unsigned int) 
                  ((strlen(note)+strlen(note_str)+10)*sizeof(char)),
                  "Unable to reallocate log note storage.");
   }
   (void) strcat(note_str, note);
}

/* flush_note - places the note onto the error stream */

flush_note()
{
   if (note_str==(char *) NULL) return;

   if (errpt!=0) {
     (void) fprintf(errchn, "\n");
     under_error();
     ln_start=lpos_no;
     err_lpos=0;
     lflterr=0;
     (void) fprintf(errchn,"\n%s\n",note_str);
     (void) fflush(errchn);
   }
   (void) xfree(note_str);
   note_str=(char *) NULL;
}
