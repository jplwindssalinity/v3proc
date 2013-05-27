function Write_SV_windfield(fname,u,v,latlim,lonlim,nlons,nlats);
% function Write_SV_windfield(fname,u,v,latlim,lonlim,nlons,nlats);
% pass in u and v as matrices arranged such that:
%     increasing column corrsponds to increasing longitude
%     increasing row corresponds to increasing latitude (double check this)
%     

fid = fopen(fname,'wb');

fwrite(fid,latlim(1),'float32');
fwrite(fid,nlons,'int32');
fwrite(fid,nlats,'int32');
fwrite(fid,latlim(2),'float32');
fwrite(fid,lonlim(1),'float32');
fwrite(fid,u','float32');
fwrite(fid,v','float32');
fwrite(fid,lonlim(2),'float32');

fclose(fid);
