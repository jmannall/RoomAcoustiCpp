# Source Directivity

Sound sources can have various directivity patterns that describe how sound is radiated from the source.
For a given angle of incidence, the directivity pattern defines the gain (or attenuation) of the sound.

## Models

Directivity modelling is bundled with the graphic equaliser filters that also model wall reflections for image sources.
Therefore, any frequency dependent directivities are stored as spherical harmonics in octave frequency bands from 32Hz to 16kHz.
This can be used to calculate the gain coefficients for each frequency band and a given angle of incidence.

A number of directivity patterns are provided:

- **Common polar patterns**: Omni, Subcardioid, Cardioid, Supercardioid, Hypercardioid, Bidirectional
- **Loudspeaker directivities**: Genelec 8020c, QSC K8
- **Normalised loudspeaker directivities**: Genelec 8020c DTF (Removes the common transfer function of the loudspeaker, not recommended for general use)

Measurements from the [Benchmark for Room Acoustic Simulations](https://doi.org/10.14279/depositonce-6726.3) (BRAS) were used to create the Genelec 8020c and QSC K8 directivities.
The spherical harmonic decomposition was calculated for each frequency band from 32Hz to 16kHz.
The spherical harmonic order was chosen as the lowest one that resulted in an average reconstruction error of less than 0.5 dB.
This approach loses some of the frequency details in the measurements but is more efficient for real-time processing and creates a plausible directivity pattern.
An alternative aproach would be to apply source directivities using convolution.