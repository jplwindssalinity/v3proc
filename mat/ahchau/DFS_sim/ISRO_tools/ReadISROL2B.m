function out = ReadISROL2B(fname,varargin)
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
    strcmp(x,'selected') | strcmp(x,'model') | strcmpi(x,'standard') | ...
    strcmpi(x,'selectedamb')) );
p.addParamValue('putNaNs',1,@(x)(x==0 | x==1))
p.parse(fname, varargin{:});
retrievaltype = p.Results.retrieval;
putNaNs = p.Results.putNaNs;


fprintf('Reading from %s ... ',fname)

hinfo=hdf5info(fname); % get info from hdf5 file
DatasetNames = {hinfo.GroupHierarchy.Groups.Datasets.Name};

% Latitude: read in the data and scale factor, and convert
ind = find(strcmp(DatasetNames,'/science_data/Latitude'));
[data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(ind),'ReadAttributes',true);
A=hdf5read(fname,'science_data','Latitude Scale');
scl = str2num(A.Data);
Olat = double(data) * scl;

% Longitude: read in the data and scale factor, and convert
ind = find(strcmp(DatasetNames,'/science_data/Longitude'));
[data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(ind),'ReadAttributes',true);
A=hdf5read(fname,'science_data','Longitude Scale');
scl = str2num(A.Data);
Olon = double(data) * scl;

% find the indicies of the speed/direction to read
switch retrievaltype
    case {'selection','selected'}
        fprintf('selected direction and speed')
        spdind = find(strcmp(DatasetNames,'/science_data/Wind_speed_selection'));
        dirind = find(strcmp(DatasetNames,'/science_data/Wind_direction_selection'));
    case {'standard','selectedamb'}
        fprintf('Will get selected ambiguity from "Wind_speed" and "Wind_direction" variables')
        spdind = find(strcmp(DatasetNames,'/science_data/Wind_speed'));
        dirind = find(strcmp(DatasetNames,'/science_data/Wind_direction'));
    case 'model'
        fprintf('model direction and speed')
        spdind = find(strcmp(DatasetNames,'/science_data/Model_speed'));
        dirind = find(strcmp(DatasetNames,'/science_data/Model_direction'));      
end

% Speed: read in the data and scale factor, and convert
[data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(spdind),'ReadAttributes',true);
A=hdf5read(fname,'science_data','Wind Speed Selection Scale');
scl = str2num(A.Data);
Ospd = double(data) * scl;
% Direction: read in the data and scale factor, and convert
[data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(dirind),'ReadAttributes',true);
A=hdf5read(fname,'science_data','Wind Direction Selection Scale');
scl = str2num(A.Data);
Odir = double(data) * scl;


switch retrievaltype
    case {'standard', 'selectedamb'}
        %         fprintf('Choosing "selected from all ambiguities"')
        selind = find(strcmp(DatasetNames,'/science_data/WVC_selection'));
        [data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(selind),'ReadAttributes',true);
        selind = data;
        
        selspd = zeros(size(squeeze(Ospd(1,:,:))));
        seldir = zeros(size(squeeze(Odir(1,:,:))));
        
        for ambind = 1:6
            ind = find(selind == ambind);
            spdtmp = Ospd(ambind,:,:);
            dirtmp = Odir(ambind,:,:);
            selspd(ind) = spdtmp(ind);
            seldir(ind) = dirtmp(ind);
        end
        
        Ospd = selspd;
        Odir = seldir;
end

[data,attr]=hdf5read(hinfo.GroupHierarchy.Groups.Datasets(9),'ReadAttributes',true);
Oflag = data;

A=hdf5read(fname,'science_data','WVC Size');

Owvcsize = str2num(A.Data);

if putNaNs
    fprintf(' ... Putting NaNs')
    ind = (Olat==0) & (Olon==0);
    Olat(ind) = nan;
    Olon(ind) = nan;
    Ospd(ind) = nan;
    Odir(ind) = nan;
end
fprintf('\n')

out.lat = Olat';
out.lon = Olon';
out.spd = Ospd';
out.dir = Odir';
out.flag = Oflag';
out.wvcsize = Owvcsize;
out.source = 'ISRO';

