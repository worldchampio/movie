#include "Movies.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>

namespace fs = std::filesystem;

namespace 
{
    constexpr auto Filename{"movies.txt"};
    constexpr auto RatingIdx{0};
    constexpr auto NameIdx{1};
    constexpr auto YearIdx{2};
    constexpr auto CYAN{1};
    constexpr auto YELLOW{2};
    constexpr auto RED{3};
    constexpr auto GREEN{4};
}

Movies::Movies()
{
    moviefile.open(Filename);
    loadMovies();
    initscr();
    curs_set(0);

    start_color();
    init_pair(CYAN,COLOR_CYAN,COLOR_BLACK);
    init_pair(YELLOW,COLOR_YELLOW,COLOR_BLACK);
    init_pair(RED,COLOR_RED,COLOR_BLACK);
    init_pair(GREEN,COLOR_GREEN,COLOR_BLACK);
    attron(COLOR_PAIR(1));

    box(stdscr,0,0);
    const std::vector menuItems{
        "Add movie." ,
        "Rate two movies",
        "Search for movie",
        "Browse",
        "Exit",
    };

    for(int i=0; i<menuItems.size(); i++)
        mvprintw(i+2,4,menuItems[i]);

    const std::vector helpItems{
        "D       - select",
        "A       - back",
        "Q       - quit",
        "W,S     - up,down"
    };

    for(int i=0; i<helpItems.size(); i++)
        mvprintw(LINES-i-2,4,helpItems[i]);
    
    char c{'\0'};
    int pos = 0;
    mvprintw(pos+2,2,">");
    refresh();
    noecho();
    while(c!='q'){
        switch(c){
            case KEY_UP:
            case 'W':
            case 'w':
            {
                mvprintw(pos+2,2," ");
                pos--;
                break;
            }
            case KEY_DOWN:
            case 'S':
            case 's':
            {
                mvprintw(pos+2,2," ");
                pos++;
                break;
            }
            case KEY_RIGHT:
            case 'D':
            case 'd':
            case KEY_ENTER:
                switch(pos)
                {
                    case 0: { addMovie(); break; }
                    case 1: { rateMovies(); break; }
                    case 2: { search(); break; }
                    case 3: { browse(); break; }
                    case 4: return;
                    default: break;
                }
        }
        if(pos > 4)
            pos = 0;
        if(pos < 0)
            pos = 4;
        mvprintw(pos+2,2,"*");
        mvchgat(pos+2,2,1,A_STANDOUT,COLOR_PAIR(1),nullptr);
        box(stdscr,0,0);
        refresh();
        c = getch();
    }
}

Movies::~Movies()
{
    moviefile.close();
    fs::remove(Filename);
    moviefile.open(Filename,std::ios_base::app);
    for(const auto& movie : movies)
        moviefile << serialize(movie);
    moviefile.close();

    endwin();
}

std::string Movies::getStrInput(WINDOW* win, int y, int x)
{
    int c{'\0'};
    std::string str;
    while(c!='\n')
    {
        c=getch();
        if(c == KEY_BACKSPACE)
        {
            str.pop_back();
            continue;
        }
        str+=c;
        mvwprintw(win,y,x,str.c_str());
        box(win,0,0);
        wrefresh(win);
    }
    if(str.back()=='\n')
        str.pop_back();
    return str;
}

void Movies::browse(){
    constexpr auto xStart{17+4};
    auto w = newwin(LINES-2,COLS-xStart-3,1,xStart+2);
    wattron(w,COLOR_PAIR(YELLOW));

    auto shift{0};
    const auto lastMovie{static_cast<int>(movies.size()-1)};
    const auto drawMovies{[this,&w,&lastMovie](int shift){
        if(shift>=movies.size()-1)
            return;
        for(int y=1; y<movies.size(); y++){
            const int adjustedShift{ std::clamp(y+shift,0,lastMovie) };
            const auto& currentMovie{movies[adjustedShift]};
            std::stringstream ss;
            ss  << currentMovie.rating << " " 
                << currentMovie.name <<" ("
                << currentMovie.year <<")";
            std::string bigSpace; bigSpace.resize(80,' ');
            mvwprintw(w,y,2,bigSpace.c_str());
            mvwprintw(w,y,2,ss.str().c_str()); }
    }};
    drawMovies(shift);
    box(w,0,0);
    wrefresh(w);
    char c{'\0'};
    int pos{0};
    while(c!='q')
    {
        c = getch();
        mvwprintw(w,pos,0," ");
        switch (c)
        {
        case 's': case 'S': case KEY_DOWN: { pos++; break; }     
        case 'w': case 'W': case KEY_UP: { pos--; break; }
        case 'd': case 'D': { pos+=10; break; }
        case 'a': case 'A': { pos-=10; break; }
        default : return;
        }
        if(pos < 1)
        {
            pos = 1;
            if(shift+pos > 0)
                shift--;
            shift = std::clamp(shift,0,lastMovie);
            drawMovies(shift);
        } 
        else if ( pos > LINES-4)
        {
            pos = LINES-4;
            if(shift+pos < lastMovie)
                shift++;
            shift = std::clamp(shift,0,lastMovie);
            drawMovies(shift);
        }
        box(w,0,0);
        mvwprintw(w,pos,0,">");
        wrefresh(w);
    }
}

void Movies::addMovie(){
    auto w = newwin(10,30,2,21);
    wattron(w,COLOR_PAIR(RED));
    mvwprintw(w,1,1,"Name:");
    mvwprintw(w,2,1,"Year:");
    mvwprintw(w,3,1,"Rate:");
    box(w,0,0);
    wrefresh(w);

    Movie movie;
    movie.name = getStrInput(w,1,7);
    if(movie.name.back()=='\n')
        movie.name.pop_back();
    movie.year = std::atoi(getStrInput(w,2,7).c_str());
    movie.rating = std::atof(getStrInput(w,3,7).c_str());
    const auto validYear{movie.year > 1900 && movie.year < 2024};
    const auto validRating{movie.rating > 0 && movie.rating <= 10};
    if(validYear && validRating)
        movies.push_back(movie);
    else
    {
        mvwprintw(w,5,1,"Movie was not added.");
        if(!validYear)
            mvwprintw(w,6,1,"Invalid year.");
        if(!validRating)
            mvwprintw(w,7,1,"Invalid rating.");

        wrefresh(w);
        getch();   
    }
}

char asciitolower(char in) {
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
};

bool Movies::stringEquals(std::string a, std::string b)
{
    std::transform(a.begin(), a.end(), a.begin(), asciitolower);
    std::transform(b.begin(), b.end(), b.begin(), asciitolower);

    if(a.find(b) != std::string::npos)
        return true;
    return false;
}

void Movies::search()
{
    auto w = newwin(15,70,2,21);
    wattron(w,COLOR_PAIR(RED));
    mvwprintw(w,1,2,"Search: ");
    box(w,0,0);
    wrefresh(w);
    auto searchInput = getStrInput(w,2,2);
    std::vector<Movie> moviesFound;
    for(const auto& movie : movies)
        if(stringEquals(movie.name,searchInput))
            moviesFound.push_back(movie);
    if(!moviesFound.empty())
    {   
        const std::string movieText{moviesFound.size() > 1 ? "Found "+std::to_string(moviesFound.size())+" movies:" : "Found movie:"};
        mvwprintw(w,4,2,movieText.c_str());
        int y{5};
        for(const auto& movie : moviesFound)
        {
            if(y>=14)
                continue;
            std::stringstream ss;
            ss  << movie.name << " ("
                << movie.year << ") - " 
                << movie.rating;
            mvwprintw(w,y,2,ss.str().c_str());
            y++;
        }
    } else {
        mvwprintw(w,4,2,"No matches.");
    }
    wrefresh(w);
    getch();
}

std::pair<Movies::Movie,Movies::Movie> Movies::getTwoRandomMovies(){
    const auto firstRng{rng(0,movies.size()-1)};    
    int secondRng{firstRng};
    while(secondRng==firstRng)
        secondRng=rng(0,movies.size()-1);
    
    return {movies[firstRng],movies[secondRng]};
}

void Movies::rateMovies()
{
    const auto[firstMovie,secondMovie]{getTwoRandomMovies()};
    std::stringstream ssFirst, ssSecond;
    ssFirst << firstMovie.name << " ("<<firstMovie.year<<") - " << firstMovie.rating;
    ssSecond << secondMovie.name << " ("<<secondMovie.year<<") - " << secondMovie.rating;
    const auto xAlign{10};
    const auto width{xAlign+std::max(ssFirst.str().size(),ssSecond.str().size())+2};
    auto w1 = newwin(3,width,2,21);
    auto w2 = newwin(3,width,6,21);

    wattron(w1,COLOR_PAIR(CYAN));
    wattron(w2,COLOR_PAIR(CYAN));

    mvwprintw(w1,1,2,"FIRST:");
    mvwprintw(w1,1,xAlign,ssFirst.str().c_str());
    mvwprintw(w2,1,2,"SECOND:");
    mvwprintw(w2,1,xAlign,ssSecond.str().c_str());
    box(w1,0,0);
    box(w2,0,0);
    wrefresh(w1);
    wrefresh(w2);
    std::optional<bool> selection;
    char c{'\0'};
    int ratings{0};
    while(ratings <10)
    {
        c = getch();
        switch (c)
        {
        case 'w': case 'W':
        {
            wattron(w1,A_STANDOUT);
            wattroff(w2,A_STANDOUT);
            box(w1,0,0);
            box(w2,0,0);
            selection = true;
            break;
        }
        case 's': case 'S':
        {            
            wattroff(w1,A_STANDOUT);
            wattron(w2,A_STANDOUT);
            box(w1,0,0);
            box(w2,0,0);
            selection = false;
            break;
        }
        case 'd': case 'D':
        {
            if(selection.has_value()){
                ++ratings;
                const auto victory{selection.value()};
                const auto[newRating1,newRating2]{computeElo(firstMovie.rating,secondMovie.rating,victory )};
                const auto diff1{newRating1-firstMovie.rating};
                const auto diff2{newRating2-secondMovie.rating};
                mvwprintw(w1,0,width-4,std::to_string(static_cast<int>(diff1)).c_str());
                mvwprintw(w2,0,width-4,std::to_string(static_cast<int>(diff2)).c_str());
                wrefresh(w1);
                wrefresh(w2);
            }
            break;
        }
        default:
            break;
        }
        wrefresh(w1);
        wrefresh(w2);
    }
}

void Movies::loadMovies()
{
    std::string str;
    while(std::getline(moviefile,str))
        movies.push_back(deserialize(str));
}

Movies::Movie Movies::deserialize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss{str};
    while(std::getline(ss,token,','))
        tokens.push_back(token);

    return{
        std::atof(tokens[RatingIdx].c_str()),
        tokens[NameIdx],
        std::atoi(tokens[YearIdx].c_str())
    };
}

std::string Movies::serialize(const Movie& movie)
{
    std::stringstream ss;
    ss << movie.rating<< ","
       << movie.name  << ","
       << movie.year  << std::endl;
    return ss.str();
}

std::pair<double,double> Movies::computeElo(double Ra, double Rb, bool score)
{
    constexpr auto K{32};
    const auto Ea = 1/( 1 + pow(10,(Rb-Ra)/400)); 
    const auto Eb = 1/( 1 + pow(10,(Ra-Rb)/400));
    const auto Ra_ = Ra + K * (score - Ea);
    const auto Rb_ = Rb + K * (!score - Eb);
    return { Ra_,Rb_ };
}

int Movies::rng(int min, int max) 
{
    std::random_device  dev;
    std::mt19937        rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(min,max);
    return dist6(rng);
}