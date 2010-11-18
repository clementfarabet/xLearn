#include "lcairo_defines.h"
#include "keytable.h"

#ifdef _LCAIRO_WIN32
#include <windows.h>
//svn check insert no login 
#define VK_A (0x41)
#define VK_0 (0x30)

int vk_key_table[][2]={
 { KEY_SPACE,	VK_SPACE },
 { KEY_ESCAPE,	VK_ESCAPE },
 { KEY_LEFT,	VK_LEFT },
 { KEY_RIGHT,	VK_RIGHT },
 { KEY_UP,	VK_UP },
 { KEY_DOWN,	VK_DOWN },
 { KEY_LSHIFT,	VK_LSHIFT },
 { KEY_RSHIFT,	VK_RSHIFT },
/* { KEY_LCONTROL,VK_LCONTROL }, */
/* { KEY_RCONTROL,VK_RCONTROL }, */
/* { KEY_LALT,	VK_LALT }, */
/* { KEY_RALT,	VK_RALT }, */
 { KEY_TAB,	VK_TAB },
 { KEY_BACK, 	VK_BACK },
/* { KEY_LT,	VK_LT }, */
/* { KEY_GT,    VK_GT }, */
/* { KEY_PLUS,	VK_PLUS }, */
/* { KEY_MINUS,	VK_MINUS }, */
 { KEY_A,	VK_A+0 },
 { KEY_B,	VK_A+1 },
 { KEY_C,	VK_A+2 },
 { KEY_D,	VK_A+3 },
 { KEY_E,	VK_A+4 },
 { KEY_F,	VK_A+5 },
 { KEY_G,	VK_A+6 },
 { KEY_H,	VK_A+7 },
 { KEY_I,	VK_A+8 },
 { KEY_J,	VK_A+9 },
 { KEY_K,	VK_A+10 },
 { KEY_L,	VK_A+11 },
 { KEY_M,	VK_A+12 },
 { KEY_N,	VK_A+13 },
 { KEY_O,	VK_A+14 },
 { KEY_P,	VK_A+15 },
 { KEY_Q,	VK_A+16 },
 { KEY_R,	VK_A+17 },
 { KEY_S,	VK_A+18 },
 { KEY_T,	VK_A+19 },
 { KEY_U,	VK_A+20 },
 { KEY_V,	VK_A+21 },
 { KEY_W,	VK_A+22 },
 { KEY_X,	VK_A+23 },
 { KEY_Y,	VK_A+24 },
 { KEY_Z,	VK_A+25 },
 { KEY_0,	VK_0+0 },
 { KEY_1,	VK_0+1 },
 { KEY_2,	VK_0+2 },
 { KEY_3,	VK_0+3 },
 { KEY_4,	VK_0+4 },
 { KEY_5,	VK_0+5 },
 { KEY_6,	VK_0+6 },
 { KEY_7,	VK_0+7 },
 { KEY_8,	VK_0+8 },
 { KEY_9,	VK_0+9 },		
 { KEY_NULL,-1}
};		
#endif

