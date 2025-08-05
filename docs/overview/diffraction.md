# Diffraction Modelling

In geometric acoustics, diffraction can be modelled using diffraction paths that travel from the source to the receiver via edges of the room geometry.

<div align="center">
  <img src="../images/DiffractionZones.png" alt="Zones around a diffracting edge" width="600"/>
  <br>
  <em>Figure 1: Zones around a diffracting edge.</em>
</div>

## Models

RoomAcoustiC++ implements multiple diffraction models.

- **Attenuate:** Attenuates the sound based on the length of the diffraction path. It is a simple model that does not consider the angle of incidence or frequency-dependent effects.

- **LPF:** Applies a 1st order low-pass filter at 1kHz to diffraction paths. It is a simple model that does not consider the angle of incidence.

- **UDFA:** The Universal Diffraction Filter Approximation[^1] uses multiple 1st order shelving IIR filters to model the low-pass filter response of diffraction paths.

- **UDFA-I:** A simplified version of UDFA that only models diffraction in the shadow zone.

- **NNBest and NNSmall:** Neural network-based models[^2] that approximate the frequency reponse of diffraction paths using a 2nd order IIR filter.

- **UTD:** The Uniform Theory of Diffraction[^3] (UTD) model implemented using a Linkwitz-Riley filterbank[^4].

- **BTM:** The Biot-Tolstoy-Medwin-Svensson[^5] (BTMS) model is a physically accurate model implemented using an FIR filter.

---

[^1]: Kirsch C, and Ewert S. "A universal filter approximation of edge diffraction for geometrical acoustics." IEEE/ACM Trans. Audio, Speech, and Lang. Proc., 31:1636-1651, 2023
[^2]: Mannall J, et al. "Efficient diffraction modeling using neural networks and infinite impulse response filters." J. Audio Eng. Soc., 71:566-576, 2023
[^3]: Kouyoumjian R, and Pathak P. "A uniform geometrical theory of diffraction for an edge in a perfectly conducting surface." Proc. IEEE, 62:1448-1461, 1974
[^4]: Schissler C, et al. "High-order diffraction and diffuse reflections for interactive sound propagation in large environments." ACM Trans. Graphics, 39:1-12, 2014
[^5]: Svensson P, et al. "An analytic secondary source model of edge diffraction impulse responses." J. Acoust. Soc. Am., 106:2331–2344, 1999