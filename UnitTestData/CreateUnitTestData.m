
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