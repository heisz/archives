calc newvar=(x y z)
     compute(x=@xmesh(2,1000)*1.0;
             y=@ymesh(2,1000)*1.0;
             z=@sin(y*@pi) : 1, 2000);

calc compute(@@t=@pi);

axis(1) order=(from 0 to 1 by 1) label=none minor=none major=none
line=none value=none;

title; 

symbol fill=7;
plot scatter=(z(x,y) colourset=(240:1.0:0.5,180:1.0:0.65,180:1.0:1.0))
haxis=1 vaxis=1 horigin=(0apct,0apct) hlen=100apct vlen=(100apct);


calc newvar=(x y data)
     compute(x=@xmesh(5,24)*0.7+0.15;
             y=@ymesh(5,24)*1.20-0.1;
             @@y=@xmesh(5,24)*2.0*@@t-@@t;
             @@x=-@ymesh(5,24)*2.0*@@t+@@t;
     data=-@exp(-@abs(@@x*@@y)/10)*(@sin(@@x+@@y)+@sin(@@x*@@y))+2.0: 1,120);

symbol char=(h=2.5 $$(%.5f)) int=none;

options mode3d 2dsize=(100,130) 2dorigin=(-50,-65,0) eulereye=(300,65,240);
options screen=235 project=48,43;
plotover vars=(y(x)[data]) horigin=(0apct,0apct) hlen=100apct vlen=100apct
     haxis=1 vaxis=1 clip=none;
options mode2d;
options 3dorigin=(-25,-30,10) 3dsize=(70,110,40);

axis(2) order=(from -$@t to $@t by 1);
axis(3) order=(from $@t to -$@t by -1);

calc newvar=(xs ys zs)
     compute(xs=@xmesh(31,31)*2.0*@@t-@@t;
             ys=@ymesh(31,31)*2.0*@@t-@@t;
             zs=-@exp(-@abs(xs*ys)/10)*(@sin(xs+ys)+@sin(xs*ys)): 1,961);

plot3d surf=(zs(ys,xs) colourset=(120:1:.5,15:1:.5)) xaxis=-2 yaxis=-3
       zaxis=0;

options colour=0:0:0.5;

/* note move(4.6apct,71apct) draw(-3apct,53apct)
     move(41.8apct,49.8apct) draw(52.9apct,11.2apct)
     move(92.8apct,60.4apct) draw(115apct,28.9apct);

note move(15.4apct,62apct) draw(10.9apct,43apct)
     move(28.3apct,59apct) draw(29.2apct,29.9apct)
     move(68.8apct,55.3apct) draw(86.9apct,21apct); */

options colour=0:0:1;
note(15,95) h=3.5apct f=romant left 'sPLOTch! Version 2.1';
note(95,5) h=2.5apct f=romand right 'Lets you draw your own conclusions.';

options aspect=1 vgsize=6apct width=0.5gpct;
options gorigin=5apct,92apct;

doodle;
4 0 3 5 0.0:0.0: 1 3
10 90 10 10 90 10
4 0 3 5 0.0:0.0: 1 2
10 23 18 23
4 0 3 5 0.0:0.0: 1 2
10 36 18 36
4 0 3 5 0.0:0.0: 1 2
10 49 26 49
4 0 3 5 0.0:0.0: 1 2
10 62 18 62
4 0 3 5 0.0:0.0: 1 2
10 75 18 75
4 0 3 5 0.0:0.0: 1 2
10 88 26 88
4 0 3 5 0.0:0.0: 1 2
25 10 25 18
4 0 3 5 0.0:0.0: 1 2
40 10 40 18
4 0 3 5 0.0:0.0: 1 2
55 10 55 18
4 0 3 5 0.0:0.0: 1 2
70 10 70 18
4 0 3 5 0.0:0.0: 1 2
85 10 85 26
17 0 0 5 0.0:0.0: 1 1 1 0.0:0.0: 7 10
35 33 33 33 25 30 14 19 13 8 17 3
27 8 27 16 30 24 35 32
17 0 0 5 0.0:0.0: 1 1 1 0.0:0.0: 7 8
70 40 76 44 86 44 90 38 89 32 83 32 79 35 71 40
17 0 0 5 0.0:0.0: 1 1 1 0.0:0.0: 7 12
75 49 68 44 63 33 63 25 60 22 56 25 56 33 59 44 
65 52 72 56 78 55 78 51
17 0 0 5 0.0:0.0: 1 1 1 0.0:0.0: 7 9
56 77 60 82 67 85 75 85 78 88 78 93 68 96 62 93 56 85
17 0 0 5 0.0:0.0: 1 1 1 0.0:0.0: 7 38
81 62 73 62 65 57 60 54 59 52 52 46 51 37 49 27
46 17 43 16 38 16 37 21 40 29 41 35 41 43 35 41
30 40 21 41 16 49 19 62 25 67 35 62 37 59 38 62
37 67 29 75 27 81 35 90 44 89 49 78 52 68 60 70
68 76 76 78 89 75 94 67 90 60 83 60
-10
