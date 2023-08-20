#include "ncurses.h"
#include <string>
#include <vector>
#include <fstream>
#include <optional> 
#include <unordered_map>

class Movies{
public:
    struct Movie
    {
        double rating{};
        std::string name;
        int year{};
    };

    struct Score
    {
        int score{};
        std::string timestamp;
    };

    Movies();
    ~Movies();

    struct MenuItem{
        std::string text;
        std::function<void()> fcn;
    };

    int execute();
private:
    void loadMovies();
    void loadHighscores();
    void createMenu();
    void initColors();

    void addMovie();
    void rateMovies();
    void browse();
    void recommend();
    void search();
    void snake();
    void reset();
    
    Movie highestRatedMovie();

    Movie deserializeMovie(const std::string& str);
    Score deserializeScore(const std::string& str);
    
    std::string serializeScore(const Score& score);
    std::string serializeMovie(const Movie& movie);

    std::string displayString(const Movie& movie, const std::string& preStr = "");
    std::string getStrInput(WINDOW* win, int y, int x);
    std::pair<Movie,double> highestDiffMovie();

    std::fstream moviefile;
    std::fstream highscoreFile;

    std::vector<Movie> movies;
    std::vector<Score> scores;
    std::unordered_map<int,double> ratedMovies;

    std::unordered_map<std::string,int> ratingCache;
    const std::vector<MenuItem> menuItems;
};