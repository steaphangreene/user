#ifndef INSOMNIA_USER_KEY_DEFS_H
#define INSOMNIA_USER_KEY_DEFS_H

#include "config.h"

#ifdef DOS
#define START_KEYBOARD_INTERRUPT() __asm__ __volatile__ ("pushal\n	pushw %ds\n	pushl %esi");
#define END_KEYBOARD_INTERRUPT() __asm__ __volatile__ ("inb $0x61, %al\n\torb $0x80, %al\n\toutb %al, $0x61\n\tandb $0x7f, %al\n\toutb %al, $0x61\n\tmovb $0x20, %al\n\toutb %al, $0x20\n\tpopl %esi\n\tpopw %ds\n\tpopal\n\tpopl %ebp\n\tsti\n\tiret");

#define SCAN_ESC        0x01
#define SCAN_1          0x02
#define SCAN_2          0x03
#define SCAN_3          0x04
#define SCAN_4          0x05
#define SCAN_5          0x06
#define SCAN_6          0x07
#define SCAN_7          0x08
#define SCAN_8          0x09
#define SCAN_9          0x0a
#define SCAN_0          0x0b
#define SCAN_MINUS      0x0c
#define SCAN_EQUALS     0x0d
#define SCAN_BACKSP     0x0e
#define SCAN_TAB        0x0f
#define SCAN_Q          0x10
#define SCAN_W          0x11
#define SCAN_E          0x12
#define SCAN_R          0x13
#define SCAN_T          0x14
#define SCAN_Y          0x15
#define SCAN_U          0x16
#define SCAN_I          0x17
#define SCAN_O          0x18
#define SCAN_P          0x19
#define SCAN_LBRAK      0x1a
#define SCAN_RBRAK      0x1b
#define SCAN_RETURN     0x1c
#define SCAN_RCTRL      0x1d
#define SCAN_LCTRL      0x1d
#define SCAN_A          0x1e
#define SCAN_S          0x1f
#define SCAN_D          0x20
#define SCAN_F          0x21
#define SCAN_G          0x22
#define SCAN_H          0x23
#define SCAN_J          0x24
#define SCAN_K          0x25
#define SCAN_L          0x26
#define SCAN_SCOLON     0x27
#define SCAN_QUOTE      0x28
#define SCAN_BQUOTE     0x29
#define SCAN_TILDE      0x29
#define SCAN_LSHIFT     0x2a
#define SCAN_BSLASH     0x2b
#define SCAN_Z          0x2c
#define SCAN_X          0x2d
#define SCAN_C          0x2e
#define SCAN_V          0x2f
#define SCAN_B          0x30
#define SCAN_N          0x31
#define SCAN_M          0x32
#define SCAN_COMA       0x33
#define SCAN_DOT        0x34
#define SCAN_SLASH      0x35
#define SCAN_GREY_SLASH 0x35
#define SCAN_RSHIFT     0x36
#define SCAN_GREY_STAR  0x37
#define SCAN_RALT       0x38
#define SCAN_LALT       0x38
#define SCAN_SPACE      0x39
#define SCAN_CAPS       0x3a
#define SCAN_F1         0x3b
#define SCAN_F2         0x3c
#define SCAN_F3         0x3d
#define SCAN_F4         0x3e
#define SCAN_F5         0x3f
#define SCAN_F6         0x40
#define SCAN_F7         0x41
#define SCAN_F8         0x42
#define SCAN_F9         0x43
#define SCAN_F10        0x44
#define SCAN_NUMLOCK    0x45
#define SCAN_SCRLOCK    0x46
#define SCAN_HOME       0x47
#define SCAN_PAD_7      0x47
#define SCAN_UP         0x48
#define SCAN_PAD_8      0x48
#define SCAN_PGUP       0x49
#define SCAN_PAD_9      0x49
#define SCAN_GREY_MINUS 0x4a
#define SCAN_LEFT       0x4b
#define SCAN_PAD_4      0x4b
#define SCAN_PAD_5      0x4c
#define SCAN_RIGHT      0x4d
#define SCAN_PAD_6      0x4d
#define SCAN_GREY_PLUS  0x4e
#define SCAN_END        0x4f
#define SCAN_PAD_1      0x4f
#define SCAN_DOWN       0x50
#define SCAN_PAD_2      0x50
#define SCAN_PGDN       0x51
#define SCAN_PAD_3      0x51
#define SCAN_INSERT     0x52
#define SCAN_PAD_0      0x52
#define SCAN_DEL        0x53
#define SCAN_PAD_DOT    0x53
#define SCAN_ENTER      0x1c
#define SCAN_F11        0x57
#define SCAN_F12        0x58
#define SCAN_PAUSE	0x45 //Scroll Lock !!!
#endif

#ifdef X_WINDOWS
#include <X11/keysym.h>

#define SCAN_ESC        XK_Escape
#define SCAN_1          XK_1
#define SCAN_2          XK_2
#define SCAN_3          XK_3
#define SCAN_4          XK_4
#define SCAN_5          XK_5
#define SCAN_6          XK_6
#define SCAN_7          XK_7
#define SCAN_8          XK_8
#define SCAN_9          XK_9
#define SCAN_0          XK_0
#define SCAN_MINUS      XK_minus
#define SCAN_EQUALS     XK_equal
#define SCAN_BACKSP     XK_BackSpace
#define SCAN_TAB        XK_Tab
#define SCAN_Q          XK_q
#define SCAN_W          XK_w
#define SCAN_E          XK_e
#define SCAN_R          XK_r
#define SCAN_T          XK_t
#define SCAN_Y          XK_y
#define SCAN_U          XK_u
#define SCAN_I          XK_i
#define SCAN_O          XK_o
#define SCAN_P          XK_p
#define SCAN_TILDE      XK_tilde
#define SCAN_BQUOTE     XK_quoteleft
#define SCAN_QUOTE      XK_quoteright
#define SCAN_RETURN     XK_Return
#define SCAN_RCTRL      XK_Control_R
#define SCAN_LCTRL      XK_Control_L
#define SCAN_A          XK_a
#define SCAN_S          XK_s
#define SCAN_D          XK_d
#define SCAN_F          XK_f
#define SCAN_G          XK_g
#define SCAN_H          XK_h
#define SCAN_J          XK_j
#define SCAN_K          XK_k
#define SCAN_L          XK_l
#define SCAN_SCOLON     XK_semicolon
#define SCAN_LBRAK     XK_braketleft
#define SCAN_RBRAK     XK_braketright
#define SCAN_LSHIFT     XK_Shift_L
#define SCAN_BSLASH     XK_backslash
#define SCAN_Z          XK_z
#define SCAN_X          XK_x
#define SCAN_C          XK_c
#define SCAN_V          XK_v
#define SCAN_B          XK_b
#define SCAN_N          XK_n
#define SCAN_M          XK_m
#define SCAN_COMA       XK_comma
#define SCAN_DOT        XK_period
#define SCAN_SLASH      XK_slash
#define SCAN_RSHIFT     XK_Shift_R
#define SCAN_GREY_STAR  XK_KP_Multiply
#define SCAN_GREY_SLASH XK_KP_Divide
#define SCAN_RALT        XK_Alt_R
#define SCAN_LALT        XK_Alt_L
#define SCAN_SPACE      XK_space
#define SCAN_CAPS       XK_Caps_Lock
#define SCAN_F1         XK_F1
#define SCAN_F2         XK_F2
#define SCAN_F3         XK_F3
#define SCAN_F4         XK_F4
#define SCAN_F5         XK_F5
#define SCAN_F6         XK_F6
#define SCAN_F7         XK_F7
#define SCAN_F8         XK_F8
#define SCAN_F9         XK_F9
#define SCAN_F10        XK_F10
#define SCAN_NUMLOCK    XK_Num_Lock
#define SCAN_SCRLOCK    XK_Scroll_Lock
#define SCAN_HOME       XK_Home
#define SCAN_UP         XK_Up
#define SCAN_PGUP       XK_Page_Up
#define SCAN_GREY_MINUS XK_KP_Subtract
#define SCAN_LEFT       XK_Left
#define SCAN_PAD_1      XK_KP_1
#define SCAN_PAD_2      XK_KP_2
#define SCAN_PAD_3      XK_KP_3
#define SCAN_PAD_4      XK_KP_4
#define SCAN_PAD_5      XK_KP_5
#define SCAN_PAD_6      XK_KP_6
#define SCAN_PAD_7      XK_KP_7
#define SCAN_PAD_8      XK_KP_8
#define SCAN_PAD_9      XK_KP_9
#define SCAN_PAD_0      XK_KP_0
#define SCAN_PAD_DOT    XK_KP_Decimal
#define SCAN_ENTER      XK_KP_Enter
#define SCAN_RIGHT      XK_Right
#define SCAN_GREY_PLUS  XK_KP_Add
#define SCAN_END        XK_End
#define SCAN_DOWN       XK_Down
#define SCAN_PGDN       XK_Page_Down
#define SCAN_INSERT     XK_Insert
#define SCAN_DEL        XK_Delete
#define SCAN_F11        XK_F11
#define SCAN_F12        XK_F12
#define SCAN_PAUSE	XK_Pause
#endif

#define KEY_DELETE      128
#define KEY_BACKSPACE   '\b'
#define KEY_ESCAPE      '\e'
#define KEY_LEFT        129
#define KEY_RIGHT       130
#define KEY_UP          131
#define KEY_DOWN        132
#define KEY_END         133
#define KEY_HOME        134
#define KEY_PGUP        135
#define KEY_PGDN        136
#define KEY_INSERT	137
#endif
