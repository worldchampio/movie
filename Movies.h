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
    void createMenu();
    void initColors();
    void eventLoop();

    void addMovie();
    void rateMovies();
    void browse();
    void recommend();
    void search();
    void about();
    void reset();
    
    Movie highestRatedMovie();
    Movie deserialize(const std::string& str);
    std::string serialize(const Movie& movie);
    std::string displayString(const Movie& movie, const std::string& preStr = "");
    std::string getStrInput(WINDOW* win, int y, int x);
    std::pair<Movie,double> highestDiffMovie();

    std::fstream moviefile;
    std::vector<Movie> movies;
    std::unordered_map<int,double> ratedMovies;

    std::unordered_map<std::string,int> ratingCache;
};