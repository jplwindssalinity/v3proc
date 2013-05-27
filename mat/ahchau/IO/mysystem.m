function out = mysystem(cmdstr,varargin);
% function out = mysystem(cmdstr,testmode);
% 
% if testmode = 0, just runs system(cmdstr)
% if testmode ~=0, does not run system call, only tells what would have
% been run
%
% p.addRequired('cmdstr',@ischar);
% p.addOptional('testmode',0);
% 
% Created by AH Chau  9/26/12

p=inputParser;
p.addRequired('cmdstr',@ischar);
p.addOptional('testmode',0);
p.parse(cmdstr,varargin{:});

if p.Results.testmode
    fprintf('Would run: %s\n',cmdstr);
else
    out = system(cmdstr);
end

end