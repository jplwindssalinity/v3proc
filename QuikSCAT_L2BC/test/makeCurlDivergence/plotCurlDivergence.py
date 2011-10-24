from numpy import *
from pylab import *
from netCDF4 import Dataset

def plot_curl_divergence_results(smooth_file="qs_l2c_44444_v3_200801010051.nc",
                                 min_row=680,max_row=780):

    sec2days = 1./(3600.*24.)
    
    fnc = Dataset(smooth_file)
    fvar = fnc.variables

    curl = fvar['wind_curl'][:]/sec2days
    div = fvar['wind_divergence'][:]/sec2days

    figure()
    subplot(2,1,1)
    imshow(curl,vmin=-5,vmax=5)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Curl (1/day)");draw()
    title('Curl: pass 44444')
    subplot(2,1,2)
    imshow(div,vmin=-5,vmax=5)
    ylim(min_row,max_row)
    cbar = colorbar(orientation='vertical',shrink=0.8)
    cbar.set_label("Divergence (1/day)");draw()
    title('Divergence: pass 44444')
    xlabel('WVC')

    savefig('curl_divergence.png')

