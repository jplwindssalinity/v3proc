function out = trimFunc(x,percent_to_trim,fhandle)
% function trimFunc(x,percent_to_trim,fhandle)
%
% This version was written very quickly and has no error checking. only
% takes vectors.
%
% percent_to_trim is the total percentage you want off the top and bottom
% combined, i.e., if you pass in 10, the function will take 5% off the top
% and 5% off the bottom.
%
% e.g.  out = trimFunc(1:100,10,@mean)
% 
% Created by A. Chau 1/25/12

% sort the vector
x_sort = sort(x);
% how many values are there?
N = length(x);
% how many values to keep?
% get the indices (inside, i.e. may trim a little more than desired percentage)
k = ceil(N*(percent_to_trim/100)/2); % don't use this many points from top and bottom
% calculate the indices
low_ind = k+1;
high_ind = N - k;

% calculate the function passed in by fhandle
out = fhandle(x_sort(low_ind:high_ind));

end