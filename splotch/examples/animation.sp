options aspect=1.0 hasize=100apct mode3d resolution=0.1in;
options 2dsize=(100.0,100.0) screen=450.0 vertical=(0,0,1);
title null;

for (t=0.0,360.0,9.0);

   options eulereye=(700.0,90,$@t);

   if( $@t > 180.0 );
    options 2dvecx=(1.0,0.0,0.0) 2dvecy=(0.0,0.0,1.0);
    options 2dorigin=(-50.0,0.0,-50.0);
    input(screen.back.in);
   else;
    options 2dvecx=(-1.0,0.0,0.0) 2dvecy=(0.0,0.0,1.0);
    options 2dorigin=(50.0,0.0,-50.0);
    input(screen.front.in);
   endif;

endfor;
