/*
*  Output test driver for sPLOTch! 2.1
*/

#include "../localdef.h"
#include <stdio.h>
#include "../sdvi.h"

FILE *sdvi_file;

main()
{
   int comm, i, tp, n, nn;
   COORD x, y, xr, yr;
   float val, valb, valc;
   char buff[1000];

   sdvi_file = stdin;

   x = readCoord(sdvi_file);
   if (feof(sdvi_file) != 0) {
     (void) fprintf(stderr, "sp_tty: Hey! You didn't output anything!!!!!\n");
     (void) exit(0);
   }
   if (x != SDVI_MAGIC) {
     (void) fprintf(stderr, "sp_tty: not a sPLOTch! sdvi file.\n");
     (void) fprintf(stderr, "Magic number %i instead of %i.\n", x, SDVI_MAGIC);
     (void) exit(1);
   }
  
   x = readCoord(sdvi_file);
   y = readCoord(sdvi_file);
   (void) printf("sPLOTch! sdvi file: Version %i.%i\n", x, y);
   (void) readCoord(sdvi_file);

   while (((comm = readCoord(sdvi_file)) != END_OF_FILE)&&(feof(sdvi_file) == 0)) {
     switch(comm) {
       case OPEN_SCR:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           val = readFloat(sdvi_file);
           (void) printf("Open_screen: %i by %i, scaled %f.\n", x, y, val);
           break;
       case UPD_SCR:
           (void) printf("Update_screen.\n");
           break;
       case CLOSE_SCR:
           (void) printf("Close_screen.\n");
           break;
       case MOVE_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           (void) printf("Move_pt to %i %i.\n", x, y);
           break;
       case DRAW_PT:
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           (void) printf("Draw_pt to %i %i.\n", x, y);
           break;
       case CH_WDTH:
           x = readCoord(sdvi_file);
           (void) printf("Change_width to %i.\n", x);
           break;
       case CH_COL:
           val = readFloat(sdvi_file);
           valb = readFloat(sdvi_file);
           valc = readFloat(sdvi_file);
           (void) printf("Change_colour to %g, %g, %g\n", val, valb, valc);
           break;
       case FILL_P:
           n = readCoord(sdvi_file);
           (void) printf("Fill_p with %i points (not listed).\n", n);
           for (i = 0; i <= n; i++) {
             x = readCoord(sdvi_file);
             y = readCoord(sdvi_file);
           }
           break;
       case DIAGRAM:
           tp = readCoord(sdvi_file);
           n = readCoord(sdvi_file);
           x = readCoord(sdvi_file);
           y = readCoord(sdvi_file);
           xr = readCoord(sdvi_file);
           yr = readCoord(sdvi_file);
           nn = readCoord(sdvi_file);
           for (i = 0; i < nn; i++) {
             buff[i] = fgetc(sdvi_file);
           }
           buff[i] = '\0';
           (void) printf("Diagram: type %i, rot %i from %i %i -> %i %i\n",
                         tp, n, x, y, xr, yr);
           (void) printf("             fname:%s:\n", buff);
           break;
       default: 
           (void) fprintf(stderr, "sp_tty: bad sdvi command %i.\n", comm);
           (void) exit(1);
           break;
     }
   }
   if (comm == END_OF_FILE) (void) printf("End of file\n");
   return(0);
}
