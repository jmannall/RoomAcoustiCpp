# Acoustic Model

RAC implements a hybrid geometric acoustic and feedback delay network model for simulating sound propagation in rooms.
It is based on the model proposed by Wendt et al.[^1] with modifications to support diffraction and non-shoebox rooms.

The Image Edge Model[^2] is used to simulate early reflections and diffraction.
The maximum reflection order and diffraction orders can be configured independently to control the complexity of the model.
The highest order image sources are used as inputs to the Feedback Delay Network (FDN) to simulate late reverberation.
The FDN is configured using the room dimensions and either Sabine or Eyring formulas to predict the frequency-dependent reverberation time.
Alternatively, the reverberation time can be set manually.

The following section provides an overview of the two key components of the acoustic model.

## Image Edge Model

The Image Edge Model[^2] is a geometric acoustic approach for simulating early reflections and diffraction in room acoustics.  
It extends the classic Image Source Model[^3] by incorporating edge diffraction, enabling more realistic spatial audio rendering in complex environments.

---

<div align="center">
  <img src="../images/ImageEdgeModel.png" alt="Image edge model" width="600"/>
  <br>
  <em>Figure 1: Image Edge Model diagram showing source, listener, and image sources.</em>
</div>

---

Early reflections are modeled by treating each wall as a mirror that creates virtual **image sources**.
For each reflection order, the position of the image source is calculated by mirroring the real source across the room's surfaces.
The listener receives sound from both the direct source and its images, with each path having a distinct delay, attenuation, and direction.
For each reflection path, the received signal at time ($t$) is given by:
$$
y(t) = \frac{(h * x)\left[t - \frac{r}{c}\right]}{r},
$$
where

- $h[\cdot]$ is a frequency-dependent filter,  
- $x[\cdot]$ is the anechoic input signal,  
- $r$ is the distance from the receiver,  
- $c$ is the speed of sound,
- $*$ denotes convolution.

In arbitrary polyhedral rooms, each image source must be checked for valid intersections and then visibility to the listener.

Real rooms contain obstacles and edges that bend sound waves, producing diffraction.
The Image Edge Model augments the image source approach by creating virtual **image edges**.
These are reflected in the same way as image sources and can be used to create diffraction paths from image sources.
This allows the model to locate sound paths that include both diffraction and reflections.
RAC currently only supports 1st-order diffraction but an extension to higher orders is planned.

The Image Edge Model can be configured with the following parameters:

- **Direct sound**: None, Check for obstructions, Always visible (i.e do not check for obstructions).
- **Reflection Order**: The maximum order paths including only reflections.
- **Shadow Diffraction Order**: The maximum order of paths including shadowed diffraction.
- **Specular Diffraction Order**: The maximum order of paths including non-shadowed edge diffraction.
- **Minimum Edge Length**: The minimum length of edges to consider for diffraction.

---

## Feedback Delay Network

Feedback Delay Networks[^4] (FDNs) are used to mimic the natural decay of sound for late reverberation by controlling the decay time and frequency response of late reverberation.
They use a recursive structure with multiple delay lines allowing for efficient processing independent of the reverberation time.

---

<div align="center">
  <img src="../images/FeedbackDelayNetwork.png" alt="Feedback Delay Network" width="600"/>
  <br>
  <em>Figure 1: Feedback Delay Network diagram. Each path represents multiple delay lines and A is a mixing matrix that controls the feedback between delay lines.</em>
</div>

---

The absorption filters are used to control the frequency dependent decay time based on the decay of each delay line.
The output of the FDN is spatialised by placing reverb sources around the the listener and applying the current spatialisation mode.
The reflection filters are set to the absorption of the nearest wall in the direction of the reverb source linked to each respective FDN channel.
This simulates directional late reverberation, for example, if the listener is beside an absorbative wall, less reverberation will be heard from that direction.

The FDN can be configured with the following parameters:

- **Matrix:** Householder, RandomOrthogonal
- **Room dimensions:** The dimensions of the room in meters (for a shoebox room this would be the width, height, depth). These determine the delay line lengths.
- **Number of delay lines/reverb sources:** Determined the number of delay lines in the FDN and the number of reverb sources. A larger number increases the computational cost of the FDN but can improve the quality of the reverberation.

---

[^1]: Wendt T, et al. "A computationally-efficient and perceptually-plausible algorithm for binaural room impulse response simulation." J. Audio. Eng. Soc., 62:748-766, 2014.
[^2]: Erraji A, et al. "The image edge model." Acta Acust., 2021, 5:1-15, 2021.
[^3]: Borish J. "Extension of the image model to arbitrary polyhedra." J. Acoust. Soc. Am., 75:1827–1836, 1984.
[^4]: Jot J, and Chaigne A. "Digital delay networks for designing artificial reverberators." Proc. Audio Eng. Soc. Conv., Paris, France. 1-12, 1991.