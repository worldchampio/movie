/*
Contains a string of random characters,
corresponding to a single column being displayed onscreen.
*/
#include "Raindrop.h"
#include "Utils.h"
#include <ncurses.h>
#include <random>

Raindrop::Raindrop(int xPos, bool startAsBlank) :
    rate{Utils::rng(1,100)},
    xPos{xPos}
{
    if(startAsBlank)
        blankSpace(Utils::rng(1,LINES-LINES/7));
    str.resize(LINES,' ');
}

void Raindrop::update(){
    for(int c = 0; c < str.size(); c++)
    {   
        if(c<str.size()-1 && static_cast<char>(mvinch(c+1,xPos))==' ')
            attron(A_BOLD);
        mvaddch(c,xPos,str[str.size()-1-c]);
        attroff(A_BOLD);
    }
    shiftCharacters();
}

void Raindrop::shiftCharacters(){
    if(rate>=Utils::rng(1,100))
        return;  
    str.erase(0,1);
    char ch;
    if(blank.empty())
        ch=char(Utils::rng('!','~'));
    else{
        ch=blank[0];
        blank.pop_back();
    }
    str.push_back(ch);
}

void Raindrop::blankSpace(int length){
    if(blank.empty())
        blank.resize(length,' ');
}