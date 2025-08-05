# Image Edge Model

The Image Edge Model[^1] is a geometric acoustic approach for simulating early reflections and diffraction in room acoustics.  
It extends the classic Image Source Model[^2] by incorporating edge diffraction, enabling more realistic spatial audio rendering in complex environments.

---

<div align="center">
  <img src="../images/ImageEdgeModel.png" alt="Image edge model" width="600"/>
  <br>
  <em>Figure 1: Image Edge Model diagram showing source, listener, and image sources.</em>
</div>

---

## Early Reflections

Early reflections are modeled by treating each wall as a mirror that creates virtual "image sources".
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

---

## Diffraction

Real rooms contain obstacles and edges that bend sound waves, producing diffraction.
The Image Edge Model augments the image source approach by creating virtual "image edges".
These are reflected in the same way as image sources and can be used to create diffraction paths from image sources.
This allows the model to locate sound paths that include both diffraction and reflections.
RoomAcoustiC++ currently only supports 1st-order diffraction but an extension to higher orders is planned.

## Configuration

The Image Edge Model can be configured with the following parameters:

- **Direct sound**: none, doCheck (Check for obstructions), alwaysTrue (Do not check for obstructions).
- **Reflection Order:** The maximum order paths including only reflections.
- **Shadow Diffraction Order:** The maximum order of paths including shadowed diffraction.
- **Specular Diffraction Order:** The maximum order of paths including non-shadowed edge diffraction.
- **Minimum Edge Length:** The minimum length of edges to consider for diffraction.

---

[^1]: Erraji A, et al. "The image edge model." Acta Acust., 2021, 5:1-15, 2021.
[^2]: Borish J. "Extension of the image model to arbitrary polyhedra." J. Acoust. Soc. Am., 75:1827–1836, 1984.