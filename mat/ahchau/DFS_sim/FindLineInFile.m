function tmpstr = FindLineInFile(fid,str)
% Read through the file and stop at the line that includes the str

stop = 0;
while ~stop & ~feof(fid)
    tmpstr = fgetl(fid);
%         disp(tmpstr)
    
    stop = ~isempty(strfind(tmpstr,str));
end

if stop==0 % means the string wasn't found
    tmpstr = [];
end