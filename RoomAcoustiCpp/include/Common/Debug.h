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
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>

// Common headers
#include "Common/Coefficients.h"

// Spatialiser headers
#include "Spatialiser/Types.h"

// 3DTI headers
#include "Common/Vector3.h"

/**
* @brief Define DLL linkage
*/
#ifdef _ANDROID
#define DLLExport __attribute__ ((visibility ("default")))
#elif _WINDOWS
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

/**
* @brief Mutexes to protect callback instances
*/
static std::mutex debugMutex;		// Protects debugCallbackInstance
static std::mutex pathMutex;		// Protects pathCallbackInstance
static std::mutex residueMutex;		// Protects residueCallbackInstance

extern "C"
{
    /**
	* @brief Define callback function types
    */
    typedef void(*FuncDebugCallback)(const char* message, int colour, int size);
    typedef void(*FuncPathCallback)(const char* key, const float* intersections, int size, int numIntersections);
    typedef void(*FuncResidueCallback)(float residue, bool isSource, int sourceIndex, int slopeIndex);

    /**
	* @brief Create callback instances
    */
    static FuncDebugCallback debugCallbackInstance = nullptr;
    static FuncPathCallback pathCallbackInstance = nullptr;
    static FuncResidueCallback residueCallbackInstance = nullptr;

    /**
    * @brief Functions to register callback instances
    */
    DLLExport void RegisterDebugCallback(FuncDebugCallback cb);
    DLLExport void RegisterPathCallback(FuncPathCallback cb);
    DLLExport void RegisterResidueCallback(FuncResidueCallback cb);

    /**
    * @brief Functions to unregister callback instances
    */
    DLLExport void UnregisterDebugCallback();
    DLLExport void UnregisterPathCallback();
    DLLExport void UnregisterResidueCallback();
}

namespace RAC
{
    using namespace Spatialiser;
    namespace Common
    {
        enum class Colour { Red, Green, Blue, Black, White, Yellow, Orange };

        /**
		* @brief Provides a simple interface for logging debug messages to a callback function
        */
        class Debug
        {
        public:
#ifdef DEBUG_LOG
            static void Log(const char* message, Colour colour = Colour::Black);
            static void Log(const std::string message, Colour colour = Colour::Black);
#else
            static inline void Log(const char* message, Colour colour = Colour::Black) {}
            static inline void Log(const std::string message, Colour colour = Colour::Black) {}
#endif

            static inline void SendPath(const std::string& key, const std::vector<Vec3>& intersections, const ::Common::CVector3& position)
            {
                Vec3 vec3Position(static_cast<Real>(position.x), static_cast<Real>(position.y), static_cast<Real>(position.z));
				SendPath(key, intersections, vec3Position);
            }

#ifdef DEBUG_PATHS
            static void SendPath(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position);
            static void RemovePath(const std::string& key);
#else
            static inline void SendPath(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position) {}
            static inline void RemovePath(const std::string& key) {}
#endif

#ifdef DEBUG_RESIDUES
            static void SendResidue(float residue, bool isSource, int sourceIndex, int slopeIndex);
#else
            static inline void SendResidue(float residue, bool isSource, int sourceIndex, int slopeIndex) {}
#endif

        private:
        };

        /**
        * @brief Converts a type T to a string
        */
        template <typename T>
        std::string ToString(const T& value)
        {
            std::stringstream ss;
            ss << value;
            return ss.str();
        }

        /**
        * @brief Converts a bool to a string
        */
        template <>
        inline std::string ToString<bool>(const bool& x)
        {
            return x ? "true" : "false";
        }

        /**
        * @brief Converts a Vertices to a string
        */
        template <>
        inline std::string ToString<Vertices>(const Vertices& x)
        {
            std::stringstream ss;
            for (int i = 0; i < x.size(); i++)
                ss << x[i];
            return ss.str();
        }
    }
}
#endif