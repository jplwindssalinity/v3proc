function y = meanNaN(x);
% function y = meanNaN(x);
% currently only works on vector data
% A Chau 4/5/10
% 
% ind = ~isnan(x);
% y = mean(x(ind));



ind = ~isnan(x);
y = mean(x(ind));