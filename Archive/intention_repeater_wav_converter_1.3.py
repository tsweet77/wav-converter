#!/usr/bin/python
# -*- coding: utf-8 -*-

# Intention Repeater WAV Converter created by Thomas Sweet.
# Converts any file <~20MB or less into a 96kHz Mono WAV File.
# Defaults to a 10 Minute WAV file.
# Updated 10/30/2020 v1.3
# Requires Python v3.5.3 or greater.
# Run using Windows: python intention_repeater_wav_converter.py
# Run using Linux: python3 intention_repeater_wav_converter.py
# Automated Example Linux: python3 intention_repeater_wav_converter.py "HH:MM:SS" "IN Filename" "OUT WAV Filename"
# Automated Example Windows: intention_repeater_wav.py "HH:MM:SS" "IN Filename" "OUT WAV Filename"
# The HH:MM:SS determines how long you want your WAV file to be. Default is 10 Minutes.
# Intention Repeater is powered by a Servitor (20 Years / 2000+ hours in the making)
# Servitor Info: https://web.archive.org/web/20200915191532/https://enlightenedstates.com/2017/04/07/servitor-just-powerful-spiritual-tool/
# Website: https://www.intentionrepeater.com/
# Forum: https://forums.intentionrepeater.com/
# Licensed under GNU General Public License v3.0
# This means you can modify, redistribute and even sell your own modified software, as long as it's open source too and released under this same license.
# https://choosealicense.com/licenses/gpl-3.0/

import time
import sys
import wave
import struct

VOLUME_LEVEL = 0.950 # Volume level should be from 0.000 to 1.000. Set at 95% to prevent possible clipping.
SAMPLING_RATE = 96000 #Mono Hz Sampling Rate.
DEFAULT_DURATION = "00:10:00" #Default length of the WAV file. HH:MM:SS Format.

def bytes_format(B):
   B = float(B)
   KB = float(1024)
   MB = float(KB ** 2) # 1,048,576
   GB = float(KB ** 3) # 1,073,741,824
   TB = float(KB ** 4) # 1,099,511,627,776

   if B < KB:
      return B
   elif KB <= B < MB:
      return '{0:.2f} k'.format(B/KB)
   elif MB <= B < GB:
      return '{0:.2f} M'.format(B/MB)
   elif GB <= B < TB:
      return '{0:.2f} G'.format(B/GB)
   elif TB <= B:
      return '{0:.2f} T'.format(B/TB)
      
def human_format(B):
   B = float(B)
   KB = float(1000)
   MB = float(KB ** 2) # 1,048,576
   GB = float(KB ** 3) # 1,073,741,824
   TB = float(KB ** 4) # 1,099,511,627,776

   if B < KB:
      return B
   elif KB <= B < MB:
      return '{0:.2f} Thousand'.format(B/KB)
   elif MB <= B < GB:
      return '{0:.2f} Million'.format(B/MB)
   elif GB <= B < TB:
      return '{0:.2f} Billion'.format(B/GB)
   elif TB <= B:
      return '{0:.2f} Trillion'.format(B/TB)

def getTimeFromSeconds(seconds): 
    seconds = seconds % (24 * 3600) 
    hour = seconds // 3600
    seconds %= 3600
    minutes = seconds // 60
    seconds %= 60

    return "%02d:%02d:%02d" % (hour, minutes, seconds)

sys.stdout.write('Intention Repeater WAV Converter v1.3 software created by Thomas Sweet.\n')
sys.stdout.write('Converts and repeats any file (<~20MB) to a 96kHz Mono WAV.\n')
sys.stdout.write('This software comes with no guarantees or warranty of any kind and is for entertainment purposes only.\n\n')

args = list(sys.argv)

try:
    duration_param = str(args[1])
    in_filename_param = str(args[2])
    out_filename_param = str(args[3])
except:
    duration_param = DEFAULT_DURATION
    in_filename_param = ''
    out_filename_param = ''

string_to_write = ''
string_to_write_value = ''
in_filename = ''
out_filename = ''
# duration_sec = '00'
# duration_minutes = '00'
# duration_hours = '00'

#Find number of seconds is in the provided duration of the output WAV file.
duration_sec = str(duration_param[6:8])
duration_minutes = str(duration_param[3:5])
duration_hours = str(duration_param[0:2])
duration_seconds = int(duration_hours) * 3600 + int(duration_minutes) * 60 + int(duration_sec)
total_samples = duration_seconds * SAMPLING_RATE

peak_value = 32767
peak_value_2 = peak_value * 2

if in_filename_param == '':
    while in_filename == '':
        in_filename = input('Input Filename: ')
else:
    in_filename = in_filename_param

if out_filename_param == '':
    while out_filename == '':
        out_filename = input('Output Filename (.wav): ')
else:
    out_filename = out_filename_param

if str.lower(out_filename[-4:]) != '.wav':
    if str.lower(out_filename[-1:]) == '.':
        out_filename += 'wav'
    else:
        out_filename += '.wav'

try:
    with open(in_filename, 'rb') as file:
        string_to_write_value = file.read()
except:
    print("Error Opening File!")
    quit()

minval = peak_value_2
maxval = -peak_value_2

for element in string_to_write_value:
    if element <= minval:
        minval = element

for element in string_to_write_value:
    if element >= maxval:
        maxval = element

widthval = maxval - minval

if widthval == 0:
    print("File has no data!")
    quit()

multiplier = peak_value_2 / widthval

num_writes = 0
num_seconds = 0

print("Press CTRL-C to stop running.\n")

obj = wave.open(out_filename, 'wb')
obj.setnchannels(1)  # mono
obj.setsampwidth(2)
obj.setframerate(SAMPLING_RATE)

# We write to the WAV file repeatedly until file length is reached.
try:
    while True:
        sample_num = 0
        num_seconds += 1
        while sample_num != total_samples:
            for element in string_to_write_value:
                sample_num += 1
                seconds = int(sample_num / SAMPLING_RATE)
                normalized_element = int((element * multiplier - peak_value) * VOLUME_LEVEL)
                value = normalized_element
                if value < -peak_value:
                    value = -peak_value
                elif value > peak_value:
                    value = peak_value
                
                data = struct.pack('<h', value)
                obj.writeframesraw(data)
                if sample_num % 25000 == 0:
                    sys.stdout.write('Status: [' + str(int(sample_num/total_samples*100)) + '%] (' + bytes_format(sample_num*2) + 'B / ' + bytes_format(total_samples*2) + 'B): ' + in_filename + ' -> ' + out_filename + '     \r' )
                    sys.stdout.flush()
                if sample_num == total_samples:
                    sys.stdout.write('Status: [' + str(int(sample_num/total_samples*100)) + '%] (' + bytes_format(sample_num*2) + 'B / ' + bytes_format(total_samples*2) + 'B): ' + in_filename + ' -> ' + out_filename + '     \r' )
                    sys.stdout.write('\nInput Filename: ' + in_filename + '\n')
                    sys.stdout.write('Output Filename: ' + out_filename + '\n')
                    sys.stdout.write('Num Times File Repeated: ' + str(int(num_writes)) + '\n')
                    sys.stdout.write('Num Samples Written: ' + str(human_format(total_samples*2)) + '\n')
                    sys.stdout.flush()
                    obj.close()
                    quit()
            num_writes += 1
            sys.stdout.write('Status: [' + str(int(sample_num/total_samples*100)) + '%] (' + bytes_format(sample_num*2) + 'B / ' + bytes_format(total_samples*2) + 'B): ' + in_filename + ' -> ' + out_filename + '     \r' )

        sys.stdout.write('\nInput Filename: ' + in_filename + '\n')
        sys.stdout.write('Output Filename: ' + out_filename + '\n')
        sys.stdout.write('Num Times File Repeated: ' + str(int(num_writes)) + '\n')
        sys.stdout.write('Num Samples Written: ' + str(human_format(total_samples*2)) + '\n')
        sys.stdout.flush()
        obj.close()
        sys.stdout.flush()
        quit()
        
except KeyboardInterrupt:

    pass

sys.stdout.write('Status: [' + str(int(sample_num/total_samples*100)) + '%] (' + bytes_format(sample_num*2) + ' / ' + bytes_format(total_samples*2) + '): ' + in_filename + ' -> ' + out_filename + '     \n' )
sys.stdout.write('Input Filename: ' + in_filename + '\n')
sys.stdout.write('Output Filename: ' + out_filename + '\n')
sys.stdout.write('Num Times File Repeated: ' + human_format(num_writes) + '\n')
sys.stdout.write('Num Samples Written: ' + human_format(total_samples*2) + '\n')
obj.close()
sys.stdout.flush()

obj.close()
