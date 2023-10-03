#ifndef UTIL_HPP
#define UTIL_HPP

#include <sstream>
#include <string>

/**
 * @param str Has form "param_header param1 param2 param3 ..."
 * @param param_header Populated from the first token in str
 * @param buffer Populated from the remaining tokens in str
 * 
 * @return 
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

#endif
