function out = ReadQuikSCATL2B(fname,varargin)
%function out = ReadISROL2B(fname,varargin)
%
% Reads an ISRO L2B file
% fname is the filename
% optional parameter pairs:
%    'retrieval' --  'selection' or 'selected' reads 'Wind_speed_selection'
%                          and 'Wind_direction_selection'
%                    'model' reads the model spd and direction 
%                          'Model_speed' and 'Model_direction'
%                    'standard' reads 'Wind_speed' and 'Wind_direction'
%                     DEFAULT is 'selection'
%    'putNaNs'   --  1 or 0, default is 1, puts nans where lat and lon==0
% output is a single structure containing 
%   out.lat, out.lon, out.spd, out.dir, out.flag, out.wvcsize,
%   out.source='ISRO'
%   where arrays are arranged with cross track index varying across columns
%     for ISRO 50 km, this should be (860x36)
%
% Created A. Chau 8/24/10

p = inputParser;
p.addRequired('fname', @ischar);
p.addParamValue('retrieval','selection', @(x)(strcmp(x,'selection') | ...
    strcmp(x,'DIRTH') | strcmp(x,'selectedamb') | ... 
    strcmp(x,'selected') | strcmp(x,'model') | strcmpi(x,'standard')) );
p.addParamValue('putNaNs',1,@(x)(x==0 | x==1))
p.parse(fname, varargin{:});
retrievaltype = p.Results.retrieval;
putNaNs = p.Results.putNaNs;


fprintf('Reading from %s ... ',fname)

hinfo = hdfinfo(fname);
% figure out if 25 or 12.5 km data
if ~isempty(strfind(hinfo.Attributes(1).Value,'25.0km'))
    wvcsize = 25;
else
    wvcsize = 12.5;
end


% Latitude: read in the data and scale factor, and convert
scale_factor = .01;
wvc_lat = hdfread(fname,'/wvc_lat');
wvc_lat = double(wvc_lat) * scale_factor;

% Longitude: read in the data and scale factor, and convert
scale_factor = .01;
wvc_lon = hdfread(fname,'/wvc_lon');
wvc_lon = double(wvc_lon) * scale_factor;



% find the indices of the speed/direction to read
switch retrievaltype
    case {'selection','DIRTH','selected'}
        fprintf('DIRTH ("selection") direction and speed')
        scale_factor = .01;
        spd = hdfread(fname,'/wind_speed_selection');
        spd = double(spd) * scale_factor;
        
        scale_factor = .01;
        dir = hdfread(fname,'/wind_dir_selection');
        dir = double(dir) * scale_factor;
        
    case {'standard','selectedamb'}
        fprintf('Selected ambiguity from "Wind_speed" and "Wind_direction" variables')
        scale_factor = .01;
        spds = hdfread(fname,'/wind_speed');
        spds = double(spds) * scale_factor;
        
        scale_factor = .01;
        dirs = hdfread(fname,'/wind_dir');
        dirs = double(dirs) * scale_factor;
        
        selind = hdfread(fname,'/wvc_selection');
        
        selspd = zeros(size(selind));
        seldir = zeros(size(selind));
        for ambind = 1:4
            ind = find(selind == ambind);
            spdtmp = spds(:,:,ambind);
            dirtmp = dirs(:,:,ambind);
            selspd(ind) = spdtmp(ind);
            seldir(ind) = dirtmp(ind);
        end
        
        spd = selspd;
        dir = seldir;
        
    case 'model'
        fprintf('model direction and speed')
        
end

flag = hdfread(fname,'/wvc_quality_flag');

if putNaNs
    fprintf(' ... Putting NaNs')
    ind = (wvc_lat==0) & (wvc_lon==0);
    wvc_lat(ind) = nan;
    wvc_lon(ind) = nan;
    spd(ind) = nan;
    dir(ind) = nan;
end
fprintf('\n')

out.lat = wvc_lat;
out.lon = wvc_lon;
out.spd = spd;
out.dir = dir;
out.flag = flag;
out.wvcsize = wvcsize;
out.source = sprintf('QuikSCAT%dkm',wvcsize);

