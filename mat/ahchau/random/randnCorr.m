function [r,g] = randnCorr(N,tau,sigma);
% function [r,g] = randnCorr(N,tau,sigma);
% r is the correlated Gaussian, g is the original uncorrelated Gaussian
%
% A. Chau 3/19/10


% as outlined in corr_gaussian_random.pdf
% http://www.cmu.edu/biolphys/deserno/pdf/corr_gaussian_random.pdf

% tau = 100;
% N=5000;


%% Correlated Gaussian 

f = exp(-1/tau);

g = randn(N,1)*sigma; % independent Gaussian realizations
r(1) =  g(1);
for q = 2:N
    r(q) = f*r(q-1) + sqrt(1-f^2)*g(q);
end
r=r(:); g=g(:);
