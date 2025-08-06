# Installation

## Usage

To install and set up the project, run the following commands:

```powershell
# Navigate to the Unity project Assets directory 
cd Assets

# Clone the repository
git clone https://github.com/jmannall/RoomAcoustiCpp-Unity.git

# Create a StreamingAssets directory if it doesn't exist
mkdir StreamingAssets

# Navigate into the project directory
cd RoomAcoustiCpp-Unity

# Copy the StreamingAssets folder into the Assets folder
.\utilities\copyStreamingAssets.ps1
```

To undo the copy of the StreamingAssets folder (in order to commit any changes) use:

```powershell
# Copy the StreamingAssets folder into the Assets folder
.\utilities\undoCopyStreamingAssets.ps1
```

The demo scene uses the new Unity Input System. If you are using a Unity 5 or older you will need to install it through the Unity package manager: [installation guide](https://docs.unity3d.com/Packages/com.unity.inputsystem@1.11/manual/Installation.html).

## Binaural Processing

RAC uses the [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit) (3DTI) for HRTF processing. Currently only the .3dti-hrtf file format is supported.
If a custom HRTF file is desired, the 3DTI toolkit provides a [SOFATo3DTI converter](https://github.com/3DTune-In/3dti_AudioToolkit/releases/download/M20221031/HRTF_SOFATo3DTI.zip).
It can be used as follows: `HRTF_SOFATo3DTI -i <SOFA file> -o <3dti-hrtf file>`.
The SOFA file should have the interaural time delay (ITD) removed and left and right ear delays stored in the Delay variable of the SOFA file.

## External content distributed together with this software

A KEMAR head HRTF is provided [https://publications.rwth-aachen.de/record/807373](https://publications.rwth-aachen.de/record/807373).

It has been processed to match the expected file format for the 3DTI Toolkit.
The ITDs were extracted[^1] using the following matlab code:

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

Audio files "EBU_FemaleSpeech", "EBU_MaleSpeech" and "EBU_PopMusic" are from the EBU Sound Quality Assessment Material (SQAM) CD [https://tech.ebu.ch/publications/sqamcd](https://tech.ebu.ch/publications/sqamcd).

[^1]: Andreopoulou A, et al. "Identification of perceptually relevant methods of inter-aural time difference estimation." J. Acoust. Soc. Am., 142:588-598, 2017