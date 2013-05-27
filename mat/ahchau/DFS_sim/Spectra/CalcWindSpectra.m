function windout = CalcWindSpectra(wind)

% windout = wind;


windout.ati1 = wind.ati1;
windout.nati = wind.nati;
windout.ncti = wind.ncti;
windout.deltalat = wind.deltalat;
windout.retfilename = wind.retfilename;
windout.trufilename = wind.trufilename;

fs = 1./wind.deltalat;
windowlength = round(1250/wind.deltalat);
windowlength = min([windowlength, round(size(wind.retumat))]);

numrowsforspec = windowlength*2;
firstrowforspec = floor((size(wind.retumat,1)-numrowsforspec)/2);
useind = firstrowforspec + (0:numrowsforspec-1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calculate spectra for components %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calculate spectra for u component
[Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(wind.retumat(useind,:), ...
    wind.truumat(useind,:),windowlength,fs);
windout.Prru = Pmat1; windout.Pttu = Pmat2; windout.Prtu = Pmat12;

% Calculate spectra for v component
[Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(wind.retvmat(useind,:), ...
    wind.truvmat(useind,:),windowlength,fs);
windout.Prrv = Pmat1; windout.Pttv = Pmat2; windout.Prtv = Pmat12;

% Calculate spectra for speed
[Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(wind.retsmat(useind,:), ...
    wind.trusmat(useind,:),windowlength,fs);
windout.Prrs = Pmat1; windout.Ptts = Pmat2; windout.Prts = Pmat12;

% Calculate spectra for direction (exp(j*direction))
[Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(exp(1i*wind.retdmat(useind,:)/180*pi), ...
    exp(1i*wind.trudmat(useind,:)/180*pi),windowlength,fs);
windout.Prrd = Pmat1; windout.Pttd = Pmat2; windout.Prtd = Pmat12;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calculate coherence between u and v components %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% coherence between u and v for the retrieved field
[~,~,Pmat12,f] = CalcSpectrumTwoImages(wind.retumat(useind,:), ...
    wind.retvmat(useind,:),windowlength,fs);
windout.Pruv = Pmat12;
if ~isempty(wind.trufilename) % check to see if there is a truth field
    [~,~,Pmat12,f] = CalcSpectrumTwoImages(wind.truumat(useind,:), ...
        wind.truvmat(useind,:),windowlength,fs);
    windout.Ptuv = Pmat12;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Calculate spectra for errors %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if ~ isempty(wind.trufilename) % check to see if there is a truth field
    % Calculate the errors first
    uerror = wind.retumat - wind.truumat;
    verror = wind.retvmat - wind.truvmat;
    serror = wind.retsmat - wind.trusmat;
    derror = wind.retdmat - wind.trudmat;
    ind = derror>180; derror(ind) = derror(ind) - 360;
    ind = derror<-180; derror(ind) = derror(ind) + 360;
    % U and V error spectra
    [Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(uerror(useind,:),verror(useind,:), ...
        windowlength,fs);
    windout.PuER = Pmat1; 
    windout.PvER = Pmat2;
    windout.PERuv = Pmat12;
    % speed and direction errors
    [Pmat1,Pmat2,Pmat12,f] = CalcSpectrumTwoImages(serror(useind,:), ...
        exp(1i*derror(useind,:)/180*pi),windowlength,fs);
    windout.PsER = Pmat1;
    windout.PdER = Pmat2; 
    windout.PERsd = Pmat12;
    
    % Also store info for later calculating error statistics
    windout.uerror = uerror;
    windout.verror = verror;
    windout.serror = serror;
    windout.derror = derror;
else
    % no truth exists
end
    
% Store other stuff too
windout.f = f;



%%  test area
% ddiff = wind.retdmat - wind.trudmat;
% ind = find(ddiff>180); ddiff(ind) = ddiff(ind) - 360;
% ind = find(ddiff<-180); ddiff(ind) = ddiff(ind) + 360;
% 
