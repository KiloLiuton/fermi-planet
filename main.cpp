#include <stdio.h>
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// global variables
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 960;

const float TILE_SCALE = 2.6;
const int TILE_PX_W = 32;
const int TILE_PX_H = 32;
const int TILE_W = TILE_PX_W*TILE_SCALE;
const int TILE_H = TILE_PX_W*TILE_SCALE;

const int TILE_GRASS = 0;
const int TILE_METAL = 1;
const int N_TILES = 2;

SDL_Rect gTileClips[ N_TILES ];

SDL_Window* gWindow;
SDL_Renderer* gRenderer;
SDL_Texture* gTexture;

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

void setTileClips( SDL_Rect* clips )
{
    clips[ TILE_GRASS ].x = 0;
    clips[ TILE_GRASS ].y = 0;
    clips[ TILE_GRASS ].w = TILE_PX_W;
    clips[ TILE_GRASS ].h = TILE_PX_H;

    clips[ TILE_METAL ].x = 0;
    clips[ TILE_METAL ].y = 32;
    clips[ TILE_METAL ].w = TILE_PX_W;
    clips[ TILE_METAL ].h = TILE_PX_H;
}

const int map[12][12] = 
    {
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,1,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,1,0,0,1,1,0,1,0,0,0,0},
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,0,0,0,1,1,1,1,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,0,0,0,0,0,0,0},
        {0,0,1,0,0,0,0,0,0,0,1,0},
        {0,0,0,0,0,0,0,0,0,0,0,0}
    };

int main( int argc, char* argv[] )
{
    if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 ) {
        printf( "Error initializing SDL: %s\n", SDL_GetError() );
        return 3;
    }
    gWindow = SDL_CreateWindow( "Fermi Planet",
              SDL_WINDOWPOS_CENTERED,
              SDL_WINDOWPOS_CENTERED,
              SCREEN_WIDTH,
              SCREEN_HEIGHT, 0 );
    if ( gWindow == NULL ) {
        printf( "Error creating window\n" );
        return 3;
    }
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED );
    if ( gRenderer == NULL ) {
        printf( "Error creating renderer\n" );
        return 3;
    }

    gTexture = loadTexture( gRenderer, "textures/tilesSpritesheet.png" );
    setTileClips( gTileClips );


    SDL_Event e;
    // MAIN LOOP
    bool quit = false;
    while ( !quit ) {
        // handle event queue
        while ( SDL_PollEvent( &e ) != 0 ) {
            if ( e.type == SDL_QUIT ) {
                quit = true;
            }
        }

        // set positions/game state

        // clear the renderer
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );

        // draw objects to renderer
        SDL_Rect dstRect = { 0, 0, TILE_W, TILE_H };
        for ( int i=0; i<12; i++ ) {
            dstRect.y = i*TILE_H;
            for ( int j=0; j<12; j++) {
                dstRect.x = j*TILE_W;
                SDL_RenderCopy( gRenderer, gTexture, &gTileClips[ map[i][j] ], &dstRect );
            }
        }

        // render to screen
        SDL_RenderPresent( gRenderer );
    }

    SDL_Quit();
    printf( "reached end\n" );
    return 0;
}
