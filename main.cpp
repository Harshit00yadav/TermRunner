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

struct sprite{
  string texture[12*12];
  string map;
  int width;
  int height;
  void init(string m, int w, int h){
    map = m;
    width = w;
    height = h;
    map_texture(m);
  }
  void map_texture(string m){
    for (int i=0; i<width*height; i++){
      switch(m[i]){
        case '0':
          texture[i] = ' ';
          break;
        case '1':
          texture[i] = "\u2580";
          break;
        case '2':
          texture[i] = "\u2584";
          break;
        case '3':
          texture[i] = "\u2588";
          break;
      }
    }
  }
};

class Display{
  private:
    int width, height;
    char clear_char;
    string array[60*30];
  public:
    Display(char c, int w, int h){
      width = w;
      height = h;
      clear_char = c;
      for (int i=0; i<w*h; i++)
        array[i] = c;
    }

    void render(void);
    void blit(sprite, int, int);

    int get_width(){return width;}
    int get_height(){return height;}
};

void Display::render(void){
  cout<<"\033[H"; // bring cursor to top of console
  string line = "";

  for (int i=0; i<width*height; i++){
    line += array[i];
    array[i] = clear_char;
    if ((i+1)%width == 0){
      cout<<line<<endl;
      line = "";
    }
  }
}

void Display::blit(sprite s, int x, int y){
  int dw = width - s.width;
  int i = (y * width) + x;
  int j = 0;
  short obp = 0; // on board pixel;
  while(j < s.width * s.height){
    if (0 <= x + obp && x + obp < width)
      array[i] = s.texture[j];
    if ((j+1)%s.width == 0){
      i += dw + 1;
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
    sprite spr;
  public:
    bool isActive;
    Platform(int x, int y, int len){
      position[0] = x;
      position[1] = y;
      length = len;
      isActive = true;
      string m = "";
      for (int i=0; i<len; i++){
        m += '3';
      }
      spr.init(m , len, 1);
    }
    sprite get_sprite(){return spr;}
    void draw(void);
    void update(void);
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
    bool isColliding;
    sprite spr;

  public:
    Character(float x, float y){
      position[0] = x;
      position[1] = y;
      velocity[0] = 0;
      velocity[1] = 0;
      direction = 4;
      isColliding = false;
      spr.init("1", 1, 1);
    }
    sprite get_sprite(){return spr;}
    void update(std::list<Platform>&);
    void checkCollision(std::list<Platform>&);
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
  if (position[1]-(int)position[1] <= 0.5f && !isColliding)
    spr.map_texture("1");
  else 
    spr.map_texture("2");
}

void Character::checkCollision(std::list<Platform>& plfms){
  for (Platform p : plfms){
    if (p.getX() <= position[0] && p.getX() + p.getLength() > position[0]){
      if ((int)position[1] + spr.height == p.getY() && velocity[1] > 0){
        velocity[1] = 0;
        isColliding = true;
      } else {
        isColliding = false;
      }
      break;
    }
  }
}

int main(){
  srand((unsigned) time(NULL)); // generating seed for random function
  Character player(15, 6);
  std::list<Platform> plfms;
  plfms.push_back(Platform(15, 10, 20));
  std::list<Platform>::iterator plit = plfms.begin();

  Display display('-', Width, Height);
  
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
      plfms.push_front(Platform(Width, 5 + (rand() % 7), 10 + (rand() % 10)));
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
