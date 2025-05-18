clear all
close all

data = load("C:\Users\jm01527\OneDrive - University of Surrey\Documents\Sound PhD\Year 2\BRAS Database\2_source_and_receiver_descriptions-Genelec_8020c\2 Source and receiver descriptions\Genelec 8020c\Genelec8020_DAF_2016_1x1_64442_IR_front_pole.mat");
fs = 44100;
N = 4096;

[data.DTF, data.CTF] = HRTFtoDTF(data.IR, fs);
%%
tf.IR = abs(fft(data.IR, N, 1));
tf.DTF = abs(fft(data.DTF, N, 1));
tf.CTF = abs(fft(data.CTF, N, 1));

HRTFMax = max(tf.IR, [], 'all');
DTFMax = max(tf.DTF, [], 'all');
CTFMax = max(tf.CTF, [], 'all');

[value, idx] = max(tf.IR, [], 'all');

posIdx = floor(idx / N) + 1;
freqIdx = idx - (posIdx - 1) * N;
check = tf.IR(freqIdx, posIdx);

normaliseIR = 1 / tf.IR(idx);
normaliseDTF = 1 / tf.DTF(idx);
normaliseCTF = 1 / tf.CTF(freqIdx);
tf.IR = tf.IR / tf.IR(idx);
tf.DTF = tf.DTF / tf.DTF(idx);
tf.CTF = tf.CTF / tf.CTF(freqIdx);

[normalise, iIR] = max(mean(tf.IR, 1), [], 'all');
tf.IR = tf.IR / normalise;
tf.DTF = tf.DTF / normalise;

normaliseIR = normaliseIR / normalise;
normaliseDTF = normaliseDTF / normalise;

[normalise, iDTF] = max(mean(tf.DTF, 1), [], 'all');
normalise = mean(tf.DTF(:,iIR));

% normaliseCTF = mean(tf.CTF);
% normaliseDTF = normaliseDTF * normaliseCTF;
% 
% tf.CTF = tf.CTF / normaliseCTF;
% tf.DTF = tf.DTF * normaliseCTF;

tf.CTF = tf.CTF * normalise;
tf.DTF = tf.DTF / normalise;

normaliseCTF = normaliseCTF * normalise;
normaliseDTF = normaliseDTF / normalise;

HRTFMax2 = max(tf.IR, [], 'all');
DTFMax2 = max(tf.DTF, [], 'all');
CTFMax2 = max(tf.CTF, [], 'all');

tfmag.IR = mag2db(tf.IR);
tfmag.DTF = mag2db(tf.DTF);
tfmag.CTF = mag2db(tf.CTF);

fvec = (0:N-1) * (fs / N);  % Frequency vector

%%
%close all

figure
semilogx(fvec, tfmag.CTF)
hold on
semilogx(fvec, tfmag.DTF(:,iIR))
semilogx(fvec, tfmag.IR(:,iIR))
semilogx(fvec, tfmag.DTF(:,iIR) + tfmag.CTF, '--')
grid on
legend('CTF', 'DTF', 'Original', 'Sum')
xlim([20 20e3])

%%
close all

freqBands = [62.5, 125, 250, 500, 1000, 2000, 4000, 8000, 16000];  % Adjust as needed
irStore = data.IR;
dtfStore = data.DTF;

data.IR = irStore * normaliseIR;
[a_lm.IR, f.IR] = CalculateSphericalHarmonics(data, freqBands, fs, false);
[a_lm.IR_Norm, f.IR_Norm] = CalculateSphericalHarmonics(data, freqBands, fs, true);

data.IR = dtfStore * normaliseDTF;
[a_lm.DTF, f.DTF] = CalculateSphericalHarmonics(data, freqBands, fs, false);
[a_lm.DTF_Norm, f.DTF_Norm] = CalculateSphericalHarmonics(data, freqBands, fs, true);

data.IR = irStore;
data.DTF = dtfStore;

%%
a = deg2rad(10:10:350);
e = deg2rad(10:10:180);

dir = zeros(length(a) * length(e) + 1, numFreqBands);
input = zeros(2, length(a) * length(e) + 1);

input(:,1) = [0, 0];
dir(1,:) = CalculateDirectivity(a_lm.DTF_Norm, 0, 0);
count = 1;

for i = 1:length(a)
    for j = 1:length(e)
        count = count + 1;
        input(:,count) = [e(j), a(i)];
        dir(count,:) = CalculateDirectivity(a_lm.DTF_Norm, a(i), e(j));
    end
end

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

%%
writematrix(freqBands, 'directivityFreq.csv');
writematrix(input, 'genelecDirectivityInput.csv');
writematrix(dir, 'genelecDTFDirectivityOutput.csv');

%%
N = 4096;
numFreqBands = 9;
fftResponses = tf.CTF;  % Perform FFT along time axis
frequencies = (0:N-1) * (fs / N);  % Frequency vector
magnitudeResponse = [];
% Loop over frequency bands
for k = 1:numFreqBands
    % Get the center frequency for the current band
    centerFreq = freqBands(k);
    
    % Define a frequency band around the center frequency (e.g., octave band)
    fLower = centerFreq / sqrt(2);  % Lower bound of the band
    fUpper = centerFreq * sqrt(2);  % Upper bound of the band
    
    % Find indices of frequencies within this band
    bandIdx = frequencies >= fLower & frequencies <= fUpper;
    
    % Compute the magnitude of the frequency response in this band
    magnitudeResponse(:,k) = mean(abs(fftResponses(bandIdx, :)), 1);  % Mean across band
end
correction = mean((mag2db(magnitudeResponse)' + mag2db(cellfun(@(x) x(1,1), f.DTF_Norm)) - mag2db(cellfun(@(x) x(1,1), f.IR_Norm))));
magnitudeResponse = magnitudeResponse' / db2mag(correction);

tf.CTF_Norm = tf.CTF / db2mag(correction);
tfmag.CTF_Norm = tfmag.CTF - correction;

%%



figure
semilogx(freqBands, mag2db(cellfun(@(x) x(1,1), f.IR)))
hold on
semilogx(freqBands, mag2db(cellfun(@(x) x(1,1), f.DTF)))
semilogx(freqBands, mag2db(cellfun(@(x) x(1,1), f.IR_Norm)))
semilogx(freqBands, mag2db(cellfun(@(x) x(1,1), f.DTF_Norm)))
semilogx(freqBands, mag2db(magnitudeResponse))
semilogx(freqBands, mag2db(magnitudeResponse) + mag2db(cellfun(@(x) x(1,1), f.DTF_Norm)))

grid on
legend('IR', 'DTF', 'IR Norm', 'DTF Norm', 'CTF', 'Sum')
xlim([20 20e3])

figure
semilogx(fvec, tfmag.DTF(:,posIdx))
hold on
semilogx(fvec, tfmag.IR(:,posIdx))
semilogx(fvec, tfmag.CTF)
semilogx(fvec, tfmag.CTF_Norm)
semilogx(fvec, tfmag.CTF + tfmag.DTF(:,posIdx), '--')
grid on
legend('DTF', 'Original', 'CTF', 'CTF Correction', 'DTF + CTF')

xlim([20 20e3])

%%

nfft = 4096*2;
fs = 48000;

fvecDefault = CreateFvec(fs, nfft);


%% Directories and files

disp('Directories and files')

directory.arHeadphones = 'C:\Users\jm01527\OneDrive - University of Surrey\Documents\Sound PhD\Year 3\ARExperiment\';

file.arHeadphones = 'ARHeadphoneEQFilter.mat';

%%

load([directory.arHeadphones file.arHeadphones], 'mat_hp')

ir.ARHeadphones = mat_hp.filter;
[tfmag.ARHeadphones, tf.ARHeadphones] = IrToTf(ir.ARHeadphones, nfft);

%%

[tfmag.Genelec, tf.Genelec] = IrToTf(ir.Genelec, nfft);

figure
semilogx(fvec, tfmag.DTF(:,posIdx))
hold on
semilogx(fvec, tfmag.CTF)
semilogx(fvec, tfmag.CTF_Norm)
semilogx(fvec, tfmag.CTF + tfmag.DTF(:,posIdx), '--')
grid on
legend('DTF', 'CTF', 'CTF Correction', 'DTF + CTF')
xlim([20 20e3])

figure
semilogx(fvec, tfmag.Genelec)
hold on
semilogx(fvec, tfmag.ARHeadphones)
semilogx(fvec, tfmag.ARHeadphones_Genelec)
semilogx(fvec, tfmag.CTF_Norm, '--')
semilogx(fvec, mag2db(abs(tf.CTF_Norm .* tf.ARHeadphones)), '--')
grid on
legend('CTF Filter', 'AR_L', 'AR_R', 'AR CTF L', 'AR CTF R', 'CTF Ref', 'AR L ref', 'AR R ref')
xlim([20 20e3])

%% Calculate FIR filters

disp('Calculate FIR filters')

fvecNorm = 2 * fvecDefault / fs;
filterLength = 4096;

ir.ARHeadphones_Genelec = zeros(filterLength, 2);
ir.Genelec = firls(filterLength - 1, fvecNorm, abs(tf.CTF_Norm))';
ir.ARHeadphones_Genelec(:,1) = firls(filterLength - 1, fvecNorm, abs(tf.CTF_Norm .* tf.ARHeadphones(:,1)))';
ir.ARHeadphones_Genelec(:,2) = firls(filterLength - 1, fvecNorm, abs(tf.CTF_Norm .* tf.ARHeadphones(:,2)))';

ir.Genelec = AKphaseManipulation(ir.Genelec, fs, 'min_phase', 6);
ir.ARHeadphones_Genelec(:,1) = AKphaseManipulation(ir.ARHeadphones_Genelec(:,1), fs, 'min_phase', 6);
ir.ARHeadphones_Genelec(:,2) = AKphaseManipulation(ir.ARHeadphones_Genelec(:,2), fs, 'min_phase', 6);

[tfmag.Genelec, tf.Genelec] = IrToTf(ir.Genelec, nfft);
[tfmag.ARHeadphones_Genelec, tf.ARHeadphones_Genelec] = IrToTf(ir.ARHeadphones_Genelec, nfft);

clear filterLength

%%

WriteBinaryFile(ir.Genelec, 'GenelecCTF.bin');
WriteBinaryFile(ir.ARHeadphones, 'ARHeadphoneEQ.bin');
WriteBinaryFile(ir.ARHeadphones_Genelec, 'ARHeadphoneEQ_GenelecCTF.bin');

function WriteBinaryFile(ir, fileName)

    [irLength, channels] = size(ir);
    irLength = int32(irLength);
    
    if (channels == 1)
        ir = [ir, ir];
        channels = 2;
    end

    if channels ~= 2
        disp('Unsupported number of channels')
        return
    end

    ir = reshape(ir', [], 1);

    fileID = fopen(fileName, 'w');
    
    fwrite(fileID, irLength, 'int32');
    
    fwrite(fileID, ir, 'single');
    
    % Close the file
    fclose(fileID);
end