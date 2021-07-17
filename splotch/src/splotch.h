/******************************************************************
                          sPLOTch!

  Splotch.h - all the wonderful type definitions, variable
              definitions and initializations needed for the 
	      various splotch commands.
 
*******************************************************************/      

#include "localdefs.h"

#define iswht(c) ((c=='\t')||(c=='\n')) /*  extra white space test  */
#define clower(c) (((c)>='A')&&((c)<='Z'))?((c)-'A'+'a'):(c)
#define dabs(c)  ((c)<0.0)?(-1.0*c):(c)

#define DBG_NONE  0 /* debug flag indicators (down==worse) */
#define DBG_INPT  1
#define DBG_UTIL  2
#define DBG_MAIN  4
#define DBG_DATA  8
#define DBG_CALC  16
#define DBG_GRAPH 32

#define MAJOR 1 /* major tick indicator */
#define MINOR 2 /* minor tick indicator */
#define TK_IN 1     /* tick into graph */
#define TK_OUT 2    /* tick out of graph */
#define TK_CENT 3 /* tick centered on axis line */
#define TYPE_INT 1 /* integer data type */
#define TYPE_FLT 2 /* float data type */
#define PRC_SING 1 /* single precision */
#define PRC_DOUB 2 /* double precision */

#define BEG_MASK 0x01 /* set masks -begin, end, step, */
#define END_MASK 0x02
#define STEP_MASK 0x04
#define START_MASK 0x08

#define MID_UNIT 10 /* units above which ar relative */
#define PCT   1  /* sizing types */
#define INCH  2
#define CM    3
#define ABS   4
#define APCT  5
#define FPCT  6
#define SPCT  7
#define GPCT  8
#define HPCT  9
#define PT    10
#define RPCT  11  /* relative sizing types */
#define RINCH 12
#define RCM   13
#define RABS  14
#define RAPCT 15
#define RFPCT 16
#define RSPCT 17
#define RGPCT 18
#define RHPCT 19
#define RPT   20
#define M_PCT  0x001  /* unit typing masks*/
#define M_INCH 0x002
#define M_CM   0x004
#define M_ABS  0x008
#define M_APCT 0x010
#define M_FPCT 0x020
#define M_SPCT 0x040
#define M_GPCT 0x080
#define M_HPCT 0x100
#define M_PT   0x200

#define T_LEFT 1  /* text alignment parameters */
#define T_RIGHT 2
#define T_CENTER 3

#define X_SET 1 /* coordinate set masks  */
#define Y_SET 2    

#define SORT_X  1 /* sorting flags */
#define SORT_Y  2
#define SORT_XY 3
#define SORT_YX 4

#define AT_NONE         1  /* argument type classes */
#define AT_NUM          2
#define AT_NUM_UNIT     3
#define AT_DUAL_UNIT    4
#define AT_NUM_INT      5
#define AT_REL_NUM      6
#define AT_COLOUR       7
#define AT_STR          8
#define AT_COMM         9

/* NOTE:  for all of these, the none flag indicate whether or not
 the item is to be displayed.  None is set to 1 if NONE has been
 stated, regardless of other definitions (quick cancel function).
 Similarly, the set flag determines if data has been entered.  For
 labels it is simply 1 if set, for axis parameters it is given by
 the masks defined above. */

struct big_int { int sign;
                 unsigned long upper;
                 unsigned long lower; 
               };

struct coordinate { /* simple coordinate structure */
    COORD x, y; 
};

struct united { /* unit definition structure */
    float val;
    int unit;
};

struct sp_poly { /* my polygon structure for clipping and fills */
     int n_points;
     int nlim;
     COORD xmin, xmax, ymin, ymax;
     struct coordinate *pts;
};

struct sp_colour { /* colour information structure, from 0.0 to 1.0 */
     float hue;
     float sat;
     float bright;
};

struct sp_linefill { /* line/fill information structure */
     COORD width;
     int pattern;
     struct sp_colour colour;
     COORD repeat;
     float curr_ln;
};

struct lnfill_def {    /* line/fill style definition*/
     struct united width;
     struct sp_colour colour;
     int colour_set;
     struct united repeat;
     int pattern;
     int pattern_set;
     int none;
     int sp_fl;
     int colour_sp_fl;
};

struct inst_key { int number; /* text instruction sequencer */
                  int arg_type;

                  union {
                    int num_int;
                    float num;
                    struct united num_unit;
                    struct { struct united a;
                             struct united b; } dual_unit;
                    char *str;
                    struct { float val;
                             int rel; } rel;
                    struct { struct sp_colour col;
                             int col_set; } colour;
                    struct inst_key *comm;
                  } arg;
         
                  int num_sp_fl; /* external key values - three sets
                                    of 4 bit flags */

                  COORD xp,yp;  /* pointer position during draws */

                  struct inst_key *next_inst;
                  struct inst_key *prev_inst;
                };

struct limits {    /* text extent structure (for space) */
      COORD xmin, xmax, ymin, ymax;
};

struct underline {      /* underline definition parameters */
      COORD x, y;
      float a, h;
      int toggle;
};

struct sp_text {    /* data structure for character display purposes */
      int font;
      float hfact;
      float angle;
      float r_angle;
      float rotate;
      float slant;
      COORD x0, y0;

      struct limits limit;
      int max_set;
      int none;
      int noprint;
      int lrc;
      int val_fl;
      int sub_fl;
      int arrow_fl;
      struct underline uline;
      struct sp_linefill style;
};

union data_t_s {  /* data union for storage facilities */
    int i;
    float f;
};

union data_t_d {  /* union for double precision storage */
    int i;
    double f;
};

struct var_t {  /* structure describing variables */
    int nrows;
    char name[20];
    int type;
    int prec;
    int nblocks;
    int maxtab;
    union data_t_s **sgle;
    union data_t_d **dble; /* ok Im lazy..... */
};

struct sp_data { /* storage area definition structure */
    int set;
    struct var_t *vars[100];
    int nvars;
};

struct parse_stack { /* calc parsing storage structure */
    int op;   
    union data_t_d num;
};

struct tick_m {   /* tick mark data structure */
    struct lnfill_def style;
    struct united length_def;
    COORD length;
    int inout;
    int num;
    int none;
    int noprint;
};

struct char_st {   /* size/type info for character components  */
    struct inst_key *inst;
    struct limits limit;
    int none;
    int set;
};

struct coord_def {   /* coordinate definition - temporary string and all */
    struct united x_c;
    struct united y_c;
    COORD x, y;
};

struct axis_def {   /* axis data structure for determining axes labelling */
    int set_flag;
    struct tick_m major_t;
    struct tick_m minor_t;
    struct char_st label;
    int label_lrc;
    struct char_st value;
    int value_lrc;
    struct lnfill_def style;

    int nvalues;
    int ndec_places;
    int extend;

    int log_flag;
    double base;

    double begin;
    double end;
    double step;
    double start;
    int set;

    int exp;
    int exp_set;

    struct coord_def offset;
    struct coord_def origin;
    struct coord_def gaps;

    struct united ln_def;
    COORD length;

    int rad_fl;

    struct inst_key *clist[40];
    int n_clist;
    double vlist[40];
    int n_vlist;
};

struct pair_v { /* float/int pair for special symbols */
    int i, type;
    double f;
};

struct sym_def { /* symbol table data for plotting  */
    int set;
    struct inst_key *pnt;
    int pnt_set;
    int pnt_none;
    struct inst_key *label;
    int label_set;
    int label_none;
    struct lnfill_def linestyle;
    int line_set;
    struct lnfill_def fillstyle;
    int fill_set;
    struct coord_def offset;
    int offset_spec;
    int inter;
    int inter_set;
    struct united special;
    int spec_spec;
    int sort_fl;
    int sort_set;
    int arrow;
    int arrow_set;
};

struct coord3d { /* 3 dimensional coordinate structure */
    float x;
    float y;
    float z;
};

struct opt_def {  /* data structure for graphics options */
    int def_unit;
    COORD curv_res;
    COORD arrow;
    float arr_ang;
    struct united margin;
    struct lnfill_def borderline;
    int border_set;
    float edge;
    int fourtk;
    COORD dev_xmax;
    COORD dev_ymax;
    COORD cp_xmax;
    COORD cp_ymax;
    float aspect;
    float winfract;
    COORD origin_x;
    COORD origin_y;
    int dfont;
    int rotate;
    COORD sht;
    COORD linewidth;
    struct sp_colour linecolour;
    COORD gp_orig_x;
    COORD gp_orig_y;
    COORD gp_xmax;
    COORD gp_ymax;

    int dim_mode;
    float screen;
    int screen_set;
    struct coord3d eyeball;
    struct coord3d orig_2d;
    struct coord3d vec_2dx;
    struct coord3d vec_2dy;
    struct coord3d size_2d;
    struct coord3d vertical;
    struct coord3d tvecx;
    struct coord3d tvecy;
    struct coord3d tvecz;
    struct coord3d size_3d;
    struct coord3d orig_3d;
    float prj_x;
    float prj_y;

    int use_gridtri;
    struct sp_colour *colourlims;
    int ncolours;
};

struct var_set { /* identifiers for graph variables */
    char name[3][20];
    int var_n[3];
    int nrows;
    int extra_n[10];
    int extra_t[10];
    int n_extra;
};

#define CL_XMIN 1
#define CL_YMIN 2
#define CL_XMAX 4
#define CL_YMAX 8

struct surf_def { /* coordinate sets defining a surface */
    struct var_set crd;
    struct var_set hull;
    int hull_set;
    int ax_n;
    double d_clips[4];
    int cl_flag;
    struct sp_colour *colourlims;
    int ncolours;
};

struct grid_def { /* data surface grid description */
   COORD *xmap;
   COORD *ymap;
   float *z_sing;
   double *z_doub;
   double zmin, zmax;
   int prec;
   float *aux_z_sing;
   double *aux_z_doub;
   double aux_zmin, aux_zmax;
   int aux_prec;
   int type_flag;
   int close_flag;

   unsigned char *map, *block_map;  /* grid stuff */
   unsigned int n_map;
   int nx;
   int ny;

   int *connect; /* triangulation stuff */
   int n_points;

   int *edgelist;  /* both */
   int nedge;
   unsigned int n_tri;
};

struct mesh_tri { /* triangle definition */
   struct coordinate c[3];
   double z[3];
   int pnt[3];
   double aux_z[3];
};
