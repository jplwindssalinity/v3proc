function y = varNaN(x)
% function y = varNaN(x)
% currently only handles a vector
% A Chau 4/5/10
% 
% ind = ~isnan(x);
% y = var(x(ind));


ind = ~isnan(x);
y = var(x(ind));

