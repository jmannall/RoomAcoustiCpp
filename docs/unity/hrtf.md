# Custom HRTF

RAC uses the [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit) (3DTI) for HRTF processing. Currently only the .3dti-hrtf file format is supported.
If a custom HRTF file is desired, the 3DTI toolkit provides a [SOFATo3DTI converter](https://github.com/3DTune-In/3dti_AudioToolkit/releases/download/M20221031/HRTF_SOFATo3DTI.zip).
It can be used as follows: `HRTF_SOFATo3DTI -i <SOFA file> -o <3dti-hrtf file>`.
The SOFA file should have the interaural time delay (ITD) removed and left and right ear delays stored in the Delay variable of the SOFA file.

## Convert to minimum phase

One approach to remove ITD is to convert the HRTF to minimum phase[^1]. The following MATLAB code can be used:
```matlab
function [leftMinPhase, rightMinPhase, leftDelay, rightDelay] = ConvertToMinimumPhase(left, right, fs)
    fc = 3e3;
    threshold = db2mag(-30);
    
    leftLPF = lowpass(left,fc,fs);
    rightLPF = lowpass(right,fc,fs);
    
    leftOffset = max(leftLPF) * threshold;
    rightOffset = max(rightLPF) * threshold;
    
    leftDelay = find(leftLPF > leftOffset, 1, "first") - 1;
    rightDelay = find(rightLPF > rightOffset, 1, "first") - 1;

    [~, leftMinPhase] = rceps(left);
    [~, rightMinPhase] = rceps(right);
end
```

## Select the HRTF in Unity

Once created, the .3dti-hrtf file can be used in the Unity project:

1. Save the .3dt-hrtf file in the `Assets/StreamingAssets` folder of your Unity project.
2. On the `RACManager` script set the HRTF file to Custom.
3. Type the name of the .3dt-hrtf file (with the extension) in the text field.
4. Press play and check the console to ensure the custom HRTF has been loaded successfully.

[^1]: Andreopoulou A, and Katz B. "Identification of perceptually relevant methods of inter-aural time difference estimation." J.
Acoust. Soc. Amer., 142:588–598, 2017