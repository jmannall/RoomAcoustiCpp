# Installation

## Downloads and prerequisites

Install [Unity Hub](https://unity.com/download).

- Launch the Unity Hub and create a new project. This will prompt you to install a Unity editor if you have not already done so.

## Adding RoomAcoustiC++ to a Unity project

Copy the `RoomAcoustiCpp-Unity` folder into the `Assets` directory of your Unity project.

- Download the latest version of RoomAcoustiCpp-Unity from the [release section](https://github.com/jmannall/RoomAcoustiCpp-Unity/releases/latest), or clone the repository with the following command:
```powershell
# Navigate to the Unity project Assets directory 
cd Assets

# Clone the repository
git clone https://github.com/jmannall/RoomAcoustiCpp-Unity.git
```

## Copy RAC Assets

Copy the `StreamingAssets` and `Resources` folder from the `RoomAcoustiCpp-Unity` directory into the `Assets` directory of your Unity project.

If you are using Windows, you can open a powershell terminal and use the `copyRACAssets.ps1` and `undoCopyRACAssets.ps1` scripts provided in the utilities folder.
You will need to create the folder structure maually the first time the scripts are run.

## Version compatibility

The RoomAcousticC++ Unity interface has been developed and tested in Unity6 (last used version 6000.3.19f1).
It will probably still work in older versions of Unity, however Unity6 has made siginificant changes to the rendering pipeline.
Therefore, the textures in the demo scenes may not render correctly in older versions.

- For new projects Unity6 is recommended.

### Additional steps for Unity 2023 or older

The demo scene uses the new Unity Input System. If you are using a Unity 2023 or older you will need to install it through the Unity package manager: [installation guide](https://docs.unity3d.com/Packages/com.unity.inputsystem@1.11/manual/Installation.html).

## External content distributed together with this software

A KEMAR head HRTF is provided [https://publications.rwth-aachen.de/record/807373](https://publications.rwth-aachen.de/record/807373).

It has been processed to match the expected file format for the 3DTI Toolkit.
The ITDs were extracted[^1] using the following MATLAB code:

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