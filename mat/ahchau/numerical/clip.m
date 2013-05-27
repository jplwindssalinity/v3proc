function a = clip(b,bmin,bmax);
% function a = clip(b, bmin, bmax)
%
% Clips all values in matrix b to fit in the range [bmin,bmax]
% and returns a matrix a of the clipped values.

if(nargin~=3),
  disp('usage: a=clip(b,bmin,bmax);');
  a=b; 
  return;
end;
if(max([size(bmin), size(bmax)])>1),
  disp('clip: Error: bmin and bmax must be scalars');
  a=b;
  return;
end;
if(bmin>bmax),
  disp('clip: bmin must be less than or equal to bmax');
  a=b;
  return;
end;

a=max(min(b,bmax),bmin);

% ver=version;
% ver=str2num(ver(1));
% if(ver>=4),
%   a=max(min(b,bmax),bmin);
% else,
%   a=b;
%   i=find(b<bmin);
%   if(length(i)),
%     a(i)=bmin*ones(size(i));
%   end;
%   i=find(b>bmax);
%   if(length(i)),
%     a(i)=bmax*ones(size(i));
%   end;
% end;


% end clip.m
