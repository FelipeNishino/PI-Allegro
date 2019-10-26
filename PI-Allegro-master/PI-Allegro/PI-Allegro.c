#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define FPS 60
#define windowWidth 800
#define windowHeight 450
#define worldWidth 1000
#define worldHeight 1000

// Felipe
#define projectileVelocity 0.6
#define projectileAccel 0.25
#define projectileOffset 10
#define projectileMax 3
#define projectileDamage 10
#define targetPracticeLife 20
#define enemyMax 2
#define enemyProjectileVelocity 2
#define enemyProjectileOffset 10
#define enemyProjectileMax 25
#define enemyProjectileDamage 10
#define tileSize 32
#define mapSize 100
#define PI 3.14159265
#define SODIUM_STATIC

enum {
	antiVirus,
	antiBiotic,
	antiMycotic
};
enum {
	backgroundL1,
	backgroundL2,
	foreground
};
enum {
	neutral,
	shooting
};
enum {
	ground,
	air,
	roof
};
enum {
	projectileI,
	enemyI
};
enum {
	contact,
	shooter
};

// Helkson
#define Left 0
#define Right 1
#define gravity 0.275

struct sprite {
	// Felipe
	float x, y;
	float x0, y0;
	int tileX, tileY;
	float width, height;
	float life, maxLife;
	int hitboxWidth, hitboxHeight;
	float hbX, hbY;
	int sprite;
	int spriteChange;
	int shotFC;
	int r;
	int g;
	int b;
	int selectedWeapon;
	int type;
	int currentDir;
	bool isShooting;
	bool onGround;
	bool hitCeiling;
	bool alive;

	int dir;
	float vel_x;
	float vel_y;
};

struct projectile {
	float x, y;
	float x0, y0;
	float width, height;
	float speed, accel;
	float angle, sin, cos;
	int hitboxWidth, hitboxHeight;
	float damage;
	int type;
	int dir;
	int r;
	int g;
	int b;
	bool projectileTravel;
	bool projectileHit;
	ALLEGRO_BITMAP* spriteBitmap;
};

struct tile {
	//ALLEGRO_BITMAP* tileSprite;
	bool isSolid;
	int id;
};

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_FONT* font = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_SAMPLE* sample = NULL;
ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;
ALLEGRO_EVENT_QUEUE* queue = NULL;
ALLEGRO_BITMAP* playerShotTemplate;
ALLEGRO_BITMAP* enemyShotTemplate;
ALLEGRO_BITMAP* stage[3];
ALLEGRO_BITMAP* playerSprites[2];
ALLEGRO_BITMAP* enemySprite;
ALLEGRO_BITMAP* enemyShooterSprite;
ALLEGRO_BITMAP* tileAtlas;
ALLEGRO_FILE* txtmap;
ALLEGRO_TRANSFORM camera;
struct sprite player;
struct projectile playerShot[projectileMax];
struct sprite enemy[enemyMax];
struct projectile enemyShot[enemyProjectileMax];
struct sprite enemyShooter;
struct tile tiles[2];

int initialize() {

	sodium_init();

	al_init();
	al_init_image_addon();
	al_install_audio();
	al_init_acodec_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_get_default_mixer();
	al_init_primitives_addon();

	timer = al_create_timer(1.0 / FPS);
	al_set_new_display_flags(ALLEGRO_RESIZABLE);
	display = al_create_display(windowWidth, windowHeight);
	queue = al_create_event_queue();
	font = al_load_font("Fonts/metal-slug.ttf", 13, 0);
	al_set_window_title(display, "Metal Slug 5");
	al_reserve_samples(1);
	sample = al_load_sample("Audio/bg_music.ogg");
	playerShotTemplate = al_load_bitmap("Img/Tiro.bmp");
	al_convert_mask_to_alpha(playerShotTemplate, al_map_rgb(255, 0, 255));

	al_install_keyboard();
	al_install_mouse();

	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());

	al_start_timer(timer);

	return 0;
}

float absF(float* x) {
	if (*x < 0) {
		*x = -1 * (*x);
	}
	return *x;
}

void setSpriteColor(struct sprite* a) {
	switch (a->selectedWeapon) {
	case antiVirus:
		a->r = 0;
		a->g = 255;
		a->b = 0;
		break;
	case antiBiotic:
		a->r = 255;
		a->g = 0;
		a->b = 255;
		break;
	case antiMycotic:
		a->r = 0;
		a->g = 0;
		a->b = 255;
		break;
	}
}

void setProjectileColor(struct projectile* a) {
	switch (a->type) {
	case antiVirus:
		a->r = 0;
		a->g = 255;
		a->b = 0;
		break;
	case antiBiotic:
		a->r = 255;
		a->g = 0;
		a->b = 255;
		break;
	case antiMycotic:
		a->r = 0;
		a->g = 0;
		a->b = 255;
		break;
	}
}

void enemyRandomizer(struct sprite* e) {
	int section;
	int lifeRandomizer = randombytes_uniform(16);

	section = randombytes_uniform(2);

	if (section == 1) {
		e->x = player.x + (100 + randombytes_uniform(201));
	}
	else {
		e->x = player.x - (100 + randombytes_uniform(201));
	}

	e->y = 150 + randombytes_uniform(101);

	e->selectedWeapon = randombytes_uniform(3);

	setSpriteColor(e);

	e->life = targetPracticeLife + lifeRandomizer;
	e->maxLife = targetPracticeLife + lifeRandomizer;

	e->alive = true;
}

//void enemyShooterRandomizer(struct sprite* f) {
//	int section;
//	int lifeRandomizer = randombytes_uniform(16);
//
//	section = randombytes_uniform(2);
//
//	if (section == 1) {
//		f->x = 550 + randombytes_uniform(201);
//	}
//	else {
//		f->x = 50 + randombytes_uniform(201);
//	}
//
//	f->y = 250 + randombytes_uniform(101);
//
//	f->selectedWeapon = randombytes_uniform(3);
//
//	setSpriteColor(f);
//
//	f->life = targetPracticeLife + lifeRandomizer;
//	f->maxLife = targetPracticeLife + lifeRandomizer;
//
//	f->alive = true;
//}

int initplayer(struct sprite* c, ALLEGRO_BITMAP* player[]) {
	player[neutral] = al_load_bitmap("Img/alienNeutral64.bmp");
	//if (!c->spriteBitmap) {
	//	fprintf(stderr, "Falha ao carregar imagem!\n");
	//	return -1;
	//}
	al_convert_mask_to_alpha(player[neutral], al_map_rgb(255, 0, 255));

	player[shooting] = al_load_bitmap("Img/alienShot.bmp");
	al_convert_mask_to_alpha(player[shooting], al_map_rgb(255, 0, 255));

	c->x = 8 * tileSize;
	c->y = 7 * tileSize;
	c->width = al_get_bitmap_width(player[neutral]);
	c->height = al_get_bitmap_height(player[neutral]);
	c->hitboxWidth = 2 * tileSize;
	c->hitboxHeight = 2 * tileSize;
	c->vel_x = 4.5;
	c->dir = 5;
	c->currentDir = Right;
	c->selectedWeapon = 0;
	c->onGround = false;
	c->hitCeiling = false;

	return 0;
}

int initenemy(struct sprite e[], ALLEGRO_BITMAP** enemy) {
	int i;
	*enemy = al_load_bitmap("Img/miniufo.bmp");
	al_convert_mask_to_alpha(*enemy, al_map_rgb(255, 0, 255));

	e[contact].type = contact;
	e[shooter].type = shooter;

	for (i = 0; i < enemyMax; i++) {
		e[i].shotFC = 0;
		enemyRandomizer(&e[i]);

		e[i].vel_x = 0;
		e[i].vel_y = 0;

		e[i].width = 44;
		e[i].height = 38;

		e[i].hitboxWidth = 44;
		e[i].hitboxHeight = 38;
	}

	return 0;
}

//int initenemyShooter(struct sprite* f, ALLEGRO_BITMAP* enemyShooter) {
//	enemyShooterSprite = al_load_bitmap("Img/miniufo.bmp");
//	al_convert_mask_to_alpha(enemyShooterSprite, al_map_rgb(255, 0, 255));
//
//	enemyRandomizer(f);
//
//	f->vel_x = 0;
//	f->vel_y = 0;
//
//	f->width = 44;
//	f->height = 38;
//
//	f->hitboxWidth = 44;
//	f->hitboxHeight = 38;
//
//	return 0;
//}

void pShoot(struct projectile* p, struct sprite* c) {
	c->sprite = shooting;

	p->speed = projectileVelocity;
	p->accel = projectileAccel;

	p->width = 46;
	p->height = 22;

	p->hitboxWidth = 46;
	p->hitboxHeight = 22;

	p->dir = c->currentDir;

	if (p->dir == Right) {
		p->x0 = c->x + c->hitboxWidth;
		p->y0 = c->y + projectileOffset;
	}
	else {
		p->x0 = c->x - p->hitboxWidth;
		p->y0 = c->y + projectileOffset;
	}

	p->damage = projectileDamage;
	p->type = c->selectedWeapon;

	p->x = p->x0;
	p->y = p->y0;

	c->isShooting = true;
	p->projectileTravel = true;
}

void eShoot(struct projectile* p, struct sprite* e, struct sprite* c, int fc) {
	/*p->spriteBitmap = al_load_bitmap("Img/alienShot.bmp");
	al_convert_mask_to_alpha(p->spriteBitmap, al_map_rgb(255, 0, 255));*/
	e->shotFC = fc;
	p->speed = enemyProjectileVelocity;

	p->width = 20;
	p->height = 20;

	p->hitboxWidth = 20;
	p->hitboxHeight = 20;

	p->dir = e->currentDir;

	p->angle = atan2((-1.0 * e->y) + (-1.0 * c->hbY), (double)e->x + c->hbX);
	p->angle = p->angle * 180.0 / PI;
	p->angle = (float)p->angle;
	p->cos = cos(p->angle);
	p->sin = sin(p->angle);
	absF(&p->cos);
	absF(&p->sin);
	//absD(&angle);

	if (p->dir == Right) {
		p->x0 = e->x;// + e->hitboxWidth;
		p->y0 = e->y;// + projectileOffset;
	}
	else {
		p->x0 = e->x;// - p->hitboxWidth;
		p->y0 = e->y;// + projectileOffset;
	}

	p->damage = projectileDamage;
	p->type = NULL;// e->selectedWeapon;

	p->x = p->x0;
	p->y = p->y0;

	p->projectileTravel = true;
}

void refreshProjectileState(struct projectile p[], int* pCount, int cx) {
	int j;

	for (j = 0; j < projectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].speed <= 20) {
				p[j].speed += p[j].accel;
			}

			if (p[j].x < cx + al_get_display_width(display) && p[j].x > cx) {
				if (p[j].dir == Right)
					p[j].x += p[j].speed;
				else
					p[j].x -= p[j].speed;
			}
			else {
				p[j].projectileTravel = false;
				*pCount -= 1;
			}
		}
	}
}

void refreshPlayerMovement(struct sprite* p, struct tile t[], int m[mapSize / 2][mapSize / 2]) {
	int botTile, btID, upTile, upID, ltTile, ltID, rtTile, rtID;

	p->hbX = (p->x + (p->width / 2)) - tileSize;
	p->hbY = (p->y + (p->height / 2)) - tileSize;

	p->x0 = p->x;
	p->y0 = p->y;

	p->tileX = p->hbX / tileSize;
	p->tileY = p->hbY / tileSize;

	botTile = p->tileY + 2;
	btID = m[botTile][p->tileX + 1];

	upTile = p->tileY;
	upID = m[upTile][p->tileX + 1];

	ltTile = p->tileX;
	ltID = m[p->tileY + 1][ltTile];

	rtTile = p->tileX + 2;
	rtID = m[p->tileY + 1][rtTile];

	if (t[upID].isSolid && p->y >= upTile + tileSize && !p->hitCeiling && p->vel_y < 0) {
		p->vel_y = 0;
		p->hitCeiling = true;
	}

	if (t[btID].isSolid && p->vel_y >= 0 && p->y >= botTile - tileSize) {
		p->onGround = true;
		p->hitCeiling = false;
		p->vel_y = 0;
		p->y -= (int)p->y % tileSize;
	}
	else {
		p->vel_y += gravity;
		p->y += p->vel_y;
	}

	switch (p->dir) {
	case Right:
		if (!t[rtID].isSolid) {
			player.x += player.vel_x;
		}
		else {
			p->x = ((rtTile - 2.5) * tileSize) + 1;
		}
		break;
	case Left:
		if (!t[ltID].isSolid) {
			player.x -= player.vel_x;
		}
		else {
			p->x = ((ltTile + 0.5) * tileSize) - 1;
		}
		break;
	}
}

void refreshEnemyProjectile(struct projectile p[], int* pCount, int cx) {
	int j;

	for (j = 0; j < enemyProjectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].x < cx + al_get_display_width(display) && p[j].x > cx && p[j].y < windowHeight + p[j].height && p[j].y > 0) {
				if (p[j].dir == Right) {
					p[j].x += p[j].speed * -p[j].cos;
				}
				else {
					p[j].x += p[j].speed * p[j].cos;
				}
				p[j].y += p[j].speed * p[j].sin;
			}
			else {
				p[j].projectileTravel = false;
				*pCount -= 1;
			}
		}
	}
}
void refreshEnemyMovement(struct sprite* e, struct sprite* p) {
	if (e->x != p->hbX && e->alive) {
		if (e->x > p->hbX) {
			if (e->vel_x >= -1.8) {
				e->vel_x -= 0.2;
			}
		}
		else {
			if (e->vel_x <= 1.8) {
				e->vel_x += 0.2;
			}
		}

		e->x += e->vel_x;// *cosPE;
	}

	if (e->y != p->hbY && e->alive) {
		if (e->y > p->hbY) {
			if (e->vel_y >= -1.8) {
				e->vel_y -= 0.2;
			}
		}
		else {
			if (e->vel_y <= 1.8) {
				e->vel_y += 0.2;
			}
		}
		e->y += e->vel_y; //*sinPE;
	}

	if (p->hbX + p->hitboxWidth - 10 > e->x && p->hbX < e->x + e->width - 10 && p->hbY + p->hitboxHeight + 10 > e->y && p->hbY < e->y + e->height - 10) {
		int i;
		for (i = 0; i > -8; i--) {
			if (e->vel_y > -8) {
				e->vel_y--;
			}
		}
	}
}

void refreshCamera(float* cx, /*float* cy,*/ struct sprite p) {
	*cx = (player.x + player.width / 2) - al_get_display_width(display) / 2;

	if (*cx < 0) {
		*cx = 0;
	}
}

void hitboxDetection(struct projectile* a, struct sprite b[], int* hitI[], int* hitCount, int* pCount) {
	float xAxisPivotA, yAxisPivotA, xAxisPivotB, yAxisPivotB, rightA, leftA, downA, upA, rightB, leftB, downB, upB;
	int i, j;
	for (j = 0; j < enemyMax; j++) {
		for (i = 0; i < projectileMax; i++) {
			if (a[i].projectileTravel) {
				xAxisPivotA = a[i].x + a[i].width / 2;
				yAxisPivotA = a[i].y + a[i].height / 2;
				xAxisPivotB = b[j].x + b[j].width / 2;
				yAxisPivotB = b[j].y + b[j].height / 2;

				rightA = xAxisPivotA + a[i].hitboxWidth / 2;
				leftA = xAxisPivotA - a[i].hitboxWidth / 2;
				downA = yAxisPivotA + a[i].hitboxHeight / 2;
				upA = yAxisPivotA - a[i].hitboxHeight / 2;

				rightB = xAxisPivotB + b[j].hitboxWidth / 2;
				leftB = xAxisPivotB - b[j].hitboxWidth / 2;
				downB = yAxisPivotB + b[j].hitboxHeight / 2;
				upB = yAxisPivotB - b[j].hitboxHeight / 2;

				if ((rightA > leftB && rightA < rightB) || (leftA > leftB && leftA < rightB)) {
					if ((upA < downB && upA > upB) || (downA > upB && downA < downB)) {
						a[i].projectileTravel = false;
						a[i].x = 0;
						a[i].y = 0;
						*hitCount += 1;
						*pCount -= 1;
						hitI[projectileI] = i;
						hitI[enemyI] = j;
					}
				}
			}
		}
	}
}

void exitGame(ALLEGRO_EVENT ev, bool* loop, bool* exit) {
	switch (ev.type) {
	case ALLEGRO_EVENT_DISPLAY_CLOSE:
		*loop = false;
		break;
	case ALLEGRO_EVENT_KEY_DOWN:
		if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
			*loop = false;
			*exit = true;
		}
		break;
	}
}

void createTileAtlas(void) {
	tileAtlas = al_create_bitmap(64, 32);
	al_set_target_bitmap(tileAtlas);
	al_clear_to_color(al_map_rgb(0, 0, 0));
	al_draw_rectangle(0, 0, 34, 34, al_map_rgb(255, 0, 0), 3);
	al_draw_filled_rectangle(34, 0, 68, 34, al_map_rgb(255, 0, 0));
	al_set_target_backbuffer(display);
}

void createTileSet(int* tileSet[]) {
	ALLEGRO_FILE* file = al_fopen("Tiles/tilemap.txt", "r");
	int i, j;

	for (i = 0; i < (mapSize / 2); i++) {
		for (j = 0; j < (mapSize / 2); j++) {
			fscanf_s(file, "%d", &tileSet[i][j]);
		}
	}
}

int main() {
	int i, j, projectileCount = 0, enemyProjectileCount = 0, enemyDmgGauge = 0, hit = 0, hitI[2] = { 0, 0 }, hitII = 0, frameCount = 0, enemyDeadFC[enemyMax] = { 0, 0 }, killCount = 0;
	int tileset[(mapSize / 2)][(mapSize / 2)] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
	};

	//float anglePE, cosPE, sinPE;
	float cx = 0, cy = 0, w = 0, h = 0, anglePE, cosPE, sinPE;
	char enemyLifeGauge[5], ptx[8], pty[8], kcText[15];
	bool gameLoop = false, menuLoop = true, toggleStartText = false, exit = false;

	initialize();
	initplayer(&player, &playerSprites);
	initenemy(&enemy, &enemySprite);
	//initenemyShooter(&enemyShooter, &enemyShooterSprite);
	createTileAtlas();
	//createTileSet(&tileset);
	//txtmap = al_fopen("Tiles/tilemap.txt", "r");
	//for (i = 0; i < (mapSize / 2); i++) {
	//	for (j = 0; j < (mapSize / 2); j++) {
	//		fscanf_s(txtmap, "%d", &tileset/*[i][j]*/);
	//	}
	//}
	tiles[ground].isSolid = true;
	tiles[ground].id = ground;

	tiles[air].isSolid = false;
	tiles[air].id = air;

	stage[backgroundL1] = al_load_bitmap("Img/backgroundLayer1.bmp");
	stage[backgroundL2] = al_load_bitmap("Img/backgroundLayer2.bmp");
	stage[foreground] = al_load_bitmap("Img/foreground.bmp");
	al_convert_mask_to_alpha(stage[foreground], al_map_rgb(255, 0, 255));

	al_clear_to_color(al_map_rgb(255, 255, 255));

	while (!exit) {
		while (menuLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(queue, &event);

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				if (event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
					menuLoop = false;
					gameLoop = true;
				}
			}

			if (event.type == ALLEGRO_EVENT_TIMER) {
				frameCount++;
			}
			exitGame(event, &menuLoop, &exit);

			if (al_is_event_queue_empty(queue)) {
				al_clear_to_color(al_map_rgb(0, 0, 0));
				if (frameCount % (FPS / 2) == 0) {
					toggleStartText = !toggleStartText;
				}
				if (toggleStartText) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, windowHeight / 2, ALLEGRO_ALIGN_CENTER, "Press space to start mission!!");
				}
				al_flip_display();
			}

		}

		al_play_sample(sample, 0.1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);

		while (gameLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(queue, &event);

			if (event.type == ALLEGRO_EVENT_TIMER) {
				refreshPlayerMovement(&player, &tiles, tileset);
				refreshEnemyMovement(&enemy, &player);
				refreshProjectileState(playerShot, &projectileCount, cx);
				refreshEnemyProjectile(enemyShot, &enemyProjectileCount, cx);

				refreshCamera(&cx, player);

				if (player.isShooting) {
					if (player.spriteChange >= FPS / 2) {
						player.spriteChange = 0;
						player.sprite = neutral;
						player.isShooting = false;
					}
					else player.spriteChange++;
				}

				/*if (event.type == ALLEGRO_EVENT_TIMER) {
					if (enemyProjectileCount < projectileMax) {
						for (i = 0; i < projectileMax; i++) {
							if (!enemyShot[i].projectileTravel) {
								enemyProjectileCount++;
								Shooting(&enemyShot[i], &enemyShooter, &player);
								break;
							}
						}
					}
				}*/

				for (i = 0; i < enemyProjectileMax; i++) {
					if (frameCount - enemy[i].shotFC >= FPS && enemy[i].type == shooter) {
						if (!enemyShot[i].projectileTravel) {
							enemyProjectileCount++;
							eShoot(&enemyShot[i], &enemy[i], &player, frameCount);
							break;
						}
					}
				}

				hitboxDetection(playerShot, &enemy, &hitI, &hit, &projectileCount);

				while (hit > 0) {
					if (enemy[hitI[enemyI]].selectedWeapon == playerShot[hitI[projectileI]].type) {
						enemy[hitI[enemyI]].life -= playerShot[0].damage + playerShot[0].damage * (randombytes_uniform(16) / 100.0);
					}
					hit--;

					if (enemy[hitI[enemyI]].life <= 0) {
						enemy[hitI[enemyI]].alive = false;
						enemy[hitI[enemyI]].y = 0;
						enemy[hitI[enemyI]].vel_x = 0;
						enemy[hitI[enemyI]].vel_y = 0;
						enemy[hitI[enemyI]].life = 0;
						enemyDeadFC[hitI[enemyI]] = frameCount;
						killCount++;
					}
				}

				for (i = 0; i < enemyMax; i++) {
					if (!enemy[i].alive && frameCount - enemyDeadFC[hitI[enemyI]] >= FPS) {
						enemyRandomizer(&enemy[hitI[enemyI]]);
						enemyDmgGauge = 0;
						enemy[i].alive = true;
					}
				}
				/*while (hit > 0) {
					if (enemyShooter.selectedWeapon == playerShot[hitI].type) {
						enemyShooter.life -= playerShot[0].damage + playerShot[0].damage * (randombytes_uniform(16) / 100.0);
					}
					hit--;
					if (enemyShooter.life <= 0) {
						enemyShooter.alive = false;
						enemyShooter.y = 0;
						enemyShooter.vel_x = 0;
						enemyShooter.vel_y = 0;
						enemyShooter.life = 0;
						auxFrameCount = frameCount;
						killCount++;
					}
				}*/


				/*if (!enemyShooter.alive && frameCount - auxFrameCount >= 60) {
					enemyShooterRandomizer(&enemyShooter);
					enemyDmgGauge = 0;
					enemyShooter.alive = true;
				}*/


				//anglePE = atan2((-1.0 * enemy.y) - (-1.0 * player.y), (double) enemy.x - player.x);
				//anglePE = anglePE * 180.0 / PI;
				//anglePE = (float) anglePE;
				//cosPE = cos(anglePE);
				//sinPE = sin(anglePE);
				//absF(&cosPE);
				//absF(&sinPE);
				//absD(&anglePE);

				//if (enemyShooter.x != player.x && enemyShooter.alive) {
				//	if (enemyShooter.x > player.x) {
				//		if (enemyShooter.vel_x >= -1.6) {
				//			enemyShooter.vel_x -= 0.2;
				//		}
				//	}
				//	else {
				//		if (enemyShooter.vel_x <= 1.6) {
				//			enemyShooter.vel_x += 0.2;
				//		}
				//	}

				//	enemyShooter.x += enemyShooter.vel_x;// *cosPE;
				//}

				//if (enemyShooter.y != player.y && enemyShooter.alive) {
				//	if (enemyShooter.y > player.y) {
				//		if (enemyShooter.vel_y >= -1.6) {
				//			enemyShooter.vel_y -= 0.2;
				//		}
				//	}
				//	else {
				//		if (enemyShooter.vel_y <= 1.6) {
				//			enemyShooter.vel_y += 0.2;
				//		}
				//	}
				//	enemyShooter.y += enemyShooter.vel_y; //*sinPE;
				//}

				//if (player.x + player.width - 10 > enemyShooter.x && player.x < enemyShooter.x + enemyShooter.width - 10 && player.y + player.height + 10 > enemyShooter.y && player.y < enemyShooter.y + enemyShooter.height - 10) {
				//	int i;
				//	for (i = 0; i > -8; i--) {
				//		if (enemyShooter.vel_y > -8) {
				//			enemyShooter.vel_y--;
				//		}
				//	}
				//}

				if (enemyDmgGauge < enemy[hitI[enemyI]].maxLife - enemy[hitI[enemyI]].life && frameCount % 2 == 0) {
					enemyDmgGauge++;
				}

				setSpriteColor(&player);

				frameCount++;
				al_flip_display();
			}

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
					if (player.onGround) {
						player.vel_y = -8;
						player.y--;
						player.onGround = false;
					}
					break;
				case ALLEGRO_KEY_LEFT:
					player.dir = Left;
					player.currentDir = Left;
					break;

				case ALLEGRO_KEY_RIGHT:
					player.dir = Right;
					player.currentDir = Right;
					break;

				case ALLEGRO_KEY_A:
					player.selectedWeapon--;
					if (player.selectedWeapon < 0) {
						player.selectedWeapon = 2;
					}
					break;

				case ALLEGRO_KEY_S:
					player.selectedWeapon++;
					player.selectedWeapon = player.selectedWeapon % 3;
					break;

				case ALLEGRO_KEY_X:
					if (projectileCount < projectileMax) {
						for (i = 0; i < projectileMax; i++) {
							if (!playerShot[i].projectileTravel) {
								projectileCount++;
								player.spriteChange = 0;
								pShoot(&playerShot[i], &player);
								break;
							}
						}
					}
					break;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_RIGHT:
					if (player.dir == Right) {
						player.dir = 5;
					}
					break;
				case ALLEGRO_KEY_LEFT:
					if (player.dir == Left) {
						player.dir = 5;
					}
					break;
				}
			}

			exitGame(event, &gameLoop, &exit);

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
			}

			if (al_is_event_queue_empty(queue)) {
				al_clear_to_color(al_map_rgb(255, 255, 255));

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.05, -cy * 0.05);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL1], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL1], al_get_bitmap_width(stage[backgroundL1]), 0, ALLEGRO_FLIP_HORIZONTAL);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.35, -cy * 0.35);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL2], 0, 240, 0);
				al_draw_bitmap(stage[backgroundL2], al_get_bitmap_width(stage[backgroundL2]), 240, 0);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx, -cy);
				al_use_transform(&camera);

				al_draw_bitmap(stage[foreground], 0, 226, 0);
				al_draw_bitmap(stage[foreground], al_get_bitmap_width(stage[foreground]), 226, ALLEGRO_FLIP_HORIZONTAL);
				al_draw_bitmap(stage[foreground], 2 * al_get_bitmap_width(stage[foreground]), 226, 0);

				for (i = 0; i < (mapSize / 2); i++) {
					for (j = 0; j < (mapSize / 2); j++) {
						switch (tileset[i][j]) {
						case ground:
							al_draw_filled_rectangle(j * tileSize, i * tileSize, j * tileSize + tileSize, i * tileSize + tileSize, al_map_rgb(255, 0, 0));
							break;
						case air:
							al_draw_rectangle(j * tileSize, i * tileSize, j * tileSize + tileSize, i * tileSize + tileSize, al_map_rgb(255, 0, 0), 2);
							break;
						case roof:
							break;
						}
					}
				}

				/*if (enemyShooter.alive) {
					if (enemyShooter.x < player.x) {
						al_draw_tinted_bitmap(enemyShooterSprite, al_map_rgb(enemy.r, enemy.g, enemy.b), enemyShooter.x, enemyShooter.y, ALLEGRO_FLIP_HORIZONTAL);
					}
					else {
						al_draw_tinted_bitmap(enemyShooterSprite, al_map_rgb(enemy.r, enemy.g, enemy.b), enemyShooter.x, enemyShooter.y, 0);
					}
				}*/

				if (player.currentDir == Right) {
					al_draw_bitmap(playerSprites[player.sprite], player.x, player.y, ALLEGRO_FLIP_HORIZONTAL);
					al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hitboxWidth, player.hbY + player.hitboxHeight, al_map_rgba(0, 0, 255, 50));
				}
				else {
					al_draw_bitmap(playerSprites[player.sprite], player.x, player.y, 0);
					al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hitboxWidth, player.hbY + player.hitboxHeight, al_map_rgba(0, 0, 255, 50));
				}

				for (i = 0; i < enemyMax; i++) {
					if (enemy[i].alive) {
						if (enemy[i].x < player.x) {
							al_draw_tinted_bitmap(enemySprite, al_map_rgb(enemy[i].r, enemy[i].g, enemy[i].b), enemy[i].x, enemy[i].y, ALLEGRO_FLIP_HORIZONTAL);
						}
						else {
							al_draw_tinted_bitmap(enemySprite, al_map_rgb(enemy[i].r, enemy[i].g, enemy[i].b), enemy[i].x, enemy[i].y, 0);
						}
					}
				}

				for (i = 0; i < projectileMax; i++) {
					if (playerShot[i].projectileTravel) {
						setProjectileColor(&playerShot[i]);
						if (playerShot[i].dir == Right) {
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].x, playerShot[i].y, ALLEGRO_FLIP_HORIZONTAL);
						}
						else {
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].x, playerShot[i].y, 0);
						}
					}
				}

				for (i = 0; i < enemyProjectileMax; i++) {
					if (enemyShot[i].projectileTravel) {
						al_draw_filled_circle(enemyShot[i].x, enemyShot[i].y, enemyShot[i].width / 2, al_map_rgb(255, 165, 0));
						al_draw_filled_circle(enemyShot[i].x, enemyShot[i].y, (enemyShot[i].width / 2) - 2, al_map_rgb(255, 255, 255));
					}
				}

				al_identity_transform(&camera);
				al_use_transform(&camera);

				al_draw_filled_rectangle(windowWidth - (2 * (enemy[hitI[enemyI]].maxLife - enemyDmgGauge) + 50), 50, windowWidth - (enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(255, 0, 0));
				al_draw_filled_rectangle(windowWidth - 50, 50, windowWidth - (2 * enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(0, 128, 0));
				sprintf_s(enemyLifeGauge, sizeof(enemyLifeGauge), "%.0f", enemy[hitI[enemyI]].life);
				al_draw_text(font, al_map_rgb(0, 0, 0), windowWidth - 48, 42, 0, enemyLifeGauge);
				sprintf_s(kcText, sizeof(kcText), "killcount = %d", killCount);
				al_draw_text(font, al_map_rgb(0, 0, 0), 10, 5, 0, kcText);
				sprintf_s(ptx, sizeof(ptx), "X = %d", player.tileX);
				al_draw_text(font, al_map_rgb(0, 0, 0), 10, 50, 0, ptx);
				sprintf_s(pty, sizeof(pty), "Y = %d", player.tileY);
				al_draw_text(font, al_map_rgb(0, 0, 0), 100, 50, 0, pty);

				al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(player.r, player.g, player.b), 0, 30, ALLEGRO_FLIP_HORIZONTAL);

				al_flip_display();
			}
		}
	}

	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_font(font);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(playerSprites[neutral]);
	al_destroy_bitmap(playerSprites[shooting]);
	al_destroy_bitmap(enemySprite);
	al_destroy_bitmap(enemyShooterSprite);
	al_destroy_bitmap(stage[backgroundL1]);
	al_destroy_bitmap(stage[backgroundL2]);
	al_destroy_bitmap(stage[foreground]);
	al_destroy_bitmap(playerShotTemplate);
	al_destroy_bitmap(enemyShotTemplate);
	al_destroy_sample(sample);

	return 0;
}

//font by everiux365