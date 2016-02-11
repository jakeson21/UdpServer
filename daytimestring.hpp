/*
 * daytimestring.hpp
 *
 *  Created on: Feb 9, 2016
 *      Author: fuguru
 */

#ifndef DAYTIMESTRING_HPP_
#define DAYTIMESTRING_HPP_

namespace Utils {
    static std::string make_daytime_string()
    {
        using namespace std; // For time_t, time and ctime;
        time_t now = time(0);
        return ctime(&now);
    }
}

#endif /* DAYTIMESTRING_HPP_ */
