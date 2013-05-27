function [fighandleN, axhandle]=makefigs_mysize(N)
% function [fighandleN, axhandle]=makefigs_mysize(N)
% Makes a figure with a single set of axes.  The input N is the figure number.
% Function returns the handles to the figure and to the axis.
% Parameters you want to set:
%                   figscl
%                   axaspectratio -- the aspect ratio you want for the axes
%                   paperwidth
%                   margin
%                   toplabelsize, bottomlabelsize, leftlabelsize, rightlabelsize -- amount of space available for axis labels and tick marks
%
% Created by A.Chau 1/23/07 (modified from B. Erkmen)
% Modified by A.Chau 2/22/07 (added help comments)

fighandleN = figure(N);
figscl = 1; %provides relatively easy parameter to scale the figure
axaspectratio=5/4; %width to height
paperwidth = figscl*5;
papersize=[paperwidth paperwidth/axaspectratio]; %Size of the bounding box
margin = figscl*[0.00 0.00];   %How much margin space do you want around your figure [left/right top/bottom]
figsize = papersize - margin; %This is the total size your figure can occupy INCLUDING all text and labeling
toplabelsize = figscl*(0.3/axaspectratio);
bottomlabelsize= figscl*(0.6/axaspectratio);           %The amount of space you need for your labels
leftlabelsize= figscl*0.6;  %default 0.6
rightlabelsize = figscl*0.2; %default 0.1

axissize = figsize - [(leftlabelsize + rightlabelsize) (toplabelsize+bottomlabelsize)]; %This is the size of the actual axis and data area

set(fighandleN,'PaperUnits','inches','Units','inches')

ScreenInitCoord = [5 7.5]; %Initial coordinates to display the figure on the screen
FixedOff = [0.2 0.75];
switch mod(N,4)
    case 1
        Hoffset = 0;
        Voffset = 0;
    case 2
        Hoffset = papersize(1)+FixedOff(1);
        Voffset = 0;
    case 3
        Hoffset = 0;
        Voffset = -papersize(2)-FixedOff(2);
    case 0
        Hoffset = papersize(1)+FixedOff(1);
        Voffset = -papersize(2)-FixedOff(2);
end

set(fighandleN,'Position',[ScreenInitCoord(1)+Hoffset ScreenInitCoord(2)+Voffset figsize],'PaperSize',papersize, ...
    'PaperPosition',[ 0.5*margin   figsize]);

axhandle=axes;
set(axhandle, 'Units','inches')
set(axhandle,'Position', [leftlabelsize bottomlabelsize axissize])