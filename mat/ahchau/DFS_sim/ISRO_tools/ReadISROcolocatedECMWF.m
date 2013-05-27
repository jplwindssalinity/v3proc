function out = ReadISROcolocatedECMWF(fname,varargin);
% function out = ReadISROcolocatedECMWF(fname);
%
% Reads a colocated ECMWF (colocated with ISRO L2B file)
% fname is the filename
% optional parameter pairs:
%    'putNaNs'   --  1 or 0, default is 1, puts nans where lat and lon==0
%
% Created by A. Chau 8/24/10


p = inputParser;
p.addRequired('fname', @ischar);
p.addParamValue('putNaNs',1,@(x)(x==0 | x==1))
p.parse(fname, varargin{:});
putNaNs = p.Results.putNaNs;

fprintf('Reading from %s ... ',fname)

fid = fopen(fname);
Elat = fread(fid,[36 860],'float');
Elon = fread(fid,[36 860],'float');
Espd = fread(fid,[36 860],'float');
Edir = fread(fid,[36 860],'float');
fclose(fid);

if putNaNs
    fprintf(' ... Putting NaNs')
    ind = (Elat==0) & (Elon==0);
    Elat(ind) = nan;
    Elon(ind) = nan;
    Espd(ind) = nan;
    Edir(ind) = nan;
end
fprintf('\n')

out.lat = Elat';
out.lon = Elon';
out.spd = Espd';
out.dir = Edir';
out.flag = [];
out.wvcsize = [];
out.source = 'ECMWF_ISRO';