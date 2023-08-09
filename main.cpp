#include <conio.h>
#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <list>
#include <string>

using namespace std;

const int Height = 15;
const int Width = 2*Height;
int spanCounter = 0;
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

void clearScreenDrawFrame(){
  string horiz = "\u2581";
  horiz = horiz * (Width + 15 - 2);
  cout<<"\033[H"<<endl;
  cout<<" "<<horiz<<" "<<endl;
  for (int i=3; i<Height; i++){
    cout<<"\033[2K\u2595"<<"\033["<<i<<";"<<Width+15<<"H\u258F"<<endl;
  }
}

class Platform{
  private:
    float position[2];
    int length;
  public:
    bool isActive;
    Platform(int x, int y, int len){
      position[0] = x;
      position[1] = y;
      length = len;
      isActive = true;
    }
    void draw(void);
    void update(void);
    int getLength(void){return length;}
    int getPositionX(void){return (int)position[0];}
    int getPositionY(void){return (int)position[1];}
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
  if (position[0] > 1) position[0] -= levelSpeed;
  else {
    length -= levelSpeed;
    if (length < 1){
      isActive = false;
    }
  }
}

class Character{
  private:
    float acceleration = 0.05f;
    float position[2];
    float velocity[2];
    float velocityLimit = 0.5f;
    int direction;
    int altitude;
    int length;
  public:
    Character(void){
      position[0] = Width/2;
      position[1] = Height/2;
      velocity[0] = 0;
      velocity[1] = 0;
      direction = 4;
      altitude = 0;
      length = 1;
    }
    Character(float x, float y){
      position[0] = x;
      position[1] = y;
      velocity[0] = 0;
      velocity[1] = 0;
      direction = 4;
      altitude = 0;
      length = 1;
    }
    void update(std::list<Platform>&);
    void draw(void);
    void checkCollision(std::list<Platform>&);
    int getX(void);
    int getY(void);
    int getLength(void);
    int getAltitude(void);
    int getDirection(void);
    void setVelocityY(float);
    void setDirection(int);
};
void Character::draw(void){

  cout<<"\033["<<(int)position[1]<<";"<<(int)position[0]<<"H";
  if (altitude) cout<<"\u2580";
  else cout<<"\u2584";
}
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
  
  if (position[1] - (int)position[1] <= 0.5) altitude = 1;
  else altitude = 0;
}

void Character::checkCollision(std::list<Platform>& plfms){
  std::list<Platform>::iterator it = plfms.begin();
  while(it != plfms.end()){
    if (it->getPositionX() <= position[0] && it->getPositionX()+it->getLength() >= position[0]){
      if ((int)position[1] == it->getPositionY()-1 && altitude == 0 && velocity[1] > 0) velocity[1] = 0;
      break;
    }
    ++it;
  }
}

int Character::getDirection(void){
  return direction;
}
int Character::getAltitude(void){
  return altitude;
}
int Character::getX(void){
  return position[0];
}
int Character::getY(void){
  return position[1];
}
int Character::getLength(void){return length;}
void Character::setVelocityY(float v){velocity[1] = v;}
void Character::setDirection(int dir){direction = dir;}



int main(){ 
  Character player(15, 6);
  std::list<Platform> plfms;
  plfms.insert(plfms.begin(), Platform(15, 10, 25));
  std::list<Platform>::iterator plit = plfms.begin();
  
  SetConsoleOutputCP(CP_UTF8);
  cout<<"\033c"; //clear screen
  cout<<"\033[?25l"; // hide cursor

  
  while (!EXIT){
    Sleep(1000/25); // 25 frames per sec
    spanCounter++;

    if (spanCounter%35 == 0){
      plfms.insert(plfms.begin(), Platform(35, (rand()%(13 - 8 + 1) + 8), 10));
    }

    if (levelSpeed < levelMaxSpeed) levelSpeed += levelAcc;
    clearScreenDrawFrame();
    player.draw();
    //player.update();

    plit = plfms.begin();
    while (plit != plfms.end()){
      if (plit->isActive){
        plit->update();
        plit->draw();
        ++plit;
      } else plfms.erase(plit++);
    } 
    player.update(plfms);

    if (kbhit()){
      // check for key hit on keyboard
      switch(getch()){
        case '\x1b': // escape key
          EXIT = true;
          break;
        case 'w':
          player.setVelocityY(-0.55f); // <-- jump velocity = 0.55f
          break; 
      }
    }
  }
  cout<<"\033[25;25Hhello";
  cout<<"\033[?25h"; // un-hide cursor
  return 0;
}
