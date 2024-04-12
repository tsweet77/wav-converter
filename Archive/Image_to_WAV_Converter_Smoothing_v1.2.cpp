/*
WAV Repeater with Smoothing
by Anthro Teacher and Nathan
To Compile: g++ -O3 -Wall -static .\Image_to_WAV_Repeater_Smoothing.cpp -o .\Image_to_WAV_Repeater_Smoothing.exe -std=c++20
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
#include <climits>
using namespace std;
using namespace filesystem;
#ifdef _WIN64
#define BYTEALIGNMENTFORWAVPRODUCER 8
#else
#define BYTEALIGNMENTFORWAVPRODUCER 4
#endif
#include <bit>
/// ////////////////////////////////////////////START OF RIFF WAVE TAG ///////////////////////////////////////////////////////////////////////////
const uint32_t headerChunkSize = 0; /// PLACE HOLDER FOR RIFF HEADER CHUNK SIZE
/// /////////FORMAT CHUNK////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double volume_level = 1.000; /// VOLUME LEVEL///
uint16_t audioFormat = 1;          /// 3 is float 1 is PCM                            /// AN UNKNOWN AT THIS TIME
uint16_t numChannels = 0;          /// 8;                                                     /// NUMBER OF CHANNELS FOR OUR AUDIO I PRESUME 1 SAMPLE PER CHANNEL
uint32_t sampleRate = 0;           ///  765000.0;                                       /// PRESUMABLY THE NUMBER OF SAMPLES PER SECOND
uint16_t bitsPerSample = 0;        /// 16                                                     /// THE NUMBER OF BITS PER SAMPLE 2 BYTES // SAME AS AMPWIDTH
uint32_t byteRate = 0;             // sampleRate * numChannels * bitsPerSample / 8;                                                                                     /// THE ABOVE COVERTED INTO BYTES
uint16_t blockAlign = 0;           // numChannels * bitsPerSample / 8;                                                                                                /// NOT SURE YET PROBABLY ALIGNMENT PACKING TYPE OF VARIABLE
uint32_t formatSize = 0;           // sizeof(audioFormat) + sizeof(numChannels) + sizeof(sampleRate) + sizeof(byteRate) + sizeof(blockAlign) + sizeof(bitsPerSample); /// FORMAT CHUNK SIZE
uint32_t sampleMax = 0;            //(bitsPerSample == 16 ? 32767 : 2147483647);
uint32_t sampleMin = 0;            //(bitsPerSample == 16 ? 32767 : 2147483647);
double frequency = 0.0;            /// Frequency To Play At
double smoothing = -1.0;           /// Interpolation Smoothing
double volume = -1.0;
uint32_t durationInSeconds = -1; /// 10                                                                                           ///DURATION OF THE WAV FILE
// uint32_t numOfDataCyclesToWrite = durationInSeconds * sampleRate * numChannels; /// THE NUMBER OF CYCLES WE COPY DATA INTO OUR DATA CHUNK
// vector<uint16_t> silenceSamples(numOfDataCyclesToWrite, 1);                     /// VECTOR OF EMPTY DATA FOR OUR SILENT WAV FILE
std::string continue_input, volume_percent = "0", inputFile = "", frequency_input = "0", smoothing_percent = "0", sampling_rate_input = "0";
int ascii_range = 0, min_ascii = 0, max_ascii = 0;
long long int numSamples = 0;
double M_PI = 3.141592653589793238462643383279502884197;
std::string filename = "";
std::vector<char> binaryIntentionData;
std::vector<char> binaryIntentionDataOriginal;
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
void ensureDataAlignment(ofstream &wavFile) {
    long pos = wavFile.tellp();
    long align = pos % 2;
    if (align != 0) {
        uint8_t paddingByte = 0;
        wavFile.write(reinterpret_cast<const char *>(&paddingByte), sizeof(paddingByte));
    }
}
void writeRiffHeader(ofstream &wavFile)
{
    wavFile.seekp(0, std::ios::beg);
    wavFile.write("RIFF", 4);                                                                 /// HEADER NAME FOR WAVE FILE ///                                         ///WRITING RIFF HEADER CALLING SIGN INTO FILE
    wavFile.write(reinterpret_cast<const char *>(&headerChunkSize), sizeof(headerChunkSize)); /// WRITING PLACEHOLDER FOR RIFF HEADER CHUNK SIZE INTO FILE
    // std::cout << "Checking Pading Details Current Pos# " << wavFile.tellp() << std::endl;
}
/// PRESUMES that when you call the function you are at end of file///
void writeRiffHeaderSizeElement(ofstream &wavFile) {
    int fileLength = static_cast<int>(wavFile.tellp());
    uint32_t fileSizeMinus8 = static_cast<uint32_t>(fileLength - 8);
    wavFile.seekp(4, ios::beg);
    wavFile.write(reinterpret_cast<const char *>(&fileSizeMinus8), sizeof(fileSizeMinus8));
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
std::pair<int, int> findMinMaxASCII(const std::string &input)
{
    if (input.empty())
    {
        // Return the maximum and minimum int values if the string is empty
        return {std::numeric_limits<int>::max(), std::numeric_limits<int>::min()};
    }

    auto minMaxPair = std::minmax_element(input.begin(), input.end());
    return {static_cast<int>(*minMaxPair.first), static_cast<int>(*minMaxPair.second)};
}
void writeDataChunk(std::ofstream &wavFile) {
    int minCharValue = INT_MAX;
    int maxCharValue = INT_MIN;

    // Iterate through binaryIntentionDataOriginal to find min and max values
    for (char c : binaryIntentionDataOriginal) {
        unsigned char uc = static_cast<unsigned char>(c); // Ensure treating as unsigned
        minCharValue = std::min(minCharValue, static_cast<int>(uc));
        maxCharValue = std::max(maxCharValue, static_cast<int>(uc));
    }

    int rangeValue = maxCharValue - minCharValue;
    double sampleMax = (bitsPerSample == 16 ? 32767 : 2147483647);
    //double scalingFactor = sampleMax / rangeValue;
    uint32_t numSamples = binaryIntentionData.size();
    std::vector<char> sample_buffer;

    double phaseIncrement = 2.0 * M_PI * frequency / sampleRate;
    double phase = 0.0;
    wavFile.write("data", 4);
    const uint32_t dataChunkSizePos = wavFile.tellp();
    wavFile.write(reinterpret_cast<const char *>(&headerChunkSize), sizeof(headerChunkSize));

    for (uint32_t i = 0; i < numSamples; ++i) {
        double charValue = static_cast<unsigned char>(binaryIntentionData[i]) - minCharValue;
        double normalizedCharValue = (charValue / rangeValue) * 2.0 - 1.0; // Normalize to [-1, 1]

        double sineValue = std::sin(phase);
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) phase -= 2.0 * M_PI;

        // Modulate based on smoothing
        double modulatedValue;
        if (smoothing == 1.0) {
            modulatedValue = sineValue;
        } else if (smoothing > 0 && smoothing < 1.0) {
            modulatedValue = (normalizedCharValue * (1.0 - smoothing)) + (sineValue * smoothing);
        } else {
            modulatedValue = normalizedCharValue;
        }

        long sample_value = static_cast<long>(modulatedValue * volume * sampleMax);

        // Write sample_value to sample_buffer
        if (bitsPerSample == 32) {
            int32_t val = static_cast<int32_t>(sample_value);
            sample_buffer.insert(sample_buffer.end(), reinterpret_cast<char*>(&val), reinterpret_cast<char*>(&val) + sizeof(val));
        } else {
            int16_t val = static_cast<int16_t>(sample_value);
            sample_buffer.insert(sample_buffer.end(), reinterpret_cast<char*>(&val), reinterpret_cast<char*>(&val) + sizeof(val));
        }

        // Write buffer to file and clear every 100000 samples
        if (i % 100000 == 0 && i > 0) {
            wavFile.write(sample_buffer.data(), sample_buffer.size());
            sample_buffer.clear();
            std::cout << "\rProgress: " << std::fixed << std::setprecision(2) << static_cast<float>(i) / static_cast<float>(numSamples) * 100.0f << "% Samples Written: " << i << "     \r" << std::flush;
        }
    }
    uint32_t actualDataSize = static_cast<uint32_t>(static_cast<int64_t>(wavFile.tellp()) - dataChunkSizePos - 4);
    if (actualDataSize % 2 != 0) {
        uint8_t paddingByte = 0;
        wavFile.write(reinterpret_cast<const char*>(&paddingByte), sizeof(paddingByte));
        actualDataSize += 1;
    }
    // Ensure data alignment after writing the main data chunk
    ensureDataAlignment(wavFile);

    wavFile.seekp(dataChunkSizePos, ios::beg);
    wavFile.write(reinterpret_cast<const char *>(&actualDataSize), sizeof(actualDataSize));
    wavFile.seekp(0, ios::end);

    // Write any remaining samples in buffer to file
    if (!sample_buffer.empty()) {
        wavFile.write(sample_buffer.data(), sample_buffer.size());
        sample_buffer.clear();
    }
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
std::string readFileToString(const string &filename)
{
    ifstream fileStream(filename, ios::binary | ios::ate); /// CREATE INPUT FILE STREAM AND OPEN A FILE NAMED FILENAME
    if (!fileStream.is_open())
    { /// CRASH IF INPUT FILE NOT FOUND ///
        std::cerr << "Error: File stream is not open." << std::endl;
        return "";
    }
    std::stringstream buffer;     /// Create a stringstream to store the file contents
    buffer << fileStream.rdbuf(); /// READ FILE CONTENTS INTO STRING BUFFER ///
    return buffer.str();          /// RETURN THE STRINGSTREAM AS A STRING ///
}
/// Function to create WAV file with binary data repeated until 1 minute ///
void createWavFile(const string &filename) {
    ofstream wavFile(filename, ios::binary | ios::trunc);
    if (!wavFile.is_open()) {
        cerr << "Error: Unable to create WAV file " << filename << endl;
        exit(EXIT_FAILURE);
    }

    // Writing headers and data chunks
    writeRiffHeader(wavFile);
    writeFormatHeader(wavFile, formatSize, audioFormat, numChannels, sampleRate, byteRate, blockAlign, bitsPerSample);
    writeDataChunk(wavFile);  // Writes the bulk of audio data
    
    // Ensure alignment before finalizing the file
    ensureDataAlignment(wavFile);

    writeRiffHeaderSizeElement(wavFile);  // Finalizes the RIFF header size
    wavFile.close();
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
std::string display_suffix(std::string num, int power, std::string designator)
{
    if (power < 3)
    {
        return num;
    }

    std::string s;

    if (designator == "Iterations")
    {
        constexpr char iterations_suffix_array[] = {' ', 'k', 'M', 'B', 'T', 'q', 'Q', 's', 'S', 'O', 'N', 'D'};
        s = iterations_suffix_array[power / 3];
    }
    else // designator == "Frequency"
    {
        constexpr char frequency_suffix_array[] = {' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y', 'R'};
        s = frequency_suffix_array[power / 3];
    }

    std::string str2 = num.substr(0, power % 3 + 1) + "." + num.substr(power % 3 + 1, 3) + s;

    return str2;
}
void setupQuestions()
{
    std::cout << "Binary File to WAV Repeater with Smoothing" << std::endl;
    std::cout << "by Anthro Teacher and Nathan" << std::endl
              << std::endl;
    while (filename.empty())
    {
        std::cout << "Enter Binary Filename: ";
        std::getline(std::cin, filename);

        std::ifstream file(filename, std::ios::binary); // Open the file in binary mode
        if (file.is_open())
        {
            inputFile = filename + "_";
            // Use a vector to store binary data
            std::vector<char> binaryIntentionData((std::istreambuf_iterator<char>(file)), (std::istreambuf_iterator<char>()));

            file.close();
            binaryIntentionDataOriginal = binaryIntentionData;
        }
        else
        {
            std::cout << "Unable to open the file." << std::endl;
        }
    }
    // std::string intentionOriginal = intention;
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
    // intention = "";
    for (int i = 0; i < repeatIntention; ++i)
        binaryIntentionData.insert(binaryIntentionData.end(), binaryIntentionDataOriginal.begin(), binaryIntentionDataOriginal.end());

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
        if (frequency_input.empty())
        {
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

    while (volume < 0.0 || volume > 0.99)
    {
        std::cout << "Volume (0.0-99%, Default 25.0%): ";
        std::getline(std::cin, volume_percent);
        // If rightmost character of volume_percent == "%", then remove that last character
        if (!volume_percent.empty() && volume_percent.back() == '%')
        {
            volume_percent.pop_back();
        }
        try
        {
            volume = std::stod(volume_percent) / 100;
            if (volume < 0.0)
            {
                volume = 0.0;
            }
        }
        catch (...)
        {
            volume = 0.25;
        }
    }
    if (volume_percent.empty())
    {
        volume = 0.25;
        volume_percent = "25";
    }
    int digits;
    int binarySize = binaryIntentionData.size() * bitsPerSample / 8.0 / 10;
    digits = std::to_string(binarySize).length();
    std::cout << "Estimated File Size: " << display_suffix(std::to_string(binarySize), digits, "Frequency") << "B, Continue: (y/N)? ";
    std::string continue_input;
    std::getline(std::cin, continue_input);
    if (continue_input.empty() || tolower(continue_input[0])!= 'y')
    {
        return;
    }
    std::cout << "Generating WAV File..." << std::endl;
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
NO_OPTIMIZE void stringMemoryAllocation(const std::string &textParameter)
{
    const size_t GB = 1e9;
    const std::string terminator = "####";
    //    std::cout << "Starting Intent Processor Script\n";
    while (!threadExit)
    {
        std::string *str = new std::string;
        volatile size_t totalSize = 0;
        while (totalSize < GB)
        {
            *str += textParameter;
            totalSize += textParameter.size();
            ++totalIterations;
        }
        // Adding terminator to mark end of string
        *str += terminator;
        // Deleting the string
        delete str;
    }
    //    std::cout <<"Intent Processor Thread Exitting\n";
}

int main()
{
    cout << "Image to WAV Converter v.1.2\n";
    cout << "Copyright (c) 2024 Anthro Teacher\n\n";
    setupQuestions();
    if (toupper(continue_input[0]) != 'Y')
    {
        std::cout << "Exiting..." << std::endl;
        return 0;
    }

    string outputFile = inputFile + frequency_input + "Hz_SmoothingPercent_" + smoothing_percent + "_" + sampling_rate_input + ".wav";
    removeOldFile(outputFile);
    //    std::jthread intentProcessor(&stringMemoryAllocation, intention);
    createWavFile(outputFile);
    // threadExit = true;
    cout << outputFile << " written." << endl;
    return 0;
}
