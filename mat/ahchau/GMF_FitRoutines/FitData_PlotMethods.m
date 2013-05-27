classdef FitData_PlotMethods
%
% Created by AHChau (unknown date)
% edited by AHChau 12/6/12s
    
    properties
        hist_sigdBbins
        hist_spdbins_nwp
        hist_spdbins_sat
        
        hist_dirbins
%         hist_dirbins_wsat
        
        binsize_geodeg
        hist_latbins
        hist_lonbins
        
        hist_rainbins
        
        cmapname
        
        xmeanstyle
        ymeanstyle
        graycolor
    end % properties
    
    
    methods
        function obj = FitData_PlotMethods
            obj.hist_sigdBbins = -40:.5:0;
            obj.hist_spdbins_nwp = 0:.2:25;
            obj.hist_spdbins_sat = 0:.2:25;
            
            obj.hist_dirbins = 0:1.5:360;
            %             obj.hist_dirbins_wsat = 0:1.5:360;
            
            obj.binsize_geodeg = 0.5;
            obj.hist_latbins = -90:obj.binsize_geodeg:90;
            obj.hist_lonbins = 0:obj.binsize_geodeg:360;
            
            obj.hist_rainbins = 0:.1:25;
            
            obj.cmapname = 'pink';
            
            obj.xmeanstyle = 'wx';
            obj.ymeanstyle = 'ro';
            obj.graycolor = [1 1 1]*.5;
        end % FitDataPlotMethods class constructor
        
        
        function [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramSpdSig0(obj, ...
                speedx,sig0y,speedxtype,varargin)
            %[hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramSpdSig0(obj,speedx,sig0y,speedxtype,varargin)
            % p.addParamValue('fignum',[],@(x)isnumeric(x));
            % p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            % A. Chau 12/6/11
            p = inputParser;
            p.addParamValue('fignum',[],@(x)isnumeric(x));
            p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            p.parse(varargin{:});
            
            switch speedxtype
                case 'nwp'
                    xbins = obj.hist_spdbins_nwp;
                case 'sat'
                    xbins = obj.hist_spdbins_sat;
            end
            ybins = obj.hist_sigdBbins;
                 
            if isempty(p.Results.fignum)
                hf = figure;
            else
                hf = figure(p.Results.fignum);
            end
            if p.Results.ClearFig
                clf;
            end
            
            % replace negative sigma0s with eps
            sig0yeps = sig0y; sig0yeps(sig0y<0) = eps; 
            
            % get the histogram
            [counts, ~,~, xbinindex,ybinindex] = hist2d(speedx,10*log10(sig0yeps),xbins,ybins);
            counts(counts==0) = nan; % set zeros to nan
            % get the means
            xmean = accumarray(ybinindex,speedx,[length(ybins) 1],@(x)mean(x));
            ymean = accumarray(xbinindex,sig0y,[length(xbins) 1],@(x)mean(x));
            
            % Make the plot
            imagesc(xbins,ybins,counts);
            colorbarnan('cmap',obj.cmapname);
            axis xy; grid on; hold on
            plot(xbins,10*log10(ymean),obj.xmeanstyle);
            plot(xmean,ybins,obj.ymeanstyle);
            plot(xbins([1 end]),xbins([1 end]),'--','color',obj.graycolor);
            
            hf = gcf;
            ax = gca;
        end % PlotHistogramSpdSig0
        
        function [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramSpdSpd(obj, ...
                speedx,speedy,speedxtype,speedytype,varargin)
            % [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramSpdSpd(obj,speedx,speedy,speedxtype,speedytype,varargin)
            % p.addParamValue('fignum',[],@(x)isnumeric(x));
            % p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            % A. Chau 12/6/11
            
            p = inputParser;
            p.addParamValue('fignum',[],@(x)isnumeric(x));
            p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            p.parse(varargin{:});
            
            switch speedxtype
                case 'nwp'
                    xbins = obj.hist_spdbins_nwp;
                case 'sat'
                    xbins = obj.hist_spdbins_sat;
            end
            switch speedytype
                case 'nwp'
                    ybins = obj.hist_spdbins_nwp;
                case 'sat'
                    ybins = obj.hist_spdbins_sat;
            end
          
            
            if isempty(p.Results.fignum)
                hf = figure;
            else
                hf = figure(p.Results.fignum);
            end
            if p.Results.ClearFig
                clf;
            end
            
            [counts,xmean,ymean] = obj.PlotHistWithMean(speedx,speedy,xbins,ybins);
            
            hf = gcf;
            ax = gca;
        end % PlotHistogramSpdSpd
        
        function [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramDirDir(obj, ...
                dirx,diry,varargin)
            % [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramDirDir(obj,dirx,diry,varargin)
            % p.addParamValue('fignum',[],@(x)isnumeric(x));
            % p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            % A. Chau 12/6/11
            
            p = inputParser;
            p.addParamValue('fignum',[],@(x)isnumeric(x));
            p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            p.parse(varargin{:});
            
            if isempty(p.Results.fignum)
                hf = figure;
            else
                hf = figure(p.Results.fignum);
            end
            if p.Results.ClearFig
                clf;
            end
            
            xbins = obj.hist_dirbins; ybins = obj.hist_dirbins;
            [counts] = obj.PlotHist(dirx,diry,xbins,ybins);
            
            hf = gcf;
            ax = gca;
            
        end % PlotHistogramDirDir
        
        function [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramRainRain(obj, ...
                rainx,rainy,varargin)
            % [hf,ax,xmean,ymean,xbins,ybins,counts] = PlotHistogramRainRain(obj, ...
            %    rainx,rainy,varargin)
            % p.addParamValue('fignum',[],@(x)isnumeric(x));
            % p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            % A. Chau 12/14/11
            
            p = inputParser;
            p.addParamValue('fignum',[],@(x)isnumeric(x));
            p.addParamValue('ClearFig',1,@(x)isnumeric(x));
            p.parse(varargin{:});
            
            if isempty(p.Results.fignum)
                hf = figure;
            else
                hf = figure(p.Results.fignum);
            end
            if p.Results.ClearFig
                clf;
            end
            
            xbins = obj.hist_rainbins; ybins = obj.hist_rainbins;
            [counts,xmean,ymean] = obj.PlotHistWithMean(rainx,rainy,xbins,ybins);
            
            hf = gcf;
            ax = gca;
        end % PlotHistogramRainRain
               
        function [counts,xmean,ymean,xstd,ystd,xmed,ymed] = PlotHistWithMean(obj,xdata,ydata,xbins,ybins)
            % get the histogram
            [counts, ~,~, xbinindex,ybinindex] = hist2d(xdata,ydata,xbins,ybins);
            counts(counts==0) = nan; % set zeros to nan
            % get the means
            xmean = accumarray(ybinindex,xdata,[length(ybins) 1],@(x)mean(x));
            ymean = accumarray(xbinindex,ydata,[length(xbins) 1],@(x)mean(x));
            
            % get the std as well, if requested
            if (nargout > 3)
                % get the std as well
                xstd = accumarray(ybinindex,xdata,[length(ybins) 1],@(x)std(x));
                ystd = accumarray(xbinindex,ydata,[length(xbins) 1],@(x)std(x));
            end
            
            % get the median, if requested
            if (nargout > 5)
                xmed = accumarray(ybinindex,xdata,[length(ybins) 1],@(x)median(x));
                ymed = accumarray(xbinindex,ydata,[length(xbins) 1],@(x)median(x));
            end
            
            % Make the plot
            imagesc(xbins,ybins,counts);
            colorbarnan('cmap',obj.cmapname);
            axis xy; grid on; hold on
            plot(xbins,ymean,obj.xmeanstyle);
            plot(xmean,ybins,obj.ymeanstyle);
            plot(xbins([1 end]),xbins([1 end]),'--','color',obj.graycolor);
        end
        
        function [counts] = PlotHist(obj,xdata,ydata,xbins,ybins)
            % get the histogram
            [counts, ~,~, xbinindex,ybinindex] = hist2d(xdata,ydata,xbins,ybins);
            counts(counts==0) = nan; % set zeros to nan
            
            % Make the plot
            imagesc(xbins,ybins,counts);
            colorbarnan('cmap',obj.cmapname);
            axis xy; grid on; hold on
            plot(xbins([1 end]),xbins([1 end]),'--','color',obj.graycolor);
            hold off
        end
    end % methods
    

end % classdef