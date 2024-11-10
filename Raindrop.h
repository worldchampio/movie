#include <string>

class Raindrop{
public:
    Raindrop(int xPos, bool startAsBlank);
    void update();
    void blankSpace(int length);
private:
    void shiftCharacters();
    int rate{0};
    int xPos{0};
    std::string str;
    std::string blank;
};