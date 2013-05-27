function [A,vB,sB,flag]=ConvertToBryanRainToBinary(infile,outfile)
% function [A,vB,sB,flag]=convert_to_binary(infile,outfile)
% Converts Svetla's rain files to binary ("ToBryan" files)
% 
% Written by B. Stiles?
% Modified by A. Chau 7/28/09 (header comment)


mat=load(infile,'-ASCII');

ndim1=max(mat(:,1))+1;
ndim2=max(mat(:,2))+1;
disp(num2str(size(mat))); 
A=reshape(mat(:,3),[ndim2,ndim1]);
vB=reshape(mat(:,4),[ndim2,ndim1]); %volome backscatter
sB=reshape(mat(:,5),[ndim2,ndim1]); %splash
% data dimension order is MATLAB lat,lon
%                          C     lon,lat
flag=(vB==0 | A==0 | sB==0); % 1=bad 0=good 

% convert to linear scale
A=10.^(0.1*A);
vB=10.^(0.1*vB);
sB=10.^(0.1*sB);


fid=fopen(outfile,'w','l'); % little endians for tsali, linus, etc
fwrite(fid,ndim1,'int32');
fwrite(fid,ndim2,'int32');
fwrite(fid,A,'float');
fwrite(fid,vB,'float');
fwrite(fid,sB,'float');
fwrite(fid,flag,'int32');
fclose(fid);

end



