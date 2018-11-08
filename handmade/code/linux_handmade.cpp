/**
 * @file linux_handmade.cpp
 * @brief
 * @author Ahmed Abd El-Aal <eng.ahmedhussein89@gmail.com>
 * @version 0.0
 * @date 2018-11-08
 */
#include <cstdio>
#include <SDL2/SDL.h>


void mainLoop(SDL_Window *pWindow) {
  // Create renderer attached to current Window
  SDL_Renderer* pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_PRESENTVSYNC);

  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        printf("Window close\n");
        done = true;
      } break;

      case SDL_WINDOWEVENT: {
        switch (event.window.event) {
        case SDL_WINDOWEVENT_RESIZED: {
          printf("Window resized (%d, %d)\n", event.window.data1,
                 event.window.data2);
        } break;
        case SDL_WINDOWEVENT_FOCUS_GAINED: {
          printf("Window Active\n");
        } break;
        case SDL_WINDOWEVENT_FOCUS_LOST: {
          printf("Window not Active\n");
        } break;
        }
      } break;
      }
    }

    static bool operation = true;
    // Set Clear color
    if(operation) {
      SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 255);
      operation = false;
    } else {
      SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 255);
      operation = true;
    }
    // Clear renderer
    SDL_RenderClear(pRenderer);
    // update current window
    SDL_RenderPresent(pRenderer);
  }
  // Destory Renderer
  SDL_DestroyRenderer(pRenderer);
}

int main(int argc, char *argv[]) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "Can not initialize SDL2\n");
    return 1;
  }

  auto pWindow =
      SDL_CreateWindow("Handmade Hero", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_RESIZABLE);
  if (!pWindow) {
    fprintf(stderr, "Can not create SDL2 Window\n");
    return -1;
  }

  if(SDL_GL_SetSwapInterval(1) < 0) {
    fprintf(stderr, "Can not enable SDL2 vSync\n");
  }

  mainLoop(pWindow);

  SDL_DestroyWindow(pWindow);
  SDL_Quit();
  return 0;
}
