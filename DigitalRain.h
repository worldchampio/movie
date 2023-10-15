#include <string>
#include <vector> 

class Raindrop;

class DigitalRain{
public:
    DigitalRain();
    ~DigitalRain();
private:
    std::vector<Raindrop*> rain;
};