#include <conio.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <list>
#include <string>

using namespace std;

const int Height = 15;
const int Width = 2*Height;
int span_counter = 0;
int tollerence = 5;
float levelSpeed = 0;
float levelMaxSpeed = 0.5f;
float levelAcc = 0.05f;
bool EXIT = false;

char *screen = new char[Width*Height];

// Overloading * operator 
string operator * (string a, unsigned int b) {
    string output = "";
    while (b--) {
        output += a;
    }
    return output;
}

class Surface{
  private:
    int width, height;
    bool isDisplay;
    char array[60*30];
    char clear_character;
  public:
    Surface(){}
    Surface(char c, int w, int h, bool isD=false){
      clear_character = c;
      width = w;
      height = h;
      isDisplay = isD;
      for (int i=0; i<w*h; i++)
        array[i] = c;
    }
    void init_surface(char c, int w, int h, bool isD=false){
      width = w;
      height = h;
      isDisplay = isD;
      for (int i=0; i<w*h; i++)
        array[i] = c;
    }

    void render(void);
    void blit(Surface, int, int);

    int get_width(){return width;}
    int get_height(){return height;}
};

void Surface::render(void){
  if (!isDisplay){
    cout<<"can't render non Display Surface!"<<endl;
    return;
  }
  cout<<"\033[H"; // bring cursor to top of console

  for (int i=0; i<width*height; i++){
    cout<<array[i];
    array[i] = clear_character;
    if ((i+1)%width == 0)
      cout<<endl;
  }
}

void Surface::blit(Surface spr, int x, int y){
  int dw = width - spr.get_width();
  int i = (y * width) + x;
  int j = 0;
  int obp = 0; /// OnBoardPixel
  while(j < spr.get_width() * spr.get_height()){
    if (0 <= x + obp && x + obp < width)
      array[i] = spr.array[j];
    if ((j+1)%spr.get_width() == 0){
      i += dw+1;
      obp = 0;
    } else {
      i++;
      obp++;
    }
    j++;
  }
}


class Platform{
  private:
    float position[2];
    int length;
    Surface sprite;
  public:
    bool isActive;
    Platform(int x, int y, int len){
      position[0] = x;
      position[1] = y;
      length = len;
      isActive = true;
      sprite.init_surface('#', len, 3);
    }
    void draw(void);
    void update(void);
    Surface get_sprite(){return sprite;}
    int getLength(void){return length;}
    int getX(void){return (int)position[0];}
    int getY(void){return (int)position[1];}
};

void Platform::draw(void){
  cout<<"\033["<<(int)position[1]<<";"<<(int)position[0]<<"H";
  if (position[0] > 1) cout<<"\u2595";
  for (int i=0; i<(int)length; i++){
    cout<<"\u2594";
  }
  cout<<"\u258F";
}

void Platform::update(void){
  if (position[0] > -length) position[0] -= levelSpeed;
  else {
    isActive = false;
  }
}

class Character{
  private:
    float acceleration = 0.05f;
    float position[2];
    float velocity[2];
    float velocityLimit = 1.0f;
    int direction;
    int length;
    Surface sprite;

  public:
    Character(float x, float y){
      position[0] = x;
      position[1] = y;
      velocity[0] = 0;
      velocity[1] = 0;
      direction = 4;
      sprite.init_surface('@', 1, 1);
    }
    void update(std::list<Platform>&);
    void checkCollision(std::list<Platform>&);
    Surface get_sprite(){return sprite;}
    int getX(void){return position[0];}
    int getY(void){return position[1];}
    void setVelocityY(float f){velocity[1] = f;}
};
void Character::update(std::list<Platform>& plfms){
  switch(direction){
    case 1:
      if (velocity[0] < velocityLimit) velocity[0] += acceleration;
      break;
    case 2:
      if (velocity[1] > -velocityLimit) velocity[1] -= acceleration;
      break;
    case 3:
      if (velocity[0] > -velocityLimit) velocity[0] -= acceleration; 
      break;
    case 4:
      if (velocity[1] < velocityLimit) velocity[1] += acceleration;
      break;
  }
  position[0] += velocity[0];
  checkCollision(plfms);
  position[1] += velocity[1];
}

void Character::checkCollision(std::list<Platform>& plfms){
  for (Platform p : plfms){
    if (p.getX() <= position[0] && p.getX() + p.getLength() > position[0]){
      if ((int)position[1] + sprite.get_height() == p.getY() && velocity[1] > 0)
        velocity[1] = 0;
      break;
    }
  }
}


void clear(){
  string horiz = "\u2581";
  horiz = horiz * (Width + 15 - 2);
  cout<<"\033[H"<<endl;
  cout<<" "<<horiz<<" "<<endl;
  for (int i=3; i<Height; i++){
    cout<<"\033[2K\u2595"<<"\033["<<i<<";"<<Width+15<<"H\u258F"<<endl;
  }
}

int main(){
  srand((unsigned) time(NULL)); // generating seed for random function
  Character player(15, 6);
  std::list<Platform> plfms;
  plfms.push_back(Platform(15, 10, 20));
  std::list<Platform>::iterator plit = plfms.begin();

  Surface display('-', Width, Height, true);
  
  SetConsoleOutputCP(CP_UTF8);
  cout<<"\033c"; //clear screen
  cout<<"\033[?25l"; // hide cursor

  while (!EXIT){
    Sleep(1000/30); // 30 frames per sec
    
    // ------ keyboard inputs ------------------

    if (kbhit()){
      // check for key hit on keyboard
      switch(getch()){
        case '\x1b': // escape key
          EXIT = true;
          break;
        case 'w':
          player.setVelocityY(-0.6f); // <-- jump velocity = 0.55f
          break; 
      }
    }

    //--------------------------------------------
    // -------- update ---------------------------
    
    if (levelSpeed < levelMaxSpeed) levelSpeed += levelAcc;

    plit = plfms.begin();
    if (plit->getX() + plit->getLength() <= Width - 10){
      plfms.push_front(Platform(Width, 4 + (rand() % 10), 15));
    }

    while (plit != plfms.end()){
      if (plit->isActive){
        plit->update();
        ++plit;
      } else plfms.erase(plit++);
    } 
    player.update(plfms);
    
    // -------------------------------------------
    // -------- render ---------------------------

    display.blit(player.get_sprite(), player.getX(), player.getY());
    for (Platform p : plfms){
      display.blit(p.get_sprite(), p.getX(), p.getY());
    }
    display.render();

    // -------------------------------------------
  }
  cout<<"\033[25;25Hhello";
  cout<<"\033[?25h"; // un-hide cursor
  return 0;
}
