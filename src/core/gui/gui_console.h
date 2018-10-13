
#ifndef ENGINE_GUI_CONSOLE_H
#define ENGINE_GUI_CONSOLE_H

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

#include "gui_obj.h"


void Gui_RefreshConsole(gui_object_p *con_root, int w, int h);
void Gui_ConScrollInternal(gui_object_p con_root, int value);
void Gui_HandleEditConsole(int cmd, uint32_t key, void *data);

#ifdef	__cplusplus
}
#endif

#endif
