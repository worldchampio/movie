#include <string>

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
    std::string storage(std::size_t bytes);

    template<class T>
    std::string typeName(const T& value) 
    {
        const auto rawName{std::string{typeid(value).name()}};
        if(rawName.find("string")!=std::string::npos)
            return "string";
        else if(rawName=="c")
            return "char";
        else if(rawName=="i")
            return "integer";
        else if(rawName=="f")
            return "float";
        else if(rawName=="d")
            return "double";

        return rawName;
    }

    template<class T>
    class Queue
    {
    public:
        Queue(std::size_t size) : m_size{size} {}
        void add(T element)
        {
            buf.push_back(element);
            if(buf.size()>m_size)
                buf.erase(buf.begin());
        }
        T get(int i) {
            return i<m_size ? buf[i] : T{};
        }
        auto begin() { return buf.begin(); }
        auto end() { return buf.end(); }
        auto get() { return buf; }
        auto size() { return m_size; }
    private:
        std::vector<T> buf;
        std::size_t m_size{0};
    };
}