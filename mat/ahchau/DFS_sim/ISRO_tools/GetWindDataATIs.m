function out = GetWindDataATIs(winddata,atis)
% function out = GetWindDataATIs(winddata,atis)
% Created by A. Chau 8/24/10


out.lat = winddata.lat(atis,:);
out.lon = winddata.lon(atis,:);
out.spd = winddata.spd(atis,:);
out.dir = winddata.dir(atis,:);
if ~isempty(winddata.flag)
    out.flag = winddata.flag(atis,:);
end
out.wvcsize = winddata.wvcsize;
out.source = winddata.source;