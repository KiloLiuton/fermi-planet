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
    Camera();
    void move();

    SDL_Rect getBox() { return this->box; };
private:
    SDL_Rect box;
};

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
void Tile::render( SDL_Rect& camera )
{
    if ( checkCollision( this->rect, camera ) ) {
        SDL_Rect dstRect = { this->rect.x - camera.x,
                             this->rect.y - camera.y,
                             TILE_W,
                             TILE_H
                           };
        SDL_RenderCopy( gRenderer, gTexture, &gTileClips[ this->type ], &dstRect );
    }
}

void setTiles( std::vector<Tile>* tiles )
{
    int tileCols = 16;
    int tileRows = 9;
    for ( int i=0; i<tileRows; i++ ) {
        int y = i*TILE_W;
        for ( int j=0; j<tileCols; j++ ) {
            int x = j*TILE_H;
            tiles->push_back( Tile( x, y, rand()%2 ) );
        }
    }

    gTileClips[ TILE_GRASS ].x = 0;
    gTileClips[ TILE_GRASS ].y = 0;
    gTileClips[ TILE_GRASS ].w = TILE_PX_W;
    gTileClips[ TILE_GRASS ].h = TILE_PX_H;
    gTileClips[ TILE_METAL ].x = 0;
    gTileClips[ TILE_METAL ].y = 32;
    gTileClips[ TILE_METAL ].w = TILE_PX_W;
    gTileClips[ TILE_METAL ].h = TILE_PX_H;
    printf("Tiles set\n");
}

int main( int argc, char* argv[] )
{
    if ( !initSDL() ) {
        printf( "Error initializing SDL\n" );
        return 3;
    }

    gTexture = loadTexture( gRenderer, "textures/tilesSpritesheet.png" );
    std::vector<Tile> tiles;
    setTiles( &tiles );

    SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };

    SDL_Event e;

    // MAIN LOOP
    bool quit = false;
    while ( !quit ) {
        // handle event queue
        while ( SDL_PollEvent( &e ) != 0 ) {
            if ( e.type == SDL_QUIT ) {
                quit = true;
            }

            // TODO: camera.handleEvent( e );
        }

        // set positions/game state

        // clear the renderer
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        // draw objects to renderer
        for ( int i=0; i < 9*16; i++ ) {
            tiles[ i ].render( camera );
        }
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0, 0, 0xFF );
        SDL_RenderDrawRect( gRenderer, &camera ); // draw cam in red

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
