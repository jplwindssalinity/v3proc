function GMF = ReadGMF_simQUIKSCAT(fname)
% function GMF = ReadGMF_simQUIKSCAT(fname)
% reads something like this:
%GMF_FILE                    = ../../dat/QS_MODLa004_le_1
%GMF_FILE_FORMAT             = QUIKSCAT
%GMF_PHI_COUNT               = 360
% this is high resolution format with max speed of 50 m/s
%
% P1 should be VV, P2 should be HH
% A. Chau 12/2/10

%% Read the file
% fname = '../ConsolidateGMFs/QS_MODLa004_le_1';
fid = fopen(fname,'rb');
g = fread(fid,'float32');
fclose(fid);

% GMF limits:
inc = 40:1:60;
spd = .1:.1:50;
% chi = 1:1:360;
chi = 0:1:359;


numexpected = length(inc)*length(spd)*length(chi);
fprintf('Expected %d floats in file, read %d\n',numexpected,length(g));

% reshape to 3d matrix
g = reshape(g,[length(inc),length(spd),length(chi)]);

% Note that the part of the table (incidence angle from 40 to 50 deg) is HH
% and the second part (incidence angle from 51 to 60) is VV 

HHind = 1:11;
VVind = 12:21;


GMF.filename = fname;
GMF.inc = inc;
GMF.spd = spd;
GMF.chi = chi;
GMF.P1 = g;
GMF.P1(HHind,:,:) = nan; % write nans in the invalid part (HH part)
GMF.P2 = g;
GMF.P2(VVind,:,:) = nan; % write nans in the invalid part (VV part)
