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
#include <mutex>

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

static std::mutex debugMutex;		// Protects debugCallbackInstance
static std::mutex pathMutex;		// Protects pathCallbackInstance
static std::mutex iemMutex;		    // Protects iemCallbackInstance

extern "C"
{
    //Create a callback delegate
    typedef void(*FuncDebugCallback)(const char* message, int colour, int size);
	typedef void(*FuncPathCallback)(const char* key, const float* intersections, int size, int numIntersections);
    typedef void(*FuncIEMCallback)(int id);
    static FuncDebugCallback debugCallbackInstance = nullptr;
    static FuncPathCallback pathCallbackInstance = nullptr;
	static FuncIEMCallback iemCallbackInstance = nullptr;
    DLLExport void RegisterDebugCallback(FuncDebugCallback cb);
	DLLExport void RegisterPathCallback(FuncPathCallback cb);
    DLLExport void RegisterIEMCallback(FuncIEMCallback cb);
    DLLExport void UnregisterDebugCallback();
	DLLExport void UnregisterPathCallback();
    DLLExport void UnregisterIEMCallback();
}

namespace RAC
{
    using namespace Common;
    using namespace Spatialiser;
    namespace Unity
    {
        //////////////////// #defines ////////////////////

/**
* @brief Debugging flags for printing debug messages
*/
#define DEBUG_INIT
// #define DEBUG_UPDATE
#define DEBUG_REMOVE
// #define DEBUG_AUDIO_THREAD
// #define DEBUG_HRTF
// #define DEBUG_WALL
// #define DEBUG_IMAGE_SOURCE
#define DEBUG_IEM
#define DEBUG_GEOMETRY

/**
* @brief Debugging flags for completed iem model
*/
#define IEM_FLAG

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

            static void send_path(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position);
            static void remove_path(const std::string& key);

            static void IEMFlag(int id);

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