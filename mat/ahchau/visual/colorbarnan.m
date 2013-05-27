function hc=colorbarnan(varargin)
%function colorbarnan(varargin)
% colorbarnan('caxis',[-5 5])
% created by A. Chau 8/9/10
% edited by A. Chau 11/29/11
% p.addParamValue('caxis',[],@(x)isnumeric(x) && length(x)==2);
% p.addParamValue('location','EastOutside',@(x)ischar(x));
% p.addParamValue('keepcolormap',0,@(x)isnumeric(x));
% p.addParamValue('cmap','jet',@(x)ischar(x));
% p.addParamValue('nancolor',[1 1 1],@(x)isnumeric(x));
% p.addParamValue('addnan',1,@(x)isnumeric(x));

p=inputParser;
p.addParamValue('caxis',[],@(x)isnumeric(x) && length(x)==2);
p.addParamValue('location','EastOutside',@(x)ischar(x));
p.addParamValue('keepcolormap',0,@(x)isnumeric(x));
p.addParamValue('cmap','jet',@(x)ischar(x));
p.addParamValue('nancolor',[1 1 1],@(x)isnumeric(x));
p.addParamValue('addnan',1,@(x)isnumeric(x));
p.parse(varargin{:});
clim = p.Results.caxis;
location = p.Results.location;

hc=colorbar('location',location);
if p.Results.keepcolormap
    a = colormap;
else
    switch p.Results.cmap
        case 'mybluewhitered'
            if isempty(clim)
                a = mybluewhitered(255,[-1 1]);
            else
                a = mybluewhitered(255,clim);
            end
        case 'jet'
            a = colormap(jet(256));
        otherwise
            %             fprintf('the cmap is %s',p.Results.cmap);
            a = eval([p.Results.cmap '(256);']); % dangerous?!
            %             a = colormap(p.Results.cmap(256));
    end
end

if isempty(clim)
%     disp('clim was empty')
    clim = caxis;
end
delta = (clim(2)-clim(1))/254;
caxis([clim(1)-delta-eps clim(2)]);

%a(1,:) = [1 1 1];
a(1,:) = p.Results.nancolor;
colormap(a);