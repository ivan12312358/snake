#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_tim.h"
#include "system_stm32f0xx.h"
#include "xprintf.h"
#include "oled_driver.h"
#include "snake.h"

static int ms = 0;

static void rcc_config()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
                                LL_RCC_PLL_MUL_12);

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    SystemCoreClock = 48000000;
}

static void gpio_config(void)
{
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
    
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_1, LL_GPIO_PULL_UP);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_UP);
    return;
}

static void timers_config(void)
{
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_SetPrescaler(TIM2, 47999);
    LL_TIM_SetAutoReload(TIM2, 99);
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_EnableIT_UPDATE(TIM2);
    LL_TIM_EnableCounter(TIM2);
 
    NVIC_EnableIRQ(TIM2_IRQn);
    NVIC_SetPriority(TIM2_IRQn, 0);
    return;
}

void TIM2_IRQHandler(void)
{
  ms++;
  LL_TIM_ClearFlag_UPDATE(TIM2);
}

static void exti_config(void)
{
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);

    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE1);
    LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);

    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_1);
    LL_EXTI_EnableIT_0_31(LL_EXTI_LINE_0);

    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_1);
    LL_EXTI_EnableRisingTrig_0_31(LL_EXTI_LINE_0);


    NVIC_EnableIRQ(EXTI0_1_IRQn);
    NVIC_SetPriority(EXTI0_1_IRQn, 1);
}

void EXTI0_1_IRQHandler(void)
{
  while(ms < 1);
  ms = 0;


  if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) && LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0))
  {
    action = 1;
  }
  else if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_1) && LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_1))
  {
    action = 2;  
  }

  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
  LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_1);
  return;
}

static void printf_config(void)
{
    xdev_out(oled_putc);
    return;
}

void Snake_(Snake* snake, uint8_t X, uint8_t Y)
{
    for(uint8_t i = 0; i < H_SIZE; i++)
    {
        snake->point[0][i] = '*';
        snake->point[V_SIZE][i] = '*';
    }

    snake->point[0][H_SIZE] = '\n';
    snake->point[V_SIZE][H_SIZE] = '\r';


    for(int i = 1; i < V_SIZE; i++)
    {
        snake->point[i][0] = '*';

        for(int j = 1; j < H_SIZE - 1; j++)
                snake->point[i][j] = ' ';

        snake->point[i][H_SIZE - 1] = '*';
        snake->point[i][H_SIZE]     = '\n';
    }


    snake->direction = right;
    snake->head = 4;
    snake->coord[0].x = X;  snake->coord[snake->head].x = X - 1;
    snake->coord[0].y = Y;  snake->coord[snake->head].y = Y;

    for(int i = 0; i < snake->head + 1; i++)
    {
        snake->coord[i].x = X + i; snake->coord[i].y = Y;
        snake->point[snake->coord[i].y][snake->coord[i].x] = '@';
    }

    Candy(snake);
}

void Game(Snake* snake)
{
  while(!Over(snake))
  {
    for(uint8_t i = 0; i < V_SIZE + 1; i++)
      for(uint8_t j = 0; j < H_SIZE + 1; j++)
        oled_putc(snake->point[i][j]);

    oled_update();

    while(ms < 2);
    ms = 0;
    
    Offset(snake);

    switch (action)
    {
      case(0):
        break;
      case(1):
        snake->direction = (snake->direction + 1) % 4;
        action = 0;
        break;
      case(2):
        snake->direction = (snake->direction + 3) % 4;
        action = 0;
        break;
    }

    snake->coord[snake->head].x += ((snake->direction - 1) % 2) * (-1);
    snake->coord[snake->head].y += (snake->direction - 2) % 2;

    if(snake->point[snake->coord[snake->head].y][snake->coord[snake->head].x] == '@')
      break;
    else if(snake->point[snake->coord[snake->head].y][snake->coord[snake->head].x] == '$')
    {
      Candy(snake);

      snake->head++;

      snake->coord[snake->head].x = snake->coord[snake->head - 1].x;
      snake->coord[snake->head].y = snake->coord[snake->head - 1].y;
    }

    snake->point[snake->coord[snake->head].y][snake->coord[snake->head].x] = '@';
  }
  
  oled_putc('\r');
  oled_clr(0x00);  
  xprintf("\n     Game over!");
  oled_update();
}

int Over(Snake* snake)
{
  uint8_t x = snake->coord[snake->head].x < H_SIZE - 1 && snake->coord[snake->head].x > 0;
  uint8_t y = snake->coord[snake->head].y < V_SIZE     && snake->coord[snake->head].y > 0;

  if(x && y) return 0;

  return 1;
}

void Offset(Snake* snake)
{
  snake->point[snake->coord[0].y][snake->coord[0].x] = ' ';

  for(int i = 1; i < snake->head + 1; i++)
  {
    snake->coord[i - 1].y = snake->coord[i].y;
    snake->coord[i - 1].x = snake->coord[i].x;
  }
}

void Candy(Snake* snake)
{
  int x = (H_SIZE - snake->coord[snake->head - 1].x);
  int y = (V_SIZE - snake->coord[snake->head - 1].y);

  x = x > H_SIZE? x - H_SIZE: x;
  y = y > V_SIZE? y - V_SIZE: y;

  while(snake->point[y][x] == '@' || snake->point[y][x] == '*')
  {
    x++;

    if(x == H_SIZE)
    {
      x = 0;
      y++;
    }
  }

  snake->point[y][x] = '$';
}

int main(void)
{
    rcc_config();
    gpio_config();
    exti_config();
    oled_config();
    printf_config();
    timers_config();

    Snake snake;

    Snake_(&snake, 1, 1);

    Game(&snake);
    
    while (1);
}