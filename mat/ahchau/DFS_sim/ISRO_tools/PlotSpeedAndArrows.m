function PlotSpeedAndArrows(data,varargin);
% function PlotSpeedAndArrows(data);
% 
% assumes data.lat exists
% assumes data.spd, data.dir, data.u, data.v (all or only u,v are present)
%
% created by A. Chau 8/30/10

p=inputParser;
p.addRequired('data',@isstruct);
p.addParamValue('skip',10,@isnumeric);
p.parse(data,varargin{:});
skip = p.Results.skip;

if ~isfield(data,'spd')
    data.spd = sqrt(data.u.^2 + data.v.^2);
end

[xmat,ymat] = meshgrid(1:size(data.spd,2), 1:size(data.spd,1));

% size(xmat)
imagesc(data.spd)
hold on


% figure out whether increasing index is going north (ascending) or south 
if mean(diff(data.lat(:,20))) > 0 
    orbitdir = 'ascending';
else
    orbitdir = 'descending';
end


switch orbitdir
    case 'ascending'
        hw = myquiver(xmat,ymat,data.u,data.v,'skip',skip);
        hb = myquiver(xmat,ymat,data.u,data.v,'skip',skip);
        axis xy
        xlabel('East \rightarrow')
        ylabel('North \rightarrow')
    case 'descending'
        hw = myquiver(xmat,ymat,-data.u,-data.v,'skip',skip);
        hb = myquiver(xmat,ymat,-data.u,-data.v,'skip',skip);
        ylabel('North \rightarrow')
        xlabel('\leftarrow East (decreasing index)')
end
set(hw,'color','white','linewidth',2);
set(hb,'color','black')

hold off