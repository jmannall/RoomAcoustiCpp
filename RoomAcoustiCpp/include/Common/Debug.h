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
#include <source_location>

// 3DTI headers
#include "Common/Vector3.h"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>   // IsDebuggerPresent
#include <intrin.h>    // __debugbreak
#endif

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
    namespace Common
    {
		// Predeclare CustomVec3 to avoid circular dependency from including Common/Vec3.h
        class CustomVec3;

        enum class DebugType { Error, Init, Update, Remove, Parameter, Warning, Assert, External };

        /**
		* @brief Provides a simple interface for logging debug messages to a callback function
        */
        class Debug
        {
        public:

#if defined(DEBUG_ASSERT)
            /**
			* @brief Asserts a condition and logs an error message if the assertion fails
			* @details If the assertion fails, the debugger hits a break point and a runtime_error exception is thrown.
            * 
			* @param result The result of the assertion (true if the assertion passes, false otherwise)
			* @param message The message to log if the assertion fails
			* @param location The source location of the assert call (filename and line number appended to message)
            */
            template<typename T>
            static inline void Assert(bool result, const T message, const std::source_location& location = std::source_location::current())
            {
                if (result)
                    return;

                WriteToLog(message, DebugType::Assert, location);
#ifdef _WIN32
                if (IsDebuggerPresent())
                    __debugbreak();
#endif
                // throw std::runtime_error(message);
            }

			#define RAC_DEBUG_ASSERT(result, message)        Debug::Assert((result), (message))
#else
			#define RAC_DEBUG_ASSERT(result, message)        (void)0
#endif

#if defined(DEBUG_LOG)
            /**
			* @brief Logs a message to the debug log callback
            * 
			* @param message The message to log
			* @param type The type of debug message
			* @param location The source location of the log call (filename and line number appended to message)
            */
            template<typename T>
            static inline void Log(const T message, DebugType type, const std::source_location& location = std::source_location::current())
            {
                Debug::WriteToLog(message, type, location);
            }

            #define RAC_DEBUG_LOG(message, type)        Debug::Log((message), (type))
#else
            #define RAC_DEBUG_LOG(message, type)        (void)0
#endif

#if defined(DEBUG_PATHS)

            /**
            * @brief Sends a path to the path callback
            *
            * @param key The unique key for the path
            * @param intersection The intersection point of the path
            * @param position The starting position of the path (as a CVector3)
            */
            static void SendPath(const std::string& key, const CustomVec3& intersection, const ::Common::CVector3& position);

            /**
            * @brief Sends a path to the path callback
            *
            * @param key The unique key for the path
            * @param intersection The intersection point of the path
            * @param position The starting position of the path (as a Vec3)
            */
            static void SendPath(const std::string& key, const CustomVec3& intersection, const CustomVec3& position);

            /**
			* @brief Sends a path to the path callback
            * 
			* @param key The unique key for the path
			* @param intersections The intersection points of the path
			* @param position The starting position of the path (as a CVector3)
            */
            static void SendPath(const std::string& key, const std::vector<CustomVec3>& intersections, const ::Common::CVector3& position);

            /**
            * @brief Sends a path to the path callback
            *
            * @param key The unique key for the path
            * @param intersections The intersection points of the path
            * @param position The starting position of the path (as a Vec3)
            */
            static inline void SendPath(const std::string& key, const std::vector<CustomVec3>& intersections, const CustomVec3& position)
            {
				WriteToSendPath(key, intersections, position);
            }

            /**
			* @brief Sends a removes a path message to the path callback
            * 
			* @param key The unique key for the path to remove
            */
            static void RemovePath(const std::string& key);

            #define RAC_DEBUG_SENDPATH(key, intersections, position)        Debug::SendPath((key), (intersections), (position))
            #define RAC_DEBUG_REMOVEPATH(key)                               Debug::RemovePath((key))
#else
            #define RAC_DEBUG_SENDPATH(key, intersections, position)        (void)0            
            #define RAC_DEBUG_REMOVEPATH(key)                               (void)0
#endif

#if defined(RESIDUE_CALLBACKS)
            /**
			* @brief Sends a residue value to the residue callback
            * 
			* @param residue The residue value to send
			* @param isSource True if the residue is for a source, false if for a listener
			* @param sourceIndex The index of the source or listener
			* @param slopeIndex The index of the slope (corresponding to the FDN)
            */
            static void SendResidue(float residue, bool isSource, int sourceIndex, int slopeIndex);

            #define RAC_DEBUG_SENDRESIDUE(residue, isSource, sourceIndex, slopeIndex)        Debug::SendResidue((residue), (isSource), (sourceIndex), (slopeIndex))
#else
            #define RAC_DEBUG_SENDRESIDUE(residue, isSource, sourceIndex, slopeIndex)        (void)0
#endif

        private:

            /**
            * @brief Internal function to write messages to the debug log
            *
			* @param message The message to log (as a C-style string)
            * @param type The type of debug message
            * @param location The source location of the log call (filename and line number appended to message)
            */
            static inline void WriteToLog(const char* message, DebugType type, const std::source_location& location = std::source_location::current())
            {
				WriteToLog(std::string(message), type, location);
            }

            /**
			* @brief Internal function to write messages to the debug log
            * 
			* @param message The message to log (as a C++ string)
			* @param type The type of debug message
			* @param location The source location of the log call (filename and line number appended to message)
            */
            static void WriteToLog(const std::string& message, DebugType type, const std::source_location& location = std::source_location::current());

            static void WriteToSendPath(const std::string& key, const std::vector<CustomVec3>& intersections, const CustomVec3& position);
        };

        /**
		* @brief Stream buffer that sends output to the debug log
		* @details Used to redirect 3DTI messages to the debug log
        */
        class DebugLogStreamBuffer : public std::streambuf
        {
        public:
            DebugLogStreamBuffer() {}

        protected:
            int overflow(int ch) override
            {
                if (ch != EOF)
                {
                    buffer += static_cast<char>(ch);

                    // When a newline appears, send message
                    if (ch == '\n')
                    {
                        RAC_DEBUG_LOG(buffer.c_str(), DebugType::External);
                        buffer.clear();
                    }
                }
                return ch;
            }

            int sync() override
            {
                if (!buffer.empty())
                {
                    RAC_DEBUG_LOG(buffer.c_str(), DebugType::External);
                    buffer.clear();
                }
                return 0;
            }

        private:
            std::string buffer;
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
		* @details Defined in cpp to avoid circular dependency on Vec3 and Vertices headers
        */
        template <>
        std::string ToString<std::array<CustomVec3, 3>>(const std::array<CustomVec3, 3>& x);

        template <>
        inline std::string ToString<DebugType>(const DebugType& type)
        {
            switch (type)
            {
            case DebugType::Error: return "Error";
            case DebugType::Init: return "Init";
            case DebugType::Update: return "Update";
            case DebugType::Remove: return "Remove";
            case DebugType::Parameter: return "Parameter";
            case DebugType::Warning: return "Warning";
            case DebugType::Assert: return "Assert";
            case DebugType::External: return "External";
            default: return "Unknown";
            }
        }
    }
}
#endif