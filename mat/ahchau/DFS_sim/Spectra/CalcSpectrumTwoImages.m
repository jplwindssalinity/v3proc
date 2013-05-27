function [Pmat1, Pmat2, Pmat12, f] = CalcSpectrumTwoImages(mat1,mat2,windowlength,fs)
% function [Pmat1, Pmat2, Pmat12, f] = CalcSpectrumTwoImages(mat1,mat2, ...
%                                      windowlength,fs)
%
% Calculates and returns spectra for each column. 
% Can use empty mat2 to calculate only the power spectrum of mat1
%
% INPUTS:
% mat1 and mat2 are the data matrices. windowlength and fs are passed into 
% CalcSpecCrossSpecWindows_Norm
%
% OUTPUTS:
% Pmat1, Pmat2 are the spectra of each column of mat1 and mat2
% Pmat12 has the cross spectrum between columns of mat1 and mat2
% f is the frequency axis (calculated from windowlength and fs)
%
% A. Chau 2/18/10


%% preallocate matrices for spectra
n = ceil((windowlength+1)/2);

% Pmat1 = zeros(n,size(mat1,2));
Pmat1 = nan(n,size(mat1,2));
Pmat2 = Pmat1;
Pmat12 = Pmat1;


%% Calculate spectra for each column
if ~isempty(mat2)
    
    if size(mat1) ~= size(mat2)
        disp('size not equal!!')
    end
    for col = 1:size(mat1,2)
        x1 = mat1(:,col);
        x2 = mat2(:,col);
        [Px1x1,Px2x2,Px1x2,f] = CalcSpecWindows(x1,x2,...
            windowlength,0,0,fs);
        
        Pmat1(:,col) = Px1x1;
        Pmat2(:,col) = Px2x2;
        Pmat12(:,col) = Px1x2;
    end
    
else
    for col = 1:size(mat1,2)
        x = mat1(:,col);
        [Pxx,~,~,f] = CalcSpecWindows(x,[],windowlength,0,0,fs);
        Pmat1(:,col) = Pxx;
    end
    
end
