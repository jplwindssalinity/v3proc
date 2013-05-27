function out = GetQSParamsForSim(hdffile);
%function out = GetQSParamsForSim(hdffile);
% A Chau 4/8/10

% hdffile = 'QS_S1B47118.20081881808'
% hdffile = 'HDF/QS_S2B47118.20081890822.CP12';

% SEMI_MAJOR_AXIS
tmp = hdfread(hdffile,'orbit_semi_major_axis');
tmp = tmp{1}';
SEMI_MAJOR_AXIS = strread(tmp,'float\n1\n%f')/1000; % in km


% ECCENTRICITY
tmp = hdfread(hdffile,'orbit_eccentricity');
tmp = tmp{1}';
ECCENTRICITY = strread(tmp,'float\n1\n%f') ;

% INCLINATION
tmp = hdfread(hdffile,'orbit_inclination');
tmp = tmp{1}';
INCLINATION = strread(tmp,'float\n1\n%f') ;

% ORBIT_EPOCH_LON
tmp = hdfread(hdffile,'EquatorCrossingLongitude');
tmp = tmp{1}';
ORBIT_EPOCH_LON = strread(tmp,'float\n1\n%f'); 

% ORBIT_EPOCH_DATE
tmp = hdfread(hdffile,'EquatorCrossingDate');
tmp = tmp{1}';
tmp = strread(tmp,'char\n1\n%s') ;
[year,daynum] = strread(tmp{1},'%d-%d');
ORBIT_EPOCH_date = datestr(doy2date(daynum,year),29);

% ORBIT_EPOCH_TIME
tmp = hdfread(hdffile,'EquatorCrossingTime');
tmp = tmp{1}';
tmp = strread(tmp,'char\n1\n%s') ;
ORBIT_EPOCH_time = tmp{1};

ORBIT_EPOCH = sprintf('%sT%s',ORBIT_EPOCH_date,ORBIT_EPOCH_time);

% LONG_OF_ASC_NODE
tmp = hdfread(hdffile,'EquatorCrossingLongitude');
tmp = tmp{1}';
LONG_OF_ASC_NODE = strread(tmp,'float\n1\n%f');

% NUDGE_WINDFIELD_FILE
tmp = hdfread(hdffile,'ancillary_data_descriptors');
tmp = tmp{1}';
tmp = strread(tmp','%s\n');
tmp = tmp{5};
NUDGE_WINDFIELD_FILE = tmp;

out.hdfname = hdffile;
out.SEMI_MAJOR_AXIS = SEMI_MAJOR_AXIS;
out.ECCENTRICITY = ECCENTRICITY;
out.INCLINATION = INCLINATION;
out.ORBIT_EPOCH_LON = ORBIT_EPOCH_LON;
out.ORBIT_EPOCH_date = ORBIT_EPOCH_date;
out.ORBIT_EPOCH_time = ORBIT_EPOCH_time;
out.ORBIT_EPOCH = ORBIT_EPOCH;
out.LONG_OF_ASC_NODE = LONG_OF_ASC_NODE;
out.NUDGE_WINDFIELD_FILE = NUDGE_WINDFIELD_FILE;

%QS_S2B47118.20081890822.CP12, lat/lon=(21.080, 41.720, 205.940, 230.420), time=2008-188T13:56:17.968, numrows: 195
%, numcells: 5452