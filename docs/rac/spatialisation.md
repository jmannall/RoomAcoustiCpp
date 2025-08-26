# Spatialisation

To create an immersive environment sources must be spatialised so that they are perceived to be located at a specific position in 3D space.
The [3D-TuneIn Toolkit](https://github.com/3DTune-In/3dti_AudioToolkit)[^1] (3DTI) is used to spatialise sources, image sources and reverb sources within RAC.
It provides three modes of operation: Quality, Performance and None.

## Quality (HRTF)

Quality mode applies a Head-Related Transfer Function[^2] (HRTF) based on the relative direction of a source compared to the listener.
A HRTF characterizes the sound at each of the listeners ears for a given incident direction.
It is unique to each individual and factors such as the shape of the head and ears will change the interaural level difference (ILD) and interaural time delay (ITD) between the two ears, as well as the frequency response.
This is applied as a convolution in the frequency domain using overlap-save and is therefore computationally intensive compared to other DSP in RAC that mainly use IIR filters.

3DTI use a custom binary file format to store and load HRTF data.
A correctly formatted KEMAR head HRTF is provided with the [Unity](../unity/installation.md/#external-content-distributed-together-with-this-software) interface.
Instructions on creating custom HRTF files can be found [here](../unity/hrtf.md).

## Performance (ITD, ILD)

Performance mode is recommended when computational resources are limited and some loss of localisation accuracy is acceptable.
It uses a simplified model where the ILDs are calculated from an averaged HRTF[^3] and then a spherical head model[^4] is used to apply ITDs and changes to low frequency ILDs at close distances.
This is applied using a 4th order IIR filter and is therefore significantly more efficient than the Quality mode.

## None (Mono)

When None is selected, no spatialisation techniques are applied and a mono signal is returned.
This mode is used when RAC is set to impulse response mode.

[^1]: Cuevas-Rodríguez M, et al. "3D Tune-In Toolkit: An open-source library for real-time binaural spatialisation," PLOS ONE, 14:1-37, 2018.
[^2]: Møller H, et al. "Head-related transfer functions of human subjects" J. Audio Eng. Soc., 43:300-321, 1995.
[^3]: Katz B, and Parseihian G. "Perceptually based head-related transfer function database optimization," J. Acoust. Soc. Am., 131:99–105, 2012.
3672641
[^4]: Duda R, and Martens W. "Range dependence of the response of a spherical head model," J. Acoust. Soc. Am., 104:3048–3058, 1998.