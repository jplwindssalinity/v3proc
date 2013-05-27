function [Z, xbins, ybins, xbinindex, ybinindex] = hist2d(x,y,varargin)
% function hist2d(x,y,xbins,ybins)
% [Z, xbins, ybins, xbinindex, ybinindex] = hist2d(x,y,varargin)
% x and y should be column vectors of the same size
% 
% 
% p.addOptional('xbins',[],@(x)isnumeric(x));
% p.addOptional('ybins',[],@(x)isnumeric(x));
% p.addParamValue('makeplot',[0],@(x)isnumeric(x));
% 
% Created by A. Chau 10/27/11
% Added comments A. Chau 12/6/11
% Added more comments and plot. A. Chau 1/24/12

% parse the inputs;
p = inputParser;
p.addRequired('x', @(x)isnumeric(x));
p.addRequired('y', @(x)isnumeric(x));
p.addOptional('xbins',[],@(x)isnumeric(x));
p.addOptional('ybins',[],@(x)isnumeric(x));
p.addParamValue('makeplot',[0],@(x)isnumeric(x));
p.parse(x,y,varargin{:});

% grab the data
xdata = p.Results.x;
ydata = p.Results.y;

% first, if the xbins and ybins were not specified
xbins = p.Results.xbins;
if isempty(xbins)
    xbins = linspace(min(xdata),max(xdata),256);
end
ybins = p.Results.ybins;
if isempty(ybins)
    ybins = linspace(min(ydata),max(ydata),256);
end

% use nearest neighbor interpolation to get the 2d histogram indices
% first need to clip the data to the min and max bin values
minx = min(xbins); xdata(xdata < minx) = minx; clear minx;
maxx = max(xbins); xdata(xdata > maxx) = maxx; clear maxx;
% figure out histogram bin indices
xbinindex = interp1(xbins,1:length(xbins),xdata,'nearest');

% first need to clip the data to the min and max bin values
miny = min(ybins); ydata(ydata < miny) = miny; clear miny;
maxy = max(ybins); ydata(ydata > maxy) = maxy; clear maxy;
% figure out histogram bin indices
ybinindex = interp1(ybins,1:length(ybins),ydata,'nearest');


% now use accumarray to make the histogram
Z = accumarray([ybinindex, xbinindex], 1, [length(ybins) length(xbins)]);

% make a plot if desired
if p.Results.makeplot
    figure
    imagesc(xbins,ybins,Z);
    colormap(pink);
    colorbar
    axis xy
end