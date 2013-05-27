function windnogaps = FillInGaps_windarray(wind)
% function FillInGaps_windarray(wind)
% expects a wind structure ( output of ReadWindAndTruthArray.m)
%
% A. Chau 5/4/10

%% Copy basic things over to new structure
windnogaps.ati1 = wind.ati1;
windnogaps.nati = wind.nati;
windnogaps.ncti = wind.ncti;
windnogaps.deltalat = wind.deltalat;



%% Prepare for retrieved winds
ind = ~isnan(wind.lon);
[lonindmat, latindmat] = meshgrid(1:size(wind.lon,2),1:size(wind.lon,1));
lonindvalid = lonindmat(ind);
latindvalid = latindmat(ind);
lonvalid = wind.lon(ind); latvalid = wind.lat(ind);
Flon = TriScatteredInterp(lonindvalid,latindvalid,wind.lon(ind));
Flat = TriScatteredInterp(lonindvalid,latindvalid,wind.lat(ind));
Fu = TriScatteredInterp(lonvalid,latvalid,wind.retumat(ind));
Fv = TriScatteredInterp(lonvalid,latvalid,wind.retvmat(ind));

%% Fill in gaps in retrieved winds
newlon = wind.lon; newlat = wind.lat;
newu = wind.retumat; newv = wind.retvmat;
newspd = wind.retsmat;
newdir = wind.retdmat;
gaps = zeros(size(wind.retumat));

for col = 1:size(wind.lon,2)
    tmp = wind.lon(:,col);
    ind = find(~isnan(tmp));
    if ~isempty(ind)
        wantind = zeros(size(tmp));
        wantind(ind(1):ind(end)) = 1; % want all these indices to have values
        % fill in any indices in ind2 that are nans
        nanind = isnan(tmp); %indices of all nans
        repind = wantind & nanind; % want to replace only these indices
        
        gaps(repind,col) = 1;
        newlon(repind,col) = Flon(lonindmat(repind,col),latindmat(repind,col));
        newlat(repind,col) = Flat(lonindmat(repind,col),latindmat(repind,col));
        newu(repind,col) = Fu(newlon(repind,col),newlat(repind,col));
        newv(repind,col) = Fv(newlon(repind,col),newlat(repind,col));
        
        newspd(repind,col) = sqrt(newu(repind,col).^2 + newv(repind,col).^2);
        newdir(repind,col) = atan2(newv(repind,col),newu(repind,col))/pi*180;
    end
end

% convert to speed and direction
% newspd = sqrt(newu.^2 + newv.^2);
% newdir = atan2(newv,newu)/pi*180;
ind = newdir<0; newdir(ind) = newdir(ind)+360;

windnogaps.retfilename = wind.retfilename;

windnogaps.lat = newlat;
windnogaps.lon = newlon;
windnogaps.retumat = newu;
windnogaps.retvmat = newv;
windnogaps.retsmat = newspd;
windnogaps.retdmat = newdir;
windnogaps.retgaps = gaps;


%% TRUE WINDS
% if isfield(wind,'trufilename')
if ~isempty(wind.trufilename)
    %% Prepare for true winds
    ind = ~isnan(wind.trulon);
    [lonindmat, latindmat] = meshgrid(1:size(wind.lon,2),1:size(wind.lon,1));
    lonindvalid = lonindmat(ind);
    latindvalid = latindmat(ind);
    lonvalid = wind.trulon(ind); latvalid = wind.trulat(ind);
    Flon = TriScatteredInterp(lonindvalid,latindvalid,wind.trulon(ind));
    Flat = TriScatteredInterp(lonindvalid,latindvalid,wind.trulat(ind));
    Fu = TriScatteredInterp(lonvalid,latvalid,wind.truumat(ind));
    Fv = TriScatteredInterp(lonvalid,latvalid,wind.truvmat(ind));
    
    
    %% Fill in gaps in true winds
    
    
    newlon = wind.trulon; newlat = wind.trulat;
    newu = wind.truumat; newv = wind.truvmat;
    gaps = zeros(size(newlon));
    newspd = wind.trusmat;
    newdir = wind.trudmat;
    
    for col = 1:size(wind.trulon,2)
        tmp = wind.trulon(:,col);
        ind = find(~isnan(tmp));
        if ~isempty(ind)
            wantind = zeros(size(tmp));
            wantind(ind(1):ind(end)) = 1; % want all these indices to have values
            % fill in any indices in ind2 that are nans
            nanind = isnan(tmp); %indices of all nans
            repind = wantind & nanind; % want to replace only these indices
            
            gaps(repind,col) = 1;
            newlon(repind,col) = Flon(lonindmat(repind,col),latindmat(repind,col));
            newlat(repind,col) = Flat(lonindmat(repind,col),latindmat(repind,col));
            newu(repind,col) = Fu(newlon(repind,col),newlat(repind,col));
            newv(repind,col) = Fv(newlon(repind,col),newlat(repind,col));
            
            newspd(repind,col) = sqrt(newu(repind,col).^2 + newv(repind,col).^2);
            newdir(repind,col) = atan2(newv(repind,col),newu(repind,col))/pi*180;
        end
    end
    
    % convert to speed and direction
    % newspd = sqrt(newu.^2 + newv.^2);
    % newdir = atan2(newv,newu)/pi*180;
    ind = newdir<0; newdir(ind) = newdir(ind)+360;
    
    windnogaps.trufilename = wind.trufilename;
    windnogaps.trulat = newlat;
    windnogaps.trulon = newlon;
    windnogaps.truumat = newu;
    windnogaps.truvmat = newv;
    windnogaps.trusmat = newspd;
    windnogaps.trudmat = newdir;
    windnogaps.trugaps = gaps;
else
    windnogaps.trufilename = [];
    windnogaps.trulat = []; windnogaps.trulon = [];
    windnogaps.truumat = []; windnogaps.truvmat = [];
    windnogaps.trusmat = []; windnogaps.trudmat = [];
    windnogaps.trugaps = [];
end

