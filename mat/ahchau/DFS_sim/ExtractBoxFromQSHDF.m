function [out] = ExtractBoxFromQSHDF(fname,boxlat,boxlon)
% function out = ExtractBoxFromQS(fname,boxlat,boxlon)
% fname = QS hdf file
% boxlat = [minlat maxlat]
% boxlon = [minlon maxlon]
% output:    
%            out.fname = fname;
%            out.lat = wvc_lat;
%            out.lon = wvc_lon;
%            out.u = u;
%            out.v = v;
%            out.spd = wind_speed_selection;
%            out.dir = wind_dir_selection;
%            out.timestr = char(wvc_row_time{1}');
%            out.ati1 = rowstoread(1);
%            out.nati = length(rowstoread);
%            out.res = (25 or 12.5 km)
%
% A. Chau 2/9/10

% boxlat = [20.892 41.921];
% boxlon = [205.740 230.625];
[pathstr, name, ext, versn] = fileparts(fname);
out.fname = fname;
if ~exist(fname)
    out.u=[];out.v=[];out.spd=[];out.dir=[];out.timestr=[];out.ati1=[];
    error('invalid file name')
else
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%% FIGURE OUT WHICH ROWS TO READ %%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Read the lat/lon positions of the wind cells in the file
    wvc_lat = hdfread(fname,'wvc_lat');
    wvc_lon = hdfread(fname,'wvc_lon');
    % Apply scale factor
    wvc_lat = double(wvc_lat)*.01;
    wvc_lon = double(wvc_lon)*.01;
    
    % Look for retrievals within the box specified by boxlat and boxlon
    indlat = (wvc_lat>boxlat(1)) & (wvc_lat<boxlat(2));
    indlon = (wvc_lon>boxlon(1)) & (wvc_lon<boxlon(2));
    indbox = find(indlat&indlon);
    % indbox has indices of WVCs within the specified box
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    %%%%% FIGURE OUT WHICH ROWS TO READ %%%%%
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % find the rows corresponding to the box
    [r,c] = ind2sub(size(wvc_lat),indbox);
    rowstoread = unique(r);
    
    if isempty(rowstoread)
        fprintf('%s%s, lat/lon=(%.3f, %.3f, %.3f, %.3f), numrows: %d\n', ...
            name,ext, boxlat, boxlon, 0)
        %         fprintf('%s%s: no rows within lat: (%f, %f), lon: (%f, %f)\n', ...
        %             name,ext, boxlat, boxlon)
        out.u=[];out.v=[];out.spd=[];out.dir=[];out.timestr=[];out.ati1=[];
    else
        numcols = size(wvc_lat,2); %total number of columns to read
        %         fprintf('%s%s: %d rows within lat: (%f, %f), lon: (%f, %f)\n', ...
        %             name,ext, length(rowstoread), boxlat, boxlon)
        %         fprintf('%s%s, lat/lon=(%.3f, %.3f, %.3f, %.3f), numrows: %d\n', ...
        %             name,ext, boxlat, boxlon, length(rowstoread))
        
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%% NOW READ ONLY THE DESIRED DATA %%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        indextoread = {[rowstoread(1) 1],[1 1],[length(rowstoread) numcols]};
        wvc_lat = hdfread(fname,'/wvc_lat','Index',indextoread);
        wvc_lon = hdfread(fname,'/wvc_lon','Index',indextoread);
        wind_speed_selection = hdfread(fname,'/wind_speed_selection', ...
            'Index',indextoread);
        wind_dir_selection = hdfread(fname,'/wind_dir_selection', ...
            'Index',indextoread);
        
        wvc_row_time = hdfread(fname,'/wvc_row_time', ...
            'Fields','wvc_row_time','FirstRecord',rowstoread(1) ,'NumRecords',1);
        
        wvc_lastrow_time = hdfread(fname,'/wvc_row_time', ...
            'Fields','wvc_row_time','FirstRecord',rowstoread(end) ,'NumRecords',1);
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%% NOW READ ONLY THE DESIRED DATA %%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%% CONVERT TO U AND V, AND PREPARE OUTPUT %%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        % convert lat lon to degrees
        wvc_lat = double(wvc_lat)*0.01;
        wvc_lon = double(wvc_lon)*0.01;
        % make speed and direction into doubles
        wind_speed_selection = double(wind_speed_selection) * 0.01;
        wind_dir_selection = double(wind_dir_selection) * 0.01;
        % direction is degrees from N (presume clockwise)
        u = wind_speed_selection .* sin(wind_dir_selection/180*pi);
        v = wind_speed_selection .* cos(wind_dir_selection/180*pi);
        
        % get the rev number
        tmp = hdfread(fname,'rev_number');
        tmp = tmp{1}';
        revnum = strread(tmp,'int\n1\n%d');
        
        % format output structure
        out.revnum = revnum;
        out.lat = wvc_lat;
        out.lon = wvc_lon;
        out.u = u;
        out.v = v;
        out.spd = wind_speed_selection;
        out.dir = wind_dir_selection;
        out.timestr = char(wvc_row_time{1}');
        out.lasttimestr = char(wvc_lastrow_time{1}');
        out.ati1 = rowstoread(1);
        out.nati = length(rowstoread);
        
        switch size(wvc_lat,2)
            case 152
                out.res = 12.5;
            case 72
                out.res = 25;
        end
        
%         fprintf('%s%s, lat/lon=(%.3f, %.3f, %.3f, %.3f), time=%s, numrows: %d\n', ...
%             name,ext, boxlat, boxlon, char(wvc_row_time{1}'),length(rowstoread))
        
        % figure out how many cells actually fall in the box
        fprintf('%s%s, lat/lon=(%.3f, %.3f, %.3f, %.3f), time=%s, numrows: %d\n, numcells: %d\n', ...
            name,ext, boxlat, boxlon, char(wvc_row_time{1}'),length(rowstoread),length(indbox))
        
        out.numcells = length(indbox);
        out.numrows = length(rowstoread);
        
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
        %%%%% CONVERT TO U AND V, AND PREPARE OUTPUT %%%%%
        %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    end
    
end
