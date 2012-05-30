/* button.h - button interface */

#ifndef BUTTON_H
#define BUTTON_H

typedef char keycode_t;

typedef struct _button_mouse_2d button_mouse;

/**
 * Mouse button structure
 */
struct _button_mouse_2d
{
	struct _button_mouse_2d *next;
	char* label;                 /*!< Label on the button */
	int left;                    /*!< Column containing the left edge of the button */
	int right;                   /*!< Column containing the right edge of the button */
	int top;                     /*!< Row containing the left edge of the button */
	int bottom;                  /*!< Row containing the right edge of the button */
	keycode_t key;               /*!< Keypress corresponding to the button */
	byte mods;                   /*!< modifiers sent with the press */
	//byte id;
	//byte list;                 /*!< button list to switch to on press */
};


/** Function prototype for the UI to provide to create native buttons */
typedef int (*button_add_2d_f)(button_mouse* button);
typedef int (*button_add_1d_f)(const char *, keycode_t);

/** Function prototype for the UI to provide to remove native buttons */
typedef int (*button_kill_f)(keycode_t);

/** Function prototype for the UI to provide to draw native buttons */
typedef size_t (*button_print_f)(int row_1d, int col_1d);

/** Function prototype for the UI to provide to test native buttons */
typedef int (*button_get_f)(int x, int y);

button_mouse* button_add_start(int top, int left, keycode_t keypress);
int button_add_end(button_mouse* button,
                   const char *label, keycode_t keypress,
                   int bottom, int right);
int button_add_2d(int top, int left, int bottom, int right,
                  const char *label, keycode_t keypress);

bool button_backup_all(void);
void button_restore(void);

int button_add_1d(const char *label, keycode_t keypress); /* 1d button */
int button_kill(keycode_t keypress);

void button_kill_all(void);
void button_init(void);
void button_free(void);

void button_hook(button_add_2d_f add, button_add_1d_f add_1d,
                 button_kill_f kill, button_print_f print,
                 button_get_f get);

keycode_t button_get_key(int x, int y);
size_t button_print(int row_1d, int col_1d);


extern keycode_t mouse_press;

#endif /* !BUTTON_H */
