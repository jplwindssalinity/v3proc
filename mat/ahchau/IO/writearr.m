function writearr(data,filename,precision)

% function writearr(data,filename,precision)
%
% data is a one or two dimensional matrix of data values.
% filename is a string that the output file will be called.
% precision is an option precision specifier, as in 'float' or 'int'.
% see help on fread for a list of allowed precisions.
% elements in data matrix are written line by line.


% process inputs
if(nargin<2 | nargin>3),
  disp('usage: writearr(data,filename,precision);');
  return;
elseif(nargin<3),
  precision='float';
end;

% do some error checking
if(~ischar(filename)),
  disp('invalid file name passed to writearr.m');
  return;
end;
if(isempty(data)),
  disp('WARNING: empty data matrix passed to writearr.m\n');
end;

% open file
fp=fopen(filename,'w');
if(fp==-1),
  disp('cannot write to file specified');
  return;
end;

% write the array
count=fwrite(fp,data',precision);
fclose(fp);

% check to make sure everything went okay
if(count ~= (size(data,1)*size(data,2))),
  disp(sprintf('error occurred during write to %s.  %d elements written.\n',...
      filename,count));
end;

% end of writearr.m
