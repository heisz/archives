options border=none;
options dfont=romand aspect=1 hsize=100apct aspect=-1 origin=(0apct,10apct);

title h=3.5 u 'Rotating Cylinder in a Moving Fluid' u;

axis(1) order=(from -3 to 3 by 1.5) 
        label=(h=5pct font=italicc 'x')
        length=60pct;
axis(2) order=(from -3 to 3 by 1.5)
        label=(h=5pct font=italicc 'y')
        length=60pct;

data file=curveball.data
     input=(x y ang vel)
     input=(xb yb)
     input (xp yp);

symbol(0) line=0 int=none char=(f=math1 angle=#1 h=#2pct 'o');
symbol(2) line=1 int=join char=none;
symbol(3) line=(2 repeat=1.5) int=join char=none;

plot vars=(y(x)[ang,vel] yb(xb) yp(xp)) haxis=1 vaxis=2 frame
     href=(0.0, 3) vref=(0.0, 3);

note(0.866abs,-0.5abs) center h=2 f=misc1 '\\';
note(-0.866abs,-0.5abs) center h=2 f=misc1 '\\';

note(20apct,11apct) h=3 f=misc1 '\\' f=romand ' represent points of'
    ' fluid stagnation';

note(20apct,7apct) h=3 f=romand 'Short dashed line represents p('
     f=greekc 'q' f=romand ')';
