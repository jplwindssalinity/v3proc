function out = ExtractNestedField(s,fnames)
% function out = ExtractNestedField(s,fnames)
% s is the structure
% fieldnames is a cell array containing the nest structure. the last
% element is the one to extract
% flattens
%
% A. Chau 2/9/10
%% debugging during development
% s = A;
% fnames = {'truth','v'}
%%
stmp = s(1);
stop = 0; q = 1; noexist = 0;
while ~stop
    % check that field exists
    if isfield(stmp,fnames(q))
        stmp = stmp.(fnames{q});
    else 
        stop = 1;
        noexist = 1;
    end
    if q == length(fnames)
        stop=1;
    end
    q = q+1;
end

%%
if noexist
    out = [];
    error('field does not exist!')
else
    % prepare a variable
    out = zeros([size(stmp),length(s)]);
    
    for qq = 1:length(s)
        stop = 0; q = 1;
        stmp = s(qq);
        while ~stop
            % check that field exists
            if isfield(stmp,fnames(q))
                stmp = stmp.(fnames{q});
            else
                stop = 1;
            end
            if q == length(fnames)
                stop=1;
            end
            q = q+1;
        end
        
        out(:,:,qq) = stmp;
    end
end

out = squeeze(out);

end