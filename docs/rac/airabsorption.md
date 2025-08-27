# Air Absorption

As sound travels through the air it loses energy, primarily at high frequencies.
The greater the distance between the source and listener the greater the attenuation.

## Model

Air absorption is implemented as a 1st order IIR filter[^1] with coefficients $b0 = e ^ {\frac{-d \cdot f_s} {7782 \times c}}$
and $a1 = b0 - 1$, where $d$ is the distance the sound has traveled through the air, $f_s$ is the sampling frequency and $c$ is the speed of sound in air.
This allows the distance to be interpolated at runtime and the filter parameters updated.

---

<div align="center">
  <img src="../images/AirAbsorption.png" alt="Air absorption 1st order IIR filter" width="500"/>
  <br>
  <em>Figure 1: Air absorption 1st order IIR filter.</em>
</div>

---

[^1]: Grimm G, et al. "Implementation and perceptual evaluation of a simulation method for coupled rooms in higher order ambisonics," Proc. of the EAA Joint Symposium on Auralization and Ambisonics, Berlin, Germany. 27-32, 2014.
