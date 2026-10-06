#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_

#define ROW1 0xFE
#define ROW2 0xFD
#define ROW3 0xFB
#define ROW4 0xF7

enum Button
{
  B_UP,
  B_DOWN,
  B_LEFT,
  B_RIGHT,
  B_PAUSE,
  B_NONE,
};

enum Button Check_Row(uint8_t row);

HAL_StatusTypeDef Set_Keyboard(void);

enum Button Get_Char(void);

#endif /* INC_KEYBOARD_H_ */