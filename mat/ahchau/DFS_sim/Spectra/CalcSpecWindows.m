function [Px1x1,Px2x2,Px1x2,f] = CalcSpecWindows(x1,x2,windowlength,noverlap,f,Fs,varargin)
% function [Px1x1,Px2x2,Px1x2,f] = CalcSpecCrossSpecWindows_Norm(x1,x2,windowlength,noverlap,f,Fs, optionalparameters);
%
% like [Pxx,f] = pwelch(x,window,noverlap,f,fs)
% currently doesn't use noverlap
% doesn't use f either
% Fs is sampling frequency
% x1 is one signal, x2 is another. The outputs Px1x1 and Px2x2 are the
% power spectra of x1 and x2. Px1x2 is the complex cross spectrum of x1 and
% x2.
%
% You can pass in empty x2 and only get out Px1x1 (Px1x2, Px1x2 will be
% empty)
% 
% OPTIONAL PARAMETERS:
%      scalemethod:   
%                     'none' (no scaling)
%                     'Parseval'
%                     'Parseval2' 
%                         'Parseval' is N^2 times 'Parseval2'
%      windowmethod:  'window_nozeros' (default) applies hann window only 
%                          over region excluding leading/trailing zeros 
%                          (may be shorter than the windowlength)
%                     'window_standard' applies hann window over entire 
%                          signal. may lead to high frequencies if long
%                          sequence of trailing or leading zeros exist
%                     
% 
% NOTE THAT THIS DETRENDS EACH INDIVIDUAL WINDOW!
%
%
% A. Chau 2/19/10 combined from CalcSpecWindows_Norm.m and
%                 CalcSpecCrossSpecWindows_norm.m

%% Parse inputs
p = inputParser;
p.addRequired('x1');
p.addRequired('x2');
p.addRequired('windowlength')
p.addRequired('noverlap')
p.addRequired('f')
p.addRequired('Fs')
p.addParamValue('scalemethod','WindowScale',@(x)any(strcmpi(x, ...
    {'none','Parseval2','Parseval','WindowScale','testme'}))); % default was Parseval2
p.addParamValue('windowmethod','window_nozeros',@(x)any(strcmpi(x, ...
    {'window_standard','window_nozeros'})));

p.parse(x1,x2,windowlength,noverlap,f,Fs,varargin{:});

scalemethod = p.Results.scalemethod;
windowmethod = p.Results.windowmethod;


%% Prepare
% Calculate the proper number of windows
numwindows = floor(length(x1)/windowlength);
% currently use floor, discard last elements

% get the wavenumber/frequency axis
N = windowlength;
if rem(N,2) == 0 % if N is even
    wavenum = linspace(0,1,N/2 + 1) * Fs/2;
else
    wavenum = linspace(0,1,ceil(N/2)) * Fs/2;
end
wavenum = wavenum';

%% 
Allx1x1 = zeros(length(wavenum),numwindows);
if ~isempty(x2)
    Allx2x2 = Allx1x1;  Allx1x2 = Allx1x1;
else
    Allx2x2 = []; Allx1x2 = [];
end

for q = 1:numwindows
    ind = (q-1)*windowlength + [1:windowlength];
    
%     tmpdata1 = x1(ind);
    [tmpdata1,AveWindowPower] = DetrendAndWindow(x1(ind),windowmethod); % grab data and detrend
    %     line(ind,tmpdata1,'color','r','linestyle','--')
    
    ft1 = FTAndScale(x1(ind),tmpdata1,scalemethod,AveWindowPower,Fs); % ft and scale
           
    % take the first half of the spectrum
    ft1 = ft1(1:length(wavenum));
    % Calculate the spectrum
    spec1 = ft1.*conj(ft1);
    Allx1x1(:,q) = spec1;
    
    % calculate the second spectrum and cross spectrum if applicable
    if ~isempty(x2)
        [tmpdata2,WindowEnergy,AveWindowPower] = DetrendAndWindow(x2(ind),windowmethod);
        ft2 = FTAndScale(x2(ind),tmpdata2,scalemethod,WindowEnergy,Fs,AveWindowPower);
        
        ft2 = ft2(1:length(wavenum));
        spec2 = ft2.*conj(ft2);
        Allx2x2(:,q) = spec2;
        
        % cross spectrum
        Allx1x2(:,q) = ft1.*conj(ft2);
        
    end
end

% Average over all the windows
f = wavenum;
%% Take means over windows
ind = find(isnan(Allx1x1(1,:))); %(if any windows were nans, make them empty)
for q = length(ind):-1:1
    Allx1x1(:,ind(q)) = [];
end
Px1x1 = mean(Allx1x1,2);

if ~isempty(x2)
    ind = find(isnan(Allx2x2(1,:)));
    for q = length(ind):-1:1
        Allx2x2(:,ind(q)) = [];
    end
    
    ind = find(isnan(Allx1x2(1,:)));
    for q = length(ind):-1:1
        Allx1x2(:,ind(q)) = [];
    end
    
    Px2x2 = mean(Allx2x2,2);
    Px1x2 = mean(Allx1x2,2);
else
    Px2x2 = []; Px1x2 = [];
end

end


function [x, WindowEnergy, AveWindowPower] = DetrendAndWindow(x,windowmethod)
% detrend the data coming in

% want to detrend without leading or trailing zeros
ind = find(x~=0);
if isempty(ind)
    indlength=0;
else
    indlength=length(ind(1):ind(end));
end
%if length(ind) > (.95*length(x))
if  indlength > (.95*length(x))
% if ~isempty(ind)
    ind2 = ind(1):ind(end); % get indices encompassing nonzero region (might have zeros within region)
    
    %     if isreal(x)
%%%%%%%%%    
    x(ind2) = detrend(x(ind2)); % leave 0s on the outside
%%%%%%%%%
%     end
    
    switch windowmethod
        case 'window_nozeros'
            % window the data (without leading or trailing zeros)
            hwindow = hann(length(ind2));
            x(ind2) = x(ind2) .* hwindow;
            AveWindowPower = mean(hwindow.^2);
            WindowEnergy = sum(hwindow.^2);
        case 'window_standard'
            hwindow = hann(length(x));
            x = x .* hwindow;
            AveWindowPower = mean(hwindow.^2);
            WindowEnergy = sum(hwindow.^2);
        otherwise 
            %             x = x;
            disp('Not windowing!!')
            AveWindowPower = 1;
            WindowEnergy = 1;
    end
else
    % x is all zero, no change
    % or not enough samples, set x to be all zero
    x = zeros(size(x));
    AveWindowPower = 1;
    WindowEnergy = 1;
end
end

function scaledft = FTAndScale(x,xdetrend,scalemethod,WindowEnergy,Fs,AveWindowPower)
% Fourier transform and appropriately scale

ft = fft(xdetrend);

% switch scalemethod
%     case 'none'
%         alph = 1;
%     case 'Parseval2'
%         alph = sqrt( var(x) / sum(conj(ft).*ft));
%     case 'Parseval'
%         alph = sqrt( length(x) * sum(x.^2) / sum(conj(ft).*ft));
% end

switch scalemethod
    case 'none'
        alph = 1;
    case 'Parseval2'
        alph = sqrt( var(xdetrend) / sum(conj(ft).*ft));
    case 'Parseval'
        alph = sqrt( length(xdetrend) * sum(xdetrend.^2) / sum(conj(ft).*ft));
    case 'WindowScale'
%         alph = sqrt(1/length(xdetrend)/AveWindowPower*Fs);
        alph = sqrt(1/WindowEnergy/Fs);
    case 'testme'
        %         alph = 1/(length(ft));
        %         alph = sqrt( sum(xdetrend.^2) / sum(conj(ft).*ft) / 1^2);
        
        %         alph = sqrt( var(xdetrend) / sum( conj(ft).*ft /length(ft))) /
        %         sqrt(length(ft));
        
        %         n = length(ft);
        %         numuniquepoints = ceil((n+1)/2);
        %         alph = 1/sqrt(n) * sqrt(numuniquepoints/.5);
        %
        %         ft = ft;
        %
        %         alph = sqrt( var(xdetrend) / sum(conj(ft).*ft));
        % %         alph=1;
        % alph = 1/sum(hann(length(x)));
        %         scaledft = ft * alph;
        
        alph = sqrt(1/(length(xdetrend)*mean(hann(length(xdetrend)).^2)));
end


scaledft = ft * alph;


end