uint8_t action = 0;
int ms_old = 0;

const uint8_t H_SIZE = 21;
const uint8_t V_SIZE = 6;

enum act
{
  right = 0,
  up    = 1,
  left  = 2,
  down  = 3
};

typedef struct Coord
{
  uint8_t x;
  uint8_t y;
}Coordinate;

typedef struct _Snake
{
  uint8_t head;
  uint8_t direction;

  char point[7][22];
  Coordinate coord[154];
}Snake;


void Delay(double diff);
void Snake_(Snake* snake, uint8_t X, uint8_t Y);
void Offset(Snake* snake);
void Candy(Snake* snake);
void Game(Snake* snake);
int  Over(Snake* snake);
