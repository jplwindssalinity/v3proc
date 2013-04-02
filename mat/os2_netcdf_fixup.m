%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ORIGINAL AUTHOR: Thomas Werne
% COMPANY: Jet Propulsion Laboratory
% VERSION:
%
% File Name:     os2_netcdf_fixup.m
%
% $Author$
% $Date$
% $Revision$
%
% Copyright 2009-2012, by the California Institute of Technology.
% ALL RIGHTS RESERVED.  United States Government Sponsorship
% acknowledged. Any commercial use must be negotiated with the Office
% of Technology Transfer at the California Institute of Technology.
%
% This software may be subject to U.S. export control laws and
% regulations.  By accepting this document, the user agrees to comply
% with all U.S. export laws and regulations.  User has the
% responsibility to obtain export licenses, or other export authority
% as may be required before exporting such information to foreign
% countries or providing access to foreign persons.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function os2_netcdf_fixup(nc_file)
%
% os2_netcdf_fixup(nc_file)
%
% This function applies rain bias correction and sets up the ancillary data
% descriptors in the NetCDF file.
%
% --- taw, 03-May-2012

%% Anonymous helper functions
n2s = @(num) num2str(num);
ncgetvar = @(ncid, var) netcdf.getVar(ncid, netcdf.inqVarID(ncid, var));
ncputvar = @(ncid, name, val) netcdf.putVar(ncid, ...
    netcdf.inqVarID(ncid, name), val);
ncgetatt = @(ncid, var, name) netcdf.getAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name);
%ncputatt = @(ncid, var, name, val) netcdf.putAtt(ncid, ...
%    netcdf.inqVarID(ncid, var), name, val);

%% Define "Constants"
global BIAS_FILE;
%BIAS_FILE = 'rel_bias_130212.mat';
BIAS_FILE = 'os2_rel_bias.mat'; % Obsoleted 13 Feb 2012, TAW
GLOBAL = netcdf.getConstant('GLOBAL');

display(['Using ' nc_file]);

ncid = netcdf.open(nc_file, 'WRITE');

%% Update data descriptors
netcdf.reDef(ncid);

history = netcdf.getAtt(ncid, GLOBAL, 'history');
[bt, ~] = dbstack();
func = bt.name;

% Ensure this function isn't called twice
if (~isempty(strfind(history, func)))
    netcdf.endDef(ncid);
    netcdf.close(ncid);
    return
end

ancillary_data_descriptors = netcdf.getAtt(ncid, GLOBAL, 'ancillary_data_descriptors');
ancillary_data_descriptors = [ancillary_data_descriptors, ...
    gather_config_files([dirname(nc_file) 'ISRO.rdf']), BIAS_FILE, 10];
netcdf.putAtt(ncid, GLOBAL, 'ancillary_data_descriptors', ...
    ancillary_data_descriptors);

utc_time = java.lang.System.currentTimeMillis/1000/86400 + ...
    datenum(1970,1,1,0,0,0);
utc_vec = datevec(utc_time);
year = utc_vec(1);
yday = sprintf('%03d', floor(utc_time - datenum(year, 1, 1, 0, 0, 0) + 1));
year = sprintf('%04d', year);
hour = sprintf('%02d', utc_vec(4));
mins = sprintf('%02d', utc_vec(5));
secs = sprintf('%02d', floor(utc_vec(6)));
exe_time = [year '-' yday 'T' hour ':' mins ':' secs '+0000'];
history = [history exe_time ' ' getenv('USER') ' matlab -r ' func '(''' nc_file ''')' 10];
netcdf.putAtt(ncid, GLOBAL, 'history', history);

netcdf.endDef(ncid);

%% Cross track speed bias
spd = ncgetvar(ncid, 'retrieved_wind_speed');
spd_fill = ncgetatt(ncid, 'retrieved_wind_speed', '_FillValue');
spd_valid = spd ~= spd_fill;
valid = spd_valid;

impact_thresh = 2.5;
rain_impact = ncgetvar(ncid, 'rain_impact');

xt_wind_speed_bias = compute_bias_adj_rainy(spd, rain_impact, impact_thresh);

xt_wind_speed_bias(~valid) = ncgetatt(ncid, 'cross_track_wind_speed_bias', ...
    '_FillValue');
ncputvar(ncid, 'cross_track_wind_speed_bias', xt_wind_speed_bias);

spd(valid) = spd(valid) - xt_wind_speed_bias(valid);
ncputvar(ncid, 'retrieved_wind_speed', spd);

% Update high/low wind speed bits
flags = uint16(ncgetvar(ncid, 'flags'));
% Unset the high & low wind speed bits for ALL retrievals
flags = bitand(flags, bitcmp(bitor(2^10, 2^11), 16));
% Then for valid retrievals, set the high/low wind speed bits
flags(valid) = bitor(flags(valid), uint16((2^10)*(spd(valid) > 30)));
flags(valid) = bitor(flags(valid), uint16((2^11)*(spd(valid) <  3)));
ncputvar(ncid, 'flags', int16(flags));

%% Close files
netcdf.close(ncid);

return
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Extracts configuration files from a .rdf file
function retstring = gather_config_files(src_file)

retstring_len = 0;

% Configuration file strings (and order) to be returned
keywords = {...
    'QS_LANDMAP_FILE', 'QS_ICEMAP_FILE', 'GMF_FILE', 'KPRS_FILE', ...
    'KPM_FILE', 'ATTEN_MAP_FILE', 'NUDGE_WINDFIELD_FILE', ...
    'NEURAL_NET_LIQUID1_FILE', 'NEURAL_NET_SPEED1_FILE', ...
    'NEURAL_NET_SPEED2_FILE', 'NEURAL_NET_RAINFLAG_FILE', ...
    };

% Initialize a configuration structure
configs = cell(length(keywords), 1);

fid = fopen(src_file);

while 1
    % Pick off a line of text
    tline = fgetl(fid);
    if ~ischar(tline), break, end
    
    % Split the text at the equals 
    dlmidx = strfind(tline, '=');
    key = strtrim(tline(1:dlmidx - 1));
    data = strtrim(tline(dlmidx + 1:end));
    
    % See if the key matches one of the keywords
    for k = 1:length(keywords)
        if (1 == strcmp(key,  keywords{k}))
            % It does, so pick the file name off its base directory
            configs{k} = basename(data);
            % Increase the length of the final string (+1 for \n)
            retstring_len = retstring_len + (1 + length(configs{k}));
        end
    end
end
fclose(fid);

retstring = char(zeros(1, retstring_len));
start_idx = 1;
% Iterate over the keywords again, this time concatenating them
for k = 1:length(keywords)
    end_idx = start_idx + length(configs{k});
    retstring(start_idx:end_idx) = sprintf('%s\n', configs{k});
    start_idx = end_idx + 1;
end

% Kill the final newline
retstring = retstring(1:(end - 1));

return
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function bias_adj = compute_bias_adj_rainy(nc_spd, nc_ri, impact_thresh)
% Computes bias adjustment (from A. Fore)

%% Load in the cross track bias map and keep it resident
persistent relspdbias spdvec spdvec2 table2_rainfree table2_rainy;
if isempty(relspdbias)
    global BIAS_FILE;
    load(BIAS_FILE);
    % remove -9999 from table; it just messes things up.
    relspdbias(isnan(relspdbias)) = 0;
    % pad table so it does nearest for extrapolation, but linear for 
    % interpolation.
    spdvec   = (1:size(relspdbias,2)) - 0.5;
    spdvec2  = [0 spdvec 100];
    
    table2_rainfree = [ relspdbias(:,1,1) relspdbias(:,:,1) ...
        relspdbias(:,end,1) ];
    table2_rainy    = [ relspdbias(:,1,2) relspdbias(:,:,2) ...
        relspdbias(:,end,2) ];
end


bias_adj_rainfree = zeros(size(nc_spd));
bias_adj_rainy    = zeros(size(nc_spd));

m = size(nc_spd, 1);

for cti=1:m
  sel = find( nc_spd(cti,:) > 0 );
  
  bias_adj_rainfree(cti,sel) = ...
    interp1( spdvec2, table2_rainfree(cti,:), nc_spd(cti,sel), 'linear' );
  
  bias_adj_rainy(cti,sel)    = ...
    interp1( spdvec2, table2_rainy(cti,:), nc_spd(cti,sel), 'linear' );
end

bias_adj = zeros(size(nc_spd));
bias_adj((nc_ri  > impact_thresh) & (nc_spd > 0)) = ...
    bias_adj_rainy((nc_ri > impact_thresh) & (nc_spd > 0));
bias_adj((nc_ri <= impact_thresh) & (nc_spd > 0)) = ...
    bias_adj_rainfree((nc_ri <= impact_thresh) & (nc_spd > 0));

return
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function base = basename(filepath)
% Emulate the `basename` command

splits = strfind(filepath, '/');
if (isempty(splits))
    base = filepath;
else
    base = filepath(splits(end) + 1:end);
end

return
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function base = dirname(filepath)
% Emulate the `dirname` command

splits = strfind(filepath, '/');
if (isempty(splits))
    base = './';
else
    base = filepath(1:splits(end));
end

return
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
