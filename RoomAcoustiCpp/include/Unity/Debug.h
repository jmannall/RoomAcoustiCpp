/*
* @class Debug
*
* @brief Declaration of Debug class
* 
* @remarks Thanks to: https://stackoverflow.com/questions/43732825/use-debug-log-from-c
*/

#ifndef Unity_Debug_h
#define Unity_Debug_h

// C++ headers
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>

// Unity headers
#include "Unity/IUnityInterface.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

#ifdef _ANDROID
#define DLLExport __attribute__ ((visibility ("default")))
#elif _WINDOWS
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

extern "C"
{
    //Create a callback delegate
    typedef void(*FuncCallBack)(const char* message, int colour, int size);
    static FuncCallBack callbackInstance = nullptr;
    DLLExport void RegisterDebugCallback(FuncCallBack cb);
}

namespace RAC
{
    using namespace Common;
    using namespace Spatialiser;
    namespace Unity
    {

        enum class Colour { Red, Green, Blue, Black, White, Yellow, Orange };

        /**
		* @brief Provides a simple interface for logging debug messages to the Unity console
        */
        class  Debug
        {
        public:
            static void Log(const char* message, Colour colour = Colour::Black);
            static void Log(const std::string message, Colour colour = Colour::Black);
            static void Log(const int message, Colour colour = Colour::Black);
            static void Log(const char message, Colour colour = Colour::Black);
            static void Log(const float message, Colour colour = Colour::Black);
            static void Log(const double message, Colour colour = Colour::Black);
            static void Log(const bool message, Colour colour = Colour::Black);

        private:
            static void send_log(const std::stringstream& ss, const Colour& colour);
        };

        /**
		* @brief Converts an int to a string
        */
        inline std::string IntToStr(int x)
        {
            std::stringstream ss;
            ss << x;
            return ss.str();
        }

        /**
        * @brief Converts a size_t to a string
        */
        inline std::string IntToStr(size_t x)
        {
            return IntToStr(static_cast<int>(x));
        }

        /**
        * @brief Converts a float to a string
        */
        inline std::string FloatToStr(float x)
        {
            std::stringstream ss;
            ss << x;
            return ss.str();
        }

        /**
        * @brief Converts a Real to a string
        */
        inline std::string RealToStr(Real x)
        {
            std::stringstream ss;
            ss << x;
            return ss.str();
        }

        /**
        * @brief Converts a bool to a string
        */
        inline std::string BoolToStr(bool x)
        {
            if (x)
                return "true";
            else
                return "false;";
        }

        /**
        * @brief Converts a Vec3 to a string
        */
        inline std::string VecToStr(const Vec3& x)
        {
            std::stringstream ss;
            ss << x;
            return ss.str();
        }

        /**
        * @brief Converts a Vertices to a string
        */
        inline std::string VerticesToStr(const Vertices& x)
        {
            std::stringstream ss;
            std::string output;
            for (int i = 0; i < x.size(); i++)
            {
                ss << x[i];
            }
            return ss.str();
        }
    }
}
#endif