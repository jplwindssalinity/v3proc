function fighandle = myfigure(varargin)
% function fighandle = myfigure(fignum, ...)
% 
% This function places figures nicely on your screen. It determines your
% screen size, then calculates the number of (default sized) figures that
% can fit on your screen. 
%
% If fignum is not specified, the lowest available figure number is
% created.
%
% Parameters that you can optionally specify:
%      'figsize'      [width height] in pixel units
%      'coloffset'    number of pixels to offset top row from top of screen
%      'rowoffset'    number of pixels to offset top column from left
%      'figcolpad'    number of pixels between columns
%      'figrowpad'    number of pixels between rows
%      'comptype'     Use this to quickly switch between nice offset and 
%                     pad values for Macs ('Mac', 'mac') and Windows
%                     machines ('Windows','windows'). 'default' uses the 
%                     values that are currently hard-coded into the
%                     function.  
%
% A note about offsets and pads:  Macs have a toolbar at the top of the
% screen, which 'coloffset' avoids.  'figsize' specifies only the active
% area of the figure, not including any window manager boundaries or the
% figure's toolbars -- 'figcolpad' and 'figrowpad' are used to account for
% these toolbars and boundaries.
% 
% Created by A. Chau 12/3/09
% DOES NOT CURRENTLY HAVE GOOD PARAMETERS FOR WINDOWS 

%% parse the inputs
p = inputParser; %  create instance of inputParser class
% add arguments
p.addOptional('fignum',0, @isnumeric);
p.addParamValue('figsize',[560 420], @(x)isnumeric(x) && length(x)==2);
p.addParamValue('coloffset',95,@(x)isnumeric(x));
p.addParamValue('rowoffset',0,@isnumeric);
p.addParamValue('figcolpad',0,@isnumeric);
p.addParamValue('figrowpad',74,@isnumeric);
p.addParamValue('comptype','default',@(x)any(strcmpi(x,{'default','Mac','mac','Windows','windows'})));
% parse
p.parse(varargin{:});

fignum = p.Results.fignum;
figsize = p.Results.figsize;
coloffset = p.Results.coloffset;
rowoffset = p.Results.rowoffset;
figcolpad = p.Results.figcolpad;
figrowpad = p.Results.figrowpad;
comptype = p.Results.comptype;

%% If a comptype was specified, change offsets and pads
switch comptype
    case 'default' % do nothing
    case {'Mac','mac'}
        coloffset = 95; rowoffset = 0;
        figcolpad = 0; figrowpad = 74;
    case {'Windows','windows'}
        coloffset = 80; rowoffset = 5;
        figcolpad = 8; figrowpad = 82;
end


%% figure number
% if fignum was not specified, get the next lowest available figure number
if fignum==0
    openfigs = findall(0,'type','figure');
    fignum = 1;
    while ~isempty(find(fignum == openfigs))
        fignum = fignum+1;
    end
end

%% Calculate position and move figure there
% Get the screensize. [width, height]
screensize = get(0,'screensize');
screensize = screensize(3:4);

% Calculate the number of rows and columns that will fit on the screen
numrows = floor(screensize(2)/(figsize(2)+figrowpad));
numcols = floor(screensize(1)/(figsize(1)+figcolpad));

% figure out which position this figure number goes to
figpos = mod(fignum,numrows*numcols);
if figpos ==0
    figpos = numrows*numcols; % wrap-aroung problem
end
[col row] = ind2sub([numcols numrows],figpos);

% Calculate the coordinates
% lower left x
LLx = (col-1)*(figsize(1)+figcolpad) + rowoffset;
% lower left y
LLy = screensize(2) - coloffset - (row)*figsize(2) - (row-1)*figrowpad;

% Make (or move) the figure
fighandle=figure(fignum);
set(fighandle,'position',[LLx LLy figsize]);

end