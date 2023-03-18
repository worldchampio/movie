#include "ncurses.h"
#include <string>
#include <vector>
#include <fstream>
#include <optional> 
#include <unordered_map>

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
private:
    void loadMovies();
    int createMenu();
    void navigationBar(int maxPos);

    void addMovie();
    void rateMovies();
    void browse();
    void recommend();
    std::pair<Movie,double> highestDiffMovie();
    Movie highestRatedMovie();
    void search();
    void about();
    
    std::string getStrInput(WINDOW* win, int y, int x);
    std::pair<int,int> getTwoRngs();
    std::pair<double,double> computeElo(double Ra, double Rb, bool score);
    bool stringEquals(std::string a, std::string b);
    int rng(int min, int max);
    Movie deserialize(const std::string& str);
    std::string serialize(const Movie& movie);
    std::string displayString(const Movie& movie, const std::string& preStr = "");

    std::fstream moviefile;
    std::vector<Movie> movies;
    std::unordered_map<int,double> ratedMovies;
    int globalWidth{90};
};