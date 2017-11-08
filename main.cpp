#include <stdio.h>
#include <stdlib.h> // rand
#include <math.h>
#include <string>
#include <vector>
#include <sstream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

struct vec2 {

    float x;
    float y;

    vec2( float x=0, float y=0 ) : x(x), y(y)
    {
    }

    vec2 operator=( const vec2& other )
    {
        x = other.x;
        y = other.y;
        return *this;
    }

    vec2 operator+( const vec2& other ) const
    {
        return vec2( other.x + x, other.y + y );
    }

    bool operator==( const vec2& other ) const
    {
        return ( other.x == x && other.y == y );
    }

    float norm()
    {
        return sqrt( x*x + y*y );
    }

    float dotProd( vec2 other )
    {
        return x * other.x + y * other.y;
    }

    std::string str()
    {
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }
};

// global variables
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const int TILE_W = 32;
const int TILE_H = 32;

const int TILE_GRASS = 0;
const int TILE_METAL = 1;

SDL_Rect gTileClips[ 2 ];

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
SDL_Texture* gTileTexture = NULL;
SDL_Texture* gFPSTexture = NULL;
TTF_Font* gFont = NULL;

bool init()
{
    // initializes SDL and creates a global window and renderer
    bool success = true;
    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
        printf( "Error initializing SDL: %s\n", SDL_GetError() );
        success = false;
    }
    gWindow = SDL_CreateWindow( "Fermi Planet",
              SDL_WINDOWPOS_CENTERED,
              SDL_WINDOWPOS_CENTERED,
              SCREEN_WIDTH,
              SCREEN_HEIGHT, 0 );
    if ( gWindow == NULL ) {
        printf( "Error creating window\n" );
        success = false;
    }
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
    if ( gRenderer == NULL ) {
        printf( "Error creating renderer\n" );
        success = false;
    }
    if ( TTF_Init() == -1 ) {
        printf( "Error initializing SDL_ttf! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }

    return success;
}

bool checkCollision( SDL_Rect A, SDL_Rect B )
{
    // if A is outside of B
    if ( A.x >= B.x+B.w || A.x+A.w <= B.x || A.y >= B.y+B.h || A.y+A.h <= B.y ) {
        return false;
    }
    return true;
}

SDL_Texture* loadTexture( SDL_Renderer* renderer, std::string path )
{
    // load a texture from file into a renderer. Return NULL on failure
    SDL_Texture* loadedTexture = NULL;
    SDL_Surface* surf = IMG_Load( path.c_str() );
    if ( surf == NULL ) {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
        return NULL;
    }
    else {
        loadedTexture = SDL_CreateTextureFromSurface( renderer, surf );
        if ( loadedTexture == NULL ) {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
            return NULL;
        }
        SDL_FreeSurface( surf );
    }

    return loadedTexture;
}

SDL_Texture* loadTextTexture( std::string textureText, SDL_Color textColor )
{
    // create a texture from a text using a global font
    SDL_Texture* loadedTexture = NULL;
    SDL_Surface* surf = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if ( surf == NULL ) {
        printf( "Unable to load text surface! SDL Error: %s\n", TTF_GetError() );
        return NULL;
    }
    else {
        loadedTexture = SDL_CreateTextureFromSurface( gRenderer, surf );
        if ( loadedTexture == NULL ) {
            printf( "Unable to create text texture from surface! SDL Error: %s\n", SDL_GetError() );
            return NULL;
        }
        SDL_FreeSurface( surf );
    }

    return loadedTexture;
}

namespace camera_constants {
    const float camSpeed = 8.f;
}

class Camera {
public:
    Camera( int w, int h );
    void handleEvent( SDL_Event& e );
    void move();

    vec2 getPos() { return this->pos; };
    SDL_Rect& rect() { return this->box; };
private:
    SDL_Rect box;
    vec2 pos;
    float velX, velY;
    float zoom;
};
Camera::Camera( int w, int h )
{
    this->box.x = 0;
    this->box.y = 0;
    this->box.w = w;
    this->box.h = h;
    this->pos = vec2( 0.0, 0.0 );
    this->velX = 0;
    this->velY = 0;
    this->zoom = 1.0;
}
void Camera::handleEvent( SDL_Event& e )
{
    // if an arrow key is pressed
    if ( e.type == SDL_KEYDOWN && e.key.repeat == 0 ) {
        switch ( e.key.keysym.sym ) {
            case SDLK_UP:
                this->velY = -camera_constants::camSpeed;
                break;
            case SDLK_DOWN:
                this->velY = camera_constants::camSpeed;
                break;
            case SDLK_RIGHT:
                this->velX = camera_constants::camSpeed;
                break;
            case SDLK_LEFT:
                this->velX = -camera_constants::camSpeed;
                break;
        }
    }
    // if an arrow key is released
    else if ( e.type == SDL_KEYUP && e.key.repeat == 0 ) {
        if ( e.key.keysym.sym == SDLK_UP || e.key.keysym.sym == SDLK_DOWN ) {
            this->velY = 0;
        }
        else if ( e.key.keysym.sym == SDLK_LEFT || e.key.keysym.sym == SDLK_RIGHT ) {
            this->velX = 0;
        }
    }
    // if mouse scroll
    if ( e.wheel.y == 1 ) { // scroll up
        this->zoom += 0.025;
        if ( this->zoom > 1.0 ) {
            this->zoom = 1.0;
        }
        this->box.w = SCREEN_WIDTH * this->zoom;
        this->box.h = SCREEN_HEIGHT * this->zoom;
        printf("zoom: %f\n", this->zoom );
    }
    else if ( e.wheel.y == -1 ) { // scroll down
        this->zoom -= 0.025;
        if ( this->zoom <= 0.025 ) {
            this->zoom = 0.025;
        }
        this->box.w = SCREEN_WIDTH * this->zoom;
        this->box.h = SCREEN_HEIGHT * this->zoom;
        printf("zoom: %f\n", this->zoom );
    }
}
void Camera::move()
{
    this->pos.x += this->velX;
    this->pos.y += this->velY;
    this->box.x = (int) this->pos.x;
    this->box.y = (int) this->pos.y;
}

class Tile {
public:
    Tile( int x, int y, int type );
    void render( Camera& cam );

    int getType() { return this->type; };
private:
    int type;
    SDL_Rect rect;
};
Tile::Tile( int x, int y, int type )
{
    this->rect.x = x;
    this->rect.y = y;
    this->rect.w = TILE_W;
    this->rect.h = TILE_H;
    this->type = type;
}
void Tile::render( Camera& cam )
{
    float scale = (float) SCREEN_WIDTH / cam.rect().w;
    if ( checkCollision( this->rect, cam.rect() ) ) {
        SDL_Rect dstRect = { (int) ( ( this->rect.x - cam.getPos().x ) * scale ),
                             (int) ( ( this->rect.y - cam.getPos().y ) * scale ),
                             (int) ( ( this->rect.w ) * scale ),
                             (int) ( ( this->rect.h ) * scale )
                           };
        SDL_RenderCopy( gRenderer, gTileTexture, &gTileClips[ this->type ], &dstRect );
    }
}

void loadChunk( std::vector<Tile>& chunk, int length )
{
    for ( int i=0; i<length; i++ ) {
        int y = i * TILE_W;
        for ( int j=0; j<length; j++ ) {
            int x = j * TILE_H;
            chunk.push_back( Tile( x, y, rand()%2 ) );
        }
    }
}

void setClips()
{
    // background clips
    const int BG_TILE_W_PX = 32;
    const int BG_TILE_H_PX = 32;
    gTileClips[ TILE_GRASS ].x = 0;
    gTileClips[ TILE_GRASS ].y = 0;
    gTileClips[ TILE_GRASS ].w = BG_TILE_W_PX;
    gTileClips[ TILE_GRASS ].h = BG_TILE_H_PX;
    gTileClips[ TILE_METAL ].x = 0;
    gTileClips[ TILE_METAL ].y = 32;
    gTileClips[ TILE_METAL ].w = BG_TILE_W_PX;
    gTileClips[ TILE_METAL ].h = BG_TILE_H_PX;
    printf("Clips set\n");
}

bool loadMedia()
{
    gFont = TTF_OpenFont( "lazy.ttf", 28 );
    if (gFont == NULL ) {
        printf( "Failed to load text font! SDL_ttf Error: %s\n", TTF_GetError() );
        return false;
    }
    return true;
}

int main( int argc, char* argv[] )
{
    if ( !init() ) {
        printf( "Error initializing SDL!\n" );
        return 3;
    }

    if ( !loadMedia() ) {
        printf("Failed to load Media!\n" );
        return 3;
    }

    gTileTexture = loadTexture( gRenderer, "textures/tilesSpritesheet.png" );
    std::vector<Tile> chunk;
    setClips();
    loadChunk( chunk, 64 );

    Camera camera( SCREEN_WIDTH, SCREEN_HEIGHT );

    SDL_Event e;

    std::stringstream FPSText;

    SDL_Color FPStextColor = { 255, 255, 0, 255 };

    int frameNumber = 0;

    const Uint32 FPSStart = SDL_GetTicks();

    // MAIN LOOP
    bool quit = false;
    while ( !quit ) {
        int FRAME_BEGIN_MS = SDL_GetTicks();

        // handle event queue
        while ( SDL_PollEvent( &e ) != 0 ) {
            if ( e.type == SDL_QUIT ) {
                quit = true;
            }

            camera.handleEvent( e );
        }


        // set positions/game state
        FPSText.str( "" );
        float FPSTime = frameNumber / ((SDL_GetTicks() - FPSStart) / 1000.f);
        if ( FPSTime > 2000000 ) {
            FPSTime = 0;
        }
        FPSText << "FPS: " << FPSTime;
        gFPSTexture = loadTextTexture( FPSText.str().c_str(), FPStextColor );
        camera.move();

        // clear the renderer
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        // draw objects to renderer
        for ( int i=0; i < 64*64; i++ ) {
            chunk[ i ].render( camera );
        }
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0, 0, 0xFF );
        SDL_RenderDrawRect( gRenderer, &camera.rect() ); // draw cam in red
        SDL_Rect FPSTextPos = { 0, 0, 200, 50 };
        SDL_RenderCopy( gRenderer, gFPSTexture, NULL, &FPSTextPos );

        // render to screen
        SDL_RenderPresent( gRenderer );
        frameNumber++;

        int FRAME_END_MS = SDL_GetTicks();
        int FRAME_ELAPSED_TIME = FRAME_END_MS - FRAME_BEGIN_MS;
        if ( FRAME_ELAPSED_TIME < 1000.f/60.f ) {
            SDL_Delay( 1000.f/60.f - FRAME_ELAPSED_TIME );
        }
    }

    SDL_Quit();
    printf( "SDL quit successfully.\n" );
    return 0;
}
