function qs_convert_netcdf(orig_nc_file, e2b12_dir)

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


%% Define "Constants"
SRC = 1;
UNFILT = 2;
FILT = 3;
AT_IDX = 1;
XT_IDX = 2;
TS_IDX = 3;

GLOBAL = netcdf.getConstant('GLOBAL');

%% Variables to copy
% Defines a mapping of variables from the source to the filtered and
% unfiltered targets.
% FMT: {source, unfiltered, filtered}
var_mapping = ...
    {'time', 'time', 'time'; ...
     'lat', 'lat', 'lat'; ...
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
run_date = netcdf.getAtt(src.ncid, GLOBAL, 'EquatorCrossingDate');
run_year = str2double(run_date(1:4));
run_doy = str2double(run_date(6:end));

leap_year = is_leap_year(run_year);

% DOY -> month #
run_month = 0;
switch run_doy
    case num2cell(1:31), run_month = 1;
    case num2cell(31 + (1:28 + leap_year)), run_month = 2;
    case num2cell(59 + leap_year + (1:31)), run_month = 3;
    case num2cell(90 + leap_year + (1:30)), run_month = 4;
    case num2cell(120 + leap_year + (1:31)), run_month = 5;
    case num2cell(151 + leap_year + (1:30)), run_month = 6;
    case num2cell(181 + leap_year + (1:31)), run_month = 7;
    case num2cell(212 + leap_year + (1:31)), run_month = 8;
    case num2cell(243 + leap_year + (1:30)), run_month = 9;
    case num2cell(273 + leap_year + (1:31)), run_month = 10;
    case num2cell(304 + leap_year + (1:30)), run_month = 11;
    case num2cell(334 + leap_year + (1:31)), run_month = 12;
    otherwise, display 'Couldn''t determine DOY -> Mo';
end

unfilt.file = sprintf('L2B_%05d_v3_%04d_%02d.nc', run_no, run_year, ...
    run_month);
unfilt.file = regexprep(src.file, '[^/]*$', unfilt.file);
filt.file   = sprintf('L2C_%05d_v3_%04d_%02d.nc', run_no, run_year, ...
    run_month);
filt.file   = regexprep(src.file, '[^/]*$', filt.file);

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
% cross-track-wind-speed bias
varid = netcdf.defVar(unfilt.ncid, 'cross_track_wind_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', -9999.0);
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', 0.0);
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', 100.0);
netcdf.putAtt(unfilt.ncid, varid, 'long_name', ['relative wind speed ' ...
    'bias with respect to sweet spot']);
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', 1.0);

% atmospheric speed bias
varid = netcdf.defVar(unfilt.ncid, 'atmospheric_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(unfilt.ncid, varid, 'FillValue', -9999.0);
netcdf.putAtt(unfilt.ncid, varid, 'valid_min', 0.0);
netcdf.putAtt(unfilt.ncid, varid, 'valid_max', 100.0);
netcdf.putAtt(unfilt.ncid, varid, 'long_name', 'atmospheric speed bias');
netcdf.putAtt(unfilt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(unfilt.ncid, varid, 'scale_factor', 1.0);

% ECMWF wind speed
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_speed', ...
    'float', [dimid_mapping(AT_IDX, FILT), ...
              dimid_mapping(XT_IDX, FILT)]);
netcdf.putAtt(filt.ncid, varid, 'FillValue', -9999.0);
netcdf.putAtt(filt.ncid, varid, 'valid_min', 0.0);
netcdf.putAtt(filt.ncid, varid, 'valid_max', 100.0);
netcdf.putAtt(filt.ncid, varid, 'long_name', 'model wind speed');
netcdf.putAtt(filt.ncid, varid, 'units', 'm s-1');
netcdf.putAtt(filt.ncid, varid, 'scale_factor', 1.0);

% ECMWF wind direction
varid = netcdf.defVar(filt.ncid, 'ecmwf_wind_direction', ...
    'float', [dimid_mapping(AT_IDX, FILT), ...
              dimid_mapping(XT_IDX, FILT)]);
netcdf.putAtt(filt.ncid, varid, 'FillValue', -9999.0);
netcdf.putAtt(filt.ncid, varid, 'valid_min', 0.0);
netcdf.putAtt(filt.ncid, varid, 'valid_max', 360.0);
netcdf.putAtt(filt.ncid, varid, 'long_name', 'model wind direction');
netcdf.putAtt(filt.ncid, varid, 'units', 'degrees');
netcdf.putAtt(filt.ncid, varid, 'scale_factor', 1.0);

%% Alter copied definitions
% Correct erroneous ProductionDateTime.  Use the access date/time for the
% source .nc file (at this point it's the best we have)
finfo = dir(src.file);
proc_date = datevec(finfo.date);

% Convert MM/DD -> DOY
proc_doy = proc_date(3);
if (proc_date(2) > 1),  proc_doy = proc_doy + 31; end
if (proc_date(2) > 2),  proc_doy = proc_doy + 28; end
if (proc_date(2) > 3),  proc_doy = proc_doy + 31; end
if (proc_date(2) > 4),  proc_doy = proc_doy + 30; end
if (proc_date(2) > 5),  proc_doy = proc_doy + 31; end
if (proc_date(2) > 6),  proc_doy = proc_doy + 30; end
if (proc_date(2) > 7),  proc_doy = proc_doy + 31; end
if (proc_date(2) > 8),  proc_doy = proc_doy + 31; end
if (proc_date(2) > 9),  proc_doy = proc_doy + 30; end
if (proc_date(2) > 10), proc_doy = proc_doy + 31; end
if (proc_date(2) > 11), proc_doy = proc_doy + 30; end

proc_doy = proc_doy + is_leap_year(proc_date(1));

proc_date_att = sprintf('%04d-%03dT%02d:%02d:%02d.%03d', ...
    proc_date(1), proc_doy, proc_date(4), proc_date(5), proc_date(6), 0);
netcdf.putAtt(unfilt.ncid, GLOBAL, 'ProductionDateTime', proc_date_att);
netcdf.putAtt(filt.ncid,   GLOBAL, 'ProductionDateTime', proc_date_att);

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
% ECMWF
data = '';
e2b12.file = sprintf('%s/%04d/E2B_%05d.cp12.dat', ...
    e2b12_dir, run_year, run_no);

% Check to see if the file exists
if (exist(e2b12.file, 'file') ~= 0)
    data = read_qs_e2b12(e2b12.file);
else
    warning(['Can''t find ' e2b12.file]);
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


% A helper function for leap-year calculations
function ly = is_leap_year(year)

ly = 0;
if (mod(year, 400) == 0)
    ly = 1;
end
if (mod(year, 4) == 0)
    if (mod(year, 100) ~= 0)
        ly = 1;
    end
end

return
