function [out, indvalid, indreplace] = FillInQSRainGaps(data,varargin)
% function [out, indvalid, indreplace] = FillInQSRainGaps(data,varargin)
%
%
% Created A. Chau 8/30/10

p = inputParser;
p.addParamValue('indvalid',-1);
p.addParamValue('indreplace',-1);
p.parse(varargin{:});
indvalid = p.Results.indvalid;
indreplace = p.Results.indreplace;

%% First determine which cells to not interpolate and to interpolate over

if indvalid == -1
    
    % If retrieval was not performed or if land flag is on, do not try
    % to interpolate -->  bit 10 = 1 or bit 8 = 1 (counting from 1)
    mask = uint16(0);
    mask = bitset(mask,10,1); mask = bitset(mask,8,1);
    tmpval = bitand(data.flag,mask);
    nointerp = tmpval>0;
    
    % If retrieval was performed, and if rain flag is usable, and if
    % rain is flagged, do interpolate --? bit 10 = 0 AND bit 13 = 0 and
    % bit 14 =1
    
    % first flip bit 14
    mask = uint16(0); mask = bitset(mask,14,1);
    tmpval = bitxor(data.flag,mask);
    
    % now check that bits 10, 13, and 14 are all 0
    mask = uint16(0); mask = bitset(mask,10,1); mask = bitset(mask,13,1); mask = bitset(mask,14,1);
    tmpval = bitand(tmpval,mask);
    indrain = tmpval == 0;
    
    % interpolate over indrain
    
    indvalid = ~nointerp & ~indrain;
    indreplace = indrain;
end

[lonindmat,latindmat] = meshgrid(1:size(data.lon,2), 1:size(data.lon,1));
lonindvalid = lonindmat(indvalid);
latindvalid = latindmat(indvalid);
lonvalid = data.lon(indvalid); latvalid = data.lat(indvalid);
Flon = TriScatteredInterp(lonindvalid(:),latindvalid(:),lonvalid(:));
Flat = TriScatteredInterp(lonindvalid(:),latindvalid(:),latvalid(:));
tmpu = data.u(indvalid); tmpv=data.v(indvalid);
Fu = TriScatteredInterp(lonvalid(:),latvalid(:),tmpu(:));
Fv = TriScatteredInterp(lonvalid(:),latvalid(:),tmpv(:));


%% Fill in gaps
newlon = data.lon; newlat = data.lat;
newu = data.u; newv = data.v;

gaps = zeros(size(data.u));

% for col = 1:size(data.lon,2)
% %     tmp = data.lon(:,col);
% %     ind = find(~isnan(tmp));
%     ind = indvalid(:,col);
%     if ~isempty(ind)
%         wantind = zeros(size(tmp));
%         wantind(ind(1):ind(end)) = 1;
%         wantind(nointerp) = 0;
%
%     end
% end


gaps(indreplace) = 1;
tmplon = Flon(lonindmat(indreplace),latindmat(indreplace));
if ~isempty(tmplon)
    newlon(indreplace) = tmplon;
    newlat(indreplace) = Flat(lonindmat(indreplace),latindmat(indreplace));
    newu(indreplace) = Fu(newlon(indreplace),newlat(indreplace));
    newv(indreplace) = Fv(newlon(indreplace),newlat(indreplace));
end

out = data;

out.lat = newlat;
out.lon = newlon;
out.u = newu;
out.v = newv;
out.spd = sqrt(newu.^2+newv.^2);
out.dir = atan2(newu,newv)/pi*180;
out.changed = gaps;
out.source = [out.source '_RainFilled'];
