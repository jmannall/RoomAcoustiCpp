/*
* @class Debug
*
* @brief Declaration of Debug class
*
* @remarks Thanks to: https://stackoverflow.com/questions/43732825/use-debug-log-from-c
*/

// C++ headers
#include<cstdio>
#include <string>
#include <sstream>

// Commone headers
#include "Common/Debug.h"
#include "Common/Vec3.h"

//////////////////// Callback instances ////////////////////

FuncDebugCallback debugCallbackInstance = nullptr;
FuncPathCallback pathCallbackInstance = nullptr;
FuncResidueCallback residueCallbackInstance = nullptr;

std::mutex debugMutex;		// Protects debugCallbackInstance
std::mutex pathMutex;		// Protects pathCallbackInstance
std::mutex residueMutex;	// Protects residueCallbackInstance

namespace RAC
{
    namespace Common
    {
        //////////////////// Debug class ////////////////////

        ////////////////////////////////////////

        std::string GetRelativePath(std::string_view fullPath)
        {
            static constexpr std::string_view marker = R"(\RoomAcoustiCpp\)";

            size_t pos = fullPath.find(marker);
            if (pos == std::string_view::npos)
                return std::string(fullPath);

            // slice after the marker
            return std::string(fullPath.substr(pos + marker.size()));
        }

        ////////////////////////////////////////

        void Debug::WriteToLog(const std::string_view message, DebugType type, const std::source_location& location)
        {
            std::string path = GetRelativePath(location.file_name());

                // Precompute size to avoid reallocations
            std::string formatted;
            formatted.reserve(
                message.size()
                + path.size()
				+ 20 // size for line number
                + 32 // constant overhead for fixed text
            );

            formatted.append(message);
            formatted.append(" (File: ");
            formatted.append(path);
            formatted.append(", Line: ");
            formatted.append(ToString(location.line()));
            formatted.push_back(')');

            std::lock_guard lock(debugMutex);
            const char* tmsg = formatted.c_str();
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(tmsg, (int)type, (int)strlen(tmsg));
        }

        ////////////////////////////////////////
#ifdef DEBUG_PATHS
        void Debug::SendPath(const std::string_view key, const CustomVec3& intersection, const ::Common::CVector3& position)
        {
            Vec3 vec3Position(static_cast<Real>(position.x), static_cast<Real>(position.y), static_cast<Real>(position.z));
            WriteToSendPath(key, { intersection }, vec3Position);
        }

        ////////////////////////////////////////

        void Debug::SendPath(const std::string_view key, const CustomVec3& intersection, const CustomVec3& position)
        {
            WriteToSendPath(key, { intersection }, position);
        }

        ////////////////////////////////////////

        void Debug::SendPath(const std::string_view key, const std::vector<CustomVec3>& intersections, const ::Common::CVector3& position)
        {
            Vec3 vec3Position(static_cast<Real>(position.x), static_cast<Real>(position.y), static_cast<Real>(position.z));
            WriteToSendPath(key, intersections, vec3Position);
        }

        ////////////////////////////////////////

        void Debug::WriteToSendPath(const std::string_view key, const std::vector<CustomVec3>& intersections, const CustomVec3& position)
        {
            std::lock_guard lock(pathMutex);
            int length = ToInt(key.size());
            const char* tmsg = key.data();
            
            // Convert intersections to an array format
            size_t size = intersections.size() + 1;
            std::vector<float> intersectionArray;
            intersectionArray.reserve(3 * size);

            int count = 0;
            for (size_t i = 0; i < size - 1; ++i)
            {
                intersectionArray[count++] = static_cast<float>(intersections[i].x());
                intersectionArray[count++] = static_cast<float>(intersections[i].y());
                intersectionArray[count++] = static_cast<float>(intersections[i].z());
            }
            intersectionArray[count++] = static_cast<float>(position.x());
            intersectionArray[count++] = static_cast<float>(position.y());
            intersectionArray[count++] = static_cast<float>(position.z());

            if (pathCallbackInstance != nullptr)
                pathCallbackInstance(tmsg, &intersectionArray[0], length, ToInt(size) );
        }

        ////////////////////////////////////////

        void Debug::RemovePath(const std::string_view key)
        {
            std::lock_guard lock(pathMutex);
            int length = ToInt(key.size());
            const char* tmsg = key.data();

            float intersectionArray = 0.0f;

            if (pathCallbackInstance != nullptr)
                pathCallbackInstance(tmsg, &intersectionArray, length, 0);
        }
#endif
        ////////////////////////////////////////
#ifdef RESIDUE_CALLBACKS
        void Debug::SendResidue(float residue, bool isSource, int sourceIndex, int slopeIndex)
        {
            std::lock_guard lock(residueMutex);
            if (residueCallbackInstance != nullptr)
                residueCallbackInstance(residue, isSource, sourceIndex, slopeIndex);
        }
#endif
        ////////////////////////////////////////

        template<>
        std::string ToString<CustomVec3>(const CustomVec3& x)
        {
            return std::string('[' + ToString(x.x()) + ", " + ToString(x.y()) + ", " + ToString(x.z()) + ']');
        }

        ////////////////////////////////////////

        template<>
        std::string ToString<std::array<Vec3, 3>>(const std::array<Vec3, 3>& x)
        {
            return std::string(ToString(x[0]) + ToString(x[1]) + ToString(x[2]));
        }
    }
}

//////////////////// Create callback delegates ////////////////////

////////////////////////////////////////

void RegisterDebugCallback(FuncDebugCallback cb)
{
    std::lock_guard lock(debugMutex);
    debugCallbackInstance = cb;
}

////////////////////////////////////////

void RegisterPathCallback(FuncPathCallback cb)
{
    std::lock_guard lock(pathMutex);
    pathCallbackInstance = cb;
}

////////////////////////////////////////

void RegisterResidueCallback(FuncResidueCallback cb)
{
    std::lock_guard lock(residueMutex);
    residueCallbackInstance = cb;
}

////////////////////////////////////////

void UnregisterDebugCallback()
{
    std::lock_guard lock(debugMutex);
    debugCallbackInstance = nullptr;
}

////////////////////////////////////////

void UnregisterPathCallback()
{
    std::lock_guard lock(pathMutex);
    pathCallbackInstance = nullptr;
}

////////////////////////////////////////

void UnregisterResidueCallback()
{
    std::lock_guard lock(residueMutex);
    residueCallbackInstance = nullptr;
}