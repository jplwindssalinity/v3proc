from numpy import *
from pylab import *
from netCDF4 import Dataset

def plot_direction_filtered_results(unsmooth_file="QS_S2B44444.20080021557_S3.L2BC.nc",
                                    smooth_file="qs_l2c_44444_v3_200801010051.nc",
                                    min_row=680,max_row=780):

    nc = Dataset(unsmooth_file)
    var = nc.variables

    fnc = Dataset(smooth_file)
    fvar = fnc.variables

    model_dir = var['model_wind_direction'][:]*pi/180
    unsmooth_dir = var['wind_to_direction'][:]*pi/180
    smooth_dir = fvar['wind_direction'][:]*pi/180

    cdphi = exp(1j*unsmooth_dir)*exp(-1j*model_dir)
    dphi = arctan2(cdphi.imag, cdphi.real)*180/pi

    scdphi = exp(1j*smooth_dir)*exp(-1j*model_dir)
    sdphi = arctan2(scdphi.imag, scdphi.real)*180/pi

    figure()
    subplot(2,1,1)
    imshow(dphi,vmin=-20,vmax=20)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Direction Difference (deg)");draw()
    title('Unsmooth - Model direction: pass 44444')
    subplot(2,1,2)
    imshow(sdphi,vmin=-20,vmax=20)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Direction Difference (deg)");draw()
    title('Smooth - Model direction: pass 44444')
    xlabel('WVC')

    savefig('smooth_vs_unsmooth_direction_comparison.png')

def plot_speed_filtered_results(unsmooth_file="QS_S2B44444.20080021557_S3.L2BC.nc",
                                    smooth_file="qs_l2c_44444_v3_200801010051.nc",
                                    min_row=680,max_row=780):

    nc = Dataset(unsmooth_file)
    var = nc.variables

    fnc = Dataset(smooth_file)
    fvar = fnc.variables

    model_speed = var['model_wind_speed'][:]
    unsmooth_speed = var['wind_speed'][:]
    smooth_speed = fvar['wind_speed'][:]

    figure()
    subplot(2,1,1)
    imshow(unsmooth_speed-model_speed,vmin=-5,vmax=5)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Speed Difference (m/s)");draw()
    title('Unsmooth - Model speed: pass 44444')
    subplot(2,1,2)
    imshow(smooth_speed-model_speed,vmin=-5,vmax=5)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Speed Difference (m/s)");draw()
    title('Smooth - Model speed: pass 44444')
    xlabel('WVC')

    savefig('smooth_vs_unsmooth_speed_comparison.png')
