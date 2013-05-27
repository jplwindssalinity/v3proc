function out = ReadQuikSCATcollocatedECMWF(fname,varargin)
% function out = ReadQuikSCATcollocatedECMWF(fname)
%
% Reads a collocated ECMF (Svetla's version of collocated QuikSCAT L2B)
% fname is the filename
% optional parameter pairs:
%      'putNaNs'   --  1 or 0, default is 1, puts nans where lat and lon==0
%  
% Created by A. Chau 8/30/10

p = inputParser;
p.addRequired('fname', @ischar);
p.addParamValue('putNaNs',1,@(x)(x==0 | x==1))
p.parse(fname, varargin{:});
putNaNs = p.Results.putNaNs;

fprintf('Reading from %s ... ', fname)


fid = fopen(fname,'rb','ieee-be');

fseek(fid,4,'bof');

b=fread(fid,[21,1624],'char*1'); % these are the text timetags
% char(b')

fseek(fid,2*4,'cof');

clear A
for q = 1:4
    A(:,:,q) = fread(fid,[76,1624],'float32')';
    fseek(fid,2*4,'cof');
end
fclose(fid);

lat = A(:,:,1);
lon = A(:,:,2);
spd = A(:,:,3);
dir = A(:,:,4);

if putNaNs
    fprintf(' ... Putting NaNs')
    ind = (lat==0) & (lon==0);
    lat(ind) = nan;
    lon(ind) = nan;
    spd(ind) = nan;
    dir(ind) = nan;
end
fprintf('\n')

out.lat = lat;
out.lon = lon;
out.spd = spd;
out.dir = dir;
out.flag = [];
out.wvcsize = [];
out.source = sprintf('QuikSCATL2B_SV-ECMWF');

end