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
    void initColors();
    void navigationBar(int maxPos);
    void addMovie();
    void rateMovies();
    void browse();
    void recommend();
    void search();
    void about();
    
    Movie highestRatedMovie();
    Movie deserialize(const std::string& str);
    std::string serialize(const Movie& movie);
    std::string displayString(const Movie& movie, const std::string& preStr = "");
    std::string getStrInput(WINDOW* win, int y, int x);
    bool stringEquals(std::string a, std::string b);
    std::pair<Movie,double> highestDiffMovie();
    std::pair<double,double> computeElo(double Ra, double Rb, bool score);
    std::pair<int,int> getTwoRngs();
    int rng(int min, int max);

    std::fstream moviefile;
    std::vector<Movie> movies;
    std::unordered_map<int,double> ratedMovies;
    int globalWidth{90};
};