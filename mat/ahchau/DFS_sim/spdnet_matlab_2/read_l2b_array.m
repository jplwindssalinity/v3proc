function [spd,dir,lat,lon,ati1,nati,ncti]=read_l2b_array(filename);
%function [spd,dir,lat,lon,ati1,nati,ncti]=read_l2b_array(filename);

fid=fopen(filename,'r','l');
ati1=fread(fid,[1,1],'int32');
nati=fread(fid,[1,1],'int32');
ncti=fread(fid,[1,1],'int32');
spd=fread(fid,[ncti,nati],'float');
dir=fread(fid,[ncti,nati],'float')*180/pi;
lat=fread(fid,[ncti,nati],'float')*180/pi;
lon=fread(fid,[ncti,nati],'float')*180/pi;
fclose(fid);
