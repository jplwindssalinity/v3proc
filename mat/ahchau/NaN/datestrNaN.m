function out = datestrNaN(in,varargin)
% function out = datestrNaN(in,varargin)
% Created by AHChau 2/11/13

% Where are the nans?
% nanhere = isnan(in);
% nonanhere = ~nanhere;
goodind = find(~isnan(in));

% How big will the output be?
outstr = datestr(in(goodind(1)),varargin{:});
out = repmat(' ',size(in,1),size(outstr,2));

out(goodind,:) = datestr(in(goodind),varargin{:});

end
