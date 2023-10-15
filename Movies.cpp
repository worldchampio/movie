#include "Movies.h"
#include <fstream>
#include <iostream>
#include <thread>

#define IfKeyUp case KEY_UP: case 'W': case 'w'
#define IfKeyDown case KEY_DOWN: case 'S': case 's'
#define IfKeyLeft case 'a': case 'A': case KEY_LEFT
#define IfKeyRight case 'd': case 'D': case KEY_RIGHT
#define IfKeyConfirm case KEY_RIGHT: case 'D': case 'd': case KEY_ENTER

namespace
{
    constexpr auto Filename{"movies.txt"};
    constexpr auto HighscoreFilename{"score.txt"};

    constexpr auto CYAN{1};
    constexpr auto YELLOW{2};
    constexpr auto RED{3};
    constexpr auto GREEN{4};
    constexpr auto MAGENTA{5};

    constexpr auto globalWidth{90};

    void setText(WINDOW* w, int y, int x, const char* text) { mvwprintw(w,y,x,text); }

    enum class Direction{ Up, Left, Down, Right};
    constexpr auto updateDirection(char c, Direction dir)
    {
        switch(c)
        {
            IfKeyUp:    return dir == Direction::Down ? Direction::Down : Direction::Up; 
            IfKeyLeft:  return dir == Direction::Right ? Direction::Right : Direction::Left; 
            IfKeyDown:  return dir == Direction::Up ? Direction::Up : Direction::Down; 
            IfKeyRight: return dir == Direction::Left ? Direction::Left : Direction::Right; 
            default: return dir;
        }
    }

    struct Diff{ double diff; int number; WINDOW* w; };

    auto cleanup(WINDOW* win, int h_win, int w_win)
    {
        for(int y=1; y<h_win-1; ++y)
        for(int x=1; x<w_win-1; ++x)
            setText(win,y,x," ");
    }
}

Movies::Movies() :
    m_menuItems{
    {
        {"Add movie.",      [this]{ addMovie(); return 1; }},
        {"Rate two movies", [this]{ for(int i=0; i<10; i++) rateMovies(); return 1; }},
        {"Search for movie",[this]{ search(); return 1; }},
        {"Browse",          [this]{ browse(); return 1; }},
        {"Recommend",       [this]{ recommend(); return 1; }},
        {"Reset ratings",   [this]{ reset(); return 1; }}},
    {
        {"Snake",           [this]{ snake(); return 1; }},
        {"Game of Life",    [this]{ gameOfLife(); return 1; }},
        {"Graph",           [this]{ graph(); return 1; }}
    },
    {
        {"Exit",            []{ return 0; }}
    }},
    m_titles{"Movies","Games","Misc."}
{  
    loadMovies();
    loadHighscores();
    initscr();
    curs_set(0);
    initColors();
    noecho();
    keypad(stdscr,true);
    createMenu();
}

Movies::~Movies()
{
    std::filesystem::remove(Filename);
    std::filesystem::remove(HighscoreFilename);
    auto moviefile{std::fstream{Filename,std::ios_base::app}};
    auto highscoreFile{std::fstream{HighscoreFilename,std::ios_base::app}};
    for(const auto& movie : m_movies) moviefile << serialize(movie);
    for(const auto& score : m_scores) highscoreFile << serialize(score);
    moviefile.close();
    highscoreFile.close();
    shutdown();
}

void Movies::createMenu() 
{
    box(stdscr,0,0);
    constexpr auto x{4};
    auto pos{2};
    for(int i=0; i<m_menuItems.size(); ++i)
    {
        attron(A_UNDERLINE);
        attron(A_BOLD);
        setText(stdscr,pos-1,x-1,m_titles[i].c_str());
        attroff(A_BOLD);
        attroff(A_UNDERLINE);
        for(int j=0; j<m_menuItems[i].size(); ++j)
            setText(stdscr,pos++,x,m_menuItems[i][j].text.c_str());
        pos++;
    }

    const std::vector helpItems{
        "D       - Select",
        "A       - Back",
        "Q       - Quit",
        "W,S     - Up, Down"
    };

    for(int i=0; i<helpItems.size(); i++)
        setText(stdscr,LINES-i-3,x,helpItems[i]);
}

void Movies::initColors()
{
    start_color();
    init_pair(CYAN,COLOR_CYAN,COLOR_BLACK);
    init_pair(YELLOW,COLOR_YELLOW,COLOR_BLACK);
    init_pair(RED,COLOR_RED,COLOR_BLACK);
    init_pair(GREEN,COLOR_GREEN,COLOR_BLACK);
    init_pair(MAGENTA,COLOR_MAGENTA,COLOR_BLACK);
    attron(COLOR_PAIR(CYAN));
}

int Movies::execute() 
{
    char c{'\0'};
    int menuIndex{ 0 };
    int pos{ 0 };
    auto offset{0}; 
    while(c!='q'){
        setText(stdscr,pos+2+offset,2," ");
        offset = 0;
        switch(c){
            IfKeyUp:    
            { 
                pos--; 
                if(pos < 0)
                {    
                    --menuIndex;
                    menuIndex = Utils::wrapAround(menuIndex,0,m_menuItems.size()-1);
                }
                break; 
            }
            IfKeyDown:  
            { 
                pos++;
                if(pos == m_menuItems[menuIndex].size())
                {    
                    ++menuIndex;
                    --pos; // reset due to index wraparound
                    menuIndex = Utils::wrapAround(menuIndex,0,m_menuItems.size()-1);
                }
                break; 
            }
            IfKeyConfirm:
            {     
                m_exitCode = m_menuItems.at(menuIndex).at(pos).fcn();
                if(!m_exitCode)
                    return m_exitCode; 
                break; 
            }
        }
        for(int i=1; i<=menuIndex; ++i)
            offset+=m_menuItems[i-1].size();
        offset+=menuIndex;
        pos = Utils::wrapAround(pos,0,m_menuItems[menuIndex].size()-1);
        setText(stdscr,pos+2+offset,2,"*");
        mvchgat(pos+2+offset,2,1,A_STANDOUT,COLOR_PAIR(1),nullptr);
        box(stdscr,0,0);
        refresh();
        c = getch();
    }
    return m_exitCode;
}

void Movies::recommend()
{
    const auto randomMovie{ m_movies[Utils::rng(0,m_movies.size()-1)] };
    auto w{ newwin(5,globalWidth+10,2,21) };
    wattron(w,COLOR_PAIR(MAGENTA));
    setText(w,1,2, displayString(randomMovie, "RANDOM:  ").c_str());
    if(!m_ratedMovies.empty())
    {
        const auto [highestDiff,diff]{highestDiffMovie()};
        const auto str{displayString(highestDiff, "HOTTEST: ") +" +"+std::to_string(static_cast<int>(diff))+""};
        setText(w,2,2, str.c_str());
        mvwchgat(w,2,str.size()-1,4,A_BOLD,COLOR_MAGENTA,nullptr);
    }
    setText(w,3,2, displayString(highestRatedMovie(), "HIGHEST: ").c_str());
    box(w,0,0);
    setText(w,0,2,"RECOMMENDATION");
    setText(w,3,1," ");
    wrefresh(w);
    getch();
    delwin(w);
}

std::pair<Movies::Movie,double> Movies::highestDiffMovie()
{
    Movie highestDiff;
    double diff{0};
    for(const auto[index, difference] : m_ratedMovies)
        if(difference > diff)
        {    
            highestDiff = m_movies[index];
            diff = difference;
        }
    return{ highestDiff,diff };
}

Movies::Movie Movies::highestRatedMovie()
{
    Movie highestRated;
    double rating{0};
    for(const auto& movie : m_movies)
        if(movie.rating > rating)
        {    
            highestRated = movie;
            rating = movie.rating;
        }
    return highestRated;    
}  

void Movies::snake()
{
    constexpr auto xStart{21};
    const auto width{COLS-xStart-3};
    const auto height{LINES-2};
    auto w{ newwin(height,width,1,xStart+2) };
    Utils::Position pos{height/2, width/2};
    box(w,0,0);
    Direction dir;
    char c{'\0'};
    std::vector<Utils::Position> snake;
    int length{10};
    int score{0};
    int totalCookies{0};
    int cookieLimit{10};
    int timeOut{60};
    while(c!='q')
    {   
        setText(w,pos.y,pos.x," ");
        snake.push_back(pos);

        if(Utils::rng(0,100) < 50 && totalCookies < cookieLimit)
        {
            const auto[y1,y2]{Utils::getTwoRngs(1,height-2)};
            const auto[x1,x2]{Utils::getTwoRngs(1,width-2)};
            setText(w,y1,x1,"o");
            setText(w,y2,x2,"o");
            totalCookies+=2;
        }
        dir = updateDirection(c,dir);
        switch (dir)
        {
        case Direction::Up:     --pos.y; timeOut = 60; break;
        case Direction::Left:   --pos.x; timeOut = 30; break;
        case Direction::Down:   ++pos.y; timeOut = 60; break;
        case Direction::Right:  ++pos.x; timeOut = 30; break;
        }

        pos.y = Utils::wrapAround(pos.y,1,height-2);
        pos.x = Utils::wrapAround(pos.x,1,width-2);

        if(mvwinch(w,pos.y,pos.x) =='*')
        {
            if(score > 0)
                m_scores.push_back({score,Utils::timeStamp()});
            break;
        }         
        if(mvwinch(w,pos.y,pos.x) == 'o')
        {
            length+=5;
            ++score;
            --totalCookies;
            ++cookieLimit;
        }

        setText(w,0,2,("[\tScore: "+std::to_string(score)+"\t]").c_str());
        while(snake.size() > length)
        {
            auto beg{snake.begin()};
            setText(w,beg->y,beg->x," ");
            snake.erase(beg);
        }

        for(const auto[y,x] : snake)
            setText(w,y,x,"*"); 

        wrefresh(w);
        timeout(timeOut);
        c = getch();
    }
    setText(w,height-3,width/2,c == 'q' ? "GAME QUIT" : "GAME OVER");
    setText(w,height-2,width/2 - 5,"Any key to return");
    std::sort(m_scores.begin(),m_scores.end(),[](const Score& s1, const Score& s2){ return s1.score > s2.score; });
    wattron(w,A_UNDERLINE);

    for(int i=0; i<m_scores.size() && i<height-4; ++i)
    {
        const auto currentScore{m_scores[i].score};
        const auto str{std::to_string(currentScore)+"\t"+m_scores[i].timestamp};
        setText(w,i+2,2,str.c_str());
    }    

    if(score == m_scores.front().score)
    {
        wattron(w,COLOR_PAIR(MAGENTA));
        setText(w,0,2,"NEW HIGHSCORE");
    }
    timeout(-1);
    wrefresh(w);
    getch();
    delwin(w);
}

void Movies::gameOfLife()
{
    constexpr auto xStart{21};
    const auto width{COLS-xStart-3};
    const auto height{LINES-2};
    auto w{ newwin(height,width,1,xStart+2) };
    char c{'\0'};
    constexpr auto cellChar{'X'};
    constexpr auto cellStr{"X"};
    static_assert(cellStr[0]==cellChar,"Use same character for these");
    enum class Status{Dead,Alive};
    std::vector<std::vector<Status>> backPane;
    Utils::Queue<int> lastElements{5};
    for(int i=0; i<height; ++i)
    {
        backPane.push_back(std::vector<Status>{});
        for(int j=0; j<width; j++)
            backPane[i].push_back(Utils::rng(0,10) < 6 ? Status::Dead : Status::Alive);

    }
    box(w,0,0);
    wrefresh(w);
    timeout(30);
    int loops{0};
    while(c!='q')
    {
        auto timer{ Utils::Timer{}};
        loops++;
        int liveCount{0};
        for(int y=1; y<height-1; ++y)
            for(int x=1; x<width-1; ++x)
            {
                const auto alive{backPane[y][x]==Status::Alive};
                liveCount+=alive;
                setText(w,y,x,alive ? cellStr : " ");
            }

        lastElements.add(liveCount);;
        if(loops > 5 && std::all_of(lastElements.begin(),lastElements.end(),[&lastElements](int i){ return i==lastElements.get(0);}))
            break;

        for(int y=1; y<backPane.size(); ++y)
            for(int x=1; x<backPane[y].size(); ++x)
            {
                int neighbours{0};
                for(int innerY = y-1; innerY < y+2; ++innerY)
                    for(int innerX = x-1; innerX < x+2; ++innerX)
                    {
                        if(innerX==x && innerY==y)
                            continue;
                        if(mvwinch(w,innerY,innerX)==cellChar)
                            neighbours++;
                    }

                const auto newState{mvwinch(w,y,x)==cellChar ? (neighbours>1 && neighbours<4) : (neighbours==3)};
                backPane[y][x] = newState ? Status::Alive : Status::Dead;
            }

        setText(w,0,2,("[ Live: "+std::to_string(liveCount)+",\ti:"+std::to_string(loops)+"\tt:"+timer.get()+" ]").c_str());
        c = getch();
        wrefresh(w);
    }
    timeout(-1);
    wattron(w,COLOR_PAIR(RED));
    setText(w,height/2,width/2-5,"Terminated");
    wrefresh(w);
    getch();
    delwin(w);
}

void Movies::graph()
{
    constexpr auto xStart{21};
    const auto width{COLS-xStart-3};
    const auto height{LINES-2};
    auto w{ newwin(height,width,1,xStart+2)};
    box(w,0,0);
    auto A{7};
    auto B{M_PI*3};
    char c{'\0'};
    while(c!='q')
    {
        timeout(60);
        for(int i=0; i<100; ++i)
        {
            c = getch();
            if(c=='q')
            {    
                timeout(-1);
                delwin(w);
                return;
            }

            for(int x=1; x<width-1; ++x)
            {
                switch(c)
                {
                    case 'w': ++A; c = '\0'; break;
                    case 's': --A; c = '\0'; break;
                    case 'a': B-= 1.0*M_PI/180.0; c = '\0'; break;
                    case 'd': B+= 1.0*M_PI/180.0; c = '\0'; break;
                }
                A = std::clamp(A,1,height/2-1);
                const auto y{ A * std::sin(180/(M_PI*2)*B*x+i)};
                setText(w,y+height/2,x,"*");
            }
            setText(w,0,2,("Amp: "+std::to_string(A)+",\tfreq: "+std::to_string(B/M_PI).substr(0,6)+"pi").c_str());
            wrefresh(w);
            cleanup(w,height,width);
        }
    }
    timeout(-1);
    delwin(w);
}

void Movies::reset() 
{
    constexpr auto xStart{21};
    const auto width{COLS-xStart-3};
    const auto height{LINES-2};
    auto w{ newwin(height,width,1,xStart+2)};
    wattron(w,COLOR_PAIR(GREEN));
    wattron(w,A_BOLD);

    setText(w,2,1,"Any key to exit");
    setText(w,4,1,"D = Delete");
    setText(w,5,1,"R = Restore");

    box(w,0,0);
    wrefresh(w);
    const auto totalMovies{std::to_string(m_movies.size())};
    switch(getch())
    {
        case 'D':
        case 'd':
        {
            for(auto& movie : m_movies)
            {
                m_ratingCache[movie.name] = movie.rating;
                movie.rating = 1000;
            }
            setText(w,7,1,("Reset "+totalMovies+" movies rating to 1000").c_str());
            break;
        }
        case 'R':
        case 'r':
        {
            if(m_ratingCache.empty())
                break;

            std::vector<Movie> restoredMovies;
            Utils::Queue<Movie> queue(height-2);
            for(auto& movie : m_movies)
                if(const auto it{ m_ratingCache.find(movie.name) }; it != m_ratingCache.end()) [[likely]]
                {
                    movie.rating = it->second;
                    restoredMovies.push_back(movie);
                    queue.add(movie);
                    std::string blank;
                    blank.resize(width-2,' ');
                    if(std::next(it) != m_ratingCache.end()) [[likely]]
                        for(int i=1; i<=queue.size(); ++i)
                            setText(w,i,1,blank.c_str());
                    auto count {1};
                    for(auto element : queue)
                    {   
                        setText(w,count++,12,("Restored "+element.name+" rating to "+std::to_string(element.rating).substr(0,6)).c_str());
                        wrefresh(w);
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(20));
                }            
            break;
        }
        default:
            break;
    }
    wrefresh(w);
    getch();
    delwin(w);
}

void Movies::shutdown()
{
    attron(COLOR_PAIR(CYAN));
    attron(A_STANDOUT);

    for(int y=0; y<LINES; ++y)
    {
        for(int x=0; x<COLS; ++x)
            setText(stdscr,y,x," ");
        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
    m_exitCode = endwin();
}

std::string Movies::getStrInput(WINDOW* win, int y, int x, int color, bool bold)
{
    int c{'\0'};
    std::string str;
    while(c!='\n')
    {
        std::string blank;
        blank.resize(str.size(),' ');
        setText(win,y,x,blank.c_str());
        c=getch();
        if(Utils::backspace(c))
        {
            if(!str.empty())
                str.pop_back();
        } 
        else if(Utils::validAscii(c))
            str+=c;

        setText(win,y,x,str.c_str());
        if(bold)
            mvwchgat(win,y,x,str.size(),A_BOLD,color,nullptr);
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
    const auto lastMovie{static_cast<int>(m_movies.size()-1)};
    const auto drawMovies{[this,&w,&lastMovie](int shift)
    {
        if(shift>=m_movies.size()-1)
            return;
        for(int y=0; y<m_movies.size(); y++)
        {
            const int adjustedShift{ std::clamp(y+shift,0,lastMovie) };
            const auto& currentMovie{m_movies[adjustedShift]};
            std::string bigSpace; bigSpace.resize(COLS-xStart-4,' ');
            setText(w,y+1,0,bigSpace.c_str());
            setText(w,y+1,2,(std::to_string(adjustedShift+1)+"\t"+displayString(currentMovie)).c_str()); 
        }
    }};
    drawMovies(shift);
    box(w,0,0);

    const std::string title{std::to_string(m_movies.size())+" movies loaded."};
    setText(w,0,2,title.c_str());
    mvwchgat(w,0,2,title.size(),A_BOLD,COLOR_PAIR(YELLOW),nullptr);
    wrefresh(w);
    char c{'\0'};
    int pos{0};
    int traversal{1};
    while(c!='q')
    {
        c = getch();
        setText(w,pos,0," ");
        mvwchgat(w,pos,0,1,A_NORMAL,COLOR_PAIR(YELLOW),nullptr);
        switch (c)
        {
            IfKeyDown:  { pos++; break; }     
            IfKeyUp:    { pos--; break; }
            IfKeyRight: { pos+=10; break; }
            IfKeyLeft:  { pos-=10; break; }
            default :   { search(); return; }
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
            if(shift+pos < lastMovie+1)
                shift++;
        }
        shift = std::clamp(shift,0,lastMovie);
        drawMovies(shift);
        box(w,0,0);
        setText(w,0,2,title.c_str());
        mvwchgat(w,0,2,title.size(),A_BOLD,COLOR_PAIR(YELLOW),nullptr);
        setText(w,pos,0,">");
        traversal = std::max(1,static_cast<int>((shift+pos)/static_cast<double>(lastMovie) * (LINES-4)));
        setText(w,traversal,COLS-xStart-3-1,"+");
        mvwchgat(w,pos,0,1,A_STANDOUT,COLOR_PAIR(YELLOW),nullptr);
        wrefresh(w);
    }
    delwin(w);
}

void Movies::addMovie()
{
    auto w{ newwin(11,globalWidth,2,21) };
    wattron(w,COLOR_PAIR(RED));
    setText(w,1,2,"Name:");
    setText(w,2,2,"Year:");
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
    for(const auto& movie : m_movies)
        if(Utils::stringEquals(movie.name,newMovie.name))
            potentialMatch = movie;

    if(Utils::validYear(newMovie.year) && !potentialMatch)
    {
        newMovie.rating = 1000;
        m_movies.push_back(newMovie);
    }
    else
    {
        setText(w,5,2,"Movie was not added.");
        if(!Utils::validYear(newMovie.year))
            setText(w,6,2,"Invalid year.");
        if(newMovie.name.empty())
            setText(w,7,2,"Empty name.");
        else if(potentialMatch.has_value())
        {
            setText(w,8,2,"Already exists: ");
            const auto match{potentialMatch.value()};
            setText(w,9,2,displayString(match).c_str());
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
    setText(w,1,2,"Search: ");
    box(w,0,0);
    wrefresh(w);

    int c{'\0'};
    std::string str;
    while(c!='\n')
    {
        std::vector<Movie> matches;
        std::string blank;
        blank.resize(str.size(),' ');
        setText(w,2,2,blank.c_str());
        c=getch();
        if(Utils::backspace(c))
        {
            if(!str.empty())
                str.pop_back();
        } 
        else if(Utils::validAscii(c))
            str+=c;

        if(str.back()=='\n')
            str.pop_back();

        for(const auto& movie : m_movies)
            if(Utils::stringEquals(movie.name,str))
                matches.push_back(movie);

        std::string blankSpace;
        blankSpace.resize(globalWidth-2,' ');
        for(int i=5; i<LINES-2; i++)
            setText(w,i,2,blankSpace.c_str());

        if(!matches.empty())
        {   
            const std::string movieText{matches.size() > 1 ? "Found "+std::to_string(matches.size())+" movies:   " : "Found movie:     "};
            setText(w,4,2,movieText.c_str());
            int y{5};
            for(const auto& movie : matches)
            {
                if(y>=LINES-3)
                    continue;
                setText(w,y,2,displayString(movie).c_str());
                y++;
            }
        }
        else 
        {
            setText(w,4,2,"No matches.      ");
        }
    
        wrefresh(w);
        setText(w,2,2,str.c_str());
        mvwchgat(w,2,2,str.size(),A_BOLD,0,nullptr);
        box(w,0,0);
        wrefresh(w);
    }
    delwin(w);
}

void Movies::rateMovies()
{
    const auto[firstNumber,secondNumber]{Utils::getTwoRngs(0,m_movies.size()-1)};
    const auto firstMovie{m_movies[firstNumber]};
    const auto secondMovie{m_movies[secondNumber]};
    auto w1{ newwin(4,globalWidth,2,21) };
    auto w2{ newwin(4,globalWidth,7,21)};
    wattron(w1,COLOR_PAIR(CYAN));
    wattron(w2,COLOR_PAIR(CYAN));
    setText(w1,1,2,displayString(firstMovie, "FIRST:  ").c_str());
    setText(w2,1,2,displayString(secondMovie, "SECOND: ").c_str());
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
        IfKeyUp:
        {
            wattron(w1,A_STANDOUT);
            wattroff(w2,A_STANDOUT);
            box(w1,0,0);
            box(w2,0,0);
            selection = true;
            break;
        }
        IfKeyDown:
        {            
            wattroff(w1,A_STANDOUT);
            wattron(w2,A_STANDOUT);
            box(w1,0,0);
            box(w2,0,0);
            selection = false;
            break;
        }
        IfKeyRight:
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
            for(const auto [diff,num,win] : { Diff{diff1,firstNumber,w1}, Diff{diff2,secondNumber,w2}}) 
            {
                m_ratedMovies[num] += diff;
                m_movies[num].rating += diff;
                const auto diffStr{ "Rating: "+ std::string(diff > 0 ? "+":"") + std::to_string(static_cast<int>(diff)) };
                setText(win, 2, 2, diffStr.c_str());
            }
        }
        wrefresh(w1);
        wrefresh(w2);
    }
    delwin(w1);
    delwin(w2);
}

void Movies::loadMovies()
{
    auto moviefile{std::fstream{Filename}};
    std::string str;
    while(std::getline(moviefile,str))
        if(const auto movie{deserialize<Movie>(str)}; Utils::validYear(movie.year))
            m_movies.push_back(movie);

    moviefile.close();
    std::sort(m_movies.begin(),m_movies.end(),[](const Movie& m1, const Movie& m2){ return m1.rating > m2.rating; });
}

void Movies::loadHighscores()
{
    auto highscoreFile{std::fstream(HighscoreFilename)};
    std::string str;
    while(std::getline(highscoreFile,str))
        if(!str.empty())
            m_scores.push_back(deserialize<Score>(str));
    highscoreFile.close();
}

std::string Movies::displayString(const Movie& movie, const std::string& preStr)
{
    std::stringstream ss;
    ss.precision(1);
    ss << std::fixed << preStr << movie.name << " ("<< movie.year << ") - " << movie.rating;
    return ss.str();
}