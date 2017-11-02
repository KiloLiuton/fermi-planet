#include <stdio.h>
#include <stdlib.h> // rand
#include <string>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// global variables
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const float TILE_SCALE = 1.0;
const int TILE_PX_W = 32;
const int TILE_PX_H = 32;
const int TILE_W = TILE_PX_W*TILE_SCALE;
const int TILE_H = TILE_PX_W*TILE_SCALE;

const int TILE_GRASS = 0;
const int TILE_METAL = 1;

SDL_Rect gTileClips[ 2 ];

SDL_Window* gWindow;
SDL_Renderer* gRenderer;
SDL_Texture* gTexture;

bool initSDL()
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
    } else {
        loadedTexture = SDL_CreateTextureFromSurface( renderer, surf );
        if ( loadedTexture == NULL ) {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
            return NULL;
        }
        SDL_FreeSurface( surf );
    }

    return loadedTexture;
}

class Camera {
public:
    static const int camSpeed = 1;
    Camera( int w, int h );
    void handleEvent( SDL_Event& e );
    void move();

    SDL_Rect& rect() { return box; };
private:
    SDL_Rect box;
    int velX, velY;
};
Camera::Camera( int w, int h )
{
    box.x = 0;
    box.y = 0;
    box.w = w;
    box.h = h;

    velX = 0;
    velY = 0;
}
void Camera::handleEvent( SDL_Event& e )
{
    if ( e.type == SDL_KEYDOWN && e.key.repeat == 0 ) {
        switch ( e.key.keysym.sym ) {
            case SDLK_UP:
                printf("UP pressed\n");
                velY -= camSpeed;
                break;
            case SDLK_DOWN:
                printf("DOWN pressed\n");
                velY += camSpeed;
                break;
            case SDLK_RIGHT:
                printf("RIGHT pressed\n");
                velX += camSpeed;
                break;
            case SDLK_LEFT:
                printf("LEFT pressed\n");
                velX -= camSpeed;
                break;
        }
    }
    else if ( e.type == SDL_KEYUP && e.key.repeat == 0 ) {
        switch ( e.key.keysym.sym ) {
            case SDLK_UP: velY += camSpeed; break;
            case SDLK_DOWN: velY -= camSpeed; break;
            case SDLK_RIGHT: velX -= camSpeed; break;
            case SDLK_LEFT: velX += camSpeed; break;
        }
    }
}
void Camera::move()
{
    box.x += velX;
    box.y += velY;
}

class Tile {
public:
    Tile( int x, int y, int type );
    void render( SDL_Rect& camera );

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
void Tile::render( SDL_Rect& camRect )
{
    if ( checkCollision( this->rect, camRect ) ) {
        SDL_Rect dstRect = { this->rect.x - camRect.x,
                             this->rect.y - camRect.y,
                             TILE_W,
                             TILE_H
                           };
        SDL_RenderCopy( gRenderer, gTexture, &gTileClips[ this->type ], &dstRect );
    }
}

void loadChunk( std::vector<Tile>& chunk )
{
    int tileCols = 40;
    int tileRows = 40;
    for ( int i=0; i<tileRows; i++ ) {
        int y = i*TILE_W;
        for ( int j=0; j<tileCols; j++ ) {
            int x = j*TILE_H;
            chunk.push_back( Tile( x, y, rand()%2 ) );
        }
    }
}

void setClips()
{
    gTileClips[ TILE_GRASS ].x = 0;
    gTileClips[ TILE_GRASS ].y = 0;
    gTileClips[ TILE_GRASS ].w = TILE_PX_W;
    gTileClips[ TILE_GRASS ].h = TILE_PX_H;
    gTileClips[ TILE_METAL ].x = 0;
    gTileClips[ TILE_METAL ].y = 32;
    gTileClips[ TILE_METAL ].w = TILE_PX_W;
    gTileClips[ TILE_METAL ].h = TILE_PX_H;
    printf("Clips set\n");
}

int main( int argc, char* argv[] )
{
    if ( !initSDL() ) {
        printf( "Error initializing SDL\n" );
        return 3;
    }

    gTexture = loadTexture( gRenderer, "textures/tilesSpritesheet.png" );
    std::vector<Tile> tiles;
    setClips();
    loadChunk( tiles );

    Camera camera( SCREEN_WIDTH, SCREEN_HEIGHT );

    SDL_Event e;

    // MAIN LOOP
    bool quit = false;
    while ( !quit ) {
        // handle event queue
        while ( SDL_PollEvent( &e ) != 0 ) {
            if ( e.type == SDL_QUIT ) {
                quit = true;
            }

            camera.handleEvent( e );
        }

        // set positions/game state
        camera.move();

        // clear the renderer
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        // draw objects to renderer
        for ( int i=0; i < 40*40; i++ ) {
            tiles[ i ].render( camera.rect() );
        }
        //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0, 0, 0xFF );
        //SDL_RenderDrawRect( gRenderer, &camera.rect() ); // draw cam in red

        /*
        // draw blue and red test rectangles
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0, 0, 0xFF );
        SDL_Rect redRect = { SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 400, 200 };
        SDL_RenderDrawRect( gRenderer, &redRect );
        SDL_SetRenderDrawColor( gRenderer, 0, 0, 0xFF, 0xFF );
        SDL_Rect bluRect = { SCREEN_WIDTH/2, SCREEN_HEIGHT/2+200, 400, 200 };
        SDL_RenderDrawRect( gRenderer, &bluRect );
        */

        // render to screen
        SDL_RenderPresent( gRenderer );
    }

    SDL_Quit();
    printf( "SDL quit successfully.\n" );
    return 0;
}
