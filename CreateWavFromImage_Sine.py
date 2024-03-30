import wave
import numpy as np
from PIL import Image
import struct
import math

def main():
    channels = 0
    sampling_rate = 0
    amplitudewidth = 0
    samplesPerPixel = 0
    image = Image

    while True:
        input_image_path = input("Enter the path to the input image: ")
        try:
            image = Image.open(input_image_path)
            break
        except FileNotFoundError:
            print("Image does not exist. Please enter a valid path.")
    print(f"Valid input received: {image}")

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
        channels_input = input("Enter Amplitude Width (2 or 4): ")
        try:
            amplitudewidth = int(channels_input)
        except ValueError:
            amplitudewidth = 0
    print(f"Valid input received: {amplitudewidth}")

    while samplesPerPixel < 1:
        samplesPerPixel_input = input("Enter Samples/Pixel [Default 1]: ")
        try:
            samplesPerPixel = int(samplesPerPixel_input)
            if samplesPerPixel < 1:
                samplesPerPixel = 1
        except ValueError:
            samplesPerPixel = 1
    print(f"Valid input received: {samplesPerPixel}")

    # Resize the image by 90% using linear interpolation
    new_width = int(image.width * 0.10)
    new_height = int(image.height * 0.10)
    resized_image = image.resize((new_width, new_height), resample=Image.LANCZOS)

    # Get resized image data broken down into pixels
    pixels = list(resized_image.getdata())

    # Determine number of channels based on whether the image has alpha channel
    num_channels = 3 if resized_image.mode == "RGB" else 4
    output_file = "Output_Sine_" + str(sampling_rate) + ".wav"

    # Open WAV file for writing
    with wave.open(output_file, "w") as wav_file:
        # Set parameters for the WAV file
        wav_file.setnchannels(channels)
        wav_file.setsampwidth(amplitudewidth)  # 32-bit audio or 16 bit audio
        wav_file.setframerate(sampling_rate)

        # Iterate over pixels and write audio samples
        sign = 1
        for i in range(len(pixels) - 1):
            pixel = pixels[i]
            next_pixel = pixels[i + 1]

            # Convert pixel data to audio samples
            sample_R = int((pixel[0] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign # Red channel
            sample_G = int((pixel[1] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign  # Green channel
            sample_B = int((pixel[2] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign  # Blue channel

            sign = -sign

            next_sample_R = int((next_pixel[0] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign  # Red channel
            next_sample_G = int((next_pixel[1] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign  # Green channel
            next_sample_B = int((next_pixel[2] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767)) * sign  # Blue channel

            # Create samplesPerPixel steps from first pixel to the second
            for j in range(samplesPerPixel):
                phase = 2 * math.pi * j / samplesPerPixel
                sample_R = int(sample_R + (next_sample_R - sample_R) * j / samplesPerPixel)
                sample_G = int(sample_G + (next_sample_G - sample_G) * j / samplesPerPixel)
                sample_B = int(sample_B + (next_sample_B - sample_B) * j / samplesPerPixel)

                sample_R = int(sample_R * abs(math.sin(phase)))
                sample_G = int(sample_G * abs(math.sin(phase)))
                sample_B = int(sample_B * abs(math.sin(phase)))

                # Add alpha channel if present
                if num_channels == 4:
                    sample_A = int((pixel[3] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767))
                    next_sample_A = int((next_pixel[3] / 255.0) * (2147483647 if amplitudewidth == 4 else 32767))
                    sample_A = int(sample_A + (next_sample_A - sample_A) * j / samplesPerPixel)
                    sample_A = sign * sample_A
                    wav_file.writeframes(struct.pack('<4i', sample_R, sample_G, sample_B, sample_A))
                else:
                    wav_file.writeframes(struct.pack('<3i', sample_R, sample_G, sample_B))

    print("Audio file saved as " + output_file)

if __name__ == "__main__":
    main()