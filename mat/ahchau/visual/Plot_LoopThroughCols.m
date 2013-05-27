function Plot_LoopThroughCols(x,y,varargin)
% function LoopThroughCols(x,y,varargin)
% p.addParamValue('fignum',[])
% makes a plot and loops through y values
% 
% Created by A H Chau 8/21/12
% 
% future improvements: add ability to plot multiple lines, i.e. loop
% through 2 variables

p = inputParser;
p.addParamValue('fignum',[])
p.parse(varargin{:});

p.Results

if isempty(p.Results.fignum)
    figure; clf
else
    figure(p.Results.fignum); clf
end

% set color order
colororder = [0 0 1; 
    0 .5 0; 
    1 0 0;
    0 .75 .75; 
    .75 0 .75;
    .74 .75 0;
    .25 .25 .25];
    

for q_mat = 1:size(y,3);
    hp(q_mat) = line(x,y(:,1,q_mat),'color',colororder(q_mat,:));
end
xlim([x(1) x(end)])
ylim([min(y(:)) max(y(:))])
grid on


for q_col = 1:size(y,2)
for q_mat = 1:size(y,3);

        set(hp(q_mat),'ydata',y(:,q_col,q_mat))
    end
    pause(.05)
end


% %%
% hp = line(x,y(:,1));
% xlim([x(1) x(end)])
% ylim([min(y(:)) max(y(:))])
% grid on
% 
% for q = 1:size(y,2)
%     set(hp,'ydata',y(:,q))
%     pause(.1)
% end

