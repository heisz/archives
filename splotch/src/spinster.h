#define AT_NONE         1  /* argument type classes */
#define AT_NUM          2
#define AT_NUM_UNIT     3
#define AT_DUAL_UNIT    4
#define AT_NUM_INT      5
#define AT_REL_NUM      6
#define AT_COLOUR       7
#define AT_STR          8
#define AT_COMM         9

struct inst_key { int number;
                  int arg_type;

                  union {
                    int num_int;
                    float num;
                    struct { float val;
                             int unit; } num_unit;
                    struct { float vala;
                             int unita;
                             float valb;
                             int unitb; } dual_unit;
                    char *str;
                    struct { float val;
                             int rel; } rel;
                    struct { struct sp_colour col;
                             int col_set; } colour;
                    struct inst_key *comm;
                  } arg;
         
                  int num_sp_fl; /* external key values - three sets
                                    of 4 bit flags */

                  int xp,yp;  /* pointer position during draws */

                  struct inst_key *next_inst;
                  struct inst_key *prev_inst;
                };
