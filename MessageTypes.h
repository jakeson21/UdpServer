/*
 * MessageTypes.h
 *
 *  Created on: Feb 7, 2016
 *      Author: fuguru
 */

#ifndef MESSAGETYPES_H_
#define MESSAGETYPES_H_

namespace Comm
{
    enum MESSAGE_TYPE
    {
        binary=1,
        boolean,
        character,
        string,
        integer,
        integer64,
        float_single,
        float_double,
    };

    static std::string toString(const MESSAGE_TYPE type)
    {
        std::string str;
        switch(type)
        {
            case MESSAGE_TYPE::binary:
                str = "binary";
                break;

            case MESSAGE_TYPE::boolean:
                str = "boolean";
                break;

            case MESSAGE_TYPE::character:
                str = "character";
                break;

            case MESSAGE_TYPE::string:
                str = "string";
                break;

            case MESSAGE_TYPE::integer:
                str = "integer";
                break;

            case MESSAGE_TYPE::integer64:
                str = "integer64";
                break;

            case MESSAGE_TYPE::float_single:
                str = "float_single";
                break;

            case MESSAGE_TYPE::float_double:
                str = "float_double";
                break;

            default:
                str.clear();
        }
        return str;
    }

}



#endif /* MESSAGETYPES_H_ */
