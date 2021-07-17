/******************************************************************
                          sPLOTch!

  Spastic.h - error definition parameters for the error routines
 
*******************************************************************/      

#define BADCOM 1  /* bad command in parse string */
#define BADEND 2  /* end of line reached before close of text string */
#define BADFONT 3 /* bad font name */
#define TWOMOVE 4 /* only one motion value for move or draw */
#define BADSIZE 5 /* bad size specification (ie not IN or CM) */
#define ILLCOM 6  /* illegal command in current context */
#define BADAXIS 7 /* bad axis definition number */
#define HI 8      /* humurous comment to myself */
#define MISSARG 9 /* missing argument for command */
#define NOFILE 10 /* unable to open input file */ 
#define NOVARS 11 /* no variable list given for input */
#define BADHIST 12 /* improper specs on histogram range */
#define NODATA 13 /* no input data found in file */
#define DATMISS 14 /* unable to type variables due to some missing */
#define NOAXIS 15 /* axis requested is undefined */
#define TWOOFF 16 /* both offsets must be specified */
#define BADTITL 17 /* bad specifier for title number */
#define NOFORM 18 /* incorrectly specified plot format */
#define UNKVAR 19 /* unknown variable */
#define BADSYM 20 /* symbol specifier too large or small */
#define BADFIT 21 /* bad fit parameter */
#define BADDATA 22 /* bad data set number */
#define BADLN  23 /* bad line type number */
#define BADDEV 24 /* bad device type */
#define NOTODEV 25 /* device type reset */
#define NOINFILE 26 /* no input file opened */
#define NOTHREE 27 /* not x*y=c for cont etc*/
#define RANGETOP 28 /* too many members in range list */
#define NOTWHIST 29 /* not two outputs for histogram */
#define NOPS 30    /* no poscript file available */
#define BADEPS 31  /* not a proper EPs file */
#define NOINP 32   /* unable to locate input file */
#define TREEFULL 33 /* file input tree overflowed */
#define NOOUT 34  /* bad output variable specs */
#define UNDOUT 35 /* no output variables defined */
#define NOBAR  36 /* wrong number of variables for bars */
#define SEVERR 37 /* severe error indicator */
#define THREEC 38 /* three coordinates for positions */
#define BADLAY 39 /* bad layout parameter */
#define NOBRA  40 /* no opening bracket to match closing */
#define BADPATT 41 /* bad pattern value */
#define BADCOL  42 /* bad colour spec */
#define BADSPEC 43 /* illegal or bad special value indicator */
#define BRAMISS 44 /* mismatched brackets */
#define MISSVAR 45 /* blank space between operands */
#define BADSING 46 /* improper special variable spec */
#define BADFUNC 47 /* bad function name */
#define BADARG 48 /* bad function arguments */
#define BADRANGE 49 /* bad variable range specification */
#define NODAT   50 /* specified data set undefined */
#define BADPIPE 51 /* error occured during operation of shell */
#define INOUT   52 /* too many input/output variables in a line */
#define TOOREF  53 /* too many reference lines of a similar type */
#define BADCLIP 54 /* bad clipping style */
#define BADDEG  55 /* bad polar degree specification */
#define BADPREC 56 /* bad precision specification */
#define BADARRW 57 /* wrong type of arrows */
#define TOOSURF 58 /* too many surfaces */
#define BADPTR  59 /* bad end arrow parameter for symbols */
#define BADSURF 60 /* bad surftype argument */
#define BADDEF  61 /* bad form for macro definition */
#define BADIFS  62 /* bad if structure .... multiple elses or endifs */
#define BADXALIGN 63 /* bad origin alignment - x direction */
#define BADYALIGN 64 /* bad origin alignment - y direction */
#define BADLOG  65 /* bad log control statement */
#define BADNEST 66 /* incorrect nesting of loops inside other loops or ifs */
#define BADFOR 67 /* bad for loop specification */
#define BADFORCH 68 /* bad for loop specification */
#define BADFOUR  69 /* missing auxiliary variable in 4d system */
#define BADTRANS 70 /* bad coordinate transformation */
#define TWOSURF  71 /* respecified a surface dummy head */
#define BADHIDE  72 /* bad hidden method specs */

/*  SEVERE execution errors...require immediate program shutdown */

#define BUFLOAD 1 /* temporary buffer overflow */
#define ERRFLOAD 2 /* overflow of temporary error storage */
#define OVERDATA 3 /* too many data subsets */
#define OVERPLOT 4 /* too many plotting coordinate sets */
#define NOSPACE  5 /* no space for spline routine */
#define HELP     6 /* something gone really wrong!!! */
#define OVERCNT  7 /* too many coordinates in contour plot!!!*/
#define TOOMAN   8 /* data space overflowed */
#define CONTCR   9 /* contour path overflow */
#define TOOBRA   10 /* too many text command layers */
#define PIPERR   11 /* pipe error */
#define TOOIFS   12 /* too many if statments nested */
#define BADSET   13 /* too many subcurves in single variable set */
#define MACFLOOD 14 /* too many macros (100?) */
