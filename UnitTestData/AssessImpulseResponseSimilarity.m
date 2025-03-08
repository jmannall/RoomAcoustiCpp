function ret = AssessImpulseResponseSimilarity(currentIR, targetIR, fs)
    % Assess the similarity between two impulse responses in time, frequency, and decay domains.
    %
    % Parameters:
    %   ir1 - First impulse response (vector)
    %   ir2 - Second impulse response (vector)
    %   fs  - Sampling frequency (scalar)

    if length(currentIR) ~= length(targetIR)
        error('Impulse responses must have the same length.');
    end

    if (currentIR == targetIR)
        ret = [1, 0 0];
        return;
    end
    
    t = (0:length(currentIR)-1) / fs; % Time vector
    NFFT = 2^nextpow2(length(currentIR)); % FFT length
    f = linspace(0, fs/2, NFFT/2+1); % Frequency vector
    
    % --- 1. Time Domain Similarity ---
    % Normalized cross-correlation
    xc = xcorr(currentIR, targetIR, 0, 'coeff'); % Zero-lag correlation
    fprintf('Normalized Cross-Correlation: %.4f\n', xc);
    
    % --- 2. Frequency Domain Similarity ---
    currentFFT = abs(fft(currentIR, NFFT));
    targetFFT = abs(fft(targetIR, NFFT));
    
    currentFFT_db = 20*log10(currentFFT(1:NFFT/2+1));
    targetFFT_db = 20*log10(targetFFT(1:NFFT/2+1));
    
    % Compute magnitude difference
    freq_diff = abs(currentFFT_db - targetFFT_db);
    mean_freq_diff = mean(freq_diff);
    fprintf('Mean Magnitude Spectrum Difference: %.2f dB\n', mean_freq_diff);
    
    % --- 3. Energy Decay Curve Similarity ---
    currentEDC = cumtrapz(currentIR(:,end:-1:1).^2); % decay curve
    targetEDC = cumtrapz(targetIR(:,end:-1:1).^2); % decay curve

    currentEDC = currentEDC(:,end:-1:1);
    targetEDC = targetEDC(:,end:-1:1);

    currentEDC_db = 10.*log10(currentEDC); % put into dB
    targetEDC_db = 10.*log10(targetEDC); % put into dB

    currentEDC_db = currentEDC_db-max(currentEDC_db); % normalise to max 0
    targetEDC_db = targetEDC_db-max(targetEDC_db); % normalise to max 0

    if any(isinf(currentEDC_db))
        currentEDC_db = currentEDC_db(:,1:find(isinf(currentEDC_db),1,'first')-1); % remove trailing infinite values
    end
    if any(isinf(targetEDC_db))
        targetEDC_db = targetEDC_db(:,1:find(isinf(targetEDC_db),1,'first')-1); % remove trailing infinite values
    end
        
    t_currentEDC = (0:length(currentEDC_db)-1) / fs; % Time vector
    t_targetEDC = (0:length(targetEDC_db)-1) / fs; % Time vector

    len = min(length(currentEDC_db), length(targetEDC_db));
    % Compute RMSE between EDCs
    EDC_rmse = sqrt(mean((currentEDC_db(1:len) - targetEDC_db(1:len)).^2));
    fprintf('EDC RMSE: %.2f dB\n', EDC_rmse);
    
    % --- Visualization ---
    figure;
    subplot(3,1,1);
    plot(t, currentIR, 'b', 'DisplayName', 'Current IR'); hold on;
    plot(t, targetIR, 'r', 'DisplayName', 'Target IR'); hold off;
    xlabel('Time (s)');
    ylabel('Amplitude');
    title('Impulse Responses in Time Domain');
    legend;
    grid on;
    
    subplot(3,1,2);
    semilogx(f, currentFFT_db, 'b', 'DisplayName', 'Current IR'); hold on;
    semilogx(f, targetFFT_db, 'r', 'DisplayName', 'Target IR'); hold off;
    xlabel('Frequency (Hz)');
    ylabel('Magnitude (dB)');
    title('Magnitude Spectrum');
    legend;
    grid on;
    
    subplot(3,1,3);
    plot(t_currentEDC, currentEDC_db, 'b', 'DisplayName', 'Current IR'); hold on;
    plot(t_targetEDC, targetEDC_db, 'r', 'DisplayName', 'Target IR'); hold off;
    xlabel('Time (s)');
    ylabel('Energy Decay (dB)');
    title('Energy Decay Curve (EDC)');
    legend;
    grid on;
    
    sgtitle('Impulse Response Comparison');
    
    % --- Threshold-based Decision ---
    % Define similarity thresholds (adjust as needed)
    if xc > 0.9 && mean_freq_diff < 2.0 && EDC_rmse < 3.0
        disp('Impulse responses are highly similar.');
    elseif xc > 0.8 && mean_freq_diff < 5.0 && EDC_rmse < 6.0
        disp('Impulse responses are somewhat similar.');
    else
        disp('Impulse responses show significant differences.');
    end
    ret = [xc, mean_freq_diff, EDC_rmse];
end
