function [spd,tspd,spdtrue,spdret]=plot_hurrspdcmp(truefile,retfile,pngfile)
%plothurrspdcmp(truefile,retfile,outfile)
% takes a .spddat truth and retrieval file and plots
% 2 by 2 figures, ret speed, true speed, speed err, 2D hist

fid=fopen(truefile,'r','l');
n=fread(fid,[1,1],'int32');  
lat=fread(fid,[n,1],'float');  
lon=fread(fid,[n,1],'float');  
tspd=fread(fid,[n,1],'float');  
ati=fread(fid,[n,1],'int32');  
cti=fread(fid,[n,1],'int32');  
fclose(fid);

fid=fopen(retfile,'r','l');
n=fread(fid,[1,1],'int32');  
lat=fread(fid,[n,1],'float');  
lon=fread(fid,[n,1],'float');  
spd=fread(fid,[n,1],'float');  
fclose(fid);


zi=find(tspd==0);
spd(zi)=0;
zi=find(spd==0);
tspd(zi)=0;

i3=ati;
i3=i3-min(i3)+1;
j3=cti;
j3=j3-min(j3)+1;
d1=max(i3);

idx3=(j3-1)*d1+i3;
spdtrue=zeros(max(i3),max(j3));


spdret=0*spdtrue;
spdtrue(idx3)=tspd;

spdret(idx3)=spd;


figure(1); clf
subplot(2,2,2);
imagesc(spdtrue,[0,60]);h=colorbar;
set(gca,'YDir','normal');
set(h,'FontSize',10);
h=title('Truth');
set(h,'FontSize',14);
set(gca,'DataAspectRatioMode','manual');
set(gca,'DataAspectRatio',[1,1,1]);

subplot(2,2,1);
imagesc(spdret,[0,60]);h=colorbar;
set(gca,'YDir','normal');
set(h,'FontSize',10);
err=spd-tspd;

rms=sqrt(mean(err.*err));

titl=sprintf('ANN RMS=%g m/s',rms);
h=title(titl);
set(h,'FontSize',14);
set(gca,'DataAspectRatioMode','manual');
set(gca,'DataAspectRatio',[1,1,1]);

subplot(2,2,3);
imagesc(spdret-spdtrue,[-10,10]);h=colorbar;
set(gca,'YDir','normal');
set(h,'FontSize',10);
h=title('ANN - Truth');
set(h,'FontSize',14);
set(gca,'DataAspectRatioMode','manual');
set(gca,'DataAspectRatio',[1,1,1]);

subplot(2,2,4);
[x,y,ct]=hist2D(tspd,spd,0:0.5:70,0:0.5:70);
imagesc(x,y,log10(ct'));h=colorbar;
hold on
plot([0,70],[0,70],'w');
hold off
       set(gca,'YDir','normal');
       set(h,'FontSize',10);
       h=title('2-D Log Histogram');
       set(h,'FontSize',14);
       set(gca,'DataAspectRatioMode','manual');
       set(gca,'DataAspectRatio',[1,1,1]);
       xlabel('True Speed (m/s)');
       ylabel('ANN Speed (m/s)');
       




  
       print('-dpng',pngfile);
