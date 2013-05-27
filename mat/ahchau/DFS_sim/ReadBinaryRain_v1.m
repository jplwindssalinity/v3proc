function [A,vB,sB,flag,ndim1,ndim2] = ReadBinaryRain(infile);
% Written by A. Chau 8/7/09

fid = fopen(infile,'rb','l');

ndim1 = fread(fid,1,'int32');
ndim2 = fread(fid,1,'int32');

A = fread(fid,ndim1*ndim2,'float');
vB = fread(fid,ndim1*ndim2,'float');
sB = fread(fid,ndim1*ndim2,'float');
flag = fread(fid,ndim1*ndim2,'int32');

fclose(fid);

A = reshape(A,[ndim1,ndim2]);
vB = reshape(vB,[ndim1,ndim2]);
sB = reshape(sB,[ndim1,ndim2]);

% ndim1=max(mat(:,1))+1;
% ndim2=max(mat(:,2))+1;
% disp(num2str(size(mat))); 
% A=reshape(mat(:,3),[ndim2,ndim1]);
% vB=reshape(mat(:,4),[ndim2,ndim1]); %volome backscatter
% sB=reshape(mat(:,5),[ndim2,ndim1]); %splash
% % data dimension order is MATLAB lat,lon
% %                          C     lon,lat
% flag=(vB==0 | A==0 | sB==0); % 1=bad 0=good 
% 
% % convert to linear scale
% A=10.^(0.1*A);
% vB=10.^(0.1*vB);
% sB=10.^(0.1*sB);
% 
% 
% fid=fopen(outfile,'w','l'); % little endians for tsali, linus, etc
% fwrite(fid,ndim1,'int32');
% fwrite(fid,ndim2,'int32');
% fwrite(fid,A,'float');
% fwrite(fid,vB,'float');
% fwrite(fid,sB,'float');
% fwrite(fid,flag,'int32');
% fclose(fid);
 
end