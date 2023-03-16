#include "ncurses.h"
#include <string>
#include <vector>
#include <fstream>
#include <optional> 
class Movies{
public:
    Movies();
    ~Movies();
    struct Movie
    {
        double rating{};
        std::string name;
        int year{};
    };

    void addMovie();
    void rateMovies(int i);
    void loadMovies();
    void browse();
    void search();
    std::string getStrInput(WINDOW* win, int y, int x);
    std::pair<int,int> getTwoRngs();
    std::pair<double,double> computeElo(double Ra, double Rb, bool score);
private:
    bool stringEquals(std::string a, std::string b);
    int rng(int min, int max);
    Movie deserialize(const std::string& str);
    std::string serialize(const Movie& movie);
    
    std::fstream moviefile;
    std::vector<Movie> movies;
};