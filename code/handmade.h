#pragma once
/*
 * TODO(Hussein): Services that the platform layer provide to the game
 */

/*
 * NOTE(Hussein): Services that the game provides to the platform layer/
 * (this may expand in the future - sound on separate thread, etc.)
 */

// FOUR THINGS - timing, contoller/keyboard input, bitmap buffer to use, sound buffer to use
//
// TODO(Hussein): In the future, rendering _specifically_ will become a three-tiered abstraction
struct game_offscreen_buffer {
  // NOTE(Hessein): Pixels are alwasy 32bits wide, Memory order BB GG RR XX
  void *Memory;
  int Width;
  int Height;
  int Pitch;
};

internal void GameUpdateAndRender(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset);

