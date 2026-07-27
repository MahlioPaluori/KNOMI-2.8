#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include <lvgl.h>

namespace StatusBar {
void create(lv_obj_t *parent);
void destroy(void);
void show(void);
void hide(void);
lv_obj_t *getRoot(void);
void setStatusMessage(const char *text);
void setPrinterState(const char *state);
void setClock(const char *time);
}

#endif
