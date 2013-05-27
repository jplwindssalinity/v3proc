function img = ReadImageRegion(read_cols,read_rows,readfile,varargin);
% function img = ReadImageRegion(read_cols,read_rows,readfile);
% 
%
% Modified from ReadImageRegion 9/9/10
% New version (old version called v1 now). reads appropriately when you ask
% for rows or cols outside the file boundaries
%
% Orig for ReadImageRegion:
% Created by A. Chau 8/17/09
% Modified to use inputParser 1/25/10
% Modified by A. Chau from ReadDEMRegion.m to be generic1/12/12

p = inputParser;
p.addRequired('read_cols', @isnumeric);
p.addRequired('read_rows', @isnumeric);
p.addRequired('imfilename',@ischar);
p.addParamValue('total_cols',[], @isnumeric);
p.addParamValue('total_rows',[], @isnumeric);
p.addParamValue('datatype','float32', @ischar);
p.addParamValue('endian','ieee-le',@ischar);
p.addParamValue('missingvalue',-10000,@isnumeric);
p.addParamValue('scaling_factor',1,@isnumeric);
p.parse(read_cols,read_rows,readfile,varargin{:});
% disp(p.Results)
read_cols = p.Results.read_cols;
read_rows = p.Results.read_rows;
imfilename = p.Results.imfilename;
%inputs = p.Results;

switch p.Results.datatype
    case 'float32'
        datumlength = 4; %bytes
    case 'int16'
        datumlength = 2; %bytes
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Calculate how to read the image %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% figure out what to skip in reading (number of elements)
headerskip = p.Results.total_cols * (read_rows(1)-1);
within_row_skip1 = read_cols(1) - 1;
within_row_skip2 = p.Results.total_cols - read_cols(end);
% calculate the number to read per line
numtoread = length(read_cols);



%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%% Read the image %%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%
% preallocate the image
img = nan(length(read_rows),length(read_cols));
% read the image
fid = fopen(imfilename,'rb');
status=fseek(fid,headerskip*datumlength,'bof'); % skip the first part
if status ~=-1
    for q = 1:length(read_rows)
        status = fseek(fid,within_row_skip1*datumlength,'cof');
        if status == -1
            error('Asked for something out of range 1')
        end
        g = fread(fid,numtoread,p.Results.datatype)';
        status = fseek(fid,within_row_skip2*datumlength,'cof');
        if status == -1
            error('Asked for something out of range 2')
        end
        img(q,:) = g;
    end
else error('Asked for something out of range 3')
end
fclose(fid);

% check for boundaries (if you've asked to read outside the boundaries of
% the file
% could just return error
% if (max(read_rows) > p.Results.total_rows) | (max(read_cols) > p.Results.total_cols);
%     error('Asked for more rows or columns than exist in file')
% end
% more useful to return nans instead
if ~isempty(read_cols>p.Results.total_cols)
    ind = find(read_cols>p.Results.total_cols);
    img(:,ind) = nan;
end

% Assuming the empty values are -10000, replace them with NaN
if ~isempty(p.Results.missingvalue)
    ind = find(img== p.Results.missingvalue);
    img(ind)=nan;
end

% disp(max(img(:)))
img = double(img) * p.Results.scaling_factor;
% disp(max(img(:)))

end