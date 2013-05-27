function MakeComparePlot(varargin)
% function MakeComparePlot(varargin)
%
%
% parameters
% 'gmfs': can either be a structure containing gmf objects, or a string.
%         if a string, will load QS4, nscat2, and Ku2_T18
%         if string = 'nadirpointed', changes nscat incidence angles
%         if string = 'allrepointed', changes QS4 incidence angles
%         if string = 'repointed', doens't change anything
% 'coeffstoload': should be a structure that looks like this:
%    t=t+1;A(t).dirname='gmftest2';A(t).plotcolor=[0 0 1];A(t).plotstyle='o-';
%                 will load the Fits.mat file in the specified directory
% 'coeffstoplot': [0:2] is default, 0 corresponds to A0 coeff
% 'savename': beginning of filename to save plots to
% 'saveplots': 0 to not save, 1 to save
% 'plotdir': name of directory to save to, will be created if necessary
%
%
% Created by A. Chau 1/23/12

%% parse the inputs
p = inputParser;
p.addParamValue('gmfs',[]);
p.addParamValue('coeffstoload',[]);
p.addParamValue('coeffstoplot',[0:2]);
p.addParamValue('savename',[]);
p.addParamValue('saveplots',1);
p.addParamValue('plotdir',[]);
p.addParamValue('xlimtotal',[0.5 30]);
p.addParamValue('xlimlow',[0.5 5]);
p.addParamValue('xlimmid',[5 10]);
p.addParamValue('xlimhigh',[10 20]);
p.addParamValue('QS4name','QSCAT 1');
p.addParamValue('nscat2name','nscat2');
p.addParamValue('Ku2_T18name','Ku2010 T18');
p.addParamValue('nscat2only',0);
p.parse(varargin{:});



%% gmfs
if ~ischar(p.Results.gmfs)
    gmfs = p.Results.gmfs;
else
    
    % QS4 (Quikscat 1)
    gmfname = '~/sim/SimRuns/Data/GMF/ConsolidateGMFs/QS_MODLa004';
    QS4 = GMF('filetype','QS_MOD','filename',gmfname);
    %     QS4.name = 'QSCAT 1';
    QS4.name = p.Results.QS4name;
    QS4.plotcolor = [0 1 0];
    % % % QS4 = QS4.ChangeDesiredIncidenceAngles('HHinc',49,'VVinc',58');
    % nscat2
    % % % nscat2 = GMF('filename','nscat2.dat','filetype','nscat2');
    gmfname = '~/sim/SimRuns/Data/GMF/nscat2.dat';
    nscat2 = GMF('filename',gmfname,'filetype','nscat2');
    nscat2.name = p.Results.nscat2name;
    % nscat2 = nscat2.ChangeDesiredIncidenceAngles('HHinc',46,'VVinc',54);
    % Ku T18
    fname = '~/sim/SimRuns/Data/GMF/RSS_Ku2010/gmf_coeffs_ku2010_t18.DAT';
    Ku2_T18 = GMF('filetype','Ku2010','filename',fname);
    Ku2_T18.name = 'Ku2010 T18';
    Ku2_T18.name = p.Results.Ku2_T18name;
    Ku2_T18.plotcolor = [1 0 0];
    
    % Change incidence angles based on string
    switch p.Results.gmfs
        case 'nadirpointed'
            nscat2 = nscat2.ChangeDesiredIncidenceAngles('HHinc',46,'VVinc',54);
        case 'allrepointed'
            QS4 = QS4.ChangeDesiredIncidenceAngles('HHinc',49,'VVinc',58');
%             Ku2_T18 = Ku2_T18.ChangeDesiredIncidenceAngles('HHinc',49,'VVinc',58');
        case 'repointed'
        otherwise
            error('not a valid string for ''gmfs''')
    end
    
    if p.Results.nscat2only
        gmfs = nscat2;
    else
        gmfs = QS4;
        gmfs(2) = nscat2;
        gmfs(3) = Ku2_T18;
    end
end

%% load the coefficients specified by coeffstoload
A = p.Results.coeffstoload;
clear allfits
for t = 1:length(A)
    clear tmp
    fprintf('Loading coefficients from %s\n', (A(t).dirname)) % for debugging
    tmp = load(fullfile(A(t).dirname,'Fits.mat'));
    tmp.fits.plotcolor = A(t).plotcolor;
    tmp.fits.plotstyle = A(t).plotstyle;
    if isfield(A(t),'namestr')
        tmp.fits.namestr = A(t).namestr;
    end
    if t == 1
        allfits = tmp.fits;
    else
        allfits(t) = tmp.fits;
    end
end

%% check whether plotdir exists, create it if not
if p.Results.saveplots
    if ~exist(p.Results.plotdir,'dir')
        mkdir(p.Results.plotdir)
        fprintf('Created directory %s\n',p.Results.plotdir)
    end
end

%% Make the plots
hx = []; 
for n = p.Results.coeffstoplot
    lwidth = 2;
    psize = [10 7];
    ax = [];
    
    %     xlimtotal = [0.5 30];
    %     xlimlow = [0.5 5];
    %     xlimmid = [5 10];
    %     xlimhigh = [10 20];
    
    xlimtotal = p.Results.xlimtotal;
    xlimlow = p.Results.xlimlow;
    xlimmid = p.Results.xlimmid;
    xlimhigh = p.Results.xlimhigh;

    %%%%%%%%%%%%%%%%%%%%%%
    %%%%% INNER BEAM %%%%%
    %%%%%%%%%%%%%%%%%%%%%%
    figure(n+101);clf
    set(gcf,'color','white','papersize',psize,'paperposition',[0 0 psize]);
    set(gcf,'units','inches','position',[1 1 psize]);
    set(gcf,'units','pixels');
    
    % MAKE THE OVERALL PLOT
    subplot(211)
    hx = cat(1,hx,gca);
    % plot the gmfs loaded in gmfs
    for t=1:length(gmfs)
        hp = plot(gmfs(t).spd,gmfs(t).A_HH(n+1,:),'color',gmfs(t).plotcolor);
        set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigHHinc));
        set(hp,'linewidth',lwidth);
        hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
    end
    % set the x and get the y limits
    xlim(xlimtotal); xlabel('Wind speed [m/s]'); yl = ylim; grid on
    % plot the fit coefficients
    for t = 1:length(allfits)
        hp = plot(allfits(t).Inner.spd, allfits(t).Inner.Acoeffs(:,n+1), ...
            allfits(t).plotstyle, ...
            'color',allfits(t).plotcolor, 'markersize',6);
        set(hp,'DisplayName',allfits(t).namestr)
    end
    % reset x and y limits
    xlim(xlimtotal); ylim(yl)
    % show legend and title
    hl = legend('location','best'); set(hl,'fontsize',8)
    title(sprintf('Inner beam HH: A%d',n))

    % MAKE SUBREGION PLOTS BELOW
    for tt = 4:6
        switch tt
            case 4, xl = xlimlow;
            case 5, xl = xlimmid;
            case 6, xl = xlimhigh;
        end
        % make the low region plots
        subplot(2,3,tt)
        % plot the gmfs loaded in gmfs
        for t=1:length(gmfs)
            hp = plot(gmfs(t).spd,gmfs(t).A_HH(n+1,:),'color',gmfs(t).plotcolor);
            set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigHHinc));
            set(hp,'linewidth',lwidth);
            hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
        end
        % set x and get y limits
        xlim(xl); xlabel('Wind speed [m/s]'); yl = ylim; grid on
        % plot the fit coefficients
        for t = 1:length(allfits)
            hp = plot(allfits(t).Inner.spd, allfits(t).Inner.Acoeffs(:,n+1), ...
                allfits(t).plotstyle, ...
                'color',allfits(t).plotcolor, 'markersize',6);
            set(hp,'DisplayName',allfits(t).namestr)
        end
        % reset x and y limits
        xlim(xl); ylim(yl);
    end
    %%%%%%%%%%%%%%%%%%%%%%
    %%%%% INNER BEAM %%%%%
    %%%%%%%%%%%%%%%%%%%%%%


    %%%%%%%%%%%%%%%%%%%%%%
    %%%%% OUTER BEAM %%%%%
    %%%%%%%%%%%%%%%%%%%%%%
    figure(n+201);clf
    set(gcf,'color','white','papersize',psize,'paperposition',[0 0 psize])
    set(gcf,'units','inches','position',[1 8 psize]);
    set(gcf,'units','pixels');
    
    % MAKE THE OVERALL PLOT
    subplot(211);
    hx = cat(1,hx,gca);
    % plot the gmfs loaded in gmfs
    for t = 1:length(gmfs)
        hp = plot(gmfs(t).spd,gmfs(t).A_VV(n+1,:),'color',gmfs(t).plotcolor);
        set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigVVinc));
        set(hp,'linewidth',lwidth);
        hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
    end
    % set xlim and get ylim
    xlim(xlimtotal); xlabel('Wind speed [m/s]'); yl = ylim; grid on
    % Add the coefficients
    for t = 1:length(allfits)
        hp = plot(allfits(t).Outer.spd, allfits(t).Outer.Acoeffs(:,n+1), ...
            allfits(t).plotstyle, ...
            'color',allfits(t).plotcolor,'markersize',6);
        set(hp,'DisplayName',allfits(t).namestr);
    end
    % reset x and y limits
    xlim(xlimtotal)
    ylim(yl)
    % show legend and title
    hl = legend('location','best'); set(hl,'fontsize',8)
    title(sprintf('Outerbeam VV: A%d',n))
    
    % MAKE SUBREGION PLOTS BELOW
    for tt = 4:6
        switch tt
            case 4, xl = xlimlow;
            case 5, xl = xlimmid;
            case 6, xl = xlimhigh;
        end
        subplot(2,3,tt)
        for t = 1:length(gmfs)
            hp = plot(gmfs(t).spd,gmfs(t).A_VV(n+1,:),'color',gmfs(t).plotcolor);
            set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigVVinc));
            set(hp,'linewidth',lwidth);
            hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
        end
        % set the xlim and get the ylim
        xlim(xl); xlabel('Wind speed [m/s]'); yl = ylim; grid on
        % Add the coefficients
        for t = 1:length(allfits)
            hp = plot(allfits(t).Outer.spd,allfits(t).Outer.Acoeffs(:,n+1), ...
                allfits(t).plotstyle, ...
                'color',allfits(t).plotcolor,'markersize',6);
            set(hp,'DisplayName',allfits(t).namestr);
            %     set(hp,'linewidth',lwidth)
        end
        % reset xlim and ylim
        xlim(xl); ylim(yl);
    end
    %%%%%%%%%%%%%%%%%%%%%%
    %%%%% OUTER BEAM %%%%%
    %%%%%%%%%%%%%%%%%%%%%%
    
    % MAKE PLOTS OF THE AN/A0 RATIOS!
    if n > 0
        %%%%%%%%%%%%%%%%%%%%%%
        %%%%% INNER BEAM %%%%%
        %%%%%%%%%%%%%%%%%%%%%%
        figure(n+301);clf
        set(gcf,'color','white','papersize',psize,'paperposition',[0 0 psize]);
        set(gcf,'units','inches','position',[1 1 psize]);
        set(gcf,'units','pixels');
        
        % MAKE THE OVERALL PLOT
        subplot(211)
        hx = cat(1,hx,gca);
        % plot the gmfs loaded in gmfs
        for t=1:length(gmfs)
            hp = plot(gmfs(t).spd,gmfs(t).A_HH(n+1,:)./gmfs(t).A_HH(1,:), ...
                'color',gmfs(t).plotcolor);
            set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigHHinc));
            set(hp,'linewidth',lwidth);
            hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
        end
        % set the x and get the y limits
        xlim(xlimtotal); xlabel('Wind speed [m/s]'); yl = ylim; grid on
        % plot the fit coefficients
        for t = 1:length(allfits)
            hp = plot(allfits(t).Inner.spd, ...
                allfits(t).Inner.Acoeffs(:,n+1)./allfits(t).Inner.Acoeffs(:,1), ...
                allfits(t).plotstyle, ...
                'color',allfits(t).plotcolor, 'markersize',6);
            set(hp,'DisplayName',allfits(t).namestr)
        end
        % reset x and y limits
        xlim(xlimtotal); ylim(yl)
        % show legend and title
        hl = legend('location','best'); set(hl,'fontsize',8)
        title(sprintf('Inner beam HH: A%d/A0',n))
        
        % MAKE SUBREGION PLOTS BELOW
        for tt = 4:6
            switch tt
                case 4, xl = xlimlow;
                case 5, xl = xlimmid;
                case 6, xl = xlimhigh;
            end
            % make the low region plots
            subplot(2,3,tt)
            % plot the gmfs loaded in gmfs
            for t=1:length(gmfs)
                hp = plot(gmfs(t).spd,gmfs(t).A_HH(n+1,:)./gmfs(t).A_HH(1,:), ...
                    'color',gmfs(t).plotcolor);
                set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigHHinc));
                set(hp,'linewidth',lwidth);
                hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
            end
            % set x and get y limits
            xlim(xl); xlabel('Wind speed [m/s]'); yl = ylim; grid on
            % plot the fit coefficients
            for t = 1:length(allfits)
                hp = plot(allfits(t).Inner.spd, ...
                    allfits(t).Inner.Acoeffs(:,n+1)./allfits(t).Inner.Acoeffs(:,1), ...
                    allfits(t).plotstyle, ...
                    'color',allfits(t).plotcolor, 'markersize',6);
                set(hp,'DisplayName',allfits(t).namestr)
            end
            % reset x and y limits
            xlim(xl); ylim(yl);
        end
        %%%%%%%%%%%%%%%%%%%%%%
        %%%%% INNER BEAM %%%%%
        %%%%%%%%%%%%%%%%%%%%%%
        
        %%%%%%%%%%%%%%%%%%%%%%
        %%%%% OUTER BEAM %%%%%
        %%%%%%%%%%%%%%%%%%%%%%
        figure(n+401);clf
        set(gcf,'color','white','papersize',psize,'paperposition',[0 0 psize])
        set(gcf,'units','inches','position',[1 8 psize]);
        set(gcf,'units','pixels');
        
        % MAKE THE OVERALL PLOT
        subplot(211);
        hx = cat(1,hx,gca);
        % plot the gmfs loaded in gmfs
        for t = 1:length(gmfs)
            hp = plot(gmfs(t).spd,gmfs(t).A_VV(n+1,:)./gmfs(t).A_VV(1,:), ...
                'color',gmfs(t).plotcolor);
            set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigVVinc));
            set(hp,'linewidth',lwidth);
            hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
        end
        % set xlim and get ylim
        xlim(xlimtotal); xlabel('Wind speed [m/s]'); yl = ylim; grid on
        % Add the coefficients
        for t = 1:length(allfits)
            hp = plot(allfits(t).Outer.spd, ... 
                allfits(t).Outer.Acoeffs(:,n+1)./allfits(t).Outer.Acoeffs(:,1), ...
                allfits(t).plotstyle, ...
                'color',allfits(t).plotcolor,'markersize',6);
            set(hp,'DisplayName',allfits(t).namestr);
        end
        % reset x and y limits
        xlim(xlimtotal)
        ylim(yl)
        % show legend and title
        hl = legend('location','best'); set(hl,'fontsize',8)
        title(sprintf('Outerbeam VV: A%d/A0',n))
        
        % MAKE SUBREGION PLOTS BELOW
        for tt = 4:6
            switch tt
                case 4, xl = xlimlow;
                case 5, xl = xlimmid;
                case 6, xl = xlimhigh;
            end
            subplot(2,3,tt)
            for t = 1:length(gmfs)
                hp = plot(gmfs(t).spd,gmfs(t).A_VV(n+1,:)./gmfs(t).A_VV(1,:), ...
                    'color',gmfs(t).plotcolor);
                set(hp,'DisplayName',sprintf('%s (%d deg)',gmfs(t).name,gmfs(t).sigVVinc));
                set(hp,'linewidth',lwidth);
                hold on; ax = cat(1,ax,gca); set(gca,'fontsize',14);
            end
            % set the xlim and get the ylim
            xlim(xl); xlabel('Wind speed [m/s]'); yl = ylim; grid on
            % Add the coefficients
            for t = 1:length(allfits)
                hp = plot(allfits(t).Outer.spd, ...
                    allfits(t).Outer.Acoeffs(:,n+1)./allfits(t).Outer.Acoeffs(:,1), ...
                    allfits(t).plotstyle, ...
                    'color',allfits(t).plotcolor,'markersize',6);
                set(hp,'DisplayName',allfits(t).namestr);
                %     set(hp,'linewidth',lwidth)
            end
            % reset xlim and ylim
            xlim(xl); ylim(yl);
        end
        %%%%%%%%%%%%%%%%%%%%%%
        %%%%% OUTER BEAM %%%%%
        %%%%%%%%%%%%%%%%%%%%%%
    end
    
    if p.Results.saveplots
        figure(n+101);
        fname = sprintf('%s_InnerHH_A%d',p.Results.savename,n);
        print('-dpng',fullfile(p.Results.plotdir,[fname '.png']))
        
        figure(n+201);
        fname = sprintf('%s_OuterVV_A%d',p.Results.savename,n);
        print('-dpng',fullfile(p.Results.plotdir,[fname '.png']))
        
        figure(n+301);
        fname = sprintf('%s_InnerHH_A%doverA0',p.Results.savename,n);
        print('-dpng',fullfile(p.Results.plotdir,[fname '.png']))
        
        figure(n+401);
        fname = sprintf('%s_OuterVV_A%doverA0',p.Results.savename,n);
        print('-dpng',fullfile(p.Results.plotdir,[fname '.png']))
    end
    
end
linkaxes(hx,'x')

[~,tmp] = system('hostname');
compname = strtok(tmp,'.');
switch compname
    case 'blueberry'
        set(figure(201),'position',[3   802   900   630]);
        set(figure(402),'position',[865   851   900   630]);
        set(figure(403),'position',[ 1684         850         900         630]);
        set(figure(202),'position',[841   802   900   630]);
        set(figure(203),'position',[1692         801         900         630]);
        set(figure(101),'position',[2   112   900   630]);
        set(figure(302),'position',[866    70   900   630]);
        set(figure(303),'position',[1713          70         900         630]);
        set(figure(102),'position',[841   113   900   630])
        set(figure(103),'position',[1692         114         900         630]);
end
    


