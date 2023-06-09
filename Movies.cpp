#include "Movies.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <random>

namespace 
{
    constexpr auto Filename{"movies.txt"};
    constexpr auto CYAN{1};
    constexpr auto YELLOW{2};
    constexpr auto RED{3};
    constexpr auto GREEN{4};
    constexpr auto MAGENTA{5};
}
namespace Utils {
    constexpr auto validYear(int year)
    { 
        return year > 1900 && year < 2024;
    }

    constexpr std::pair<double,double> computeElo(double Ra, double Rb, bool victor)
    {
        constexpr auto K{32};
        const auto Ea = 1 / ( 1 + pow(10, ( Rb - Ra ) / 400) ); 
        const auto Eb = 1 / ( 1 + pow(10, ( Ra -Rb ) / 400) );
        const auto Ra_ = Ra + K * (victor - Ea);
        const auto Rb_ = Rb + K * (!victor - Eb);
        return { Ra_, Rb_ };
    }

    int rng(int min, int max) 
    {
        std::random_device  dev;
        std::mt19937        rng(dev());
        std::uniform_int_distribution<std::mt19937::result_type> dist6(min,max);
        return dist6(rng);
    }

    bool stringEquals(std::string a, std::string b)
    {
        const auto asciitolower{[](char in) -> char 
        { 
            return (in <= 'Z' && in >= 'A') ? in - ('Z' - 'z') : in;
        }};
        std::transform(a.begin(), a.end(), a.begin(), asciitolower);
        std::transform(b.begin(), b.end(), b.begin(), asciitolower);
        return a.find(b) != std::string::npos;
    }
    
    constexpr std::pair<int,int> getTwoRngs(int max){
        const auto firstRng{rng(0,max)};    
        int secondRng{firstRng};
        while(secondRng==firstRng)
            secondRng=rng(0,max);
        
        return {firstRng, secondRng};
    }
}

Movies::Movies()
{  
    loadMovies();
    initscr();
    curs_set(0);
    initColors();
    noecho();
    createMenu();
    eventLoop();
}

Movies::~Movies()
{
    moviefile.close();
    std::filesystem::remove(Filename);
    moviefile.open(Filename,std::ios_base::app);

    for(const auto& movie : movies)
        moviefile << serialize(movie);

    moviefile.close();
    endwin();
}

void Movies::createMenu() 
{
    box(stdscr,0,0);
    constexpr auto x{4};
    const std::vector menuItems{
        "Add movie." ,
        "Rate two movies",
        "Search for movie",
        "Browse",
        "Recommend",
        "About",
        "Exit",
    };
    menuSize = static_cast<int>(menuItems.size())-1;
    for(int i=0; i<menuItems.size(); i++)
        mvprintw(i+2,x,menuItems[i]);

    const std::vector helpItems{
        "SHIFT+9 - Backspace",
        "D       - Select",
        "A       - Back",
        "Q       - Quit",
        "W,S     - Up, Down"
    };

    for(int i=0; i<helpItems.size(); i++)
        mvprintw(LINES-i-3,x,helpItems[i]);
}

void Movies::initColors()
{
    start_color();
    init_pair(CYAN,COLOR_CYAN,COLOR_BLACK);
    init_pair(YELLOW,COLOR_YELLOW,COLOR_BLACK);
    init_pair(RED,COLOR_RED,COLOR_BLACK);
    init_pair(GREEN,COLOR_GREEN,COLOR_BLACK);
    init_pair(MAGENTA,COLOR_MAGENTA,COLOR_BLACK);
    attron(COLOR_PAIR(1));
}

void Movies::eventLoop() 
{
    char c{'\0'};
    int pos = 0;
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
                    case 1: { for(int i=0; i<10; i++) rateMovies(); break; }
                    case 2: { search(); break; }
                    case 3: { browse(); break; }
                    case 4: { recommend(); break; }
                    case 5: { about(); break; }
                    case 6: { endwin(); return; }
                    default: break;
                }
        }
        pos = pos < 0 ? menuSize : pos > menuSize ? 0 : pos;
        mvprintw(pos+2,2,"*");
        mvchgat(pos+2,2,1,A_STANDOUT,COLOR_PAIR(1),nullptr);
        box(stdscr,0,0);
        refresh();
        c = getch();
    }
}

void Movies::recommend()
{
    const auto randomMovie{ movies[Utils::rng(0,movies.size()-1)] };
    auto w{ newwin(5,globalWidth+10,2,21) };
    wattron(w,COLOR_PAIR(MAGENTA));
    mvwprintw(w,1,2, displayString(randomMovie, "RANDOM:  ").c_str());
    if(!ratedMovies.empty())
    {
        const auto [highestDiff,diff]{highestDiffMovie()};
        const auto str{displayString(highestDiff, "HOTTEST: ") +" +"+std::to_string(static_cast<int>(diff))+""};
        mvwprintw(w,2,2, str.c_str());
        mvwchgat(w,2,str.size()-1,4,A_BOLD,COLOR_MAGENTA,nullptr);
    }
    mvwprintw(w,3,2, displayString(highestRatedMovie(), "HIGHEST: ").c_str());
    box(w,0,0);
    mvwprintw(w,0,2,"RECOMMENDATION");
    mvwprintw(w,3,1," ");
    wrefresh(w);
    getch();
    delwin(w);
}

std::pair<Movies::Movie,double> Movies::highestDiffMovie()
{
    Movie highestDiff;
    double diff{0};
    for(const auto& movieIndex : ratedMovies)
        if(movieIndex.second > diff)
        {    
            highestDiff = movies[movieIndex.first];
            diff = movieIndex.second;
        }
    return{ highestDiff,diff };
}

Movies::Movie Movies::highestRatedMovie()
{
    Movie highestRated;
    double rating{0};
    for(const auto& movie : movies)
        if(movie.rating > rating)
        {    
            highestRated = movie;
            rating = movie.rating;
        }
    return highestRated;    
}  

void Movies::about()
{
    const std::vector str{
        "Standard ELO ranking algorithm: ",
        " ",
        "                 1",
        "Ea = ------------------------",
        "     1 + 10^( (Rb - Ra)/400 )",
        " ",
        "                 1",
        "Eb = ------------------------",
        "     1 + 10^( (Ra - Rb)/400 )",
        " ",
        "Ra,new = Ra + K * (score - Ea) , K = 32",
        "Rb,new = Rb + K * (score - Eb) , K = 32"
    };
    auto w{ newwin(str.size()+4,46,1,21) };
    wattron(w,COLOR_PAIR(GREEN));
    wattron(w,A_BOLD);
    for(int i=0; i<str.size(); i++)
        mvwprintw(w,i+2,3,str[i]);
    box(w,0,0);
    wrefresh(w);
    getch();
    delwin(w);
}

std::string Movies::getStrInput(WINDOW* win, int y, int x)
{
    int c{'\0'};
    std::string str;
    while(c!='\n')
    {
        std::string blank;
        blank.resize(str.size(),' ');
        mvwprintw(win,y,x,blank.c_str());
        c=getch();
        if(c == KEY_BACKSPACE || c==')')
        {
            if(!str.empty())
                str.pop_back();
        } 
        else
            str+=c;

        mvwprintw(win,y,x,str.c_str());
        mvwchgat(win,y,x,str.size(),A_BOLD,0,nullptr);
        box(win,0,0);
        wrefresh(win);
    }
    if(str.back()=='\n')
        str.pop_back();
    return str;
}

void Movies::browse()
{
    constexpr auto xStart{17+4};
    auto w{ newwin(LINES-2,COLS-xStart-3,1,xStart+2) };
    wattron(w,COLOR_PAIR(YELLOW));

    auto shift{0};
    const auto lastMovie{static_cast<int>(movies.size()-1)};
    const auto drawMovies{[this,&w,&lastMovie](int shift)
    {
        if(shift>=movies.size()-1)
            return;
        for(int y=1; y<movies.size(); y++)
        {
            const int adjustedShift{ std::clamp(y+shift,0,lastMovie) };
            const auto& currentMovie{movies[adjustedShift]};
            std::string bigSpace; bigSpace.resize(COLS-xStart-4,' ');
            mvwprintw(w,y,0,bigSpace.c_str());
            mvwprintw(w,y,2,displayString(currentMovie).c_str()); 
        }
    }};
    drawMovies(shift);
    box(w,0,0);
    const std::string title{std::to_string(movies.size())+" movies loaded."};
    mvwprintw(w,0,2,title.c_str());
    mvwchgat(w,0,2,title.size(),A_BOLD,COLOR_PAIR(YELLOW),nullptr);
    wrefresh(w);
    char c{'\0'};
    int pos{0};
    while(c!='q')
    {
        c = getch();
        mvwprintw(w,pos,0," ");
        mvwchgat(w,pos,0,1,A_NORMAL,COLOR_PAIR(YELLOW),nullptr);
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
        } 
        else if ( pos > LINES-4)
        {
            pos = LINES-4;
            if(shift+pos < lastMovie)
                shift++;
        }
        shift = std::clamp(shift,0,lastMovie);
        drawMovies(shift);
        box(w,0,0);
        mvwprintw(w,0,2,title.c_str());
        mvwchgat(w,0,2,title.size(),A_BOLD,COLOR_PAIR(YELLOW),nullptr);
        mvwprintw(w,pos,0,">");
        mvwchgat(w,pos,0,1,A_STANDOUT,COLOR_PAIR(YELLOW),nullptr);
        wrefresh(w);
    }
    delwin(w);
}

void Movies::addMovie()
{
    auto w{ newwin(10,globalWidth,2,21) };
    wattron(w,COLOR_PAIR(RED));
    mvwprintw(w,1,2,"Name:");
    mvwprintw(w,2,2,"Year:");
    box(w,0,0);
    wrefresh(w);

    Movie newMovie{
        1000,
        getStrInput(w,1,7),
        std::atoi(getStrInput(w,2,7).c_str())
    };

    if(newMovie.name.back()=='\n') 
        newMovie.name.pop_back();

    std::optional<Movie> potentialMatch;
    for(const auto& movie : movies)
        if(Utils::stringEquals(movie.name,newMovie.name))
            potentialMatch = movie;

    if(Utils::validYear(newMovie.year) && !potentialMatch)
    {
        newMovie.rating = 1000;
        movies.push_back(newMovie);
    }
    else
    {
        mvwprintw(w,5,2,"Movie was not added.");
        if(!Utils::validYear(newMovie.year))
            mvwprintw(w,6,2,"Invalid year.");
        if(potentialMatch.has_value())
        {
            mvwprintw(w,7,2,"Already exists: ");
            const auto match{potentialMatch.value()};
            mvwprintw(w,8,2,displayString(match).c_str());
        }
        wrefresh(w);
        getch();   
    }
    delwin(w);
}

void Movies::search()
{
    auto w{ newwin(LINES-2,globalWidth,1,21) };
    wattron(w,COLOR_PAIR(RED));
    mvwprintw(w,1,2,"Search: ");
    box(w,0,0);
    wrefresh(w);

    int c{'\0'};
    std::string str;
    while(c!='\n')
    {
        std::vector<Movie> matches;
        std::string blank;
        blank.resize(str.size(),' ');
        mvwprintw(w,2,2,blank.c_str());
        c=getch();
        if(c == KEY_BACKSPACE || c==')')
        {
            if(!str.empty())
                str.pop_back();
        } else
            str+=c;

        if(str.back()=='\n')
            str.pop_back();

        for(const auto& movie : movies)
            if(Utils::stringEquals(movie.name,str))
                matches.push_back(movie);

        std::string blankSpace;
        blankSpace.resize(globalWidth-2,' ');
        for(int i=5; i<LINES-2; i++)
            mvwprintw(w,i,2,blankSpace.c_str());

        if(!matches.empty())
        {   
            const std::string movieText{matches.size() > 1 ? "Found "+std::to_string(matches.size())+" movies:   " : "Found movie:     "};
            mvwprintw(w,4,2,movieText.c_str());
            int y{5};
            for(const auto& movie : matches)
            {
                if(y>=LINES-3)
                    continue;
                mvwprintw(w,y,2,displayString(movie).c_str());
                y++;
            }
        }
        else 
        {
            mvwprintw(w,4,2,"No matches.      ");
        }
    
        wrefresh(w);
        mvwprintw(w,2,2,str.c_str());
        mvwchgat(w,2,2,str.size(),A_BOLD,0,nullptr);
        box(w,0,0);
        wrefresh(w);
    }
    delwin(w);
}

void Movies::rateMovies()
{
    const auto[firstNumber,secondNumber]{Utils::getTwoRngs(movies.size()-1)};
    const auto firstMovie{movies[firstNumber]};
    const auto secondMovie{movies[secondNumber]};
    auto w1{ newwin(4,globalWidth,2,21) };
    auto w2{ newwin(4,globalWidth,7,21)};
    wattron(w1,COLOR_PAIR(CYAN));
    wattron(w2,COLOR_PAIR(CYAN));
    mvwprintw(w1,1,2,displayString(firstMovie, "FIRST:  ").c_str());
    mvwprintw(w2,1,2,displayString(secondMovie, "SECOND: ").c_str());
    box(w1,0,0);
    box(w2,0,0);
    wrefresh(w1);
    wrefresh(w2);

    std::optional<bool> selection;
    std::optional<std::pair<double,double>> newRatings;
    auto loop{true};
    while(loop){
        switch (getch())
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
            loop = false;
            wrefresh(w1);
            wrefresh(w2);
            return;
        }
        default:
            break;
        }
                   
        if(selection.has_value())
            newRatings = Utils::computeElo(firstMovie.rating,secondMovie.rating,selection.value());

        if(newRatings.has_value())
        {
            const auto diff1{ newRatings.value().first - firstMovie.rating };
            const auto diff2{ newRatings.value().second - secondMovie.rating };
            ratedMovies[firstNumber] += diff1;
            ratedMovies[secondNumber] += diff2;
            movies[firstNumber].rating += diff1;
            movies[secondNumber].rating += diff2;
            const auto diff1Str{ "Rating: "+ std::string(diff1 > 0 ? "+":"") + std::to_string(static_cast<int>(diff1)) };
            const auto diff2Str{ "Rating: "+ std::string(diff2 > 0 ? "+":"") + std::to_string(static_cast<int>(diff2)) };
            mvwprintw(w1, 2, 2, diff1Str.c_str());
            mvwprintw(w2, 2, 2, diff2Str.c_str());
        }
        wrefresh(w1);
        wrefresh(w2);
    }
    delwin(w1);
    delwin(w2);
}

void Movies::loadMovies()
{
    moviefile.open(Filename);

    std::string str;
    while(std::getline(moviefile,str))
    {
        const auto movie{deserialize(str)};
        if(Utils::validYear(movie.year))
            movies.push_back(movie);
    }
}

Movies::Movie Movies::deserialize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss{str};
    while(std::getline(ss,token,','))
        tokens.push_back(token);

    return{
        std::atof(tokens[0].c_str()),
        tokens[1],
        std::atoi(tokens[2].c_str())
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

std::string Movies::displayString(const Movie& movie, const std::string& preStr)
{
    std::stringstream ss;
    ss.precision(1);
    ss << std::fixed << preStr << movie.name << " ("<< movie.year << ") - " << movie.rating;
    return ss.str();
}