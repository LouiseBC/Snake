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
const int SQUARES = 21;
const int PADDING_TOP = 32;
const int PADDING_SIDES = 8;

const int SCREEN_WIDTH = SPRITE_SIZE * SQUARES;
const int SCREEN_HEIGHT = SPRITE_SIZE * SQUARES;
const int SCREEN_CENTRE_X = PADDING_SIDES + (SCREEN_WIDTH/2)-(SPRITE_SIZE/2);
const int SCREEN_CENTRE_Y = PADDING_TOP + (SCREEN_HEIGHT/2)-(SPRITE_SIZE/2);

const int WINDOW_WIDTH = SCREEN_WIDTH + 2 * PADDING_SIDES;
const int WINDOW_HEIGHT = SCREEN_HEIGHT + PADDING_TOP + PADDING_SIDES;

const int FPS = 15;
const int FRAME_PERIOD = 1000.0f / FPS;

enum direction { stop, left, right, up, down };
direction dir { stop };

// For randomly generating position of item
std::random_device rd;
std::mt19937 rng(rd());
std::uniform_int_distribution<int> posX(0, SQUARES-1); // -1 to account for sprite size
std::uniform_int_distribution<int> posY(0,SQUARES-1);

// Snake class details
//////////////////////

Snake::Snake()
: headX {SCREEN_CENTRE_X}, headY {SCREEN_CENTRE_Y}, tailN {0}, score {0}, gameOver {false}, turnOver {false}, pause {false} {
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
    
    // Splash Screen
    /////////////////
    SDL_Window* splashwindow = SDL_CreateWindow("Splash", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 400, 250, SDL_WINDOW_SHOWN);
    SDL_Surface* splashsurface = SDL_GetWindowSurface( splashwindow );
    SDL_Surface* splash = IMG_Load("playground/snMEOWake.png");

    
    SDL_BlitSurface( splash, NULL, splashsurface, NULL );
    SDL_UpdateWindowSurface( splashwindow );
    SDL_Delay(2000);
    
    SDL_FreeSurface(splash);
    SDL_FreeSurface(splashsurface);
    SDL_DestroyWindow(splashwindow);
    
    // Main Game Window
    ////////////////////
    window = SDL_CreateWindow("sn-MEOW-ake", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr)
        std::cerr << "Error: Create Window";
    
    // Create Renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == nullptr)
        std::cerr << "Error: Create renderer";
    
    // Load textures
    texture = IMG_LoadTexture(renderer, "playground/sprites.png");
    if (texture == nullptr)
        std::cerr << "Error: Load texture";
    
    // Load scores
    updateScore();
    getHighScore();
    updateHighScore();
    
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
    
    // Set random location for item
    itemX = posX(rng) * SPRITE_SIZE + PADDING_SIDES;
    itemY = posY(rng) * SPRITE_SIZE + PADDING_TOP;
}

void Snake::draw() {
    // darkest grey shade, background
    SDL_SetRenderDrawColor(renderer, 52, 51, 50, 0);
    SDL_RenderClear(renderer);
    
    // Fill screen rectangle
    SDL_SetRenderDrawColor(renderer, 172, 119, 120, 22);
    SDL_Rect screenfill { PADDING_SIDES, PADDING_TOP, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderFillRect(renderer, &screenfill);
    
    // Draw game elements
    //////////////////////
    renderTexture(printscore, renderer, PADDING_SIDES+3, 10, nullptr);     // Score
    renderTexture(printhighscore, renderer, SCREEN_WIDTH/2+205, 10, nullptr); // Highscore
    renderTexture(texture, renderer, itemX, itemY, &snakeClips[1]);        // item
    renderTexture(texture, renderer, headX, headY, &snakeClips[0]);        // snake head
    
    // snake tail
    if (tailN > 0){
        for (int i = 0; i < tailN; ++i)
            renderTexture(texture, renderer, tailX[i], tailY[i], &snakeClips[1]);
     }
    
    // Game Over text rect - Messy. to do.
    SDL_Rect gameoverRect; //gameover
    SDL_Rect gameoverRect2; // press space
    void getRect(const SDL_Texture t);
    SDL_QueryTexture(printgameover, NULL, NULL, &gameoverRect.w, &gameoverRect.h);
    SDL_QueryTexture(printreplay, NULL, NULL, &gameoverRect2.w, &gameoverRect2.h);
    gameoverRect.x = PADDING_SIDES + (SCREEN_WIDTH/2)-(gameoverRect.w/2);
    gameoverRect.y = PADDING_TOP + (SCREEN_HEIGHT/2)-(gameoverRect.h); // higher than centre - personal preference
    gameoverRect2.x = (SCREEN_WIDTH/2)-(gameoverRect2.w/2);
    gameoverRect2.y = gameoverRect.y + gameoverRect.h + gameoverRect2.h;
    
    // Game Over text rendering
    renderTexture(printgameover, renderer, gameoverRect, nullptr); // Game Over
    renderTexture(printreplay, renderer, gameoverRect2, nullptr); // Space to continue
    
    SDL_RenderPresent(renderer);
}

void Snake::loop() {
    
    Uint32 frameStart, frameTime;
    SDL_Event event;
    while (gameOver == false){
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
                if (event.key.keysym.sym == SDLK_ESCAPE){
                    gameOver = true;
                    dir = stop;
                }
                if (event.key.keysym.sym == SDLK_SPACE){
                    if (turnOver == true)      // Play again when turn is over
                        reset();
                    else if (pause == true)
                        pause = false;
                    else if (pause == false)
                        pause = true;
                }
            }
        }
        if (pause == false && tailCollision() == false){
            logic();
            draw();
        }
        
        frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < FRAME_PERIOD){
            SDL_Delay((int)(FRAME_PERIOD - frameTime));
        }
    }
    SDL_DestroyTexture(texture);
    SDL_DestroyTexture(printscore);
    SDL_DestroyTexture(printgameover);
    SDL_DestroyTexture(printreplay);
    Mix_FreeChunk(meow);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
}

void Snake::logic() {
    // 'Pick up' item
    if (itemCollision()){
        score += 10;
        if (score > highScore) updateHighScore();
        ++tailN;
        Mix_PlayChannel(-1, meow, 0);
        updateScore(); // updates text texture
        itemX = posX(rng) * SPRITE_SIZE + PADDING_SIDES;
        itemY = posY(rng) * SPRITE_SIZE + PADDING_TOP;
    }
    
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
    
    // Tail collision check
    if(tailCollision()) {
        // Render GAME OVER message
        SDL_Color fontcolour = { 255, 255, 255, 255 };
        printgameover = renderText("GAME OVER", "playground/GreenFlame.ttf", fontcolour, 60, renderer);
        printreplay = renderText("Press spacebar to continue", "playground/GreenFlame.ttf", fontcolour, 13, renderer);
        turnOver = true;
    }
    
    // When snake leaves window loop to opposite side
    if (headX < PADDING_SIDES)                                  headX = SCREEN_WIDTH + PADDING_SIDES - SPRITE_SIZE;
    if (headX > (PADDING_SIDES + SCREEN_WIDTH - SPRITE_SIZE) )  headX = PADDING_SIDES;
    if (headY < PADDING_TOP)                                    headY = SCREEN_HEIGHT + PADDING_TOP - SPRITE_SIZE;
    if (headY > (PADDING_TOP + SCREEN_HEIGHT - SPRITE_SIZE) )   headY = PADDING_TOP;
}

bool Snake::itemCollision() {
    if ( headY + SPRITE_SIZE <= itemY )    // bottom_head <= top_item
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

void Snake::reset() {
    printgameover = nullptr;
    printreplay = nullptr;
    headX = SCREEN_CENTRE_X;
    headY = SCREEN_CENTRE_Y;
    itemX = posX(rng) * SPRITE_SIZE + PADDING_SIDES;
    itemY = posY(rng) * SPRITE_SIZE + PADDING_TOP;
    
    dir = stop;
    tailN = 0;
    score = 0;
    updateScore();
}

void Snake::updateScore() {
    SDL_Color fontcolour = { 240, 228, 228, 4 }; // light red / white
    
    std::string scorestring = std::to_string(score);
    std::string scoremsg = "score: " + scorestring;
    printscore = renderText(scoremsg, "playground/GreenFlame.ttf", fontcolour, 16, renderer);
}

void Snake::updateHighScore() {
    if (score >= highScore) {
        highScore = score;
        
        // Update highscores file
        SDL_RWops* highscores = SDL_RWFromFile("playground/highscores.txt", "w");
        if (highscores == NULL)
            printf( "Warning: Unable to open file! SDL Error: %s\n", SDL_GetError() );
        SDL_RWwrite(highscores, &highScore, sizeof(int), 1);
        SDL_RWclose(highscores);
    }
    
    // Render new score
    SDL_Color fontcolour = { 240, 228, 228, 4 }; // light red / white
    
    std::string scorestring = std::to_string(highScore);
    std::string scoremsg = "highscore: " + scorestring;
    printhighscore = renderText(scoremsg, "playground/GreenFlame.ttf", fontcolour, 16, renderer);
}

void Snake::getHighScore() {
    SDL_RWops* highscores = SDL_RWFromFile("playground/highscores.txt", "r");
    if (highscores == NULL){
        std::cerr << "Error: Could not open highscores.txt or file does not exist" << std::endl;
        highScore = score;
    }
    else SDL_RWread(highscores, &highScore, sizeof(int), 1);
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
    SDL_Surface *surf = TTF_RenderText_Solid(font, message.c_str(), color);
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
