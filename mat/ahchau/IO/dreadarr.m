function A=dreadarr(filename,linelength,precision)

% function A=dreadarr(filename,linelength,precision)
%
% Reads array from file.  The file should be written in line order and
% have the line length specified.  If no precision string is given,
% 'double' is assumed.


if(nargin<2 | nargin>3),
% A=[];
  disp('usage: A=dreadarr(filename,linelength,precision);');
  return;
elseif(nargin<3),
  precision='double';
end;

% open file
fp=fopen(filename,'r');
if(fp==-1),
  disp(sprintf('can''t open file %s',filename));
  return; 
end;

% read data (matlab reads in column order, so take transpose)
A=fread(fp,[linelength inf],precision)';  

% close file and return
fclose(fp);
return;

% end of dreadarr.m

