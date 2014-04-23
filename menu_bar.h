
#ifndef MENU_BAR_H
#define MENU_BAR_H

union SDL_Event;

/* Creates the debug menu bar. OpenGL must already be initialized and
the window size must be provided because the menu will overlay the whole
window content. */
void MenuBar_Init(int wnd_width, int wnd_height);

/* Destroys the menu bar. Should be called before the window is destroyed. */
void MenuBar_Destroy();

/* Pass an SDL_Event to the menu bar to be processed. Returns true if the event
has been processed by the menu and false otherwise. */
bool MenuBar_Event(const SDL_Event& event);

void MenuBar_Render();

bool MenuBar_IsVisible();
void MenuBar_Show();
void MenuBar_Hide();
void MenuBar_ToggleVisibility();

#endif // MENU_BAR_H
