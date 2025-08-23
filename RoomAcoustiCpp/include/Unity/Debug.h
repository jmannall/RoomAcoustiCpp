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

// Common headers
#include "Common/Coefficients.h"

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
static std::mutex residueMutex;		// Protects residueCallbackInstance
static std::mutex iemStartMutex;	// Protects iemStartCallbackInstance
static std::mutex iemEndMutex;	    // Protects iemEndCallbackInstance
static std::mutex rtmStartMutex;	// Protects rtmStartCallbackInstance
static std::mutex rtmEndMutex;	    // Protects rtmEndCallbackInstance

extern "C"
{
    //Create a callback delegate
    typedef void(*FuncDebugCallback)(const char* message, int colour, int size);
    typedef void(*FuncPathCallback)(const char* key, const float* intersections, int size, int numIntersections);
    typedef void(*FuncResidueCallback)(float residue, bool isSource, int channelIndex, int slopeIndex);
    typedef void(*FuncIEMStartCallback)();
    typedef void(*FuncIEMEndCallback)();
    typedef void(*FuncRTMStartCallback)();
    typedef void(*FuncRTMEndCallback)();
    static FuncDebugCallback debugCallbackInstance = nullptr;
    static FuncPathCallback pathCallbackInstance = nullptr;
    static FuncResidueCallback residueCallbackInstance = nullptr;
    static FuncIEMStartCallback iemStartCallbackInstance = nullptr;
    static FuncIEMEndCallback iemEndCallbackInstance = nullptr;
    static FuncRTMStartCallback rtmStartCallbackInstance = nullptr;
    static FuncRTMEndCallback rtmEndCallbackInstance = nullptr;
    DLLExport void RegisterDebugCallback(FuncDebugCallback cb);
    DLLExport void RegisterPathCallback(FuncPathCallback cb);
    DLLExport void RegisterResidueCallback(FuncResidueCallback cb);
    DLLExport void RegisterIEMStartCallback(FuncIEMStartCallback cb);
    DLLExport void RegisterIEMEndCallback(FuncIEMEndCallback cb);
    DLLExport void RegisterRTMStartCallback(FuncRTMStartCallback cb);
    DLLExport void RegisterRTMEndCallback(FuncRTMEndCallback cb);
    DLLExport void UnregisterDebugCallback();
    DLLExport void UnregisterPathCallback();
    DLLExport void UnregisterResidueCallback();
    DLLExport void UnregisterIEMStartCallback();
    DLLExport void UnregisterIEMEndCallback();
    DLLExport void UnregisterRTMStartCallback();
    DLLExport void UnregisterRTMEndCallback();
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

            static void send_path(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position);
            static void remove_path(const std::string& key);

            static void send_residue(float residue, bool isSource, int channelIndex, int slopeIndex);
            
            static void IEMStartFlag();
            static void IEMEndFlag();
            static void RTMStartFlag();
            static void RTMEndFlag();

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
            for (int i = 0; i < x.size(); i++)
                ss << x[i];
            return ss.str();
        }

        /**
        * @brief Converts a Vec3 to a string
        */
        inline std::string CoefficientToStr(const Coefficients<>& x)
        {
            std::stringstream ss;
            ss << x;
            return ss.str();
        }
    }
}
#endif