#ifndef Snake_h
#define Snake_h

#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <string>

class Snake {
public:
    Snake();
    
    void setup();
    void loop();
    void draw();
    void logic();
    void replay();
    
    void updateScore();
    void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip);
    void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip);
    SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer);
    bool itemCollision();
    bool tailCollision();
private:
    bool gameOver;
    int score;
    int tailN;
    
    int headX;
    int headY;
    int tailX[50];
    int tailY[50];
    
    int itemX;
    int itemY;
    
    SDL_Rect snakeClips[4];
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Texture *printscore;
    SDL_Texture *gameover;
    Mix_Chunk *meow;
};

#endif
