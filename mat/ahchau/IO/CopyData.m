function CopyData(copyinfo)
% function CopyData(copyinfo)
%
% A. Chau 4/29/10


LocationDir = copyinfo.LocationDir;
ResultsDir = copyinfo.ResultsDir;
groups = copyinfo.groups;
dothecopy = copyinfo.dothecopy;
filetypestocopy = copyinfo.filetypestocopy;

currdir = pwd;

tic
for t = 1:length(groups)
    scenedirs = groups(t).scenedirs;
    rundirs = groups(t).rundirs;
    
    for s_count = 1:length(scenedirs)
        cd(LocationDir);
        copyfromdir = LocationDir;
        copytodir = ResultsDir;
        if ~exist(copytodir), mkdir(copytodir), end
        
        this_scene = scenedirs{s_count};
        if exist(this_scene,'dir')
            
            copyfromdir = fullfile(LocationDir,this_scene);
            copytodir = fullfile(ResultsDir,this_scene);
            if ~exist(copytodir), mkdir(copytodir), end
            cd(this_scene)
            % now in the scene directory
            
            
            for r_count = 1:length(rundirs);
                this_rundir = rundirs{r_count};
                if exist(this_rundir)
                    copyfromdir = fullfile(LocationDir,this_scene,this_rundir);
                    copytodir = fullfile(ResultsDir,this_scene,this_rundir);
                    if ~exist(copytodir), mkdir(copytodir), end
                    
                    cd(this_rundir)
                    
                    % now in the run dir
                    
                    
                    for filetypecount = 1:length(filetypestocopy);
                        filestocopy = dir(filetypestocopy{filetypecount});
                        for f_count = 1:length(filestocopy)
                            fname = fullfile(copyfromdir,filestocopy(f_count).name);
                            if dothecopy
                                copyfile(fname,copytodir);
                            else
                                fprintf('Will copy %s to %s\n',fname,copytodir);
                            end
                        end
                    end
                    %                     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    %                     % look for arrays.dat files %
                    %                     %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
                    %                     filestocopy = dir('*arrays.dat');
                    %                     for f_count = 1:length(filestocopy)
                    %                         fname = fullfile(copyfromdir,filestocopy(f_count).name);
                    %                         if dothecopy
                    %                             copyfile(fname,copytodir);
                    %                         else
                    %                             fprintf('Will copy %s to %s\n',fname,copytodir);
                    %                         end
                    %                     end
                
                    
                    
                    cd('..')
                else
                    fprintf('%s does not exist',this_rundir);
                end
                
            end
            
        else
            fprintf('%s does not exist',this_scene)
        end
    end
    
end
toc


cd(currdir);