
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
for i = 1:length(zW)
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

csvData = readtable("C:\BRASDatabase\2_source_and_receiver_descriptions-Genelec_8020c\2 Source and receiver descriptions\Genelec 8020c\Genelec8020c_1x1_64442_IR_front_pole.csv");

data.IR = csvData{:,2:end}';
data.Phi = str2double(extractBetween(csvData{:,1}, 'P', 'T'));
data.Theta = str2double(extractAfter(csvData{:,1}, 'T'));

fs = 44100;
freqBands = [62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];  % Adjust as needed

[a_lm, ~, ~, invDF] = CalculateSphericalHarmonics(data, freqBands, fs, true);

[data.IR, ctf] = HRTFtoDTF(data.IR, fs);
a_lm_dtf = CalculateSphericalHarmonics(data, freqBands, fs, true);

out = a_lmToText(a_lm);
out_dtf = a_lmToText(a_lm_dtf);

%% QSC K8 Directivity

clear all
close all

csvData = readtable("C:\BRASDatabase\2_source_and_receiver_descriptions-QSC_K8\2 Source and receiver descriptions\QSC K8\QSCK8_1x1_64442_IR_front_pole.csv");

data.IR = csvData{:,2:end}';
data.Phi = str2double(extractBetween(csvData{:,1}, 'P', 'T'));
data.Theta = str2double(extractAfter(csvData{:,1}, 'T'));

fs = 44100;
freqBands = [62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];  % Adjust as needed

[a_lm, ~, ~, invDF] = CalculateSphericalHarmonics(data, freqBands, fs, true);

[data.IR, ctf] = HRTFtoDTF(data.IR, fs);
a_lm_dtf = CalculateSphericalHarmonics(data, freqBands, fs, true);

out = a_lmToText(a_lm);
out_dtf = a_lmToText(a_lm_dtf);

%%
a = deg2rad(0:10:350);
e = deg2rad(0:10:180);
count = 0;

dir = zeros(length(a) * length(e), numFreqBands);
input = zeros(2, length(a) * length(e));
for i = 2:length(a)
    for j = 2:length(e)
        count = count + 1;
        input(:,count) = [e(j), a(i)];
        dir(count,:) = CalculateDirectivity(a_lm, a(i), e(j));
    end
end

%%
writematrix(freqBands, 'directivityFreq.csv');
writematrix(input, 'genelecDirectivityInput.csv');
writematrix(dir, 'genelecDirectivityOutput.csv');

%%
writematrix(freqBands, 'directivityFreq.csv');
writematrix(input, 'qscDirectivityInput.csv');
writematrix(dir, 'qscDirectivityOutput.csv');

function dir = CalculateDirectivity(harmonics, azimuth, elevation)
    dir = zeros(1, length(harmonics));
    for k = 1:length(harmonics)
        order = sqrt(length(harmonics{k})) - 1;
        for l = 0:order
            for m = -l:l
                Y_lm = harmonicY(l, m, elevation, azimuth);
                dir(k) = dir(k) + (harmonics{k}(l^2 + l + m + 1) .* Y_lm);
            end
        end
    end
    dir = abs(dir);
end

function out = a_lmToText(a_lm)
    for i = 1:length(a_lm)
        out{i} = '{ ';
        for j = 1:length(a_lm{i})
            aReal = real(a_lm{i}(j));
            aImag = imag(a_lm{i}(j));
            if (aImag == 0)
                out{i} = [out{i} num2str(a_lm{i}(j), 16) ','];
            else
                out{i} = [out{i} 'Complex(' num2str(aReal, 16) ', ' num2str(aImag, 16) '),'];
            end
        end
        out{i} = [out{i} ' },'];
    end
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

    E = exp(1i*abs(m)*phi);
    if m < 0
        % Include phase factor for negative m
        E = conj(E);
        if mod(m,2) == 1
            E = -E;
        end
    end

    Ylm = C * P_lm_m .* E;
end

%%
l = 2;
m = 2;
theta = 0.2;
phi = 0.8;

test = spharm(l, m, theta, -phi);
test1 = harmonicY(l, m, theta, phi);