function update_ct_bias( ncfile, ctbiasfile )

global BIAS_FILE;
BIAS_FILE = ctbiasfile;

n2s = @(num) num2str(num);
ncgetvar = @(ncid, var) netcdf.getVar(ncid, netcdf.inqVarID(ncid, var));
ncputvar = @(ncid, name, val) netcdf.putVar(ncid, ...
    netcdf.inqVarID(ncid, name), val);
ncgetatt = @(ncid, var, name) netcdf.getAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name);

ncid = netcdf.open(nc_file, 'WRITE');
netcdf.reDef(ncid);

%% Cross track speed bias
spd = ncgetvar(ncid, 'retrieved_wind_speed');
spd_fill  = ncgetatt(ncid, 'retrieved_wind_speed', '_FillValue');
spd_valid = spd ~= spd_fill;

xt_bias_orig = ncgetvar(ncid, 'cross_track_wind_speed_bias');
xt_bias_fill = ncgetatt(ncid, 'cross_track_wind_speed_bias', '_FillValue');
xt_bias_valid = xt_bias_orig ~= xt_bias_fill;

impact_thresh = 2.44;
rain_impact   = ncgetvar(ncid, 'rain_impact');


% remove existing speed bias
spd(spd_valid&xt_bias_valid) = spd(spd_valid&xt_bias_valid) + ...
  xt_bias_orig(spd_valid&xt_bias_valid);


% compute new speed bias
xt_wind_speed_bias = compute_bias_adj_rainy(spd, rain_impact, impact_thresh);
xt_wind_speed_bias(~spd_valid) = xt_bias_fill;

% write new speed bias out to NetCDF file
ncputvar(ncid, 'cross_track_wind_speed_bias', xt_wind_speed_bias);

% compute new speed & write it out.
spd(spd_valid) = spd(spd_valid) - xt_wind_speed_bias(spd_valid);
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
end


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
