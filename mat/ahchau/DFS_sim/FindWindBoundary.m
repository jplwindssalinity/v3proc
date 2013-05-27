function [lonbound latbound numlon numlat] = FindWindBoundary(ToDraganafile)
% function [lonbound latbound] = FindWindBoundary(ToDraganafile)
% 
% lonbound and latbound are vectors with the boundary of the wind field in
% the ToDraganafile passed in
% 
% Created by A. Chau 8/21/09

% load TestTrees
% lat = latmat(:);
% lon = lonmat(:);


fid=fopen(ToDraganafile,'rt');
a = textscan(fid,['%*f %*f %*f %f %f %*f %*f %*f %*f %*f ' ...
    '%*f %*f %*f %*f %*f %*f %*f %*f %*f']);
lat=a{:,1}; lon=a{:,2}; 
fclose(fid);

%%
% figure out how many rows there are (latitude)
d = diff(lat);
% plot(d(1:2000))
ind = find(abs(d)>1); % find when difference in lat vals is greater than 1 deg
numlat = ind(1);
% figure out how many cols there are  (longitude)
numlon = length(lat)/numlat;
% reshape
disp(sprintf('Num lat = %d, Num lon = %d',numlat, numlon))
latmat = reshape(lat,[numlat numlon]);
lonmat = reshape(lon,[numlat numlon]);
%%
% clf
% plot(lonmat(1:10:end,1:10:end),latmat(1:10:end,1:10:end),'*b')


%%
latbound = [latmat(1:end,1); latmat(end,2:end)'; ...
    flipud(latmat(2:end,end)); fliplr(latmat(1,2:end-1))'];
lonbound = [lonmat(1:end,1); lonmat(end,2:end)'; ...
    flipud(lonmat(2:end,end)); fliplr(lonmat(1,2:end-1))'];

% figure(1); 
% plot(lon,lat,'*')
% 
% figure(2)
% plot(lonbound,latbound,'*')
% axis equal



%% Information about the ToDragana file format
% % To Dragana file format has 19 columns
% % 1: column averaged RR [mm/h], RR_final_av
% % 2: column integrated RR [mm/h*km], RR_final_int
% % 3: surface rain rate [mm/h], RR_final_sfc
% % 4: lat
% % 5: lon
% % 6: volume backscatter by the rain (sigma0), attenuated by the
% % intervening layers, missing = -200, data_VolSigma_attn
% % 7: 2-way attenuation at the incidence angle, in posistive dB, missing
% % = 0, data_PathAttn
% % 8: vertically integrated vapor [kg/m^2=mm], data_Qv_int
% % 9: vertically integrated vapor [kg/m^2=mm], data_Qc_int
% % 10: vertically integrated vapor [kg/m^2=mm], data_Qr_int
% % 11: vertically integrated vapor [kg/m^2=mm], data_Qi_int
% % 12: vertically integrated vapor [kg/m^2=mm], data_Qs_int
% % 13: vertically integrated vapor [kg/m^2=mm], data_Qg_int
% % 14: max attenuated reflectivity in the column [dBz],
% % data_MaxdBz_attn_ave
% % 15: volumne backscatter by the rain (sigma0), NOT attenuated by the
% % intervening layers, missing = -200, data_VolSigma
% % 16: 2-way attenuation at the incidence angle, in positive dB minuse
% % ClearAir attenuation, missing = 0, data_PathAttn-clearAir_PathAttn
% % 17: splash [dB], sigma0_splash_sfc
% % 18: u component at 10 m in [m/s], data_U10
% % 19: v component at 10 m in [m/s], data_V10
