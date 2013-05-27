function h = myquiver(x,y,u,v, varargin)
% function h=myquiver(x,y,u,v,...)
% function h=myquiver(x,y,u,v,s, ...)
%
% myquiver(x,y,u,v,'skip',10,'norm',1)
% myquiver(x,y,u,v,'xskip',2,'yskip',10)
%
% optional argument s is the scale factor (like original quiver.m function)
% optional parameters are: skip, xskip, yskip
% and norm, which is 1 if you want normalized vectors and 0 if not
%
% A. Chau 4/22/10
%
% Added some comments A. Chau 2/15/10


% Parse inputs
p=inputParser;
p.addRequired('x', @isnumeric);
p.addRequired('y', @isnumeric);
p.addRequired('u', @isnumeric);
p.addRequired('v', @isnumeric);
p.addOptional('s',[], @isnumeric);
p.addParamValue('skip', 1, @isnumeric);
p.addParamValue('xskip', [], @isnumeric);
p.addParamValue('yskip', [], @isnumeric);
p.addParamValue('norm', 0, @isnumeric);
p.parse(x,y,u,v, varargin{:})
s = p.Results.s;
skip = p.Results.skip;
xskip = p.Results.xskip;
yskip = p.Results.yskip;
donorm = p.Results.norm;


% set xskip and yskip if they were not explicitly set
if isempty(xskip)
    xskip = skip;
end
if isempty(yskip)
    yskip = skip;
end

% calculate normalized vectors if desired
if donorm
    mag = sqrt(u.^2+v.^2);
    u = u./mag;
    v = v./mag;
end

% call Matlab's quiver, using the newly defined skip parameters
if isempty(s) % with no "s" input to quiver
    h = quiver(x(1:yskip:end,1:xskip:end), y(1:yskip:end,1:xskip:end), ...
        u(1:yskip:end,1:xskip:end), v(1:yskip:end,1:xskip:end));
else % with an "s" input to quiver
    h = quiver(x(1:yskip:end,1:xskip:end), y(1:yskip:end,1:xskip:end), ...
        u(1:yskip:end,1:xskip:end), v(1:yskip:end,1:xskip:end),s);
end


end %function