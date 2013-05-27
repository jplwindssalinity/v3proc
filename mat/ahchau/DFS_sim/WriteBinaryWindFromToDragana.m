function [latlim lonlim] = WriteBinaryWindFromToDragana(ToDraganafile,binarywindfile)
% function [latlim lonmin] = WriteBinaryWindFromToDragana( ...
%                             ToDraganafile,binarywindfile)
% 
% Reads a ToDragana... file and writes a binary wind file, little endian
% The inputs TODRAGANAFILE and BINARYWINDFILE are both strings containing
% the filenames. If BINARYWINDFILE exists, I think it will be overwritten.
%
% Created by A. Chau 7/29/09

%
% ToDraganafile = 'ToDragana_bandC_46deg_res4.0_2008188_120000_Raid107.dat'
% binarywindfile = 'testwind.unf';


%%% Read the lat/lon and winds from the ToDragana file
fid=fopen(ToDraganafile,'rt');
a = textscan(fid,['%*f %*f %*f %f %f %*f %*f %*f %*f %*f ' ...
    '%*f %*f %*f %*f %*f %*f %*f %f %f']);
lat=a{:,1}; lon=a{:,2}; u=a{:,3}; v=a{:,4};
fclose(fid);

% 
nlat = length(unique(lat));
nlon = length(unique(lon));
latmin = min(lat(:));
latmax = max(lat(:));
lonmin = min(lon(:));
lonmax = max(lon(:));
latlim = [latmin latmax];
lonlim = [lonmin lonmax];

u = reshape(u,600,600); u = u';
v = reshape(v,600,600); v = v';
% 
fidwrite = fopen(binarywindfile,'wb','l');
fwrite(fidwrite,latmin,'float');
fwrite(fidwrite,nlon,'int32');
fwrite(fidwrite,nlat,'int32');
fwrite(fidwrite,latmax,'float');
fwrite(fidwrite,lonmin,'float');
fwrite(fidwrite,u,'float');
fwrite(fidwrite,v,'float');
fwrite(fidwrite,lonmax,'float');
fclose(fidwrite);


% this was for testing
%[t1,t2,t3,t4]=read_SV_windfield(binarywindfile);
% [a1,a2,a3,a4]=read_SV_windfield('wind_model_SAM.unf')

end


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
