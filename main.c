#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

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

bool initSDL()
{
    // initializes SDL and creates a global window and renderer
    int success = true;
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

SDL_Texture* loadTexture( SDL_Renderer* renderer, char* path )
{
    // load a texture from file into a renderer. Return NULL on failure
    SDL_Texture* loadedTexture = NULL;
    SDL_Surface* surf = IMG_Load( path );
    if ( surf == NULL ) {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path, SDL_GetError() );
        return NULL;
    }
    else {
        loadedTexture = SDL_CreateTextureFromSurface( renderer, surf );
        if ( loadedTexture == NULL ) {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
            return NULL;
        }
        SDL_FreeSurface( surf );
    }

    return loadedTexture;
}

SDL_Texture* loadTextTexture( char* textureText, SDL_Color textColor )
{
    // create a texture from a text using a global font
    SDL_Texture* loadedTexture = NULL;
    SDL_Surface* surf = TTF_RenderText_Solid( gFont, textureText, textColor );
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


struct Camera {
    SDL_Rect box;
    float x, y;
    float velX, velY;
		float speed;
    bool zoomedThisTick;
    float zoom;
};
typedef struct Camera Camera;

Camera createCamera( int w, int h )
{
	Camera cam;
  cam.box.x = 0;
  cam.box.y = 0;
  cam.box.w = w;
  cam.box.h = h;
  cam.x = 0;
  cam.y = 0;
  cam.velX = 0;
  cam.velY = 0;
	cam.speed = 8.f;
	cam.zoomedThisTick = false;
	cam.zoom = 0.5f;
	return cam;
}

void moveCam(Camera* cam)
{
    cam->x += cam->velX;
    cam->y += cam->velY;
    cam->box.x = (int) cam->x;
    cam->box.y = (int) cam->y;
};

float getCamZoom(Camera* cam) { return cam->zoom; };

SDL_Rect getCamRect(Camera* cam) { return cam->box; };

void handleCamEvent( SDL_Event* e, Camera* cam )
{
    cam->zoomedThisTick = false;
    // if an arrow key is pressed
    if ( e->type == SDL_KEYDOWN && e->key.repeat == 0 ) {
        switch ( e->key.keysym.sym )
				{
						case SDLK_UP:
                cam->velY = -cam->speed;
                break;
            case SDLK_DOWN:
                cam->velY = cam->speed;
                break;
            case SDLK_RIGHT:
                cam->velX = cam->speed;
                break;
            case SDLK_LEFT:
                cam->velX = -cam->speed;
                break;
        }
    }
    // if an arrow key is released
    else if ( e->type == SDL_KEYUP && e->key.repeat == 0 ) {
        if ( e->key.keysym.sym == SDLK_UP || e->key.keysym.sym == SDLK_DOWN ) {
            cam->velY = 0;
        }
        else if ( e->key.keysym.sym == SDLK_LEFT || e->key.keysym.sym == SDLK_RIGHT ) {
            cam->velX = 0;
        }
    }
    // if mouse scroll
    if ( e->wheel.y == 1 ) { // scroll up
        cam->zoom += 0.025;
        if ( cam->zoom > 1.0 ) {
            cam->zoom = 1.0;
        }
        cam->zoomedThisTick = true;
        printf("zoom: %f\n", cam->zoom);
    }
    else if ( e->wheel.y == -1 ) { // scroll down
        cam->zoom -= 0.025;
        if ( cam->zoom <= 0.0 ) {
            cam->zoom = 0.025;
        }
        cam->zoomedThisTick = true;
        printf("zoom: %f\n", cam->zoom);
    }
};

struct Tile {
    int type;
    SDL_Rect rect;
};
typedef struct Tile Tile;

Tile createTile(int x, int y, int type)
{
		Tile tile;
    tile.rect.x = x;
    tile.rect.y = y;
    tile.rect.w = TILE_W;
    tile.rect.h = TILE_H;
    tile.type = type;
		return tile;
}

void renderTile( SDL_Rect* camRect, Tile* tile )
{
    if ( checkCollision( tile->rect, *camRect ) ) {
        SDL_Rect dstRect = { tile->rect.x - camRect->x,
                             tile->rect.y - camRect->y,
                             tile->rect.w,
                             tile->rect.h
                           };
        SDL_RenderCopy( gRenderer, gTileTexture, &gTileClips[ tile->type ], &dstRect );
    }
		return;
}

void applyTileZoom( int row, int col, float zoomLevel, Tile* tile )
{
    tile->rect.w = TILE_W * zoomLevel;
    tile->rect.h = TILE_H * zoomLevel;
    tile->rect.x = tile->rect.w * col;
    tile->rect.y = tile->rect.h * row;
		return;
}

int getWidth(Tile* tile) { return tile->rect.w; }

int getType(Tile* tile) { return tile->type; }

void loadChunk( Tile* chunk, int length )
{
    for ( int i=0; i<length; i++ ) {
        int y = i * TILE_W;
        for ( int j=0; j<length; j++ ) {
            int x = j * TILE_H;
            chunk[i*length + j] = createTile(x, y, rand()%2);
        }
    }
}

void zoomChunk(Tile* chunk, int length, float zoomLevel)
{
		return;
    for ( int i=0; i<length; i++ ) {
        for ( int j=0; j<length; j++ ) {
						Tile tile = chunk[i*length + j];
						applyTileZoom(i, j, zoomLevel, &tile);
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
    if ( !initSDL() ) {
        printf( "Error initializing SDL!\n" );
        return 3;
    }

    if ( !loadMedia() ) {
        printf("Failed to load Media!\n" );
        return 3;
    }

		int chunk_size = 64;

    gTileTexture = loadTexture( gRenderer, "textures/tilesSpritesheet.png" );
    Tile* chunk;
		chunk = malloc(sizeof(Tile) * chunk_size * chunk_size);

    setClips();
    printf("Clips set.\n");
    loadChunk(chunk, chunk_size);
    printf("Chunk loaded.\n");

    Camera camera = createCamera(SCREEN_WIDTH,SCREEN_HEIGHT);
    SDL_Event e;
    char FPSText[16];
    SDL_Color FPStextColor = { 255, 255, 0, 255 };
    int frameNumber = 0;

    const Uint32 FPSStart = SDL_GetTicks();

    // Main loop
    bool quit = false;
    while ( !quit ) {
        int FRAME_BEGIN_MS = SDL_GetTicks();

        // Handle event queue
        while ( SDL_PollEvent( &e ) != 0 ) {
            if ( e.type == SDL_QUIT ) { quit = true; }
            if ( e.key.keysym.sym == SDLK_ESCAPE ) { quit = true; }
            handleCamEvent(&e, &camera);
        }


        // Set positions/game state
        float FPSTime = frameNumber / ((SDL_GetTicks() - FPSStart) / 1000.f);
        if ( FPSTime > 2000000.f ) {
            FPSTime = 0.f;
        }
        //FPSText << "FPS: " << FPSTime;
				snprintf(FPSText, sizeof(FPSText), "FPS: %.1f", FPSTime);
        gFPSTexture = loadTextTexture( FPSText, FPStextColor );
				moveCam(&camera);
        if ( camera.zoomedThisTick ) {
            zoomChunk( chunk, 64, getCamZoom(&camera)*2.5 );
        }


        // Clear the renderer
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_RenderClear( gRenderer );


        // Draw objects to renderer
        for ( int i=0; i < 64*64; i++ ) {
            //chunk[i].render( getCamRect(&camera) );
						SDL_Rect cam_rect = getCamRect(&camera);
            renderTile(&cam_rect, &chunk[i]);
        }
        //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0, 0, 0xFF );
        //SDL_RenderDrawRect( gRenderer, &camera.rect() ); // draw cam in red
        SDL_Rect FPSTextPos = { 0, 0, 200, 50 };
        SDL_RenderCopy( gRenderer, gFPSTexture, NULL, &FPSTextPos );


        // Render to screen
        SDL_RenderPresent( gRenderer );
        frameNumber++;

        int FRAME_END_MS = SDL_GetTicks();
        int FRAME_ELAPSED_TIME = FRAME_END_MS - FRAME_BEGIN_MS;
        if ( FRAME_ELAPSED_TIME < 1000.f/60.f ) {
            SDL_Delay( 1000.f/60.f - FRAME_ELAPSED_TIME );
        }
    }

    SDL_Quit();
		free(chunk);
    printf( "SDL quit successfully.\n" );
    return 0;
}
