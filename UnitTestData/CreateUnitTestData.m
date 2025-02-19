
%% BTM

clear all
close all

fs = 48e3;
nfft = 8192;
temperature = 20.0;
c = 331.5 + 0.6 * temperature;
controlparameters = struct('fs', fs, 'nfft', nfft, 'difforder', 1, 'c', c, 'saveFiles', 2, 'noDirect', true);

zW = [2.5,	10,	    7,	    5,	    2];
tW = [270,	320,	350,	212,	290];
tS = [45,	10,	    2,	    60,	    33];
tR = [226,	318,	207,	198,	200];
rS = [1,	2,	    7,	    3.5,	2.7];
rR = [1,	3.1,	4,	    2,	    1.8];
zS = [1,	3.4,	3.2,	2,	    1.7];
zR = [1,	4.2,	2.1,	2.5,	1.9];

irOut = [];
for i = 4:length(zW)
    ir = SingleWedge(zW(i), tW(i), tS(i), tR(i), rS(i), rR(i), zS(i), zR(i), controlparameters, false);
    idx = find(ir.diff1 ~= 0, 1);
    storeIr{i} = ir.diff1(idx:end - 1);
    irOut = [irOut; ir.diff1(idx:end - 1)];
end

parameters = [zW; tW; tS; tR; rS; rR; zS; zR];

writematrix(parameters, 'diffractionPaths.csv')
writematrix(irOut, 'btm.csv')

%% Genelec Directivity

clear all
close all

y = readmatrix("C:\Users\jmann\OneDrive - University of Surrey\Documents\Sound PhD\Year 2\BRAS Database\2_source_and_receiver_descriptions-Genelec_8020c\2 Source and receiver descriptions\Genelec 8020c\Genelec8020c_1x1_64442_IR_front_pole.csv");
yText = readtable("C:\Users\jmann\OneDrive - University of Surrey\Documents\Sound PhD\Year 2\BRAS Database\2_source_and_receiver_descriptions-Genelec_8020c\2 Source and receiver descriptions\Genelec 8020c\Genelec8020c_1x1_64442_IR_front_pole.csv");

positions = yText.Var1;
data = y(:,2:end);

clear y yText;

%%

azimuths = str2double(extractBetween(positions, 'P', 'T'));
elevations = str2double(extractAfter(positions, 'T'));

[AZ, EL] = meshgrid(unique(azimuths), unique(elevations));
AZ = [AZ, AZ(:,1)];
EL = [EL, EL(:,1)];

az = AZ(1,:);
el = EL(:,1);

[idx, magRes] = deal(zeros(size(AZ)));
for i = 1:length(positions)
    a = azimuths(i);
    e = elevations(i);

    idxA = az == a;
    idxE = el == e;
    idx(idxE, idxA) = i;
end
idx(1,:) = 1;
idx(end,:) = length(positions);

%X = cosd(EL);
%Y = sind(AZ) .* sind(EL);
%Z = cosd(AZ) .* sind(EL);

% Parameters
fs = 44100;    % Sampling frequency (Hz)
N = size(data, 2);

% Frequency bands (in Hz), for example: center frequencies of octave bands
freqBands = [62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];  % Adjust as needed
numFreqBands = length(freqBands);  % Number of frequency bands to analyze
targets = cell(numFreqBands, 1);
a_lm = cell(numFreqBands, 1);

% Precompute FFT for all impulse responses
fftResponses = fft(data, N, 2);  % Perform FFT along time axis (3rd dimension)
frequencies = (0:N-1) * (fs / N);  % Frequency vector

magnitudeResponse = zeros(length(positions), numFreqBands);

% Loop over frequency bands
for k = 1:numFreqBands
    % Get the center frequency for the current band
    centerFreq = freqBands(k);
    
    % Define a frequency band around the center frequency (e.g., octave band)
    fLower = centerFreq / sqrt(2);  % Lower bound of the band
    fUpper = centerFreq * sqrt(2);  % Upper bound of the band
    
    % Find indices of frequencies within this band
    bandIdx = find(frequencies >= fLower & frequencies <= fUpper);
    
    % Compute the magnitude of the frequency response in this band
    magnitudeResponse(:,k) = mean(abs(fftResponses(:, bandIdx)), 2);  % Mean across band
end

% Normalise
magnitudeResponse = magnitudeResponse / max(sum(magnitudeResponse, 2), [], 'all') * numFreqBands;

for k = 1:numFreqBands
    store = magnitudeResponse(:,k);
    targets{k} = store(idx);
end

phi = deg2rad(AZ);
theta = deg2rad(EL);

% Parameters
lMax = 3;  % Maximum degree of spherical harmonics (adjust as needed)
numTheta = 181;  % 181
numPhi = 361;      % 361

for l = 0:lMax
    for m = -l:l
        % Initialize the sum for this (l, m) pair
        Y_lmStore{l^2 + l + m + 1} = zeros(numTheta, numPhi);
        
        % Calculate spherical harmonic for each grid point
        for i = 1:numTheta
            for j = 1:numPhi
                Y_lmStore{l^2 + l + m + 1}(i, j) = spharm(l, m, theta(i, j), phi(i, j));  % Compute Y_lm(theta, phi)
            end
        end
    end
end

sinTheta = sin(theta);  % same size as Theta
for k = 1:numFreqBands
    f = targets{k};
    fReconstructed = zeros(numTheta, numPhi);
    % Loop over all (l, m) pairs to compute spherical harmonic coefficients
    for l = 0:lMax
        for m = -l:l
            % Initialize the sum for this (l, m) pair
            Y_lm = Y_lmStore{l^2 + l + m + 1};
    
            % Compute the integral using numerical integration
            integrand = f .* conj(Y_lm) .* sinTheta;
            % Integrate over theta (rows) and phi (columns) using trapz
            integral_theta = theta(2,2) * trapz(integrand, 1);  % integrate over theta (dim=1)
            a_lm{k}(l^2 + l + m + 1) = phi(2,2) * trapz(integral_theta, 2);  % integrate over phi (dim=2)
    
            fReconstructed = fReconstructed + (a_lm{k}(l^2 + l + m + 1) .* Y_lm);
        end
        error = mean(abs(mag2db(f) - mag2db(abs(fReconstructed))), 'all');
        if (error < 0.5)
            disp(['End early: ', num2str(l)]);
            break;
        end
    end
end
%%
a = deg2rad(0:10:350);
e = deg2rad(0:10:180);
count = 0;

dir = zeros(length(a) * length(e), numFreqBands);
input = zeros(2, length(a) * length(e));
for i = 1:length(a)
    for j = 1:length(e)
        count = count + 1;
        input(:,count) = [e(j), a(i)];
        dir(count,:) = CalculateDirectivity(a_lm, a(i), e(j));
    end
end

writematrix(freqBands, 'directivityFreq.csv');
writematrix(input, 'genelecDirectivityInput.csv');
writematrix(dir, 'genelecDirectivityOutput.csv');


function dir = CalculateDirectivity(harmonics, azimuth, elevation)
    dir = zeros(1, length(harmonics));
    for k = 1:length(harmonics)
        order = sqrt(length(harmonics{k})) - 1;
        for l = 0:order
            for m = -l:l
                Y_lm = spharm(l, m, elevation, azimuth);
                dir(k) = dir(k) + (harmonics{k}(l^2 + l + m + 1) .* Y_lm);
            end
        end
    end
    dir = abs(dir);
end

function Ylm = spharm(l, m, theta, phi)
    % Calculate the spherical harmonic Y_l^m(theta, phi)
    % Using MATLAB's built-in `legendre` function for associated Legendre polynomials
    
    % Normalization constant for spherical harmonics
    C = sqrt((2*l+1)/(4*pi) * factorial(l-abs(m))/factorial(l+abs(m)));

    % Compute the associated Legendre polynomial
    P_lm = legendre(l, cos(theta), 'unnorm');
    
    % Extract the m-th order component from the output
    P_lm_m = P_lm(abs(m)+1);
    E = exp(-1i*m*phi);        
    Ylm = C * P_lm_m .* E;
end