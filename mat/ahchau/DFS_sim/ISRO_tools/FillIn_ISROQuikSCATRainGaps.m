function [out, indvalid, indreplace] = ...
    FillIn_ISROQuikSCATRainGaps(data,varargin)
% function [out, indvalid, indreplace] = FillIn_ISROQuikSCATRainGaps(data,varargin)
%
% Created by A. Chau 8/31/10

p = inputParser;
p.addRequired('data',@isstruct);
p.addRequired('scatterometer',[]
p.addParamValue('indvalid',-1);
p.addParamValue('indreplace',-1);
p.parse(varargin{:});
indvalid = p.Results.indvalid;
indreplace = p.Results.indreplace;