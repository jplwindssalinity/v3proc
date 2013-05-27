function GetLooksFromL2A(fname)
% function GetLooksFromL2A(fname)
% Give filename of an ascii l2a file (e.g. fname = 'l2a_ku.asc').
% Reads the l2a file for number of looks of each of 4 types: 
%    inner/outer fore/aft. Stores in a structure variable, 'Looks', and 
%    saves in a file IN THE WORKING DIRECTORY called 
%    ['LookCount_' strrep(fname,'.asc','') '.mat']
%
%
% Created by A. Chau 9/10/09



%fname = 'l2a_ku.txt';
% fname = 'l2a_ku_CENTROID.asc';

fid = fopen(fname,'rt');

%%
frewind(fid)
% First read the Header
stop = 0;
while ~stop
    str = fgetl(fid);
    disp(str);
    %stop if the line contains CrossTrackRes
    stop = ~isempty(strfind(str,'CrossTrackRes'));
end
% read the resolution line
tmp = textscan(str,'CrossTrackRes: %f AlongTrackRes: %f',1);
hdr.CrossTrackRes = tmp{1};
hdr.AlongTrackRes = tmp{2};
% read the number of bins line
str = fgetl(fid);
tmp = textscan(str,'CrossTrackBins: %d AlongTrackBins: %d',1);
hdr.CrossTrackBins = tmp{1};
hdr.AlongTrackBins = tmp{2};

hdr


%% Figure out the last pair of indices in the file
system(['grep Rev ' fname ' > tmp.txt'])
fid2 = fopen('tmp.txt','rt');
% find the last line
while ~feof(fid2)
    str=fgetl(fid2);
end
tmp = textscan(str,'Rev: 0 AlongTrackIndex: %d CrossTrackIndex: %d');
LastATI = tmp{1}; LastCTI = tmp{2};
fclose(fid2)

disp(sprintf('LastATI = %d, Last CTI = %d',LastATI,LastCTI))

% %% Figure out the total counts
%  system(['grep Count ' fname ' > tmp.txt'])
% fid2 = fopen('tmp.txt','rt');
% tmp = textscan(fid2,'### Slice/Composite Count: %d ####')
% fclose(fid2)

%%
% Repeat this until the end of the file
tic
readnum = 0;
while ~feof(fid) 
    % Go through file looking for l2A Data Records
    % use AlongTrackIndex as row, CrossTrackIndex as column
    
    % Find the beginning of the next L2A  Data Record
    str = FindLineInFile(fid,'L2A  Data Record');
    
    % First figure out wvc index
    str = FindLineInFile(fid,'Rev: 0');
    if ~isempty(str)
        tmp = textscan(str,'Rev: 0 AlongTrackIndex: %d CrossTrackIndex: %d');
        ATI = tmp{1};
        CTI = tmp{2};
    end
    
    % Then read number of slices
    str = FindLineInFile(fid,'Slice/Composite Count');
    if ~isempty(str)
        tmp = textscan(str,'### Slice/Composite Count: %d');
        count = tmp{1};
   
        % Assign number of slices to a matrix
        countmat(ATI,CTI) = count;
    end
    
    % Figure out how many belong to each group
    VV = [0 0]; HH = [0 0];
    
    for q = 1:count
        str = FindLineInFile(fid,'BeamIdx');
        tmp = textscan(str,'BeamIdx: %*d %*s %*d %*s %*d ScanAngle: %f %*s %*f %*s %*f %*s %*f MeasType: %s IncAngle: %f eastAzimuth: %f');
        MeasType = tmp{2};
        ScanAngle = tmp{1};
        switch char(MeasType)
            case 'VV'
                if (ScanAngle >90) & (ScanAngle<270)
                    VV(1) = VV(1)+1;
                else
                    VV(2) = VV(2)+1;
                end
            case 'HH'
                if (ScanAngle>90) & (ScanAngle<270)
                    HH(1) = HH(1)+1;
                else
                    HH(2) = HH(2)+1;
                end
        end
    end
    
    VV1_mat(ATI,CTI) = VV(1);
    VV2_mat(ATI,CTI) = VV(2);
    HH1_mat(ATI,CTI) = HH(1);
    HH2_mat(ATI,CTI) = HH(2);
        
    readnum = readnum+1;
    if rem(readnum,5000)==0
        disp(sprintf('Currently on ATI: %d, CTI: %d',ATI,CTI))
        toc
    end
  
%     disp(ATI)
end
toc

disp(sprintf('Finished with ATI: %d, CTI: %d',ATI,CTI))

Looks.fname = fname;
Looks.hdr = hdr;
Looks.VV1_mat = VV1_mat;
Looks.VV2_mat = VV2_mat;
Looks.HH1_mat = HH1_mat;
Looks.HH2_mat = HH2_mat;
Looks.countmat = countmat;

save(['LookCount_' strrep(fname,'.asc','')],'Looks')

%%
fclose(fid)


end
%%
% % % %%
% % % % Go through file looking for l2A Data Records
% % % % use AlongTrackIndex as row, CrossTrackIndex as column
% % % stopWVC = 0;
% % % while ~stopWVC
% % %     str = fgetl(fid);
% % %     disp(str);
% % %     stopWVC = ~isempty(strfind(str,'L2A  Data Record'));
% % % end
% % % 
% % % 
% % % % First figure out wvc index
% % % stopindex = 0;
% % % while ~stopindex
% % %     str=fgetl(fid);
% % %     disp(str)
% % %     stopindex = ~isempty(strfind(str,'Rev: 0'));
% % % end
% % % tmp = textscan(str,'Rev: 0 AlongTrackIndex: %d CrossTrackIndex: %d');
% % % ATI = tmp{1}
% % % CTI = tmp{2}
% % % 
% % % % Then read number of slices
% % % str = FindLineInfile(fid,'Slice/Composite Count');
% % % tmp = textscan(str,'### Slice/Composite Count: %d');
% % % count = tmp{1}
% % % countmat(ATI,CTI) = count;