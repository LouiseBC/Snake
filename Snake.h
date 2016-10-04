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
    void reset();
    
    bool itemCollision();
    bool tailCollision();
    void updateScore();
    void updateHighScore();
    void getHighScore();
    void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip);
    void renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip);
    SDL_Texture* renderText(const std::string &message, const std::string &fontFile, SDL_Color color, int fontSize, SDL_Renderer *renderer);
private:
    bool gameOver;
    bool turnOver;
    bool pause;
    int score;
    int highScore;
    int tailN;
    
    int headX;
    int headY;
    int tailX[21*21];
    int tailY[21*21];
    
    int itemX;
    int itemY;
    
    SDL_Rect snakeClips[2];

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
    SDL_Texture *printscore = NULL;
    SDL_Texture *printhighscore = NULL;
    SDL_Texture *printgameover = NULL;
    SDL_Texture *printreplay = NULL;
    Mix_Chunk *meow = NULL;
};

#endif
