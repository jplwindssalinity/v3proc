%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ORIGINAL AUTHOR: Thomas Werne
% COMPANY: Jet Propulsion Laboratory
% VERSION:
%
% File Name:     qs_cf_compliance.m
% Creation Date: 28 Nov 2011
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

function qs_cf_compliance(filename)

str_in_cell = @(str, attrs_map) cellfun(@(x) strcmp(x, str), attrs_map);
global NCGLOBAL;
NCGLOBAL = netcdf.getConstant('NC_GLOBAL');

ncid = netcdf.open(filename, 'NC_WRITE');

% Check to see if this file has already been processed
if ~strcmp('CF-1.4', netcdf.getAtt(ncid, NCGLOBAL, 'Conventions'))
    display(['Skipping: ' filename])
    update_productiondatetime(filename, ncid);
    netcdf.close(ncid);
    return
end

netcdf.reDef(ncid);

[~, nvars, ngatts, ~] = netcdf.inq(ncid);

% Convert FillValue -> _FillValue
for k = 0:(nvars - 1)
    try %#ok
        netcdf.renameAtt(ncid, k, 'FillValue', '_FillValue');
    end
end

% Remove 'scale_factor' & '_FillValue' attributes from 'lat' and 'lon' variables
attr_names = {'scale_factor', '_FillValue'};
varlist = {'lat', 'lon'};

for varname = varlist
    for attr_name = attr_names
        varid = netcdf.inqVarID(ncid, varname{1});
        netcdf.delAtt(ncid, varid, attr_name{1});
    end
end

% Add "coordinates: lon lat"
attr_name = 'coordinates';
attr_val  = 'lon lat';

if (strfind(netcdf.getAtt(ncid, NCGLOBAL, 'LongName'), 'Level 2B'))
    varlist = { ...
        'retrieved_wind_speed', 'retrieved_wind_direction', 'rain_impact', ...
        'flags', 'eflags', 'nudge_wind_speed', 'nudge_wind_direction', ...
        'retrieved_wind_speed_uncorrected', 'num_ambiguities', ...
        'cross_track_wind_speed_bias', 'atmospheric_speed_bias' ...
    };
else
    varlist = { ...
        'wind_speed', 'wind_direction', 'wind_divergence', 'wind_curl', ...
        'flags', 'eflags', 'ecmwf_wind_speed', 'ecmwf_wind_direction'
    };
end

for varname = varlist
    varid = netcdf.inqVarID(ncid, varname{1});
    netcdf.putAtt(ncid, varid, attr_name, attr_val);
end

% Change '_FillValue' for 'flags' and 'eflags' to 'short's.
% I can't find a way to change the xtype of an attribute in Matlab once it's
% been defined using the NetCDF library.  Accomplish this via read-modify-write:
% 1) Collect all global attributes in a local variable
% 2) Delete them from the .nc file
% 3) Modify the local variable to desired format
% 4) Write them back into the .nc file
varlist = {'flags', 'eflags'};

for varname = varlist
    varid = netcdf.inqVarID(ncid, varname{1});

    [~, ~, ~, natts] = netcdf.inqVar(ncid, varid);
    attr_map = cell(natts, 2); %#ok

    for k = 1:natts
        attrs_map{k, 1} = netcdf.inqAttName(ncid, varid, k - 1); %#ok
        attrs_map{k, 2} = netcdf.getAtt(ncid, varid, attrs_map{k, 1}); %#ok
    end

    for k = 1:natts
        netcdf.delAtt(ncid, varid, attrs_map{k, 1});
    end

    idx = str_in_cell('_FillValue', attrs_map);
    attrs_map{idx, 2} = int16(attrs_map{idx, 2}); %#ok

    for k = 1:natts
        netcdf.putAtt(ncid, varid, attrs_map{k, 1}, attrs_map{k, 2});
    end
end

% Add global attributes
% Attribute *order* needs to be modified.  Accomplish this via
% read-modify-write:
% 1) Collect all global attributes in a local variable
% 2) Delete them from the .nc file
% 3) Modify the local variable to desired format
% 4) Write them back into the .nc file

attrs_map = cell(ngatts + 4, 2);
% Gather existing global attributes
for k = 1:ngatts
    attrs_map{k, 1} = netcdf.inqAttName(ncid, NCGLOBAL, k - 1);
    attrs_map{k, 2} = netcdf.getAtt(ncid, NCGLOBAL, attrs_map{k, 1});
end

% Delete them
for k = 1:ngatts
    netcdf.delAtt(ncid, NCGLOBAL, attrs_map{k, 1});
end

% Insert 'title' as the first attribute
for k = length(attrs_map):-1:2
    attrs_map{k, 1} = attrs_map{k - 1, 1};
    attrs_map{k, 2} = attrs_map{k - 1, 2};
end
attrs_map{1, 1} = 'title';

% Insert 'source' immediately after 'PlatformShortName'
idx = find(str_in_cell('PlatformShortName', attrs_map));
for k = length(attrs_map):-1:idx + 2
    attrs_map{k, 1} = attrs_map{k - 1, 1};
    attrs_map{k, 2} = attrs_map{k - 1, 2};
end
attrs_map{idx + 1, 1} = 'source';
attrs_map{idx + 1, 2} = 'QuikSCAT';

% Insert 'comment' immediately before 'history'
idx = find(str_in_cell('history', attrs_map));
for k = length(attrs_map):-1:idx + 1
    attrs_map{k, 1} = attrs_map{k - 1, 1};
    attrs_map{k, 2} = attrs_map{k - 1, 2};
end
attrs_map{idx, 1} = 'comment';
attrs_map{idx, 2} = '';

% Replace 'producer_institution' with 'institution'
idx = str_in_cell('producer_institution', attrs_map);
attrs_map{idx, 1} = 'institution';

% Change 'Conventions': 'CF-1.4' -> 'CF-1.5'
idx = str_in_cell('Conventions', attrs_map);
attrs_map{idx, 2} = 'CF-1.5';

% Add 'references' as the final attribute
attrs_map{end, 1} = 'references'; 
attrs_map{end, 2} = ''; 

% Add all global attributes back in
for k = 1:length(attrs_map)
    netcdf.putAtt(ncid, NCGLOBAL, attrs_map{k, 1}, attrs_map{k, 2});
end

% Update production date/time string
update_productiondatetime(filename, ncid);

netcdf.close(ncid);

end

function update_productiondatetime(fname, ncid)

global NCGLOBAL;

finfo = dir(fname);
proc_date = datevec(finfo.date);

proc_doy = floor(datenum(finfo.date) - ...
    datenum(['1/0/' num2str(proc_date(1))]));

proc_date_att = sprintf('%04d-%03dT%02d:%02d:%02d.%03d', ...
    proc_date(1), proc_doy, proc_date(4), proc_date(5), proc_date(6), 0);
netcdf.putAtt(ncid, NCGLOBAL, 'ProductionDateTime', proc_date_att);

end
