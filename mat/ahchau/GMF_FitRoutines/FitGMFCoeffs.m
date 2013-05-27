function [fits, chi, s0, keepind] = FitGMFCoeffs(varargin)
% function fits = FitGMFCoeffs(varargin)
%
% parameters:
%   outerinner: 'outer','inner','both'
%   datadir: directory from which to read the big file
%   %%%%%%savename: 
%   spdbinwidth: single number for bin widths
%   spdbincenterspacing: spacing (0.25)
%   spdbincenters: vector of centers. if set, code ignores spdbinwidth and
%                  spdbincenterspacing
%   makeintplots: make intermediate plots (histograms, etc.)
%   saveintplots: save intermediate plots (histograms, etc.)
%   makefitplots: make fit coefficient plots
%   savefitplots: save fit coefficient plots
%   savedir: location to save plots
%   savegmf:
%   numcoefs: default is 3
%   namestr: 
%   foldtopi: default is 0, 1 will fold rel az is 180 to 360 interval
%             before fitting (and refitting)
%   inputAcoeffs_outer:
%   inputAcoeffs_inner:  can input some A coefficients to constrain the fit.
%       Should pass in a matrix with the correct number of parameters,
%       putting 0 in the columns where the parameters should be fit.
%       Best to also pass in corresponding spdbincenters!!
%
%   refitmethod: 'dBthreshhold' - does fit, then removes data outside some
%                   threshold in dB. Must also pass in keyword and value,
%                   e.g.:   'threshold',5
%                'kpthreshold', pass in kp value, e.g. 'threshold',.1 
%                'sigmafromdata_bybin' estimate sigma from data, refit.
%                   pass in 'threshold',3 to throw out +/- 3 sigma
%                   pass in width of azimuth bins to use to estimate sigma,
%                     'threshold_width',20 uses 20 deg wide bins
%                'sigmafromdata_bymodel' is similar to
%                  'sigmafromdata_bybin', but uses the model mean instead
%                  of the bin mean 
%                'sigmafromdatatrimmed_bymodel' is similar to
%                  'sigmafromdata_bymodel', but uses a trimmed std dev
%                  estimate from the data. pass in 'trim_val',20 to throw
%                  out top 10% and bottom 10% when calculate std.
%                'sigmafromdatatrimmed_bymodel_trimmedfit' -- NOT
%                IMPLEMENTED YET!!
%    inputfileformat: 'catmat', a single mat file with ALL the data
%                     'unsortedmats', multiple mat files ("sorted" by rev,
%                           but not by speed
%                     'v0binspd', *v0.bin file, already segregated by speed
%
% Created by A. Chau 1/17/12
% Modified by A. Chau 1/24/12. Add ability to exclude data and refit using
%   keywords (v1, v2)
% Modified by A. Chau 1/25/12.  Added 'sigmafromdata_bybin',
%   'sigmafromdata_bymodel', and 'sigmafromdatatrimmed_bymodel' (v3,v4)
% Modified by A. Chau 1/26/12.  To add
% sigmafromdatatrimmed_bymodel_trimmedfit. NOt done yet.
% Modfiied by A. Chau 1/31/12. Added comments for better log file.  Fixed
%   an error when passing in true_spd_field
% Modified by A. Chau 2/13/12 (old ver v8). Added 'foldtopi'.
% Modified by A. Chau 2/22/12 (old ver is v10). Make inputAcoeffs work!
% Modified by A. Chau 8/2/12 (old ver is v11). Modify to read data from 
%   many files and do the concatenation before fitting.  (for spinning
%   data)
% Modified by A. Chau 8/21/12 (old ver is v12). Added output variables
% Modified by A. Chau 10/9/12 (old ver is v13). Make function able to read
%   from v0bin file. (first created for ISRO data)



%%%%%%%%%%%%%%%%%%%%
%%% Parse inputs %%%
%%%%%%%%%%%%%%%%%%%%
p = inputParser;
p.addParamValue('outerinner',[],@ischar);
p.addParamValue('datadir',[],@ischar);
p.addParamValue('spdbinwidth',0.5,@isnumeric);
p.addParamValue('spdbincenterspacing',0.25,@isnumeric);
p.addParamValue('spdbincenters',[],@isnumeric);
p.addParamValue('makeintplots',1);
p.addParamValue('saveintplots',0);
p.addParamValue('makefitplots',1);
p.addParamValue('savefitplots',0);
p.addParamValue('savedir',[],@ischar);
p.addParamValue('savegmf',1,@isnumeric);
p.addParamValue('numcoefs',3,@isnumeric);
p.addParamValue('namestr',[],@ischar);
p.addParamValue('refitmethod',[]);
p.addParamValue('threshold',[]);
p.addParamValue('stopcondition',[]);
p.addParamValue('threshold_width',[]);
p.addParamValue('trim_val',[]);
p.addParamValue('true_spd_field','wsat_wspd_aw');
p.addParamValue('true_dir_field','chi');
p.addParamValue('maxiter',20,@isnumeric);
p.addParamValue('foldtopi',0);
p.addParamValue('inputAcoeffs_outer',[],@isnumeric);
p.addParamValue('inputAcoeffs_inner',[],@isnumeric);
p.addParamValue('inputfileformat','catmat',@ischar);
p.parse(varargin{:});

% deal with the inputs
outerinner = p.Results.outerinner;
outerinner(1) = upper(outerinner(1));
datadir = p.Results.datadir;
spdbinwidth = p.Results.spdbinwidth;
spdbincenterspacing = p.Results.spdbincenterspacing;
if ~isempty(p.Results.spdbincenters)
    spdbincenters = p.Results.spdbincenters;
else
    spdbincenters = spdbinwidth/2 : spdbincenterspacing : 20;
end
makeintplots = p.Results.makeintplots;
saveintplots = p.Results.saveintplots;
makefitplots = p.Results.makefitplots;
savefitplots = p.Results.savefitplots;
savedir = p.Results.savedir;
numcoefs = p.Results.numcoefs;
inputfileformat = p.Results.inputfileformat;

%%%%%%%%%%%%%%%%%%%%%
%%% Initial stuff %%%
%%%%%%%%%%%%%%%%%%%%%
% if the savedir doesn't exist, create it
if ~exist(savedir)
    mkdir(savedir)
    createddir = 1;
else
    createddir = 0;
end

% open the log file
logfile = LogFile(fullfile(savedir,['FitGMFLog_' datestr(now,30) '.txt']));
logfile.PrintScreenLog('\n*******************************\n');
logfile.PrintScreenLog('***** Started fitting GMF *****\n');
logfile.PrintScreenLog('*******************************\n');
% print some information
logfile.PrintScreenLog(sprintf('Will fit data in %s\n', datadir));
if createddir
    logfile.PrintScreenLog(sprintf('Created directory %s\n',savedir));
end
if saveintplots
    logfile.PrintScreenLog('Will save intermediate plots\n');
else
    logfile.PrintScreenLog('Will NOT save intermediate plots\n');
end
if savefitplots
    logfile.PrintScreenLog('Will save fit coefficient plots\n');
else
    logfile.PrintScreenLog('Will NOT save fit coefficient plots\n');
end



% Prepare coast
load coast
long(long<0) = long(long<0)+360;




%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Start the big loop %%%
%%%%%%%%%%%%%%%%%%%%%%%%%%

% How many times do we need to run the big loop?
switch outerinner
    case 'Both'
        num_loop = 2; doboth = 1;
    case {'Outer','Inner'}
        num_loop = 1; doboth = 0;
    otherwise
        num_loop = 0;
end

for q_bigloop = 1:num_loop % big loop should run at most twice, once for outer and once for inner
    if doboth
        switch q_bigloop % outer or inner on this loop?
            case 1; outerinner = 'Outer';
            case 2; outerinner = 'Inner';
        end % switch q_bigloop % outer or inner on this loop?
    end
    
    % start a timer and show some output
    loopstart = tic;
    logfile.PrintScreenLog(sprintf('\nStarting to fit %s beam!\n',outerinner));
   
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%% Which input data files will we use? %%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %     bigcatfile = 0; % should be one if a big concatenated file exists
    switch inputfileformat
        case 'catmat' % Load from a big concatenated data file
            filename = fullfile(datadir,sprintf('Cat%s.mat',outerinner));
            if exist(filename,'file')
                N_loadandfit = 1;
                logfile.PrintScreenLog(sprintf('Will load once from %s...',filename));
            else
                % error('%s does not exist!',filename)
                break
            end
        case 'unsortedmats' % Load from several mat files, which each contain a range of speeds
            fileroot = sprintf('Cat%s_',outerinner);
            % get the files that start with fileroot
            tmp = dir(fullfile(datadir,sprintf('%s*.mat',fileroot)));
            filenames = {tmp.name};
            % will read data from
            totalnumfiles = length(filenames);
            filenames'
            %         totalnumfiles = 2; % this is for debugging
            N_loadandfit = length(spdbincenters);
        case 'v0binspd' % Load from binary files segregated by speed 
            fileroot = sprintf('Cat%s_',outerinner);
            % construct the filenames from the speed centers and bin width
            for q_spd = 1:length(spdbincenters)
               tmp = sprintf('%sctr%05.2f_wdt%03.2f_v0.bin',fileroot, ...
                   spdbincenters(q_spd),spdbinwidth);
               filenames(q_spd) = {tmp};
            end
            totalnumfiles = length(filenames);
            filenames'
            N_loadandfit = length(spdbincenters);
    end
    
    
    % loop to load and do fits
    for q_loadandfit = 1:N_loadandfit
        %%%%%%%%%%%%%%%%%%%%%
        %%% Load the data %%%
        %%%%%%%%%%%%%%%%%%%%%
        
        switch inputfileformat
            case 'catmat'
                % if there is one big cat file, we can just load it here
                % if not, we need to load, filter, and concatenate
                logfile.PrintScreenLog(sprintf('Loading %s...',filename));
                load(filename)
                logfile.PrintScreenLog('done!\n');
                spdstr = sprintf('%.2fto%.2fmpers', ...
                    spdbincenters(1) - spdbinwidth/2, ...
                    spdbincenters(end) + spdbinwidth/2);
            case 'unsortedmats'
                % several mat files not sorted by speed
                thisloopspeed = spdbincenters(q_loadandfit);
                logfile.PrintScreenLog(sprintf('%.2f m/s:  Loading files: ',thisloopspeed));
                for q_file = 1:totalnumfiles
                    logfile.PrintScreenLog(sprintf('%d..',q_file));
                    % load the data
                    load(fullfile(datadir,filenames{q_file}));
                    allfieldnames = fieldnames(BIGdata);
                    % filter the data
                    ind_spd = abs(BIGdata.(p.Results.true_spd_field) ...
                        - thisloopspeed) < (spdbinwidth/2);
                    % concatenate the data
                    if ~exist('data','var')
                        for q_fieldnames = 1:length(allfieldnames)
                            %                         data.(RemapFieldName(allfieldnames{q_fieldnames})) = ...
                            %                             BIGdata.(allfieldnames{q_fieldnames})(ind_spd);
                            data.(allfieldnames{q_fieldnames}) = ...
                                BIGdata.(allfieldnames{q_fieldnames})(ind_spd);
                        end
                    else
                        for q_fieldnames = 1:length(allfieldnames)
                            %                         data.(RemapFieldName(allfieldnames{q_fieldnames})) = cat(1, ...
                            %                             data.(RemapFieldName(allfieldnames{q_fieldnames})), ...
                            %                             BIGdata.(allfieldnames{q_fieldnames})(ind_spd));
                            data.(allfieldnames{q_fieldnames}) = cat(1, ...
                                data.(allfieldnames{q_fieldnames}), ...
                                BIGdata.(allfieldnames{q_fieldnames})(ind_spd));
                        end
                    end
                end
                spdstr = sprintf('%.2fto%.2fmpers', ...
                    thisloopspeed - spdbinwidth/2, ...
                    thisloopspeed + spdbinwidth/2);
            case 'v0binspd'
                % v0 bin already sorted by speed
                thisloopspeed = spdbincenters(q_loadandfit);
                logfile.PrintScreenLog(sprintf('%.2f m/s: Loading file %s\n', ...
                    thisloopspeed,filenames{q_loadandfit}))
                %load(fullfile(datadir,filenames{q_loadandfit}))
                
                clear data
                
                data = Readv0bin(fullfile(datadir, ...
                    filenames{q_loadandfit}));
                spdstr = sprintf('%.2fto%.2fmpers', ...
                    thisloopspeed - spdbinwidth/2, ...
                    thisloopspeed + spdbinwidth/2);
        end
        
        %%%%%%%%%%%%%%%%%%%%%%%%%
        %%% Report statistics %%%
        %%%%%%%%%%%%%%%%%%%%%%%%%
        
        % Plotting setup
        % meant to use subfigure but never did, don't need this part:
        %subfigparams = [4 5]; subfigtotal = prod(subfigparams);
        
        % save axes handles
        axspdsighist = [];
        
        clear pm
        pm = FitData_PlotMethods; %class that contains plotting methods
        
        binsize_deg = 0.5;
        hist_latbins = -90:binsize_deg:90;
        hist_lonbins = 0:binsize_deg:360;
        goodncep=[]; goodssmi=[]; goodwsatspd = []; goodwsatdir=[];
        
        if isfield(data,'rainrate')
            num_total = length(data.rainrate);
        else if isfield(data,'ssmi_rain')
                num_total = length(data.ssmi_rain);
            else if isfield(data,'l2b_rainimpact')
                    num_total = length(data.l2b_rainimpact);
                end
            end
        end
        if isfield(data,'ncep_speed')
            goodncep = ~isnan(data.ncep_speed);
        else if isfield(data,'ncep_spd')
                goodncep = ~isnan(data.ncep_spd);
            end
        end
        if isfield(data,'wind')
            goodssmi = (~isnan(data.wind)) & (data.wind < 250);
        else if isfield(data,'ssmi_spd')
                goodssmi = (~isnan(data.ssmi_spd)) & (data.ssmi_spd < 250);
            end
        end
        if isfield(data,'wsat_wspd_aw')
            goodwsatspd = (~isnan(data.wsat_wspd_aw)) & (data.wsat_wspd_aw < 250);
            goodwsatdir = (~isnan(data.wsat_wspd_aw)) & (data.wsat_wspd_aw < 250) ...
                & (data.wsat_wdir ~= 251) ;
        else if isfield(data,'wsat_spd_aw')
                goodwsatspd = (~isnan(data.wsat_spd_aw)) & (data.wsat_spd_aw < 250);
                goodwsatdir = (~isnan(data.wsat_spd_aw)) & (data.wsat_spd_aw < 250) ...
                    & (data.wsat_dir ~= 251) ;
            end
        end

        % Print the statistics
        logfile.PrintScreenLog(sprintf('Number of points: %d\n',num_total));
        logfile.PrintScreenLog(sprintf('Number of points with good ncep: %d\n',length(find(goodncep))));
        logfile.PrintScreenLog(sprintf('Number of points with good ssmi: %d\n',length(find(goodssmi))));
        logfile.PrintScreenLog(sprintf('Number of points with good windsat speed: %d\n',length(find(goodwsatspd))));
        logfile.PrintScreenLog(sprintf('Number of points with good windsat dir: %d\n',length(find(goodwsatdir))));
        
        %% Plot of sigma0 vs ecmwf_speed
        if makeintplots
            if isfield(data,'ecmwf_speed')
                tmp_ecmwf = data.ecmwf_speed;
            else if isfield(data,'ecmwf_spd')
                    tmp_ecmwf = data.ecmwf_spd;
                end
            end
            if isfield(data,'sig0lin')
                tmp_sig = data.sig0lin;
            else if isfield(data,'sigma0_attn_corr_lin')
                    tmp_sig = data.sigma0_attn_corr_lin;
                end
            end
            
            hf = myfigure(2);
            pm.PlotHistogramSpdSig0(tmp_ecmwf,tmp_sig,'nwp','fignum',hf);
            clear tmp_ecmwf tmp_sig
            
            xlabel('ECMWF speed  [m/s]');
            ylabel('sigma0 [dB]');
            title({'Histogram: sigma0 vs ECMWF speed', ...
                'for histogram, negative sigma0s are in lowest bin', ...
                'mean was taken in linear units with negative sigma0s kept'})
            legend({'Mean sigma0 as a function of wind speed', ...
                'Mean wind speed as a function of sigma0'})
            axspdsighist = cat(1,axspdsighist,gca);
            
            if saveintplots
                fname = sprintf('Hist_Sig0vsECMWF_%s_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Plot of sigma0 vs ncep_speed
        if makeintplots
            ind = goodncep;
            if isfield(data,'ncep_speed')
                tmp_ncep = data.ncep_speed(ind);
            else if isfield(data,'ncep_spd')
                    tmp_ncep = data.ncep_spd(ind);
                end
            end
            if isfield(data,'sig0lin')
                tmp_sig = data.sig0lin(ind);
            else if isfield(data,'sigma0_attn_corr_lin')
                    tmp_sig = data.sigma0_attn_corr_lin(ind);
                end
            end
            
            hf = myfigure(3);
            pm.PlotHistogramSpdSig0(tmp_ncep,tmp_sig,'nwp','fignum',hf);
            clear ind tmp_ncep tmp_sig
            
            xlabel('NCEP speed [m/s]');
            ylabel('sigma0 [dB]');
            title({'Histogram: sigma0 vs NCEP speed', ...
                'for histogram, negative sigma0s are in lowest bin', ...
                'mean was taken in linear units with negative sigma0s kept'});
            legend({'Mean sigma0 as a function of wind speed', ...
                'Mean wind speed as a function of sigma0'});
            axspdsighist = cat(1,axspdsighist,gca);
            
            if saveintplots
                fname = sprintf('Hist_Sig0vsNCEP_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Plot of sigma0 vs ssmi_speed
        if makeintplots
            ind = goodssmi;
            if isfield(data,'wind')
                tmp_ssmi = data.wind(ind);
            else if isfield(data,'ssmi_spd')
                tmp_ssmi = data.ssmi_spd(ind);
                end
            end
            if isfield(data,'sig0lin')
                tmp_sig = data.sig0lin(ind);
            else if isfield(data,'sigma0_attn_corr_lin')
                    tmp_sig = data.sigma0_attn_corr_lin(ind);
                end
            end
                        
            hf = myfigure(4);
            pm.PlotHistogramSpdSig0(tmp_ssmi,tmp_sig,'sat','fignum',hf);
            clear ind tmp_ssmi tmp_sig
            
            xlabel('SSMI speed [m/s]');
            ylabel('sigma0 [dB]')
            title({'Histogram: sigma0 vs SSMI speed', ...
                'for histogram, negative sigma0s are in lowest bin', ...
                'mean was taken in linear units with negative sigma0s kept'})
            legend({'Mean sigma0 as a function of wind speed', ...
                'Mean wind speed as a function of sigma0'})
            axspdsighist = cat(1,axspdsighist,gca);
            
            if saveintplots
                fname = sprintf('Hist_Sig0vsSSMI_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Plot of sigma0 vs WindSat all-weather
        if makeintplots
            ind = goodwsatspd;
            if isfield(data,'wsat_wspd_aw')
                tmp_wsat = data.wsat_wspd_aw(ind);
            else if isfield(data,'wsat_spd_aw')
                tmp_wsat = data.wsat_spd_aw(ind);
                end
            end
            if isfield(data,'sig0lin')
                tmp_sig = data.sig0lin(ind);
            else if isfield(data,'sigma0_attn_corr_lin')
                    tmp_sig = data.sigma0_attn_corr_lin(ind);
                end
            end
                      
            hf = myfigure(1);
            pm.PlotHistogramSpdSig0(tmp_wsat,tmp_sig,'sat','fignum',hf);
            clear ind tmp_wsat tmp_sig
            
            xlabel('WindSat (all weather) speed [m/s]');
            ylabel('sigma0 [dB]');
            title({'Histogram: sigma0 vs WindSat speed', ...
                'for histogram, negative sigma0s are in lowest bin', ...
                'mean was taken in linear units with negative sigma0s kept'})
            legend({'Mean sigma0 as a function of wind speed', ...
                'Mean wind speed as a function of sigma0'})
            axspdsighist = cat(1,axspdsighist,gca);
            
            linkaxes(axspdsighist)
            
            if saveintplots
                fname = sprintf('Hist_Sig0vsWSAT_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Histogram the locations of the data
        if makeintplots
            if isfield(data,'lon')
                [hist_latlon] = hist2d(data.lon,data.lat,hist_lonbins,hist_latbins);
            else if isfield(data,'cell_lon')
                [hist_latlon] = hist2d(data.cell_lon,data.cell_lat,hist_lonbins,hist_latbins);
                end
            end
            hist_latlon(hist_latlon == 0) = nan;
            myfigure(8);clf
            imagesc(hist_lonbins,hist_latbins,hist_latlon); colorbarnan;
            axis xy; grid on
            hold on; plot(long,lat,'k.')
            title({'Histogram: Data location', ...
                sprintf('bin size = %.2f deg',binsize_deg)})
            
            if saveintplots
                fname = sprintf('Hist_Location_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Histogram of NCEP speed vs ECMWF speed
        if makeintplots
            ind = goodncep;
            if isfield(data,'ecmwf_speed')
                tmp_ecmwf = data.ecmwf_speed(ind);
            else if isfield(data,'ecmwf_spd')
                tmp_ecmwf = data.ecmwf_spd(ind);
                end
            end
            if isfield(data,'ncep_speed')
                tmp_ncep = data.ncep_speed(ind);
            else if isfield(data,'ncep_spd')
                tmp_ncep = data.ncep_spd(ind);
                end
            end
            
            hf = myfigure(9);
            pm.PlotHistogramSpdSpd(tmp_ecmwf,tmp_ncep,'nwp','nwp','fignum',hf);
            clear ind tmp_ecmwf tmp_ncep
            
            xlabel('ECMWF speed [m/s]');
            ylabel('NCEP speed [m/s]');
            title('Histogram: NCEP speed vs ECMWF speed')
            
            if saveintplots
                fname = sprintf('Hist_NCEPvsECMWFspeed_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% Histogram of NCEP direction vs ECMWF direction
        if makeintplots
            ind = goodncep;
            tmp_ecmwf = data.ecmwf_dir(ind);
            tmp_ncep = data.ncep_dir(ind);
            
            hf = myfigure(10);
            pm.PlotHistogramDirDir(tmp_ecmwf,tmp_ncep,'fignum',hf);
            clear ind tmp_ecmwf tmp_ncep
            
            xlabel('ECMWF direction [deg]');
            ylabel('NCEP direction [deg]');
            title('Histogram: NCEP direction vs ECMWF direction')
            
            if saveintplots
                fname = sprintf('Hist_NCEPvsECMWFdirection_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        
        %% WindSat speed vs ECMWF speed
        if makeintplots
            ind = goodwsatspd;
            if isfield(data,'wsat_wspd_aw')
                tmp_wsat = data.wsat_wspd_aw(ind);
            else if isfield(data,'wsat_spd_aw')
                tmp_wsat = data.wsat_spd_aw(ind);
                end
            end
            if isfield(data,'ecmwf_speed')
                tmp_ecmwf = data.ecmwf_speed(ind);
            else if isfield(data,'ecmwf_spd')
                tmp_ecmwf = data.ecmwf_spd(ind);
                end
            end
            
            hf = myfigure(5);
            [~,ax] = pm.PlotHistogramSpdSpd(tmp_ecmwf,tmp_wsat,'nwp','sat','fignum',hf);
            clear ind tmp_wsat tmp_ecmwf
            
            xlabel('ECMWF speed [m/s]');
            ylabel('WindSat speed [m/s]');
            title('Histogram: WindSat speed vs ECMWF speed (all good speeds)')
            
            if saveintplots
                fname = sprintf('Hist_WSATvsECMWFspeed_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% WindSat direction vs ECMWF direction
        if makeintplots
            ind = goodwsatdir;
            if isfield(data,'wsat_wdir')
                tmp_wsat = data.wsat_wdir(ind);
            else if isfield(data,'wsat_dir')
                tmp_wsat = data.wsat_dir(ind);
                end
            end
            tmp_ecmwf = data.ecmwf_dir(ind);
            
            hf = myfigure(6);
            [~,ax] = pm.PlotHistogramDirDir(tmp_ecmwf,tmp_wsat,'fignum',hf);
            clear ind tmp_wsat tmp_ecmwf
            
            xlabel('ECMWF direction [deg]');
            ylabel('WindSat direction [deg]');
            title('Histogram: WindSat direction vs ECMWF direction')
            
            if saveintplots
                fname = sprintf('Hist_WSATvsECMWFdir_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% WindSat speed vs ECMWF speed (good direction only)
        if makeintplots
            ind = goodwsatspd & goodwsatdir;
            if isfield(data,'wsat_wspd_aw')
                tmp_wsat = data.wsat_wspd_aw(ind);
            else if isfield(data,'wsat_spd_aw')
                tmp_wsat = data.wsat_spd_aw(ind);
                end
            end
            if isfield(data,'ecmwf_speed')
                tmp_ecmwf = data.ecmwf_speed(ind);
            else if isfield(data,'ecmwf_spd')
                tmp_ecmwf = data.ecmwf_spd(ind);
                end
            end
                        
            hf = myfigure(7);
            [~,ax] = pm.PlotHistogramSpdSpd(tmp_ecmwf,tmp_wsat,'nwp','sat','fignum',hf);
            clear ind tmp_wsat tmp_ecmwf
            
            xlabel('ECMWF speed [m/s]');
            ylabel('WindSat speed [m/s]');
            title('Histogram: WindSat speed vs ECMWF speed (good speed and direction)')
            
            if saveintplots
                fname = sprintf('Hist_WSATvsECMWFspeed(gooddir)_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        %% SSMI speed vs WindSat speed (good direction only)
        if makeintplots
            ind = goodwsatspd & goodwsatdir & goodssmi;
            if isfield(data,'wsat_wspd_aw')
                tmp_wsat = data.wsat_wspd_aw(ind);
            else if isfield(data,'wsat_spd_aw')
                tmp_wsat = data.wsat_spd_aw(ind);
                end
            end
            if isfield(data,'wind')
                tmp_ssmi = data.wind(ind);
            else if isfield(data,'ssmi_spd')
                tmp_ssmi = data.ssmi_spd(ind);
                end
            end
                        
            hf = myfigure(11);
            [~,ax] = pm.PlotHistogramSpdSpd(tmp_ssmi,tmp_wsat,'sat','sat','fignum',hf);
            clear ind tmp_wsat tmp_ssmi
            
            xlabel('SSMI speed [m/s]');
            ylabel('WindSat speed [m/s]');
            title('Histogram: WindSat speed (good speed and direction) vs SSMI speed')
            
            if saveintplots
                fname = sprintf('Hist_WSATvsSSMIspeed_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        
        %% SSMI speed vs ECMWF speed
        if makeintplots
            ind = goodssmi;
            if isfield(data,'ecmwf_speed')
                tmp_ecmwf = data.ecmwf_speed(ind);
            else if isfield(data,'ecmwf_spd')
                    tmp_ecmwf = data.ecmwf_spd(ind);
                end
            end
            if isfield(data,'wind')
                tmp_ssmi = data.wind(ind);
            else if isfield(data,'ssmi_spd')
                    tmp_ssmi = data.ssmi_spd(ind);
                end
            end
                        
            hf = myfigure(12);
            [~,ax] = pm.PlotHistogramSpdSpd(tmp_ecmwf,tmp_ssmi,'nwp','sat','fignum',hf);
            clear ind tmp_ecmwf tmp_ssmi
            
            xlabel('ECMWF speed [m/s]');
            ylabel('SSMI speed [m/s]');
            title('Histogram: SSMI speed vs ECMWF speed')
            
            if saveintplots
                fname = sprintf('Hist_SSMIvsECMWFspeed_%s',outerinner,spdstr);
                print('-dtiff',fullfile(savedir,[fname '.tif']));
            end
        end
        
        
        
        
        
        
        
        %%
        %%%%%%%%%%%%%%%%%%%
        %%% Do the fits %%%
        %%%%%%%%%%%%%%%%%%%
        inputparams.spdbinwidth = spdbinwidth;
        inputparams.numcoefs = numcoefs;
        inputparams.plotbincenters = 2.5:5:360; % chi bin centers for plotting
        %         if bigcatfile
        %             inputparams.spdbincenters = spdbincenters(1:1:end);
        %         else
        %             inputparams.spdbincenters = thisloopspeed;
        %         end
        switch inputfileformat
            case 'catmat'
                inputparams.spdbincenters = spdbincenters(1:1:end);
            case {'unsortedmats','v0binspd'}
                inputparams.spdbincenters = thisloopspeed;
        end
        logfile.PrintScreenLog(sprintf('\nFitting with spdbinwidth of %.2f\n',spdbinwidth));
        logfile.PrintScreenLog(sprintf('Number of coefficients to fit: %d\n',numcoefs));
        
        
        %% Do the fit using all points and WindSat true speed and ECMWF direction
        %     ind = goodwsatspd;
        %     chi = data.chi(ind); % ecmwf relative azimuth
        %     spd = data.wsat_wspd_aw(ind);
        
        tspdfield = p.Results.true_spd_field;
        ind = (~isnan(data.(tspdfield))) & (data.(tspdfield) < 250);
        % the directions should have already been filtered by this point, no
        % need to filter it again here
        chi = data.(p.Results.true_dir_field)(ind);
        spd = data.(p.Results.true_spd_field)(ind);
        
        logfile.PrintScreenLog(sprintf('Fitting with true direction = %s\n',p.Results.true_dir_field));
        logfile.PrintScreenLog(sprintf('Fitting with speed = %s\n',p.Results.true_spd_field));
        logfile.PrintScreenLog(sprintf('Number of points to be used in fit = %d\n',length(spd)));
        
        if strcmp(p.Results.true_spd_field, 'ncep_speed') ...
                | strcmp(p.Results.true_spd_field,'ncep_spd')
            spd = spd*.84;
            logfile.PrintScreenLog('Multiplied ncep_speed by 0.84\n\n');
        end
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % THIS CURRENTLY WILL BEHAVE STRANGELY FOR bigcatfile~=1
        % determine which inputAcoeffs to pass in
        switch outerinner
            case 'Outer'
                inputAcoeffs = p.Results.inputAcoeffs_outer;
            case 'Inner'
                inputAcoeffs = p.Results.inputAcoeffs_inner;
        end
        % THIS CURRENTLY WILL BEHAVE STRANGELY FOR bigcatfile~=1
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        if isfield(data,'sig0lin')
            sig0 = data.sig0lin(ind);
        else if isfield(data,'sigma0_attn_corr_lin')
            sig0 = data.sigma0_attn_corr_lin(ind);
            end
        end
        if isfield(data,'incang')
            incang = data.incang(ind);
        else if isfield(data,'cell_incidence')
                incang = data.cell_incidence(ind);
            else 
                incang = zeros(size(data.cell_lat));
            end
        end
        
        [ts,chi,s0,keepind] = FitData_DoFit(spd,chi,sig0,incang,inputparams, ...
            'makefitplots',makefitplots, ...
            'whichbeamstr',outerinner, ...
            'makefitplots',makefitplots, ...
            'savefitplots',savefitplots,'saveplotdir',savedir, ...
            'saveroot','WSatSpdTruth_', ...
            'namestr',p.Results.namestr, ...
            'logfile',logfile, ...
            'refitmethod',p.Results.refitmethod, ...
            'threshold',p.Results.threshold, ...
            'stopcondition',p.Results.stopcondition, ...
            'threshold_width',p.Results.threshold_width, ...
            'trim_val',p.Results.trim_val, ...
            'maxiter',p.Results.maxiter, ...
            'foldtopi',p.Results.foldtopi, ...
            'inputAcoeffs',inputAcoeffs);
        
        if q_loadandfit == 1
            tsfull = ts;
        else
            tsfields = fieldnames(tsfull);
            for q_tsfield = 1:length(tsfields);
                tsfull.(tsfields{q_tsfield}) = cat(1, ...
                    tsfull.(tsfields{q_tsfield}), ...
                    ts.(tsfields{q_tsfield}));
            end
        end
        
        if p.Results.savegmf
            % rename tsfull to ts for saving, for consistency
            ts = tsfull;
            savefilename = fullfile(savedir,sprintf('Fits_%s.mat',outerinner,spdstr));
            logfile.PrintScreenLog(sprintf('Saving ''ts'' to %s\n',savefilename));
            save(savefilename,'ts')
        end
    end % end loop for loadandfit
    
    
    % construct the output variable, fits
    fits.(outerinner) = tsfull;
end %for q_bigloop = 1:num_loop% big loop should run at most twice, once for outer and once for inner

% construct the output variable, fits
fits.namestr = p.Results.namestr;
fits.numcoefs = numcoefs;
fits.datadir = datadir;
fits.savedir = savedir;

if p.Results.savegmf     % save the overall fits variable
    savefilename = fullfile(savedir,sprintf('Fits'));
    logfile.PrintScreenLog(sprintf('Saving ''Fits'' to %s',savefilename));
    save(savefilename,'fits')
end


%%%%%%%%%%%%%%%%
%%% Clean up %%%
%%%%%%%%%%%%%%%%
logfile.PrintScreenLog('\n********************************\n');
logfile.PrintScreenLog('***** Finished fitting GMF *****\n');
logfile.PrintScreenLog('********************************\n');
% close the log file
logfile.delete;

end % function FitGMFCoeffs(varargin)



function [ts,chi,s0,keepind] = FitData_DoFit(allspd,allchi,allsig0,allincang,params,varargin)
% Copied from GMF/FitData_DoFit.m on 1/17/12

p = inputParser;
p.addParamValue('makefitplots',1,@(x)isnumeric(x));
p.addParamValue('whichbeamstr',[],@(x)ischar(x));
p.addParamValue('savefitplots',0,@(x)isnumeric(x));
p.addParamValue('saveplotdir',[],@(x)ischar(x));
p.addParamValue('saveroot',[],@(x)ischar(x));
p.addParamValue('namestr',[]);
p.addParamValue('logfile',[]);
p.addParamValue('refitmethod',[]);
p.addParamValue('threshold',[]);
p.addParamValue('stopcondition',0.01);
p.addParamValue('threshold_width',[]);
p.addParamValue('trim_val',[]);
p.addParamValue('maxiter',[]);
p.addParamValue('foldtopi',0);
p.addParamValue('inputAcoeffs',[],@isnumeric);
p.parse(varargin{:});
logfile = p.Results.logfile;

inputAcoeffs = p.Results.inputAcoeffs;
% get parameters from params
spdbinwidth = params.spdbinwidth;
spdbincenters = params.spdbincenters;
plotbincenters = params.plotbincenters;

if isempty(inputAcoeffs)
    numcoefs = params.numcoefs;
    fitthiscoef = ones(1,numcoefs);
else
    numcoefs = size(inputAcoeffs,2);
    % figure out which coefficients to fit
    fitthiscoef = ~logical(sum(inputAcoeffs,1)); % 1 if want fit, 0 if no fit
end

% set some more parameters
bincenters = plotbincenters;
chi_reg = 0:1:360; chi_reg=chi_reg(:); % this is for plotting only

%% Set up the output structure: ts "temporary structure"
tmpnan = nan(length(spdbincenters),1);
% ts "temporary structure"
ts.meanincang = tmpnan;

ts.spd = spdbincenters(:);
ts.sig_mean = tmpnan;
ts.sig_sum = tmpnan;
ts.sig_sumsquare = tmpnan;
ts.sig_numpoints = tmpnan;
ts.sig_median = tmpnan;
ts.sig_std = tmpnan;

ts.resid_mean = tmpnan;
ts.resid_sum = tmpnan;
ts.resid_sumsquare = tmpnan;
ts.resid_numpoints = tmpnan;
ts.resid_median = tmpnan;
ts.resid_std = tmpnan;
ts.resid_rms = tmpnan;

ts.Acoeffs = nan(length(spdbincenters),numcoefs);
if any(~fitthiscoef)
    ts.Acoeffs(:,~fitthiscoef) = inputAcoeffs(:,~fitthiscoef);
    logfile.PrintScreenLog(sprintf('Will NOT fit these coefficients:\t%s\n', ...
        sprintf('A%d\t',find(~fitthiscoef)-1)));
    logfile.PrintScreenLog(sprintf('Will fit these coefficients:\t%s\n', ...
        sprintf('A%d\t',find(fitthiscoef)-1)));
end




%% Loop over speed bins
for q_spd = 1:length(spdbincenters)
    thisspd = spdbincenters(q_spd);
    %     fprintf('Calculating for %.2f m/s bin\n',thisspd);
    logfile.PrintScreenLog(sprintf('Calculating for %.2f m/s bin\n',thisspd));

    % get indices for points in this bin
    ind = abs(allspd-thisspd) < (spdbinwidth/2);
  
    % grab the data into shorter-named temporary variabls for convenience
    chi = allchi(ind);
    s0 = allsig0(ind);
   
    % if requested, fold chis to 180 to 360 inteveral
    if p.Results.foldtopi
        tmpind = chi<180;
        chi(tmpind) = 360-chi(tmpind);
        clear tmpind
    end
    
    % Do the simple least squares fit
    Amat = [cosd(0*chi) cosd(chi) cosd(2*chi) cosd(3*chi) cosd(4*chi) cosd(5*chi)];
    Amat = Amat(:,1:numcoefs);
    
    if any(~fitthiscoef)
        % grab the existing coefficients
        x_fixed = zeros(size(ts.Acoeffs(q_spd,:)));
        x_fixed = x_fixed(:);
        x_fixed(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
        x = x_fixed;
        x(fitthiscoef) = Amat(:,fitthiscoef)\ (s0-Amat*x_fixed)
    else
        x = Amat\s0;
    end
    b = Amat*x;
    resid = s0-b;
    
    % % %     %%%
    % % %     % refit the coefficients
    % % %     if any(~fitthiscoef)
    % % %         xnew = zeros(size(x));
    % % %         xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
    % % %         xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
    % % %     else
    % % %         xnew = Amat(keepind,:)\s0(keepind);
    % % %     end
    % % %     %%%
    
    % this is for plotting, do it now so it can be used in refit routines
    if p.Results.makefitplots
        Amat_reg = [cosd(0*chi_reg) cosd(chi_reg) cosd(2*chi_reg) ...
            cosd(3*chi_reg) cosd(4*chi_reg) cosd(5*chi_reg)];
        Amat_reg = Amat_reg(:,1:numcoefs);
        b_reg = Amat_reg*x;
    end
    
    keepind = logical(ones(size(s0)));
    xnew = x; % set this now for convenience, will be changed by refits
    bnew = b;
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%% IF ASKED TO REFIT, DO IT HERE %%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    if ~isempty(p.Results.refitmethod)
        refitmethod = p.Results.refitmethod;
        
        if isempty(p.Results.maxiter)
            maxiter = 20;
        else
            maxiter = p.Results.maxiter;
        end
        
        logfile.PrintScreenLog(sprintf('   \t initial coeffs = %s\n', ...
            sprintf('%.6f\t',x )));
        
        switch refitmethod
            case 'dBthreshold'
                if isempty(p.Results.threshold)
                    error('Must set a dB threshold')
                else
                    dBthreshold = p.Results.threshold;
                    if isempty(p.Results.stopcondition)
                        stopcondition = 0.01;
                    else
                        stopcondition = p.Results.stopcondition;
                    end
                end
                
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with dBthreshold of %.2f\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        dBthreshold,stopcondition))
                end
                           
                if any(keepind) % if there is no data, don't do loop
                    % initialize plot if needed
                    if p.Results.makefitplots
                        cmap = winter(5);
                        figure(1002); clf
                        hp = plot(chi,10*log10(s0),'k.', ...
                            chi_reg,10*log10(b_reg),'w', ...
                            chi_reg,10*log10(b_reg),'k');
                        set(hp(2),'linewidth',2);
                        hold on; xlim([0 360])
                         
                        b_regall = b_reg;
                    end
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter=0;
                    while ~stoploop
                        iter = iter+1;
                        % determine which values to keep
                        keepind = ((s0./bnew) < 10^(dBthreshold/10)) ...
                            & ((s0./bnew) > 10^(-dBthreshold/10));
                        % refit the coefficients
                        if any(~fitthiscoef)
                            xnew = zeros(size(x));
                            xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                            xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                        else
                            xnew = Amat(keepind,:)\s0(keepind);
                        end
                        
                        xall = [xall xnew];
                        x = xnew; 
                        bnew = Amat*xnew;
                        % plot if requested
                        if p.Results.makefitplots
                            b_regnew = Amat_reg*xnew;
                            b_regall = [b_regall b_regnew];
                            b_reg = b_regnew;
                            cmaprow = mod(iter,size(cmap,1));
                            if cmaprow ==0; cmaprow = size(cmap,1); end
                            figure(1002)
                            plot(chi(keepind),10*log10(s0(keepind)),'o','color',cmap(cmaprow,:));
                            plot(chi_reg,10*log10(b_regnew),'w','linewidth',2);
                            hp = plot(chi_reg,10*log10(b_regnew), ...
                                chi_reg,10*log10(b_regnew)+dBthreshold,'--', ...
                                chi_reg,10*log10(b_regnew)-dBthreshold,'--');
                            set(hp,'color',cmap(cmaprow,:))
                        end
                        % check whether you meet the stop condition
                        pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                        logfile.PrintScreenLog(sprintf('   iter = %d,\tfracdiff = %s\n', ...
                            iter,sprintf('%.6f\t',pdiff)));
                        stoploop = all(abs(pdiff) < stopcondition);
                        if iter == maxiter
                            stoploop = 1;
                        end
                    end
                end % if length(keepind) > 0
                % end of case 'dBthreshold'
                
            case 'kpthreshold'
                if isempty(p.Results.threshold)
                    error('Must set a kp value (threshold)')
                else
                    kp_value = p.Results.threshold;
                    if isempty(p.Results.stopcondition)
                        stopcondition = 0.01;
                    else
                        stopcondition = p.Results.stopcondition;
                    end
                end
                
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with kpthreshold of %.2f,\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        kp_value,stopcondition))
                end
                
                if any(keepind) % if there is no data, don't do loop
                    % initialize plot if needed
                    if p.Results.makefitplots
                        cmap = winter(5);
                        figure(1002); clf
                        sigforplot = b_reg*kp_value;
%                         sigforplot = b*kp_value;
                        hp = plot(chi,(s0),'k.', ...
                            chi_reg,(b_reg),'w', ...
                            chi_reg,(b_reg),'k', ...
                            chi_reg,(b_reg+3*sigforplot),'k--', ...
                            chi_reg,(b_reg-3*sigforplot),'k--');
                        %                         chi,10*log10(b+3*sigforplot),'ro', ...
                        %                             chi,10*log10(b-3*sigforplot),'ro');
                        set(hp(2),'linewidth',2);
                        hold on;                          
                        xlim([0 360]); grid on
                        
                        b_regall = b_reg;
                    end
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter=0;
                    while ~stoploop
                        iter = iter+1;
                        % determine which values to keep
                        
                        % calculate what sigstd is 
                        sigstd = bnew*kp_value;
                        keepind = abs(s0-bnew) < sigstd*3;
                        
                        % refit the coefficients
                        if any(~fitthiscoef)
                            xnew = zeros(size(x));
                            xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                            xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                        else
                            xnew = Amat(keepind,:)\s0(keepind);
                        end
                        
                        xall = [xall xnew];
                        x = xnew; 
                        bnew = Amat*xnew;
                        % plot if requested
                        if p.Results.makefitplots
                            b_regnew = Amat_reg*xnew;
                            b_regall = [b_regall b_regnew];
                            b_reg = b_regnew;
                            sigforplot = b_regnew*kp_value;
                            cmaprow = mod(iter,size(cmap,1));
                            if cmaprow ==0; cmaprow = size(cmap,1); end
                            figure(1002)
                            plot(chi(keepind),(s0(keepind)),'o','color',cmap(cmaprow,:));
                            plot(chi_reg,(b_regnew),'w','linewidth',2);
                            hp = plot(chi_reg,(b_regnew), ...
                                chi_reg,(b_regnew+3*sigforplot),'--', ...
                                chi_reg,(b_regnew-3*sigforplot),'--');
                            set(hp,'color',cmap(cmaprow,:))
                        end
                        % check whether you meet the stop condition
                        pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                        logfile.PrintScreenLog(sprintf('   iter = %d,\tpdiff = %s\n', ...
                            iter,sprintf('%.6f\t',pdiff)));
                        stoploop = all(abs(pdiff) < stopcondition);
                        if iter == maxiter
                            stoploop = 1;
                        end
                    end
                end % if length(keepind) > 0
                % end of case 'kpthreshold'
            case 'sigmafromdata_bybin'
                % make sure the std dev threshold is set
                if isempty(p.Results.threshold)
                    error('Must set the std threshold (i.e. throw out threshold*std)(threshold)')
                else
                    sig_thresh = p.Results.threshold;
                end
                % make sure the thresholdazimuth bin width was set
                if isempty(p.Results.threshold_width)
                    error('Must set the width of the threshold bins (threshold_width)')
                else
                    chibinwidth_forsig = p.Results.threshold_width;
                    bincenters_forsig = (chibinwidth_forsig/2):chibinwidth_forsig:360;
                end
                
                % check if the stop condition was set
                if isempty(p.Results.stopcondition)
                    stopcondition = 0.01;
                else
                    stopcondition = p.Results.stopcondition;
                end
                
                
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with sigmafromdata_bybin,\n' ...
                        '\twill throw out %.2f*(estimated sigma)\n' ...
                        '\twhere sigma is estimated in bins of width %d degrees,\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        sig_thresh,chibinwidth_forsig,stopcondition))
                end
                
                
                if any(keepind) % if there is no data, don't do loop
                    % need to bin the sigma0s into chi bins
                    chiclipped = chi; %s0tmp = s0(keepind);
                    chiclipped(chiclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                    chiclipped(chiclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                    % get index
                    chiind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chiclipped,'nearest');
                    
                    % get mean and std
                    s0chiindmean_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)mean(x));
                    s0chiindstd_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)std(x));
                    % get count 
                    count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                    s0_sig = s0chiindstd_forsig(chiind_forsig);
                    s0_mean = s0chiindmean_forsig(chiind_forsig);
                    
                    % initialize plot if needed
                    if p.Results.makefitplots
                        cmap = winter(5);
                        figure(1002); clf
                        plot(chi,s0,'k.', ...
                            chi_reg,(b_reg),'w', ...
                            chi_reg,(b_reg),'k')
                        hold on
                        xx = [bincenters_forsig - chibinwidth_forsig/2; ...
                            bincenters_forsig + chibinwidth_forsig/2];
                        % plot means
                        yy = [s0chiindmean_forsig'; s0chiindmean_forsig'];
                        plot(xx,yy,'w','linewidth',2)
                        plot(xx,yy,'k')
                        % plot sig_thresh*sigma
                        yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                            + sig_thresh*[s0chiindstd_forsig'; s0chiindstd_forsig'];
                        plot(xx,yy,'w','linewidth',2)
                        plot(xx,yy,'k')
                        % plot sig_thresh*sigma
                        yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                            - sig_thresh*[s0chiindstd_forsig'; s0chiindstd_forsig'];
                        plot(xx,yy,'w','linewidth',2)
                        plot(xx,yy,'k')
                        % add left vertical lines
                        xx = [bincenters_forsig - chibinwidth_forsig/2; ...
                            bincenters_forsig - chibinwidth_forsig/2];
                        yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                            + sig_thresh*[s0chiindstd_forsig'; -s0chiindstd_forsig'];
                        plot(xx,yy,'w','linewidth',2)
                        plot(xx,yy,'k')
                        % add right vertical lines
                        xx = [bincenters_forsig + chibinwidth_forsig/2; ...
                            bincenters_forsig + chibinwidth_forsig/2];
                        yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                            + sig_thresh*[s0chiindstd_forsig'; -s0chiindstd_forsig'];
                        plot(xx,yy,'w','linewidth',2)
                        plot(xx,yy,'k')
                        
                        xlim([0 360]); grid on
                        
                        b_regall = b_reg;
                    end
                
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter = 0;
                    while ~stoploop % the fitting loop
                        iter = iter+1;
                        % determine which values to keep
                        keepind = abs(s0-s0_mean) < sig_thresh*s0_sig;
                        
                        % refit the coefficients
                        if any(~fitthiscoef)
                            xnew = zeros(size(x));
                            xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                            xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                        else
                            xnew = Amat(keepind,:)\s0(keepind);
                        end
                        
                            
                        xall = [xall xnew];
                        x = xnew;
                        bnew = Amat*xnew;
                        
                        % recalculate the means and stds and count
                        % get mean and std
                        s0chiindmean_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)mean(x));
                        s0chiindstd_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)std(x));
                        % get count
                        count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                        s0_sig = s0chiindstd_forsig(chiind_forsig);
                        s0_mean = s0chiindmean_forsig(chiind_forsig);

                        % plot if requested
                        if p.Results.makefitplots
                            b_regnew = Amat_reg*xnew;
                            b_regall = [b_regall b_regnew];
                            b_reg = b_regnew;
                            
                            cmaprow = mod(iter,size(cmap,1));
                            if cmaprow ==0; cmaprow = size(cmap,1); end
                            figure(1002)
                            plot(chi(keepind),(s0(keepind)),'o','color',cmap(cmaprow,:));
                            plot(chi_reg,(b_regnew),'w','linewidth',2);
                            plot(chi_reg,(b_regnew),'color',cmap(cmaprow,:));
                            
                            xx = [bincenters_forsig - chibinwidth_forsig/2; ...
                                bincenters_forsig + chibinwidth_forsig/2];
                            % plot means
                            yy = [s0chiindmean_forsig'; s0chiindmean_forsig'];
                            plot(xx,yy,'w','linewidth',2)
                            plot(xx,yy,'color',cmap(cmaprow,:))
                            % plot sig_thresh*sigma
                            yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                                + sig_thresh*[s0chiindstd_forsig'; s0chiindstd_forsig'];
                            plot(xx,yy,'w','linewidth',2)
                            plot(xx,yy,'color',cmap(cmaprow,:))
                            % plot sig_thresh*sigma
                            yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                                - sig_thresh*[s0chiindstd_forsig'; s0chiindstd_forsig'];
                            plot(xx,yy,'w','linewidth',2)
                            plot(xx,yy,'color',cmap(cmaprow,:))
                            % add left vertical lines
                            xx = [bincenters_forsig - chibinwidth_forsig/2; ...
                                bincenters_forsig - chibinwidth_forsig/2];
                            yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                                + sig_thresh*[s0chiindstd_forsig'; -s0chiindstd_forsig'];
                            plot(xx,yy,'w','linewidth',2)
                            plot(xx,yy,'color',cmap(cmaprow,:))
                            % add right vertical lines
                            xx = [bincenters_forsig + chibinwidth_forsig/2; ...
                                bincenters_forsig + chibinwidth_forsig/2];
                            yy = [s0chiindmean_forsig'; s0chiindmean_forsig'] ...
                                + sig_thresh*[s0chiindstd_forsig'; -s0chiindstd_forsig'];
                            plot(xx,yy,'w','linewidth',2)
                            plot(xx,yy,'color',cmap(cmaprow,:))
                        end
                        % check whether you meet the stop condition
                        pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                        logfile.PrintScreenLog(sprintf('   iter = %d,\tpdiff = %s\n', ...
                            iter,sprintf('%.6f\t',pdiff)));
                        stoploop = all(abs(pdiff) < stopcondition);
                        if iter == maxiter
                            stoploop = 1;
                        end
                    end %while ~stoploop % the fitting loop
                end %if length(keepind) > 0
                % end of case 'sigmafromdata_bybin'
                
            case 'sigmafromdata_bymodel'
                if isempty(p.Results.threshold)
                    error('Must set the width of azimuth bin (threshold)')
                else
                    sig_thresh = p.Results.threshold;
                end
                % make sure the thresholdazimuth bin width was set
                if isempty(p.Results.threshold_width)
                    error('Must set the width of the threshold bins (threshold_width)')
                else
                    chibinwidth_forsig = p.Results.threshold_width;
                    bincenters_forsig = (chibinwidth_forsig/2):chibinwidth_forsig:360;
                end
                 % check if the stop condition was set                
                if isempty(p.Results.stopcondition)
                    stopcondition = 0.01;
                else
                    stopcondition = p.Results.stopcondition;
                end
                
                  
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with sigmafromdata_bymodel,\n' ...
                        '\twill throw out %.2f*(estimated sigma)\n' ...
                        '\twhere sigma is estimated in bins of width %d degrees,\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        sig_thresh,chibinwidth_forsig,stopcondition))
                end
                
                                
                if any(keepind) % if there is no data, don't do loop
                    
                    % need to bin the sigma0s into chi bins
                    chiclipped = chi; 
                    chiclipped(chiclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                    chiclipped(chiclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                    % get index
                    chiind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chiclipped,'nearest');
                    
                    % get mean and std
                    % mean will be the most recently fit model value
                    s0_mean = b;
                    s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                        s0(keepind)-s0_mean(keepind),[length(bincenters_forsig) 1], ...
                        @(x)std(x));
                    % get count 
                    % count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                    s0_sig = s0chiindstd_forsig(chiind_forsig);
                    
                    % initialize plot if needed
                    if p.Results.makefitplots
                        chi_regclipped = chi_reg;
                        chi_regclipped(chi_regclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                        chi_regclipped(chi_regclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                        % get index
                        chi_regind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chi_regclipped,'nearest');
                        % get sigma for each bin
                        chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);
                        
                        cmap = winter(5);
                        figure(1002); clf
                        plot(chi,s0,'k.', ...
                            chi_reg,(b_reg),'w', ...
                            chi_reg,(b_reg),'k', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--')
                        hold on

                        xlim([0 360]); grid on
                        
                        b_regall = b_reg;
                    end
                
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter = 0;
                    while ~stoploop % the fitting loop
                        iter = iter+1;
                        % determine which values to keep
                        keepind = abs(s0-s0_mean) < sig_thresh*s0_sig;
                        
                        % refit the coefficients
                        if any(~fitthiscoef)
                            xnew = zeros(size(x));
                            xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                            xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                        else
                            xnew = Amat(keepind,:)\s0(keepind);
                        end
                        
                        xall = [xall xnew];
                        x = xnew;
                        bnew = Amat*xnew;
                        
                        % recalculate the means and stds and count
                        % get mean and std
                        %%%% s0chiindmean_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)mean(x));
                        %%%% s0_mean = s0chiindmean_forsig(chiind_forsig);
                        s0_mean = bnew;
                        s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                            s0(keepind)-s0_mean(keepind),[length(bincenters_forsig) 1],@(x)std(x));
                        % get count
                        %count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                        s0_sig = s0chiindstd_forsig(chiind_forsig);
                        
                        % plot if requested
                        if p.Results.makefitplots
                            b_regnew = Amat_reg*xnew;
                            b_regall = [b_regall b_regnew];
                            b_reg = b_regnew;
                            
                            chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);

                            cmaprow = mod(iter,size(cmap,1));
                            if cmaprow ==0; cmaprow = size(cmap,1); end
                            figure(1002)
                            plot(chi(keepind),(s0(keepind)),'o','color',cmap(cmaprow,:));
                            plot(chi_reg,(b_regnew),'w','linewidth',2);                            
                            plot(chi_reg,(b_regnew), ...
                                chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                                chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--', ...
                                'color',cmap(cmaprow,:));
                        end
                        % check whether you meet the stop condition
                        pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                        logfile.PrintScreenLog(sprintf('   iter = %d,\tpdiff = %s\n', ...
                            iter,sprintf('%.6f\t',pdiff)));
                        stoploop = all(abs(pdiff) < stopcondition);
                        if iter == maxiter
                            stoploop = 1;
                        end
                    end %while ~stoploop % the fitting loop
                end %if length(keepind) > 0
                % end of case 'sigmafromdata_bymodel'
                
            case 'sigmafromdatatrimmed_bymodel'
                % make sure the std dev threshold is set
                if isempty(p.Results.threshold)
                    error('Must set the std threshold (i.e. throw out threshold*std)(threshold)')
                else
                    sig_thresh = p.Results.threshold;
                end
                % make sure the trim value was set
                if isempty(p.Results.trim_val)
                    error('Must set trim value')
                else
                    trim_val = p.Results.trim_val;
                end
                % make sure the thresholdazimuth bin width was set
                if isempty(p.Results.threshold_width)
                    error('Must set the width of the threshold bins (threshold_width)')
                else
                    chibinwidth_forsig = p.Results.threshold_width;
                    bincenters_forsig = (chibinwidth_forsig/2):chibinwidth_forsig:360;
                end
                % get the stop condition
                stopcondition = p.Results.stopcondition;
                
                
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with sigmafromdatatrimmed_bymodel,\n' ...
                        '\twill throw out %.2f*(estimated sigma)\n' ...
                        '\twhere sigma is estimated in bins of width %d degrees,\n' ...
                        '\twhile trimming %d%%%% of the data (total top and bottom) in each bin\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        sig_thresh,chibinwidth_forsig,trim_val,stopcondition))
                end
                
                
                if any(keepind) % if there is no data, don't do loop
                    
                    % need to bin the sigma0s into chi bins
                    chiclipped = chi; 
                    chiclipped(chiclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                    chiclipped(chiclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                    % get index
                    chiind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chiclipped,'nearest');
                    
                    % get mean and std
                    % mean will be the most recently fit model value
                    s0_mean = b;
                    s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                        s0(keepind)-s0_mean(keepind),[length(bincenters_forsig) 1], ...
                        @(x)trimFunc(x,trim_val,@std));
                    % get count 
                    count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                    s0_sig = s0chiindstd_forsig(chiind_forsig);
                    
                    % initialize plot if needed
                    if p.Results.makefitplots
                        chi_regclipped = chi_reg;
                        chi_regclipped(chi_regclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                        chi_regclipped(chi_regclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                        % get index
                        chi_regind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chi_regclipped,'nearest');
                        % get sigma for each bin
                        chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);
                        
                        cmap = winter(3);
                        figure(1002); clf
                        plot(chi,s0,'k.', ...
                            chi_reg,(b_reg),'w', ...
                            chi_reg,(b_reg),'k', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--')
                        hold on

                        xlim([0 360]); grid on
                        
                        b_regall = b_reg;
                    end
                
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter = 0;
                    while ~stoploop % the fitting loop
                        iter = iter+1;
                        % determine which values to keep
                        keepind = abs(s0-s0_mean) < sig_thresh*s0_sig;
                        
                        if any(keepind)
                            % refit the coefficients
                            if any(~fitthiscoef)
                                xnew = zeros(size(x));
                                xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                                xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                            else
                                xnew = Amat(keepind,:)\s0(keepind);
                            end
                        
                            xall = [xall xnew];
                            x = xnew;
                            bnew = Amat*xnew;
                            
                            % recalculate the means and stds and count
                            % get mean and std
                            %%%%% s0chiindmean_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)mean(x));
                            %%%%%  s0_mean = s0chiindmean_forsig(chiind_forsig);
                            s0_mean = bnew;
                            s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                                s0(keepind),[length(bincenters_forsig) 1], ...
                                @(x)trimFunc(x,trim_val,@std));
                            % get count
                            count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                            s0_sig = s0chiindstd_forsig(chiind_forsig);
                            
                            % plot if requested
                            if p.Results.makefitplots
                                b_regnew = Amat_reg*xnew;
                                b_regall = [b_regall b_regnew];
                                b_reg = b_regnew;
                                
                                chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);
                                
                                cmaprow = mod(iter,size(cmap,1));
                                if cmaprow ==0; cmaprow = size(cmap,1); end
                                figure(1002)
                                plot(chi(keepind),(s0(keepind)),'o','color',cmap(cmaprow,:));
                                plot(chi_reg,(b_regnew),'w','linewidth',2);
                                plot(chi_reg,(b_regnew), ...
                                    chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                                    chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--', ...
                                    'color',cmap(cmaprow,:));
                            end
                            % check whether you meet the stop condition
                            pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                            logfile.PrintScreenLog(sprintf('   iter = %d,\tpdiff = %s\n', ...
                                iter,sprintf('%.6f\t',pdiff)));
                            stoploop = all(abs(pdiff) < stopcondition);
                            if iter == maxiter
                                stoploop = 1;
                            end
                        else
                            stoploop = 1;
                            xnew = nan(size(xnew));
                        end
                    end %while ~stoploop % the fitting loop
                  
                end %if length(keepind) > 0
                % end of case 'sigmafromdatatrimmed_bymodel'
                
                
            case 'sigmafromdatatrimmed_bymodel_trimmedfit'
                % make sure the std dev threshold is set
                if isempty(p.Results.threshold)
                    error('Must set the std threshold (i.e. throw out threshold*std)(threshold)')
                else
                    sig_thresh = p.Results.threshold;
                end
                % make sure the trim value was set
                if isempty(p.Results.trim_val)
                    error('Must set trim value')
                else
                    trim_val = p.Results.trim_val;
                end
                % make sure the thresholdazimuth bin width was set
                if isempty(p.Results.threshold_width)
                    error('Must set the width of the threshold bins (threshold_width)')
                else
                    chibinwidth_forsig = p.Results.threshold_width;
                    bincenters_forsig = (chibinwidth_forsig/2):chibinwidth_forsig:360;
                end
                % get the stop condition
                stopcondition = p.Results.stopcondition;
                
                if q_spd ==1
                    logfile.PrintScreenLog(sprintf(['Refitting with sigmafromdatatrimmed_bymodel_trimmedfit,\n' ...
                        '\twill throw out %.2f*(estimated sigma)\n' ...
                        '\twhere sigma is estimated in bins of width %d degrees,\n' ...
                        '\twhile trimming %d%%%% of the data (total top and bottom) in each bin\n' ...
                        '\tuntil An coefficients all change by less than %.4f\n'], ...
                        sig_thresh,chibinwidth_forsig,trim_val,stopcondition))
                end
                
                if any(keepind) % if there is no data, don't do loop
                                   
                    %%% REDO THE FIT HERE, USING THE TRIMMED DATA
                    % make some temporary bins
                    tmpbincenters = 5:10:360;
                    % temporarily bin the data into these bins 
                    chiclipped = chi;
                    chiclipped(chiclipped<tmpbincenters(1)) = tmpbincenters(1);
                    chiclipped(chiclipped>tmpbincenters(end)) = tmpbincenters(end);
                    % get index
                    chiind_forsig = interp1(tmpbincenters,1:length(tmpbincenters),chiclipped,'nearest');
                    % get mean and std for each bin
                    s0chiindmean_forsig = accumarray(chiind_forsig(keepind), ...
                        s0(keepind),[length(tmpbincenters) 1], ...
                        @(x)trimFunc(x,trim_val,@mean));
                    s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                        s0(keepind),[length(tmpbincenters) 1], ...
                        @(x)trimFunc(x,trim_val,@std));
                    s0_mean = s0chiindmean_forsig(chiind_forsig);
                    s0_sig = s0chiindstd_forsig(chiind_forsig);
                    % decide which ones to keep
                    tmpkeepind = abs(s0-s0_mean) < sig_thresh*s0_sig;
                    x = Amat(tmpkeepind,:)\s0(tmpkeepind);
                    b = Amat*x;
                    resid = s0-b;
                    if p.Results.makefitplots
                        b_reg = Amat_reg*x;
                    end
                    clear chiclipped chiind_forsig s0chiindmean_forsig s0chiindstd_forsig s0_mean s0_sig
                    %%% REDO THE FIT HERE, USING THE TRIMMED DATA
                    
                    % need to bin the sigma0s into chi bins
                    chiclipped = chi; 
                    chiclipped(chiclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                    chiclipped(chiclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                    % get index
                    chiind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chiclipped,'nearest');
                    
                    % get mean and std
                    % mean will be the most recently fit model value
                    s0_mean = b;
                    s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                        s0(keepind)-s0_mean(keepind),[length(bincenters_forsig) 1], ...
                        @(x)trimFunc(x,trim_val,@std));
                    % get count 
                    count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                    s0_sig = s0chiindstd_forsig(chiind_forsig);
                    
                    
                    % initialize plot if needed
                    if p.Results.makefitplots
                        chi_regclipped = chi_reg;
                        chi_regclipped(chi_regclipped<bincenters_forsig(1)) = bincenters_forsig(1); % clip low
                        chi_regclipped(chi_regclipped>bincenters_forsig(end)) = bincenters_forsig(end); % clip high
                        % get index
                        chi_regind_forsig = interp1(bincenters_forsig,1:length(bincenters_forsig),chi_regclipped,'nearest');
                        % get sigma for each bin
                        chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);
                        
                        cmap = winter(3);
                        figure(1002); clf
                        plot(chi,s0,'r.', ...
                            chi(tmpkeepind),s0(tmpkeepind),'k.', ...
                            chi_reg,(b_reg),'w', ...
                            chi_reg,(b_reg),'k', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'w', ...
                            chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--')
                        hold on

                        xlim([0 360]); grid on
                        
                        b_regall = b_reg;
                    end
                
                    % initialize variables before going into loop
                    xall = x;
                    bnew = b;
                    stoploop = 0; iter = 0;
                    while ~stoploop % the fitting loop
                        iter = iter+1;
                        % determine which values to keep
                        keepind = abs(s0-s0_mean) < sig_thresh*s0_sig;
                        
                        if any(keepind)
                            % refit the coefficients
                            if any(~fitthiscoef)
                                xnew = zeros(size(x));
                                xnew(~fitthiscoef) = ts.Acoeffs(q_spd,~fitthiscoef);
                                xnew(fitthiscoef) = Amat(keepind,fitthiscoef)\(s0(keepind)-Amat(keepind,:)*x_fixed);
                            else
                                xnew = Amat(keepind,:)\s0(keepind);
                            end
                            
                            xall = [xall xnew];
                            x = xnew;
                            bnew = Amat*xnew;
                            
                            % recalculate the means and stds and count
                            % get mean and std
                            %%%%% s0chiindmean_forsig = accumarray(chiind_forsig(keepind),s0(keepind),[length(bincenters_forsig) 1],@(x)mean(x));
                            %%%%%  s0_mean = s0chiindmean_forsig(chiind_forsig);
                            s0_mean = bnew;
                            s0chiindstd_forsig = accumarray(chiind_forsig(keepind), ...
                                s0(keepind),[length(bincenters_forsig) 1], ...
                                @(x)trimFunc(x,trim_val,@std));
                            % get count
                            count_forsig = hist(chiclipped(keepind),bincenters_forsig);
                            s0_sig = s0chiindstd_forsig(chiind_forsig);
                            
                            % plot if requested
                            if p.Results.makefitplots
                                b_regnew = Amat_reg*xnew;
                                b_regall = [b_regall b_regnew];
                                b_reg = b_regnew;
                                
                                chi_reg_sigma = s0chiindstd_forsig(chi_regind_forsig);
                                
                                cmaprow = mod(iter,size(cmap,1));
                                if cmaprow ==0; cmaprow = size(cmap,1); end
                                figure(1002)
                                plot(chi(keepind),(s0(keepind)),'o','color',cmap(cmaprow,:));
                                plot(chi_reg,(b_regnew),'w','linewidth',2);
                                plot(chi_reg,(b_regnew), ...
                                    chi_reg,(b_reg + sig_thresh*chi_reg_sigma),'k--', ...
                                    chi_reg,(b_reg - sig_thresh*chi_reg_sigma),'k--', ...
                                    'color',cmap(cmaprow,:));
                            end
                            % check whether you meet the stop condition
                            pdiff = (xall(:,end)-xall(:,end-1))./xall(:,end-1);
                            logfile.PrintScreenLog(sprintf('   iter = %d,\tpdiff = %s\n', ...
                                iter,sprintf('%.6f\t',pdiff)));
                            stoploop = all(abs(pdiff) < stopcondition);
                            if iter == maxiter
                                stoploop = 1;
                            end
                        else
                            stoploop = 1;
                            xnew = nan(size(xnew));
                        end
                    end %while ~stoploop % the fitting loop
                  
                end %if length(keepind) > 0
                % end of case 'sigmafromdatatrimmed_bymodel_trimmedfit'
        end
        
        logfile.PrintScreenLog(sprintf('   \t final coeffs = %s\n', ...
            sprintf('%.6f\t',xnew)));
        logfile.PrintScreenLog(sprintf('        Used %d/%d = %.2f%%%% of data points\n', ...
            length(find(keepind)), length(keepind), ...
            length(find(keepind))/length(keepind)*100 ))
        
        if p.Results.makefitplots
            titlestr = sprintf('\n Speed = %.2f (+/- %.2f) m/s\n %d points\nCoeffs:\t %s', ...
                thisspd,spdbinwidth/2, length(s0(keepind)), ...
                sprintf('%.8f\t',x));
            if any(~keepind)
                titlestr = strrep(titlestr,'points', ...
                    sprintf('points (discarded %d after %d iterations)', ...
                    length(find(~keepind)),iter) );
            end
            if ~isempty(p.Results.whichbeamstr)
                titlestr = sprintf('%s beam, %s',p.Results.whichbeamstr,titlestr);
            end
            if ~isempty(p.Results.namestr)
                titlestr = sprintf('%s, %s',p.Results.namestr,titlestr);
            end
            title(titlestr)
            xlabel('Relative azimuth angle [deg]');
            ylabel('Sigma0 (linear)')
            
            
            if p.Results.savefitplots
                hf = figure(1002);
                figsize = [9 7];
                set(hf,'papersize',figsize,'paperposition',[0 0 figsize]);
                %                 set(hf,'units','inches');
                %                 pos = get(hf,'position');
                %                 set(hf,'position',[pos(1:2) figsize])
                %                 set(hf,'units','pixels')
                
                saveplotdir = p.Results.saveplotdir;
                savename = sprintf('%sRefit_%s_beam_Speed%.1f_Bin%.1f', ...
                    p.Results.saveroot,p.Results.whichbeamstr, ...
                    thisspd,spdbinwidth);
                print('-dtiff',fullfile(saveplotdir,[savename '.tif']))
            end
        end
    end % if ~isempty(p.Results.refitmethod)
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%% FITTING IS TOTALLY DONE %%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % calculate some statistics to save
    tmp = allincang(ind); meanincang = mean(tmp(keepind));
    ts.meanincang(q_spd) = meanincang;
    % save stats about sigma0's used    
    ts.sig_mean(q_spd) = mean(s0(keepind));
    ts.sig_sum(q_spd) = sum(s0(keepind));
    ts.sig_sumsquare(q_spd) = sum(s0(keepind).^2);
    ts.sig_numpoints(q_spd) = length(s0(keepind));
    ts.sig_median(q_spd) = median(s0(keepind));
    ts.sig_std(q_spd) = std(s0(keepind));
    % save stats about residual
    resid = s0(keepind)-bnew(keepind);
    ts.resid_mean(q_spd) = mean(resid);
    ts.resid_sum(q_spd) = sum(resid);
    ts.resid_sumsquare(q_spd) = sum(resid.^2);
    ts.resid_numpoints(q_spd) = length(resid);
    ts.resid_median(q_spd) = median(resid);
    ts.resid_std(q_spd) = std(resid);
    ts.resid_rms(q_spd) = sqrt( mean(resid.^2) );
    % save the coefficients
    ts.Acoeffs(q_spd,:) = xnew'; % save coefficients into ts structure
    
    
    if p.Results.makefitplots
        %%
        hf = figure(1001);
        figsize = [9 7];
        set(hf,'papersize',figsize,'paperposition',[0 0 figsize]);
        set(hf,'units','inches');
        pos = get(hf,'position');
        set(hf,'position',[pos(1:2) figsize])
        set(hf,'units','pixels')
        
        chitmp = chi;
        chitmp(chitmp<bincenters(1)) = bincenters(1);
        chitmp(chitmp>bincenters(end)) = bincenters(end);
        chibincenter = interp1(bincenters,bincenters,chitmp,'nearest');
        
        % also get chiind
        chiind = interp1(bincenters,1:length(bincenters),chitmp,'nearest');
        s0chiindmean = accumarray(chiind,s0,[length(bincenters) 1],@(x)mean(x));
        if sum(keepind)>0
            s0chiindmeankeep = accumarray(chiind(keepind),s0(keepind),[length(bincenters) 1],@(x)mean(x));
        else
            s0chiindmeankeep = zeros(size(s0chiindmean));
        end
        % mean s0 for each bin
        
        axwidth = .85;
        axheight1 = .3;
        axheight2 = .2;
        axleft = .08;
        ax1bottom = .6;
        ax2bottom = 0.38;
        ax3bottom = 0.06;
        
        clf; ax = [];
        xtick = [0:15:360];
        
        % DATA AND FIT
        axes('position',[axleft ax1bottom axwidth axheight1])
        plot(chi, s0,'.')
        hold on
        plot(chi(~keepind), s0(~keepind),'x','color',[1 1 1]*.6);
        plot(chi_reg,b_reg,'c','linewidth',2)
        grid on
        set(gca,'xtick',xtick,'xticklabel',[])
        ax = cat(1,ax,gca);
        ylabel('Sigma0 (linear)')
        %     ylim([-0.01 .1])
        xlim(xtick([1 end]))
        
        % TITLE
        titlestr = sprintf('Incidence angle = %.2f deg\n Speed = %.2f (+/- %.2f) m/s\n %d points\nCoeffs:\t %s', ...
            meanincang,thisspd,spdbinwidth/2, length(s0(keepind)), ...
            sprintf('%.8f\t',x));
        if any(~keepind)
            titlestr = strrep(titlestr,'points', ...
                sprintf('points (discarded %d after %d iterations)', ...
                length(find(~keepind)),iter) );
        end
        if ~isempty(p.Results.whichbeamstr)
            titlestr = sprintf('%s beam, %s',p.Results.whichbeamstr,titlestr);
        end
        if ~isempty(p.Results.namestr)
            titlestr = sprintf('%s, %s',p.Results.namestr,titlestr);
        end
        title(titlestr)
        
        % PLOT THE HISTOGRAM OF THE MEASUREMENTS
        axes('position',[axleft ax2bottom axwidth axheight2]);
        count = hist(chi,bincenters);
        stem(bincenters,count/sum(count),'k');
        hold on
        count = hist(chi(keepind),bincenters);
        stem(bincenters,count/sum(count),'b');
        ylabel('Histogram of rel az angle (pmf)')
        set(gca,'xtick',xtick,'xticklabel',[])
        grid on
        ax = cat(1,ax,gca);
        xlim(xtick([1 end]))
        
        % PLOT THE BOX PLOT AND THE FIT
        axes('position',[axleft ax3bottom axwidth axheight1])
        plot(chi_reg,b_reg,'c','linewidth',2)
        hold on
        plot(bincenters,s0chiindmean,'kx'); % plot the means
        plot(bincenters,s0chiindmeankeep,'b.')
        %     h = boxplot(s0,chibincenter,'position',bincenters);%,'plotstyle','compact')
        if any(keepind)
            h = boxplot(s0(keepind),chibincenter(keepind),'position',unique(chibincenter(keepind)));
        end
        set(gca,'XTickLabel',{' '})
        hax = gca;
        set(hax,'xtick',xtick,'xticklabel',xtick);
        ylabel('Sigma0 (linear)')
        grid on
        set(gca,'position',[axleft ax3bottom axwidth axheight1])
        ax = cat(1,ax,gca);
        
        % set ylim
        %         yl = ylim; tmpyl = yl;
        %         yl(1) = max([yl(1) -1e-4]);
        %         yl(2) = min([yl(2) abs(x(1))*5]);
        %         if yl(2)<yl(1)
        %             yl(2) = tmpyl(2);
        %         end
        %         ylim(yl)
        
        linkaxes(ax,'x')
        xlim([0, 360])
        xlabel('Relative azimuth angle [deg]')
        
        pause(.001)
        
        
        %% Special plotting code, not to be used regularly!!!
        % AC put this in to make a little figure for quarterly review slide
        % 1/26/12
        if 0
            hf = figure(1003);clf
            figsize = [4.5 1.75];
            set(hf,'papersize',figsize,'paperposition',[0 0 figsize]);
            set(hf,'units','inches');
            pos = get(hf,'position');
            set(hf,'position',[pos(1:2) figsize])
            set(hf,'units','pixels')
            
            plot(chi_reg,b_reg,'c','linewidth',2)
            hold on
            %             plot(bincenters,s0chiindmean,'kx'); % plot the means
            %             plot(bincenters,s0chiindmeankeep,'b.')
            if any(keepind)
                h = boxplot(s0(keepind),chibincenter(keepind),'position',unique(chibincenter(keepind)));
            end
            xticktmp = 0:45:360;
            set(gca,'xtick',xticktmp,'xticklabel',xticktmp);
            xlabel('Relative azimuth angle [deg]')
            ylabel('Sigma0 (linear)')
            
            grid on
            
        end
        
        %%
        if p.Results.savefitplots
            saveplotdir = p.Results.saveplotdir;
            
            savename = sprintf('%sSimpleFit_%s_beam_Speed%.1f_Bin%.1f', ...
                p.Results.saveroot,p.Results.whichbeamstr, ...
                thisspd,spdbinwidth);
            print('-dtiff',fullfile(saveplotdir,[savename '.tif']))
            % print('-depsc',fullfile(saveplotdir,[savename '.eps']))
        end
    end
end

end % function ts = FitData_DoFit(allspd,allchi,allsig0,allincang,params,varargin)


function outputname = RemapFieldName(inputname)
% need this function because I renamed some of the fields
switch inputname
    case 'wsat_spd_lf'
        outputname = 'wsat_wspd_lf';
    case 'wsat_spd_mf'
        outputname = 'wsat_wspd_mf';
    otherwise
        outputname = inputname;
end


end