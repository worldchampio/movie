#include "Utils.h"
#include <cmath>
#include <random>

bool Utils::validYear(int year)
{ 
    return year > 1900 && year < 2024;
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

std::pair<int,int> Utils::getTwoRngs(int max)
{
    const auto firstRng{rng(0,max)};    
    int secondRng{firstRng};
    while(secondRng==firstRng)
        secondRng=rng(0,max);
    
    return {firstRng, secondRng};
}

int Utils::wrapAround(int val, int min, int max)
{
    if(val < min)
        val = max;
    if(val > max)
        val = min;
    
    return val;
}
