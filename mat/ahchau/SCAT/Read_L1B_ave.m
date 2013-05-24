function out = Read_L1B_ave(filename,varargin);
% function out = Read_L1B_ave(filename,varargin);
% optional parameter pair inputs:
%     'endian' : [ {'l'} | 'b' ] 
%     'nanland': [ {1} | 0]
% Created by A. Chau 7/7/11

%%%%%%%%%%%%%%%%%%%%%%%%
%%% Parse the inputs %%%
%%%%%%%%%%%%%%%%%%%%%%%%
p = inputParser;
p.addRequired('filename', @ischar);
p.addParamValue('endian','l',@(x)(strcmp(x,'l') | strcmp(x,'b')) );
p.addParamValue('nanland',1,@(x)(x==0 | x==1));
p.parse(filename,varargin{:});


%%% Check if file exists %%%
if exist(filename)
    %     fprintf('Reading %s\n',filename);
else
    %     error('Uh oh! This file does not exist: %s',filename);
    fprintf('UH OH! FILE DOES NOT EXIST: %s',filename);
    out = nan;
end


%%%%%%%%%%%%%%%%%%%%%
%%% Read the file %%%
%%%%%%%%%%%%%%%%%%%%%
% Read one field of all records, skipping the appropriate number of bytes,
%      rewind file, start again reading the next field.
% This is much faster than reading records one at a time (factor of 10 to
% 100)

fid = fopen(filename,'r',p.Results.endian);
numbytesperrecord = 75;
out.timestr = char(fread(fid,[21,inf],'21*char',numbytesperrecord-21)');
fseek(fid,21,'bof'); ftell(fid);
out.time = fread(fid,[1,inf],'double',numbytesperrecord-8)';
fseek(fid,29,'bof');
out.lon = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,33,'bof');
out.lat = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,37,'bof');
out.fp_start = fread(fid,[1,inf],'int32',numbytesperrecord-4)';
fseek(fid,41,'bof');
out.fp_end = fread(fid,[1,inf],'int32',numbytesperrecord-4)';
fseek(fid,45,'bof');
out.npts = fread(fid,[1,inf],'int32',numbytesperrecord-4)';
fseek(fid,49,'bof');
out.sig0 = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,53,'bof');
out.incang = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,57,'bof');
out.cellazi = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,61,'bof');
out.atten = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,65,'bof');
out.beam = fread(fid,[1,inf],'int8',numbytesperrecord-1)';
fseek(fid,66,'bof');
out.land = fread(fid,[1,inf],'int8',numbytesperrecord-1)';
fseek(fid,67,'bof');
out.ecmwf_speed = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fseek(fid,71,'bof');
out.ecmwf_dir = fread(fid,[1,inf],'float32',numbytesperrecord-4)';
fclose(fid);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Perform optional filtering %%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Where the land flag is true, put nans in selected outputs
if p.Results.nanland 
    ind = (out.land == 1);
    out.lon(ind) = nan;
    out.lat(ind) = nan;
    out.sig0(ind) = nan;
    out.ecmwf_speed(ind) = nan;
    out.ecmwf_dir(ind) = nan;
end

end