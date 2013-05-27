flist=dir('*.unf');


n=length(flist)

for i=1:n
infile=flist(i).name
outfile=sprintf('little_endian/%s',infile);
u=[];
v=[];
latlim=[-100,-100];
lonlim=[0,0];

%read bigendian
fid=fopen(infile,'r','b');
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
 
  % write little endian
  sz=size(u);
  nlons=sz(1);
  nlats=sz(2);
  fid=fopen(outfile,'w','l');
  fwrite(fid,latlim(1),'float');
  fwrite(fid,nlons,'int32');
  fwrite(fid,nlats,'int32');
  fwrite(fid,latlim(2),'float');
  fwrite(fid,lonlim(1),'float');
  fwrite(fid,u,'float');
  fwrite(fid,v,'float');
  fwrite(fid,lonlim(2),'float');
  fclose(fid);
end
