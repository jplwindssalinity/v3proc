%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% ORIGINAL AUTHOR: Thomas Werne
% COMPANY: Jet Propulsion Laboratory
% VERSION: 
%
% File Name:     fix_rain_impact_bug.m
% Creation Date: 03 May 2011
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

function fix_rain_impact_bug(nc_file)
%
% fix_rain_impact_bug(nc_file)
%
% This function is used to correct a bug in the QuikSCAT L2B -> NetCDF
% conversion software that wrote incorrect rain_impact values.  
%
% The given NetCDF file (nc_file) is modified (in place) with the correct
% rain_imapct values from the l2b.dat file in the same directory.
%
% The bug should be corrected after 05 May 2011, so this function should not be
% necessary after that date.
%
% --- taw, 04-May-2011

ncputvar = @(ncid, name, val) netcdf.putVar(ncid, ...
    netcdf.inqVarID(ncid, name), val);
ncgetatt = @(ncid, var, name) netcdf.getAtt(ncid, ...
    netcdf.inqVarID(ncid, var), name);

ncid = netcdf.open(nc_file, 'NC_WRITE');

% Assume the l2b.dat file is in the same directory as the .nc file
l2bfile = regexprep(nc_file, '[^/]*$', 'l2b.dat');

fillVal = ncgetatt(ncid, 'rain_impact', 'FillValue');

rainImpact = l2b_extraction(l2bfile, 'rainImpact', fillVal);
rainCorrectedSpeed = l2b_extraction(l2bfile, 'rainCorrectedSpeed', ...
    fillVal);

rainImpact((rainImpact == 0) & (rainCorrectedSpeed == -1)) = fillVal;
ncputvar(ncid, 'rain_impact', rainImpact);

netcdf.close(ncid);

