#include "Utils.h"
#include "ncurses.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>

class Movies{
public:
    struct Movie
    {
        double rating{};
        std::string name;
        int year{};
    };
    Movies();
    ~Movies();
    int execute();
private:
    struct Score
    {
        int score{};
        std::string timestamp;
    };

    struct MenuItem{
        std::string text;
        std::function<int()> fcn;
    };
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
    void gameOfLife();
    void graph();
    void list();
    void reset();
    void shutdown();

    Movie highestRatedMovie();

    std::string displayString(const Movie& movie, const std::string& preStr = "");
    std::string getStrInput(WINDOW* win, int y, int x, int color = 0, bool bold = true);
    std::pair<Movie,double> highestDiffMovie();

    std::vector<Movie> m_movies;
    std::vector<Score> m_scores;

    std::unordered_map<int,double> m_ratedMovies;
    std::unordered_map<std::string,int> m_ratingCache;

    const std::vector<std::vector<MenuItem>> m_menuItems;
    const std::vector<std::string> m_titles;
    
    int m_exitCode{0};

    template<class T>
    void serializeToFile(const std::string& fileName, const std::vector<T>& data)
    {
        auto file{std::fstream{fileName,std::ios_base::app}};
        for(const auto& object : data)
            file << serialize(object);
        file.close();
    }

    template<class T>
    std::string serialize(const T& object)
    {
        std::stringstream ss;
        if constexpr (std::is_same<T,Score>())
        {
            ss << object.score << ","
               << object.timestamp << std::endl;
        }
        else if constexpr (std::is_same<T,Movie>())
        {
            ss << object.rating<< ","
               << object.name  << ","
               << object.year  << std::endl; 
        }
        return ss.str();
    }

    template<class T>
    T deserialize(const std::string& str) 
    {
        const auto tokens{Utils::tokenize(str)};
        if constexpr (std::is_same<T,Score>())
            return { std::atoi(tokens[0].c_str()),tokens[1]};
        
        if constexpr (std::is_same<T,Movie>())
            return{ std::atof(tokens[0].c_str()), tokens[1], std::atoi(tokens[2].c_str())};
    }
};