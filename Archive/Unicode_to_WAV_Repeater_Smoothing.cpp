/*
Unicode to WAV Repeater with Smoothing
by Anthro Teacher and Nathan
To Compile: g++ -O3 -Wall -static .\Unicode_to_WAV_Repeater_Smoothing.cpp -o .\Unicode_to_WAV_Repeater_Smoothing.exe -std=c++20
*/

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <filesystem>
#include <cstring>
#include <cmath>
#include <array>
#include <string>
#include <thread>
#include <algorithm>
using namespace std;
using namespace filesystem;
#ifdef _WIN64
#define BYTEALIGNMENTFORWAVPRODUCER 8
#else
#define BYTEALIGNMENTFORWAVPRODUCER 4
#endif
#include <bit>
#include <codecvt>
#include <locale>
/// ////////////////////////////////////////////START OF RIFF WAVE TAG ///////////////////////////////////////////////////////////////////////////
const uint32_t headerChunkSize = 0; /// PLACE HOLDER FOR RIFF HEADER CHUNK SIZE
/// /////////FORMAT CHUNK////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double volume_level = 0.99;                                                                                                                    /// VOLUME LEVEL///
uint16_t audioFormat = 1;                                                                                                                             /// 3 is float 1 is PCM                            /// AN UNKNOWN AT THIS TIME
uint16_t numChannels = 0;                                                                                                                             /// 8;                                                     /// NUMBER OF CHANNELS FOR OUR AUDIO I PRESUME 1 SAMPLE PER CHANNEL
uint32_t sampleRate = 0;                                                                                                                              ///  765000.0;                                       /// PRESUMABLY THE NUMBER OF SAMPLES PER SECOND
uint16_t bitsPerSample = 0;                                                                                                                           /// 16                                                     /// THE NUMBER OF BITS PER SAMPLE 2 BYTES // SAME AS AMPWIDTH
uint32_t byteRate = 0; //sampleRate * numChannels * bitsPerSample / 8;                                                                                     /// THE ABOVE COVERTED INTO BYTES
uint16_t blockAlign = 0; //numChannels * bitsPerSample / 8;                                                                                                /// NOT SURE YET PROBABLY ALIGNMENT PACKING TYPE OF VARIABLE
uint32_t formatSize = 0;//sizeof(audioFormat) + sizeof(numChannels) + sizeof(sampleRate) + sizeof(byteRate) + sizeof(blockAlign) + sizeof(bitsPerSample); /// FORMAT CHUNK SIZE
uint32_t sampleMax = 0; //(bitsPerSample == 16 ? 32767 : 2147483647);
uint32_t sampleMin = 0; //(bitsPerSample == 16 ? 32767 : 2147483647);
double frequency = 0.0;          /// Frequency To Play At
double smoothing = -1.0;         /// Interpolation Smoothing
uint32_t durationInSeconds = -1; /// 10                                                                                           ///DURATION OF THE WAV FILE
// uint32_t numOfDataCyclesToWrite = durationInSeconds * sampleRate * numChannels; /// THE NUMBER OF CYCLES WE COPY DATA INTO OUR DATA CHUNK
// vector<uint16_t> silenceSamples(numOfDataCyclesToWrite, 1);                     /// VECTOR OF EMPTY DATA FOR OUR SILENT WAV FILE
std::string inputFile = "", intentionOriginal = "", frequency_input = "0", smoothing_percent = "0", sampling_rate_input = "0";
int ascii_range = 0, min_ascii = 0, max_ascii = 0;
long long int numSamples = 0;
double M_PI = 3.141592653589793238462643383279502884197;
std::wstring intention = L"";
std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;

std::wstring utf8_to_wstring(const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring result;

    try {
        result = converter.from_bytes(str);
    } catch (const std::range_error& e) {
        // Handle invalid UTF-8 sequence
        //std::cerr << "Warning: Invalid UTF-8 sequence encountered. Skipping invalid characters." << std::endl;

        // Iterate over each byte of the input string
        for (std::string::const_iterator it = str.begin(); it != str.end(); ++it) {
            try {
                // Attempt to convert each byte individually
                std::string byte_str(1, *it);
                std::wstring converted = converter.from_bytes(byte_str);
                result += converted;
            } catch (const std::range_error& e) {
                // Skip the invalid byte and continue with the next one
                continue;
            }
        }
    }

    return result;
}
std::string wstring_to_utf8(const std::wstring& wstr)
{
    try {
        return conv.to_bytes(wstr);
    } catch (const std::range_error& e) {
        std::cerr << "Error: Invalid wide string sequence. " << e.what() << std::endl;
        return ""; // Return an empty string or a default value
    }
}

uint32_t utf8_codepoint(const wchar_t ch)
{
    if (ch < 0x80)
        return ch;
    else if (ch < 0x800)
        return ((ch >> 6) & 0x1F) | 0xC0;
    else if (ch < 0x10000)
        return ((ch >> 12) & 0x0F) | 0xE0;
    else if (ch < 0x110000)
        return ((ch >> 18) & 0x07) | 0xF0;
    else
        return '?'; // Return a default character for invalid code points
}
/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void addPadding(std::ofstream &file)
{
    int currentPosition = file.tellp();
    //std::cout << "Checking Pading Details Current Pos# " << currentPosition << std::endl;
    int remainder = currentPosition % BYTEALIGNMENTFORWAVPRODUCER;
    //std::cout << "Checking Pading Details Remainder# " << remainder << std::endl;
    if (remainder != 0)
    {
        int paddingSize = BYTEALIGNMENTFORWAVPRODUCER - remainder;
        std::cout << "Checking Pading Details Padding Size# " << paddingSize << std::endl;
        for (int i = 0; i < paddingSize; ++i)
        {
            const uint8_t paddingByte = 0;
            file.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
        }
    }
}
*/
std::string removeNonAlphanumeric(const std::string &str)
{
    std::string result;
    for (char c : str)
    {
        if (std::isalnum(c) || std::isspace(c))
        {
            result += c;
        }
    }
    return result;
}
void writeRiffHeader(ofstream &wavFile)
{
    wavFile.seekp(0, std::ios::beg);
    wavFile.write("RIFF", 4);                                                                 /// HEADER NAME FOR WAVE FILE ///                                         ///WRITING RIFF HEADER CALLING SIGN INTO FILE
    wavFile.write(reinterpret_cast<const char *>(&headerChunkSize), sizeof(headerChunkSize)); /// WRITING PLACEHOLDER FOR RIFF HEADER CHUNK SIZE INTO FILE
    // std::cout << "Checking Pading Details Current Pos# " << wavFile.tellp() << std::endl;
}
/// PRESUMES that when you call the function you are at end of file///
void writeRiffHeaderSizeElement(ofstream &wavFile)
{
    wavFile.seekp(4, ios::beg);
    const uint32_t realFileSizeMinusHeader = static_cast<uint32_t>(static_cast<int64_t>(wavFile.tellp()) - 8);
    wavFile.write(reinterpret_cast<const char *>(&realFileSizeMinusHeader), 4);
    wavFile.seekp(0, ios::end);
}
/// EXAMPLE: writeFormatHeader(wavFile,formatSize,audioFormat,numChannels,sampleRate,byteRate,blockAlign,bitsPerSample);
void writeFormatHeader(ofstream &wavFile, const uint32_t formatSize, const uint16_t audioFormat, const uint16_t numChannels, const uint32_t sampleRate, const uint32_t byteRate,
                       /*                                 */ const uint16_t blockAlign, const uint16_t bitsPerSample)
{
    wavFile.write("WAVEfmt ", 8);                                                         /// WRITING FORMAT CHUNK HEADER CALLING SIGN INTO FILE
    wavFile.write(reinterpret_cast<const char *>(&formatSize), sizeof(formatSize));       /// WRITING FORMAT CHUNK SIZE INTO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&audioFormat), sizeof(audioFormat));     /// WRITING FORMAT CHUNK ELEMENT - AUDIO FORMAT TO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&numChannels), sizeof(numChannels));     /// WRITING FORMAT CHUNK ELEMENT - NUMBER OF CHANNELS TO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&sampleRate), sizeof(sampleRate));       /// WRITING FORMAT CHUNK ELEMENT - SAMPLE RATE TO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&byteRate), sizeof(byteRate));           /// WRITING FORMAT CHUNK ELEMENT - BYTE RATE TO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&blockAlign), sizeof(blockAlign));       /// WRITING FORMAT CHUNK ELEMENT - BLOCK ALIGNMENT TO THE FILE
    wavFile.write(reinterpret_cast<const char *>(&bitsPerSample), sizeof(bitsPerSample)); /// WRITING FORMAT CHUNK ELEMENT - BITS PER SAMPLE TO THE FILE
    // std::cout << "Checking Pading Details Current Pos# " << wavFile.tellp() << std::endl;
    //     addPadding(wavFile);
}
void writeListSubChunk(ofstream &wavFile, const char *ID, const char *data, const uint32_t dataSize, const uint32_t isNullTerminated = 1)
{
    wavFile.write(ID, strlen(ID));                                                                                                              /// WRITE THE INFO CHUNK ID
    uint32_t alignedDataSize = (dataSize + BYTEALIGNMENTFORWAVPRODUCER - isNullTerminated) & ~(BYTEALIGNMENTFORWAVPRODUCER - isNullTerminated); /// CHECK FOR WORD ALIGNMENT FOR CHUNK
    wavFile.write(reinterpret_cast<const char *>(&alignedDataSize), sizeof(alignedDataSize));                                                   /// WRITE THE SIZE OF THE CHUNK
    wavFile.write(data, dataSize);                                                                                                              /// WRITE THE DATA OF THE CHUNK
    if (alignedDataSize == dataSize)
        alignedDataSize += BYTEALIGNMENTFORWAVPRODUCER; /// FOR LOOKS SAKE IN THE HEX EDITOR WE HAVE ALIGNMENT NO MATTER WHAT
    for (uint32_t i = dataSize; i < alignedDataSize; ++i)
    { /// IF DATA DOESNT END ON ALIGNMENT ADD PADDING
        const uint8_t paddingByte = 0;
        wavFile.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
    }
}

void writeListSubChunk(ofstream &wavFile, const char *ID, const std::string &data)
{
    wavFile.write(ID, strlen(ID));                                                                        /// WRITE THE INFO CHUNK ID
    const uint32_t dataSize = data.length() + 1;                                                          /// SIZE OF CHUNK TO BE ADDED DEFAULTLY
    uint32_t alignedDataSize = (dataSize + BYTEALIGNMENTFORWAVPRODUCER) & ~(BYTEALIGNMENTFORWAVPRODUCER); /// CHECK FOR WORD ALIGNMENT FOR CHUNK
    wavFile.write(reinterpret_cast<const char *>(&alignedDataSize), sizeof(alignedDataSize));             /// WRITE THE SIZE OF THE CHUNK
    wavFile.write(data.c_str(), data.length());                                                           /// WRITE THE DATA OF THE CHUNK
    if (alignedDataSize == dataSize)
        alignedDataSize += BYTEALIGNMENTFORWAVPRODUCER; /// FOR LOOKS SAKE IN THE HEX EDITOR WE HAVE ALIGNMENT NO MATTER WHAT
    for (uint32_t i = dataSize; i < alignedDataSize; ++i)
    { /// IF DATA DOESNT END ON ALIGNMENT ADD PADDING
        const uint8_t paddingByte = 0;
        wavFile.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
    }
}

uint32_t writeListHeader(ofstream &wavFile)
{
    wavFile.write("LIST", 4);                                                                 /// WRITING LIST HEADER ID
    const uint32_t listHeaderChunkSizePos = wavFile.tellp();                                  /// GETTING LIST HEADER SIZE ELEMENT POS FOR RETURN VALUE
    wavFile.write(reinterpret_cast<const char *>(&headerChunkSize), sizeof(headerChunkSize)); /// WRITING PLACEHOLDER SIZE ELEMENT
    return listHeaderChunkSizePos;                                                            /// RETURNING THE LIST HEADER SIZE ELEMENT POSITION
}
void writeListHeaderSizeChunkElement(ofstream &wavFile, const uint32_t position, const uint32_t value)
{
    wavFile.seekp(position, ios::beg);                        /// GOING BACK TO FILL IN LIST SIZE ///
    wavFile.write(reinterpret_cast<const char *>(&value), 4); /// FILLING IN LIST SIZE WITH PROPER NUMERATION ////
    wavFile.seekp(0, ios::end);                               /// SEEKING END OF FILE ///
}
///////////////PRELIMINARY DATA CHUNK VARIABLES/////////////////
int ord(char c)
{
    return static_cast<int>(c);
}
std::pair<uint32_t, uint32_t> findMinMaxASCII(const std::wstring &input)
{
    if (input.empty())
    {
        // Return the maximum and minimum int values if the string is empty
        return {std::numeric_limits<uint32_t>::max(), std::numeric_limits<uint32_t>::min()};
    }

    uint32_t minOrd = std::numeric_limits<uint32_t>::max();
    uint32_t maxOrd = std::numeric_limits<uint32_t>::min();

    for (wchar_t ch : input)
    {
        if (ch < 0x110000)
        {
            uint32_t codepoint = utf8_codepoint(ch);
            minOrd = std::min(minOrd, codepoint);
            maxOrd = std::max(maxOrd, codepoint);
        }
    }

    return {minOrd, maxOrd};
}
void writeDataChunk(ofstream &wavFile, const std::wstring textToTransmit)
{
    auto [minOrd, maxOrd] = findMinMaxASCII(textToTransmit);
    min_ascii = minOrd;
    max_ascii = maxOrd;
    ascii_range = maxOrd - minOrd;

    numSamples = intention.length() * sampleRate / frequency;

    //    double interpolation_denominator = static_cast<double>(sampleRate) / frequency;
    //    double phaseFirst = 2 * M_PI * frequency;
    //    long samples_per_character = sampleRate / frequency;
    //    double phaseIncrement = (2.0f * M_PI * frequency) / static_cast<float>(sampleRate);

    wavFile.write("data", 4);
    const uint32_t dataChunkSizePos = wavFile.tellp();
    wavFile.write(reinterpret_cast<const char *>(&headerChunkSize), sizeof(headerChunkSize));

    double phaseIncrement = (2.0 * M_PI * frequency) / sampleRate;
    double phase = 0.0; // Phase accumulator

    byteRate = sampleRate * numChannels * bitsPerSample / 8;                                                                                     /// THE ABOVE COVERTED INTO BYTES
    blockAlign = numChannels * bitsPerSample / 8;                                                                                                /// NOT SURE YET PROBABLY ALIGNMENT PACKING TYPE OF VARIABLE
    formatSize = sizeof(audioFormat) + sizeof(numChannels) + sizeof(sampleRate) + sizeof(byteRate) + sizeof(blockAlign) + sizeof(bitsPerSample); /// FORMAT CHUNK SIZE

    numSamples = (sampleRate / frequency) * intention.length();

    std::vector<char> frames;
    const size_t bufferSize = 1024 * 1024; // Adjust the buffer size as needed

    for (uint32_t i = 0; i < numSamples; ++i) {
        long char_index = fmod((i / (sampleRate / frequency)), textToTransmit.size());
        wchar_t current_char = textToTransmit[char_index];
        wchar_t next_char = textToTransmit[(char_index + 1) % textToTransmit.size()];

        uint32_t current_codepoint = utf8_codepoint(current_char);
        uint32_t next_codepoint = utf8_codepoint(next_char);

        double amplitude_current = ((current_codepoint) - min_ascii) / static_cast<double>(ascii_range);
        double amplitude_next = ((next_codepoint) - min_ascii) / static_cast<double>(ascii_range);
        double interpolation_factor = fmod(i, (static_cast<double>(sampleRate / frequency))) / (sampleRate / frequency);

        double amplitude_interpolated = 2.0 * (amplitude_current + (amplitude_next - amplitude_current) * interpolation_factor) - 1.0;

        phase += phaseIncrement;
        if (phase > 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }

        long long int sample_value;
        if (smoothing == 0.0) {
            sample_value = amplitude_interpolated * sin(phase) * sampleMax;
        } else {
            double trendingWave = smoothing * sin(phase);
            double smoothedAmplitude = (1.0 - smoothing) * amplitude_interpolated;
            sample_value = (trendingWave + smoothedAmplitude) * sampleMax * volume_level;
        }

        for (int j = 0; j < numChannels; ++j) {
            if (bitsPerSample == 32) {
                int32_t sample_value_32bit = static_cast<int32_t>(sample_value);
                frames.insert(frames.end(), reinterpret_cast<char*>(&sample_value_32bit), reinterpret_cast<char*>(&sample_value_32bit) + sizeof(sample_value_32bit));
            } else {
                int16_t sample_value_16bit = static_cast<int16_t>(sample_value);
                frames.insert(frames.end(), reinterpret_cast<char*>(&sample_value_16bit), reinterpret_cast<char*>(&sample_value_16bit) + sizeof(sample_value_16bit));
            }
        }

        if (frames.size() >= bufferSize * numChannels * (bitsPerSample / 8)) {
            wavFile.write(frames.data(), frames.size());
            frames.clear();
        }

        if (i % 100000 == 0) {
            std::cout << "\rProgress: " << std::fixed << std::setprecision(2) << static_cast<float>(i) / static_cast<float>(numSamples) * 100.0f << "% Samples Written: " << std::to_string(i) << "     \r" << std::flush;
        }
    }

    if (frames.size() > 0) {
        wavFile.write(frames.data(), frames.size());
        frames.clear();
    }

    uint32_t actualDataSize = static_cast<uint32_t>(static_cast<int64_t>(wavFile.tellp()) - dataChunkSizePos - 4);
    if (actualDataSize % 2 != 0) {
        uint8_t paddingByte = 0;
        wavFile.write(reinterpret_cast<const char*>(&paddingByte), sizeof(paddingByte));
        actualDataSize += 1;
    }
    wavFile.seekp(dataChunkSizePos, ios::beg);
    wavFile.write(reinterpret_cast<const char*>(&actualDataSize), 4);
    wavFile.seekp(0, ios::end);

    std::cout << "\rProgress: 100.00%" << " Samples Written: " << std::to_string(numSamples) << "     \r" << std::endl;

    if (frames.size() > 0)
    {
        wavFile.write(frames.data(), frames.size());
        frames.clear();
    }
    std::cout << "\rProgress: 100.00%"
            << " Samples Written: " << std::to_string(numSamples) << "     \r" << std::endl;
}
/// //////////////////////////////END OF RIFF TAG////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// /////////////////////////////START OF ID3V2 TAG////////////////////////////////////////////////////////////////////////////////////////////
const uint8_t majorVersion = 0x04; // ID3v2.4 version number
const uint8_t minorVersion = 0x00; // Revision number
const uint8_t paddingByte = 0x00;  // Zero byte
void writeLittleEndian(std::ofstream &file, uint32_t value)
{
    file.put(value & 0xFF);
    file.put((value >> 8) & 0xFF);
    file.put((value >> 16) & 0xFF);
    file.put((value >> 24) & 0xFF);
}
void writeBigEndian(std::ofstream &file, uint32_t value)
{
    file.put((value >> 24) & 0xFF);
    file.put((value >> 16) & 0xFF);
    file.put((value >> 8) & 0xFF);
    file.put(value & 0xFF);
}
uint32_t checkForAndWritePadding(ofstream &wavFile)
{ /// Calculate padding needed to align frame start to a multiple of 4 bytes
    const uint32_t paddingSize = (BYTEALIGNMENTFORWAVPRODUCER - (wavFile.tellp() % BYTEALIGNMENTFORWAVPRODUCER)) % BYTEALIGNMENTFORWAVPRODUCER;
    for (uint32_t i = 0; i < paddingSize; ++i)
    {
        wavFile.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
    }
    return paddingSize;
}
void writePadding(ofstream &wavFile)
{ /// Calculate padding needed to align frame start to a multiple of 4 bytes
    uint32_t paddingSize = (BYTEALIGNMENTFORWAVPRODUCER - (wavFile.tellp() % BYTEALIGNMENTFORWAVPRODUCER)) % BYTEALIGNMENTFORWAVPRODUCER;
    if (paddingSize == 0)
        paddingSize = BYTEALIGNMENTFORWAVPRODUCER;
    for (uint32_t i = 0; i < paddingSize; ++i)
    {
        wavFile.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
    }
}
uint32_t createID3Header(ofstream &wavFile, const uint32_t tagSize = 0, const uint8_t flags = 0x00)
{
    //    const uint32_t paddingAdded = checkForAndWritePadding(wavFile);
    wavFile.write("ID3", 3);                                         /// WRITE TAG OF HEADER OF ID3TAG
    wavFile.write(reinterpret_cast<const char *>(&majorVersion), 1); /// WRITE MAJOR VERSION
    wavFile.write(reinterpret_cast<const char *>(&minorVersion), 1); /// WRITE MINOR VERSION
    wavFile.write(reinterpret_cast<const char *>(&flags), 1);        /// WRITE FLAGS
    const uint32_t ID3HeaderSizeElementLocation = wavFile.tellp();
    const uint32_t actualTagSize = tagSize;
    if constexpr (std::endian::native == std::endian::little)
    { /// WRITE TAG SIZE = SIZE OF ID3 TAG MINUS HEADER
        writeLittleEndian(wavFile, actualTagSize);
    }
    else if constexpr (std::endian::native == std::endian::big)
    {
        writeBigEndian(wavFile, actualTagSize);
    }
    else
    {
        wavFile.write(reinterpret_cast<const char *>(&actualTagSize), 4);
    }
    return ID3HeaderSizeElementLocation;
}
void createID3Footer(ofstream &wavFile, const uint32_t tagSize = 0, const uint8_t flags = 0x00)
{
    //    checkForAndWritePadding(wavFile);
    wavFile.write("3DI", 3);                                         /// WRITE TAG OF FOOTER OF ID3TAG WHICH IS BACKWARDS HEADER ID
    wavFile.write(reinterpret_cast<const char *>(&majorVersion), 1); /// WRITE MAJOR VERSION
    wavFile.write(reinterpret_cast<const char *>(&minorVersion), 1); /// WRITE MINOR VERSION
    wavFile.write(reinterpret_cast<const char *>(&flags), 1);        /// WRITE FLAGS
    wavFile.write(reinterpret_cast<const char *>(&tagSize), 4);      /// WRITE TAG SIZE
}
uint32_t createID3FrameHeader(ofstream &wavFile, const char *frameID, const uint32_t frameSize = 0, const uint16_t flags = 0x00)
{
    //    checkForAndWritePadding(wavFile);
    wavFile.write(frameID, 4); /// The frame ID is made out of the characters capital A-Z and 0-9. Identifiers beginning with �X�, �Y� and �Z�
    const uint32_t frameSizePosition = wavFile.tellp();
    wavFile.write(reinterpret_cast<const char *>(&frameSize), 4);
    wavFile.write(reinterpret_cast<const char *>(&flags), 2);
    return frameSizePosition;
}
void setID3HeaderSize(ofstream &wavFile, const uint32_t frameSizeElementLocation, const uint32_t frameChunkSize)
{
    const uint32_t currentPos = wavFile.tellp();
    wavFile.seekp(frameSizeElementLocation, std::ios::beg);
    wavFile.write(reinterpret_cast<const char *>(&frameChunkSize), 4);
    wavFile.seekp(currentPos, std::ios::beg);
    createID3Footer(wavFile, frameChunkSize);
}
void insertID3Frame(ofstream &wavFile, const string &key, const string &value)
{
    const uint32_t frameSize = key.size() + value.size() + 1; /// Size of frame = size of key + size of value + 1 (for null terminator)
    createID3FrameHeader(wavFile, key.c_str(), frameSize);
    wavFile.write("\0", 1); // Null terminator between key and value
    wavFile.write(value.c_str(), value.size());
}
/// ////////////////////////////////////////////////END OF ID3V2 TAG///////////////////////////////////////////////////////////////////////////
/// CONVERT A TEXT FILE TO A VECTOR OF SINGLE BYTES ///
std::wstring readFileToWString(const string &filename)
{
    ifstream fileStream(filename, ios::binary | ios::ate); /// CREATE INPUT FILE STREAM AND OPEN A FILE NAMED FILENAME
    if (!fileStream.is_open())
    { /// CRASH IF INPUT FILE NOT FOUND ///
        std::cerr << "Error: File stream is not open." << std::endl;
        return L"";
    }
    std::stringstream buffer;     /// Create a stringstream to store the file contents
    buffer << fileStream.rdbuf(); /// READ FILE CONTENTS INTO STRING BUFFER ///
    return utf8_to_wstring(buffer.str());          /// RETURN THE STRINGSTREAM AS A STRING ///
}
/// Function to create WAV file with binary data repeated until 1 minute ///
void createWavFile(const string &filename, const std::wstring textData)
{
    ofstream wavFile(filename, ios::binary | ios::trunc); /// CREATE A OUTPUT FILE STREAM OBJECT WHICH CREATES A FILE NAMED FILENAME
    if (!wavFile.is_open())
    { /// IF FAILED TO CREATE OR ITS NOT OPEN CRASH THE PROGRAM
        cerr << "Error: Unable to create WAV file " << filename << endl;
        exit(EXIT_FAILURE);
    }
    ///  ///////////////////////////////////// WRITING ACTUAL WAV FILE NOW////////////////////////////////////////////////////////////////////////////////////////
    writeRiffHeader(wavFile);                                                                                          /// WRITING THE RIFF HEADER WITH PLACEHOLDER SIZE
    writeFormatHeader(wavFile, formatSize, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample); /// WRITING FORMAT HEADER TELLING WAV FORMAT
                                                                                                                       /// /////////////////////////////////ACCOMPANYING DATA//////////////////////////////////////////////////////////////////////////////////
    const uint32_t listHeaderSizeChunkElementPos = writeListHeader(wavFile);                                           /// Write LIST HEADER
                                                                                                                       /// END  OF WRITING SUB CHUNKS //////////////////////////////////////////////////////////////////////////////////////////////////////
    const uint32_t realListChunkSize = static_cast<uint32_t>(static_cast<int64_t>(wavFile.tellp()) - listHeaderSizeChunkElementPos - 4);
    writeListHeaderSizeChunkElement(wavFile, listHeaderSizeChunkElementPos, realListChunkSize); /// INPUTTING LIST SIZE IN LIST SIZE CHUNK ELEMENT
    writeDataChunk(wavFile, textData);                                                          ///  WRITE DATA CHUNK
    writeRiffHeaderSizeElement(wavFile);                                                        /// WRITING RIFF HEADER SIZE
                                                                                                ///////////////////////////DONE WRITING RIFF TAG/////////////////////////////////////
                                                                                                //////////////////////////START WRITING ID3V2.4 TAG////////////////////////////////
                                                                                                //    const uint32_t ID3HeaderSizeChunkLocation = createID3Header(wavFile);
                                                                                                //    insertID3Frame(wavFile,"AFFIRMING THAT I AM AWESOME","THAT IS CORRECT");
                                                                                                //    setID3HeaderSize(wavFile,ID3HeaderSizeChunkLocation,(wavFile.tellp() - ID3HeaderSizeChunkLocation)-4);
                                                                                                //    const uint32_t headerSizeElementSelector = insertCustomID3Header(wavFile,0);
                                                                                                //    insertCommentFrame(wavFile,"THIS IS A COMMENT");
                                                                                                //    EditCustomID3HeaderSize(wavFile,headerSizeElementSelector,(wavFile.tellp() - headerSizeElementSelector) - 4);
                                                                                                //    InsertCustomID3Footer(wavFile);
    wavFile.close();                                                                            /// CLOSE THE WAV FILE WE ARE DONE
}
short removeOldFile(const std::string &filename)
{
    if (exists(filename))
    {
        remove(filename);
    }
    return 0;
}
bool isFilePathValid(const std::string &filePath)
{
    std::ifstream file(filePath);
    return file.good();
}
bool is_hz_suffix(const std::string &str)
{
    return str.size() >= 2 && (str.substr(str.size() - 2) == "HZ" || str.substr(str.size() - 2) == "hz" || str.substr(str.size() - 2) == "Hz");
}
void setupQuestions()
{
    std::cout << "WAV Repeater with Smoothing" << std::endl;
    std::cout << "by Anthro Teacher and Nathan" << std::endl
              << std::endl;
    while (intention.empty())
    {
        std::cout << "Enter Binary Filename: ";
        std::getline(std::cin, inputFile);
        
        std::ifstream file(inputFile);
        if (file.is_open())
        {
            inputFile += "_";
            std::stringstream buffer;
            buffer << file.rdbuf();
            intention = utf8_to_wstring(buffer.str());
            intentionOriginal = wstring_to_utf8(intention);
            file.close();
        }
        else
        {
            std::cout << "Unable to open the file." << std::endl;
            inputFile.clear();
        }
    }
    int repeatIntention = 0;
    while (repeatIntention < 1 || repeatIntention > 1000000)
    {
        std::string input;
        std::cout << "# Times to Repeat [1 to 1000000, Default 1]: ";
        std::getline(std::cin, input);
        try
        {
            repeatIntention = std::stoi(input);
        }
        catch (...)
        {
            repeatIntention = 1;
        }
    }
    std::wstring intentionOriginalWStr = intention;
    intention = L"";
    for (int i = 0; i < repeatIntention; ++i)
        intention += intentionOriginalWStr;
    while (sampleRate < 44100 || sampleRate > 767500)
    {
        std::cout << "Enter Sampling Rate [Default 48000, Max 767500]: ";
        std::getline(std::cin, sampling_rate_input);
        try
        {
            sampleRate = std::stoi(sampling_rate_input);
        }
        catch (...)
        {
            sampleRate = 48000;
        }
    }
    while (numChannels < 1 || numChannels > 8)
    {
        std::string channels_input;
        std::cout << "Enter Channels (1-8): ";
        std::getline(std::cin, channels_input);
        try
        {
            numChannels = std::stoi(channels_input);
        }
        catch (...)
        {
            numChannels = 1;
        }
    }

    while (bitsPerSample != 16 && bitsPerSample != 32)
    {
        std::string bitsPerSample_input;
        std::cout << "Enter Bytes Per Amplitude (2 or 4): ";
        std::getline(std::cin, bitsPerSample_input);
        try
        {
            bitsPerSample = std::stoi(bitsPerSample_input) * 8;
        }
        catch (...)
        {
            bitsPerSample = 32;
        }
    }
    sampleMax = (bitsPerSample == 16 ? 32767 : 2147483647);
    sampleMin = (bitsPerSample == 16 ? -32768 : -2147483648);

    byteRate = sampleRate * numChannels * bitsPerSample / 8;                                                                                     /// THE ABOVE COVERTED INTO BYTES
    blockAlign = numChannels * bitsPerSample / 8;                                                                                                /// NOT SURE YET PROBABLY ALIGNMENT PACKING TYPE OF VARIABLE
    formatSize = sizeof(audioFormat) + sizeof(numChannels) + sizeof(sampleRate) + sizeof(byteRate) + sizeof(blockAlign) + sizeof(bitsPerSample); /// FORMAT CHUNK SIZE

    while (frequency < 20)
    {
        std::cout << "Enter Frequency (Default 432Hz): ";
        std::getline(std::cin, frequency_input);
        if (frequency_input.empty()) {
            frequency = 432.0;
            frequency_input = "432";
            break;
        }

        // If lowercase right 2 characters are hz, then remove them
        if (is_hz_suffix(frequency_input))
        {
            frequency_input = frequency_input.substr(0, frequency_input.size() - 2);
        }
        try
        {
            frequency = std::stod(frequency_input);
        }
        catch (...)
        {
            frequency = 432.0;
        }
    }

    numSamples = repeatIntention * intention.length() * sampleRate / frequency;

    while (smoothing < 0.0 || smoothing > 1.0)
    {
        std::cout << "Smoothing Factor (0.0-100.0%, Default 50.0%): ";
        std::getline(std::cin, smoothing_percent);
        // If rightmost character of smoothing_percent == "%", then remove that last character
        if (!smoothing_percent.empty() && smoothing_percent.back() == '%')
        {
            smoothing_percent.pop_back();
        }
        try
        {
            smoothing = std::stod(smoothing_percent) / 100;
            if (smoothing < 0.0)
            {
                smoothing = 0.0;
            }
        }
        catch (...)
        {
            smoothing = 0.50;
        }
    }
    if (smoothing_percent.empty())
    {
        smoothing = 0.50;
        smoothing_percent = "50.0";
    }
}

#if defined(__GNUC__) || defined(__clang__)
#define NO_OPTIMIZE __attribute__((optimize("O0")))
#elif defined(_MSC_VER)
#define NO_OPTIMIZE __declspec(noinline)
#else
#define NO_OPTIMIZE
#endif
volatile bool threadExit = false;
long long totalIterations = 0;
NO_OPTIMIZE void stringMemoryAllocation(const std::wstring &textParameter)
{
    const size_t GB = 1e9;
    const std::wstring terminator = L"####";
    //    std::wcout << L"Starting Intent Processor Script\n";
    while (!threadExit)
    {
        std::wstring *str = new std::wstring;
        volatile size_t totalSize = 0;
        while (totalSize < GB)
        {
            *str += textParameter;
            totalSize += textParameter.size() * sizeof(wchar_t);
            ++totalIterations;
        }
        // Adding terminator to mark end of string
        *str += terminator;
        // Deleting the string
        delete str;
    }
    //    std::wcout << L"Intent Processor Thread Exitting\n";
}

int main()
{
    setupQuestions();

    string outputFile = inputFile + frequency_input + "Hz_SmoothingPercent_" + smoothing_percent + "_" + sampling_rate_input + ".wav";
    removeOldFile(outputFile);
    std::jthread intentProcessor(&stringMemoryAllocation, intention);
    createWavFile(outputFile, intention);
    threadExit = true;
    cout << outputFile << " written." << endl;
    return 0;
}
