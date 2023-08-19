#include <string>
#include <utility>

namespace Utils
{
    struct Position{ int y; int x; };

    int wrapAround(int val, int min, int max);
    int rng(int min, int max);
    bool validYear(int year);
    bool stringEquals(std::string a, std::string b);
    std::pair<double,double> computeElo(double Ra, double Rb, bool victor);
    std::pair<int,int> getTwoRngs(int max);
}