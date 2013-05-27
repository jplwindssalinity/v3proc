tags={'trnKatrina1_5km','trnKatrina2_5km','trnKatrina1_10km','trnKatrina2_10km','trnRita1_5km','trnRita2_5km','trnRita1_10km','trnRita2_10km'};
res={5,5,10,10,5,5,10,10};

for i=1:8
	inpt_s0meanvar_file=sprintf('jaxaAC_s0meanvar_%s.txt',tags{i});
        inpt_l2b_array_file=sprintf('jaxaAC_%s_l2b_arrays.dat',tags{i});
        outpt_testset_file=sprintf('jaxaAC_%s_trainset.dat',tags{i});
        outpt_latlon_file=sprintf('jaxaAC_%s_truth.spddat',tags{i});
        outpt_mleret_file=sprintf('jaxaAC_%s_mleret.spddat',tags{i});
% load original s0meanvar file for training case
mat0=load(inpt_s0meanvar_file,'-ASCII');
ati=mat0(:,1);
cti=mat0(:,2);
%load l2b_array file for each training case
[dummy1,dummy2,lat,lon,at1,nati,ncti]=read_l2b_array(inpt_l2b_array_file);

idx=(ati-at1)*ncti+cti+1;

lat=lat(idx);
lon=lon(idx);
tspd=mat0(:,28);
n=length(tspd);
rmschk=std(dummy1(idx)-tspd)

% write truth spddat file ( original speeds )
fid=fopen(outpt_latlon_file,'w','l');
fwrite(fid,n,'int32');  
fwrite(fid,lat,'float');  
fwrite(fid,lon,'float');  
fwrite(fid,tspd,'float');  
fwrite(fid,ati,'int32');  
fwrite(fid,cti,'int32');  
fclose(fid);

% write mleret spddat file ( original speeds )
fid=fopen(outpt_mleret_file,'w','l');
fwrite(fid,n,'int32');  
fwrite(fid,lat,'float');  
fwrite(fid,lon,'float');  
fwrite(fid,dummy1(idx),'float');  
fwrite(fid,ati,'int32');  
fwrite(fid,cti,'int32');  
fclose(fid);
debugfile=sprintf('debug_mleret_%s.png',tags{i});
plot_hurrspdcmp(outpt_latlon_file,outpt_mleret_file,debugfile);

% write testset output
X=mat0(:,[5,6,8,9,11,12,14,15,17,18,20,21,23,24,26,27,2]);   
if(res{i}==5)
X(:,17)=(X(:,17)-200)/200.0;
 else
X(:,17)=(X(:,17)-100)/100.0;
end
Y=mat0(:,28);

fid = fopen(outpt_testset_file,'w');
nsamps=length(Y);
fprintf(fid,'TYPE:STATIC\n#INPUTS:17\n#OUTPUTS:1\n#SAMPLES:%d\n',nsamps);
omat=[X';Y'];
fprintf(fid,'%g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g %g\n',omat);
fclose(fid);
end
