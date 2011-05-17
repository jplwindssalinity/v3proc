%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ORIGINAL AUTHOR: Thomas Werne
% COMPANY: Jet Propulsion Laboratory
% VERSION:
%
% File Name:     qs_convert_netcdf.m
% Creation Date: 26 Apr 2011
%
% $Author$
% $Date$
% $Revision$
%
% Copyright 2009-2011, by the California Institute of Technology.
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

function qs_convert_netcdf(orig_nc_file, e2b12_dir)
%
% qs_convert_netcdf(orig_nc_file, e2b12_dir)
%
% This function is used to convert the "old style" QuikSCAT NetCDF files 
% to the "new style" L2B.nc/L2C.nc files.  Some attributes are 
% added/modified to conform with PO.DAAC requirements.
%
% The given NetCDF file (orig_nc_file) and appropriate E1B12 file (found in
% a year-indexed subdirectory of e2b12_dir) are used to produce two new 
% NetCDF files.
%
% --- taw, 04-May-2011

%% Load in the cross track bias map and keep it resident
persistent relspdbias2year;
if isempty(relspdbias2year)
    load 'crosstrackbiasmap.mat'
end

%% Anonymous helper functions
ncgetvar = @(ncid, var) netcdf.getVar(ncid, netcdf.inqVarID(ncid, var));
ncputvar = @(ncid, name, val) netcdf.putVar(ncid, ...
    netcdf.inqVarID(ncid, name), val);
ncgetatt = @(ncid, var, name) netcdf.getAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name);
ncputatt = @(ncid, var, name, val) netcdf.putAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name, val);


%% Define "Constants"
SRC = 1;
UNFILT = 2;
FILT = 3;
AT_IDX = 1;
XT_IDX = 2;
TS_IDX = 3;

GLOBAL = netcdf.getConstant('GLOBAL');

EPOCH = datenum('1/1/1999', 'mm/dd/yyyy');

%% Variables to copy
% Defines a mapping of variables from the source to the filtered and
% unfiltered targets.
% FMT: {source, unfiltered, filtered}
var_mapping = ...
    { 'lat', 'lat', 'lat'; ...
     'lon', 'lon', 'lon'; ...
     'wind_speed', 'retrieved_wind_speed', 'wind_speed'; ...
     'wind_to_direction', 'retrieved_wind_direction' ...
        'wind_direction'; ...
     'rain_impact', 'rain_impact', ''; ...
     'wind_divergence', '', 'wind_divergence'; ...
     'wind_curl', '', 'wind_curl'; ...
     'stress', '', 'stress'; ...
     'stress_divergence', '', 'stress_divergence'; ...
     'stress_curl', '', 'stress_curl'; ...
     'flags', 'flags', 'flags'; ...
     'eflags', 'eflags', 'eflags'; ...
     'model_wind_speed', 'nudge_wind_speed', ''; ...
     'model_wind_direction', 'nudge_wind_direction', ''; ...
     'wind_speed_uncorrected', 'retrieved_wind_speed_uncorrected', ''; ...
     'num_ambiguities', 'num_ambiguities', ''};

src.file = orig_nc_file;
src.ncid = netcdf.open(src.file, 'NOWRITE');

%% Add standard_name attributes
unfilt.standard_names = {'lon', 'longitude'; ...
                         'lat', 'latitude'; ...
                         'time', 'time'; ...
                         'retrieved_wind_speed', 'wind_speed'; ...
                         'retrieved_wind_speed_uncorrected', ...
                            'wind_speed';
                         'retrieved_wind_direction', ...
                            'wind_to_direction';
                         'nudge_wind_speed', 'wind_speed'; ...
                         'nudge_wind_direction', 'wind_to_direction'};
filt.standard_names = {'lon', 'longitude'; ...
                       'lat', 'latitude'; ...
                       'time', 'time'; ...
                       'wind_speed', 'wind_speed'; ...
                       'stress', ...
                        'magnitude_of_surface_downward_stress';
                       'wind_direction', 'wind_to_direction'; ...
                       'wind_divergence', 'divergence_of_wind'; ...
                       'stress_divergence', ...
                            'divergence_of_surface_downward_stress'; ...
                       'wind_curl', 'atmosphere_relative_vorticity'; ...
                       'ecmwf_wind_speed', 'wind_speed'; ...
                       'ecmwf_wind_direction', 'wind_to_direction'};
                         

%% Get dimensions
dimid_mapping = zeros(3, 3);

% Build a table with the correspondences between the dimension ids across
% the three files
dimid_mapping(AT_IDX, SRC) = netcdf.inqDimID(src.ncid, 'along_track');
dimid_mapping(XT_IDX, SRC) = netcdf.inqDimID(src.ncid, 'cross_track');
dimid_mapping(TS_IDX, SRC) = netcdf.inqDimID(src.ncid, 'time_strlength');

[~, at_len] = netcdf.inqDim(src.ncid, dimid_mapping(AT_IDX, SRC));
[~, xt_len] = netcdf.inqDim(src.ncid, dimid_mapping(XT_IDX, SRC));
[~, ts_len] = netcdf.inqDim(src.ncid, dimid_mapping(TS_IDX, SRC));

%% Compute new file name
run_no = netcdf.getAtt(src.ncid, GLOBAL, 'StopOrbitNumber');
run_date = netcdf.getAtt(src.ncid, GLOBAL, 'RangeBeginningDate');
run_time = netcdf.getAtt(src.ncid, GLOBAL, 'RangeBeginningTime');
run_year = run_date(1:4);
run_doy = str2double(run_date(6:end));
run_hr  = run_time(1:2);
run_min = run_time(4:5);

run_date = datenum(['1/0/', run_year]) + run_doy;

run_month = datestr(run_date, 'mm');
run_dom = datestr(run_date, 'dd');

unfilt.filename = sprintf('qs_l2b_%05d_v3_%s%s%s%s%s.nc', run_no, ...
    run_year, run_month, run_dom, run_hr, run_min);
unfilt.file     = regexprep(src.file, '[^/]*$', unfilt.filename);
filt.filename   = sprintf('qs_l2c_%05d_v3_%s%s%s%s%s.nc', run_no, ...
    run_year, run_month, run_dom, run_hr, run_min);
filt.file       = regexprep(src.file, '[^/]*$', filt.filename);

%% Open the product files
unfilt.ncid = netcdf.create(unfilt.file, 'CLASSIC_MODEL');
filt.ncid   = netcdf.create(filt.file,   'CLASSIC_MODEL');

dimid_mapping(AT_IDX, UNFILT) = netcdf.defDim(unfilt.ncid, ...
    'along_track', at_len);
dimid_mapping(XT_IDX, UNFILT) = netcdf.defDim(unfilt.ncid, ...
    'cross_track', xt_len);
dimid_mapping(TS_IDX, UNFILT) = netcdf.defDim(unfilt.ncid, ...
    'time_strlength', ts_len);
dimid_mapping(AT_IDX, FILT) = netcdf.defDim(filt.ncid, ...
    'along_track', at_len);
dimid_mapping(XT_IDX, FILT) = netcdf.defDim(filt.ncid, ...
    'cross_track', xt_len);
dimid_mapping(TS_IDX, FILT) = netcdf.defDim(filt.ncid, ...
    'time_strlength', ts_len);

%% Copy the global attributes
[~, ~, ngatts, ~] = netcdf.inq(src.ncid);
for k = 0:ngatts - 1
    name = netcdf.inqAttName(src.ncid, GLOBAL, k);
    netcdf.copyAtt(src.ncid, GLOBAL, name, unfilt.ncid, GLOBAL);
    netcdf.copyAtt(src.ncid, GLOBAL, name, filt.ncid, GLOBAL);
end

%% Copy variable definitions
[nvar, ~] = size(var_mapping);
for k = 1:nvar
    % copy the information from the old file
    varid_src = netcdf.inqVarID(src.ncid, var_mapping{k, SRC});
    [~, xtype, dimids, natts] = netcdf.inqVar(src.ncid, varid_src);

    % Given source dimension ids, lookup the index in the dimid LUT
    [X, Y] = meshgrid(dimid_mapping(:, SRC), dimids);
    [~, dimids] = find(X == Y);

    % Unfiltered file
    if (~isempty(var_mapping{k, UNFILT}))
        varid_tgt = netcdf.defVar(unfilt.ncid, var_mapping{k, UNFILT}, ...
            xtype, dimid_mapping(dimids, UNFILT));

        % Copy the variable attributes
        for m = 0:natts - 1
            name = netcdf.inqAttName(src.ncid, varid_src, m);
            netcdf.copyAtt(src.ncid, varid_src, name, ...
                unfilt.ncid, varid_tgt);
        end
    end

    % Filtered file
    if (~isempty(var_mapping{k, FILT}))
        varid_tgt = netcdf.defVar(filt.ncid, var_mapping{k, FILT}, ...
            xtype, dimid_mapping(dimids, FILT));

        % Copy the variable attributes
        for m = 0:natts - 1
            name = netcdf.inqAttName(src.ncid, varid_src, m);
            netcdf.copyAtt(src.ncid, varid_src, name, ...
                filt.ncid, varid_tgt);
        end
    end
end

%% Build the new variables
% time
varid = netcdf.defVar(unfilt.ncid, 'time', 'float', ...
    dimid_mapping(AT_IDX, UNFILT));
netcdf.putAtt(unfilt.ncid, varid, 'long_name', 'date and time');
netcdf.putAtt(unfilt.ncid, varid, 'units', ...
    'seconds since 1999-01-01 0:0:0');

varid = netcdf.defVar(filt.ncid, 'time', 'float', ...
    dimid_mapping(AT_IDX, FILT));
netcdf.putAtt(filt.ncid, varid, 'long_name', 'date and time');
netcdf.putAtt(filt.ncid, varid, 'units', 'seconds since 1999-01-01 0:0:0');

% cross-track-wind-speed bias
varid = netcdf.defVar(unfilt.ncid, 'cross_track_wind_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(unfilt.ncid, varid, 'long_name', ['relative wind speed ' ...
    'bias with respect to sweet spot']);
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', single(1.0));

% atmospheric speed bias
varid = netcdf.defVar(unfilt.ncid, 'atmospheric_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(unfilt.ncid, varid, 'long_name', 'atmospheric speed bias');
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', single(1.0));

% ECMWF wind speed
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_speed', ...
    'float', [dimid_mapping(AT_IDX, FILT), ...
              dimid_mapping(XT_IDX, FILT)]);
netcdf.putAtt(filt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(filt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(filt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(filt.ncid, varid, 'long_name', 'model wind speed');
netcdf.putAtt(filt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(filt.ncid, varid, 'scale_factor', single(1.0));

% ECMWF wind direction
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_direction', ...
    'float', [dimid_mapping(AT_IDX, FILT), ...
              dimid_mapping(XT_IDX, FILT)]);
netcdf.putAtt(filt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(filt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(filt.ncid, varid, 'valid_max', single(360.0));
netcdf.putAtt(filt.ncid, varid, 'long_name', 'model wind direction');
netcdf.putAtt(filt.ncid, varid, 'units', 'degrees');
netcdf.putAtt(filt.ncid, varid, 'scale_factor', single(1.0));

%% Alter copied definitions
% Correct erroneous ProductionDateTime.  Use the access date/time for the
% source .nc file (at this point it's the best we have)
finfo = dir(src.file);
proc_date = datevec(finfo.date);

% Convert MM/DD -> DOY
proc_doy = floor(datenum(finfo.date) - ...
    datenum(['1/0/' num2str(proc_date(1))]));

proc_date_att = sprintf('%04d-%03dT%02d:%02d:%02d.%03d', ...
    proc_date(1), proc_doy, proc_date(4), proc_date(5), proc_date(6), 0);
netcdf.putAtt(unfilt.ncid, GLOBAL, 'ProductionDateTime', proc_date_att);
netcdf.putAtt(filt.ncid,   GLOBAL, 'ProductionDateTime', proc_date_att);

% Fix long and short name attributes
netcdf.putAtt(unfilt.ncid, GLOBAL, 'ShortName', ...
    'QSCAT_LEVEL_2B_OWV_COMP_12');
netcdf.putAtt(filt.ncid, GLOBAL, 'ShortName', ...
    'QSCAT_LEVEL_2C_SFOWSV_COMP_12');

netcdf.putAtt(unfilt.ncid, GLOBAL, 'LongName', ...
    'QuikSCAT Level 2B Ocean Wind Vectors in 12.5km Slice Composites');
netcdf.putAtt(filt.ncid, GLOBAL, 'LongName', ...
    ['QuikSCAT Level 2C Spatially Filtered Ocean Wind and Stress ' ...
     'Vectors in 12.5km Slice Composites']);

% Set the standard_name attributes
for k = 1:length(unfilt.standard_names)
    ncputatt(unfilt.ncid, unfilt.standard_names{k, 1}, 'standard_name', ...
        unfilt.standard_names{k, 2});
end
for k = 1:length(filt.standard_names)
    ncputatt(filt.ncid, filt.standard_names{k, 1}, 'standard_name', ...
        filt.standard_names{k, 2});
end

% Granule Pointers
netcdf.putAtt(unfilt.ncid, GLOBAL, 'GranulePointer', unfilt.filename);
netcdf.putAtt(filt.ncid, GLOBAL, 'GranulePointer', filt.filename);
netcdf.delAtt(unfilt.ncid, GLOBAL, 'QAGranulePointer');
netcdf.delAtt(filt.ncid, GLOBAL, 'QAGranulePointer');

% Add new history attribute
netcdf.putAtt(unfilt.ncid, GLOBAL, 'history', '');
netcdf.putAtt(filt.ncid, GLOBAL, 'history', '');

% Set units in the flags variables
ncputatt(unfilt.ncid, 'flags',  'units', 'bit');
ncputatt(filt.ncid,   'flags',  'units', 'bit');
ncputatt(unfilt.ncid, 'eflags', 'units', 'bit');
ncputatt(filt.ncid,   'eflags', 'units', 'bit');

% Change the units of certain variables...
ncputatt(filt.ncid, 'stress', 'units', 'N m-2');
ncputatt(filt.ncid, 'stress_divergence', 'units', 'N m-3');
ncputatt(filt.ncid, 'stress_curl', 'units', 'N m-3');

% Change the Conventions attribute value
netcdf.putAtt(unfilt.ncid, GLOBAL, 'Conventions', 'CF-1.4');
netcdf.putAtt(filt.ncid,   GLOBAL, 'Conventions', 'CF-1.4');

% Set algorithm description
netcdf.putAtt(unfilt.ncid, GLOBAL, 'l2a_algorithm_descriptor', ...
    ['Uses Ku2011 GMF from Remote Sensing Systems and Neural ', ...
    'Network Rain Correction']);
netcdf.putAtt(filt.ncid, GLOBAL, 'l2a_algorithm_descriptor', ...
    ['Uses Ku2011 GMF from Remote Sensing Systems and Neural ', ...
    'Network Rain Correction']);

%% End the definitions stage
netcdf.endDef(unfilt.ncid);
netcdf.endDef(filt.ncid);

%% Copy over the data
[nvar, ~] = size(var_mapping);
for k = 1:nvar
    data = ncgetvar(src.ncid, var_mapping{k, SRC});

    if (~isempty(var_mapping{k, UNFILT}))
        ncputvar(unfilt.ncid, var_mapping{k, UNFILT}, data);
    end
    if (~isempty(var_mapping{k, FILT}))
        ncputvar(filt.ncid, var_mapping{k, FILT}, data);
    end
end

%% Fill in the new data
% Time
sampletimes = ncgetvar(src.ncid, 'time');
sampleyears = sampletimes(1:4, :);
sampledoys  = sampletimes(6:8, :);
samplehours = sampletimes(10:11, :);
samplemins  = sampletimes(13:14, :);
samplesecs  = sampletimes(16:17, :);
samplemsecs = sampletimes(19:21, :); 

sampletimes = zeros(at_len, 6);
for k = 1:at_len
    sampletimes(k, :) = datevec(datenum(['1/0/' sampleyears(:, k)'], ...
        'mm/dd/yyyy') + str2double(sampledoys(:, k)));    
    sampletimes(k, 4:6) = [str2double(samplehours(:, k)), ...
                str2double(samplemins(:, k)), ...
                str2double(samplesecs(:, k)) + ...
                str2double(samplemsecs(:, k))/1000];
end

sampletimes = 24*60*60*(datenum(sampletimes) - EPOCH); % in seconds
ncputvar(unfilt.ncid, 'time', sampletimes);
ncputvar(filt.ncid, 'time', sampletimes);

% ECMWF
data = '';
e2b12.file = sprintf('%s/%s/E2B_%05d.cp12.dat', ...
    e2b12_dir, run_year, run_no);

% Check to see if the file exists
if (exist(e2b12.file, 'file') ~= 0)
    data = read_qs_e2b12(e2b12.file);
else warning(['Can''t find ' e2b12.file]); %#ok
    data.spd = ncgetatt(filt.ncid, 'ecmwf_wind_speed', ...
        'FillValue').*ones(xt_len, at_len);
    data.dir = ncgetatt(filt.ncid, 'ecmwf_wind_direction', ...
        'FillValue').*ones(xt_len, at_len);
end
ncputvar(filt.ncid, 'ecmwf_wind_speed', data.spd);
ncputvar(filt.ncid, 'ecmwf_wind_direction', data.dir);

% Cross track speed bias
impact_thresh = 2.5;
rain_impact = ncgetvar(unfilt.ncid, 'rain_impact');
speed = ncgetvar(unfilt.ncid, 'retrieved_wind_speed');

% index into dimension 3 of cross track speed bias table
impact_idx = 1 + double(rain_impact > impact_thresh);

% used during interpolation
ref_speed = (0.5:19.5)';
dbias = diff(relspdbias2year, [], 2);

lower_idx = floor(speed - 0.5) + 1;
lower_idx(lower_idx <  1) =  1;
lower_idx(lower_idx > 20) = 20;

% pre-allocate for speed
data = zeros(size(speed));

% vector-ized interpolation
% Step through each column separately
[m, n] = size(speed);
for c = 1:n
    % pick off relevant columns so we can do logical indexing
    lower_col  = lower_idx(:, c); % copy-on-write => this is an alias
    impact_col = impact_idx(:, c);
    speed_col  = speed(:, c);
    data_col   = zeros(m, 1);

    % step through the two rain impact cases
    for k = 1:2
        dbias_col = dbias(c, :, k)';
        bias_col   = relspdbias2year(c, :, k)';

        % If it's the highest bin, saturate
        idx = (lower_col == 20) & (impact_col == k);
        data_col(idx) = bias_col(lower_col(idx));

        % Otherwise, interpolate
        idx = (lower_col < 20) & (impact_col == k);
        data_col(idx) = bias_col(lower_col(idx)) + (speed_col(idx) - ...
            ref_speed(lower_col(idx))).*dbias_col(lower_col(idx));

    end
    % Insert into the data structure
    data(:, c) = data_col;
end

% Account for invalid cells
data(speed == ncgetatt(unfilt.ncid, 'retrieved_wind_speed', ...
    'FillValue')) = ncgetatt(unfilt.ncid, ...
    'cross_track_wind_speed_bias', 'FillValue');
% Insert data structure into the file
ncputvar(unfilt.ncid, 'cross_track_wind_speed_bias', data);

wind_speed = ncgetvar(unfilt.ncid, 'retrieved_wind_speed');
wind_speed_fill = ncgetatt(unfilt.ncid, 'retrieved_wind_speed', ...
    'FillValue');
val_idx = (wind_speed ~= wind_speed_fill);

wind_speed(val_idx) = wind_speed(val_idx) - data(val_idx);
ncputvar(unfilt.ncid, 'retrieved_wind_speed', wind_speed);

% Atmospheric speed bias
% assume that the l2b.dat file is in the same directory as the .nc file
l2bfile = regexprep(src.file, '[^/]*$', 'l2b.dat');
ncputvar(unfilt.ncid, 'atmospheric_speed_bias', ...
    l2b_extraction(l2bfile, 'speedBias', ncgetatt(unfilt.ncid, ...
    'atmospheric_speed_bias', 'FillValue')));


%% Close files
netcdf.close(src.ncid);
netcdf.close(unfilt.ncid);
netcdf.close(filt.ncid);

return
