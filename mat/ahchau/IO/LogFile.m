classdef LogFile < handle
    % classdef LogFile 
    % this is a little class to handle writing to both the screen and a
    % file at the same time
    % Created by A. Chau 1/17/12
    % Edited by AHChau 3/13/13 Allow passing in empty filename to not write
    % to file
    
    properties
        fid
        filename
    end
    
    methods
        function obj = LogFile(filename,varargin)
            % function obj = LogFile(filename,varargin)
            % first optional argument is the permission string passed into
            % fopen!
            % e.g.  logfile = LogFile('test.txt','a') will open for
            % appending
            p=inputParser;
            p.addOptional('permstring','wt',@ischar);
            p.parse(varargin{:});
            
            obj.filename = filename;
            if ~isempty(obj.filename)
                obj.fid = fopen(filename,p.Results.permstring);
                fprintf('Opened %s for writing with ''%s''...\n', ...
                    filename, p.Results.permstring);
            else
                fprintf('Will print to screen only!\n');
            end
        end
        
        function PrintScreenLog(obj,str)
            fprintf(str);
            if ~isempty(obj.filename)
                fprintf(obj.fid,str);
            end
        end
        
        function PrintLogOnly(obj,str)
            if ~isempty(obj.filename)
                fprintf(obj.fid,str);
            end
        end
        
        function PrintScreenLogToc(obj,varargin)
            if nargin>1
                toctmp = toc(varargin{1});
            else
                toctmp = toc;
            end
            obj.PrintScreenLog(sprintf('Elapsed time is %.6f seconds.\n',toctmp));
        end
        
        function delete(obj)
            if ~isempty(obj.filename)
                fclose(obj.fid);
                fprintf('Closed %s.\n',obj.filename);
            end
        end
    end % methods
end