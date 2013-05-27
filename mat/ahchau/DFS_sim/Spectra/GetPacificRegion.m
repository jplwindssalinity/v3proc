function out = GetPacificRegion(winddata,varargin)
% function out = GetPacificRegion(winddata,varargin)
%
% pass in winddata structure
% optional argument: 'makeplot' 1 or 0
% output is structure containing ati's for each Region
% note that this version returns overlapping regions!  checks that at least
%    one lat in a row (constant ati) is withing boundary, so the region
%    partitions are overlapping MAYBE FIX THIS LATER??!?!
%
% RegionI = struct('lat',[-45 -25],'lon',[160 280]);
% RegionII = struct('lat',[-25 -5],'lon',[160 280]);
% RegionIII = struct('lat',[5 25],'lon',[140 250]);
% RegionIV = struct('lat',[25 45],'lon',[150 230]);
%
% Created by A. Chau 8/24/10

p=inputParser;
p.addRequired('winddata');
p.addParamValue('makeplot',0,@(x)(x==0 | x==1));
p.parse(winddata,varargin{:});
makeplot = p.Results.makeplot;
% makeplot=1
% DEFINE THE REGIONS BASED ON Freilich and Chelton 1986
% Wavenumber Spectra of Pacific Winds Measured by the Seasat Scatterometer
% all are in degrees, north and east
RegionI = struct('lat',[-45 -25],'lon',[160 280]);
RegionII = struct('lat',[-25 -5],'lon',[160 280]);
RegionIII = struct('lat',[5 25],'lon',[140 250]);
RegionIV = struct('lat',[25 45],'lon',[150 230]);


% how to do this? 
% either check that for each row
%  at least one cti is within latitude range
%  or require that all ctis be within latitude range

% maybe easier to check that all cells in the row are within the boundaries
% in lat and lon, exclude entire row if not

% for latitude, just use latitude as is and simple > <
% for longitude, if there are negative degrees, convert to 0 to 360
ind = winddata.lon < 0;
winddata.lon(ind) = winddata.lon(ind) + 360;

% RegionI
ind = find((winddata.lat > RegionI.lat(1)) & (winddata.lat < RegionI.lat(2)) & ...
    (winddata.lon > RegionI.lon(1)) & (winddata.lon < RegionI.lon(2)));
[row,col] = ind2sub(size(winddata.lat),ind);
out.RegionI_ati = unique(row);

% RegionII
ind = find((winddata.lat > RegionII.lat(1)) & (winddata.lat < RegionII.lat(2)) & ...
    (winddata.lon > RegionII.lon(1)) & (winddata.lon < RegionII.lon(2)));
[row,col] = ind2sub(size(winddata.lat),ind);
out.RegionII_ati = unique(row);

% RegionIII
ind = find((winddata.lat > RegionIII.lat(1)) & (winddata.lat < RegionIII.lat(2)) & ...
    (winddata.lon > RegionIII.lon(1)) & (winddata.lon < RegionIII.lon(2)));
[row,col] = ind2sub(size(winddata.lat),ind);
out.RegionIII_ati = unique(row);

% RegionIV
ind = find((winddata.lat > RegionIV.lat(1)) & (winddata.lat < RegionIV.lat(2)) & ...
    (winddata.lon > RegionIV.lon(1)) & (winddata.lon < RegionIV.lon(2)));
[row,col] = ind2sub(size(winddata.lat),ind);
out.RegionIV_ati = unique(row);

if makeplot
    load coast
    ind = long<0; long(ind) = long(ind) + 360;
    myfigure(1);clf
    plot(long,lat,'k.', ...
        winddata.lon(out.RegionI_ati,:), winddata.lat(out.RegionI_ati,:), 'bo', ...
        winddata.lon(out.RegionII_ati,:), winddata.lat(out.RegionII_ati,:),'g.', ...
        winddata.lon(out.RegionIII_ati,:), winddata.lat(out.RegionIII_ati,:), 'r.', ...
        winddata.lon(out.RegionIV_ati,:), winddata.lat(out.RegionIV_ati,:), 'co')
    axis tight
end
