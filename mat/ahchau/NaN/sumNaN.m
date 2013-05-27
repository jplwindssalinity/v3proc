function y = sumNaN(x);
% function y = sumNaN(x);
% currently only works on vector data
% A.H. Chau 5/2/12
% 
% ind = isnan(x);
% x(ind) = 0;
% y = sum(x);

ind = isnan(x);
x(ind) = 0;
y = sum(x);