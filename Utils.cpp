#include "Utils.h"
#include <cmath>
#include <random>
#include <sstream>
#include <chrono>

auto getYear()
{
    const std::chrono::time_point now{std::chrono::system_clock::now()};
    const std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(now)};
    return static_cast<int>(ymd.year());
}

bool Utils::validYear(int year)
{ 
    return year > 1900 && year <= getYear();
}

bool Utils::validAscii(char c) 
{
    return c > 31 && c < 127;
}

bool Utils::backspace(char c)
{
    return c == 127 || c == '\b';
}

std::pair<double,double> Utils::computeElo(double Ra, double Rb, bool victor)
{
    constexpr auto K{32};
    const auto Ea = 1 / ( 1 + pow(10, ( Rb - Ra ) / 400) ); 
    const auto Eb = 1 / ( 1 + pow(10, ( Ra -Rb ) / 400) );
    const auto Ra_ = Ra + K * (victor - Ea);
    const auto Rb_ = Rb + K * (!victor - Eb);
    return { Ra_, Rb_ };
}

int Utils::rng(int min, int max) 
{
    std::random_device  dev;
    std::mt19937        rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(min,max);
    return dist6(rng);
}

bool Utils::stringEquals(std::string a, std::string b)
{
    const auto asciitolower{[](char in) -> char 
    { 
        return (in <= 'Z' && in >= 'A') ? in - ('Z' - 'z') : in;
    }};
    std::transform(a.begin(), a.end(), a.begin(), asciitolower);
    std::transform(b.begin(), b.end(), b.begin(), asciitolower);
    return a.find(b) != std::string::npos;
}

std::pair<int,int> Utils::getTwoRngs(int min,int max)
{
    const auto firstRng{rng(min,max)};    
    int secondRng{firstRng};
    while(secondRng==firstRng)
        secondRng=rng(min,max);
    
    return {firstRng, secondRng};
}

int Utils::wrapAround(int val, int min, int max)
{
    return val < min ? max : val > max ? min : val;
}

std::vector<std::string> Utils::tokenize(const std::string& str, char delimiter)
{    
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss{str};
    while(std::getline(ss,token,delimiter))
        tokens.push_back(token);
    return tokens;
}

std::string Utils::timeStamp()
{
    auto end = std::chrono::system_clock::now();
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);
    return std::ctime(&end_time);
}

Utils::Timer::Timer() : 
    timeStart{std::chrono::high_resolution_clock::now()}
{}

std::string Utils::Timer::get()
{
    const std::chrono::duration<double,std::milli> duration{(std::chrono::high_resolution_clock::now() - timeStart)};
    const auto count{duration.count()};
    return std::to_string(count) + "ms";
}