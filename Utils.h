#include <string>
#include <utility>
#include <array>

#pragma once

namespace Utils
{
    class Timer
    {
        std::chrono::steady_clock::time_point timeStart;
    public:
        Timer();
        std::string get();
    };
    
    struct Position{ int y; int x; };

    int wrapAround(int val, int min, int max);
    int rng(int min, int max);
    bool validYear(int year);
    bool validAscii(char c);
    bool backspace(char c);
    bool stringEquals(std::string a, std::string b);
    std::pair<double,double> computeElo(double Ra, double Rb, bool victor);
    std::pair<int,int> getTwoRngs(int min, int max);
    std::vector<std::string> tokenize(const std::string& str, char delimiter = ',');
    std::string timeStamp();

    template<class T>
    class Queue
    {
    public:
        Queue(int size) : size{size} {}
        void add(T element)
        {
            buf.push_back(element);
            if(buf.size()>size)
                buf.erase(buf.begin());
        }
        T get(int i) {
            return i<size ? buf[i] : T{};
        }
        auto begin() { return buf.begin(); }
        auto end() { return buf.end(); }
    private:
        std::vector<T> buf;
        int size{0};
    };
}