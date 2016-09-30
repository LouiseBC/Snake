#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <iostream>
#include <random>
#include "Snake.h"

// Naughty globals
//////////////////////
const int SPRITE_SIZE = 32;
const int SCREEN_WIDTH = SPRITE_SIZE*20;
const int SCREEN_HEIGHT = SPRITE_SIZE*20;
const int SCREEN_CENTRE_X = (SCREEN_WIDTH/2)-(SPRITE_SIZE/2);
const int SCREEN_CENTRE_Y = (SCREEN_HEIGHT/2)-(SPRITE_SIZE/2);
const int FPS = 15;
const int FRAME_PERIOD = 1000.0f / FPS;

enum direction { stop, left, right, up, down };
direction dir { stop };

// For randomly generating position of item
std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> posX(0, SCREEN_WIDTH-SPRITE_SIZE);
std::uniform_int_distribution<int> posY(0,SCREEN_HEIGHT-SPRITE_SIZE);

// Snake class details
//////////////////////

Snake::Snake()
: headX {SCREEN_CENTRE_X}, headY {SCREEN_CENTRE_Y}, tailN {0}, score {0}, gameOver {false} {
    setup();
    loop();
}

void Snake::setup() {
    // Set up SDL, define sprite clips and default game values.
    // Needs error checking
    
    // Init Video & Fonts
    SDL_INIT_VIDEO;
    SDL_INIT_AUDIO;
    TTF_Init();
    
    // Create Window
    window = SDL_CreateWindow("Catch the kitty", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
        std::cerr << "Error: Create Window";
    
    // Create Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
        std::cerr << "Error: Create renderer";
    SDL_SetRenderDrawColor(renderer, 133, 159, 156, -10);
    
    // Load textures
    texture = IMG_LoadTexture(renderer, "playground/sprites.png");
    if (texture == nullptr)
        std::cerr << "Error: Load texture";
    
    // Load font
    updateScore();
    
    //Audio
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    meow = Mix_LoadWAV("playground/cat_meow_x.wav");
    
    // Set clip for snake
    snakeClips[0].x = 0;
    snakeClips[0].y = 0;
    snakeClips[0].w = SPRITE_SIZE;
    snakeClips[0].h = SPRITE_SIZE;
    
    // Set clip for item ('fruit')
    snakeClips[1].x = 0;
    snakeClips[1].y = 4*SPRITE_SIZE;
    snakeClips[1].w = SPRITE_SIZE;
    snakeClips[1].h = SPRITE_SIZE;
    
    // Set clip for tail, currently not in use
    snakeClips[2].x = 0;
    snakeClips[2].y = 2*SPRITE_SIZE;
    snakeClips[2].w = SPRITE_SIZE;
    snakeClips[2].h = SPRITE_SIZE;
    
    // Set random location for item
    itemX = posX(rng);
    itemY = posY(rng);
}

void Snake::draw() {
    SDL_RenderClear(renderer);
    renderTexture(printscore, renderer, 15, 15, nullptr);  // "Score: "
    renderTexture(texture, renderer, itemX, itemY, &snakeClips[1]); // item
    renderTexture(texture, renderer, headX, headY, &snakeClips[0]); // snake head
    
    // snake tail
    if (tailN > 0){
        for (int i = 0; i < tailN; ++i){
            renderTexture(texture, renderer, tailX[i], tailY[i], &snakeClips[1]);
        }
     }
    
    SDL_RenderPresent(renderer);
}

void Snake::loop() {
    
    Uint32 frameStart, frameTime;
    SDL_Event event;
    while (gameOver != true){
        frameStart = SDL_GetTicks();
        while (SDL_PollEvent(&event)){
            
            if (event.type == SDL_QUIT)
                gameOver = true;
            
            if (event.type == SDL_KEYDOWN){
                if ((event.key.keysym.sym == SDLK_a ||
                    event.key.keysym.sym == SDLK_LEFT) && dir != right)
                    dir = left;
                if ((event.key.keysym.sym == SDLK_d ||
                     event.key.keysym.sym == SDLK_RIGHT) && dir != left)
                    dir = right;
                if ((event.key.keysym.sym == SDLK_w ||
                     event.key.keysym.sym == SDLK_UP) && dir != down)
                    dir = up;
                if ((event.key.keysym.sym == SDLK_s ||
                     event.key.keysym.sym == SDLK_DOWN) && dir != up)
                    dir = down;
                if (event.key.keysym.sym == SDLK_ESCAPE)
                    gameOver = true;
            }
        }
        logic();
        draw();
        
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_PERIOD){
            SDL_Delay((int)(FRAME_PERIOD - frameTime));
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(printscore);
    Mix_FreeChunk(meow);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

void Snake::logic() {
    // Snake movement
    if (dir == left)
        headX -= SPRITE_SIZE;
    else if (dir == right)
        headX += SPRITE_SIZE;
    else if (dir == up)
        headY -= SPRITE_SIZE;
    else if (dir == down)
        headY += SPRITE_SIZE;
    
    // Tail movement [1] to [tailN]
     for (int i = tailN-1; i > 0; --i){ // Tail starts counting at 1
            tailX[i] = tailX[i-1];
            tailY[i] = tailY[i-1];
     }
     
     // Tail[0] movement
     if (dir == left){
     tailX[0] = headX + SPRITE_SIZE;
     tailY[0] = headY;
     }
     else if (dir == right){
     tailX[0] = headX - SPRITE_SIZE;
     tailY[0] = headY;
     }
     else if (dir == up){
     tailX[0] = headX;
     tailY[0] = headY + SPRITE_SIZE;
     }
     else if (dir == down){
     tailX[0] = headX;
     tailY[0] = headY - SPRITE_SIZE;
     }
    
    // 'Pick up' item
    if (itemCollision()){
        ++score;
        ++tailN;
        Mix_PlayChannel(-1, meow, 0);
        updateScore(); // updates text texture
        itemX = posX(rng);
        itemY = posY(rng);
    }
    
    // Tail collision check
    if(tailCollision())
        gameOver = true;
    
    // When snake leaves window loop to opposite side
    if (headX + SPRITE_SIZE < 0)  headX = SCREEN_WIDTH;
    if (headX > SCREEN_WIDTH)     headX = 0 - SPRITE_SIZE;
    if (headY + SPRITE_SIZE < 0)  headY = SCREEN_HEIGHT;
    if (headY > SCREEN_HEIGHT)    headY = 0 - SPRITE_SIZE;
    
}

bool Snake::itemCollision() {
    if ( headY + SPRITE_SIZE <= itemY)    // bottom_head <= top_item
        return false;
    if ( headY >= itemY+SPRITE_SIZE )     // top_head >= bottom_item
        return false;
    if ( headX + SPRITE_SIZE <= itemX )   // right_head <= left_item
        return false;
    if ( headX >= itemX+SPRITE_SIZE )     // left_head >= right_item
        return false;
    return true;
}

bool Snake::tailCollision() {
    for (int i = 0; i < tailN; ++i) {
        if (headX == tailX[i] && headY == tailY[i])
            return true;
    }
    return false;
}

void Snake::updateScore() {
    SDL_Color fontcolour = { 255, 255, 255, 255 };
    std::string scorestring {"0"};
    std::string scoremsg {"Score: 0"};
    
    scorestring = std::to_string(score);
    scoremsg = "Score: " + scorestring;
    printscore = renderText(scoremsg, "playground/Calibril.ttf", fontcolour, 18, renderer);
}

// Render texture with existing destination rect
void Snake::renderTexture(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip = nullptr) {
    SDL_RenderCopy(ren, tex, clip, &dst);
}
// Create destination rect from x and y, render texture.
void Snake::renderTexture(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip = nullptr) {
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;
    if (clip != nullptr){
        dst.w = clip->w;
        dst.h = clip->h;
    }
    else {
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }
    SDL_RenderCopy(ren, tex, clip, &dst);
}

SDL_Texture* Snake::renderText(const std::string &message, const std::string &fontFile,
                        SDL_Color color, int fontSize, SDL_Renderer *renderer)
{
    //Open the font
    TTF_Font *font = TTF_OpenFont(fontFile.c_str(), fontSize);
    if (font == nullptr){
        std::cout << "TTF_OpenFont";
        return nullptr;
    }
    //render to a surface as that's what TTF_RenderText returns
    SDL_Surface *surf = TTF_RenderText_Blended(font, message.c_str(), color);
    if (surf == nullptr){
        TTF_CloseFont(font);
        std::cout << "TTF_RenderText";
        return nullptr;
    }
    //then load that surface into a texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surf);
    if (texture == nullptr){
        std::cout << "CreateTexture";
    }
    //Clean up the surface and font
    SDL_FreeSurface(surf);
    TTF_CloseFont(font);
    return texture;
}
