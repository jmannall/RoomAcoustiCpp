# Creating an audio listener

Each scene should only have one `RACAudioListener`.
To create a new audio listener add the `RACAudioListener` component to a GameObject in your scene.
It should be placed at the position of the player's camera (first person perspective) or head (third person perspective) as the transform of this GameObject will be used to position the audio listener in 3D space.
This will also add an `AudioListener` component if there is not one already.

Only one `AudioListener` is allowed per scene.
If an `AudioListener` is already present on a different GameObject, you will need to remove it before adding the `RACAudioListener`.