#include "raylib.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "time.h"
#include "stdlib.h"
#include "unistd.h"


const int screenWidth = 1200;
const int screenHeight = 800;


const int mapWidth = 2000;
const int mapHeight = 2000;

const double moveSpeed = 250.0;
const int playerSize = 50;
const int playerMaxHealth = 100;
const double playerHealingPerSec = 5;

const double expPerExp = 1;

//Zombie default stats
const double zDefMoveSpeed = 120.0;
const int zDefSize = playerSize;
const double zDefDamage = 15.0;
const double zDefAttackDelay = 0.5;
const double zDefHealth = 50.0;

const int zombieTypesCount = 4;

//Zombie movement variables
const int zViewDistance = 300;
const double zSeparation = 5;


//Wave diffiulty
const int difficulty = 20;
const float rareZombieChance = 0.2;
const int maxZombieCount = 2048;
const double zombieSpawnDelay = 0.1;

const int fps = 160;

//Guns 
const int maxBulletCount = 1024;

//Detailing
const int environmentDetailLimit = 150;
const int environmentDetailTypes = 2;

const int particleLimit = 1024;



typedef struct ZombieType {
    int firstSpawnWave;
    int spawnTicketsCount;
    int expCount;
    int size;
    double speed;
    double damage;
    double health;
    double attackDelay;
    Color color;
    
} ZombieType;

typedef struct Zombie {
    int type;
    double currentHealth;
    float direction;
    Vector2 pos; 
    double lastAttackTime;
} Zombie;

typedef struct Gun {
    int bulletCount;
    int rpm;
    double damage;
    int penetration;
    double speed;
    int bulletSize;
    double accuracy; 
    
} Gun;

typedef struct Bullet {
    Vector2 pos; 
    int targetsLeft;
    float direction;
    float xVel;
    float yVel;
    int gunIndex;
    int zHitIndexes[15];
    double damage;
    
} Bullet;

typedef struct MapDetail {
    Vector2 pos;
    int type;
    
} MapDetail;

typedef struct Particle {
    int shape; 
    int size;
    int moveType;
    Vector2 pos;
    Vector2 vel;
    Color color;
    double deathTime;
    double lifeTime;
    double speed;
    float rotation;
    
} Particle;


float GetAngle(Vector2 a, Vector2 b) {
    return atan2((a.y - b.y), (a.x - b.x))*(180/(float)PI);
}


float GetPos(float playerPos, float playerScreenPos, float objectPos) {
    return (objectPos - playerPos + playerScreenPos); 
}

int GenerateRandInt(int randNumMax) {
    return rand() % randNumMax;
}

double GetCurrentTime() {
    long currentClock = clock();
    double timeSec = (double)currentClock/CLOCKS_PER_SEC;
    
    return (timeSec);
}

int Vector2Compare(Vector2 a, Vector2 b) {
    if (a.x != b.x) {
        return 0;
    }
    if (a.y != b.y) {
        return 0;
    }
    return 1;
}

int ChooseZombieType (ZombieType* zombieTypes, int wave) {
    
    int totalSpawnTickets = 0;
    
    for (int i = 0; i < zombieTypesCount; i++) {
        if (zombieTypes[i].firstSpawnWave <= wave) {
            totalSpawnTickets += zombieTypes[i].spawnTicketsCount;
        }
    }
    
    int spawnNum = GenerateRandInt(totalSpawnTickets);
    
    for (int i = 0; i < zombieTypesCount; i++) {
        
        spawnNum -= zombieTypes[i].spawnTicketsCount;
        
        if (spawnNum <= 0) {
            return i;
        }
        
    }
    
    return 0;
}

double SpawnZombie(Zombie* zombies, ZombieType* zombieTypes, int wave, double lastZombieSpawnTime, Vector2 defaultZombiePos) {
    int zombieCount = difficulty * wave;
    for (int i = 0; i < zombieCount; i++) {
        
        if (Vector2Compare(defaultZombiePos, zombies[i].pos)) {
            
            zombies[i].type = ChooseZombieType(zombieTypes, wave);
            zombies[i].currentHealth = zombieTypes[zombies[i].type].health;
            
            int spawnPos = ((rand() % 4)); //4 becuase map has 4 walls that zombies can spawn at.
            int spawnWidth = (rand() % mapWidth)-mapWidth/2;
            int spawnHeight = (rand() % mapHeight)-mapHeight/2;
            
            if (spawnPos == 0) { //Top (-y)
                zombies[i].pos.x = spawnWidth;
                zombies[i].pos.y = -mapHeight/2-100;
            } else if (spawnPos == 1) { //Right (+x)
                zombies[i].pos.x = mapWidth/2+100;
                zombies[i].pos.y = spawnHeight;
            } else if (spawnPos == 2) { //Left (-x)
                zombies[i].pos.x = -mapWidth/2-100;
                zombies[i].pos.y = spawnHeight;
            } else { //Bottom (+y)
                zombies[i].pos.x = spawnWidth;
                zombies[i].pos.y = mapHeight/2+100;
            }
            
            zombies[i].direction = 0.0f;
            zombies[i].lastAttackTime = 0.0;
            
            return (GetCurrentTime());
            
        }
    }
    
    return (lastZombieSpawnTime);
}

double CreateZombie(Zombie* zombies, ZombieType* zombieTypes, int wave, double lastZombieSpawnTime, Vector2 defaultZombiePos) {
    
    if (zombieSpawnDelay + lastZombieSpawnTime <= GetCurrentTime()) {
        lastZombieSpawnTime = SpawnZombie(zombies, zombieTypes, wave, lastZombieSpawnTime, defaultZombiePos);
    }
    
    return(lastZombieSpawnTime);    
}
    
    


double CalcCos(float v, double distance) {
    float vRadians = v * PI / 180.0;
    
    return(cos(vRadians)*distance);
}

double CalcSin(float v, double distance) {
    float vRadians = v * PI / 180.0;
    
    return(sin(vRadians)*distance);
}

double CalcHypotenuse(double x, double y) {
    return(sqrt(x*x + y*y));
}

double GetDistance(Vector2 a, Vector2 b) {
    float xDist = a.x - b.x;
    float yDist = a.y - b.y;
    
    return (sqrt(pow(xDist, 2) + pow(yDist, 2)));
}


int CheckPlayerHealth (double* playerHealth) {
    
    if (*playerHealth <= 0) {
        return(1);
    }
    
    return(0);
}


void DamagePlayer (double* playerHealth, double damage) {

    *playerHealth -= damage;
    
}


void ZombieAttackCheck(Zombie* zombies, int currentZombie, Vector2 playerPos, ZombieType* zombieTypes, double* playerHealth) {
    
    double currentTime = GetCurrentTime();
    
    if (currentTime > zombies[currentZombie].lastAttackTime + zombieTypes[zombies[currentZombie].type].attackDelay) {
        
        double playerDistance = GetDistance(zombies[currentZombie].pos, playerPos);
        
        if (playerDistance < zombieTypes[zombies[currentZombie].type].size) {
            
            zombies[currentZombie].lastAttackTime = currentTime;
            DamagePlayer(playerHealth, zombieTypes[zombies[currentZombie].type].damage);
            
        }
        
    }
    
}




void MoveZombie(Zombie* zombies, int zombieIndex, Vector2 playerPos, ZombieType* zombieTypes, Vector2 defaultZombiePos) {
    float v = GetAngle(zombies[zombieIndex].pos, playerPos);
    
    double xChange = CalcCos(v, zombieTypes[zombies[zombieIndex].type].speed);
    double yChange = CalcSin(v, zombieTypes[zombies[zombieIndex].type].speed);
    
    for (int i = 0; i<maxZombieCount; i++) {
        
        if (!Vector2Compare(zombies[i].pos, defaultZombiePos) && i != zombieIndex) {
            
            double cDist = GetDistance(zombies[i].pos, zombies[zombieIndex].pos);

            if (cDist < zViewDistance) { 
                float xDist = zombies[i].pos.x - zombies[zombieIndex].pos.x;
                float yDist = zombies[i].pos.y - zombies[zombieIndex].pos.y;
                xChange += xDist*zSeparation/cDist;
                yChange += yDist*zSeparation/cDist;
            }
        }
    }
    
    Vector2 zTarget = {xChange, yChange};
    Vector2 self = {0, 0};
    
    v = GetAngle(zTarget, self);


    zombies[zombieIndex].pos.x -= (xChange*GetFrameTime());
    zombies[zombieIndex].pos.y -= (yChange*GetFrameTime());
    zombies[zombieIndex].direction = v;

}

void DrawZombie(Zombie* zombies, int zombieIndex, ZombieType* zombieTypes, Vector2 playerPos, Vector2 playerScreenPos){
    
    int zombieSize = zombieTypes[zombies[zombieIndex].type].size;
    float zombieScreenX = GetPos(playerPos.x, playerScreenPos.x, zombies[zombieIndex].pos.x);
    float zombieScreenY = GetPos(playerPos.y, playerScreenPos.y, zombies[zombieIndex].pos.y);
    
    
    Rectangle zombieRec = {zombieScreenX, zombieScreenY, zombieSize, zombieSize};
    Rectangle outlineRec = {zombieScreenX, zombieScreenY, zombieSize + 8, zombieSize + 8};
    Vector2 zombieOffset = {zombieSize/2, zombieSize/2};
    Vector2 outlineOffset = {zombieSize/2 + 4, zombieSize/2 + 4};
    
    //Draw outline
    DrawRectanglePro(outlineRec, outlineOffset, zombies[zombieIndex].direction, BLACK);
    
    //Draw Zombie
    DrawRectanglePro(zombieRec, zombieOffset, zombies[zombieIndex].direction, zombieTypes[zombies[zombieIndex].type].color);
    

}

void ResetZombie(Zombie* zombies, int currentZombie, Vector2 defaultZombiePos) {
    zombies[currentZombie].pos = defaultZombiePos;
}


void CreateBullets(Gun* guns, int currentGun, Bullet* bullet, float direction, Vector2 origin, Vector2 bulletDefaultPos, int playerBonusStatsIndex) {
    
    for (int i = 0; i < (guns[currentGun].bulletCount + guns[playerBonusStatsIndex].bulletCount); i++) {
        float accuracy = (GenerateRandInt(201)-100)/(guns[currentGun].accuracy + guns[playerBonusStatsIndex].accuracy);
        
        for (int j = 0; j < maxBulletCount; j++) {
            
            if (Vector2Compare(bullet[j].pos, bulletDefaultPos)) {
                bullet[j].pos = origin;
                bullet[j].direction = direction + accuracy;
                
                bullet[j].xVel = CalcCos(bullet[j].direction, (guns[currentGun].speed + guns[playerBonusStatsIndex].speed));
                bullet[j].yVel = CalcSin(bullet[j].direction, (guns[currentGun].speed + guns[playerBonusStatsIndex].speed));
                bullet[j].targetsLeft = guns[currentGun].penetration + guns[playerBonusStatsIndex].penetration;
                bullet[j].gunIndex = currentGun;
                bullet[j].damage = guns[currentGun].damage + guns[playerBonusStatsIndex].damage;
                
                break;
            }
        }
    }
}

void DrawBullet(Gun* guns, Bullet* bullet, int currentBullet, Vector2 playerPos, Vector2 playerScreenPos, int playerBonusStatsIndex) {
    
    int bulletSize = guns[bullet[currentBullet].gunIndex].bulletSize + guns[playerBonusStatsIndex].bulletSize;
    float bulletScreenX = GetPos(playerPos.x, playerScreenPos.x, bullet[currentBullet].pos.x);
    float bulletScreenY = GetPos(playerPos.y, playerScreenPos.y, bullet[currentBullet].pos.y);
    
    Vector2 bulletScreenPos = {bulletScreenX, bulletScreenY};
    
    DrawPoly(bulletScreenPos, 3, bulletSize, bullet[currentBullet].direction-30, BLACK);          
    
    
}

double Shoot(Gun* guns, int currentGun, double lastShotTime, Bullet* bullet, Vector2 playerPos, Vector2 defaultBulletPos, float playerRotation, int playerBonusStatsIndex) {

    double minute = 60.0;
    double currentTime = GetCurrentTime();
    
    
    if (currentTime - lastShotTime > minute/(guns[currentGun].rpm + guns[playerBonusStatsIndex].rpm)) {
        
        CreateBullets(guns, currentGun, bullet, playerRotation, playerPos, defaultBulletPos, playerBonusStatsIndex);
        return currentTime;
    }
    
    return lastShotTime;
    
}

void ResetBullet(Bullet* bullet, int currentBullet, Vector2 defaultBulletPos) {
    bullet[currentBullet].pos = defaultBulletPos;
    bullet[currentBullet].xVel = 0;
    bullet[currentBullet].yVel = 0;
    
    int collisionArrayLength = sizeof(bullet[0].zHitIndexes) / sizeof(bullet[0].zHitIndexes[0]);
    
    for (int i = 0; i < collisionArrayLength; i++) {
        bullet[currentBullet].zHitIndexes[i] = -1;
    }
    
}

void MoveBullet(Bullet* bullet, int currentBullet, Vector2 defaultBulletPos, Vector2 playerPos) {
    bullet[currentBullet].pos.x -= bullet[currentBullet].xVel*GetFrameTime();
    bullet[currentBullet].pos.y -= bullet[currentBullet].yVel*GetFrameTime();
    
    int bulletDespawnDistance = screenWidth/2 + 100;
    
    if (bullet[currentBullet].pos.x > playerPos.x + bulletDespawnDistance) {
        ResetBullet(bullet, currentBullet, defaultBulletPos);
    } else if (bullet[currentBullet].pos.x < playerPos.x - bulletDespawnDistance){
        ResetBullet(bullet, currentBullet, defaultBulletPos);    
    }
    
    if (bullet[currentBullet].pos.y > playerPos.y + bulletDespawnDistance) {
        ResetBullet(bullet, currentBullet, defaultBulletPos);
    } else if (bullet[currentBullet].pos.y < playerPos.y - bulletDespawnDistance){
        ResetBullet(bullet, currentBullet, defaultBulletPos);
    }
    
}

int CollisionCheckBullet(Vector2 bulletPos, Zombie* zombies, ZombieType* zombieTypes, int currentZombie) {
    
    int maxCheckDistance = 100;
    int distance = abs(GetDistance(bulletPos, zombies[currentZombie].pos));
    
    if (distance < maxCheckDistance) {
        
        int zombieSize = zombieTypes[zombies[currentZombie].type].size;
        
        float zombieX = zombies[currentZombie].pos.x;
        float zombieY = zombies[currentZombie].pos.y;
        
        Rectangle zombieRec = {zombieX - zombieSize/2, zombieY - zombieSize/2, zombieSize, zombieSize};
        
        if (CheckCollisionPointRec(bulletPos, zombieRec)) {

            return(currentZombie);
        }
        
    }
    
    return(-1);
    
}



void DrawParticle(Vector2 particlePos, int size, float direction, Color color, int shape, Vector2 playerPos, Vector2 playerScreenPos) {

    if (shape == 0) {
        DrawCircle(GetPos(playerPos.x, playerScreenPos.x, particlePos.x), GetPos(playerPos.y, playerScreenPos.y, particlePos.y), size, color);
    }
    else if (shape == 1) {
        Rectangle particleRec = {particlePos.x, particlePos.y, size, size};
        particlePos.x -= size/2;
        particlePos.y -= size/2;
        DrawRectanglePro(particleRec, particlePos, direction, color);
    }
}

Vector2 SetParticleVel(Vector2 originVel, float velChangeMax) {

    Vector2 returnVel = originVel;

    float velChangePercent = (float)GenerateRandInt(100)/100; 
    returnVel.x += (velChangeMax/2 - velChangeMax * velChangePercent);

    velChangePercent = (float)GenerateRandInt(100)/100; 
    returnVel.y += (velChangeMax/2 - velChangeMax * velChangePercent);
    
    return returnVel;
    
}

double SetParticleLifeTime(double lifeTime, double lifeTimeDiffMax) {
    
    double lifeTimeChangePercent = (double)GenerateRandInt(100)/100; 
    double returnTime = lifeTime + (lifeTimeDiffMax/2 - lifeTimeDiffMax * lifeTimeChangePercent);

    return returnTime;
    
}


void CreateParticles(Particle* particles, Vector2 originPos, Vector2 originVel, float velChangeMax, int shape, Color color, int size, int count, float rotation, double lifeTime, double lifeTimeDiffMax, int type) {
    
    double currentTime = GetCurrentTime();
    
    for (int i = 0; i < count; i++) {
        
        for (int j = 0; j < particleLimit; j++) {
            
            if (particles[j].deathTime < currentTime) {
                
                particles[j].pos = originPos;
                particles[j].size = size;
                particles[j].shape = shape;
                particles[j].color = color;
                particles[j].vel = SetParticleVel(originVel, velChangeMax);
                particles[j].lifeTime = SetParticleLifeTime(lifeTime, lifeTimeDiffMax);
                particles[j].deathTime = currentTime + particles[j].lifeTime;
                particles[j].moveType = type;
                particles[j].speed = CalcHypotenuse(particles[j].vel.x, particles[j].vel.y);
                
                if (rotation != 0) {
                    particles[j].rotation = rotation;
                } else {
                    Vector2 zero = {0, 0};
                    particles[j].rotation = GetAngle(zero, particles[j].vel);
                }

                
                break;
                
            }
            
        }
        
    }
    
}


void DrawAllParticles (Particle* particles, Vector2 playerPos, Vector2 playerScreenPos) {
    double currentTime = GetCurrentTime();
    
    for (int i = 0; i < particleLimit; i++) {
        
        if (particles[i].deathTime > currentTime) {

            DrawParticle(particles[i].pos, particles[i].size, particles[i].rotation, particles[i].color, particles[i].shape, playerPos, playerScreenPos); 
            
        }
        
    }
    
}

void MoveParticleLinear(Particle* particles, int currentParticle) {
    
    particles[currentParticle].pos.x -= particles[currentParticle].vel.x * GetFrameTime();
    particles[currentParticle].pos.y -= particles[currentParticle].vel.y * GetFrameTime();
    
}

void MoveParticleSlowDown(Particle* particles, int currentParticle) {
    
    particles[currentParticle].pos.x -= particles[currentParticle].vel.x * GetFrameTime();
    particles[currentParticle].pos.y -= particles[currentParticle].vel.y * GetFrameTime();
    
    
    double currentTime = GetCurrentTime();
    
    double remainingLife = particles[currentParticle].deathTime - currentTime;
    double remainingLifePercent = remainingLife/particles[currentParticle].lifeTime;
    
    particles[currentParticle].vel.x = particles[currentParticle].vel.x * remainingLifePercent;
    particles[currentParticle].vel.y = particles[currentParticle].vel.y * remainingLifePercent;
    
}

float SlowTurn(float turnAngle, float rotationPerSecond) {
    
    double rotationCurrentFrame = rotationPerSecond * GetFrameTime();
    
    if (turnAngle > 0) {
        
        return(rotationCurrentFrame);
        
    } else {
        
        return(-rotationCurrentFrame);
        
    }
    
}

void ResetParticle(Particle* particles, int currentParticle) {
    
    particles[currentParticle].deathTime = 0; 
    particles[currentParticle].speed = 0; 
    particles[currentParticle].rotation = 0;
    
}

int CollisionCheckParticle(Particle* particles, int currentParticle, Vector2 target, float targetOffset) {
    
    
    Rectangle playerRec = {target.x - targetOffset/2, target.y - targetOffset/2, playerSize, playerSize};
    
    if (CheckCollisionCircleRec(particles[currentParticle].pos, particles[currentParticle].size, playerRec)) {
        return(1);
    } else {
        return(0);
    }
    
}

void MoveParticleTowardsTargetSlow(Particle* particles, int currentParticle, Vector2 target, double* playerExpPointer) {
    
    float targetOffset = playerSize;
    
    if (CollisionCheckParticle(particles, currentParticle, target, targetOffset)) {
        ResetParticle(particles, currentParticle);
        *playerExpPointer += expPerExp;
    }
    
    float targetAngle = GetAngle(target, particles[currentParticle].pos);
    float currentAngle = particles[currentParticle].rotation;

    
    float angleDiff = targetAngle - currentAngle;
    
    if (angleDiff > 180) {
        angleDiff -= 360.0f;
    } else if (angleDiff < -180) {
        angleDiff += 360.0f;
    }
    
    float rotationPerSecond = 360;
    
    particles[currentParticle].rotation -= SlowTurn(angleDiff, rotationPerSecond);
    
    if (particles[currentParticle].rotation > 180) {
        particles[currentParticle].rotation = -(360 - particles[currentParticle].rotation);
    } else if (particles[currentParticle].rotation < -180) {
        particles[currentParticle].rotation = 360 + particles[currentParticle].rotation;
    }
    
    particles[currentParticle].pos.x -= CalcCos(particles[currentParticle].rotation, particles[currentParticle].speed)*GetFrameTime();
    particles[currentParticle].pos.y -= CalcSin(particles[currentParticle].rotation, particles[currentParticle].speed)*GetFrameTime();
    
}

void MoveAllParticles (Particle* particles, Vector2 playerPos, double* playerExpPointer) {
    double currentTime = GetCurrentTime();
    
    for (int i = 0; i < particleLimit; i++) {
        
        if (particles[i].deathTime > currentTime) {
            
            if (particles[i].moveType == 0) { 
            
                MoveParticleLinear(particles, i);
                
            } else if (particles[i].moveType == 1) {
                
                MoveParticleSlowDown(particles, i);
                
            } else if (particles[i].moveType == 2) {
                
                MoveParticleTowardsTargetSlow(particles, i, playerPos, playerExpPointer);
                
            } else if (particles[i].moveType == 3) {
                
                MoveParticleTowardsTargetSlow(particles, i, playerPos, playerExpPointer);
                
            }
            
        }
        
    }
    
}


void AddBloodExplosion(Particle* particles, Color color, Vector2 bulletVel, Vector2 bulletPos, int zombieSize) {
    
    int bloodCount = zombieSize/4;
    float velChangeMax = 45*zombieSize;
    float rotation = 0;
    int shape = 0;
    int size = zombieSize/8;
    double bloodLifeTime = 2.0;
    double LifeTimeDiffMax = 0.25;
    int moveType = 1;
    
    CreateParticles(particles, bulletPos, bulletVel, velChangeMax, shape, color, size, bloodCount, rotation, bloodLifeTime, LifeTimeDiffMax, moveType);
    
}

void AddExperienceExplosion(Particle* particles, Color color, Vector2 bulletVel, Vector2 bulletPos, int zombieExpCount) {
    
    float velChangeMax = 1500;
    float rotation = 0;
    int shape = 0;
    int size = 6;
    double lifeTime = 25;
    double LifeTimeDiffMax = 0.25;
    int moveType = 2;
    Color experienceColor = GOLD;
    
    CreateParticles(particles, bulletPos, bulletVel, velChangeMax, shape, experienceColor, size, zombieExpCount, rotation, lifeTime, LifeTimeDiffMax, moveType);
    
}

void AddBloodSplatter(Particle* particles, Color color, Vector2 bulletVel, Vector2 bulletPos, int zombieSize) {
    
    int bloodCount = 4;
    float velChangeMax = 300;
    float rotation = 0;
    int shape = 0;
    int size = zombieSize/10;
    double bloodLifeTime = 2;
    double LifeTimeDiffMax = 0.5;
    int moveType = 1;
    
    CreateParticles(particles, bulletPos, bulletVel, velChangeMax, shape, color, size, bloodCount, rotation, bloodLifeTime, LifeTimeDiffMax, moveType);
    
}

void GenerateDetail(MapDetail* mapDetails, int currentDetail) {
    int spawnX = GenerateRandInt(mapWidth) - mapWidth/2;
    int spawnY = GenerateRandInt(mapHeight) - mapHeight/2;
    int type = GenerateRandInt(environmentDetailTypes);
    
    mapDetails[currentDetail].pos.x = spawnX;
    mapDetails[currentDetail].pos.y = spawnY;
    mapDetails[currentDetail].type = type;
}

void DrawAllDetail (MapDetail* mapDetails, int detailRandomizer, Vector2 playerPos, Vector2 playerScreenPos) {
    
    for (int i = 0; i < environmentDetailLimit; i++) {
        
        if (mapDetails[i].type == 0) { //Grass
            float size = i*detailRandomizer % 100/10 + 1;
            
            DrawCircle(GetPos(playerPos.x, playerScreenPos.x, mapDetails[i].pos.x), GetPos(playerPos.y, playerScreenPos.y, mapDetails[i].pos.y), size, DARKGREEN);
            
        } else if (mapDetails[i].type == 1) { //Rock
            float size = i*detailRandomizer % 200/10 + 10;
            int rotation = i*detailRandomizer % 359;
            int sides = i*detailRandomizer % 7;
             
            Vector2 center = {GetPos(playerPos.x, playerScreenPos.x, mapDetails[i].pos.x), GetPos(playerPos.y, playerScreenPos.y, mapDetails[i].pos.y)};
            
            DrawPoly(center, sides, size, rotation, GRAY);

        }
    }
}

void DamageZombie (Bullet* bullets, int currentBullet, int hitZombieIndex, Zombie* zombies, Vector2 defaultZombiePos, Vector2 defaultBulletPos, Particle* particles, ZombieType* zombieTypes) {
    
    zombies[hitZombieIndex].currentHealth -= bullets[currentBullet].damage;
    bullets[currentBullet].targetsLeft --;
    Vector2 bulletVel = {bullets[currentBullet].xVel/2, bullets[currentBullet].yVel/2};
    AddBloodSplatter(particles, zombieTypes[zombies[hitZombieIndex].type].color, bulletVel, zombies[hitZombieIndex].pos,  zombieTypes[zombies[hitZombieIndex].type].size);
    
    if (zombies[hitZombieIndex].currentHealth <= 0) {
        Vector2 zero = {0, 0};
        AddBloodExplosion(particles, zombieTypes[zombies[hitZombieIndex].type].color, zero, zombies[hitZombieIndex].pos,  zombieTypes[zombies[hitZombieIndex].type].size); 
        AddExperienceExplosion(particles, zombieTypes[zombies[hitZombieIndex].type].color, zero, zombies[hitZombieIndex].pos,  zombieTypes[zombies[hitZombieIndex].type].expCount); 
        ResetZombie(zombies, hitZombieIndex, defaultZombiePos);
    }
    if (bullets[currentBullet].targetsLeft <= 0) {
        ResetBullet(bullets, currentBullet, defaultBulletPos);
    }
    
}

void AddColision(Bullet* bullets, int currentBullet, int collisionID, Zombie* zombies, Vector2 defaultZombiePos, Vector2 defaultBulletPos, ZombieType* zombieTypes, Particle* particles) {
    
    int collisionArrayLength = sizeof(bullets[0].zHitIndexes) / sizeof(bullets[0].zHitIndexes[0]);
    
    for(int i = 0; i < collisionArrayLength; i++) {

        if (bullets[currentBullet].zHitIndexes[i] == collisionID) {
            return;
        } else if (bullets[currentBullet].zHitIndexes[i] == -1) {
            
            bullets[currentBullet].zHitIndexes[i] = collisionID;
            DamageZombie (bullets, currentBullet, collisionID, zombies, defaultZombiePos, defaultBulletPos, particles, zombieTypes);
            return;
        }
        
    }
    
}

void CheckHitsAll(Bullet* bullets, int currentBullet, Zombie* zombies, ZombieType* zombieTypes, Vector2 defaultZombiePos, Vector2 defaultBulletPos, Particle* particles) {
    

    for (int i = 0; i < maxZombieCount; i++) {
        
        if (!Vector2Compare(zombies[i].pos, defaultZombiePos)) {
            
            int collisionID = CollisionCheckBullet(bullets[currentBullet].pos, zombies, zombieTypes, i);
            
            if (collisionID != -1) {
                AddColision(bullets, currentBullet, collisionID, zombies, defaultZombiePos, defaultBulletPos, zombieTypes, particles);
            }
            
        }
        
    }
    
}

void ShowDeathScreen() {    
    DrawRectangle(screenWidth/2 - 100 ,screenHeight/2 - 12, 200, 50, RAYWHITE);
    DrawText("YOU DIED", screenWidth/2 - 75 ,screenHeight/2, 30, BLACK);    
}

void DrawPlayerHealthBar(double playerHealth, Vector2 playerScreenPos) {
    
    int healthBarWidth = 120;
    int healthBarHeight = 15;
    int healthBarMargin = 30;
    int healthBarBorder = 6; 
    Color remainingHealthColor = GREEN;
    Color baseColor = RED;
    
    double remainingHealthWidth = healthBarWidth * (playerHealth/playerMaxHealth);
    
    DrawRectangle(playerScreenPos.x - healthBarWidth/2 - healthBarBorder/2, playerScreenPos.y + healthBarHeight + healthBarMargin - healthBarBorder/2, healthBarWidth + healthBarBorder, healthBarHeight + healthBarBorder, BLACK);
    DrawRectangle(playerScreenPos.x - healthBarWidth/2, playerScreenPos.y + healthBarHeight + healthBarMargin, healthBarWidth, healthBarHeight, baseColor);
    DrawRectangle(playerScreenPos.x - healthBarWidth/2, playerScreenPos.y + healthBarHeight + healthBarMargin, remainingHealthWidth, healthBarHeight, remainingHealthColor);
    

}

void DrawPlayerExpBar(double currentPlayerExp, double neededPlayerExp, int playerLevel) {
    
    int expBarBorder = 6; 
    int expBarWidth = screenWidth-expBarBorder*2;
    Vector2 expBarPos = {expBarBorder, expBarBorder};
    int expBarHeight = 16;
    int lvlTextSize = 15;

    Color remainingExpColor = GOLD;
    Color baseColor = GRAY;
    
    double remainingExpWidth = expBarWidth * (currentPlayerExp/neededPlayerExp);
    
    DrawRectangle(expBarPos.x - expBarBorder/2, expBarPos.y - expBarBorder/2, expBarWidth + expBarBorder, expBarHeight + expBarBorder, BLACK);
    DrawRectangle(expBarPos.x, expBarPos.y, expBarWidth, expBarHeight, baseColor);
    DrawRectangle(expBarPos.x, expBarPos.y, remainingExpWidth, expBarHeight, remainingExpColor);
    
    char level[BUFSIZ];
    
    sprintf(level, "%d", playerLevel);
    
    DrawText("Lvl.", screenWidth - 45 , expBarPos.y + expBarHeight/2 - lvlTextSize/2, lvlTextSize, BLACK);    
    DrawText(level, screenWidth - 20, expBarPos.y + expBarHeight/2 - lvlTextSize/2, lvlTextSize, BLACK);
    
    
}

bool ArrayContainsNumber(int arr[], int length, int number) {
    for (int i = 0; i < length; i++) {
        if (arr[i] == number) {
            return true;
        }
    }
    return false;
}

int GetUpgrade(int* currentUpgrades, int* gunsRollTickets, int upgradesCount) {
    
    int totalStatsForWeapons = 7; 

    int totalTickets = 0; 
    for (int i = 0; i < totalStatsForWeapons; i++) {
        totalTickets += gunsRollTickets[i];
    }
    
    while(true) {
        
        int upgradeNum = GenerateRandInt(totalTickets);
        
        for (int i = 0; i < totalStatsForWeapons; i++) {
            
            upgradeNum -= gunsRollTickets[i];
            
            if (upgradeNum <= 0 && !ArrayContainsNumber(currentUpgrades, upgradesCount, i)) {
                
                return(i);
                
            }
            
        }
        
    }
    
}

int* GetPlayerUpgrades(int upgradesCount, int* gunsRollTickets) {
    
    int *chosenUpgrades = (int*)malloc(upgradesCount * sizeof(int));
    
    for (int i = 0; i < upgradesCount; i++) {
       
        chosenUpgrades[i] = GetUpgrade(chosenUpgrades, gunsRollTickets, upgradesCount);
        printf("GetUpgrades: %d\n", chosenUpgrades[i]);   
    }
    
    return(chosenUpgrades);
    
}

Rectangle GetUpgradeRectangle(int i, int upgradesCount) {
    
    int width = 300;
    int height = 450;

    Vector2 pos;
    
    pos.x = (screenWidth/upgradesCount * i + (width/upgradesCount)/2);
    pos.y = (screenHeight/2 - height/2);
    
    Rectangle rec = {pos.x, pos.y, width, height};
    
    return rec;
    
}

void WriteUpgradeDetails(Rectangle rec, int chosenUpgrade) {
    
    int fontSize = 30;
    
    char upgradeNum[BUFSIZ];
    sprintf(upgradeNum, "%d", chosenUpgrade);
    
    DrawText(upgradeNum, rec.x + 3, rec.y, fontSize, BLACK);
    DrawText(upgradeNum, rec.x + rec.width - fontSize/2 - 3, rec.y + rec.height - fontSize + 2, fontSize, BLACK);
    
    if (chosenUpgrade == 0) {
        DrawText("Bullet Count", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK);  
    } else if (chosenUpgrade == 1) {
        DrawText("Gun RPM", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    } else if (chosenUpgrade == 2) {
        DrawText("Bullet Damage", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    } else if (chosenUpgrade == 3) {
        DrawText("Bullet Penetration", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    } else if (chosenUpgrade == 4) {
        DrawText("Bullet Speed", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    } else if (chosenUpgrade == 5) {
        DrawText("Bullet Size", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    } else if (chosenUpgrade == 6) {
        DrawText("Gun Accuracy", rec.x + rec.width/4, rec.y + rec.height/2, fontSize, BLACK); 
    }

}

void DrawPlayerUpgrades(int* chosenUpgrades, int upgradesCount) {
    
    Color color = LIGHTGRAY;
    
    Rectangle *upgradesRectangles = malloc(upgradesCount * sizeof(Rectangle));
    
    for (int i = 0; i < upgradesCount; i++) {
       
        upgradesRectangles[i] = GetUpgradeRectangle(i, upgradesCount);
        DrawRectangleRec(upgradesRectangles[i], color);
        WriteUpgradeDetails(upgradesRectangles[i], chosenUpgrades[i]);
     
    }
    
    free(upgradesRectangles);
    
}

void DoUpgrade(int chosenUpgrade, int playerBonusStatsIndex, Gun* guns) {
    
    /*
    int bulletCount;
    int rpm;
    double damage;
    int penetration;
    double speed;
    int bulletSize;
    double accuracy; 
    */
    
    int bulletCountPlus = 1;
    int rpmPlus = 60;
    double damagePlus = 5;
    int penetrationPlus = 1;
    double speedPlus = 100;
    int bulletSizePlus = 2;
    double accuracyPlus = 4;
    
    if (chosenUpgrade == 0) {
        guns[playerBonusStatsIndex].bulletCount += bulletCountPlus;
    } else if (chosenUpgrade == 1) {
        guns[playerBonusStatsIndex].rpm += rpmPlus;
    } else if (chosenUpgrade == 2) {
        guns[playerBonusStatsIndex].damage += damagePlus;
    } else if (chosenUpgrade == 3) {
        guns[playerBonusStatsIndex].penetration += penetrationPlus;
    } else if (chosenUpgrade == 4) {
        guns[playerBonusStatsIndex].speed += speedPlus;
    } else if (chosenUpgrade == 5) {
        guns[playerBonusStatsIndex].bulletSize += bulletSizePlus;
    } else if (chosenUpgrade == 6) {
        guns[playerBonusStatsIndex].accuracy += accuracyPlus;
    }  
    
}

bool CheckUpgradeHitboxes(int* chosenUpgrades, int upgradesCount, int playerBonusStatsIndex, Gun* guns) {
    
    Rectangle *upgradesRectangles = malloc(upgradesCount * sizeof(Rectangle));
    
    Vector2 mousePosition = GetMousePosition();
    
    for (int i = 0; i < upgradesCount; i++) {
       
        upgradesRectangles[i] = GetUpgradeRectangle(i, upgradesCount);
        
        if (CheckCollisionPointRec(mousePosition, upgradesRectangles[i])) {
            
            DoUpgrade(chosenUpgrades[i], playerBonusStatsIndex, guns);
            free(chosenUpgrades);
            return(true);
            
        }
     
    }
    
    free(upgradesRectangles);
    
    return(false);
    
} 






int main(void)
{   
    //Creating map walls
    struct Vector2 mapWalls[4];
    
    mapWalls[0].x = -mapWidth/2+playerSize/2;
    mapWalls[0].y = mapHeight/2+playerSize/2;
    
    mapWalls[1].x = mapWidth/2+playerSize/2;
    mapWalls[1].y = mapHeight/2+playerSize/2;
    
    mapWalls[2].x = mapWidth/2+playerSize/2;
    mapWalls[2].y = -mapHeight/2+playerSize/2;
    
    mapWalls[3].x = -mapWidth/2+playerSize/2;
    mapWalls[3].y = -mapHeight/2+playerSize/2;

    //Creating zombie types
    ZombieType zombieTypes[4] = {
        //Normal
        {0, 100, 3, zDefSize, zDefMoveSpeed, zDefDamage, zDefHealth, zDefAttackDelay, GREEN},
        
        //Strong
        {2, 10, 6, zDefSize*1.5, zDefMoveSpeed*0.9, zDefDamage*2, zDefHealth*4, zDefAttackDelay*1.5, RED}, 
        
        //Fast
        {4, 10, 3, zDefSize*0.8, zDefMoveSpeed*2, zDefDamage, zDefHealth*0.6, zDefAttackDelay*0.7, BLUE},
        
        //Giant
        {6, 2, 20, zDefSize*3.5, zDefMoveSpeed*0.7, zDefDamage*3.5, zDefHealth*10, zDefAttackDelay*5, PURPLE}
        
    };
    int wave = 1;
    
    Vector2 defaultZombiePos = {mapWidth, mapHeight};
    Vector2 defaultBulletPos = {mapWidth, mapHeight};
    
    Zombie currentZombies[maxZombieCount];
    for (int i = 0; i<maxZombieCount; i++) {
        currentZombies[i].pos = defaultZombiePos;
    }
    
    for (int i = 0; i<maxZombieCount; i++) {
        currentZombies[i].pos = defaultZombiePos;
    }
    
    Bullet currentBullets[maxBulletCount];
    int collisionArrayLength = sizeof(currentBullets[0].zHitIndexes) / sizeof(currentBullets[0].zHitIndexes[0]);
    for (int i = 0; i<maxBulletCount; i++) {
        currentBullets[i].pos = defaultBulletPos;
        
        for (int j = 0; j<collisionArrayLength; j++) {
            currentBullets[i].zHitIndexes[j] = -1;
        }
        
    }
    
    
    //Creating gun types
    Gun guns[7] = {
        /*
        int bulletCount;
        int rpm;
        double damage;
        int penetration;
        double speed;
        int bulletSize;
        double accuracy; 
        */
        
        //PlayerBonusStats
        {0, 0, 0, 0, 0, 0, 0},
        
        //Pistol        
        {1, 200, 20.0, 1, 800.0, 10, 20},
        //shotgun
        {15, 200, 20, 1, 800.0, 6, 4},
        //shotgun 2
        {45, 500, 2, 1, 800.0, 8, 3.5},
        //sniper
        {1, 80, 100, 10, 1500.0, 15, 100},
        //minigun
        {1, 2500, 12, 1, 800.0, 8, 10},
        //obliteration
        {360, 100, 100, 2, 100, 100, 0.55}
        
    };

    int gunsRollTickets[7] = {1, 3, 6, 2, 3, 0, 6};
    
    
    int currentGun = 1;
    int playerBonusStatsIndex = 0;
    
    MapDetail mapDetails[environmentDetailLimit];
    int detailRandomizer = GenerateRandInt(1000);
    for (int i = 0; i < environmentDetailLimit; i++){
        GenerateDetail(mapDetails, i);
    }
    
    Particle particles[particleLimit];
    for (int i = 0; i < particleLimit; i++){
        particles[i].deathTime = 0; 
        particles[i].speed = 0; 
    }
    
    double lastZombieSpawnTime = 0.0;
    double oldLastZombieSpawnTime = 0.0;
    
    Vector2 playerOffset = {playerSize/2, playerSize/2};
    float playerRotation = 0.0f;
    
    Vector2 playerPos = {0, 0};
    Vector2 playerScreenPos = {(screenWidth)/2, (screenHeight)/2};
    
    double currentMoveSpeed = moveSpeed;
    
    int playerLevel = 1;
    double playerExp = 0;
    double* playerExpPointer = &playerExp;
    double expPerLevel = 35;
    double neededPlayerExp =  playerLevel * expPerLevel;
    
    int counter = 0;
    
    int playerDead = 0;
    int upgradeTime = 0; 
    int upgradesCount = 3;
    int* upgradesPointer;
    
    int spawnedZombieCount = 0;

    double lastShotTime = GetCurrentTime();
    
    double playerHealth = playerMaxHealth;
    double* playerHealthPointer = &playerHealth;
  

    InitWindow(screenWidth, screenHeight, "raylib test");
    SetTargetFPS(fps);
    
    srand((unsigned int)(clock()) ^ getpid()); 
    
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        
        //if player is alive
        if (playerDead == 0 && upgradeTime == 0) {
            
            //Movement Calculation:
            currentMoveSpeed = moveSpeed*GetFrameTime();
           
            double yVel = 0;
            double xVel = 0;
            
            if (IsKeyDown(KEY_D)) {
                xVel += currentMoveSpeed; 
            }
            if (IsKeyDown(KEY_A)) {
                xVel -= currentMoveSpeed;
            }
            if (IsKeyDown(KEY_W)) {
                yVel -= currentMoveSpeed;
            }
            if (IsKeyDown(KEY_S)) {
                yVel += currentMoveSpeed; 
            }      
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsKeyDown(KEY_SPACE)) {
                lastShotTime = Shoot(guns, currentGun, lastShotTime, currentBullets, playerPos, defaultBulletPos, playerRotation, playerBonusStatsIndex);
            }
            
            if (xVel != 0 && yVel != 0) {
                
                double k = currentMoveSpeed/(sqrt(pow(yVel, 2) + pow(xVel, 2)));
                
                playerPos.x = playerPos.x + xVel*k;
                playerPos.y = playerPos.y + yVel*k;
                
            } else {
                playerPos.x = playerPos.x + xVel;
                playerPos.y = playerPos.y + yVel;
            }
            
            if (playerPos.x > mapWidth/2) {
                playerPos.x = mapWidth/2;
            } else if (playerPos.x < -mapWidth/2+playerSize){
                playerPos.x = -mapWidth/2+playerSize;
            }
            
            if (playerPos.y > mapHeight/2) {
                playerPos.y = mapHeight/2;
            } else if (playerPos.y < -mapHeight/2+playerSize){
                playerPos.y = -mapHeight/2+playerSize;
            }
            
            //Player Rotation Calculation:
            playerRotation = atan2((playerScreenPos.y - GetMouseY()), (playerScreenPos.x - GetMouseX()))*(180/PI);
            
            //Heal player
            playerHealth += playerHealingPerSec * GetFrameTime();
            if (playerHealth > playerMaxHealth) {
                
                playerHealth = playerMaxHealth;
                
            } else if (playerHealth <= 0) {
                
                playerDead = 1;
                
            }
            
            //Check lvlUp
            if (playerExp > neededPlayerExp) {
                
                playerExp -= neededPlayerExp;
                playerLevel += 1;
                neededPlayerExp =  playerLevel * expPerLevel;
                
                upgradeTime = 1;
                upgradesPointer = GetPlayerUpgrades(upgradesCount, gunsRollTickets);
            }
            
            
            
            //Zomibe alive check
            int targetZombieCount = difficulty*wave;

            if (targetZombieCount != spawnedZombieCount) {
                oldLastZombieSpawnTime = lastZombieSpawnTime;
                
                lastZombieSpawnTime = CreateZombie(currentZombies, zombieTypes, wave, lastZombieSpawnTime, defaultZombiePos);
                
                if (oldLastZombieSpawnTime != lastZombieSpawnTime) {
                    spawnedZombieCount++;
                }
            }
            
            int aliveZombies = 0;
            for (int i = 0; i<maxZombieCount; i++) {
                if (!Vector2Compare(currentZombies[i].pos, defaultZombiePos)) {
                    MoveZombie(currentZombies, i, playerPos, zombieTypes, defaultZombiePos);
                    ZombieAttackCheck(currentZombies, i, playerPos, zombieTypes, playerHealthPointer);
                    aliveZombies ++;
                }
            }
            
            if (aliveZombies == 0 && targetZombieCount == spawnedZombieCount) {
                wave++;
                spawnedZombieCount = 0;
            } 
            
            MoveAllParticles(particles, playerPos, playerExpPointer);
            
            for (int i = 0; i<maxBulletCount; i++) {
                if (!Vector2Compare(currentBullets[i].pos, defaultBulletPos)) {
                    MoveBullet(currentBullets, i, defaultBulletPos, playerPos);
                    CheckHitsAll(currentBullets, i, currentZombies, zombieTypes, defaultZombiePos, defaultBulletPos, particles);
                }
            }
            
        } else if (upgradeTime == 1) {
            
            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsKeyReleased(KEY_SPACE) && counter < 1) {
                if (CheckUpgradeHitboxes(upgradesPointer, upgradesCount, playerBonusStatsIndex, guns)) {
                    
                    upgradeTime = 0;
                    counter = 0;
                    
                }
                
            } else {
                
                counter += 1;
                
            }
            
        }
        
        
        // Draw
        //---------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(LIME);
            
            DrawAllDetail (mapDetails, detailRandomizer, playerPos, playerScreenPos);
            
            DrawAllParticles(particles, playerPos, playerScreenPos);

            Rectangle playerRec = {playerScreenPos.x, playerScreenPos.y, playerSize, playerSize};
            DrawRectanglePro(playerRec, playerOffset, playerRotation, BLACK);
            

            
            for (int i = 0; i<maxBulletCount; i++) {
                if (!Vector2Compare(currentBullets[i].pos, defaultBulletPos)) {
                    DrawBullet(guns, currentBullets, i, playerPos, playerScreenPos, playerBonusStatsIndex);
                }
            }
            
            for (int i = 0; i<maxZombieCount; i++) {
                if (!Vector2Compare(currentZombies[i].pos, defaultZombiePos)) {
                    DrawZombie(currentZombies, i, zombieTypes, playerPos, playerScreenPos);
                }
            }
            
            float rotation = 0;
            for (int i = 0; i < 4; i++) {
                int wallWidth = 1000;
                
                Rectangle rect = {GetPos(playerPos.x, playerScreenPos.x, mapWalls[i].x), GetPos(playerPos.y, playerScreenPos.y, mapWalls[i].y), mapHeight+wallWidth, wallWidth};
                Vector2 offset = {0, 0};
                DrawRectanglePro(rect, offset, rotation, GRAY);
                
                rotation -= 90;
            }
        
            DrawPlayerHealthBar(playerHealth, playerScreenPos);
            DrawPlayerExpBar(playerExp, neededPlayerExp, playerLevel);
            
            if (playerDead == 1) {
                ShowDeathScreen();
            }
            
            if (upgradeTime == 1) {
                DrawPlayerUpgrades(upgradesPointer, upgradesCount);
            }            
            
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //-------------------------------------------------------------------------------------- 
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}





