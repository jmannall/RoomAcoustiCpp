/*
*
*  \Unity interface Debug class
*
*/

// C++ headers
#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>

// UNity headers
#include "Unity/Debug.h"

namespace UIE
{
    namespace Unity
    {
        //////////////////// Debug class ////////////////////

        void  Debug::Log(const char* message, Colour colour)
        {
            if (callbackInstance != nullptr)
                callbackInstance(message, (int)colour, (int)strlen(message));
        }

        void  Debug::Log(const std::string message, Colour colour)
        {
            const char* tmsg = message.c_str();
            if (callbackInstance != nullptr)
                callbackInstance(tmsg, (int)colour, (int)strlen(tmsg));
        }

        void  Debug::Log(const int message, Colour colour)
        {
            std::stringstream ss;
            ss << message;
            send_log(ss, colour);
        }

        void  Debug::Log(const char message, Colour colour)
        {
            std::stringstream ss;
            ss << message;
            send_log(ss, colour);
        }

        void  Debug::Log(const float message, Colour colour)
        {
            std::stringstream ss;
            ss << message;
            send_log(ss, colour);
        }

        void  Debug::Log(const double message, Colour colour)
        {
            std::stringstream ss;
            ss << message;
            send_log(ss, colour);
        }

        void Debug::Log(const bool message, Colour colour)
        {
            std::stringstream ss;
            if (message)
                ss << "true";
            else
                ss << "false";

            send_log(ss, colour);
        }

        void Debug::send_log(const std::stringstream& ss, const Colour& colour)
        {
            const std::string tmp = ss.str();
            const char* tmsg = tmp.c_str();
            if (callbackInstance != nullptr)
                callbackInstance(tmsg, (int)colour, (int)strlen(tmsg));
        }
    }
}

//////////////////// Functions ////////////////////

// Create a callback delegate
void RegisterDebugCallback(FuncCallBack cb)
{
    callbackInstance = cb;
}