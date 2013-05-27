function ConcatenateSubFiles(varargin)
% function ConcatenateSubFiles(varargin)
%
% parameters:
%   outerinner: 'outer','inner','both'
%   datadir: directory from which to read subcat files and write big file
%   debugmode: 0 to do all files, if larger than 0, will do only this
%              number of files
%
% Created by A. Chau 1/17/12


%%%%%%%%%%%%%%%%%%%%
%%% Parse inputs %%%
%%%%%%%%%%%%%%%%%%%%

p = inputParser;
p.addParamValue('outerinner',[],@ischar);
p.addParamValue('datadir',[],@ischar);
p.addParamValue('debugmode',0,@isnumeric);
p.parse(varargin{:});
% deal with the inputs
outerinner = p.Results.outerinner;
outerinner(1) = upper(outerinner(1));
datadir = p.Results.datadir;
debugmode = p.Results.debugmode;

%%%%%%%%%%%%%%%%%%%%%
%%% Initial stuff %%%
%%%%%%%%%%%%%%%%%%%%%

% open the log file
logfile = LogFile(fullfile(datadir,['CatLog_' datestr(now,30) '.txt']));
logfile.PrintScreenLog('\n**************************************\n');
logfile.PrintScreenLog('***** Started concatenating data *****\n');
logfile.PrintScreenLog('**************************************\n');
% print some information
logfile.PrintScreenLog(sprintf('Will concatenate data in %s\n', datadir));


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
    
    % start a timer and show some output
    loopstart = tic;
    logfile.PrintScreenLog(sprintf('\nStarting to concatenate %s beam!\n',outerinner));
    
    tmp = dir(fullfile(datadir,sprintf('Cat%s_r*',outerinner)));
    filenames = {tmp.name};
    if debugmode > 0
        filenames = filenames(1:min(debugmode,length(filenames)));
    end
    
    %                 % read the first file and get the fieldnames
    %                 tic
    %                 logfile.PrintScreenLog(sprintf('%d: Loading %s...',1,filenames{1}));
    %                 load(fullfile(datadir,filenames{1}));
    %                 BIGdata = data;
    %                 datafieldnames = fieldnames(data);
    %                 logfile.PrintScreenLogToc;
    %
    % loop through the files
    for q_file = 1:length(filenames);
        tic
        logfile.PrintScreenLog(sprintf('%d: Loading %s...',q_file,filenames{q_file}));
        load(fullfile(datadir,filenames{q_file}));
        if q_file == 1 % if it's the first one, initialize BIGdata and grab fieldnames
            BIGdata = data;
            datafieldnames = fieldnames(data);
        else % for all other files, concatenate
            for q_datafieldnames = 1:length(datafieldnames)
                BIGdata.(datafieldnames{q_datafieldnames}) = ...
                    cat(1,BIGdata.(datafieldnames{q_datafieldnames}), ...
                    data.(datafieldnames{q_datafieldnames}) );
            end %for q_datafieldnames = 1:length(datafieldnames)
        end
        logfile.PrintScreenLogToc;
    end % for q_file = 2:length(filenames);
    
    if ~isempty(filenames)
        % save BIGdata as data
        clear data
        data = BIGdata;
        savenameroot = fullfile(datadir,['Cat' outerinner]);
        logfile.PrintScreenLog(sprintf('Saving %s...',savenameroot))
        tic
        save(savenameroot,'data','-v7.3');
        logfile.PrintScreenLogToc;
    else
    end
    
    % save statistics for the whole loop
    logfile.PrintScreenLog(sprintf('Finished concatenating subfiles for %s beam...',outerinner))
    logfile.PrintScreenLogToc(loopstart);
    
end %for q_bigloop = 1:num_loop


%%%%%%%%%%%%%%%%
%%% Clean up %%%
%%%%%%%%%%%%%%%%
logfile.PrintScreenLog('\n***************************************\n');
logfile.PrintScreenLog('***** Finished concatenating data *****\n');
logfile.PrintScreenLog('***************************************\n');

logfile.delete;
end % function ConcatenateSubFiles(varargin)
