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
persistent relspdbias2year; %#ok
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


%% Define variables for later use
build_id = 'reproc_v1_0_0:A';

l2b_algorithm_descriptor = sprintf([...
'Uses Ku2011 GMF from Remote Sensing Systems.  Applies median filter technique for ambiguity\n' ...
'removal.  Ambiguity removal median filter is based on wind vectors over a 7\n' ...
'by 7 wind vector cell window.  Applies no median filter weights.\n' ...
'Enhances the direction of the selected ambiguity based on the range of\n' ...
'directions which exceed a specified probability threshold.\n' ...
'Applies multi-pass median filter technique to reduce the effects of rain\n' ...
'flagged cells on ambiguity selection.\n' ...
'Assigns values from mp_rain_probability, nof_rain_index, and bits 12, 13\n' ...
'of wvc_quality_flag in the L2B pulse 25km product.  Applies Neural Network Rain Correction.']);

FLAG_FILL = 32767;


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
%     'stress', '', 'stress'; ...
%     'stress_divergence', '', 'stress_divergence'; ...
%     'stress_curl', '', 'stress_curl'; ...
     'flags', 'flags', 'flags'; ...
     'eflags', 'eflags', 'eflags'; ...
     'model_wind_speed', 'nudge_wind_speed', ''; ...
     'model_wind_direction', 'nudge_wind_direction', ''; ...
     'wind_speed_uncorrected', 'retrieved_wind_speed_uncorrected', ''; ...
     'num_ambiguities', 'num_ambiguities', ''};

src.file = orig_nc_file
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
%                       'stress', ...
%                        'magnitude_of_surface_downward_stress';
                       'wind_direction', 'wind_to_direction'; ...
                       'wind_divergence', 'divergence_of_wind'; ...
%                       'stress_divergence', ...
%                            'divergence_of_surface_downward_stress'; ...
%                       'wind_curl', 'atmosphere_relative_vorticity'; ...
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
dimid_mapping(TS_IDX, UNFILT) = -1; % Not implemented
dimid_mapping(AT_IDX, FILT) = netcdf.defDim(filt.ncid, ...
    'along_track', at_len);
dimid_mapping(XT_IDX, FILT) = netcdf.defDim(filt.ncid, ...
    'cross_track', xt_len);
dimid_mapping(TS_IDX, FILT) = -1; % Not implemented

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
    [dimids, ~] = find(X == Y);

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
    'float', fliplr([dimid_mapping(AT_IDX, UNFILT), ...
                     dimid_mapping(XT_IDX, UNFILT)]));
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(unfilt.ncid, varid, 'long_name', ['relative wind speed ' ...
    'bias with respect to sweet spot']);
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', single(1.0));

% atmospheric speed bias
varid = netcdf.defVar(unfilt.ncid, 'atmospheric_speed_bias', ...
    'float', fliplr([dimid_mapping(AT_IDX, UNFILT), ...
                     dimid_mapping(XT_IDX, UNFILT)]));
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(unfilt.ncid, varid, 'long_name', 'atmospheric speed bias');
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', single(1.0));

% ECMWF wind speed
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_speed', ...
    'float', fliplr([dimid_mapping(AT_IDX, FILT), ...
                     dimid_mapping(XT_IDX, FILT)]));
netcdf.putAtt(filt.ncid, varid, 'FillValue', single(-9999.0));
netcdf.putAtt(filt.ncid, varid, 'valid_min', single(0.0));
netcdf.putAtt(filt.ncid, varid, 'valid_max', single(100.0));
netcdf.putAtt(filt.ncid, varid, 'long_name', 'model wind speed');
netcdf.putAtt(filt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(filt.ncid, varid, 'scale_factor', single(1.0));

% ECMWF wind direction
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_direction', ...
    'float', fliplr([dimid_mapping(AT_IDX, FILT), ...
                     dimid_mapping(XT_IDX, FILT)]));
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
netcdf.putAtt(filt.ncid,   GLOBAL, 'GranulePointer', filt.filename);
netcdf.delAtt(unfilt.ncid, GLOBAL, 'QAGranulePointer');
netcdf.delAtt(filt.ncid,   GLOBAL, 'QAGranulePointer');

% Update InputPointer
netcdf.putAtt(unfilt.ncid, GLOBAL, 'InputPointer', ...
    basename(netcdf.getAtt(unfilt.ncid, GLOBAL, 'source_file')));
netcdf.putAtt(filt.ncid,   GLOBAL, 'InputPointer', ...
    basename(netcdf.getAtt(filt.ncid,   GLOBAL, 'source_file')));
netcdf.delAtt(unfilt.ncid, GLOBAL, 'source_file');
netcdf.delAtt(filt.ncid,   GLOBAL, 'source_file');

% Add new history attribute
netcdf.putAtt(unfilt.ncid, GLOBAL, 'history', '');
netcdf.putAtt(filt.ncid, GLOBAL, 'history', '');

% Set units in the flags variables
ncputatt(unfilt.ncid, 'flags',  'units', 'bit');
ncputatt(filt.ncid,   'flags',  'units', 'bit');
ncputatt(unfilt.ncid, 'eflags', 'units', 'bit');
ncputatt(filt.ncid,   'eflags', 'units', 'bit');

% Change the units of certain variables...
%ncputatt(filt.ncid, 'stress', 'units', 'N m-2');
%ncputatt(filt.ncid, 'stress_divergence', 'units', 'N m-3');
%ncputatt(filt.ncid, 'stress_curl', 'units', 'N m-3');

% Change the Conventions attribute value
netcdf.putAtt(unfilt.ncid, GLOBAL, 'Conventions', 'CF-1.4');
netcdf.putAtt(filt.ncid,   GLOBAL, 'Conventions', 'CF-1.4');

% Set algorithm description
netcdf.putAtt(unfilt.ncid, GLOBAL, 'l2b_algorithm_descriptor', ...
    l2b_algorithm_descriptor);
netcdf.putAtt(filt.ncid, GLOBAL, 'l2b_algorithm_descriptor', ...
    l2b_algorithm_descriptor);

% Revise version attributes
netcdf.delAtt(unfilt.ncid, GLOBAL, 'version_id_major');
netcdf.delAtt(filt.ncid, GLOBAL, 'version_id_major');
netcdf.delAtt(unfilt.ncid, GLOBAL, 'version_id_minor');
netcdf.delAtt(filt.ncid, GLOBAL, 'version_id_minor');
netcdf.delAtt(unfilt.ncid, GLOBAL, 'HDF_version_id');
netcdf.delAtt(filt.ncid, GLOBAL, 'HDF_version_id');

netcdf.putAtt(unfilt.ncid, GLOBAL, 'NetCDF_version_id', netcdf.inqLibVers);
netcdf.putAtt(filt.ncid, GLOBAL, 'NetCDF_version_id', netcdf.inqLibVers);

netcdf.putAtt(unfilt.ncid, GLOBAL, 'build_id', build_id);
netcdf.putAtt(filt.ncid, GLOBAL, 'build_id', build_id);

% Update data descriptors
ancillary_data_descriptors = gather_config_files(regexprep(src.file, ...
    '[^/]*$', 'QS.rdf'));

netcdf.putAtt(unfilt.ncid, GLOBAL, 'ancillary_data_descriptors', ...
    ancillary_data_descriptors);
netcdf.putAtt(filt.ncid, GLOBAL, 'ancillary_data_descriptors', ...
    ancillary_data_descriptors);

% Update fill values
ncputatt(unfilt.ncid, 'flags',  'FillValue', FLAG_FILL);
ncputatt(filt.ncid,   'flags',  'FillValue', FLAG_FILL);
ncputatt(unfilt.ncid, 'eflags', 'FillValue', FLAG_FILL);
ncputatt(filt.ncid,   'eflags', 'FillValue', FLAG_FILL);

% Update max rain impact
ncputatt(unfilt.ncid, 'rain_impact',  'valid_max', 100);

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

%% Correct eflags
eflags = uint16(ncgetvar(unfilt.ncid, 'eflags'));
rain_corr_perf = (eflags ~= FLAG_FILL) & bitand(eflags, 2^0);
eflags(rain_corr_perf) = bitand(eflags(rain_corr_perf), ...
    bitand(bitcmp(2^1, 16), bitcmp(2^3, 16)));
ncputvar(unfilt.ncid, 'eflags', int16(eflags));

eflags = uint16(ncgetvar(filt.ncid, 'eflags'));
rain_corr_perf = (eflags ~= FLAG_FILL) & bitand(eflags, 2^0);
eflags(rain_corr_perf) = bitand(eflags(rain_corr_perf), ...
    bitand(bitcmp(2^1, 16), bitcmp(2^3, 16)));
ncputvar(filt.ncid, 'eflags', int16(eflags));

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

data = compute_bias_adj_rainy(speed, relspdbias2year, rain_impact, ...
    impact_thresh);

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

% Update high/low wind speed bits
flags = uint16(ncgetvar(src.ncid, 'flags'));
% Unset the high & low wind speed bits for ALL retrievals
flags = bitand(flags, bitcmp(bitor(2^10, 2^11), 16));
% Then for valid retrievals, set the high/low wind speed bits
flags(val_idx) = bitor(flags(val_idx), ...
    uint16((2^10)*(wind_speed(val_idx) > 30)));
flags(val_idx) = bitor(flags(val_idx), ...
    uint16((2^11)*(wind_speed(val_idx) <  3)));
ncputvar(unfilt.ncid, 'flags', int16(flags));
ncputvar(filt.ncid,   'flags', int16(flags));

% Atmospheric speed bias
atmospheric_speed_bias = ncgetatt(unfilt.ncid, 'atmospheric_speed_bias', ...
    'FillValue')*ones(size(wind_speed));

retrieved_wind_speed_uncorr = ncgetvar(src.ncid, 'wind_speed_uncorrected');
xt_wind_speed_bias = ncgetvar(unfilt.ncid, 'cross_track_wind_speed_bias');

% bit 0 == 0 => rain correction applied
good = bitand(uint16(ncgetvar(unfilt.ncid, 'eflags')), 2^0) == 0; 
good = bitand(good, retrieved_wind_speed_uncorr ~= ncgetatt(src.ncid, ...
    'wind_speed_uncorrected', 'FillValue'));
good = bitand(good, wind_speed ~= wind_speed_fill);
good = bitand(good, xt_wind_speed_bias ~= ncgetatt(unfilt.ncid, ...
    'cross_track_wind_speed_bias', 'FillValue'));

atmospheric_speed_bias(good) = retrieved_wind_speed_uncorr(good) - ...
    wind_speed(good) - xt_wind_speed_bias(good) ;

ncputvar(unfilt.ncid, 'atmospheric_speed_bias', atmospheric_speed_bias);

%% Close files
netcdf.close(src.ncid);
netcdf.close(unfilt.ncid);
netcdf.close(filt.ncid);

return

%% Extracts configuration files from a .rdf file
function retstring = gather_config_files(src_file)

retstring_len = 0;

% Configuration file strings (and order) to be returned
keywords = {...
    'QS_LANDMAP_FILE', 'QS_ICEMAP_FILE', 'GMF_FILE', 'KPRS_FILE', ...
    'KPM_FILE', 'ATTEN_MAP_FILE', 'NUDGE_WINDFIELD_FILE', ...
    'NEURAL_NET_LIQUID1_FILE', 'NEURAL_NET_SPEED1_FILE', ...
    'NEURAL_NET_SPEED2_FILE', 'NEURAL_NET_RAINFLAG_FILE', 'L2B_HDF_FILE'...
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

function bias_adj = compute_bias_adj_rainy(nc_spd, relspdbias2year, ...
    nc_ri, impact_thresh)
% Computes bias adjustment (from A. Fore)

% remove -9999 from table; it just messes things up.
relspdbias2year(relspdbias2year==-9999) = 0;

% pad table so it does nearest for extrapolation, but linear for 
% interpolation.
spdvec   = (1:size(relspdbias2year,2)) - 0.5;
spdvec2  = [0 spdvec 100];

table2_rainfree = [ relspdbias2year(:,1,1) relspdbias2year(:,:,1) ...
    relspdbias2year(:,end,1) ];
table2_rainy    = [ relspdbias2year(:,1,2) relspdbias2year(:,:,2) ...
    relspdbias2year(:,end,2) ];

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

function base = basename(filepath)
% Emulate the `basename` command

splits = strfind(filepath, '/');
base = filepath(splits(end) + 1:end);

return
