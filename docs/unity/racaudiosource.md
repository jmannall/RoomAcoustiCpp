# Creating an audio source

To create a new audio source add the `RACAudioSource` component to a GameObject in your scene.
The transform of this GameObject will be used to position the audio source in 3D space.
This will also add an `AudioSource` component if there is not one already.

The `RACAudioSource` component has several properties that can be adjusted in the inspector to customize the audio source's behavior, including the audio clip, gain, loop, playOnAwake and directivity settings.
These will override any equivalent properties set on the Unity `AudioSource` component when the game starts.

- Audio clip: The audio clip to play.
- Gain: The gain (volume) of the audio source.
- Loop: Whether the audio source should loop.
- Play On Awake: Whether the audio source should start playing automatically when the scene starts.
- Directivity: The directivity pattern of the audio source.

## Changing the audio clip

The easiest way to assign an audio clip to a `RACAudioSource` is to use the Inspector.

1. Save a new audio asset (.wav, .mp3) in your Unity project.
2. Select the GameObject with the `RACAudioSource` component and drag the new `AudioClip` to the audio clip slot in the Inspector.

## Play the audio source

The simplest way to play a source is to set the `Play On Awake` property to true in the inspector.
This will make the audio source start playing automatically when the scene starts.

## Source directivity

A number of directivity patterns are provided:

- **Common polar patterns**: Omni, Subcardioid, Cardioid, Supercardioid, Hypercardioid, Bidirectional
- **Loudspeaker directivities**: Genelec8020c, QSC_K8
- **Normalised loudspeaker directivities**: Genelec8020cDTF (Removes the common transfer function of the loudspeaker, not recommended for general use)

The directivity can be modified at runtime using the dropdown menu in the `RACAudioSource` component.

## Controlling the audio source using C# scripts

For more experienced users the `RACAudioSource` component can be controlled using C# scripts.
To play the audio source, you can call the `Play` function on the `RACAudioSource` component.
For example:

```csharp
RACAudioSource audioSource;

void Start()
{
    audioSource = GetComponent<RACAudioSource>();
    audioSource.Play();
}
```

Similarly, you can control the audio source by calling the following functions:

```csharp
RACAudioSource audioSource;

audioSource.Play();       // Play the audio source. This creates the source in RAC if it is not already initialised
audioSource.Pause();      // Pause the audio source
audioSource.PlayPause();  // Toggle play/pause
audioSource.Stop();       // Stop the audio source. This removes the source from RAC
audioSource.Restart();    // Restart the audio source in RAC

double time = AudioSettings.dspTime + 1.0;
audioSource.PlayScheduled(time);    // Play the current clip in 1 second

AudioClip clip;
audioSource.PlayOneShot(clip);    // Play the assigned audio clip once
audioSource.SetClip(clip);        // Change the current audio clip

bool isPlaying = audioSource.IsPlaying();   // Return true if the audio source is playing

audioSource.UpdateGain(-3.0f);      // Update the gain (in dB) of the audio source
audioSource.SetLoop(true);          // Set the audio source to loop
audioSource.SetPlayOnAwake(true);   // Set the audio source to play on awake
```

