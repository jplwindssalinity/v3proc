function FilterData(varargin)
% function FilterData(varargin)
% 
% parameters:
%   outerinner: 'outer','inner','both'
%   inputdir: directory from which to read subcat files
%   filterstr: which filter to implement
%   outputdir: directory to which to write subcat files
%   debugmode: 0 to do all files, if larger than 0, will do only this
%              number of files
%   fieldnamesversion: 1 for original, 2 for new for spinning data (
%   7/19/12)
%   writebinv0: write binary file for ISRO data
%
% Created by A. Chau 1/7/12
% modified by A. Chau 1/25/12 (added FilterSatConsistency v3)
% modified by A. Chau 1/26/12 (add FilterSatConsistency v4) 
% modified by A. Chau 2/1/12 (added v31 and v32)
% modified by A. Chau 7/19/12 (spinning data has new field names, be able
% to use them!)
% modified by A. Chau 10/4/12 (add Filterl2bRainImpact v0)
% modified by a. Chau 10/8/12 (add writebinv0 -- ISRO binary)
% modified by A. Chau 10/10/12 (add FilterSSMIRain v0) old ver is v6
% 
% summary of the filters:
% FilterSatConsistency
%   v0: keep a point if all of these are satisfied
%       ssmi rainrate = 0
%       ssmi wind is valid
%       ssmi timediff with QS is < 60 (absolute value)
%       (ssmi speed - ecwmf speed) < 2 m/s (absolute value)
%       (ncep speed - ecmwf speed) < 2 (absolute value)
%       ncep dir and ecmwf dir within 20 deg
%       wsat timediff with QS < 60
%       wsat all weather wind speed is valid
%   v1: everything in v0, plus
%       wsat rain = 0
%   v2: change ncep speed - ecmwf speed to match function
%       ssmi and wsat timediff reduced to 30 mins (abs) 
%       add abs(wsat_wspd_aw - ecmwf_speed) < 2
%   v3: like v2 but change timediff back to 60 mins (abs)
%   v31: like v3 but uses wsat lf speed
%   v32: like v4 but uses wsat mf speed
%
% LowPassFilter2
%       Different versions have different FWHM and sigma multiplier.
%       For each point, look for enough points around it, take a weighted
%         average with Gaussian specified by FWHM.  If there are not enough
%         neighbors close enough, discard the point.
%   v1: FWHM = 30, numpoints_sigmamultiplier = 3, threshold = 0.01
%       This means the Gaussian FHWM = 30 km.  Calculate corresponding
%         sigma. Look for enough points to span
%         sigma*numpoints_sigmamultiplier in one direction , plus a few extra
%         for margin.  Thus we'll grab enough neighbors to span 6*sigma + a
%         few (4, I think). To make sure that the points are all close
%         enough, we sum the gaussian pmf and make sure sum is not smaller  
%         than 1-threshold (1-0.01 = .99) 
%       
% Filterl2bRainImpact_v1
%   meant for ISRO data where we use the l2b rain impact info as a rain
%        flag. The data seems to use 2.61 as the threshold for flagging as
%        rainy (rainimpact >= 2.61 is rainy)
%   v1: keep only if rain impact is less than 2.5 (be a litte conservative)
% 
% FilterSSMIRain_v0
%   v0: only look for SSMI rain = 0
% 

%%%%%%%%%%%%%%%%%%%%
%%% Parse inputs %%%
%%%%%%%%%%%%%%%%%%%%

p = inputParser;
p.addParamValue('outerinner',[],@ischar);
p.addParamValue('inputdir',[],@ischar);
p.addParamValue('filterstr',[],@ischar);
p.addParamValue('outputdir',[],@ischar);
p.addParamValue('debugmode',0,@isnumeric);
p.addParamValue('writebinv0',0,@isnumeric);
% p.addParamValue('fieldnamesversion',1,@isnumeric);
p.parse(varargin{:});
% deal with the inputs:
outerinner = p.Results.outerinner;
outerinner(1) = upper(outerinner(1));
inputdir = p.Results.inputdir;
filterstr = p.Results.filterstr;
outputdir = p.Results.outputdir;
debugmode = p.Results.debugmode;
% fieldnamesversion = p.Results.fieldnamesversion;

%%%%%%%%%%%%%%%%%%%%%
%%% Initial stuff %%%
%%%%%%%%%%%%%%%%%%%%%
% Figure out which filter to use!
[filtmethod, filtversion] = strtok(filterstr,'_');
filtversion = filtversion(2:end);

if ~exist(outputdir)
    madefolder = 1;
    mkdir(outputdir)
else
    madefolder = 0;
end

logfile = LogFile(fullfile(outputdir,['FilterLog_' datestr(now,30) '.txt']));
logfile.PrintScreenLog('\n********************************\n')
logfile.PrintScreenLog('***** Start filtering data *****\n')
logfile.PrintScreenLog('********************************\n')

% Print some information
logfile.PrintScreenLog(sprintf('\nWill read data from %s\n', inputdir));
logfile.PrintScreenLog(sprintf('Will filter with %s, %s\n', filtmethod, filtversion));
logfile.PrintScreenLog(sprintf('Will write data to %s\n',outputdir));
if madefolder
    logfile.PrintScreenLog(sprintf('Created folder %s...\n',outputdir));
end

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
% Start the big loop
for q_bigloop = 1:num_loop % big loop should run at most twice, once for outer and once for inner
    if doboth
        switch q_bigloop % outer or inner on this loop?
            case 1; outerinner = 'Outer';
            case 2; outerinner = 'Inner';
        end % switch q_bigloop % outer or inner on this loop?
    end
    
    % initialize stuff
    loopstart = tic;
    logfile.PrintScreenLog(sprintf('\nStarting %s on %s beam!\n',filterstr,outerinner));
    
    % Find the relevant files in the output directory
    tmp = dir(fullfile(inputdir,sprintf('Cat%s_r*',outerinner)));
    filenames = {tmp.name};
    if debugmode > 0
        filenames = filenames(1:min(debugmode,length(filenames)));
    end
    
    % start the loop over all files
    for fname = filenames;
        tic;
        logfile.PrintScreenLog(sprintf('Working on %s...',fname{1}));
        % load the input file
        load(fullfile(inputdir,fname{1}));
        
        % figure out which version of fieldnames you are using
        if isfield(data,'rainrate')
            fieldnamesversion = 0;
        else
            if isfield(data,'ssmi_rain')
                fieldnamesversion = 2;
            else 
               if  isfield(data,'l2b_rainimpact')
                   fieldnamesversion = 3;
               end
            end
        end
            
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%% DO ALL THE FILTERING OPERATIONS HERE!!! %%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        switch filtmethod
            case 'FilterSatConsistency'
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'FilterSatConsistency' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                switch filtversion
                    case 'v0'
                        goodssmi = (data.rainrate == 0) ...
                            & (data.wind < 250) ...
                            & (abs(data.timediff) < 60) ...
                            & (abs(data.wind - data.ecmwf_speed) < 2);
                        goodncep = (abs(data.ecmwf_speed - data.ncep_speed) < 2) ...
                            & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                        goodwsat = (abs(data.wsat_timediff) < 60) & ...
                            (~isnan(data.wsat_wspd_aw)) & ...
                            (data.wsat_wspd_aw < 250);
                    case 'v1'
                        goodssmi = (data.rainrate == 0) ...
                            & (data.wind < 250) ...
                            & (abs(data.timediff) < 60) ...
                            & (abs(data.wind - data.ecmwf_speed) < 2);
                        goodncep = (abs(data.ecmwf_speed - data.ncep_speed) < 2) ...
                            & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                        goodwsat = (abs(data.wsat_timediff) < 60) ...
                            & (~isnan(data.wsat_wspd_aw))  ...
                            & (data.wsat_wspd_aw < 250) ...
                            & (data.wsat_rain == 0) ;
                    case 'v2'
                        goodssmi = (data.rainrate == 0) ...
                            & (data.wind < 250) ...
                            & (abs(data.timediff) < 30) ...
                            & (abs(data.wind - data.ecmwf_speed) < 2);
                        goodncep = (abs(data.ncep_speed - 1.2*data.ecmwf_speed + 0.6) < 1) ...
                            & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                        goodwsat = (abs(data.wsat_timediff) < 30) ...
                            & (~isnan(data.wsat_wspd_aw))  ...
                            & (data.wsat_wspd_aw < 250) ...
                            & (data.wsat_rain == 0) ...
                            & abs(data.wsat_wspd_aw - data.ecmwf_speed) < 2;
                    case 'v3'
                        goodssmi = (data.rainrate == 0) ...
                            & (data.wind < 250) ...
                            & (abs(data.timediff) < 60) ...
                            & (abs(data.wind - data.ecmwf_speed) < 2);
                        goodncep = (abs(data.ncep_speed - 1.2*data.ecmwf_speed + 0.6) < 1) ...
                            & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                        goodwsat = (abs(data.wsat_timediff) < 60) ...
                            & (~isnan(data.wsat_wspd_aw))  ...
                            & (data.wsat_wspd_aw < 250) ...
                            & (data.wsat_rain == 0) ...
                            & abs(data.wsat_wspd_aw - data.ecmwf_speed) < 2;
                    case 'v31'
                        switch fieldnamesversion
                            case 1
                                goodssmi = (data.rainrate == 0) ...
                                    & (data.wind < 250) ...
                                    & (abs(data.timediff) < 60) ...
                                    & (abs(data.wind - data.ecmwf_speed) < 2);
                                goodncep = (abs(data.ncep_speed - 1.2*data.ecmwf_speed + 0.6) < 1) ...
                                    & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                                goodwsat = (abs(data.wsat_timediff) < 60) ...
                                    & (~isnan(data.wsat_wspd_lf))  ...
                                    & (data.wsat_wspd_lf < 250) ...
                                    & (data.wsat_rain == 0) ...
                                    & abs(data.wsat_wspd_lf - data.ecmwf_speed) < 2;
                            case 2
                                goodssmi = (data.ssmi_rain == 0) ...
                                    & (data.ssmi_spd < 250) ...
                                    & (abs(data.ssmi_timediff) < 60) ...
                                    & (abs(data.ssmi_spd - data.ecmwf_spd) < 2);
                                goodncep = (abs(data.ncep_spd - 1.2*data.ecmwf_spd + 0.6) < 1) ...
                                    & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                                goodwsat = (abs(data.wsat_timediff) < 60) ...
                                    & (~isnan(data.wsat_spd_lf)) ...
                                    & (data.wsat_spd_lf < 250) ...
                                    & (data.wsat_rain == 0) ...
                                    & (abs(data.wsat_spd_lf - data.ecmwf_spd) < 2);
                        end
                    case 'v32'
                        goodssmi = (data.rainrate == 0) ...
                            & (data.wind < 250) ...
                            & (abs(data.timediff) < 60) ...
                            & (abs(data.wind - data.ecmwf_speed) < 2);
                        goodncep = (abs(data.ncep_speed - 1.2*data.ecmwf_speed + 0.6) < 1) ...
                            & (angdif(data.ecmwf_dir,data.ncep_dir) < 20);
                        goodwsat = (abs(data.wsat_timediff) < 60) ...
                            & (~isnan(data.wsat_wspd_mf))  ...
                            & (data.wsat_wspd_mf < 250) ...
                            & (data.wsat_rain == 0) ...
                            & abs(data.wsat_wspd_mf - data.ecmwf_speed) < 2;
                end % switch filtversion
                keepind = goodssmi & goodncep & goodwsat;
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'FilterSatConsistency' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                
            case 'LowPassFilter2'
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'LowPassFilter2' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                switch filtversion
                    case 'v1'
                        FWHM = 30; %FWHM of the filter
                        numpoints_sigmamultiplier = 3; % number of points used will cover this times sigma
                        threshold = 0.01;
                end % switch filtversion
                
                if ~isempty(data.sig0lin)
                    % Calculate distance between points using Haversine formula
                    R = 6371; % km
                    del_lat = diff(data.lat/180*pi);
                    del_lon = diff(data.lon/180*pi);
                    lat1 = data.lat(1:end-1)/180*pi;
                    lat2 = data.lat(2:end)/180*pi;
                    % calculate the distance
                    a = (sin(del_lat/2)).^2 + cos(lat1).*cos(lat2).*(sin(del_lon/2)).^2;
                    c = 2*atan2(sqrt(a), sqrt(1-a));
                    data.dist = R * c;
                    
                    % Calculate the filter properties
                    % sigma from full width half max
                    sigma = FWHM/(2*sqrt(2*log(2)));
                    % how many pints to use in average?
                    numpoints = ceil(sigma*numpoints_sigmamultiplier/3.6 + 2);
                    % currently, numpoints corresponds to just over 3*sigma in
                    % distance (v1)
                    % how many points to do? as many as possible! (need pad)
                    dothesepoints = (numpoints+1) : (length(data.lat)-numpoints);
                    
                    % initialize the output of the filter
                    yfilt = nan(size(data.sig0lin));
                    badind = [];
                    
                    for ind = dothesepoints
                        % calculate distance from ths point to surrounding points
                        dfuture = cumsum(data.dist(ind+[0:(numpoints-1)]));
                        dback = -flipud(cumsum(data.dist(ind-[1:numpoints])));
                        
                        xx = [dback; 0; dfuture]; % vector describing distance from this point
                        if max(abs(xx)) < (3.6* (numpoints+2))
                            % the maximum distance has to be within expected boundary, else
                            % don't calculate an average
                            
                            % calculate the area associated with each pixel of the gaussian
                            r_half = diff(xx)/2; r_half(end+1) = r_half(end);
                            l_half = diff(xx)/2; l_half(2:end+1) = l_half;
                            
                            % calculate the gaussian
                            gaussy = 1/sqrt(2*pi)/sigma * exp(-xx.^2/(2*sigma^2));
                            % check if the gaussian pmf sums to 1
                            if abs(sum((l_half+r_half) .* gaussy) -1) < threshold
                                % if it does, do the filter
                                ind2 = ind + [-numpoints:numpoints];
                                % filter is the weighted average with the gaussian
                                msig = sum(data.sig0lin(ind2).*gaussy.*(r_half+l_half));
                                yfilt(ind) = msig;
                                
                                if 0 % makeplot
                                    figure(3); clf;
                                    [ax,hp1,hp2] = plotyy(data.lat(ind2), data.sig0lin(ind2), ...
                                        data.lat(ind2), gaussy);
                                    set(hp1,'marker','.'); set(hp2,'marker','.')
                                    hold on
                                    plot(data.lat(ind),msig,'.r')
                                end
                                
                            else
                                fprintf('Uh oh, check %d: %.4f!!!\n', ind,sum((l_half+r_half) .* gaussy))
                                yfilt(ind) = nan;
                                badind = cat(1,badind,ind);
                            end
                        else
                            %         figure(3); clf
                        end % if max(abs(xx)) < (3.6* (numpoints+2))
                    end % for ind = dothesepoints
                    
                    % assign the sig0 to a new variable
                    data.sig0linorig = data.sig0lin;
                    % reassign sig0lin to the filtered version
                    data.sig0lin = yfilt;
                    
                else
                    logfile.PrintScreenLog('empty data file...');
                end % if ~isempty(data.sig0lin)
                keepind = find(~isnan(data.sig0lin));
                
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'LowPassFilter2' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'Filterl2bRainImpact' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
            case 'Filterl2bRainImpact'
                switch filtversion
                    case 'v1'
                        goodrain = (abs(data.l2b_rainimpact - (-9999))>1) ...
                            & (data.l2b_rainimpact < 2.5);
                end  % switch filtversion
                keepind = goodrain;
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'Filterl2bRainImpact' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'FilterSSMIRain' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
            case 'FilterSSMIRain'
                switch filtversion
                    case 'v0'
                        goodrain = data.rainrate == 0;
                end
                keepind = goodrain;
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                %%%%% 'FilterSSMIRain' %%%%%
                %%%%%%%%%%%%%%%%%%%%%%%%%%%%
                
            otherwise
                error(sprintf('Filter %s does not exist !',filterstr))
        end % switch filtmethod
        
        % what to keep? everything
        datafieldnames = fieldnames(data);
        for q = 1:length(datafieldnames)
            keepdata.(datafieldnames{q}) = data.(datafieldnames{q})(keepind);
        end
        data = keepdata;
        % write the output file
        save(fullfile(outputdir,fname{1}),'data')
        
        if p.Results.writebinv0
            if ~isfield(data,'cell_incidence')
                data.cell_incidence = zeros(size(data.cell_lat));
            end
            AAA = [data.cell_lat, data.cell_lon, data.cell_northazimuth, ...
                data.cell_incidence, data.sigma0_attn_corr_dB, ...
                data.sigma0_attn_corr_lin, double(data.sigma0_flag), ...
                data.ecmwf_spd, data.ecmwf_dir, data.ecmwf_land, ...
                data.l2b_ati_index, data.l2b_cti_index, data.l2b_spd, ...
                data.l2b_dir, data.l2b_rainimpact, data.echi];
            thisoutname = strrep(fname{1},'.mat','v0.bin');
            fidout = fopen(fullfile(outputdir,thisoutname),'wb');
            fwrite(fidout,AAA','float32');
            fclose(fidout);
        end
        
        
        toctmp = toc ; % display the time for this one file
        logfile.PrintScreenLogToc;
    end
    
    logfile.PrintScreenLog(sprintf('Finished %s on %s beam...',filterstr,outerinner));
    logfile.PrintScreenLogToc(loopstart);
    
    
end % for q_bigloop = 1:num_loop


%%%%%%%%%%%%%%%%
%%% Clean up %%%
%%%%%%%%%%%%%%%%
logfile.PrintScreenLog('\n***********************************\n');
logfile.PrintScreenLog('***** Finished filtering data *****\n');
logfile.PrintScreenLog('***********************************\n');

logfile.delete;
end % function FilterData(varargin)

