function [u,v,latlim,lonlim,nlons,nlats]=read_SV_windfield(file)
%function [u,v,latlim,lonlim,nlons,nlats]=read_SV_windfield(file)
% reads a SV format windfield which can also be read
% by the QSCATSim library. The format of the header
%(and footer) info is counter intuitive, because the
% space that holds the bounding box values was
% originally used for FORTRAN unformatted special
% values. The QSCATSim library just skips these, but
% but it was useful for MATLAB to put the bounds here.
% The file is read as:
%  fid=fopen(file,'r');
%  latlim(1)=fread(fid,1,'float');
%  nlons=fread(fid,1,'int32');
%  nlats=fread(fid,1,'int32');
%  latlim(2)=fread(fid,1,'float');
%  lonlim(1)=fread(fid,1,'float');
%  u=fread(fid,[nlons,nlats],'float');
%  v=fread(fid,[nlons,nlats],'float');
%  lonlim(2)=fread(fid,1,'float');
%  fclose(fid);
u=[];
v=[];
latlim=[-100,-100];
lonlim=[0,0];
  fid=fopen(file,'r','l');
  if(fid==-1)
    return;
  end
  latlim(1)=fread(fid,1,'float');
  nlons=fread(fid,1,'int32');
  nlats=fread(fid,1,'int32');
  latlim(2)=fread(fid,1,'float');
  lonlim(1)=fread(fid,1,'float');
  u=fread(fid,[nlons,nlats],'float');
  v=fread(fid,[nlons,nlats],'float');
  lonlim(2)=fread(fid,1,'float');
  fclose(fid);
