#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <string>

/**
 * @param str Has form "param_header param1 param2 param3 ..."
 * @param param_header Populated from the first token in str
 * @param buffer Populated from the remaining tokens in str
 * 
 * @return Number of parameters found.
*/
template <typename T>
int parse_parameter_str(std::string& str,
                         std::string& param_header,
                         T* buffer,
                         size_t buffer_size) {
    std::istringstream ss{str};
    std::string token;
    ss >> param_header;

    size_t count = 0;
    ss >> token;
    while (ss) {
        if (count == buffer_size) { return -1; }

        buffer[count++] = std::stod(token);
        ss >> token;
    }
    return count;
};

/* Parses string of form 'f 1//1 2//1 3//1' into two buffers */
int parseStrTwoBuff(std::string& str,
                    uint8_t* vBuff,
                    uint8_t* nBuff,
                    std::string pairDelim = "//",
                    uint8_t buff_sz = 3)
{
    std::istringstream ss{str};
    std::string token;
    ss >> token;  // reads 'vn'

    uint8_t count = 0;
    while (ss) {
        if (count == buff_sz) { assert(false); }

        uint8_t pos = token.find(pairDelim);
        std::string n1Str = token.substr(0, pos);
        std::string n2Str = token.substr(pos+pairDelim.size(),
                                         std::string::npos);
        vBuff[count] = std::stoi(n1Str);
        nBuff[count] = std::stoi(n2Str);
        count++;
    }
    return count;
}

#endif
