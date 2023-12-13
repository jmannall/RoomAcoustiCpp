#pragma once
#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include "vec3.h""

#define DLLExport __declspec(dllexport)

extern "C"
{
    //Create a callback delegate
    typedef void(*FuncCallBack)(const char* message, int color, int size);
    static FuncCallBack callbackInstance = nullptr;
    DLLExport void RegisterDebugCallback(FuncCallBack cb);
}

//Color Enum
enum class Color { Red, Green, Blue, Black, White, Yellow, Orange };

class  Debug
{
public:
    static void Log(const char* message, Color color = Color::Black);
    static void Log(const std::string message, Color color = Color::Black);
    static void Log(const int message, Color color = Color::Black);
    static void Log(const char message, Color color = Color::Black);
    static void Log(const float message, Color color = Color::Black);
    static void Log(const double message, Color color = Color::Black);
    static void Log(const bool message, Color color = Color::Black);

private:
    static void send_log(const std::stringstream& ss, const Color& color);
};

using namespace std;
inline string IntToStr(int x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

inline string IntToStr(size_t x) {
    IntToStr((int)x);
}

inline string FloatToStr(float x) {
    stringstream ss;
    ss << x;
    return ss.str();
}

inline string BoolToStr(bool x) {
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