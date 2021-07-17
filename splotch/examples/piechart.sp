options aspect=1 hsize=80pct origin=(50apct,50apct,center,center);

title;

calc newvar(val[5] index rad count)
     compute(val=@rand()*75.0+25.0) 
     compute(@@t=0.0)
     compute(index=@@t+val/2.0; @@t=@@t+val)
     compute(val=val/@@t*360.0; index=index/@@t*360.0; rad=1.0; count=#);

symbol int=vbarfix special=(#1) line=(1 width=2)
       label=none;
symbol(1) fill=(7 colour=0:0:0) offset=(#2,10pct);
symbol(2) fill=(7 colour=0:0:0.25);
symbol(3) fill=(7 colour=0:0:1);
symbol(4) fill=(7 colour=0:0:0.75);
symbol(5) fill=(7 colour=0:0:0.5);

axis(1) order=(from 0 to 1 by 0) label=none line=none minor=none
        major=none value=none;
axis(2) label=none line=none minor=none major=none value=none;

plot polar=degree vars=(rad(index,count)[val,index]) haxis=2 vaxis=1
     clip=none;
