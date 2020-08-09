#include "raylib.h"
#include <math.h>
#define ARRSIZE(x) (sizeof(x)/sizeof(x[0]))

typedef enum {
    STPO_HIGHSCORE = 0
} StorageData;

float Rad2Deg(float value) {
    return (360/(2*PI))*value;
}


int KeyMoveRightDown() {
    int out = 0;
    if (IsGamepadAvailable(GAMEPAD_PLAYER1)) {
        if (IsGamepadButtonDown(GAMEPAD_PLAYER1,GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
            out = 1;
        }
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        out = 1;
    }
    return out;
}
int KeyMoveLeftDown() {
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        return 1;
    } else {return 0;}
}
int KeyMoveRightPressed() {
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
        return 1;
    } else {return 0;}
}
int KeyMoveLeftPressed() {
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
        return 1;
    } else {return 0;}
}
int KeyJumpPressed() {
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        return 1;
    } else {return 0;}
}




float VecDistance(Vector2 vecA, Vector2 vecB) {
    return sqrt((vecB.x-vecA.x)*(vecB.x-vecA.x)+(vecB.y-vecA.y)*(vecB.y-vecA.y));
}

/*size_t ArraySizeInt(int* arr) {
    return sizeof(arr)/sizeof(int);
}*/

float clamp(float value, float min, float max) {
    if (value<min) {value = min;}
    if (value>max) {value = max;}
    return value;
}

Vector2 ground(float x) {
    Vector2 vec = (Vector2){x,0};
    vec.y = 0.0005*pow(x-400,2)+400;
    return vec;
}

Vector2 ChickenCollisionVec(Vector2 pos, float rot, float shift) {
    return (Vector2){pos.x+cos(rot)*32,pos.y-shift};
}

int main() {
    const int WIDTH = 800;
    const int HEIGHT = 600;
    InitWindow(WIDTH,HEIGHT,"Save The Egg");
    InitAudioDevice();
    
    int devmode = 0;
    int tab = 2;
    
    Sound sfxPutsh = LoadSound("sfx/putsh.ogg");
    Sound sfxChew = LoadSound("sfx/chew.ogg");
    Sound sfxHighscore = LoadSound("sfx/highscore.ogg");
    Sound sfxChickenFast1 = LoadSound("sfx/chickenfast1.ogg");
    Sound sfxChickenFast2 = LoadSound("sfx/chickenfast2.ogg");
    const float CHICKEN_SCARE_DISTANCE = 32;
    Sound sfxChickenScared = LoadSound("sfx/chickenscared.ogg");
    const float CHICKEN_FAST_SPEED = 325;
    //SetSoundVolume(sfxChew,1);
    
    const float GRAVITY = 1000;
    const float JUMP_FORCE = -500;
    const float EGG_JUMP_MULTIPLIER = 1.3;
    const Vector2 SPAWN_POS = (Vector2){400,500};;
    Vector2 playerPos = (Vector2){400,500};
    Vector2 playerVel = (Vector2){0,0};
    const float GROUND_ACCEL = 500;
    const float AIR_ACCEL = 200;
    float playerAccel = 0;
    float playerRot = 0;
    Vector2 eggPos = (Vector2){0,0};
    int onFloor = 0;
    const float DEATH_ANGLE = 6*2*PI/32;
    const float DEATH_OFFSCREEN_DISTANCE = 100;
    const float STARTING_DIFF = 1;
    float difficulty = STARTING_DIFF;
    const float MAX_ROCK_DIFF = 30;
    const float MAX_DIFF = MAX_ROCK_DIFF;
    int rocksOnScreen = 0;
    const int MAX_ROCKS = 6;
    const float ROCK_SPEED_MULTIPLIER = 0.5;
    const float DIFF_SPEED = 0.1;
    Vector2 rocks[MAX_ROCKS];
    Vector2 rockVel = (Vector2){-15,30};
    int frameRock = 0;
    const Vector2 ROCK_SPAWN_X = (Vector2){WIDTH*0.25,WIDTH+WIDTH/2};
    const Vector2 ROCK_SPAWN_Y = (Vector2){-200,-32};
    const float ROCK_COLLISION_DISTANCE = 16;
    const float CHICKEN_COLLISION_DISTANCE = 16;
    const float CHICKEN_COLLISION_SHIFT = 16;
    const float CORN_COLLISION_DISTANCE = 32;
    const float CORN_SPAWN_X = 128;
    const float CORN_ROT_RANGE = 20;
    const float CORN_ROT_SPEED = 5;
    float cornRotValue = 0;
    float cornPopUpScale = 0;
    Vector2 cornPos = (Vector2){CORN_SPAWN_X,ground(CORN_SPAWN_X).y-CORN_COLLISION_DISTANCE};
    int score = 0;
    const int SCORE_PER_CORN = 100;
    const float MAX_CORN_HEIGHT = 150;
    int highscore = 0;
    int newHighscore = 0;
    Vector2 scoreTextPos = (Vector2){0,0};
    float scoreTextFade = 0;
    const float SCORE_TEXT_FADE_SPEED = 0.5;
    
    Font fFont = LoadFont("vassets/BitPap.ttf");
    Texture2D tAtlas = LoadTexture("vassets/atlas.png");
    Texture2D tBackground = LoadTexture("vassets/bg.png");
    Rectangle rChicken = (Rectangle){0,0,64,64};
    Vector2 oChicken = (Vector2){32,64};
    Rectangle rEgg = (Rectangle){64,0,64,64};
    Vector2 oEgg = (Vector2){32,32};
    Rectangle rBrokenEgg = (Rectangle){0,64,64,64};
    Vector2 oBrokenEgg = (Vector2){32,64};
    Rectangle rLeftFallenChicken = (Rectangle){64,64,64,64};
    Rectangle rRightFallenChicken = (Rectangle){128,64,64,64};
    Rectangle animRock[4] = {(Rectangle){128,0,64,64},(Rectangle){192,0,64,64},(Rectangle){256,0,64,64},(Rectangle){256+64,0,64,64}};
    Vector2 oRock = (Vector2){32,32};
    Rectangle rCorn = (Rectangle){192,64,64,64};
    Vector2 oCorn = (Vector2){32,32};
    Rectangle rVolcanoAF[8] = {
        (Rectangle){300*0,128,300,460},
        (Rectangle){300*1,128,300,460},
        (Rectangle){300*2,128,300,460},
        (Rectangle){300*3,128,300,460},
        (Rectangle){300*0,128+460,300,460},
        (Rectangle){300*1,128+460,300,460},
        (Rectangle){300*2,128+460,300,460},
        (Rectangle){300*3,128+460,300,460}
        };
    Rectangle rCornEatAF[4] = {(Rectangle){256-64,64,64,64},(Rectangle){256,64,64,64},(Rectangle){256+64,64,64,64},(Rectangle){256+128,64,64,64}};
    int frameCornEat = 0;
    int ateCorn = 0;
    Vector2 ateCornPos = (Vector2){0,0};
    Rectangle rGameOverBoard = (Rectangle){256+128,0,256+128,64};
    float gameOverBoardY = 0;
    const float GAME_OVER_BOARD_SPEED = 3;
    Rectangle rScroll = (Rectangle){1200,0,256,256};
    float scrollY = 1;
    const float SCROLL_SPEED = 1;
    Vector2 breakingRocks[MAX_ROCKS];
    int frameBreakingRocks[MAX_ROCKS];
    Rectangle rBreakingRockAF[3] = {(Rectangle){512-64,64,64,64},(Rectangle){512,64,64,64},(Rectangle){512+64,64,64,64}};
    
    int animVolcano[20] = {
        0,0,0,
        0,1,2,3,4,5,
        3,4,5,3,4,5,
        3,4,5,6,7};
    int frameVolcano = 0;
    float animationTimer = 0;
    float animationSpeed = 10;
    
    Music mUnprepared = LoadMusicStream("sfx/unprepared.ogg");
    //PlayMusicStream(mUnprepared);
    int muted = 0;
    SetMasterVolume(1);
    SetMusicVolume(mUnprepared,0.9);
    
    float gameOverDelayTimer = 0;
    const float GAME_OVER_DELAY = 0.5;
    highscore = LoadStorageValue(STPO_HIGHSCORE);
    
    float raylibFadeDelay = 1.5;
    float raylibFade = 1;
    Texture2D raylibLogo = LoadTexture("vassets/raylib_128x128.png");
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateMusicStream(mUnprepared);
        if (raylibFadeDelay>0) {
            raylibFadeDelay-=GetFrameTime();
        } else {
            if (raylibFade>0) raylibFade-=GetFrameTime()*2;
        }
        if (IsKeyPressed(KEY_M)) {
            if (muted) {
                SetMasterVolume(1);
                muted = 0;
            } else {SetMasterVolume(0);muted=1;}
        }
        // devmode toggler
        if (IsKeyPressed(KEY_PERIOD)) {
            if (devmode) {
                devmode = 0;
            } else {
                devmode = 1;
            }
        }
        // fullscreen toggler
        if (IsKeyPressed(KEY_F)) {ToggleFullscreen();}
        if (tab==2) {
            /*for (int i=0;i<MAX_ROCKS;i++) {
                rocks[i] = (Vector2){-100,-100};
            }*/
            difficulty = STARTING_DIFF;
            rocksOnScreen = 0;
            score = 0;
            playerPos.x = SPAWN_POS.x;
            playerPos.y = ground(playerPos.x).y;
            eggPos.x = playerPos.x + cos(playerRot)*32;
            eggPos.y = playerPos.y - sin(playerRot)*32;
            playerVel = (Vector2){0,0};
            playerRot = PI/2;
            cornPos.x = CORN_SPAWN_X;
            cornPos.y = ground(CORN_SPAWN_X).y-CORN_COLLISION_DISTANCE;
        }
        if (tab==2) {
            if (KeyMoveLeftPressed() || KeyMoveRightPressed() || KeyJumpPressed()) {tab = 0;PlayMusicStream(mUnprepared);}
        } else if (tab==0 /*GAME PLAY*/) {
            if (difficulty<MAX_DIFF) {difficulty += GetFrameTime()*DIFF_SPEED;}
            else {difficulty = MAX_DIFF;}
            playerVel.y += GRAVITY*GetFrameTime();
            playerPos.y += playerVel.y*GetFrameTime();
            if (playerPos.y > ground(playerPos.x).y) {
                playerPos.y = ground(playerPos.x).y;
            }
            if (playerPos.y > ground(playerPos.x).y-10) {
                onFloor = 1;
            } else {onFloor = 0;}
            if (KeyJumpPressed() && onFloor) {
                playerVel.y = JUMP_FORCE;
            }
            if (onFloor) {playerAccel=GROUND_ACCEL;} else {playerAccel=AIR_ACCEL;}
            if (KeyMoveRightDown()) {
                playerVel.x += playerAccel*GetFrameTime();
            } else if (KeyMoveLeftDown()) {
                playerVel.x -= playerAccel*GetFrameTime();
            } else {
                if (playerVel.x > 10) {
                    playerVel.x -= playerAccel*GetFrameTime();
                } else if (playerVel.x < -10) {
                    playerVel.x += playerAccel*GetFrameTime();
                } else {
                    playerVel.x = 0;
                }
            }
            playerPos.x += playerVel.x*GetFrameTime();
            playerRot = playerVel.x/400 + PI/2;
            eggPos.x = playerPos.x + cos(playerRot)*32;
            if (onFloor) {eggPos.y = playerPos.y - sin(playerRot)*32;}
            else {eggPos.y += playerVel.y*GetFrameTime()*EGG_JUMP_MULTIPLIER;}
            if (playerRot < PI/2-DEATH_ANGLE || playerRot > PI/2+DEATH_ANGLE) {
                if (onFloor) {tab = 1;gameOverDelayTimer = GAME_OVER_DELAY;StopMusicStream(mUnprepared);}
            }
            if (playerPos.x < -DEATH_OFFSCREEN_DISTANCE || playerPos.x > WIDTH+DEATH_OFFSCREEN_DISTANCE) {
                tab = 1;
                gameOverDelayTimer = GAME_OVER_DELAY;
                StopMusicStream(mUnprepared);
                PlaySound(sfxChickenScared);
            }
            if (rocksOnScreen<MAX_ROCKS) {
                rocks[rocksOnScreen] = (Vector2){GetRandomValue(ROCK_SPAWN_X.x,ROCK_SPAWN_X.y),GetRandomValue(ROCK_SPAWN_Y.x,ROCK_SPAWN_Y.y)};
                rocksOnScreen += 1;
            }
            if (CheckCollisionCircles(cornPos,CORN_COLLISION_DISTANCE,ChickenCollisionVec(playerPos,playerRot,CHICKEN_COLLISION_SHIFT),CHICKEN_COLLISION_DISTANCE)) {
                scoreTextFade = 1;
                scoreTextPos = cornPos;
                ateCornPos = cornPos;
                ateCorn = 1;
                if (cornPos.x<WIDTH/2) {
                    cornPos.x = WIDTH-CORN_SPAWN_X;
                } else {cornPos.x = CORN_SPAWN_X;}
                cornPos.y = ground(cornPos.x).y-CORN_COLLISION_DISTANCE-clamp(score*0.1,0,MAX_CORN_HEIGHT);
                score += SCORE_PER_CORN;
                if (score>highscore) {highscore=score;newHighscore=1;PlaySound(sfxHighscore);}
                else {newHighscore=0;}
                cornPopUpScale = 0;
                PlaySound(sfxChew);
            }
            if (scrollY>0) {scrollY-=GetFrameTime()*SCROLL_SPEED;}
            else {scrollY = 0;}
            if (playerVel.x>=CHICKEN_FAST_SPEED || playerVel.x <= -CHICKEN_FAST_SPEED) {
                if (!IsSoundPlaying(sfxChickenFast1) && !IsSoundPlaying(sfxChickenFast2)) {
                    if (GetRandomValue(0,1)) {
                        PlaySound(sfxChickenFast1);
                    } else {PlaySound(sfxChickenFast2);}
                }
            }
        } else if (tab==1 /*GAME OVER*/) {
            //playerPos.y = ground(playerPos.x).y;
            playerVel.y += GRAVITY*GetFrameTime();
            playerPos.y += playerVel.y*GetFrameTime();
            if (playerPos.y > ground(playerPos.x).y) {
                playerPos.y = ground(playerPos.x).y;
            }
            if (playerPos.y > ground(playerPos.x).y-20) {
                onFloor = 1;
            } else {onFloor = 0;}
            //eggPos.y = ground(eggPos.x).y;
            if (onFloor) {eggPos.y = playerPos.y - sin(playerRot)*32;}
            else {eggPos.y += playerVel.y*GetFrameTime()*EGG_JUMP_MULTIPLIER;}
            if (playerVel.x > 10) {
                playerVel.x -= playerAccel*GetFrameTime();
            } else if (playerVel.x < -10) {
                playerVel.x += playerAccel*GetFrameTime();
            } else {
                playerVel.x = 0;
            }
            playerPos.x += playerVel.x*GetFrameTime();
            eggPos.x = playerPos.x + cos(playerRot)*32;
            if (KeyJumpPressed() && gameOverDelayTimer<=0) {
                tab = 2;
            }
            if (gameOverBoardY<1) {gameOverBoardY+=GetFrameTime()*GAME_OVER_BOARD_SPEED;}
            else {gameOverBoardY = 1;}
            if (gameOverDelayTimer>0) {gameOverDelayTimer-=GetFrameTime();}
        }
        if (tab!=2) {
            for (int i=0;i<rocksOnScreen;i++) {
                if (rocks[i].y >= ground(rocks[i].x).y) {
                    breakingRocks[i] = rocks[i];
                    frameBreakingRocks[i] = 0;
                    rocks[i] = (Vector2){clamp(GetRandomValue(playerPos.x,playerPos.x+400),ROCK_SPAWN_Y.x,ROCK_SPAWN_X.y),GetRandomValue(ROCK_SPAWN_Y.x,ROCK_SPAWN_Y.y)};
                    //SetSoundPitch(sfxPutsh,GetRandomValue(80,120)/100);
                    PlaySound(sfxPutsh);
                } else {
                    rocks[i].x += rockVel.x*GetFrameTime()*(clamp(difficulty,0,MAX_ROCK_DIFF)*ROCK_SPEED_MULTIPLIER); 
                    rocks[i].y += rockVel.y*GetFrameTime()*(clamp(difficulty,0,MAX_ROCK_DIFF)*ROCK_SPEED_MULTIPLIER); 
                }
                if (tab==0) {
                    if (CheckCollisionCircles(ChickenCollisionVec(playerPos,playerRot,CHICKEN_COLLISION_SHIFT),CHICKEN_COLLISION_DISTANCE,rocks[i],ROCK_COLLISION_DISTANCE)) {
                        tab = 1;
                        gameOverDelayTimer = GAME_OVER_DELAY;
                        StopMusicStream(mUnprepared);
                    } else if (CheckCollisionCircles(ChickenCollisionVec(playerPos,playerRot,CHICKEN_COLLISION_SHIFT),CHICKEN_COLLISION_DISTANCE+CHICKEN_SCARE_DISTANCE,rocks[i],ROCK_COLLISION_DISTANCE)) {
                        if (!IsSoundPlaying(sfxChickenScared)) PlaySound(sfxChickenScared);
                    }
                }
            }
        }
        if (tab!=1) {
            if (gameOverBoardY>0) {gameOverBoardY-=GetFrameTime()*GAME_OVER_BOARD_SPEED;}
            else {gameOverBoardY = 0;}
        }
        if (tab!=0) {
            if (scrollY<1) {scrollY+=GetFrameTime()*SCROLL_SPEED;}
            else {scrollY = 1;}
        }
        // Do some animations
        animationTimer += GetFrameTime()*animationSpeed;
        if (animationTimer>=1) {
            if (frameVolcano+1<ARRSIZE(animVolcano)) {
                frameVolcano += 1;
            } else {frameVolcano=0;}
            if (frameRock+1<ARRSIZE(animRock)) {
                frameRock += 1;
            } else {frameRock=0;}
            if (frameCornEat+1<ARRSIZE(rCornEatAF) && ateCorn) {
                frameCornEat += 1;
            } else {frameCornEat=0;ateCorn=0;}
            for (int i=0;i<MAX_ROCKS;i++) {
                if (frameBreakingRocks[i]>=0) {
                    frameBreakingRocks[i] += 1;
                }
                if (frameBreakingRocks[i]>=3) {
                    frameBreakingRocks[i] = -1;
                }
            }
            animationTimer=0;
        }
        
        cornRotValue += GetFrameTime()*CORN_ROT_SPEED;
        if (cornRotValue>PI*2) {cornRotValue = 0;}
        if (cornPopUpScale<1) {cornPopUpScale+=3*GetFrameTime();}
        else {cornPopUpScale=1;}
        
        if (scoreTextFade>0) {
            scoreTextFade -= GetFrameTime()*SCORE_TEXT_FADE_SPEED;
        }
        
        if (devmode) {
            if (IsKeyPressed(KEY_ONE)) {
                score += 10;
            }
            if (IsKeyPressed(KEY_TWO)) {
                score += 100;
            }
            if (IsKeyPressed(KEY_THREE)) {
                score += 1000;
            }
            if (IsKeyPressed(KEY_FOUR)) {
                score += 10000;
            }
            if (IsKeyPressed(KEY_FIVE)) {
                score += 100000;
            }
        }
        
        // Draw stuff
        BeginDrawing();
            ClearBackground(RAYWHITE);
            
            
            DrawTexture(tBackground,0,0,WHITE);
            DrawTextureRec(tAtlas,rVolcanoAF[animVolcano[frameVolcano]],(Vector2){WIDTH-300,0},WHITE);
            if (tab==0 || tab==2) {
                // Draw while playing
                DrawTexturePro(tAtlas,rEgg,(Rectangle){eggPos.x,eggPos.y,rEgg.width,rEgg.height},
                oEgg,Rad2Deg(PI/2-playerRot),WHITE);
                DrawTexturePro(tAtlas,rChicken,(Rectangle){playerPos.x,playerPos.y,rChicken.width,rChicken.height},
                oChicken,Rad2Deg(PI/2-playerRot),WHITE);
            } else if (tab==1) {
                // Draw when game over OR just started game
                if (playerRot<PI/2) {
                    DrawTextureRec(tAtlas,rBrokenEgg,(Vector2){eggPos.x+oBrokenEgg.x,eggPos.y},WHITE);
                    DrawTextureRec(tAtlas,rRightFallenChicken,playerPos,WHITE);
                } else {
                    DrawTextureRec(tAtlas,rBrokenEgg,(Vector2){eggPos.x-oBrokenEgg.x,eggPos.y},WHITE);
                    DrawTextureRec(tAtlas,rLeftFallenChicken,playerPos,WHITE);
                }
            }
            DrawTexturePro(tAtlas,rCorn,(Rectangle){cornPos.x+rCorn.width*0.5*(1-cornPopUpScale),cornPos.y+rCorn.height*0.5*(1-cornPopUpScale),rCorn.width*cornPopUpScale,rCorn.height*cornPopUpScale},oCorn,sin(cornRotValue)*CORN_ROT_RANGE,WHITE);
            if (ateCorn) {
                DrawTexturePro(tAtlas,rCornEatAF[frameCornEat],(Rectangle){ateCornPos.x,ateCornPos.y,rCornEatAF[frameCornEat].width,rCornEatAF[frameCornEat].height},oCorn,0,WHITE);
            }
            for (int i=0;i<rocksOnScreen;i++) {
                DrawTexturePro(tAtlas,animRock[frameRock],(Rectangle){rocks[i].x,rocks[i].y,animRock[frameRock].width,animRock[frameRock].height},oRock,
                0,WHITE);
            }
            for (int i=0;i<MAX_ROCKS;i++) {
                if (frameBreakingRocks[i]>=0 && frameBreakingRocks[i]<=2) {
                    DrawTexturePro(tAtlas,rBreakingRockAF[frameBreakingRocks[i]],(Rectangle){breakingRocks[i].x,breakingRocks[i].y,rBreakingRockAF[frameBreakingRocks[i]].width,rBreakingRockAF[frameBreakingRocks[i]].height},oRock,
                    0,WHITE);
                }
            }
            if (scoreTextFade>0) {
                DrawTextEx(fFont,TextFormat("%i",score-SCORE_PER_CORN),(Vector2){scoreTextPos.x,scoreTextPos.y-64-16},16,0,(Color){0,155,0,255*scoreTextFade});
                DrawTextEx(fFont,TextFormat("+%i",SCORE_PER_CORN),(Vector2){scoreTextPos.x,scoreTextPos.y-64},24,0,(Color){0,155,0,255*scoreTextFade});
                if (newHighscore) {
                    DrawTextEx(fFont,"HIGHSCORE!",(Vector2){scoreTextPos.x,scoreTextPos.y-64+24},20,0,(Color){200,0,0,255*pow(scoreTextFade,8)});
                }
                
            }
            if (scrollY>0) {
                DrawTextureRec(tAtlas,rScroll,(Vector2){-10,-256+(scrollY*scrollY*256)},WHITE);
            }
            if (gameOverBoardY>0) {
                DrawTextureRec(tAtlas,rGameOverBoard,(Vector2){200,-64+(gameOverBoardY*gameOverBoardY*64)},WHITE);
            }
            if (score<10) {
                DrawTextEx(fFont,TextFormat("     %i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            } else if (score<100) {
                DrawTextEx(fFont,TextFormat("    %i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            } else if (score<1000) {
                DrawTextEx(fFont,TextFormat("   %i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            } else if (score<10000) {
                DrawTextEx(fFont,TextFormat("  %i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            } else if (score<100000) {
                DrawTextEx(fFont,TextFormat(" %i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            } else {
                DrawTextEx(fFont,TextFormat("%i",score),(Vector2){264+8,560+8},22,0,(Color){155,0,0,155});
            }
            if (highscore<10) {
                DrawTextEx(fFont,TextFormat("     %i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            } else if (highscore<100) {
                DrawTextEx(fFont,TextFormat("    %i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            } else if (highscore<1000) {
                DrawTextEx(fFont,TextFormat("   %i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            } else if (highscore<10000) {
                DrawTextEx(fFont,TextFormat("  %i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            } else if (highscore<100000) {
                DrawTextEx(fFont,TextFormat(" %i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            } else {
                DrawTextEx(fFont,TextFormat("%i",highscore),(Vector2){520+8,560+8},22,0,(Color){155,0,0,155});
            }
            
            DrawTextEx(fFont,"a game by Morfy",(Vector2){2,600-16-16-16-16-2},16,0,(Color){255,255,255,170});
            DrawTextEx(fFont,"music by Joshua McLean",(Vector2){2,600-16-16-16-2},16,0,(Color){255,255,255,170});
            DrawTextEx(fFont,"some SFXs from freesound.org",(Vector2){2,600-16-16-2},16,0,(Color){255,255,255,170});
            DrawTextEx(fFont,"font by Cile (dafont.com)",(Vector2){2,600-16-2},16,0,(Color){255,255,255,170});
            
            if (raylibFade>0) {
                DrawTexture(raylibLogo,WIDTH/2 - 64, 128,(Color){255,255,255,255*raylibFade});
            }
            
            // Draw things for development purposes
            if (devmode) {
                for (int i=0;i<WIDTH;i++) {
                    DrawLineV(ground(i),ground(i+1),RED);
                }
                for (int i=0;i<rocksOnScreen;i++) {
                    DrawCircleV(rocks[i],ROCK_COLLISION_DISTANCE,(Color){0,0,255,100});
                }
                DrawCircleV(cornPos,CORN_COLLISION_DISTANCE,(Color){0,255,0,150});
                DrawCircleV(ChickenCollisionVec(playerPos,playerRot,CHICKEN_COLLISION_SHIFT),CHICKEN_COLLISION_DISTANCE,(Color){0,0,255,100});
                DrawCircleV(playerPos,4,RED);
                DrawLineV(playerPos,eggPos,RED);
                DrawCircleV(eggPos,2,RED);
                DrawRectangle(0,0,128,22*4,(Color){0,0,0,50});
                DrawFPS(2,20*0+2);
                DrawText(TextFormat("Vel: %i",(int)playerVel.x),2,20*1+2,18,WHITE);
                DrawText(TextFormat("Rot: %f",playerRot),2,20*2+2,18,WHITE);
                DrawText(TextFormat("Diff: %i",(int)difficulty),2,20*3+2,18,RED);
            }

        EndDrawing();
    }
    SaveStorageValue(STPO_HIGHSCORE,highscore);
    UnloadMusicStream(mUnprepared);
    UnloadSound(sfxChew);
    UnloadSound(sfxHighscore);
    UnloadSound(sfxChickenFast1);
    UnloadSound(sfxChickenFast2);
    UnloadSound(sfxChickenScared);
    UnloadSound(sfxPutsh);
    UnloadTexture(tAtlas);
    UnloadTexture(tBackground);
    UnloadFont(fFont);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}