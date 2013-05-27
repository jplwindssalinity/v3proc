function e2b = read_qs_e2b12( filename )
% function e2b = read_qs_e2b12( filename )
% from email from Alex Fore 12/2/10
%
% A. Chau 12/14/10 added filename and transposed the arrays

e2b_size = [ 152, 3248];

fid = fopen( filename, 'r' );

lat = fread( fid, e2b_size, 'float' );
lon = fread( fid, e2b_size, 'float' );
spd = fread( fid, e2b_size, 'float' );
dir = fread( fid, e2b_size, 'float' );

fclose( fid );

e2b.lat = lat';
e2b.lon = lon';
e2b.spd = spd';
e2b.dir = dir';
e2b.u   = e2b.spd .* sind(e2b.dir);
e2b.v   = e2b.spd .* cosd(e2b.dir);

e2b.lon(e2b.lon>180)=e2b.lon(e2b.lon>180)-360;

e2b.flag = [];
e2b.wvcsize = [];
e2b.filename = filename;
e2b.source = sprintf('QuikSCATL2B_AF-ECMWF');

return
end

