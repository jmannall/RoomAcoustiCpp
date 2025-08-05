# Feedback Delay Network (FDN)

FDNs are used to mimic the natural decay of sound for late reverberation.
The can be used to control the decay time and frequency response of the reverberation.
They use a recursive structure with multiple delay lines allowing for efficient processing independent of the reverberation time.

---

<div align="center">
  <img src="../images/FeedbackDelayNetwork.png" alt="Feedback Delay Network" width="600"/>
  <br>
  <em>Figure 1: Feedback Delay Network diagram. Each path represents multiple delay lines and A is a mixing matrix that controls the feedback between the delay lines.</em>
</div>

---

The absorption filters are used to control the frequency dependent decay time based on the decay of each delay line.
The output of the FDN is spatialised by placing reverb sources around the the listener and applying the current spatialisation mode.
The reflection filters are set to the absorption of the nearest wall in the direction of the reverb source linked to each respective FDN channel.
This simulates directional late reverberation, for example, if the listener is beside an absorbative wall, less reverberation will be heard from that direction.

## Configuration

The FDN can be configured with the following parameters:

- **Matrix:** Householder, RandomOrthogonal
- **Room dimensions:** The dimensions of the room in meters (for a shoebox room this would be the width, height, depth). These determine the delay line lengths.
- **Number of delay lines/reverb sources:** Determined the number of delay lines in the FDN and the number of reverb sources. A larger number increases the computational cost of the FDN but can improve the quality of the reverberation.