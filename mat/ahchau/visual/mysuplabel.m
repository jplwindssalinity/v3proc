function [ax,h]=mysuplabel(text,whichLabel,supAxes)
% PLaces text as a title, xlabel, or ylabel on a group of subplots.
% Returns a handle to the label and a handle to the axis.
%  [ax,h]=suplabel(text,whichLabel,supAxes)
% returns handles to both the axis and the label.
%  ax=suplabel(text,whichLabel,supAxes)
% returns a handle to the axis only.
%  suplabel(text) with one input argument assumes whichLabel='x'
%
% whichLabel is any of 'x', 'y', 'yy', or 't', specifying whether the 
% text is to be the xlable, ylabel, right side y-label, 
% or title respectively.
%
% supAxes is an optional argument specifying the Position of the 
%  "super" axes surrounding the subplots. 
%  supAxes defaults to [.08 .08 .84 .84]
%  specify supAxes if labels get chopped or overlay subplots
%
% EXAMPLE:
%  subplot(2,2,1);ylabel('ylabel1');title('title1')
%  subplot(2,2,2);ylabel('ylabel2');title('title2')
%  subplot(2,2,3);ylabel('ylabel3');xlabel('xlabel3')
%  subplot(2,2,4);ylabel('ylabel4');xlabel('xlabel4')
%  [ax1,h1]=suplabel('super X label');
%  [ax2,h2]=suplabel('super Y label','y');
%  [ax3,h2]=suplabel('super Y label (right)','yy');
%  [ax4,h3]=suplabel('super Title'  ,'t');
%  set(h3,'FontSize',30)
%
% SEE ALSO: text, title, xlabel, ylabel, zlabel, subplot,
%           suptitle (Matlab Central)
%
% Author: Ben Barrowes <barrowes@alum.mit.edu>
%
%modified 3/16/2010 by IJW to make axis behavior re "zoom" on exit same as
%at beginning. Requires adding tag to the invisible axes
%
% modified A. Chau 9/30/10 to be a little faster (don't use commented out
% "axes" command)
%
% modified by AHChau 11/19/12 to add tags to the labels. also replaced
% nested ifs with switch statement

% currax=findobj(gcf,'type','axes','-not','tag','suplabel');

if nargin < 3
 supAxes=[.08 .08 .84 .84];
 ah=findall(gcf,'type','axes');
 if ~isempty(ah)
  supAxes=[inf,inf,0,0];
  leftMin=inf;  bottomMin=inf;  leftMax=0;  bottomMax=0;
  axBuf=.04;
  set(ah,'units','normalized')
  ah=findall(gcf,'type','axes');
  for ii=1:length(ah)
   if strcmp(get(ah(ii),'Visible'),'on')
    thisPos=get(ah(ii),'Position');
    leftMin=min(leftMin,thisPos(1));
    bottomMin=min(bottomMin,thisPos(2));
    leftMax=max(leftMax,thisPos(1)+thisPos(3));
    bottomMax=max(bottomMax,thisPos(2)+thisPos(4));
   end
  end
  supAxes=[leftMin-axBuf,bottomMin-axBuf,leftMax-leftMin+axBuf*2,bottomMax-bottomMin+axBuf*2];
 end
end
if nargin < 2, whichLabel = 'x';  end
if nargin < 1, help(mfilename); return; end

% AC changed this part 4/15/11 based on comment on matlabcentral 
%             by Lars 11 Dec 2009
% if ~isstr(text) | ~isstr(whichLabel)
%   error('text and whichLabel must be strings')
% end
if ~(ischar(text) || (iscell(text) && all(cellfun(@ischar,text))))
  error('text must be a string or a cell string')
end
if ~ischar(whichLabel)
  error('whichLabel must be a string')
end
% AC changed this part 4/15/11
whichLabel=lower(whichLabel);

ax=axes('Units','Normal','Position',supAxes,'Visible','off','tag','suplabel');

switch whichLabel
    case 't'
        set(get(ax,'Title'),'Visible','on')
        ht=title(text);set(ht,'verticalalignment','middle')
        set(ht,'Tag','suplabeltitle');
    case 'x'
        set(get(ax,'XLabel'),'Visible','on')
        hx=xlabel(text);
        set(hx,'Tag','suplabelx');
    case 'y'
        set(get(ax,'YLabel'),'Visible','on')
        hy=ylabel(text);
        set(hy,'Tag','suplabely');
    case 'yy'
        set(get(ax,'YLabel'),'Visible','on')
        hy=ylabel(text);
        set(ax,'YAxisLocation','right')
        set(hy,'Tag','suplabelyright')
end

%  for k=1:length(currax), axes(currax(k));end % restore all other axes
ch = get(get(ax, 'parent'), 'children');
set(get(ax, 'parent'), 'children', [ch(2:end); ch(1)]);


if (nargout < 2)
  return
end
if strcmp('t',whichLabel)
  h=get(ax,'Title');
  set(h,'VerticalAlignment','middle')
elseif strcmp('x',whichLabel)
  h=get(ax,'XLabel');
elseif strcmp('y',whichLabel) | strcmp('yy',whichLabel)
  h=get(ax,'YLabel');
end

%%%ah=findall(gcf,'type','axes');
%%%'sssssssss',kb