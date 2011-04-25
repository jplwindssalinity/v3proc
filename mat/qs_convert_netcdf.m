function qs_convert_netcdf(file_src)

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
     'model_wind_speed', 'nudge_wind_speed', ...
        'ecmwf_wind_speed'; ...
     'model_wind_direction', 'nudge_wind_direction', ...
        'ecmwf_wind_direction'; ...
     'wind_speed_uncorrected', 'retrieved_wind_speed_uncorrected', ''; ...
     'num_ambiguities', 'num_ambiguities', ''};
% questions: wind_speed -> ?
%            wind_to_direciton -> ?
%            model_wind_speed -> ?
%            model_wind_direction -> ?
%            cross_track_wind_speed_bias in both?
       
ncid_src = netcdf.open(file_src, 'NOWRITE');

%% Get dimensions
dimid_mapping = zeros(3, 3);

% Build a table with the correspondences between the dimension ids across
% the three files
dimid_mapping(AT_IDX, SRC) = netcdf.inqDimID(ncid_src, 'along_track');
dimid_mapping(XT_IDX, SRC) = netcdf.inqDimID(ncid_src, 'cross_track');
dimid_mapping(TS_IDX, SRC) = netcdf.inqDimID(ncid_src, 'time_strlength');

[~, at_len] = netcdf.inqDim(ncid_src, dimid_mapping(AT_IDX, SRC));
[~, xt_len] = netcdf.inqDim(ncid_src, dimid_mapping(XT_IDX, SRC));
[~, ts_len] = netcdf.inqDim(ncid_src, dimid_mapping(TS_IDX, SRC));

%% Compute new file name
run_no = netcdf.getAtt(ncid_src, GLOBAL, 'StopOrbitNumber');
run_date = netcdf.getAtt(ncid_src, GLOBAL, 'EquatorCrossingDate');
run_year = str2double(run_date(1:4));
run_doy = str2double(run_date(6:end));

leap_year = is_leap_year(run_year);

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

file_unfilt = sprintf('L2B_%05d_v3_%04d_%02d.nc', run_no, run_year, ...
    run_month);
file_filt   = sprintf('L2C_%05d_v3_%04d_%02d.nc', run_no, run_year, ...
    run_month);

%% Open the product files
ncid_unfilt = netcdf.create(file_unfilt, 'CLASSIC_MODEL');
ncid_filt   = netcdf.create(file_filt,   'CLASSIC_MODEL');

dimid_mapping(AT_IDX, UNFILT) = netcdf.defDim(ncid_unfilt, ...
    'along_track', at_len);
dimid_mapping(XT_IDX, UNFILT) = netcdf.defDim(ncid_unfilt, ...
    'cross_track', xt_len);
dimid_mapping(TS_IDX, UNFILT) = netcdf.defDim(ncid_unfilt, ...
    'time_strlength', ts_len);
dimid_mapping(AT_IDX, FILT) = netcdf.defDim(ncid_filt, ...
    'along_track', at_len);
dimid_mapping(XT_IDX, FILT) = netcdf.defDim(ncid_filt, ...
    'cross_track', xt_len);
dimid_mapping(TS_IDX, FILT) = netcdf.defDim(ncid_filt, ...
    'time_strlength', ts_len);

%% Copy the global attributes
[~, ~, ngatts, ~] = netcdf.inq(ncid_src);
for k = 0:ngatts - 1
    name = netcdf.inqAttName(ncid_src, GLOBAL, k);
    netcdf.copyAtt(ncid_src, GLOBAL, name, ncid_unfilt, GLOBAL);
    netcdf.copyAtt(ncid_src, GLOBAL, name, ncid_filt, GLOBAL);
end

%% Copy variable definitions
[nvar, ~] = size(var_mapping);
for k = 1:nvar
    % copy the information from the old file
    varid_src = netcdf.inqVarID(ncid_src, var_mapping{k, SRC});
    [~, xtype, dimids, natts] = netcdf.inqVar(ncid_src, varid_src);
        
    % Given source dimension ids, lookup the index in the dimid LUT
    [X, Y] = meshgrid(dimid_mapping(:, SRC), dimids);
    [~, dimids] = find(X == Y);
   
    if (~isempty(var_mapping{k, UNFILT}))
        varid_tgt = netcdf.defVar(ncid_unfilt, var_mapping{k, UNFILT}, ...
            xtype, dimid_mapping(dimids, UNFILT));
        
        % Copy the variable attributes
        for m = 0:natts - 1
            name = netcdf.inqAttName(ncid_src, varid_src, m);
            netcdf.copyAtt(ncid_src, varid_src, name, ...
                ncid_unfilt, varid_tgt);
        end
    end
    if (~isempty(var_mapping{k, FILT}))
        varid_tgt = netcdf.defVar(ncid_filt, var_mapping{k, FILT}, ...
            xtype, dimid_mapping(dimids, FILT));
        
        % Copy the variable attributes
        for m = 0:natts - 1
            name = netcdf.inqAttName(ncid_src, varid_src, m);
            netcdf.copyAtt(ncid_src, varid_src, name, ...
                ncid_filt, varid_tgt);
        end        
    end
end

%% Build the new variables
% cross-track-wind-speed bias
varid = netcdf.defVar(ncid_unfilt, 'cross_track_wind_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(ncid_unfilt, varid, 'FillValue', '-9999.0');
netcdf.putAtt(ncid_unfilt, varid, 'valid_min', '0.0');
netcdf.putAtt(ncid_unfilt, varid, 'valid_max', '100.0');
netcdf.putAtt(ncid_unfilt, varid, 'long_name', ['relative wind speed ' ...
    'bias with respect to sweet spot']);
netcdf.putAtt(ncid_unfilt, varid, 'units', 'm s-1');
netcdf.putAtt(ncid_unfilt, varid, 'scale_factor', '1.0');

% atmospheric speed bias
varid = netcdf.defVar(ncid_unfilt, 'atmospheric_speed_bias', ...
    'float', [dimid_mapping(AT_IDX, UNFILT), ...
              dimid_mapping(XT_IDX, UNFILT)]);
netcdf.putAtt(ncid_unfilt, varid, 'FillValue', '-9999.0');
netcdf.putAtt(ncid_unfilt, varid, 'valid_min', '0.0');
netcdf.putAtt(ncid_unfilt, varid, 'valid_max', '100.0');
netcdf.putAtt(ncid_unfilt, varid, 'long_name', 'atmospheric speed bias');
netcdf.putAtt(ncid_unfilt, varid, 'units', 'm s-1');
netcdf.putAtt(ncid_unfilt, varid, 'scale_factor', '1.0');
 

%% Alter copied definitions
% Correct erroneous ProductionDateTime.  Use the access date/time for the
% source .nc file (at this point it's the best we have
finfo = dir(file_src);
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
netcdf.putAtt(ncid_unfilt, GLOBAL, 'ProductionDateTime', proc_date_att);
netcdf.putAtt(ncid_filt,   GLOBAL, 'ProductionDateTime', proc_date_att);

%% End the definitions stage
netcdf.endDef(ncid_unfilt);
netcdf.endDef(ncid_filt);

%% Copy over the data
[nvar, ~] = size(var_mapping);
for k = 1:nvar
    data = ncgetvar(ncid_src, var_mapping{k, SRC});
    
    if (~isempty(var_mapping{k, UNFILT}))
        ncputvar(ncid_unfilt, var_mapping{k, UNFILT}, data);
    end
    if (~isempty(var_mapping{k, FILT}))
        ncputvar(ncid_filt, var_mapping{k, FILT}, data);
    end
end

%% Close files
netcdf.close(ncid_src);
netcdf.close(ncid_unfilt);
netcdf.close(ncid_filt);

return

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
