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

// Unity headers
#include "Common/Debug.h"

namespace RAC
{
    namespace Common
    {
        //////////////////// Debug class ////////////////////

        ////////////////////////////////////////
#ifdef DEBUG_LOG
        void Debug::Log(const char* message, Colour colour)
        {
            std::lock_guard lock(debugMutex);
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(message, (int)colour, (int)strlen(message));
        }

        ////////////////////////////////////////

        void Debug::Log(const std::string message, Colour colour)
        {
            std::lock_guard lock(debugMutex);
            const char* tmsg = message.c_str();
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(tmsg, (int)colour, (int)strlen(tmsg));
        }
#endif
        ////////////////////////////////////////
#ifdef DEBUG_PATHS
        void Debug::SendPath(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position)
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
#ifdef DEBUG_RESIDUES
        void Debug::SendResidue(float residue, bool isSource, int sourceIndex, int slopeIndex)
        {
            std::lock_guard lock(residueMutex);
            if (residueCallbackInstance != nullptr)
                residueCallbackInstance(residue, isSource, sourceIndex, slopeIndex);
        }
#endif
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