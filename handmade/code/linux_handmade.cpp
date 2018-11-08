/**
 * @file linux_handmade.cpp
 * @brief
 * @author Ahmed Abd El-Aal <eng.ahmedhussein89@gmail.com>
 * @version 0.0
 * @date 2018-11-08
 */

#include <SDL2/SDL.h>

int main(int argc, char *argv[]) {
  const SDL_MessageBoxData messageboxdata = {
      SDL_MESSAGEBOX_INFORMATION, // .flags
      NULL,                       // .window
      "Handmade Hero",            // .title
      "This is Handemade Hero",   // .message
      0,                          // .numbuttons
      NULL,                       // .buttons
      NULL                        // .colorScheme
  };

  int buttonid;
  SDL_ShowMessageBox(&messageboxdata, &buttonid);
  return 0;
}
