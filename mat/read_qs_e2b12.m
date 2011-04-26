function e2b = read_qs_e2b12( filename )

e2b_size = [ 152, 3248];

fid = fopen( filename, 'r' );

lat = fread( fid, e2b_size, 'float' );
lon = fread( fid, e2b_size, 'float' );
spd = fread( fid, e2b_size, 'float' );
dir = fread( fid, e2b_size, 'float' );

fclose( fid );

e2b.lat = lat;
e2b.lon = lon;
e2b.spd = spd;
e2b.dir = dir;
e2b.u   = e2b.spd .* sind(e2b.dir);
e2b.v   = e2b.spd .* cosd(e2b.dir);

e2b.lon(e2b.lon>180)=e2b.lon(e2b.lon>180)-360;

return
end

