function wind = ReadWRFArray(filename,res)
% function wind = ReadWRFArray(filename,res)
% 
% A. Chau 2/9/10

[u,v,latlim,lonlim,nlons,nlats]=read_SV_windfield(filename);

wind.filename = filename;
wind.latlim = latlim;
wind.lonlim = lonlim;
wind.u = u';
wind.v = v';
wind.nlons = nlons;
wind.nlats = nlats;
wind.deltalat = res;

