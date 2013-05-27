function OUT = GrabMetrics(speedrange,ret_types);
% function OUT = GrabMEtric(speedrange,ret_types)
%
% Created by A. Chau 7/27/09
% Modified by A. Chau 9/9/09 to make ret_types an input argument. this may
% break other things from July.
% ret_types = {'GS','S3'};


OUT = [];

% speedrange = [3 30];
OUT.speedrange=speedrange;

speedstr = [num2str(speedrange(1)) 'to' num2str(speedrange(end))];


% metstoget = {'sel_rms_dir_err','sel_rms_spd_err', ...
%     'sel_spd_bias','sel_dir_bias', ...
%     'near_rms_dir_err','near_rms_spd_err', ...
%     'near_dir_bias','near_spd_bias', 'skill','wvc'}

metstoget = {'sel_rms_dir_err','sel_rms_spd_err', ...
    'sel_spd_bias','near_rms_dir_err','near_rms_spd_err', ...
    'near_spd_bias', 'skill','wvc'};


for qq = 1:length(ret_types)
    ret_type = ret_types{qq};
    
    metroot = ['met' ret_type '_' speedstr '.'];
    
    for q = 1:length(metstoget)
        tmp = metstoget{q};
        fname = [metroot tmp];
        fid = fopen(fname);
        A = textscan(fid,'%f %f %f','commentstyle','@');
        fclose(fid);
        A(end+1)={tmp};
        OUT.(ret_type).(tmp)=A;
        
        %     figure(1)
        %     plot(GS.(tmp){1},GS.(tmp){2})
        %     title(['GS ' speedstr ' m/s, ' tmp])
        %     pause(1)
    end
end



