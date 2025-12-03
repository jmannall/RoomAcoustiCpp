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

namespace RAC
{
    namespace Common
    {
        //////////////////// Debug class ////////////////////

        ////////////////////////////////////////

        std::string GetRelativePath(std::string fullPath)
        {
            std::string path(fullPath);
            const std::string marker = "\\RoomAcoustiCpp\\";

            size_t pos = path.find(marker);
            if (pos != std::string::npos)
            {
                // Keep everything *after* RoomAcoustiCpp
                return path.substr(pos + marker.size());
            }

            // Marker not found, return original path
            return path;
        }

        ////////////////////////////////////////

        void Debug::WriteToLog(const std::string& message, DebugType type, const std::source_location& location)
        {
            std::string formatted = message;

            formatted += " (File: ";
            formatted += GetRelativePath(location.file_name());
            // formatted += ", Function: ";
			// formatted += location.function_name();
            formatted += ", Line: ";
            formatted += ToString(location.line());
            formatted += ")";

            std::lock_guard lock(debugMutex);
            const char* tmsg = formatted.c_str();
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(tmsg, (int)type, (int)strlen(tmsg));
        }

        ////////////////////////////////////////
#ifdef DEBUG_PATHS
        void Debug::SendPath(const std::string& key, const CustomVec3& intersection, const ::Common::CVector3& position)
        {
            Vec3 vec3Position(static_cast<Real>(position.x), static_cast<Real>(position.y), static_cast<Real>(position.z));
            WriteToSendPath(key, { intersection }, vec3Position);
        }

        ////////////////////////////////////////

        void Debug::SendPath(const std::string& key, const CustomVec3& intersection, const CustomVec3& position)
        {
            WriteToSendPath(key, { intersection }, position);
        }

        ////////////////////////////////////////

        void Debug::SendPath(const std::string& key, const std::vector<CustomVec3>& intersections, const ::Common::CVector3& position)
        {
            Vec3 vec3Position(static_cast<Real>(position.x), static_cast<Real>(position.y), static_cast<Real>(position.z));
            WriteToSendPath(key, intersections, vec3Position);
        }

        ////////////////////////////////////////

        void Debug::WriteToSendPath(const std::string& key, const std::vector<CustomVec3>& intersections, const CustomVec3& position)
        {
            std::lock_guard lock(pathMutex);
            const char* tmsg = key.c_str();
            
            // Convert intersections to an array format
            size_t size = intersections.size() + 1;
            float* intersectionArray = new float[3 * size];
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
                pathCallbackInstance(tmsg, &intersectionArray[0], ToInt(strlen(tmsg)), ToInt(size) );

            delete[] intersectionArray;
        }

        ////////////////////////////////////////

        void Debug::RemovePath(const std::string& key)
        {
            std::lock_guard lock(pathMutex);
            const char* tmsg = key.c_str();

            float intersectionArray = 0.0f;

            if (pathCallbackInstance != nullptr)
                pathCallbackInstance(tmsg, &intersectionArray, (int)strlen(tmsg), 0);
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
        std::string ToString<std::array<Vec3, 3>>(const std::array<Vec3, 3>& x)
        {
            std::stringstream ss;
            for (auto i = 0; i < x.size(); i++)
                ss << x[i];
            return ss.str();
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