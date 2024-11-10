#include "DigitalRain.h"
#include "Raindrop.h"
#include "Utils.h"
#include "ncurses.h"

DigitalRain::DigitalRain()
{
    for(int column=0; column<COLS; column++)
        rain.push_back(new Raindrop(column,rand()%2));

    while(getch()!='q'){
        for(const auto& raindrop : rain)
        {   
            if(rand()%800 < 10)
                raindrop->blankSpace(Utils::rng(LINES/2,LINES-LINES/8)); 
            raindrop->update();
        }
        refresh();
    }
}

DigitalRain::~DigitalRain(){
    for(const auto& raindrop : rain)
        delete raindrop;
}