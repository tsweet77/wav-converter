/*
Intention Repeater WAV Converter v2 created by Thomas Sweet.
Converts any file <~20MB or less into a 96kHz Mono WAV File.
Defaults to a 10 Minute WAV file.
Updated 2/14/2021 v2.0
Usage: wav_repeater.exe --help
Intention Repeater is powered by a Servitor (20 Years / 2000+ hours in the making)
Servitor Info: https://web.archive.org/web/20200915191532/https://enlightenedstates.com/2017/04/07/servitor-just-powerful-spiritual-tool/
Website: https://www.intentionrepeater.com/
Forum: https://forums.intentionrepeater.com/
Licensed under GNU General Public License v3.0
This means you can modify, redistribute and even sell your own modified software, as long as it's open source too and released under this same license.
https://choosealicense.com/licenses/gpl-3.0/
*/

// This is the same function in Python file. just re-wrote in C++
std::string bytes_format(unsigned int B) 
    { 
    if (B < KB) { return std::to_string(B) + " Bytes"; }
    else if (B>KB &&B<MB) { return std::to_string(B/ KB) + " k"; }
    else if (B > MB && B < GB) { return std::to_string(B/ MB) + " M"; }
    else if (B > GB && B < TB) { return std::to_string(B / GB) + " G"; }
    else if (B > TB ) { return std::to_string(B / TB) + " T"; }
    }
//This is the same function in Python file. just re-wrote in C++
std::string human_format(unsigned int B) 
{
         if (B < KB_h) { return std::to_string(B) + " b"; }
    else if (B > KB_h && B < MB_h) { return std::to_string(B / KB_h) + " Thousand"; }
    else if (B > MB_h && B < GB_h) { return std::to_string(B / MB_h) + " Million"; }
    else if (B > GB_h && B < TB_h) { return std::to_string(B / GB_h) + " Billion"; }
    else if (B > TB_h) { return std::to_string(B / TB_h) + " Trillion"; }
}
// get seconds from string time (ex: 00:01:00 -> 60 seoonds
int get_seconds(std::string str_time)
{
    int h, m, s = 0;
    int seconds = -1;
    if (sscanf(str_time.c_str(), "%d:%d:%d", &h, &m, &s) >= 2)
    {
        seconds = h * 3600 + m * 60 + s;
    }
    return seconds;
}

// check if string ends with certain string (example, check if string ends with ".wav"
bool ends_with(std::string const& value, std::string const& ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

// to handle ctrl+c in windows
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        //Beep(750, 300);
        exit(-2);

        return TRUE;
    default:
        return FALSE;
    }
}



int main(int argc, char* argv[])
{
    // enabling ctrl +c 
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    // adding the options
    cxxopts::Options options("MyProgram", "One line description of MyProgram");
    options.add_options()
       ("d,dur", "Duration in HH:MM:SS format. Length of the final file", cxxopts::value<std::string>()->default_value(DEFAULT_DURATION))
        //("i,intent", "Int param", cxxopts::value<int>())
       ("f,freq", "Frequency of repeatition", cxxopts::value<double>()->default_value("0.0"))
       ("v,volume", "Volume 0-1.00 of max. Default = 0.95", cxxopts::value<double>()->default_value(VOLUME_LEVEL))
       ("r,rate", "Sampling Rate of the WAV file created. Default = 96000", cxxopts::value<int32_t>()->default_value(SAMPLING_RATE))
       ("n,infile", "Name of input file. Default = Ask for it at the prompt", cxxopts::value<std::string>()->default_value(""))
       ("o,outfile", "Name of output WAV file. Default = Ask for it at the prompt.", cxxopts::value<std::string>()->default_value(""))
       ("h,help", "Print usage")
        ;
    
    
    // parsing arguments
    auto result = options.parse(argc, argv);

    // check if help, print the help message
    if (result.count("help"))
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }
    // getting parameters from the given options
    // rate
    int32_t sampling_rate = result["rate"].as<int32_t>();
    // volume level
    double volume_level = result["volume"].as<double>();
    // freq (0 if not given)
    double freq_r = result["freq"].as<double>();
    // druation
    std::string duration_param = result["dur"].as<std::string>();//DEFAULT_DURATION;
    // input file name
    std::string in_filename_param  = result["infile"].as<std::string>();
    // output file name
    std::string out_filename_param = result["outfile"].as<std::string>();
    //
    std::cout << "rate= " << sampling_rate << "\nvolume= " << volume_level << "\nduration= " << duration_param << "\n";
    

    // get the seconds from the given duration 
    int duration_seconds = get_seconds(duration_param);
    // total samples 
    int total_samples = duration_seconds * sampling_rate;


    // freq. calcuations
    // This part is un-finished, skip it
    int32_t silence_samples = 0;
    int32_t period_samples  = 0;
    

    //std::cout << "silence= " << silence_samples << "\n";

    // peak values
    int peak_value = 32767;
    int peak_value_2 = peak_value * 2;

    // handling parameters
    // if no input filename is given (length=0)-> read it from std in
    if (in_filename_param.length() == 0)
    {
        std::cout << "Input Filename: \n";
        std::cin >> in_filename_param;
    }
    // if no output filename is given (length=0)-> read it from std in
    if (out_filename_param.length() == 0)
    {
        std::cout << "Output Filename (.wav):  \n";
        std::cin >> out_filename_param;
    }
    // if output filename doesn't end with .wav, add it
    if (!ends_with(out_filename_param, ".wav"))
    {
        out_filename_param += ".wav";
    }

    int minval = peak_value_2;
    int maxval = -peak_value_2;


    /*Reading the file*/
    FILE* fp = fopen(in_filename_param.c_str(), "r");
    // checking if the file is found
    if (fp == nullptr) 
        { 
            std::cout << "Error Opening File!\n";
            return -1; 
        }
    // get file size, to reserve enough memory
    fseek(fp, 0L, SEEK_END);
    unsigned int sz = ftell(fp);
    rewind(fp);

    // buffer for holding the file
    BYTE* buffer = new BYTE[sz + 1];

    /*reading all bytes*/
    while (fread(buffer, sizeof(char), sz, fp) ==sz)
    {
    }

    /*getting min and max of the file*/
    for (unsigned int i = 0; i < sz; i++)
    {
        if (buffer[i] <= minval)
            minval = buffer[i];

        if (buffer[i] >= maxval)
            maxval = buffer[i];
    }

    unsigned int widthval = maxval - minval;
    if (widthval == 0)
    {
        std::cout << "File has no data!\n";
        return -2;
    }
    double multiplier = peak_value_2 / (double)widthval;
    uint32_t num_writes  = 0;
    uint32_t num_seconds = 1;
    uint32_t sample_num = 0;



    // determining if the selected freq is doabale
    if (freq_r != 0)
    {
        // length of input in seconds
        double input_length = (double)sz / sampling_rate;
        double period = 1 / freq_r;

        std::cout << "input length=" << input_length << "\n";
        std::cout << "period=" << period << "\n";

        if (period < input_length)
        {
            std::cout << "This frequency is not applicable. Max freq for this file=" <<1/ input_length << "Hz \n";
            exit(-1);
        }
    
        period_samples = (int32_t)(sampling_rate * (1 / freq_r));
        silence_samples = period_samples - sz;
    }


    TinyWav  audioFile;
    tinywav_open_write(&audioFile, 1, sampling_rate, TW_INT16, TW_INLINE, out_filename_param.c_str());

    int16_t* audio_buffer= new int16_t[total_samples];
    //audio_buffer.resize(total_samples);
    

    // looping till we reach all samples
    while (sample_num <= total_samples)
    {
        // looping the file. bytes
        if (sample_num < total_samples)
        {
            for (uint32_t i = 0; i < sz; i++)
            {
                // exactly like python
                int32_t normalized_element = ((buffer[i] * multiplier - peak_value) * volume_level);
                int32_t value = normalized_element;

                if (value < -peak_value) { value = -peak_value; }
                else if (value > peak_value) { value = peak_value; }

                audio_buffer[sample_num] = value;
                sample_num += 1;

                // logging exactly like in Python code.
                if (sample_num % 25000 == 0)
                {
                    uint32_t percentage = (uint32_t)  (  ((double)sample_num/ total_samples) * 100);
                    std::cout << "   \r" << "Status: [" << percentage << "%] (" << bytes_format(sample_num * 2) << "B /" << bytes_format(total_samples * 2) << "B): " << in_filename_param << " -> " << out_filename_param;
                }

                if (sample_num >= total_samples)
                    break;
            }
        }

        // adding silence if freq is given.
        if ((sample_num < total_samples)&&freq_r>0 )
        {
            for (uint32_t j = 0; j < silence_samples; j++)
            {
                audio_buffer[sample_num] = 0;
                sample_num += 1;
                if (sample_num >= total_samples)
                    break;
                
            }
        }

        // when we finish all we come here
        else if (sample_num >= total_samples)
        {
            uint32_t percentage = (uint32_t)(((double)sample_num / total_samples) * 100);
            std::cout << "   \r" << "Status: [" << percentage << "%] (" << bytes_format(sample_num * 2) << "B /" << bytes_format(total_samples * 2) << "B): " << in_filename_param << " -> " << out_filename_param;
            std::cout << "\nInput Filename: " << in_filename_param << '\n';
            std::cout << "Output Filename: " << out_filename_param << '\n';
            std::cout << "Num Times File Repeated: " << int(num_writes) << '\n';
            std::cout << "Num Samples Written: " << human_format(total_samples * 2) << '\n';


            tinywav_write_f(&audioFile, audio_buffer, total_samples * sizeof(BYTE));
            // free the reserved buffer memory
            delete buffer;
            delete audio_buffer;
            tinywav_close_write(&audioFile);

            // return
            return 0;

        }

        
       
        num_writes += 1;
        uint32_t percentage = (uint32_t)(((double)sample_num / total_samples) * 100);
        std::cout << "   \r" << "Status: [" << percentage << "%] (" << bytes_format(sample_num * 2) << "B /" << bytes_format(total_samples * 2) << "B): " << in_filename_param << " -> " << out_filename_param;

    }



    uint32_t percentage = (uint32_t)(((double)sample_num / total_samples) * 100);
    std::cout << "Status: [" << percentage << "%] (" << bytes_format(sample_num * 2) << "B /" << bytes_format(total_samples * 2) << "B): " << in_filename_param << " -> " << out_filename_param << "   \r" << std::endl;
    std::cout << "\nInput Filename: " << in_filename_param << '\n';
    std::cout << "Output Filename: " << out_filename_param << '\n';
    std::cout << "Num Times File Repeated: " << int(num_writes) << '\n';
    std::cout << "Num Samples Written: " << human_format(total_samples * 2) << '\n';
    std::cout.flush();

    return 0;
}