function outvals = ExtractFromRDF(fname,keywords)
% function outvals = ExtractFromRDF(fname,keywords)
%
% Extract parameters from the rdf file given in FNAME. Pass in the keywords
% that you want as a cell array of strings, KEYWORDS. The output parameters
% are given in OUTVALS, which is a cell array of cells. (This was done so
% that if a keyword appears more than once, OUTVALS will have the values
% for ALL appearances of the keyword. 
% 
% EXAMPLE USAGE: Call like this
%                   tmp = ExtractFromRDF('time.rdf',{'TRUTH_WIND_FILE'});
%                If the keyword "TRUTH_WIND_FILE" appears more than once,
%                tmp will be a single cell containing a multielemnet cell
%                array. To access the last parameter pair in the rdf file,
%                use 
%                   truthfile = char(tmp{end});
%
% Created by A. Chau 8/20/09


% fname = 'tmp.rdf';
% tok = {'TRUTH_WIND_FILE','USE_SIGMA0_MAPS','RAIN_FIELD_LAT_MIN'};

tok = keywords;
outvals = cell(size(tok)); % this will be the output values

% read the file. Each cell element of s is one line of the file
s = textread(fname,'%s','delimiter','\n','commentstyle','shell'); 

% disp(' ')
% disp(['Reading keywords from ' fname])
for q = 1:length(tok)
    ctok = tok{q};
    tokval = [];
    
    for qq = 1:length(s)
        if strfind(s{qq},ctok)
            % get part of line without keyword, including =;
            [tmp1 tmp2] = strtok(s{qq},'='); 
            
            % get part of line without the '=', has leading space
            [tmp1 val] = strtok(tmp2);
            
            % get rid of any units that may appear
            [val tmp] = strtok(val,'<');
            
            % gid rid of leading and trailing white space
            val = strtrim(val);
            disp([ctok ' : ' val]);

            % if the keyword is found multiple times, save all of them
            tokval = cat(2,tokval,{val});            
            
        end
    end
    
    outvals(q)={tokval};  %need to put tokval in cell in case it has 
             % multiple elements (multiple keywords)
end

end