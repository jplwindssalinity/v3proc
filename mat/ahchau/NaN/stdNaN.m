function y = stdNaN(x)
% function y = stdNaN(x)
% currently only handles a vector
% A Chau 4/5/10
% 
% ind = ~isnan(x);
% y = std(x(ind));


% ind = ~isnan(x);
% y = std(x(ind));

 y = std(x(~isnan(x)));