#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#if _WIN32
# include <windows.h>
#endif
#if __APPLE__
# include <OpenGL/gl.h>
# include <OpenGL/glu.h>
# include <GLUT/glut.h>
#else
# include <GL/gl.h>
# include <GL/glu.h>
# include <GL/glut.h>
#endif

// CONSTANT DECLARTION // CONFIGURABLE
# define ROTATE_SPEED 0.005
# define SHIP_SPEED 0.08
# define BULLET_SIZE 1.0
# define BULLET_SPEED 5.0
# define TERMINAL_VELOCITY 1.5
# define INERTIA_DAMPNER 0.0005
# define TERMINAL_TURN_SPEED 0.02
# define TWICE_PI 6.28318530718
# define ASTEROID_VERTICES 10
# define DELAY 30
# define SPAWN_DELAY 250
# define PUFF_SPEED 0.8
# define DEATH_TIME 300

//CHANGE THE RESOLUTION TO WHAT EVER YOU ARE USING
# define SCREEN_WIDTH 2560
# define SCREEN_HEIGHT 1440


// STUCTURE DECLARATION
// Spaceship Structure
typedef struct {

    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;

    float angle;
    float angular_velocity;

    int loaded;
    int reload;

    int destroyed;
    

} spaceship_t;

// Projectile Structure
//Linked list pointer because of multiple instances
typedef struct {

    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;

    struct projectile_t* next_projectile;

} projectile_t;

typedef struct {

    float x_position;
    float y_position;
    float radius;

} explosion_t;

// Alpha determines transperency
typedef struct {

    float x_position;
    float y_position;
    float radius;
    float alpha;

}asteroid_explosion_t;

// Asteroids vertices related to how their drawn
typedef struct {

    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;
    float angle;
    float angular_velocity;
    
    int vertices[2][ASTEROID_VERTICES];
    float radius;

    struct asteroid_t *next_asteroid;

} asteroid_t;

typedef struct {


    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;
    float radius;

    float alpha;

    struct particle_puff_t* next_puff;


}particle_puff_t;

typedef struct {

    float x_position;
    float y_position;
    float x_velocity;
    float y_velocity;

    float radius;
    float alpha;

}black_hole_t;

// GLOBAL VARIABLES
spaceship_t player;
projectile_t* projectiles = NULL;
asteroid_t* asteroids = NULL;
explosion_t explosion;
asteroid_explosion_t asteroid_explosion; 
particle_puff_t* puffs;
black_hole_t hole; 
int score = 0;
int spawner;


// FUNCTION DECLARATION
void init_spaceship(spaceship_t* player);
void init_projectile(projectile_t* bullet);
void init_asteroid(asteroid_t* enemy);
void on_idle();
void mouse(int button, int state, int x, int y);
void update_game_state();
void update_player();
void update_projectile(projectile_t* bullet);
void update_asteroid(asteroid_t* enemy);
void free_projectiles(projectile_t* bullet);
void free_asteroids(asteroid_t* enemy);
void keyboard(unsigned char key, int x, int y);
void display(int button, int state, int x, int y);
void on_reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void init_explosion(); 
void update_explosion();
void init_asteroid_explosion(asteroid_t* enemy);
void update_asteroid_explosion(); 
void delete_projectile(projectile_t* bullet);
void split_asteroid(asteroid_t* enemy);
void delete_asteroid(asteroid_t* enemy);
void init_particle_puff(particle_puff_t *puff); 
void update_puff(particle_puff_t*);
void free_puff();
void init_black_hole();
void update_black_hole();

void init()
{
    /* These OpenGL calls only need to be done once in this sample,
     * but normally they would go elsewhere, e.g. display()
     */
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Asteroids");
    glutFullScreen();
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(on_idle);

}

int main(int argc, char** argv)
{   
    // Declare the intial properties of the intialised objects
    glutInit(&argc, argv);
    init_spaceship(&player);
    init_asteroid(asteroids);
    init_black_hole();
    init();
    glutMainLoop();
    free_projectiles(projectiles);
    free_asteroids(asteroids);
    free_puff();
    return(EXIT_SUCCESS);

}

void display(int button, int state, int x, int y)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    char score_s = (char)(score%10 + '0');
    // 2D rotational matrices to figure out the sin and cos
    if (player.destroyed != 1){
        glBegin(GL_LINE_LOOP);
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(player.x_position + 15 * sin(player.angle), player.y_position + 15 * cos(player.angle), -0.5);
        glVertex3f(player.x_position + 15 * cos(player.angle) - 15 * sin(player.angle), player.y_position - 15* cos(player.angle) - 15 * sin(player.angle), -0.5);
        glVertex3f(player.x_position - 8 * sin(player.angle), player.y_position - 8 * cos(player.angle), -0.5);
        glVertex3f(player.x_position - 15 * cos(player.angle) - 15 * sin(player.angle), player.y_position - 15* cos(player.angle) + 15 * sin(player.angle), -0.5);
        glEnd();

        glBegin(GL_LINES);
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(player.x_position + 15 * sin(player.angle), player.y_position + 15 * cos(player.angle), -0.5);
        glVertex3f(player.x_position - 8 * sin(player.angle), player.y_position - 8 * cos(player.angle), -0.5);
        glEnd();

    }
    // Gets the head for the linked list to draw everyone of the bullets
    projectile_t *bullet = projectiles;
    
    // Draws bullets until the value bullet hits NULL
    while (bullet != NULL) {
        glBegin(GL_POINTS);
        glColor3f(1.0, 1.0, 1.0);
        glVertex3f(bullet->x_position, bullet->y_position, -0.5);
        glVertex3f(bullet->x_position + BULLET_SIZE, bullet->y_position + BULLET_SIZE, -0.5);
        glVertex3f(bullet->x_position + BULLET_SIZE, bullet->y_position - BULLET_SIZE, -0.5);
        glVertex3f(bullet->x_position - BULLET_SIZE, bullet->y_position - BULLET_SIZE, -0.5);
        glVertex3f(bullet->x_position - BULLET_SIZE, bullet->y_position + BULLET_SIZE, -0.5);
        glEnd();
        bullet = bullet->next_projectile;
    }

    particle_puff_t* puff = puffs;

    while (puff != NULL) {
        glBegin(GL_POINTS);
        glColor3f(puff->alpha, puff->alpha, puff->alpha);
        glVertex3f(puff->x_position, puff->y_position, -0.5);
        glVertex3f(puff->x_position + BULLET_SIZE, puff->y_position + BULLET_SIZE, -0.5);
        glVertex3f(puff->x_position + BULLET_SIZE, puff->y_position - BULLET_SIZE, -0.5);
        glVertex3f(puff->x_position - BULLET_SIZE, puff->y_position - BULLET_SIZE, -0.5);
        glVertex3f(puff->x_position - BULLET_SIZE, puff->y_position + BULLET_SIZE, -0.5);
        glEnd();
        puff = puff->next_puff;
    }

        // Gathered from the internet draws points in a circular shape
        glBegin(GL_POINTS);
        glColor3f(hole.alpha, hole.alpha, hole.alpha);
        for (int i = 0; i < 1000; i++) {
            glVertex3f(hole.x_position + hole.radius * cos(i * TWICE_PI / 1000) / 3,
                hole.y_position + hole.radius * sin(i * TWICE_PI / 1000) / 3, -0.5);
        }
        glEnd();

    asteroid_t* enemy = asteroids;

    while (enemy != NULL){

        glBegin(GL_LINE_LOOP);
        glColor3f(1.0, 1.0, 1.0);
        for (int i = 0; i < ASTEROID_VERTICES; i++) {
            glVertex3f(enemy->x_position + enemy->vertices[0][i]*cos(enemy->angle) - enemy->vertices[1][i] * sin(enemy->angle),
                enemy->y_position + enemy->vertices[1][i] * cos(enemy->angle) + enemy->vertices[0][i] * sin(enemy->angle), -0.5);
        }
        glEnd(); 
        enemy = enemy->next_asteroid;
    }

    //experimented and made two circles grow together at different speeds and alternate between big circle and smaller circle for the spikey effect
    if (player.destroyed == 1) {
        glBegin(GL_LINE_LOOP);
        glColor3f(1.0, 1.0, 1.0);
        for (int i = 0; i < 100; i++) {
            glVertex3f(explosion.x_position + explosion.radius * cos(i * TWICE_PI / 100) / 3,
                explosion.y_position + explosion.radius * sin(i * TWICE_PI / 100) / 3, -0.5);
            glVertex3f(explosion.x_position + explosion.radius * cos(i * TWICE_PI / 100) / 2,
                explosion.y_position + explosion.radius * sin(i * TWICE_PI / 100) / 2, -0.5);
        }
        glEnd();
        glRasterPos3f(SCREEN_WIDTH / 2, SCREEN_HEIGHT/2 + 20, -0.5);
        glColor3f(1.0, 1.0, 1.0);
        char string2[10] = "GAME OVER";

        // GLUT library only had certain fonts and font sizes 
        for (char* c2 = string2; *c2 != '\0'; c2++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c2);

        }
        glEnd();
        if (explosion.radius > DEATH_TIME) {
        
        glRasterPos3f(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20, -0.5);
        glColor3f(1.0, 1.0, 1.0);
        char string3[] = "PRESS ANY KEY TO CONTINUE";

        for (char* c3 = string3; *c3 != '\0'; c3++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c3);

        }
        glEnd();
        }

    }

    if (TRUE) {
        glBegin(GL_POINTS);
        glColor3f(asteroid_explosion.alpha, asteroid_explosion.alpha, asteroid_explosion.alpha);
        for (int i = 0; i < 100; i++) {
            glVertex3f(asteroid_explosion.x_position + asteroid_explosion.radius * cos(i * TWICE_PI / 100) / 3,
                asteroid_explosion.y_position + asteroid_explosion.radius * sin(i * TWICE_PI / 100) / 3, -0.5);
            glVertex3f(asteroid_explosion.x_position + asteroid_explosion.radius * cos(i * TWICE_PI / 100) / 2,
                asteroid_explosion.y_position + asteroid_explosion.radius * sin(i * TWICE_PI / 100) / 2, -0.5);
        }
        glEnd();
    }

    // Border code if player position is outside of the playable area
    // Border consists of multiple rectangles that fade away
    if (player.x_position > SCREEN_WIDTH - 80 || player.x_position < 0 + 80 || player.y_position < 0 + 80|| player.y_position > SCREEN_HEIGHT - 80) {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 20; i++) {
            glColor3f(1.0 - (double)i / (20), 0, 0);
            glVertex3f(0 + i, 0 + i, -.5);
            glVertex3f(0 + i, SCREEN_HEIGHT - i, -.5);
            glVertex3f(SCREEN_WIDTH - i, SCREEN_HEIGHT - i, -.5);
            glVertex3f(SCREEN_WIDTH - i, 1 + i, -.5);
        }

        glEnd();
    }
    // Figured out how to draw the font but not how to convert an int into a string to display 
    glColor3f(1.0, 1.0, 1.0);
    glRasterPos3f(SCREEN_WIDTH/30, SCREEN_HEIGHT - 100, -0.5);
    char string[10] = "SCORE:";
    
    for (char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    
    }
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, score_s);
    glEnd();
    int err;
    while ((err = glGetError()) != GL_NO_ERROR)
        printf("display: %s\n", gluErrorString(err));
    glEnd();
    glutSwapBuffers();

}
void on_reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

}

void keyboard(unsigned char key, int x, int y)
{   

    // Linked list allocates memory and freeing the data allows the memory to be used again
    if (player.destroyed == 1) {
        switch (key) {
        case 27:
        case 'q':
            free_projectiles(projectiles);
            free_asteroids(asteroids);
            free_puff();
            exit(EXIT_SUCCESS);
            break;
        default:
            if(explosion.radius > DEATH_TIME){
                free(asteroids);
                asteroids = NULL;
                init_spaceship(&player);
                init_black_hole();
            }
            break;
        }

        return;
    }

    asteroid_t* new_projectile = asteroids;
    particle_puff_t* new_puff = puffs;
    switch (key)
    {

    case 'a':
        player.angular_velocity = player.angular_velocity - ROTATE_SPEED;
        if (-player.angular_velocity > TERMINAL_TURN_SPEED) {
            // Above a certain value which is turn speed plus the value back to negate the -
            player.angular_velocity = player.angular_velocity + ROTATE_SPEED;
        }
        break;
    case 'w':
        player.x_velocity = player.x_velocity + sin(player.angle) * SHIP_SPEED;
        player.y_velocity = player.y_velocity + cos(player.angle) * SHIP_SPEED;
        init_particle_puff(new_puff);
        // Tests the magnitude of the ships speed
        if (sqrt(player.x_velocity * player.x_velocity + player.y_velocity * player.y_velocity) > TERMINAL_VELOCITY) {
            player.x_velocity = player.x_velocity - sin(player.angle) * SHIP_SPEED;
            player.y_velocity = player.y_velocity - cos(player.angle) * SHIP_SPEED;
        }
        break;
    case 'd':
        player.angular_velocity = player.angular_velocity + ROTATE_SPEED;
        if (player.angular_velocity > TERMINAL_TURN_SPEED) {
            player.angular_velocity = player.angular_velocity - ROTATE_SPEED;
        }
        break;
    case 't':  

        break;
    case 27:
    case 'q':
        exit(EXIT_SUCCESS);
        break;
    default:
        break;


    }
}


void mouse(int button, int state, int x, int y){

    projectile_t *new_projectile = projectiles;

    switch (button) {
    
    case GLUT_LEFT_BUTTON:
        // Will only shoot a bullet when one is loaded which a reload time to stop spamming
        if (state == GLUT_DOWN && player.loaded == 1) {
            init_projectile(new_projectile);
            player.loaded = 0;
            player.reload = 0;
        }
        break;

    default:

        break;
    }

}

void update_game_state() {

    asteroid_t *new_asteroid = asteroids;

    // So the game counts up to the spawn delay and when it becomes greater than the delay it will spawn the next asteroid
    // How many times the game calls an update is related how pc runs the code so the game might sped up because of it
    if(spawner > SPAWN_DELAY){
        while (new_asteroid != NULL) {
            new_asteroid = new_asteroid->next_asteroid;
        }
        init_asteroid(new_asteroid);
        spawner = 0;
    }
    else {
        spawner++;
    }

    projectile_t *bullet = projectiles;
    while (bullet != NULL) {
        update_projectile(bullet);
        if (bullet == NULL) {
            free(bullet);
            break;
        }
        bullet = bullet->next_projectile;
    }
    asteroid_t* enemy = asteroids;
    while (enemy != NULL) {
        update_asteroid(enemy);
        enemy = enemy->next_asteroid;
    }
    particle_puff_t* puff = puffs;
    while (puff != NULL) {
        update_puff(puff);
        if (puff == NULL) {
            free(puff);
            break;
        }
        puff = puff->next_puff;
    }

    // These are seperated because they are unique and are not stored within a linked list
    update_asteroid_explosion();
    update_player(); 
    update_explosion();
    update_black_hole();

}

void update_player() {

    if (player.destroyed != 1) {
        player.x_position = player.x_position + player.x_velocity;
        player.y_position = player.y_position + player.y_velocity;
        player.x_velocity = player.x_velocity - player.x_velocity * INERTIA_DAMPNER + 0.9*((double)hole.x_position - (double)player.x_position)/(pow((double)hole.x_position-player.x_position,2)+ pow((double)hole.y_position-player.y_position, 2));
        player.y_velocity = player.y_velocity - player.y_velocity * INERTIA_DAMPNER + 0.9*((double)hole.y_position - (double)player.y_position)/ (pow((double)hole.y_position - player.x_position, 2) + pow((double)hole.y_position - player.y_position, 2));
        player.angle = player.angle + player.angular_velocity;
        if (player.loaded != 1) {
            if (player.reload > DELAY) {
                player.loaded = 1;
                player.reload = 0;
            }
            else {
                player.reload++;
            }
        }

        // Collision checks
        asteroid_t* enemy = asteroids;
        while (enemy != NULL) {
            if(sqrt(pow(player.x_position -enemy->x_position,2)+pow(player.y_position -enemy->y_position,2)) < enemy->radius){
                    player.destroyed = 1;
                    player.loaded = 0;
                    init_explosion();

            }

            enemy = enemy->next_asteroid;
        }
        
        if (player.x_position > SCREEN_WIDTH || player.x_position < 0 || player.y_position < 0 || player.y_position > SCREEN_HEIGHT) {
            player.destroyed = 1;
            player.loaded = 0;
            init_explosion();

        }
    }

    if (sqrt(pow(player.x_position - hole.x_position, 2) + pow(player.y_position - hole.y_position, 2)) < 50) {
        player.destroyed = 1;
        player.loaded = 0;
        init_explosion();

    }

}

void update_projectile(projectile_t* bullet) {
    if (bullet == NULL) {
        return;
    }
    bullet->x_position = bullet->x_position + bullet->x_velocity;
    bullet->y_position = bullet->y_position + bullet->y_velocity;
    
    asteroid_t* enemy = asteroids;
    while (enemy != NULL) {
        if (sqrt(pow(bullet->x_position - enemy->x_position, 2) + pow(bullet->y_position - enemy->y_position, 2)) < enemy->radius) {
                init_asteroid_explosion(enemy);
                delete_projectile(bullet);
                split_asteroid(enemy);
                delete_asteroid(enemy);
                free(enemy);
                score ++;
                return;
        }
        enemy = enemy->next_asteroid;
    }
}

void update_puff(particle_puff_t* puff) {

    puff->x_position = puff->x_position + puff->x_velocity;
    puff->y_position = puff->y_position + puff->y_velocity;
    if (puff->alpha > 0) {
        puff->alpha = puff->alpha - 0.01;
    }

}

void init_spaceship(spaceship_t *player) {

    player->angle = 1;
    player->x_position = 150;
    player->y_position = 100;
    player->x_velocity = 0;
    player->y_velocity = 0;
    player->angular_velocity = 0;
    player->destroyed = 0;
    score = 0;

}

void init_projectile(projectile_t* bullet) {
    
    bullet = (projectile_t*)malloc(sizeof(projectile_t));
    bullet->x_position = player.x_position;
    bullet->y_position = player.y_position;
    bullet->x_velocity = sin(player.angle) * BULLET_SPEED;
    bullet->y_velocity = cos(player.angle) * BULLET_SPEED; 
    bullet->next_projectile = projectiles;
    projectiles = bullet;

}

void init_asteroid(asteroid_t* enemy) {

    enemy = (asteroid_t*)malloc(sizeof(asteroid_t));
    int wall = rand() % 4;

    switch (wall) {
    case 0:
        enemy->x_position = 0 - 160;
        enemy->y_position = rand() % SCREEN_HEIGHT;
        break;
    case 1:
        enemy->x_position = SCREEN_WIDTH + 160;
        enemy->y_position = rand() % SCREEN_HEIGHT;
        break;
    case 2:
        enemy->x_position = rand() % SCREEN_WIDTH;
        enemy->y_position = 0 - 160;
        break;
    case 3:
        enemy->x_position = rand() % SCREEN_WIDTH;
        enemy->y_position = SCREEN_HEIGHT + 160;
        break;
    default:
        free(enemy);
        return;
        break;
    }

    enemy->x_velocity = (player.x_position-enemy->x_position)/1000 + (double)((rand() % 100)-50)/50;
    enemy->y_velocity = (player.y_position - enemy->y_position)/1000 +(double)((rand() % 100) - 50) /50;
    enemy->angle = 0;
    enemy->angular_velocity = ((float)(rand() % 100)-50)/10000;
    enemy->radius = 100 + (rand() % 60);
    for (int i = 0; i < ASTEROID_VERTICES; i++) {
        // Draws a random asteroid in a random position and draws the circle and randomizes the vertices to give each asteroid and irregular shape
        enemy->vertices[0][i] = enemy->radius*cos(i * TWICE_PI / ASTEROID_VERTICES) + (rand()%60);
        enemy->vertices[1][i] = enemy->radius*sin(i * TWICE_PI / ASTEROID_VERTICES) + (rand()%60);
    }
    enemy->next_asteroid = asteroids;
    asteroids = enemy;

}

void init_explosion() {

    explosion.x_position = player.x_position;
    explosion.y_position = player.y_position;
    explosion.radius = 5;    

}


void init_asteroid_explosion(asteroid_t* enemy) {

    asteroid_explosion.x_position = enemy->x_position;
    asteroid_explosion.y_position = enemy->y_position;
    asteroid_explosion.radius = 250;
    asteroid_explosion.alpha = 1.0;

}


void update_explosion() {

    explosion.radius = explosion.radius + 1;


}

void update_asteroid_explosion() {

    asteroid_explosion.radius = asteroid_explosion.radius + 1;
    if (asteroid_explosion.alpha > 0) {
        asteroid_explosion.alpha = asteroid_explosion.alpha - 0.01;
    }

}

void update_asteroid(asteroid_t* enemy) {
    // Year 11 psyhics F = G Mm/r2 But I m1, m2, G and single constant
    // r2 is the distance between the two objects and F represents the change in the velocity of the two objects hence acceleration 
    enemy->x_position = enemy->x_position + enemy->x_velocity;
    enemy->y_position = enemy->y_position + enemy->y_velocity;
    enemy->x_velocity = enemy->x_velocity+ 3 * ((double)hole.x_position - (double)enemy->x_position) / (pow((double)hole.y_position - enemy->y_position, 2) + pow((double)hole.x_position - enemy->x_position, 2));
    enemy->y_velocity = enemy->y_velocity + 3 * ((double)hole.y_position - (double)enemy->y_position) / (pow((double)hole.y_position - enemy->y_position, 2) + pow((double)hole.x_position - enemy->x_position, 2));
    enemy->angle = enemy->angle + enemy->angular_velocity;
    asteroid_t* enemyb = asteroids;
    while (enemyb != NULL) {
        if (enemyb == enemy || enemy-> radius <3 || enemyb-> radius < 3) {

        }
        else if (sqrt(pow(enemy->x_position - enemyb->x_position, 2) + pow(enemy->y_position - enemyb->y_position, 2)) < enemyb->radius) {
            enemy->x_velocity = enemy->x_velocity/3+(double)(rand() % 30)/50;
            enemy->y_velocity = enemy->y_velocity/3+ (double)(rand() % 30)/50;
            enemyb->x_velocity = -enemy->x_velocity;
            enemyb->y_velocity = -enemy->y_velocity;
        }
        enemyb = enemyb->next_asteroid;
    }

}


void free_asteroids(asteroid_t* enemy) {
    
    asteroid_t* temp;
    while (enemy != NULL) {
        temp = enemy;
        enemy = enemy->next_asteroid;
        free(temp);
    }
}

void split_asteroid(asteroid_t* enemy){

    asteroid_t *asteroid_child_left = (asteroid_t*)malloc(sizeof(asteroid_t));
    asteroid_t *asteroid_child_right = (asteroid_t*)malloc(sizeof(asteroid_t)); 

    float velocity_change_x = (double)((rand() % 100) - 50) / 50;
    float velocity_change_y = (double)((rand() % 100) - 50) / 50;
    float spin_change = (double)((rand() % 100) - 50) / 5000;

    asteroid_child_left->x_position = enemy->x_position + enemy->radius/2;
    asteroid_child_left->y_position = enemy->y_position ;
    asteroid_child_right->x_position = enemy->x_position - enemy->radius/2;
    asteroid_child_right->y_position = enemy->y_position;
    asteroid_child_left->x_velocity = enemy->x_velocity + velocity_change_x;
    asteroid_child_left->y_velocity = enemy->y_velocity + velocity_change_y;
    asteroid_child_right->x_velocity = enemy->x_velocity - velocity_change_x;
    asteroid_child_right->y_velocity = enemy->y_velocity - velocity_change_y;
    asteroid_child_left->angular_velocity = enemy->angular_velocity + spin_change;
    asteroid_child_right->angular_velocity = enemy->angular_velocity - spin_change;
    asteroid_child_left->radius = enemy->radius / 1.5;
    asteroid_child_right->radius = enemy->radius / 1.5;
    asteroid_child_left->angle = 0;
    asteroid_child_right->angle = 0;

    for (int i = 0; i < ASTEROID_VERTICES; i++) {
        asteroid_child_left->vertices[0][i] = asteroid_child_left->radius * (cos(i * TWICE_PI / ASTEROID_VERTICES) + (double)(rand() % 60)/120);
        asteroid_child_left->vertices[1][i] = asteroid_child_left->radius * (sin(i * TWICE_PI / ASTEROID_VERTICES) + (double)(rand() % 60)/120);
    }
    for (int i = 0; i < ASTEROID_VERTICES; i++) {
        asteroid_child_right->vertices[0][i] = asteroid_child_right->radius * (cos(i * TWICE_PI / ASTEROID_VERTICES) + (double)(rand() % 60) / 120);
        asteroid_child_right->vertices[1][i] = asteroid_child_right->radius * (sin(i * TWICE_PI / ASTEROID_VERTICES) + (double)(rand() % 60) / 120);
    }

    asteroid_child_left->next_asteroid = asteroids;
    asteroid_child_right->next_asteroid = asteroid_child_left;
    asteroids = asteroid_child_right;
    
}

void on_idle() {

    update_game_state();
    glutPostRedisplay();

}

void free_projectiles(projectile_t* bullet) {
    
    projectile_t* temp;
    while (bullet != NULL) {
        temp = bullet;
        bullet = bullet->next_projectile;
        free(temp);
    }

}

void delete_projectile(projectile_t* bullet) {

    projectile_t* hold = projectiles;

    if (projectiles == bullet) {

        projectiles = NULL;
        return;

    }

    while (hold->next_projectile!= bullet) {
        hold = hold->next_projectile;
    }
    hold->next_projectile = bullet->next_projectile;
    bullet == NULL;

}

void delete_asteroid(asteroid_t* enemy) {

    asteroid_t* hold = asteroids;

    if (asteroids == enemy) {

        asteroids= NULL;
        return;

    }

    while (hold->next_asteroid != enemy) {
        hold = hold->next_asteroid;
    }
    hold->next_asteroid = enemy->next_asteroid;
    enemy == NULL;

}

void init_particle_puff(particle_puff_t* puff) {

    puff = (particle_puff_t*)malloc(sizeof(particle_puff_t));
    puff->x_position = player.x_position;
    puff->y_position = player.y_position;
    puff->x_velocity = -sin(player.angle) * PUFF_SPEED  + (double)((rand() % 100) - 50) / 100;
    puff->y_velocity = -cos(player.angle) * PUFF_SPEED  + (double)((rand() % 100) - 50) / 100;
    puff->alpha = 1.0;
    puff->next_puff = puffs;
    puffs = puff;

}

void free_puff() {

    particle_puff_t* temp;
    while (puffs != NULL) {
        temp = puffs;
        puffs = puffs->next_puff;
        free(temp);
    }

}

void init_black_hole() {

    int wall = rand() % 4;

    switch (wall) {
    case 0:
        hole.x_position = 0 - 160;
        hole.y_position = rand() % SCREEN_HEIGHT;
        break;
    case 1:
        hole.x_position = SCREEN_WIDTH + 160;
        hole.y_position = rand() % SCREEN_HEIGHT;
        break;
    case 2:
        hole.x_position = rand() % SCREEN_WIDTH;
        hole.y_position = 0 - 160;
        break;
    case 3:
        hole.x_position = rand() % SCREEN_WIDTH;
        hole.y_position = SCREEN_HEIGHT + 160;
        break;
    default:
        return;
        break;
    }
    hole.alpha = 0;

    hole.radius = 100;

    hole.x_velocity = (player.x_position - hole.x_position) / 10000 + (double)((rand() % 100) - 50) / 500;
    hole.y_velocity = (player.y_position - hole.y_position) / 10000 + (double)((rand() % 100) - 50) / 500;

}

void update_black_hole() {

    hole.x_position = hole.x_position + hole.x_velocity;
    hole.y_position = hole.y_position + hole.y_velocity;
    hole.alpha = hole.alpha + 0.01;
    hole.radius = hole.radius - 10;
    if (hole.alpha > 1) {
        hole.alpha = 0;
    }
    if (hole.radius < 0) {
        hole.radius = 1000;

    }

    asteroid_t* enemy = asteroids;
    while (enemy != NULL) {
        if (enemy->radius < 3) {

        }
        else if (sqrt(pow(hole.x_position - enemy->x_position, 2) + pow(hole.y_position - enemy->y_position, 2)) < enemy->radius) {
            split_asteroid(enemy);
            delete_asteroid(enemy);
            free(enemy);
            return;
        }
        enemy = enemy->next_asteroid;
    }
}
    
