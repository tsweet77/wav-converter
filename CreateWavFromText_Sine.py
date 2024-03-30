import wave
import numpy as np
from PIL import Image
import struct
import os
import math

def get_valid_filename(prompt):
    while True:
        filename = input(prompt)
        if not filename.endswith(".txt"):
            filename += ".txt"
        if os.path.exists(filename):
            return filename
        print("File not found. Please enter a valid filename.")

def main():
    channels = 0
    sampling_rate = 0
    amplitudewidth = 0
    samplesPerCharacter = 0
    text = None
    input_file_path = get_valid_filename("Enter Input Text File: ")
    with open(input_file_path, 'r', encoding='utf-8') as f:
        text = f.read()
    print("Read Input Data")
    print(f"Valid input received: {input_file_path}")

    # Find lowest and highest ASCII values
    min_ascii = min(ord(char) for char in text)
    max_ascii = max(ord(char) for char in text)
    # Find the range of ASCII values
    ascii_range = max_ascii - min_ascii

    while sampling_rate < 48000 or sampling_rate > 767500:
        sampling_rate_input = input("Enter Sampling Rate [Default 48000, Max 767500]: ")
        try:
            sampling_rate = int(sampling_rate_input)
        except ValueError:
            sampling_rate = 0
    print(f"Valid input received: {sampling_rate}")

    while channels < 1 or channels > 8:
        channels_input = input("Enter Channels (1-8): ")
        try:
            channels = int(channels_input)
        except ValueError:
            channels = 0
    print(f"Valid input received: {channels}")

    while amplitudewidth != 2 and amplitudewidth != 4:
        amplitudewidth_input = input("Enter Amplitude Width (2 or 4): ")
        try:
            amplitudewidth = int(amplitudewidth_input)
        except ValueError:
            amplitudewidth = 0
    print(f"Valid input received: {amplitudewidth}")

    while samplesPerCharacter < 1:
        samplesPerCharacter_input = input("Enter Samples/Character [Default 1]: ")
        try:
            samplesPerCharacter = int(samplesPerCharacter_input)
            if samplesPerCharacter < 1:
                samplesPerCharacter = 1
        except ValueError:
            samplesPerCharacter = 1
    print(f"Valid input received: {samplesPerCharacter}")

    output_file = "Output_sine_" + str(sampling_rate) + ".wav"

    # Open WAV file for writing
    sampleMax = (2147483647 if amplitudewidth == 4 else 32767)
    sampleMin = -(2147483647 if amplitudewidth == 4 else 32767)

    # Calculate the scaling factor
    scaling_factor = sampleMax / ascii_range

    sign = 1

    with wave.open(output_file, "w") as wav_file:
        # Set parameters for the WAV file
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(amplitudewidth)  # 32-bit audio or 16 bit audio
        wav_file.setframerate(sampling_rate)

        # Iterate over pixels and write audio samples
        frames_to_write = []
        for i in range(len(text) - 1):
            char = text[i]
            next_char = text[i + 1]

            # Convert pixel data to audio samples
            sample_current = int((ord(char) - min_ascii) * scaling_factor) * sign
            sign = -sign  # Alternate between positive and negative for each character
            next_sample = int((ord(next_char) - min_ascii) * scaling_factor) * sign

            # Create samplesPerCharacter steps from first pixel to the second
            for j in range(samplesPerCharacter):
                # Calculate the phase angle for the sine wave
                phase = 2 * math.pi * j / samplesPerCharacter

                # Generate the sine wave sample
                sample_new = int((sample_current + (next_sample - sample_current) * j / samplesPerCharacter) * math.sin(phase))
                #sample_new = int(sample_current + (next_sample - sample_current) * j / samplesPerCharacter)

                for _ in range(channels):
                    frames_to_write.append(struct.pack(('<i' if amplitudewidth == 4 else '<h'), sample_new))

            if len(frames_to_write) >= 1000:
                wav_file.writeframes(b"".join(frames_to_write))
                frames_to_write = []

        if frames_to_write:
            wav_file.writeframes(b"".join(frames_to_write))

    print("Audio file saved as " + output_file)

if __name__ == "__main__":
    main()