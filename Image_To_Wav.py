#pip install numpy scipy Pillow

import numpy as np
from scipy.io.wavfile import write
from PIL import Image

def main():
    inputfile = input("Input image (.jpg, .png): ")
    outputfile = input("Output WAV: ")

    # Open the image
    image = Image.open(inputfile)
    pixels = np.array(image)

    # Parameters
    sampling_rate = 48000
    samples_per_pixel = 100
    max_rgb = 255 * 3  # Max possible value of the sum of RGB

    # Prepare wav_samples array
    wav_samples = []
    pixel_A_RGB = None

    sign = 1  # Start with positive values
    # Iterate through the image pixels
    for row in pixels:
        for pixel in row:
            if pixel_A_RGB is None:  # If it's the first pixel
                pixel_A_RGB = sum(pixel[:3])  # Get the sum of RGB values
                continue
            
            # Get the next pixel RGB sum
            pixel_B_RGB = sum(pixel[:3])

            # Linear interpolation between pixels
            pixel_samples = np.linspace(pixel_A_RGB, pixel_B_RGB, samples_per_pixel)

            # Scale and append to wav_samples
            multiplier = 32767 / max_rgb * 0.95
            wav_samples.extend(pixel_samples * multiplier * sign)

            # Prepare for next iteration
            pixel_A_RGB = pixel_B_RGB

            sign *= -1  # Flip the sign for the next pixel

    # Convert to 16-bit integers
    wav_samples = np.array(wav_samples, dtype=np.int16)

    # Write the .wav file
    write(outputfile, sampling_rate, wav_samples)
    print("WAV file has been created successfully.")

if __name__ == "__main__":
    main()
