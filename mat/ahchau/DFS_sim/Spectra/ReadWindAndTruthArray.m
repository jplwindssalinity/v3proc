function wind = ReadWindAndTruthArray(filename,tfilename,res)
% function wind = ReadWindAndTruthArray(filename,tfilename,res)
%
% FILENAME is the name of the retrieved winds in an array file 
% TFILENAME is the name of the true winds in an array file
% RES is supposed to be resolution, in km on the ground, in the latitude
% direction. It gets stored in the output structure but is not used for
% calculating anything, so you need not set it.
%
%
% EXAMPLE USAGE: 
%  wind = ReadWindArray('l2b_QS12.5_GS_arrays.dat','l2b_QS12.5_GS_trutharrays.dat',12.5)
%
%
% A. Chau 2/2/10
% modified from ReadwindArray.m 2/8/10
% modified comments 4/16/10
% modified 5/4/10

[spd,dir,lat,lon,ati1,nati,ncti]=read_l2b_array(filename);

spd = spd';
dir = dir';
ind = spd==-1;
spd(ind) = 0;
u = spd.*cos(dir/180*pi);
v = spd.*sin(dir/180*pi);
spd(ind) = nan; dir(ind) = nan; u(ind) = nan; v(ind) = nan;%%%%%

ind = lat ==0; lat(ind) = nan; lon(ind) = nan;



% wind.spd = spd; wind.dir = dir;
wind.ati1 = ati1; wind.nati = nati; wind.ncti = ncti;
wind.lat = lat'; wind.lon = lon';
% wind.retlat = lat; wind.retlon = lon;
wind.deltalat = res;
wind.retfilename = filename;
wind.retumat = u; wind.retvmat = v;
wind.retsmat = spd; wind.retdmat = dir;


% % read in the truth array too
% tfilename = strrep(filename,'_arrays','trutharrays');
if exist(tfilename,'file')
    [spd,dir,lat,lon,ati1,nati,ncti]=read_l2b_array(tfilename);
    
    spd = spd';
    dir = dir';
    % get direction from 0 to 360 instead of -180 to 180
    ind = dir<0;
    dir(ind) = dir(ind) + 360;
    
    ind = spd==-1;
    spd(ind) = 0;
    u = spd.*cos(dir/180*pi);
    v = spd.*sin(dir/180*pi);
    spd(ind) = nan; dir(ind) = nan; u(ind) = nan; v(ind) = nan;%%%%%
    
    ind = lat ==0; lat(ind) = nan; lon(ind) = nan;
    
    
    % wind.spd = spd; wind.dir = dir;
    wind.trufilename = tfilename;
    wind.truumat = u; wind.truvmat = v;
    wind.trusmat = spd; wind.trudmat = dir;
    wind.trulat = lat'; wind.trulon = lon';
else
    wind.trufilename = [];
    wind.truumat = []; wind.truvmat = [];
    wind.trusmat = []; wind.trudmat = [];
    wind.trulat = []; wind.trulon = [];
end