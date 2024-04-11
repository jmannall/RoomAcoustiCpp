/*
*
*  \Unity interface Debug class
*
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

// Common headers
#include "Common/Vec3.h"

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
    // UI_EXPORT void UI_API RegisterDebugCallback(FuncCallBack cb);
}

namespace RAC
{
    using namespace Common;
    namespace Unity
    {
        //////////////////// Colour enum ////////////////////

        enum class Colour { Red, Green, Blue, Black, White, Yellow, Orange };

        //////////////////// Debug class ////////////////////

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

        using namespace std;

        //////////////////// Functions ////////////////////

        inline string IntToStr(int x)
        {
            stringstream ss;
            ss << x;
            return ss.str();
        }

        inline string IntToStr(size_t x)
        {
            return IntToStr(static_cast<int>(x));
        }

        inline string FloatToStr(float x)
        {
            stringstream ss;
            ss << x;
            return ss.str();
        }

        inline string RealToStr(Real x)
        {
            stringstream ss;
            ss << x;
            return ss.str();
        }

        inline string BoolToStr(bool x)
        {
            if (x)
                return "true";
            else
                return "false;";
        }

        inline string VecToStr(const vec3& x)
        {
            stringstream ss;
            ss << x;
            return ss.str();
        }

        inline string VecArrayToStr(const std::vector<vec3>& x)
        {
            stringstream ss;
            string output;
            for (int i = 0; i < x.size(); i++)
            {
                ss << x[i];
            }
            return ss.str();
        }
    }
}
#endif