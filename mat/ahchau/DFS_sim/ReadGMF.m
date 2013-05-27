function GMF = ReadGMF(fname)
% function GMF = ReadGMF(fname)
% GMF.P1 is first half, GMF.P2 is second half
% P1 should be VV, P2 should be HH
%
% GMF = ReadGMF('Ku_MergedQSIWRAP_20100318.dat') 
%
% SOME PLOT COMMANDS FOLLOW
% % plot for given incidence angle, range of speeds, function of azimuth 
% figure(5);clf; incs = [18 21];
% for q = 1:2
%     subplot(1,2,q); this_inc = incs(q); this_spd = 2:2:40;
%     % to_plot = log10(squeeze(g1mat(this_inc,this_spd,:)))';
%     to_plot = squeeze(g1mat(this_inc,this_spd,:))';
%     plot(chi,to_plot); grid on; xlim([0 180])
%     for q = 1:length(this_spd)
%         text(2,to_plot(1,q),num2str(this_spd(q)),'verticalalignment','bottom')
%     end
%     xlabel('Azimuth (chi) [\circ]')
%     title([strrep(fname,'_','\_'), ', incidence angle = ' num2str(inc(this_inc))])
% end
%
% A. Chau 9/24/09

% fname = 'GMF_c.dat';

dtr = 1.745329252e-2;

%% Read the file

fid = fopen(fname,'rb');
% frewind(fid)
g = fread(fid,'float32');
fclose(fid);

% The file should have 2 (met_idx = 5,6) * 37 (iazimuth angles, chi) * ...
% 99 (speed steps, not including 0) * 26 (incidence angles)
numexpected = 2*37*99*26;
disp(sprintf('Expected %d floats in file, read %d',numexpected,length(g)))

% These numbers represent how the GMF should have been written
inc= 16:2:66;
spd = 1:1:99;
chi = 0:5:180;

%% Reformat the file
% Make it a 3D matrix.

g1 = g(1:numexpected/2);
g2 = g(numexpected/2+1:end);

disp(sprintf('number elements different between 1st and 2nd half: %d',length(find(g1-g2))))

% g1mat = reshape(g1,[26,99,37]);
% g2mat = reshape(g2,[26,99,37]);

g1mat = reshape(g1,[length(inc),length(spd),length(chi)]);
g2mat = reshape(g2,[length(inc),length(spd),length(chi)]);

GMF.filename = fname;
GMF.inc = inc;
GMF.spd = spd;
GMF.chi = chi;
GMF.P1 = g1mat;
GMF.P2 = g2mat;


end
