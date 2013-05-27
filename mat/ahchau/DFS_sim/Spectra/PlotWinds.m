
function out = PlotWinds(IN);
% function out = PlotWinds(IN);
% IN.scenerootdir = '/Users/achau/sim/SimRuns/Runs/Region4'
% IN.scenes = 
%
% Created by A. Chau 4/16/10

out=1;
% use a structure to bring the data in?
fsize = [12 5];

% IN.scenerootdir = fullfile('/Users/achau/sim/SimRuns/Runs/Region4');
% % IN.scenes = {'2006-12-13_000000','2006-12-13_020000'};
% IN.scenes = {'2006-12-12_120000','2006-12-12_140000', ...
%             '2006-12-12_160000','2006-12-12_180000', ...
%             '2006-12-12_200000','2006-12-12_220000', ...
%             '2006-12-13_000000','2006-12-13_020000', ...
%             '2006-12-13_040000','2006-12-13_060000', ...
%             '2006-12-13_080000','2006-12-13_100000', ...
%             '2006-12-13_120000','2006-12-13_140000', ...
%             '2006-12-13_160000','2006-12-13_180000', ...
%             '2006-12-13_200000','2006-12-13_220000', ...
%             '2006-12-14_000000','2006-12-14_020000', ...
%             '2006-12-14_040000','2006-12-14_060000', ...
%             '2006-12-14_080000','2006-12-14_100000', ...
%             '2006-12-14_120000','2006-12-14_140000', ...
%             '2006-12-14_160000','2006-12-14_180000', ...
%             '2006-12-14_200000','2006-12-14_220000', ...
%             '2006-12-15_000000','2006-12-15_020000', ...
%             '2006-12-15_040000','2006-12-15_060000', ...
%             '2006-12-15_080000','2006-12-15_100000', ...
%             '2006-12-15_120000','2006-12-15_140000', ...
%             '2006-12-15_160000','2006-12-15_180000', ...
%             '2006-12-15_200000','2006-12-15_220000', ...
%             '2006-12-16_000000','2006-12-16_020000', ...
%             '2006-12-16_040000','2006-12-16_060000', ...
%             '2006-12-16_080000','2006-12-16_100000', ...
%             '2006-12-16_120000','2006-12-16_140000', ...
%             '2006-12-16_160000','2006-12-16_180000'};
% IN.savedir = fullfile('/Users/achau/sim/SimRuns/SeniorReview/Spectra_201004/ZPlots_R4_20100415');
% 
% IN.rundir = 'qs_SR_norain';
% clear rets
% if 1
%     t=0;
%     t=t+1;rets(t)=struct('fname','l2b_QS12.5_GS_arrays.dat', ...
%         'truename','l2b_QS12.5_GS_trutharrays.dat', ...
%         'name','GS12.5 selected','res',12.5,'rundir',IN.rundir, ...
%         'fieldname','GS12p5sel');
%     t=t+1;rets(t)=struct('fname','l2b_QS12.5_GS_neararrays.dat', ...
%         'truename','l2b_QS12.5_GS_trutharrays.dat', ...
%         'name','GS12.5 nearest nudge','res',12.5,'rundir',IN.rundir, ...
%         'fieldname','GS12p5nearnudge');
%     t=t+1;rets(t)=struct('fname','l2b_QS12.5_GS_neartrutharrays.dat', ...
%         'truename','l2b_QS12.5_GS_trutharrays.dat', ...
%         'name','GS12.5 nearest truth','res',12.5,'rundir',IN.rundir, ...
%         'fieldname','GS12p5neartruth');
%     
%     t=t+1;rets(t)=struct('fname','l2b_QS12.5_S3_arrays.dat', ...
%         'truename','l2b_QS12.5_S3_trutharrays.dat', ...
%         'name','S312.5 selected','res',12.5,'rundir',IN.rundir, ...
%         'fieldname','S312p5');
%     
%     %     t=t+1;rets(t)=struct('fname','l2b_QS25_GS_arrays.dat', ...
%     %         'truename','l2b_QS25_GS_trutharrays.dat', ...
%     %         'name','GS25 selected','res',25,'rundir',rundir,'fieldname','GS25sel');
%     %     t=t+1;rets(t)=struct('fname','l2b_QS25_S3_arrays.dat', ...
%     %         'truename','l2b_QS25_S3_trutharrays.dat', ...
%     %         'name','S325 selected','res',25,'rundir',rundir,'fieldname','S325sel');
%     %
%     %     t=t+1;rets(t)=struct('fname','l2b_QS50_GS_arrays.dat', ...
%     %         'truename','l2b_QS50_GS_trutharrays.dat', ...
%     %         'name','GS50 selected','res',50,'rundir',rundir,'fieldname','GS50sel');
%     %     t=t+1;rets(t)=struct('fname','l2b_QS100_GS_arrays.dat', ...
%     %         'truename','l2b_QS100_GS_trutharrays.dat', ...
%     %         'name','GS100 selected','res',100,'rundir',rundir,'fieldname','GS100sel');
%     
%     %     t=t+1;rets(t)=struct('fname','l2b_QS12.5overlap_S3_arrays.dat', ...
%     %         'truename','l2b_QS12.5overlap_S3_trutharrays.dat', ...
%     %         'name','S12.5ovr','res',12.5,'rundir',rundir,'fieldname','S312p5ovr')
% else
%     rets = [];
% end
% 
% IN.rets = rets;


%%
rets = IN.rets;
%%
for qqq = 1:length(IN.scenes);
    
    scenedir = fullfile(IN.scenerootdir,IN.scenes{qqq});
    fprintf('In %s\n',scenedir);
    
    clear WRF SIM
    
    % do this for each ret type
    for q = 1:length(rets)
        fprintf('Reading %s\n',rets(q).name)
        filename = fullfile(scenedir,rets(q).rundir,rets(q).fname);
        truearrayfilename = fullfile(scenedir,rets(q).rundir,rets(q).truename);
        tmp = ReadWindAndTruthArray(filename,truearrayfilename,rets(q).res);
        SIM.(rets(q).fieldname) = tmp;
    end
    
    % Get the WRF field
    rdffile = fullfile(scenedir,IN.rundir,[IN.rundir '.rdf']);
    tmp = ExtractFromRDF(rdffile,{'TRUTH_WIND_FILE'});
    truthfile = char(tmp{end});
    if exist(truthfile)
        WRF = ReadWRFArray(truthfile, 4); %res =4;
    else
        disp(sprintf('%s does not exist\n'))
    end
    
    % Here is where we make the plots!
    if 1
        for q = 1:length(rets)
            
            h=figure(1);clf;
            set(h,'papersize',fsize,'paperposition',[0 0 fsize])
            set(h,'units','inches','position',[0.2 1 fsize])
            
            
            switch rets(q).res
                case 12.5
                    xskip = 8;
                case 25
                    xskip = 4;
                case 50
                    xskip = 2;
                case 100
                    xskip =1;
            end
            yskip = xskip;
            
            subplot(141)
            utmp = SIM.(rets(q).fieldname).retumat;
            vtmp = SIM.(rets(q).fieldname).retvmat;
            imagesc(sqrt(utmp.^2 + vtmp.^2));axis xy
            hold on
            [xtmp,ytmp] = meshgrid(1:size(utmp,2),1:size(utmp,1));
            quiver(xtmp(1:yskip:end,1:xskip:end), ...
                ytmp(1:yskip:end,1:xskip:end),utmp(1:yskip:end,1:xskip:end), ...
                vtmp(1:yskip:end,1:xskip:end),'k')
            caxs = caxis;
            xlabel('CTI');ylabel('ATI');axis image
            title(rets(q).name)
            
            
            subplot(142)
            utmp = SIM.(rets(q).fieldname).truumat;
            vtmp = SIM.(rets(q).fieldname).truvmat;
            imagesc(sqrt(utmp.^2 + vtmp.^2));axis xy
            hold on
            [xtmp,ytmp] = meshgrid(1:size(utmp,2),1:size(utmp,1));
            quiver(xtmp(1:yskip:end,1:xskip:end), ...
                ytmp(1:yskip:end,1:xskip:end),utmp(1:yskip:end,1:xskip:end), ...
                vtmp(1:yskip:end,1:xskip:end),'k')
            caxs(2,:) = caxis;
            xlabel('CTI');ylabel('ATI');axis image
            title('Truth on same grid')
            
            
            subplot(122)
            xskip = 15; yskip = xskip;
            utmp = WRF.u; vtmp = WRF.v;
            imagesc(WRF.lonlim,WRF.latlim, ...
                sqrt(utmp.^2 + vtmp.^2));axis xy
            hold on
            lonlim = WRF.lonlim;
            latlim = WRF.latlim;
            [xtmp,ytmp] = meshgrid(linspace(lonlim(1),lonlim(2),size(utmp,2)), ...
                linspace(latlim(1),latlim(2),size(utmp,1)));
            quiver(xtmp(1:yskip:end,1:xskip:end), ...
                ytmp(1:yskip:end,1:xskip:end),utmp(1:yskip:end,1:xskip:end), ...
                vtmp(1:yskip:end,1:xskip:end),'k')
            [a b c] = fileparts(WRF.filename);
            title(strrep(['WRF: ' b c],'_','\_'))
            
            xlabel('Longitude [degrees]');ylabel('Latitude [degrees]')
            caxs(3,:) = caxis;
            colorbar; axis image
            
            subplot(141);caxis([0 ceil(max(caxs(:,2)))])
            subplot(142);caxis([0 ceil(max(caxs(:,2)))])
            subplot(122);caxis([0 ceil(max(caxs(:,2)))])
                        
            if isfield(IN,'savedir')
                if exist(IN.savedir,'dir')
                    fname = [IN.rundir '_SimWindFields_' b(1:14) '_' rets(q).fieldname];
                    print('-dpdf',fullfile(IN.savedir,'eps',[fname '.pdf']))
                    % print('-depsc','-adobecset',fullfile(savedir,'eps',[fname '.eps']))
                    print('-dtiff','-r150',fullfile(IN.savedir,'tif',[fname '.tif']))
                    print('-dpng',fullfile(IN.savedir,'png',[fname '.png']))
                end
            end
        end
    end
    
    
end