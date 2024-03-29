function [lon,lat,spd,dir]=read_vctr(filename)
%function [lon,lat,spd,dir]=read_vctr(filename)
% function for reading wind vector files generated by l2b_to_vctr
% useful for plotting with plot_windfield

fid=fopen(filename,'r','l'); % for little endian Mac Intel G5s
header=fread(fid,4,'uchar');
mat=fread(fid,'float');
N=length(mat);
mat=reshape(mat,[4,N/4]);
lon=mat(1,:)*180/pi;
lat=mat(2,:)*180/pi;
spd=mat(3,:);
dir=mat(4,:)*180/pi;
% dir=450-dir;
fclose(fid);

end