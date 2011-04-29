%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ORIGINAL AUTHOR: Thomas Werne
% COMPANY: Jet Propulsion Laboratory
% VERSION:
%
% File Name:     average_to_25km.m
% Creation Date: 29 Apr 2011
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

function average_to_25km(fname_12p5)

ncgetvar = @(ncid, var) netcdf.getVar(ncid, netcdf.inqVarID(ncid, var));
ncputvar = @(ncid, name, val) netcdf.putVar(ncid, ...
    netcdf.inqVarID(ncid, name), val);
ncgetatt = @(ncid, var, name) netcdf.getAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name);


low_speed_thresh = 3.0;
high_speed_thresh = 10.0;

low_speed_flag  = uint16(2048);
high_speed_flag = uint16(1024);

div = 2;

ncid_12p5 = netcdf.open(fname_12p5, 'NOWRITE');

if (~isempty(strfind(fname_12p5, '_GS')))
    fname_25p0 = strrep(fname_12p5, '_GS', '_25km_GS');
else
    fname_25p0 = strrep(fname_12p5, '_S3', '_25km_S3');
end

% Get dimensions
[~, nvars, ngatts, ~] = netcdf.inq(ncid_12p5);
[~, at_len] = netcdf.inqDim(ncid_12p5, netcdf.inqDimID(ncid_12p5, ...
    'along_track'));
[~, xt_len] = netcdf.inqDim(ncid_12p5, netcdf.inqDimID(ncid_12p5, ...
    'cross_track'));
[~, ts_len] = netcdf.inqDim(ncid_12p5, netcdf.inqDimID(ncid_12p5, ...
    'time_strlength'));

% Get the variables
ncgetvar12p5 = @(name) ncgetvar(ncid_12p5, name);
ncgetatt12p5 = @(var, att) ncgetatt(ncid_12p5, var, att);

time = ncgetvar12p5('time');
lat = ncgetvar12p5('lat');
    lat_fill = ncgetatt12p5('lat', 'FillValue');
lon = ncgetvar12p5('lon');
    lon_fill = ncgetatt12p5('lon', 'FillValue');
wind_speed = ncgetvar12p5('wind_speed');
    wind_speed_fill = ncgetatt12p5('wind_speed', 'FillValue');
wind_dir = ncgetvar12p5('wind_to_direction');
    wind_dir_fill = ncgetatt12p5('wind_to_direction', 'FillValue');
rain_impact = ncgetvar12p5('rain_impact');
    rain_impact_fill = ncgetatt12p5('rain_impact', 'FillValue');
wind_divergence = ncgetvar12p5('wind_divergence');
    wind_divergence_fill = ncgetatt12p5('wind_divergence', 'FillValue');
wind_curl = ncgetvar12p5('wind_curl');
    wind_curl_fill = ncgetatt12p5('wind_curl', 'FillValue');
stress = ncgetvar12p5('stress');
    stress_fill = ncgetatt12p5('stress', 'FillValue');
stress_div = ncgetvar12p5('stress_divergence');
    stress_div_fill = ncgetatt12p5('stress_divergence', 'FillValue');
stress_curl = ncgetvar12p5('stress_curl');
    stress_curl_fill = ncgetatt12p5('stress_curl', 'FillValue');
flags = uint16(ncgetvar12p5('flags'));
%    flags_fill = ncgetatt12p5('flags', 'FillValue');
eflags = uint16(ncgetvar12p5('eflags'));
%    eflags_fill = ncgetatt12p5('eflags', 'FillValue');
model_wind_speed = ncgetvar12p5('model_wind_speed');
    model_wind_speed_fill = ncgetatt12p5('model_wind_speed', 'FillValue');
model_wind_dir = ncgetvar12p5('model_wind_direction');
    model_wind_dir_fill = ncgetatt12p5('model_wind_direction', ...
        'FillValue');
wind_speed_uncorrected = ncgetvar12p5('wind_speed_uncorrected');
    wind_speed_uncorrected_fill = ncgetatt12p5('wind_speed_uncorrected', ...
        'FillValue');
num_ambiguities = ncgetvar12p5('num_ambiguities');
    num_ambiguities_fill = ncgetatt12p5('num_ambiguities', 'FillValue');


% Do the appropriate conversions and averaging
u = wind_speed.*cosd(wind_dir);
v = wind_speed.*sind(wind_dir);
model_u = model_wind_speed.*cosd(model_wind_dir);
model_v = model_wind_speed.*sind(model_wind_dir);

warning('Just using time idx %d instead of averaging', ...
    floor((div + 1)/2)); %#ok

time = time(:, floor((div + 1)/2):div:end);
lat = avg(lat, div, lat_fill);
lon = avg(lon, div, lon_fill);
u = avg(u, div);
v = avg(v, div);
wind_divergence = avg(wind_divergence, div, wind_divergence_fill);
wind_curl = avg(wind_curl, div, wind_curl_fill);
stress = avg(stress, div, stress_fill);
stress_div = avg(stress_div, div, stress_div_fill);
stress_curl = avg(stress_curl, div, stress_curl_fill);
model_u = avg(model_u, div);
model_v = avg(model_v, div);
wind_speed_uncorrected = avg(wind_speed_uncorrected, div);

% For rain_impact, num_ambiguities, we want to take the max over
% the averaged cells
[endr, endc] = size(rain_impact);
endr = div*floor(endr/div);
endc = div*floor(endc/div);

new_rain_impact = -Inf*ones(endr/div, endc/div);
new_num_ambiguities = int8(-Inf*ones(endr/div, endc/div));
for m = 1:div
    for n = 1:div
        new_rain_impact = max(new_rain_impact, rain_impact(m:div:endr, ...
            n:div:endc));
        new_num_ambiguities = max(new_num_ambiguities, ...
            num_ambiguities(m:div:endr, n:div:endc));
    end
end

[inval_idx_r, inval_idx_c] = find(rain_impact(1:endr, 1:endc) == ...
                                  rain_impact_fill);

new_rain_impact(sub2ind(size(new_rain_impact), ceil(inval_idx_r/div), ...
    ceil(inval_idx_c/div))) = rain_impact_fill;

rain_impact = new_rain_impact;

[invalid_idx_r, invalid_idx_c] = find(num_ambiguities(1:endr, 1:endc) == ...
                                     num_ambiguities_fill);

new_num_ambiguities(sub2ind(size(new_num_ambiguities), ...
    ceil(inval_idx_r/div), ceil(inval_idx_c/div))) == rain_impact_fill;

num_ambiguities = new_num_ambiguities;

new_wind_speed = sqrt(u.^2 + v.^2);
new_wind_dir = atan2(v, u)*180/pi;
new_model_wind_speed = sqrt(model_u.^2 + model_v.^2);
new_model_wind_dir = atan2(model_v, model_u)*180/pi;

% Correct phase values
new_wind_dir(new_wind_dir < 0) = new_wind_dir(new_wind_dir < 0) + 360;
new_model_wind_dir(new_model_wind_dir < 0) = ...
    new_model_wind_dir(new_model_wind_dir < 0) + 360;

% Fix cells with FillValue speed and direction
[endr, endc] = size(wind_dir);
endr = div*floor(endr/div);
endc = div*floor(endc/div);

[inval_idx_r, inval_idx_c] = ...
    find(wind_dir(1:endr, 1:endc) == wind_dir_fill);
new_wind_dir(sub2ind(size(new_wind_dir), ceil(inval_idx_r/div), ...
    ceil(inval_idx_c/div))) = wind_dir_fill;
new_wind_speed(sub2ind(size(new_wind_speed), ceil(inval_idx_r/div), ...
    ceil(inval_idx_c/div))) = wind_speed_fill;

[inval_idx_r, inval_idx_c] = ...
    find(wind_speed(1:endr, 1:endc) == wind_speed_fill);
new_wind_dir(sub2ind(size(new_wind_dir), ceil(inval_idx_r/div), ...
    ceil(inval_idx_c/div))) = wind_dir_fill;
new_wind_speed(sub2ind(size(new_wind_speed), ceil(inval_idx_r/div), ...
    ceil(inval_idx_c/div))) = wind_speed_fill;

[inval_idx_r, inval_idx_c] = ...
    find(model_wind_dir(1:endr, 1:endc) == model_wind_dir_fill);
new_model_wind_dir(sub2ind(size(new_model_wind_dir), ...
    ceil(inval_idx_r/div), ceil(inval_idx_c/div))) = model_wind_dir_fill;
new_model_wind_speed(sub2ind(size(new_model_wind_speed), ...
    ceil(inval_idx_r/div), ceil(inval_idx_c/div))) = model_wind_speed_fill;

[inval_idx_r, inval_idx_c] = ...
    find(model_wind_speed(1:endr, 1:endc) == model_wind_speed_fill);
new_model_wind_dir(sub2ind(size(new_model_wind_dir), ...
    ceil(inval_idx_r/div), ceil(inval_idx_c/div))) = model_wind_dir_fill;
new_model_wind_speed(sub2ind(size(new_model_wind_speed), ...
    ceil(inval_idx_r/div), ceil(inval_idx_c/div))) = model_wind_speed_fill;

wind_speed = new_wind_speed;
wind_dir = new_wind_dir;
model_wind_speed = new_model_wind_speed;
model_wind_dir = new_model_wind_dir;

% Recompute flag values and take care of fill values in computed values
new_flags = zeros(floor(size(flags)/div), 'uint16');
new_eflags = new_flags;
[endr, endc] = size(flags);
endr = div*floor(endr/div);
endc = div*floor(endc/div);

for m = 1:div
    for n = 1:div
        new_flags = bitor(new_flags, flags(m:div:endr, n:div:endc));
        new_eflags = bitor(new_eflags, eflags(m:div:endr, n:div:endc));
    end
end

new_flags(logical((0 <= wind_speed).*(wind_speed <= low_speed_thresh))) ...
    = bitor(low_speed_flag, new_flags(logical((0 <= wind_speed).* ...
    (wind_speed <= low_speed_thresh))));
new_flags(high_speed_thresh <= wind_speed) = bitor(high_speed_flag, ...
    new_flags(high_speed_thresh <= wind_speed));

flags  = int16(new_flags);
eflags = int16(new_eflags);


% Redefine the NetCDF file parameters and reload the data
ncid_25p0 = netcdf.create(fname_25p0, 'CLASSIC_MODEL');
at_len = floor(at_len/div);
xt_len = floor(xt_len/div);

netcdf.defDim(ncid_25p0, 'along_track', at_len);
netcdf.defDim(ncid_25p0, 'cross_track', xt_len);
netcdf.defDim(ncid_25p0, 'time_strlength', ts_len);

% Copy the global attributes
for k = 0:ngatts - 1
    name = netcdf.inqAttName(ncid_12p5, netcdf.getConstant('GLOBAL'), k);
    netcdf.copyAtt(ncid_12p5, netcdf.getConstant('GLOBAL'), name, ...
        ncid_25p0, netcdf.getConstant('GLOBAL'));
end

% Copy the variable definitions
for k = 0:nvars - 1
    [name, xtype, dimids, natts] = netcdf.inqVar(ncid_12p5, k);
    netcdf.defVar(ncid_25p0, name, xtype, dimids);

    for m = 0:natts - 1
        name = netcdf.inqAttName(ncid_12p5, k, m);
        netcdf.copyAtt(ncid_12p5, k, name, ncid_25p0, k);
    end
end

netcdf.endDef(ncid_25p0);

% Write variables back out into NetCDF file
ncputvar25p0 = @(name, val) ncputvar(ncid_25p0, name, val);
ncputvar25p0('time', time);
ncputvar25p0('lat', lat);
ncputvar25p0('lon', lon);
ncputvar25p0('wind_speed', wind_speed);
ncputvar25p0('wind_to_direction', wind_dir);
ncputvar25p0('rain_impact', rain_impact);
ncputvar25p0('wind_divergence', wind_divergence);
ncputvar25p0('wind_curl', wind_curl);
ncputvar25p0('stress', stress);
ncputvar25p0('stress_divergence', stress_div);
ncputvar25p0('stress_curl', stress_curl);
ncputvar25p0('flags', flags);
ncputvar25p0('eflags', eflags);
ncputvar25p0('model_wind_speed', model_wind_speed);
ncputvar25p0('model_wind_direction', model_wind_dir);
ncputvar25p0('wind_speed_uncorrected', wind_speed_uncorrected);
ncputvar25p0('num_ambiguities', num_ambiguities);

netcdf.close(ncid_12p5);
netcdf.close(ncid_25p0);

return

function avg_data = avg(data, div, fill_val)

% Preallocate for speed
avg_data = zeros(floor(size(data)/div));
[endr, endc] = size(data);
endr = div*floor(endr/div);
endc = div*floor(endc/div);

% Do the subaveraging
for m = 1:div
    for n = 1:div
        avg_data = avg_data + data(m:div:endr, n:div:endc);
    end
end
avg_data = avg_data/(div^2);

if (exist('fill_val', 'var'))
    % Look for invalid data in the original array...
    [inval_idx_r, inval_idx_c] = find(data(1:endr, 1:endc) == fill_val);

    % and set the appropriate cells
    avg_data(sub2ind(size(avg_data), ceil(inval_idx_r/div), ...
        ceil(inval_idx_c/div))) = fill_val;
end

return
