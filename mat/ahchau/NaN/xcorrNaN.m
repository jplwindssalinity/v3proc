function [c,lags,cvalid] = xcorrNaN(x,varargin)
% uses on xcorr
%
% IMPORTANT! 'coeff' might not be right
%
% A. Chau 4/2/10



% call xcorr with no scaling, then scale appropriately

% First parse the inputs.
[maxlag,scaleType,y] = parseinput(x,varargin{:});

if isempty(scaleType)
    scaleType = 'none';
end

% Check if scaleType is valid
%%%%% NEED TO DO THIS STILL %%%%%

% Calculate number of NaNs in data
% NaNxind = isnan(x); numNaNx = find(NaNxind);
% x(NaNxind) = 0;
% figure(101);
% subplot(311);plot(x)
tmp = isnan(x); x(tmp) = 0; validX = ~tmp; validX = uint8(validX);
% subplot(312);plot(x)
% subplot(313);plot(validX)

% Run xcorr to get unnormalized correlation
if isempty(y)
    if isempty(maxlag)
        disp('Running xcorr(x)')
        [c0,lags] = xcorr(x);
        cvalid = xcorr(validX);
    else
        disp('Running xcorr(x,maxlag)')
        [c0,lags] = xcorr(x,maxlag);
        cvalid = xcorr(validX,maxlag);
    end
else
    %     NaNyind = isnan(y); numNaNy = find(NaNyind);
    %     y(NaNyind) = 0;
    tmp = isnan(y); y(tmp) = 0; validY = ~tmp; validY = uint8(validY);
    if isempty(maxlag)
        disp('Running xcorr(x,y)')
        [c0,lags] = xcorr(x,y);
        cvalid = xcorr(validX,validY);
    else
        disp('Running xcorr(x,y,maxlag)')
        [c0,lags] = xcorr(x,y,maxlag);
        cvalid = xcorr(validX,validY,maxlag);
    end
end

% Normalize based on scaleType
switch scaleType
    case 'biased'
        c = c0;
        % need to change this!!
    case 'unbiased'
        %         c = c0./(cvalid - abs(lags));
        c = c0./cvalid;
    case 'coeff'
        scale = sum(abs(x).^2); %cxx0
        if ~isempty(y)
            scale = scale*sum(abs(y).^2); %cyy0
        end
        c = c0./scale;
    case 'none'
        c = c0;
end




%-------------------------------------------------------------------
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%% These functions came directly from xcorr.m %%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    function [maxlag,scaleType,y] = parseinput(x,varargin)
        %    Parse the input and determine optional parameters:
        %
        %    Outputs:
        %       xIsMatrix - flag indicating if x is a matrix
        %       AUTOFLAG  - 1 if autocorrelation, 0 if xcorrelation
        %       maxlag    - Number or lags to compute
        %       scaleType - String with the type of scaling wanted
        %       msg       - possible error message
        
        % Set some defaults
        scaleType = '';
        maxlag = [];
        y=[];
                
        errMsg = 'Input argument is not recognized.';
        
      switch nargin,
            case 2,
                % Can be (x,y), (x,maxlag), or (x,scaleType)
                if ischar(varargin{1}),
                    % Second arg is scaleType
                    scaleType = varargin{1};
                    
                elseif isnumeric(varargin{1}),
                    % Can be y or maxlag
                    if length(varargin{1}) == 1,
                        maxlag = varargin{1};
                    else
                        autoFlag = 0;
                        y = varargin{1};
                    end
                else
                    % Not recognized
                    msg = errMsg;
                    return
                end
            case 3,
                % Can be (x,y,maxlag), (x,maxlag,scaleType) or (x,y,scaleType)
                maxlagflag = 0; % By default, assume 3rd arg is not maxlag
                if ischar(varargin{2}),
                    % Must be scaletype
                    scaleType = varargin{2};
                    
                elseif isnumeric(varargin{2}),
                    % Must be maxlag
                    maxlagflag = 1;
                    maxlag = varargin{2};
                    
                else
                    % Not recognized
                    msg = errMsg;
                    return
                end
                
                if isnumeric(varargin{1}),
                    if maxlagflag,
                        autoFlag = 0;
                        y = varargin{1};
                    else
                        % Can be y or maxlag
                        if length(varargin{1}) == 1,
                            maxlag = varargin{1};
                        else
                            autoFlag = 0;
                            y = varargin{1};
                        end
                    end
                else
                    % Not recognized
                    msg = errMsg;
                    return
                end
                
            case 4,
                % Must be (x,y,maxlag,scaleType)
                autoFlag = 0;
                y = varargin{1};
                
                maxlag = varargin{2};
                
                scaleType = varargin{3};
        end
        
    end

end