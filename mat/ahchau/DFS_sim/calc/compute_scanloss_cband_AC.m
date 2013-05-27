for beam=1:2
  beamstr={'DFS Cband Inner', 'DFS Cband Outer', 'QuikSCAT Inner', 'QuikSCAT Outer'};
pref={'DFSCbandin','DFSCbandout','QSin','QSout'};
r= 6378.15    % km at equator, spherical earth should be good enough 
spinrate=[22,22,18,18]; % rpm
spinrate=spinrate(beam)*2*pi/60; % rad/s
pri = [0.01,0.01,0.0108,0.0108]; % seconds
pri=pri(beam);
look=[43, 49, 40, 46]; % deg
speed_light=2.9979e5; % km/s
alt=[710,710,800,800]; 
alt=alt(beam);

% we estimate the radius of the scan circle on the ground, and the slant range
% using law of sines
look=look(beam)*pi/180;
inc=asin((sin(look)*(r+alt))/r);
slantrange=sin(inc-look)*r/sin(look);
scanrad=r*(inc-look);

% we estimate the arc length on the ground (really we want the interior
% line segment of the arc but this is close enough)
rtt=2*slantrange/speed_light;
arclength = scanrad * rtt *spinrate;

% by breaking isoseles triangle bounded by 2 slantrange lengths and 
% the arclength on the ground into two right triangles we get
scanlossang=2*atan(arclength/(2*slantrange));


beamwidth = [2.0, 2.0, 1.8,1.7]  % degrees 1-way

beamwidth=beamwidth(beam)


res1way=slantrange*beamwidth*pi/180
x=-3*beamwidth:0.001:3*beamwidth;
xoff=x-scanlossang*180/pi;
xmid=x+scanlossang*180/pi/2;
wf=0.44295/(beamwidth/2);
gpattrans=sinc(x*wf).^2;
gpatrec=sinc(xoff*wf).^2;

glossfree=gpattrans.*gpattrans;
glossy=gpattrans.*gpatrec;

figure(beam)
plot(x,10*log10(gpattrans),'--k',x,10*log10(gpatrec),'--k',x,10*log10(glossy),'r',xmid,10*log10(glossfree),'k');
set(gca,'Ylim',[-10,0]);
grid on
hold on
plot([-beamwidth/2,beamwidth/2],[-3,-3],'k--x')
hold off
i=find(x<0);
[m,j1]=min(abs(glossfree(i)-0.5));
j1=i(j1);

i=find(x>0);
[m,j2]=min(abs(glossfree(i)-0.5));
j2=i(j2);
res=slantrange*(x(j2)-x(j1))*pi/180
scloss=10*log10(sum(glossfree)/sum(glossy));

titl=sprintf('%s beam Scan Loss Diagram %g km azres %g dB scanloss',beamstr{beam},res,scloss);
title(titl);
legend('Transmit','Recieve','Lossy Two-way','Ideal Two-way',0);
xlabel('Azimuth in Antenna Frame (deg)');
ylabel('dB');


outfile=sprintf('scanloss_cband_diagram_%s.png',pref{beam});
print('-dpng',outfile);
end
