/*****************************************************************
                          sPLOTch!

  Splotch - main routine for plotting package.  Processes command
       line options, reads plotting directions and routes
       plotting instructions.
 
*******************************************************************/      

#include "splotch.h"
#include <stdio.h>
#include <signal.h>
#include <math.h>
#include "spastic.h"
#include "version.h"

#ifdef EBUG 
   FILE *deb_log;
   int  debug_level=0;
#endif

int interactive_mode;

int on_the_fly;
int s_argc;
char **s_argv;

char *prog_name;
struct axis_def *axes[20];
struct sp_data *datas[10];
struct char_st titles[10];
struct char_st foots[10];
struct sym_def *symbols[101];
struct opt_def options;

extern union data_t_d sing_val[27];
extern int sing_type[27];

char *hist_set[40], *inter_buff, *replay_buff;
int hist_set_n, curr_hist;

extern int flow_nest[40], flow_loop[40], curr_flow;
extern char *flow_strs[40];
struct _nothing {
         int var_num;
         union {double f; int i;} start;
         union {double f; char *list;} end;
         double step;
       } loop_defs[40], *lp_ptr;

COORD n_x=0,n_y=0;    /* note coordinates (here allows relative) */
COORD llc_x,llc_y,urc_x,urc_y;
struct axis_def p_axes[3];
COORD xrad_cent, yrad_cent;
int polar_fl=0, gr_drawn=0;
float eccentricity=1.0;

char argbuff[1000], inbuff[1000], tmpbuff[1000];
double m_pi, deg_to_rad;
extern void piperr();

main(argc,argv)
int argc;
char *argv[];
{
   int i,c,rc,argl,default_prog,nolog,do_log,pipe_fl,trace_fl,macro_num;
   char *con_name, *dev_name, *out_name, *ptr;
   static char *d_name=DEFFILE, *dft_n="default", *nowhere="/dev/null";
   static char *pipeline="pipeline", *inter="interactive";
   FILE *search_path(), *main_file, *rc_file, *dvi_file;

   /* The following line is for SUN debugging only...adjust Makefile */
   /* malloc_debug(2); */
   rdfont();

   con_name=dev_name=out_name=(char *) NULL;
   default_prog=0;
   do_log=nolog=0;
   curr_flow=argl=0;
   hist_set_n=trace_fl=pipe_fl=0;
   interactive_mode=0;
   curr_hist=1;

   /* note: this check is no good if splotch -n is defined */
   if (argc<=1) {
     (void) fprintf(stderr,"usage: splotch [-options] control.file\n");
     (void) fprintf(stderr,"or: splotch [-options] < data.file\n");
     (void) exit(1);
   }

   init_macros();

   for (c=1;c<argc;c++) {
     ptr=argv[c]+2;
     if (*argv[c]=='-') {
       switch (*(argv[c]+1)) {
	  case '\0': argl=1;
		     break;
	  case 'd' : if ((*ptr=='\0')&&(argv[c+1]!=(char *) NULL))
                       ptr=argv[++c];
                     dev_name=ptr;
                     break;
	  case 'D' : if ((*ptr=='\0')&&(argv[c+1]!=(char *) NULL))
                       ptr=argv[++c];
                     rc= -1;
                     for (i=0;i<strlen(ptr);i++) {
                       if (*(ptr+i)=='=') rc=i;
                     }
                     if (rc==-1) {
                       macro_num=add_macro_name(ptr);
                       add_macro(macro_num, "");
                     }else{
                       *(ptr+rc)='\0';
                       macro_num=add_macro_name(ptr);
                       add_macro(macro_num, (ptr+rc+1));
                     }
                     break;
          case 'h' : usage();
                     break;
          case 'i' : interactive_mode=1;
                     break;
          case 'j' : out_name=nowhere;
                     break;
	  case 'l' : do_log=1;
		     break;
	  case 'n' : nolog=1;
		     break;
	  case 'o' : if ((*ptr=='\0')&&(argv[c+1]!=(char *) NULL))
                       ptr=argv[++c];
                     out_name=ptr;
                     break;
	  case 'p' : pipe_fl=1;
		     break;
          case 't' : trace_fl=1;
                     break;
          case 'w' : on_the_fly=-1;
                     break;
	  case 'v' : (void) fprintf(stderr,
	                   "\n\n          sPLOTch! Version %i.%i\n",
                           MAJ_VERS_NO, MIN_VERS_NO);
                     (void) fprintf(stderr,
                           "          Patchlevel %i\n", PATCHLEVEL);
		     (void) fprintf(stderr,"          %s\n\n", VERS_DATE);
		     (void) exit(0);
		     break;
#ifdef EBUG
	  case 'x' : if ((*ptr=='\0')&&(argv[c+1]!=(char *) NULL))
                       ptr=argv[++c];
                     debug_level=atoi(ptr);
		     break;
#endif
          case 'z' : on_the_fly=1;
                     break;
	  default  : (void) fprintf(stderr,"Error: option not one of ");
		     (void) fprintf(stderr,"fix this please\n");
		     break;
	}
	if (argl==1) break;
     }else{
        con_name=argv[c];
     }
   }

   if ((con_name==(char *) NULL)&(pipe_fl==0)&(interactive_mode==0)) 
         default_prog=1;
   if (pipe_fl!=0) con_name=pipeline;
   if (interactive_mode!=0) {
     con_name=inter;
     inter_buff=(char *) xalloc((unsigned int) (5000*sizeof(char)),
        "Unable to allocate interactive command line buffer.");
   }

#ifdef EBUG
   if (debug_level!=0) {
     (void) fprintf(stderr,"Debugging on level:%i\n",debug_level);
     deb_log=fopen("splotch.debug.log","w");
     if (deb_log==(FILE *) NULL) {
       (void) fprintf(stderr,"Unable to open debug log.\n");
       (void) exit(1);
     }
     marktab();
   }
#endif

   (void) (*signal)(SIGPIPE, piperr);

   if (default_prog==1) {
     nolog=1;
      if ((con_name=getenv("SPLOTCH_DEF"))==(char *) NULL) {
        con_name=d_name;
      }
   }

   s_argc=argc-c-1;
   s_argv=argv;
   if (argc>0) s_argv=(argv+c+1);
   else argc=0;

   more_macros(con_name);

   if (pipe_fl!=0) {
     main_file=stdin;
   }else if (interactive_mode!=0) {
     main_file=(FILE *) NULL;
   }else{
     if (default_prog==1) {
       main_file=fopen(con_name,"r");
       if (main_file==(FILE *) NULL) {
         (void) fprintf(stderr,"Unable to open default file:");
         (void) fprintf(stderr,"%s.\n",con_name);
         (void) fprintf(stderr,"Check or define the SPLOTCH_DEF");
         (void) fprintf(stderr," environment variable.\n");
         (void) exit(1);
       }
     }else{
       main_file=search_path(con_name,"r",1,1);
       if (main_file==(FILE *) NULL) {
          (void) fprintf(stderr,"Unable to open file:%s.\n",con_name);
          (void) exit(1);
       }
     }
   }

   if (default_prog==1) {
      prog_name=dft_n;
   }else{
      for (i=strlen(con_name);i>-1;i--) {
        if (*(con_name+i)=='/') break;
      }
      prog_name=con_name+i+1;
   }
   if ((nolog==0)||(do_log==1)||(interactive_mode!=0)||(trace_fl!=0)) {
      (void) sprintf(argbuff,"%s.log",prog_name);
      open_err(argbuff, interactive_mode, trace_fl);
   }

   if (out_name==(char *) NULL) {
     if (dev_name==(char *) NULL) {
       (void) sprintf(argbuff,"%s.sdvi",prog_name);
       dvi_file=search_path(argbuff, "w", 0, 0);
       if (dvi_file==(FILE *) NULL) {
         (void) fprintf(stderr,
             "Unable to open default sdvi output file: %s\n", argbuff);
         (void) exit(1);
       }else{
         reg_sdvi(dvi_file, 0);
       }
     }else{
       dvi_file=popen(dev_name, "w");
       if (dvi_file==(FILE *) NULL) {
         (void) fprintf(stderr,
             "Unable to start output device driver: %s\n", dev_name);
         (void) exit(1);
       }else{
         reg_sdvi(dvi_file, 1);
       }
     }
   }else{
     if (*out_name=='-') {
       reg_sdvi(stdout, 2);
     }else{
       dvi_file=search_path(out_name, "w", 0, 0);
       if (dvi_file==(FILE *) NULL) {
         (void) fprintf(stderr,
                   "Unable to open sdvi output file: %s\n", out_name);
         (void) exit(1);
       }else{
         reg_sdvi(dvi_file, 0);
       }
     }
   }

   (void) strcpy(argbuff,"~/.splotchrc");
   expand_home(argbuff);
   rc_file=fopen(argbuff,"r");
      
   init_opt(&options);

   new_file(main_file);
   if (rc_file!=(FILE *) NULL) new_file(rc_file);

   init_tables();
   m_pi=4.0*atan((double) 1.0);
   deg_to_rad=m_pi/180.0;

   while (end_of_prog()==0) {
     rc=do_commands();
     if (rc<0) break;
   }

   if (interactive_mode!=0) {
     while(feof(stdin)==0) {
       update_screen();
       read_int_line(inter_buff);
       ptr=inter_buff;
       if (do_int_comm(&ptr)<0) continue;
       direct_add_buff(ptr);
       while (end_of_prog()==0) {
         rc=do_commands();
         if (rc<0) break;
       }
       if (rc<0) break;
     }
   }

   close_screen();
   close_err(default_prog);
   if ((pipe_fl==0)&&(interactive_mode==0)) (void) fclose(main_file);
   if (interactive_mode!=0) {
     (void) fprintf(stderr,"\n\nThanks for playing!\n\n");
   }
   dereg_sdvi();

#ifdef EBUG
   if (debug_level!=0) (void) fclose(deb_log);
#endif
   return(0);
}


/*   do_command - reads in the function commands and redirects to the
                    appropriate handler routine */

int do_commands()
{
   struct char_st *tfptr; 
   int i,k,l,n,n1,n2,rc,com[4],tf_fl,tmp, flow_level,com2[4];
   struct sp_text txt; 
   struct united jnk_vu;
   struct inst_key *note_inst;
   int d_table[101];
   struct sym_def *sym_defable[101];
   FILE *tmp_file,*search_path();
   COORD crd_sz;
   char ch, next_non_white(), *copy_buff();
   double val;

   do { 
     com[1]=0;
     rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 1);
     if (com[0]==-2) line_buff_flush(com[1], 1);
   } while(com[0]==-2);

#ifdef EBUG
   if (debug_level>0) {
    (void) fprintf(deb_log,"\n\nFunction cntrl -> %i %i %i\n",rc,com[0],com[1]);
    (void) fflush(deb_log);
   }
#endif

   if (rc<0) {
      dump_line(BADCOM,com);
   }else{
     flow_level=0;
     if (curr_flow!=0) {
       flow_level=flow_nest[curr_flow-1];
       if ((flow_level==2)||(flow_level==4)||(flow_level==5)) {
         if ((com[0]<162)||(com[0]>164)) {
           dump_line(0,com);
           return(1);
         }
       }
       if (flow_level==9) {
         if ((com[0]<173)||(com[0]>175)) {
           dump_line(0,com);
           return(1);
         }
       }
     }
     tf_fl=0;
     switch (com[0]) {
         case -2  :
         case 0   : dump_line(0,com);
                    break;
         case 162 : if (curr_flow!=0) {
                      if ((flow_level==2)||(flow_level==4)||(flow_level==5)) {
                        flow_loop[curr_flow]=0;
                        flow_nest[curr_flow++]=5;
                        if (curr_flow==40) sev_err(TOOIFS);
                        dump_line(0, com);
                        break;
                      }
                    }
                    rc=test_comp(argbuff);
                    flow_loop[curr_flow]=0;
                    if (rc!=0) flow_nest[curr_flow++]=1;
                    else flow_nest[curr_flow++]=2;
                    if (curr_flow==40) sev_err(TOOIFS);
                    dump_line(0, com);
                    break;
         case 163 : if (curr_flow!=0) {
                      rc=0;
                      if ((flow_level==3)||(flow_level==4)) rc=BADIFS;
                      else if (flow_level==1) flow_nest[curr_flow-1]=4;
                      else if (flow_level==2) flow_nest[curr_flow-1]=3;
                      else if (flow_level!=5) rc=BADIFS;
                    }else rc=BADIFS;
                    dump_line(rc, com);
                    break;
         case 164 : if ((curr_flow!=0)&&(flow_level<6)) {
                      curr_flow--;
                      dump_line(0, com);
                    }else dump_line(BADIFS, com);
                    break;
         case 171 : init_loop(curr_flow);
                    flow_nest[curr_flow++]=6;
                    if (curr_flow==40) sev_err(TOOIFS);
                    dump_line(0, com);
                    break;
         case 172 : if ((curr_flow==0)||(flow_level!=6)) {
                      dump_line(BADNEST, com);
                    }else{
                      rc=test_comp(argbuff);
                      if (rc!=0) {
                        add_readback(flow_strs[curr_flow-1]);
                        flow_loop[curr_flow-1]= -1;
                      }else{
                        curr_flow--;
                        xfree(flow_strs[curr_flow]);
                        flow_loop[curr_flow]=0;
                      }
                      dump_line(0, com);
                    }
                    break;
         case 173 : if ((curr_flow!=0)&&(flow_level==9)) {
                      flow_loop[curr_flow]=0;
                      flow_nest[curr_flow++]=9;
                      if (curr_flow==40) sev_err(TOOIFS);
                      dump_line(0, com);
                      break;
                    }
                    k=0;
                    lp_ptr= &(loop_defs[curr_flow]);
                    rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                    if (rc<0) {
                      sp_err(BADFORCH, tmp, l);
                      dump_line(0, com);
                      break;
                    }
                    lp_ptr->var_num=add_macro_name(inbuff);
                    lp_ptr->end.list=copy_buff(argbuff+k);
                    com2[1]=0;
                    yank(lp_ptr->end.list, com2, inbuff);
                    if (is_empty(inbuff)!=0) {
                      flow_loop[curr_flow]=0;
                      flow_nest[curr_flow++]=9;
                    }else{
                      add_macro(lp_ptr->var_num, inbuff);
                      lp_ptr->start.i=com2[1];
                      init_loop(curr_flow);
                      flow_nest[curr_flow++]=8;
                    }
                    if (curr_flow==40) sev_err(TOOIFS);
                    dump_line(0, com);
                    break;
         case 174 : if ((curr_flow!=0)&&(flow_level==9)) {
                      flow_loop[curr_flow]=0;
                      flow_nest[curr_flow++]=9;
                      if (curr_flow==40) sev_err(TOOIFS);
                      dump_line(0, com);
                      break;
                    }
                    k=0;
                    lp_ptr= &(loop_defs[curr_flow]);
                    rc=getc_buff(argbuff, &k, inbuff, 1000, '=');
                    if (rc<0) {
                      sp_err(BADFOR, tmp, l);
                      dump_line(0, com);
                      break;
                    }
                    rc=getc_buff(argbuff, &k, tmpbuff, 1000, ',');
                    lp_ptr->start.f=atof(tmpbuff);
                    rc=getc_buff(argbuff, &k, tmpbuff, 1000, ',');
                    lp_ptr->end.f=atof(tmpbuff);
                    if (rc<0) {
                      lp_ptr->step=1.0;
                    }else{
                      val=atof(argbuff+k);
                      if (val==0.0) val=1.0;
                      lp_ptr->step=val;
                    }
                    k=0;
                    ch=next_non_white(inbuff, &k);
                    ch=clower(ch);
                    if ((ch<'a')||(ch>'z')) {
                      sp_err(BADFOR, tmp, l);
                      dump_line(0, com);
                      break;
                    }
                    lp_ptr->var_num=ch-'a';
                    sing_type[lp_ptr->var_num]=TYPE_FLT;
                    sing_val[lp_ptr->var_num].f=lp_ptr->start.f;

                    rc=0;
                    if (lp_ptr->step>0.0) {
                      if (lp_ptr->start.f>lp_ptr->end.f) rc=1;
                    }else{
                      if (lp_ptr->start.f<lp_ptr->end.f) rc=1;
                    }
                    if (rc!=0) {
                      flow_loop[curr_flow]=0;
                      flow_nest[curr_flow++]=9;
                    }else{
                      init_loop(curr_flow);
                      flow_nest[curr_flow++]=7;
                    }
                    if (curr_flow==40) sev_err(TOOIFS);
                    dump_line(0, com);
                    break;
         case 175 : if ((curr_flow==0)||(flow_level<7)) {
                      dump_line(BADNEST, com);
                      break;
                    }
                    if (flow_level==9) {
                      curr_flow--;
                      dump_line(0, com);
                      break;
                    }
                    rc=0;
                    lp_ptr= &(loop_defs[curr_flow-1]);
                    if (flow_level==7) {
                      val=sing_val[lp_ptr->var_num].f;
                      val=val+lp_ptr->step;
                      sing_val[lp_ptr->var_num].f=val;
                      if (lp_ptr->step>0.0) {
                        if (val<=lp_ptr->end.f) rc=1;
                      }else{
                        if (val>=lp_ptr->end.f) rc=1;
                      }
                    }else{
                      com2[1]=lp_ptr->start.i;
                      yank(lp_ptr->end.list, com2, inbuff);
                      if (is_empty(inbuff)!=0) {
                        xfree((char *) lp_ptr->end.list);
                      }else{
                        del_macro(lp_ptr->var_num);
                        add_macro(lp_ptr->var_num, inbuff);
                        lp_ptr->start.i=com2[1];
                        rc=1;
                      }
                    }
                    if (rc!=0) {
                      add_readback(flow_strs[curr_flow-1]);
                      flow_loop[curr_flow-1]= -1;
                    }else{
                      curr_flow--;
                      xfree(flow_strs[curr_flow]);
                      flow_loop[curr_flow]=0;
                    }
                    dump_line(0, com);
                    break;
         case 165 : clean_string(argbuff);
                    (void) system(argbuff);
                    dump_line(0, com);
                    break;
         case 72  : dump_line(0,com);
                    open_screen();
                    break;
         case 92  : line_buff_flush(com[1], 1);;
                    define_macros();
                    break;
         case 120 : dump_line(0,com);
                    close_screen();
                    break; 
         case 122 :
         case 121 : dump_line(0,com);
                    com[0]=121;
                    break;
         case 114 : dump_line(0,com);
                    doodle();
     	            break;
         case 168 : dump_line(0,com);
                    doodle3d();
                    break;
         case 32  : line_buff_flush(com[1],1);
                    com[1]=0;
                    option(&options);
                    break;
         case 41  : clean_string(argbuff);
                    (void) fprintf(stderr,"%s\n", argbuff);
                    dump_line(0,com);
                    break;
         case 25  : tmp_file=search_path(argbuff,"r",0,1);
		    if (tmp_file==(FILE *) NULL) {
	      	      sp_err(NOINP,tmp,l);
		    }else{
                      if (num_files()>=19) {
	      	        sp_err(TREEFULL,-com[3],com[2]);
                        (void) fclose(tmp_file);
                        tmp_file=(FILE *) NULL;
                      }
                    }
                    dump_line(0,com);
                    if (tmp_file!=(FILE *) NULL) {
                      add_note("");
                      flush_note();
                      new_file(tmp_file);
                    }
                    break;
         case 118 :
         case 130 : case 79  : case 88  :
         case 102 : case 26  : case 73  :
         case 23  : if (rc==0) n=0;
                    else n=atoi(argbuff)-1;

		    if((n<0)||(n>9)) {
		      sp_err(BADDATA,tmp,l);
                      dump_line(0, com);
                      break;
		    }
                    if (datas[n]==(struct sp_data *) NULL) {
                      if ((com[0]==23)||(com[0]==118)) {
                        datas[n]=(struct sp_data *) xalloc(
                            (unsigned int) sizeof(struct sp_data),
                            "Unable to allocate data storage structure.");
                        datas[n]->set=0;
                      }else{
		        sp_err(NODAT,tmp,l);
                        dump_line(0, com);
                        break;
                      }
                    }
#ifdef EBUG
   if (debug_level&DBG_MAIN) {
     (void) fprintf(deb_log,"Command %i on data set %i\n",com[0],n);
     (void) fflush(deb_log);
   }
#endif
                    line_buff_flush(com[1],1);
                    com[1]=0;
                    switch(com[0]) {
                       case 130: output(datas[n]);
                                 break;
                       case 23 : data_p(datas[n]);
                                 break;
                       case 118: calc(datas[n]);
                                 break;
                       case 26:  open_screen();
                       case 73:  plot_it(datas[n]);
                                 break;
                       case 88:  error_bar(datas[n]);
                                 break;
                       case 79:  histo_gram(datas[n]);
                                 break;
                       case 102: plot_3d(datas[n]);
                                 break;
                       default : dump_line(0, com);
		                 break;
                    }
                    break;
           case 22: if (rc!=0) {
                      k=0; 
                      rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                      if (rc<0) k= -1;
		      if (is_empty(inbuff)==0) {
		        rc=scsize(inbuff, 'x', &crd_sz, &jnk_vu, 0, 0);
			if (rc<0) {
			  sp_err(BADSIZE, tmp, l);
			}else{
			  if (rc>MID_UNIT) n_x+=crd_sz;
			  else n_x=crd_sz;
			}
		      }
	 	      if (k>=0) {
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
			if (is_empty(inbuff)==0) {
		   	  rc=scsize(inbuff, 'y', &crd_sz, &jnk_vu, 0, 0);
			  if (rc<0) {
			    sp_err(BADSIZE, tmp, l);
			  }else{
			    if (rc>MID_UNIT) n_y+=crd_sz;
			    else n_y=crd_sz;
			  }
		  	}
	 	      }
		    }
	            init_text(&txt);
                    note_inst=(struct inst_key *) NULL;
                    line_buff_flush(com[1],1);
                    speak((char *) NULL, &txt, 0, 0, 0, &note_inst, 1, 0);
                    crd_sz=0;
                    halign_text(note_inst, n_x, n_x, n_y, 
                         &crd_sz, 0, 0, T_LEFT);
                    del_inst_tree(note_inst);
		    break;
           case 8:  n=atoi(argbuff)-1;
	            if((n<0)||(n>19)) {
	              sp_err(BADAXIS, tmp, l);
                      dump_line(0, com);
	            }else{
#ifdef EBUG
   if (debug_level!=0) {
     (void) fprintf(deb_log,"Doing AXIS number %i\n",n);
     (void) fflush(deb_log);
   }
#endif
                      if (axes[n]==(struct axis_def *) NULL) {
                        axes[n]=(struct axis_def *) xalloc(
                             (unsigned int) sizeof(struct axis_def),
                             "Unable to allocate axis structure.");
                        axes[n]->set_flag=0;
                      }
                      line_buff_flush(com[1],1);
                      com[1]=0;
		      do_axis(axes[n]);
		    }
		    break;
           case 42: for (i=0;i<101;i++) d_table[i]=0;
                    if (rc!=0) {
                      tf_fl=k=0;
                      while (rc>=0) {
                        l=k;
                        rc=getc_buff(argbuff, &k, inbuff, 1000, ',');
                        if (is_empty(inbuff)==0) {
                          for (i=0;i<strlen(inbuff);i++) {
                            if (inbuff[i]=='-') break;
                          }
                          if ((inbuff[i]=='\0')||(is_empty(inbuff+i+1)!=0)) {
                            n1=n2=atoi(inbuff);
                          }else{
                            inbuff[i]='\0';
                            n1=atoi(inbuff); n2=atoi(inbuff+i+1);
                            if (n1>n2) { n1=n2; n2=atoi(inbuff); }
                            inbuff[i]='-';
                          }
                          if ((n1<0)||(n1>100)||(n2<0)||(n2>100)) {
			    sp_err(BADSYM, (tmp+l), (int) strlen(inbuff));
                          }else{
                            for (i=n1;i<=n2;i++) d_table[i]=1;
                          }
                        }
                      }
                    }else{
                      d_table[0]=1;
                    }
                    tf_fl=0;
                    for (i=0;i<101;i++) {
                      if (d_table[i]!=0) {
                        if (symbols[i]==(struct sym_def *) NULL) {
                          symbols[i]=(struct sym_def *) xalloc((unsigned int)
                                sizeof(struct sym_def),
                                "Unable to allocate symbol storage.");
                          symbols[i]->set=0;
                        }
                        sym_defable[tf_fl++]=symbols[i];
                      }
                    }
                    line_buff_flush(com[1],1);
                    com[1]=0;
                    do_symbol(sym_defable, tf_fl);
                    break;
	   case 31: tf_fl++;
	   case 35: if (rc==0) n=0;
                    else n=atoi(argbuff)-1;

		    if ((n<0)||(n>9)) {
		      sp_err(BADTITL, tmp, l);
                      dump_line(0, com);
                      break;
		    }else{
		      if (tf_fl==1) { 
		        tfptr= &titles[n];
		      }else{
		        tfptr= &foots[n];
		      }
                      if (tfptr->set!=0) del_inst_tree(tfptr->inst);
		      tfptr->set=1;
		      init_text(&txt);
                      tfptr->inst=(struct inst_key *) NULL;
                      line_buff_flush(com[1],1);
                      com[1]=0;
                      speak((char *) NULL, &txt, 0, 1, 0, &(tfptr->inst), 1, 0);
		      tfptr->none=txt.none;
		      /* tfptr->lrc=txt.lrc; */
                    }
		    break;
         default  : dump_line(ILLCOM,com);
                    break;
     }
   }
   flush_note();
   if (com[0]==121) return(-1);
   else return(1);
}

/* dump_line - dumps an input line 
             - command error if err!=0 (using err) */

dump_line(err,com)
int err,com[4];
{
   int rc, l, tmp;
  
   if (err!=0) {
     com[1]=com[3];
     what(err, (char *) NULL, com, 0, 1);
   }
   line_buff_flush(com[1], 1);
   do { 
     com[1]=0;
     rc=scan_cmd((char *) NULL, com, 0, &l, &tmp, argbuff, 0);
     if (rc<0) what(err, (char *) NULL, com, 0, 0);
     line_buff_flush(com[1], 1);
   } while (com[0]!=0);
}

/* read_int_line - reads a line in interactive mode
                 - done reading when end-of-line entered and last
                   item on line is semi-colon    */

read_int_line(buff)
char *buff;
{
   int i;
   char ch, last_ch;

   (void) fprintf(stderr,"\nsPLOTch!(%i): ",curr_hist);
   last_ch=i=0;
   while(((ch=getchar())!=EOF)&&(feof(stdin)==0)) {
     if (ch=='\\') {
       ch=getchar();
       if (ch!='\n') {
         (void) ungetc(ch, stdin);
         ch='\\';
       }
     }else{
       if ((ch=='\n')&&(last_ch==';')) break;
     }
     *(buff+i++)=ch;
     if (i==4999) {
       (void) fprintf(stderr,
         "Interactive command line length restricted to 5000 characters.\n");
       (void) exit(1);
     }
     if ((ch!=' ')&&(!iswht(ch))) last_ch=ch;
   }
   *(buff+i++)='\n';
   *(buff+i)='\0';
   (void) fprintf(stderr,"\n");
}

static char *spec_com[]={"help", "edit", "hist", "store", "replay"};

int do_int_comm(i_buff)
char **i_buff;
{
   int i, j, k, rc, rpt_fl;
   char ch, next_non_white(), *buff;

   buff= *i_buff;
   rpt_fl=rc=k=0;
   ch=next_non_white(buff, &k);
   if (ch=='!') {
     for (j=k;j<strlen(buff);j++) {
       ch= *(buff+j);
       if ((ch==' ')||iswht(ch)||(ch==';')) break;
     }
     *(buff+j)='\0';
     if (is_numeric((buff+k+1), 0)!=0) {
       rpt_fl=atoi(buff+k+1);
       if (rpt_fl<0) rpt_fl=hist_set_n+rpt_fl;
       else rpt_fl=rpt_fl-(curr_hist-hist_set_n);
       if ((rpt_fl<0)||(rpt_fl>=hist_set_n)) {
         (void) fprintf(stderr,"%i: Event not found.\n", atoi(buff+k+1));
         return(-1);
       }
     }else{
       j=strlen(buff+k+1);
       for (i=hist_set_n-1;i>=0;i--) {
         if (strncmp((buff+k+1), hist_set[i], j)==0) break;
       }
       if (i==hist_set_n) {
         (void) fprintf(stderr,"%s: Event not found.\n", (buff+k+1));
         return(-1);
       }
       rpt_fl=i;
     }
     (void) strcpy(buff, hist_set[rpt_fl]);
   }else{
     for (i=0;i<5;i++) {
       for (j=0;j<strlen(spec_com[i]);j++) {
         if (clower(*(buff+j+k))!=*(spec_com[i]+j)) break;
       }
       if (j==strlen(spec_com[i])) break;
     }
     if (i!=5) {
       switch(i) {
         case 0:  help();
                  rc= -1;
                  break;
         case 2:  for (i=0;i<hist_set_n;i++) {
                    if (strlen(hist_set[i])<60) {
                      (void) fprintf(stderr,"%i  %s", 
                         (curr_hist-hist_set_n+i), hist_set[i]);
                    }else{
                      (void) fprintf(stderr,"%i  %.60s ...\n", 
                         (curr_hist-hist_set_n+i), hist_set[i]);
                    }
                  }
                  rc= -1;
                  break;
         default: break;
       }
     }else{
     }
   }
   if (rc!=-1) {
     curr_hist++;
     if (hist_set_n==40) {
       xfree((char *) hist_set[0]);
       for (i=0;i<39;i++) hist_set[i]=hist_set[i+1];
       hist_set_n--;
     }
     if (rpt_fl==0) {
       hist_set[hist_set_n]=(char *) xalloc((unsigned int)
          ((strlen(buff)-k+10)*sizeof(char)),
          "Unable to allocate command history storage buffer.\n");
       (void) strcpy(hist_set[hist_set_n], (buff+k));
     }else{
       hist_set[hist_set_n]=hist_set[rpt_fl];
     }
     hist_set_n++;
   }
   return(rc);
}

/*  test_comp - returns non-zero if test passes */

static char *comp_types[]={"!=", ">=", "<=", "==", "<", ">"};

int test_comp(buff)
char *buff;
{
  int i, j, k, rc, quote, is_string, arg_fl, match;
  double vala, valb;

  match= -1;
  j=quote=is_string=arg_fl=0;
  inbuff[0]=tmpbuff[0]='\0';
  for (i=0;i<strlen(buff);i++) {
    if (*(buff+i)=='"') {
      is_string=1;
      if (quote!=0) {
        if (arg_fl==1) inbuff[j]='\0';
        else if (arg_fl==2) tmpbuff[j]='\0';
        quote=0;
      }else{
        j=0;
        quote=1;
        arg_fl++;
      }
    }else{
      if (quote!=0) {
        if (arg_fl==1) inbuff[j++]= *(buff+i);
        else if (arg_fl==2) tmpbuff[j++]= *(buff+i);
      }else{
        if (match==-1) {
          for (k=0;k<6;k++) {
            if (strncmp((buff+i), comp_types[k], strlen(comp_types[k]))==0) {
              break;
            }
          }
          if (k!=6) match=i;
        }
      }
    }
  }
  if ((arg_fl>2)||(match==-1)) return(!is_empty(buff));

  if (is_string!=0) {
    clean_string(inbuff);
    clean_string(tmpbuff);
    rc=strcmp(inbuff, tmpbuff);
  }else{
    *(buff+match)='\0';
    vala=atof(buff);
    valb=atof(buff+match+strlen(comp_types[k]));
    if (vala<valb) rc= -1;
    else if (vala==valb) rc=0;
    else rc=1;
  }
  switch (k%3) {
    case 0: if (rc==0) rc=1;
            else rc=0;
            break;
    case 1: if (rc==-1) rc=1;
            else rc=0;
            break;
    case 2: if (rc==1) rc=1;
            else rc=0;
            break;
  }
  if (k<3) rc=1-rc;
  return(rc);
}
 
/*   usage - outputs information on sPLOTch! options */

usage()
{
   (void) fprintf(stderr,"\nusage: splotch [-options] [control.file]\n");
   (void) fprintf(stderr,"or: splotch -dDEVICE [-options] < data.file\n");
   (void) fprintf(stderr,"\nOptions\n-------\n");
   (void) fprintf(stderr,"  -h         => generates this output.\n");
   (void) fprintf(stderr,"  -i         => interactive mode.\n");
   (void) fprintf(stderr,"  -l         => forces error log output (overrides -n).\n");
   (void) fprintf(stderr,"  -n         => requests no error log output.\n");
   (void) fprintf(stderr,"  -p         => reads control program from stdin (pipe).\n");
   (void) fprintf(stderr,"  -s         => sideways output for oblique screens.\n");
   (void) fprintf(stderr,"  -v         => outputs sPLOTch! version.\n");
#ifdef EBUG
   (void) fprintf(stderr,"  -xNN       => requests debugging info on level NN.\n");
#endif
   (void) fprintf(stderr,"  -          => ends options list (start of arguments).\n\n");
}

/* help - prints out a simple help for commands in interactive mode */

help()
{
   (void) fprintf(stderr,"\nsPLOTch! Interactive Command Mode\n\n");
   (void) fprintf(stderr,"   This interactive mode allows sPLOTch! commands to be processed\n");
   (void) fprintf(stderr,"directly from the keyboard, with instant feedback.  All sPLOTch!\n");
   (void) fprintf(stderr,"commands are valid in this mode;  see the Reference Manual for a\n");
   (void) fprintf(stderr,"description of the sPLOTch!  command language, as well as a more\n");
   (void) fprintf(stderr,"in-depth description of using sPLOTch! interactively.\n");
   (void) fprintf(stderr,"\n   The prompt in interactive mode indicates the output device\n");
   (void) fprintf(stderr,"presently in use (which can be changed using the DEVICE option),\n");
   (void) fprintf(stderr,"as well as the command number.  The command history can be dis-\n");
   (void) fprintf(stderr,"played by entering 'hist;' (remember that all sPLOTch! commands\n");
   (void) fprintf(stderr,"end with a semi-colon).  Previously entered commands can be re-\n");
   (void) fprintf(stderr,"trieved using !<number> or !<string>, where <number> is an actual\n");
   (void) fprintf(stderr,"command number (positive) or relative to the current command\n");
   (void) fprintf(stderr,"(negative), and <string> is matched to the beginning of previous\n");
   (void) fprintf(stderr,"commands.  The previous command can be editted use ^old^new^,\n");
   (void) fprintf(stderr,"which replaces 'old' with 'new'.  Appending a 'g' to this command\n");
   (void) fprintf(stderr,"(ie. ^old^new^g) replaces all occurences of 'old'.  Using !! or\n");
   (void) fprintf(stderr,"^^ as the command leader performs the appropriate function, but\n");
   (void) fprintf(stderr,"does not execute the result (for multiple edits or editting old\ncommands).\n");
   (void) fprintf(stderr,"\n   Finally, user commands can be recorded into a file using the\n");
   (void) fprintf(stderr,"record command.  'record on <file>' activates recording (into\n");
   (void) fprintf(stderr,"<file>, if specified), while 'record off' stops recording.\n");
}

/* init_tables - initialize the storage tables to NULL pointers */

init_tables()
{
   int i;

   for (i=0;i<10;i++) datas[i]=(struct sp_data *) NULL;
   for (i=0;i<20;i++) axes[i]=(struct axis_def *) NULL;
   for (i=0;i<10;i++) titles[i].set=0;
   for (i=0;i<10;i++) foots[i].set=0;
   for (i=0;i<4;i++) p_axes[i].set_flag=0;
   for (i=0;i<101;i++) symbols[i]=(struct sym_def *) NULL;
   symbols[0]=(struct sym_def *) xalloc((unsigned int) sizeof(struct sym_def),
                  "Unable to allocate default symbol storage.");
   symbols[0]->set=0;
   init_sym(symbols[0], 1);
}
