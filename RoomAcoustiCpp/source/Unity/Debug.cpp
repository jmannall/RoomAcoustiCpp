/*
*
*  \Unity interface Debug class
*
*/

#ifdef USE_UNITY_DEBUG

// C++ headers
#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>

// UNity headers
#include "Unity/Debug.h"

namespace RAC
{
    namespace Unity
    {
        //////////////////// Debug class ////////////////////

        void  Debug::Log(const char* message, Colour colour)
        {
            std::lock_guard lock(debugMutex);
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(message, (int)colour, (int)strlen(message));
        }

        void  Debug::Log(const std::string message, Colour colour)
        {
            std::lock_guard lock(debugMutex);
            const char* tmsg = message.c_str();
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(tmsg, (int)colour, (int)strlen(tmsg));
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
            std::lock_guard lock(debugMutex);
            const std::string tmp = ss.str();
            const char* tmsg = tmp.c_str();
            if (debugCallbackInstance != nullptr)
                debugCallbackInstance(tmsg, (int)colour, (int)strlen(tmsg));
        }

        void Debug::send_path(const std::string& key, const std::vector<Vec3>& intersections, const Vec3& position)
        {
            std::lock_guard lock(pathMutex);
            const char* tmsg = key.c_str();
            
            // Convert intersections to an array format
            size_t size = intersections.size() + 1;
            float* intersectionArray = new float[3 * size];
            int count = 0;
            for (size_t i = 0; i < size - 1; ++i)
            {
                intersectionArray[count++] = intersections[i].x;
                intersectionArray[count++] = intersections[i].y;
                intersectionArray[count++] = intersections[i].z;
            }
            intersectionArray[count++] = position.x;
            intersectionArray[count++] = position.y;
            intersectionArray[count++] = position.z;

            if (pathCallbackInstance != nullptr)
                pathCallbackInstance(tmsg, &intersectionArray[0], (int)strlen(tmsg), (int)size);

            delete[] intersectionArray;
        }

        void Debug::remove_path(const std::string& key)
        {
            std::lock_guard lock(pathMutex);
            const char* tmsg = key.c_str();

            float intersectionArray = 0.0f;

            if (pathCallbackInstance != nullptr)
                pathCallbackInstance(tmsg, &intersectionArray, (int)strlen(tmsg), 0);
        }

		void Debug::IEMFlag(int id)
		{
            std::lock_guard lock(iemMutex);
			if (iemCallbackInstance != nullptr)
				iemCallbackInstance(id);
		}
    }
}

//////////////////// Functions ////////////////////

// Create a callback delegate
void RegisterDebugCallback(FuncDebugCallback cb)
{
    std::lock_guard lock(debugMutex);
    debugCallbackInstance = cb;
}

void RegisterPathCallback(FuncPathCallback cb)
{
	std::lock_guard lock(pathMutex);
	pathCallbackInstance = cb;
}

void RegisterIEMCallback(FuncIEMCallback cb)
{
    std::lock_guard lock(iemMutex);
    iemCallbackInstance = cb;
}

void UnregisterDebugCallback()
{
    std::lock_guard lock(debugMutex);
    debugCallbackInstance = nullptr;
}

void UnregisterPathCallback()
{
	std::lock_guard lock(pathMutex);
	pathCallbackInstance = nullptr;
}

void UnregisterIEMCallback()
{
    std::lock_guard lock(iemMutex);
    iemCallbackInstance = nullptr;
}

#endif