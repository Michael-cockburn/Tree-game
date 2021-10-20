#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include <time.h>

#define winWidth 1000
#define winHeight 1000

#define mapWidth 3000
#define mapHeight 3000

#define tileSize 100

#define zeroX mapWidth/2
#define zeroY mapHeight/2
#define zeroTilei mapWidth/tileSize/2
#define zeroTilej mapHeight/tileSize/2

#define scrollSpeed 50

#define resDensity 50 //smaller is more resources

bool buildCursor = false;
int mouseX = NULL;
int mouseY = NULL;
int absMouseX = NULL;
int absMouseY = NULL;
int cursorTexture = NULL;

//tile struct
typedef struct
{
    int x, y;
    SDL_Texture *texture;
    int texTag; //NULL: 0, heart: 1, niche: 2, branch: 3, cradle: 4, tower: 5,
    float supply;
    int drain, supplyLim;
    bool visible, assigned;
} Tile;

//tile array
Tile buildTiles[mapWidth/tileSize][mapHeight/tileSize] = {NULL};

//supply array
Tile *supplyTiles[(mapWidth/tileSize)/resDensity*2] = {NULL};

//drain array
Tile *drainTiles[(mapWidth*mapHeight*2)/tileSize] = {NULL};

//function to check branch adjacency (prevents placing blocks of 2x2)
bool adjCheck(int i, int j)
{
    if(buildTiles[i-1][j-1].texture &&
       buildTiles[i-1][j].texture &&
       buildTiles[i][j-1].texture
       )
    {
        return false;
        printf("you can't make adjacent branches");
    }

    else if(buildTiles[i-1][j].texture &&
            buildTiles[i-1][j+1].texture &&
            buildTiles[i][j+1].texture
            )
    {
        return false;
        printf("you can't make adjacent branches");
    }

    else if(buildTiles[i+1][j-1].texture &&
            buildTiles[i+1][j].texture &&
            buildTiles[i+1][j+1].texture
            )
    {
        return false;
        printf("you can't make adjacent branches");
    }

    else if(buildTiles[i+1][j].texture &&
            buildTiles[i+1][j+1].texture &&
            buildTiles[i][j+1].texture
            )
    {
        return false;
        printf("you can't make adjacent branches");
    }

    else
    {
        return true;
    }
}

//function to check if there are any adjacent tiles
bool supAdjCheck(int k, int l)
{
    for(int i = -1; i < 1; i++)
    {
        for(int j = -1; j < 1; j++)
        {
            if(buildTiles[buildTiles[k][l].x /tileSize + i][buildTiles[k][l].y /tileSize + j].texture)
            {
                return false;
            }
        }
    }
    return true;
}

bool branchCheck(int i, int j)
{
    if(buildTiles[i-1][j].supply > 0 ||
       buildTiles[i][j-1].supply > 0 ||
       buildTiles[i+1][j].supply > 0 ||
       buildTiles[i][j+1].supply > 0
       )
    {
        return true;
    }
    else
    {
        return false;
    }

}

void buildCalc() //populates drain array with first round of supply adjacent buildings and calculates their supply
{

    printf("************start build calc ************\n");
    //for(int k = 0; k < sizeof(supplyTiles)/sizeof(supplyTiles[0]); k++)
    int k = 0;

    while(supplyTiles[k])
    {
        Tile *supplyArr[4] = {NULL};
        int n = 0;

        //loop through adjacent tiles to find any tiles with a non-supply building
        //below
        if(buildTiles[(supplyTiles[k]->x +mapWidth/2) /tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1].texture &&
           (buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1].texTag == 3 ||
           buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1].texTag == 5)
           )
        {
            int m = 0;
            while(drainTiles[m])
            {
                m++;
            }
            if(!buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1].assigned)
            {
                drainTiles[m] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1]; //add adjacent tile to next free element in drain array
                buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1].assigned = true;
            }
            supplyArr[n] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize +1]; //add adjacent tile to next free element in temp supply array
            n++; //number of elements in temp supply array
        }
        //left
        if(buildTiles[(supplyTiles[k]->x +mapWidth/2) /tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texture &&
           (buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texTag == 3 ||
           buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texTag == 5)
           )
        {
            int m = 0;
            while(drainTiles[m])
            {
                m++;
            }
            if(!buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize].assigned)
            {
                drainTiles[m] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize]; //add adjacent tile to next free element in drain array
                buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize].assigned = true;
            }
            supplyArr[n] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize -1][(supplyTiles[k]->y +mapHeight/2)/tileSize]; //add adjacent tile to next free element in temp supply array
            n++; //number of elements in temp supply array

        }
        //above
        if(buildTiles[(supplyTiles[k]->x +mapWidth/2) /tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1].texture &&
           (buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1].texTag == 3 ||
           buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1].texTag == 5)
           )
        {
            int m = 0;
            while(drainTiles[m])
            {
                m++;
            }
            if(!buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1].assigned)
            {
                drainTiles[m] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1]; //add adjacent tile to next free element in drain array
                buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1].assigned = true;
            }
            supplyArr[n] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize][(supplyTiles[k]->y +mapHeight/2)/tileSize -1]; //add adjacent tile to next free element in temp supply array
            n++; //number of elements in temp supply array

        }
        //right
        if(buildTiles[(supplyTiles[k]->x +mapWidth/2) /tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texture &&
           (buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texTag == 3 ||
           buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize].texTag == 5)
           )
        {
            int m = 0;
            while(drainTiles[m])
            {
                m++;
            }
            if(!buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize].assigned)
            {
                drainTiles[m] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize]; //add adjacent tile to next free element in drain array
                buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize].assigned = true;
            }
            supplyArr[n] = &buildTiles[(supplyTiles[k]->x +mapWidth/2)/tileSize +1][(supplyTiles[k]->y +mapHeight/2)/tileSize]; //add adjacent tile to next free element in temp supply array
            n++; //number of elements in temp supply array

        }

        //loop through temp supply array to check if supply to be added is larger than supply limit
        for(int o = 0; o < n; o++)
        {
            if((supplyTiles[k]->supply /  n) >= supplyArr[o]->supplyLim)
            {
                supplyArr[o]->supply =  supplyArr[o]->supplyLim; //prevent supply from exceeding supply limit
            }
            else
            {
                supplyArr[o]->supply = supplyTiles[k]->supply / n; //add supply as normal
            }
        }
        k++;
    }
    int m = 0;
    while(drainTiles[m])
    {
        m++;
    }


    printf("drain array size: %d\n", m);
    printf("************end build calc ************\n");
}

void supplyCalc() //performs calculation on each tile in drain array, adds adjacent tiles to drain array
{
    printf("************start supply calc ************\n");
    int i = 0;
    while(drainTiles[i]) //what about adjacent tiles with supply same as this one? *****************
    {
        printf("drain loop: %d\n", i);
        /*if(drainTiles[i]->supply > 2)
        {*/
            int p = 0;
            int j = 0;
            Tile *supplyArr[4] = {NULL};

            if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1].texTag != 2 &&
               buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1].texTag != 0)
            {
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1].supply == 0 /*< drainTiles[i]->supply -1*/)
                {
                    supplyArr[p] = &buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1];       //Tile below has less supply than this one
                    printf("tile below supply: %f\n", buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1].supply);
                    p++;
                }
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize +1].supply > 0)
                {
                    j++;
                }
            }
            if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize].texTag != 2 &&
               buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize].texTag != 0)
            {
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply == 0 /*< drainTiles[i]->supply -1*/)
                {
                    supplyArr[p] = &buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize];       //Right
                    printf("tile right supply: %f\n", buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply);
                    p++;
                }
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize+1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply > 0)
                {
                    j++;
                }
            }
            if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1].texTag != 2 &&
               buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1].texTag != 0)
            {
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1].supply == 0 /*< drainTiles[i]->supply -1*/)
                {
                    supplyArr[p] = &buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1];       //Above
                    printf("tile above supply: %f\n", buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1].supply);
                    p++;
                }
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize][(drainTiles[i]->y +mapHeight/2)/tileSize -1].supply > 0)
                {
                    j++;
                }
            }
            if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize].texTag != 2 &&
               buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize].texTag != 0)
            {
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply == 0 /*< drainTiles[i]->supply -1*/)
                {
                    supplyArr[p] = &buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize];       //Left
                    printf("tile left supply: %f\n", buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply);
                    p++;
                }
                if(buildTiles[(drainTiles[i]->x +mapWidth/2)/tileSize-1][(drainTiles[i]->y +mapHeight/2)/tileSize].supply > 0)
                {
                    j++;
                }
            }
            if(j == 0)
            {
                printf("drain tile old supply: %f", drainTiles[i]->supply);
                drainTiles[i]->supply = drainTiles[i]->supply - 1;
                printf("drain tile new supply: %f", drainTiles[i]->supply);
                if((drainTiles[i]->supply = 0))
                {
                    drainTiles[i]->texture = NULL;
                    drainTiles[i]->texTag = 0;
                    drainTiles[i] = NULL;
                    int k = i;
                    while(drainTiles[k+1])
                    {
                        drainTiles[k] = drainTiles[k+1];
                    }
                    drainTiles[k+1] = NULL;
                    printf("drain array rearanged");
                }
            }
            if(p != 0)
            {
                for(int l = 0; l < p; l++)
                {
                    supplyArr[l]->supply = drainTiles[i]->supply / p;
                    supplyArr[l]->supply = supplyArr[l]->supply - 1;
                    int m = 0;
                    while(drainTiles[m])
                    {
                        m++;
                    }
                    drainTiles[m] = supplyArr[l];
                }
            }
        //}
        i++;
    }
    int m = 0;
    while(drainTiles[m])
    {
        m++;
    }
    printf("drain array size: %d\n", m);
    printf("************end supply calc ************\n");
}

void processEvents(bool *running_p,
                   bool *startScreen_p,
                   SDL_Rect *startRectangle,
                   int *buttonHover_p,
                   int buildCount,
                   bool *firstEvent_p,
                   int *mapX_p,
                   int *mapY_p,
                   SDL_Texture *buildTextures[0]
                   )
{
    //initialise variables only needed in this function

    SDL_Event event;

    //event loop
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    {
                        *running_p = false;
                        printf("Escape------\n");
                        break;
                    }
                    case SDLK_RETURN:
                    {
                        break;
                    }
                }
                break;
            }

            case SDL_QUIT:
            {
                *running_p = false;
                printf("Quit------\n");
                break;
            }

            case SDL_MOUSEMOTION:
            {
               //get cursor position
               SDL_GetMouseState(&mouseX, &mouseY);
               //printf("Mouse x: %d\n", mouseX);
               //printf("Mouse y: %d\n", mouseY);
               absMouseX = zeroX + *mapX_p + mouseX - winWidth/2;
               absMouseY = zeroY + *mapY_p + mouseY - winHeight/2;
               //check if hovering over start button
               if(*startScreen_p)
               {
                   if(mouseX > (startRectangle->x) &&
                        mouseX < (startRectangle->x+startRectangle->w) &&
                        mouseY > (startRectangle->y) &&
                        mouseY < (startRectangle->y+startRectangle->h)
                        )
                   {
                       *buttonHover_p = 1;
                   }
                   else
                   {
                       *buttonHover_p = 0;
                   }
               }

               if(!*startScreen_p)
               {
                   //check if hovering over build menu by looping over each rectangle
                   for(int i = 0; i < buildCount; ++i)
                   {
                       if(mouseX > (winWidth/2-(buildCount*112+(buildCount-1)*6)/2+i*118) &&
                            mouseX < (winWidth/2-(buildCount*112+(buildCount-1)*6)/2+i*118+112) &&
                            mouseY < (winHeight-140+112) &&
                            mouseY > (winHeight-140)
                          )
                      {
                          *buttonHover_p = i + 2; //hovering over rectangle, selects from build array
                          break;
                      }
                      else
                      {
                          *buttonHover_p = 0; //not hovering or just stopped hovering
                      }
                   }



               }
               break;
            }

            case SDL_MOUSEBUTTONDOWN:
            {
                if(event.button.button == SDL_BUTTON_LEFT)
                {
                    if(*startScreen_p && *buttonHover_p == 1)
                    {
                        *startScreen_p = false; //clicked on start button
                    }
                    else if(!*startScreen_p && *buttonHover_p > 1 && *buttonHover_p <= buildCount + 2)
                    {
                        buildCursor = true;
                        cursorTexture = *buttonHover_p - 2; //clicked on build menu
                        break;
                    }
                    else if(buildCursor && *buttonHover_p == 0)
                    {
                        switch(cursorTexture)
                               {
                               case 0: //attempted to place branch
                                    if(!(buildTiles[absMouseX / tileSize][absMouseY / tileSize].texture) &&
                                       adjCheck(absMouseX / tileSize, absMouseY / tileSize) &&
                                       branchCheck(absMouseX / tileSize, absMouseY / tileSize)
                                       )
                                    {
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texture = buildTextures[0]; //assigned branch texture to tile
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].drain = 1;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].supplyLim = 10;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texTag = 3;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].supply = 0;
                                        buildCalc();
                                        //int calcloop = 0;
                                        //while(calcloop < 5)
                                        //{
                                            supplyCalc(); //calculate supply
                                            //calcloop++;
                                        //}
                                    }
                                    break;

                               case 1: //attempted to place cradle
                                    if(buildTiles[absMouseX / tileSize][absMouseY / tileSize].texTag == 2 && //must be on niche
                                       branchCheck(absMouseX / tileSize, absMouseY / tileSize)
                                       )
                                    {
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texture = buildTextures[1]; //assigned cradle texture to tile
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].supply = 10; //assigned cradle supply to tile
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texTag = 4;

                                        //add new cradle to next empty element in supply array
                                        int i = 0;
                                        while(supplyTiles[i])
                                        {
                                            i++;
                                        }
                                        supplyTiles[i] = &buildTiles[absMouseX / tileSize][absMouseY / tileSize];
                                        buildCursor = false;
                                    }
                                    break;

                               case 2: //attempted to place tower
                                    if(!(buildTiles[absMouseX / tileSize][absMouseY / tileSize].texture) &&
                                       adjCheck(absMouseX / tileSize, absMouseY / tileSize) &&
                                       branchCheck(absMouseX / tileSize, absMouseY / tileSize)
                                       )
                                    {
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texture = buildTextures[2]; //assigned tower texture to tile
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].drain = 3;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].supplyLim = 15;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].texTag = 5;
                                        buildTiles[absMouseX / tileSize][absMouseY / tileSize].supply = 0;
                                        buildCursor = false;
                                    }
                                    break;
                               }
                    }
                }
                else if(event.button.button == SDL_BUTTON_RIGHT)
                {
                    buildCursor = false; //cleared build cursor
                }
                break;
            }
        }
    }//end poll events

    if(!*startScreen_p)
    {
        //check if hovering at edges of map for scrolling (but not over build menu)
        if((mouseX < 50) && !(*mapX_p < -mapWidth/2 +20 + winWidth/2))
        {
           *mapX_p = *mapX_p - scrollSpeed;
           SDL_Delay(5);
        }

        if((mouseX > winWidth - 50) && !(*mapX_p > mapWidth/2 -10 - winWidth/2))
        {
           *mapX_p = *mapX_p + scrollSpeed;
           SDL_Delay(5);
        }
        if((mouseY < 50) && !(*mapY_p < -mapHeight/2 +10 + winHeight/2))
        {
           *mapY_p = *mapY_p - scrollSpeed;
           SDL_Delay(5);
        }
        if((mouseY > winHeight - 50) && (*mapY_p < mapHeight/2 -10 - winHeight/2) &&
              !(mouseX > (winWidth/2-(buildCount*(100+2*6)+(buildCount-1)*6)/2) &&
                mouseX < (winWidth/2+(buildCount*(100+2*6)+(buildCount-1)*6)/2) &&
                mouseY < (winHeight-140+112) &&
                mouseY > (winHeight-140))
               )
        {
           *mapY_p = *mapY_p + scrollSpeed;
           SDL_Delay(5);
        }

    }
    if(*firstEvent_p)
    {
        printf("completed first event loop\n");
        *firstEvent_p = false;
    }

}

void gameRender(SDL_Renderer *renderer,
                bool startScreen,
                int startX,
                int startY,
                int startW,
                int startH,
                int buttonHover,
                SDL_Texture *spawnTextures[0],
                SDL_Texture *buildTextures[0],
                int buildCount,
                SDL_Rect *startRectangle,
                SDL_Texture *startTexture,
                bool *firstRender_p,
                int mapX,
                int mapY
                )
{
    //start screen
    if(startScreen)
    {
        //Background colour white

        SDL_SetRenderDrawColor(renderer,
                               255, //R
                               255, //G
                               255, //B
                               255  //a
                               );

        SDL_RenderClear(renderer);

        //add fading green borders to top and bottom third of screen by decrementing alpha value for each line drawn

        for(int i = winHeight/3-winHeight%3; i>0; --i)
        {
            SDL_SetRenderDrawColor(renderer,
                                   67,
                                   240,
                                   67,
                                   (float) 255/(winHeight/3)*(winHeight/3-i)
                                   ); //green with dynamic alpha

            SDL_RenderDrawLine(renderer,
                               0,           //x1
                               i,           //y1
                               winWidth,    //x2
                               i            //y2
                           ); //top third

            SDL_RenderDrawLine(renderer,
                               0,               //x1
                               winHeight-i,     //y1
                               winWidth,        //x2
                               winHeight-i      //y2
                               ); //bottom third

        }

        //add start button

        SDL_SetRenderDrawColor(renderer, 0,0,0,255); //black

        //build start dimensions
        startX = winWidth/2-startW/2;
        startY = winHeight/2-startH/2;

        SDL_Rect txtRect = {startX,
                            startY,
                            startW,
                            startH
                            };

        SDL_RenderCopy(renderer, startTexture, NULL, &txtRect); //write start on screen

        //draw rectangle around start

        startRectangle->x = startX-10;
        startRectangle->y = startY;
        startRectangle->w = startW+25;
        startRectangle->h = startH;


        SDL_RenderDrawRect(renderer, startRectangle);

        //highlight start button if mouse hover
        if(buttonHover == 1)
        {
            for(int i = 0; (i < 5); ++i)
            {
                SDL_Rect highlightStartRectangle = {startRectangle->x-i,
                                                    startRectangle->y-i,
                                                    startRectangle->w+2*i,
                                                    startRectangle->h+2*i
                                                    };
                SDL_RenderDrawRect(renderer, &highlightStartRectangle);
            }
        }
    }

    //game screen
    else
    {
        //white background
        SDL_SetRenderDrawColor(renderer,
                       255,
                       255,
                       255,
                       255
                       );
        SDL_RenderClear(renderer);

        //tile grid
        SDL_SetRenderDrawColor(renderer, 10,10,10,255); //light grey

        for(int  i = 0; i < winWidth/tileSize+1; i++)
        {
            SDL_RenderDrawLine(renderer,
                               tileSize * i - mapX % 100,
                               0,
                               tileSize * i - mapX % 100,
                               winHeight
                               );
        }

        for(int  i = 0; i < winHeight/tileSize+1; i++)
        {
            SDL_RenderDrawLine(renderer,
                               0,
                               tileSize * i - mapY % 100,
                               winWidth,
                               tileSize * i - mapY % 100
                               );
        }

        //tiles
        char supply[3] = {NULL}; //for int-char conversion
        SDL_Surface *supSur = {NULL};
        SDL_Texture *supTex = {NULL};
        int supTexW = NULL;
        int supTexH = NULL;
        SDL_Rect supRect = {NULL};

        for(int i = 0; i < mapWidth/tileSize; i++)
        {
            for(int j = 0; j < mapHeight/tileSize; j++)
            {
                SDL_Rect buildTileRect = {buildTiles[i][j].x - mapX + winWidth/2, buildTiles[i][j].y - mapY + winHeight/2, tileSize, tileSize};
                SDL_RenderCopy(renderer, buildTiles[i][j].texture, NULL, &buildTileRect);

                //supply indicators
                if(buildTiles[i][j].supply)
                {
                    SDL_Color supCol = {0, 0, 0, 255};
                    if(buildTiles[i][j].texTag == 1 || buildTiles[i][j].texTag == 2)
                    {
                        supCol.r = 0;
                        supCol.g = 152;
                        supCol.b = 0;
                    }

                    sprintf(supply, "%f", buildTiles[i][j].supply);
                    TTF_Font *supFont = TTF_OpenFont("GIL.TTF", tileSize/7);
                    supSur = TTF_RenderUTF8_Blended(supFont, supply, supCol);
                    supTex = SDL_CreateTextureFromSurface(renderer, supSur);
                    SDL_QueryTexture(supTex, NULL, NULL, &supTexW, &supTexH);
                    supRect.x = buildTiles[i][j].x - mapX + winWidth/2;
                    supRect.y = buildTiles[i][j].y - mapY + winHeight/2 + tileSize - supTexH;
                    supRect.w = supTexW;
                    supRect.h = supTexH;
                    SDL_RenderCopy(renderer, supTex, NULL, &supRect);

                }

            }
        }

        SDL_FreeSurface(supSur);
        SDL_DestroyTexture(supTex);

        //build menu

        for(int i = 0; i < buildCount; i++)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); //white background
            SDL_Rect buildMenuTile = {winWidth/2-(buildCount*112+(buildCount-1)*6)/2+i*118+6,
                                        winHeight -140+6,
                                        100,
                                        100
                                        };
            SDL_RenderFillRect(renderer, &buildMenuTile);

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //black borders

            for(int j = 0; j < 6; ++j)
            {
                //build menu borders NOTE: if you change this, change the one below too.
                //will also need to look at mouse hover check in processEvents
                SDL_Rect buildBorderRect = {
                                        winWidth/2-(buildCount*112+(buildCount-1)*6)/2+i*118+j,
                                        winHeight-140+j,
                                        112-2*j,
                                        112-2*j
                                        };
                SDL_RenderDrawRect(renderer, &buildBorderRect);
            }

            //build menu textures
            SDL_RenderCopy(renderer, buildTextures[i], NULL, &buildMenuTile);
        }
        //printf("button hover: %d\n", buttonHover);
        //build menu highlight
        if(buttonHover > 1)
        {
            SDL_SetRenderDrawColor(renderer, 67, 240, 67, 255); //green

            for(int i = 0; i < 6; ++i)
            {
                //build menu border highlight NOTE: if you change this, change the one above too.
                //also if you add any more buttons to button hover, change the buttonHover-2
                //will also need to look at mouse hover check in processEvents
                SDL_Rect buildBorderRect = {winWidth/2-(buildCount*112+(buildCount-1)*6)/2+(buttonHover-2)*118+i,
                                            winHeight-140+i,
                                            112-2*i,
                                            112-2*i
                                            };
                SDL_RenderDrawRect(renderer, &buildBorderRect);

            }
        }

        //hover building shadow underneath cursor
        //printf("build cursor %d\n", buildCursor);
        if(buildCursor)
        {
            SDL_Rect buildTextureRect = {mouseX-50, mouseY-50, 100, 100};
            SDL_RenderCopy(renderer, buildTextures[cursorTexture], NULL, &buildTextureRect);
        }

        //window border
        for(int i = 0; i < 5; i++)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_Rect windowBorder = {i, i, winWidth - 2*i, winHeight - 2*i};
            SDL_RenderDrawRect(renderer, &windowBorder);
        }
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect windowBorder = {0, 0, winWidth, winHeight};
        SDL_RenderDrawRect(renderer, &windowBorder);

        //printf("mapX: %d\n", mapX);
        //printf("mapY: %d\n", mapY);
    }//end game screen



    SDL_RenderPresent(renderer);

    if(*firstRender_p)
    {
        printf("completed first Render loop\n");
        *firstRender_p = false;
    }

}

int main(int argc, char *argv[])
{
    //start up
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    //initialise window
    SDL_Window *window = SDL_CreateWindow("Branchy",
                                          SDL_WINDOWPOS_CENTERED,   //x pos
                                          SDL_WINDOWPOS_CENTERED,   //y pos
                                          winHeight,                //width
                                          winWidth,                 //height
                                          0                         //flags
                                          );

    //initialise renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                -1,
                                                0 //SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
                                                );

    //blend alpha
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    //load textures
    //initialise surface
    SDL_Surface *spriteSurface = 0;

    //building sprites
    char spriteBuildFiles[][10] =
    {
        "Branch1",
        "Cradle1",
        "Tower1"
    };

    //initialise texture variables because I can't do it dynamically. maybe could with macros if I could be bothered.
    SDL_Texture *branch1Texture = NULL;
    SDL_Texture *cradle1Texture = NULL;
    SDL_Texture *tower1Texture = NULL;

    SDL_Texture *buildTextures[] =
    {
        branch1Texture,
        cradle1Texture,
        tower1Texture
    };

    int buildCount = sizeof(spriteBuildFiles)/sizeof(spriteBuildFiles[0]);

    //spawn sprites
    char spriteSpawnFiles[][10] =
    {
        "Heart1",
        "Niche"
    };

    //initialise spawn texture variables
    SDL_Texture *heart1Texture = NULL;
    SDL_Texture *niche1Texture = NULL;

    SDL_Texture *spawnTextures[] =
    {
        heart1Texture,
        niche1Texture
    };

    int spawnCount = sizeof(spriteSpawnFiles)/sizeof(spriteSpawnFiles[0]);

    //build textures
    char fileName[15];
    for(int i = 0; i < buildCount; ++i)
    {
        //use temp char array to concatenate .png onto file names. clear the array after use ready for next loop.
        strcpy(fileName, spriteBuildFiles[i]);
        strcat(fileName, ".png");
        printf("fileName: %s\n", fileName);
        spriteSurface = IMG_Load(fileName);
        fileName[0] = 0;

        if(!spriteSurface)
        {
            printf("failed to load %s\n", spriteBuildFiles[i]);
            return 1;
        }

        buildTextures[i] = SDL_CreateTextureFromSurface(renderer, spriteSurface);
        SDL_FreeSurface(spriteSurface);
    }

    //spawn textures
    for(int i = 0; i < spawnCount; ++i)
    {
        //use temp char array to concatenate .png onto file names. clear the array after use ready for next loop.
        strcpy(fileName, spriteSpawnFiles[i]);
        strcat(fileName, ".png");
        printf("fileName: %s\n", fileName);
        spriteSurface = IMG_Load(fileName);
        fileName[0] = 0;
        if(!spriteSurface)
        {
            printf("failed to load %s\n", spriteSpawnFiles[i]);
            return 1;
        }

        spawnTextures[i] = SDL_CreateTextureFromSurface(renderer, spriteSurface);
        SDL_FreeSurface(spriteSurface);
    }

    //start font texture
    SDL_Color startColor = {0,0,0,255}; //black
    TTF_Font *startFont = TTF_OpenFont("GIL.TTF",100); //load font
    SDL_Surface *startSurface = TTF_RenderUTF8_Blended(startFont,"START",startColor); //surface
    SDL_Texture *startTexture = SDL_CreateTextureFromSurface(renderer, startSurface); //texture
    SDL_FreeSurface(startSurface); //clean surface
    int startX = NULL;
    int startY = NULL;
    int startW = NULL;
    int startH = NULL;
    //get size of resultant texture
    SDL_QueryTexture(startTexture,
                     NULL,
                     NULL,
                     &startW,
                     &startH
                     );

    //initialise variables to be passed between functions
    bool running = true;
    bool startScreen = true;
    SDL_Rect startRectangle;
    int buttonHover = 0; //0=no hover, 1=start button, 2 onwards = build menu
    bool firstEvent = true;
    bool firstRender = true;
    int mapX = tileSize/2;
    int mapY = tileSize/2;


    printf("Load successful\n");

    //tile array locations
    for(int i = 0; i < mapWidth/tileSize; i++)
    {
        for(int j = 0; j < mapHeight/tileSize; j++)
        {
            buildTiles[i][j].x = tileSize * i - mapWidth/2;
            buildTiles[i][j].y = tileSize * j - mapHeight/2;
            buildTiles[i][j].texture = NULL;
            buildTiles[i][j].texTag = 0;
            buildTiles[i][j].supply = 0;
            buildTiles[i][j].supplyLim = 0;
            buildTiles[i][j].drain = 0;
            buildTiles[i][j].visible = false;
            buildTiles[i][j].assigned = false;
        }
    }

    //spawn natural resources
    //add heart
    buildTiles[zeroTilei][zeroTilej].texture = spawnTextures[0];
    buildTiles[zeroTilei][zeroTilej].texTag = 1;
    buildTiles[zeroTilei][zeroTilej].supply = 20;
    printf("heart supply: %f\n", buildTiles[zeroTilei][zeroTilej].supply);
    printf("heart tag: %d\n", buildTiles[zeroTilei][zeroTilej].texTag);
    supplyTiles[0] = &buildTiles[zeroTilei][zeroTilej];
    printf("supplyTiles[0] -> supply: %f\n", supplyTiles[0]->supply);
    printf("supplyTiles[0] -> x: %d\n", supplyTiles[0]->x);
    printf("supplyTiles[0] -> y: %d\n", supplyTiles[0]->y);

    time_t t;
    srand((unsigned) time(&t));
    //add niches
    for(int i = 0; i < mapWidth/tileSize; i++)
    {
        for(int j = 0; j < mapHeight/tileSize; j++)
        {
            if(rand() % resDensity == 1 && supAdjCheck(i, j))
            {
                buildTiles[i][j].texture = spawnTextures[1];
                buildTiles[i][j].texTag = 2;
            }
        }
    }

    printf("test: %d\n", 243/100);
    printf("Spawn successful\n");

    printf("build tiles array size: %d\n", sizeof(buildTiles)/sizeof(buildTiles[0]) + sizeof(buildTiles)%sizeof(buildTiles[0]));

    //***********************main loop*****************************

    while(running)
    {
        processEvents(&running,
                      &startScreen,
                      &startRectangle,
                      &buttonHover,
                      buildCount,
                      &firstEvent,
                      &mapX,
                      &mapY,
                      buildTextures
                      );

        gameRender(renderer,
                   startScreen,
                   startX,
                   startY,
                   startW,
                   startH,
                   buttonHover,
                   spawnTextures,
                   buildTextures,
                   buildCount,
                   &startRectangle,
                   startTexture,
                   &firstRender,
                   mapX,
                   mapY
                   );

        SDL_Delay(50);
    }

    //cleanup
    TTF_Quit();

    //destroy textures
    for(int i = 0; i < buildCount; ++i)
    {
        SDL_DestroyTexture(buildTextures[i]);
    }

    for(int i = 0; i< spawnCount; ++i)
    {
        SDL_DestroyTexture(spawnTextures[i]);
    }

    IMG_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Quit successful\n");

    return 0;
}
