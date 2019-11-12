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
#define queueSize 5
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
#define helksonChupaRola 24

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
	air,
	ground,
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
enum {
	friendly,
	foe
};

// Helkson
#define Left 0
#define Right 1
#define gravity 0.275

typedef struct TQueue {
	int tamanho, inicio, fim, total;
	char fila[queueSize];
} queue;


typedef struct entity {
	float x, y;
	float x0, y0;
	int tileX, tileY;
	float width, height;
	float life, maxLife;
	int hbWidth, hbHeight;
	float hbX, hbY;
	int sprite;
	int spriteChange;
	int shotFC;
	int r;
	int g;
	int b;
	int selectedWeapon;
	int attack;
	int currentDir;
	bool isShooting;
	bool onGround;
	bool hitCeiling;
	bool alive;

	int dir;
	int count;
	float vel_x;
	float vel_y;
	bool immortality;
} entity;

struct sprite {
	// Felipe
	float x, y;
	float x0, y0;
	int tileX, tileY;
	float width, height;
	float life, maxLife;
	int hbWidth, hbHeight;
	float hbX, hbY;
	int sprite;
	int spriteChange;
	int shotFC;
	int r;
	int g;
	int b;
	int selectedWeapon;
	int attack;
	int currentDir;
	bool isShooting;
	bool onGround;
	bool hitCeiling;
	bool alive;

	int dir;
	int count;
	float vel_x;
	float vel_y;
	bool immortality;

};

typedef struct projectile {
	float x, y;
	float x0, y0;
	float width, height;
	float speed, accel;
	float angle, sin, cos;
	int hbWidth, hbHeight;
	float damage;
	int type;
	int dir;
	int r;
	int g;
	int b;
	int origin;
	bool projectileTravel;
	bool projectileHit;
} projectile;

typedef struct tile {
	//ALLEGRO_BITMAP* tileSprite;
	bool isSolid;
	int id;
} tile;

typedef struct objective {
	int count;
	int type;
	//int stage;
} objective;

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_FONT* font = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_SAMPLE* sample = NULL;
ALLEGRO_SAMPLE* shot = NULL;
ALLEGRO_SAMPLE* sfx_jump = NULL;
ALLEGRO_SAMPLE* sfx_sp1 = NULL;
ALLEGRO_SAMPLE* sfx_sp2 = NULL;
ALLEGRO_SAMPLE* sfx_sp3 = NULL;
ALLEGRO_SAMPLE* sfx_hit = NULL;
ALLEGRO_SAMPLE_INSTANCE* sampleInstance = NULL;
ALLEGRO_EVENT_QUEUE* evQueue = NULL;
ALLEGRO_BITMAP* playerShotTemplate;
ALLEGRO_BITMAP* enemyShotTemplate;
ALLEGRO_BITMAP* stage[3];
ALLEGRO_BITMAP* playerSprites[2];
ALLEGRO_BITMAP* enemySprite[3];
ALLEGRO_BITMAP* enemyShooterSprite;
ALLEGRO_BITMAP* tileAtlas;
ALLEGRO_FILE* txtmap;
ALLEGRO_TRANSFORM camera;
entity player;
projectile playerShot[projectileMax];
projectile playerTripleShot[projectileMax];
entity enemy[enemyMax];
projectile enemyShot[enemyProjectileMax];
entity enemyShooter;
tile tiles[2];
objective killCount;
objective endurance;

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
	evQueue = al_create_event_queue();
	font = al_load_font("Fonts/metal-slug.ttf", 13, 0);
	al_set_window_title(display, "Metal Slug 5");
	al_reserve_samples(4);
	
	sfx_sp1 = al_load_sample("Audio/spawn.wav");
	shot = al_load_sample("Audio/tiro.ogg");
	sfx_jump = al_load_sample("Audio/jump.wav");
	sample = al_load_sample("Audio/bg_music.ogg");
	playerShotTemplate = al_load_bitmap("Img/Tiro.bmp");
	al_convert_mask_to_alpha(playerShotTemplate, al_map_rgb(255, 0, 255));

	al_install_keyboard();
	al_install_mouse();

	al_register_event_source(evQueue, al_get_display_event_source(display));
	al_register_event_source(evQueue, al_get_timer_event_source(timer));

	al_register_event_source(evQueue, al_get_mouse_event_source());
	al_register_event_source(evQueue, al_get_keyboard_event_source());

	al_start_timer(timer);

	return 0;
}

void initQueue(queue* f) {
	int i;
	f->inicio = 0;
	f->fim = 0;
	f->total = 0;
	f->tamanho = queueSize;
	for (i = 0; i < queueSize; i++) {
		f->fila[i] = "";
	}
}

bool isQueueEmpty(queue* f) {
	if (f->total == 0) {
		return true;
	}
	else {
		return false;
	}
}

bool isQueueFull(queue* f) {
	if (f->total >= f->tamanho) {
		return true;
	}
	else {
		return false;
	}
}

void enqueue(queue* f, char val[]) {
	if (!isQueueFull(f)) {
		f->fila[f->fim] = val[0];
		f->fim = f->fim + 1;
		f->total = f->total + 1;
		if (f->fim >= f->tamanho) {
			f->fim = 0;
		}
	}
}

char dequeue(queue* f) {
	char x = -1;

	if (!isQueueEmpty(f)) {
		x = f->fila[f->inicio];
		f->inicio = f->inicio + 1;
		f->total = f->total - 1;
		if (f->inicio >= f->tamanho) {
			f->inicio = 0;
		}
	}
	return x;
}

char firstIn(queue* f) {
	return f->fila[f->inicio];
}

void viewQueue(queue* f) {
	int i, j = 0;

	for (i = f->inicio; j < f->total; i++) {
		if (i >= f->tamanho) {
			i = 0;
		}
		printf("Elemento %c posicao %d\n", f->fila[i], i);
		j++;
	}
}

float absF(float* x) {
	if (*x < 0) {
		*x = -1 * (*x);
	}
	return *x;
}

void setSpriteColor(entity* a) {
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

void setProjectileColor(projectile* a) {
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

void enemyRandomizer(entity* e) {
	int section;
	int lifeRandomizer = randombytes_uniform(16);

	section = randombytes_uniform(2);

	if (section == 1) {
		e->x = player.x + (100 + randombytes_uniform(201));
	}
	else {
		e->x = player.x - (100 + randombytes_uniform(201));
	}

	e->y = player.y + randombytes_uniform(101);

	e->selectedWeapon = randombytes_uniform(3);

	setSpriteColor(e);

	e->life = targetPracticeLife + lifeRandomizer;
	e->maxLife = targetPracticeLife + lifeRandomizer;

	e->alive = true;
}

int initplayer(entity* c, ALLEGRO_BITMAP* player[]) {
	int aux = 0;
	player[neutral] = al_load_bitmap("Img/canvas.png");
	//if (!c->spriteBitmap) {
	//	fprintf(stderr, "Falha ao carregar imagem!\n");
	//	return -1;
	//}
	al_convert_mask_to_alpha(player[neutral], al_map_rgb(0, 0, 0));

	player[shooting] = al_load_bitmap("Img/canvas.png");
	al_convert_mask_to_alpha(player[shooting], al_map_rgb(0, 0, 0));

	

	c->x = 50 * tileSize;
	c->y = 50 * tileSize;
	c->width = al_get_bitmap_width(player[neutral]);
	aux = c->width / tileSize;
	c->hbWidth = aux * tileSize;
	c->height = al_get_bitmap_height(player[neutral]);
	aux = c->height / tileSize;
	c->hbHeight = aux * tileSize;
	c->vel_x = 4.5;
	c->dir = 5;
	c->currentDir = Right;
	c->selectedWeapon = 0;
	c->onGround = false;
	c->hitCeiling = false;
	c->alive = true;
	c->immortality = false;
	c->life = 30;

	return 0;
}

int initenemy(entity e[], ALLEGRO_BITMAP** enemy) {
	int i, aux = 0;
	enemy[antiBiotic] = al_load_bitmap("Img/bacteria.png");
	al_convert_mask_to_alpha(enemySprite[antiBiotic], al_map_rgb(255, 0, 255));

	enemy[antiMycotic] = al_load_bitmap("Img/fungo.png");
	al_convert_mask_to_alpha(enemySprite[antiMycotic], al_map_rgb(255, 0, 255));

	enemy[antiVirus] = al_load_bitmap("Img/virus.png");
	al_convert_mask_to_alpha(enemySprite[antiVirus], al_map_rgb(255, 0, 255));

	e[contact].attack = contact;
	e[shooter].attack = shooter;

	for (i = 0; i < enemyMax; i++) {
		e[i].shotFC = 0;
		enemyRandomizer(&e[i]);

		e[i].vel_x = 0;
		e[i].vel_y = 0;

		e[i].width = al_get_bitmap_width(*enemy);
		aux = e[i].width / tileSize;
		e[i].hbWidth = aux * tileSize;
		e[i].height = al_get_bitmap_height(*enemy);
		aux = e[i].height / tileSize;
		e[i].hbHeight = aux * tileSize;
	}

	return 0;
}

void pShoot(projectile* p, entity* c) {
	c->sprite = shooting;

	p->speed = projectileVelocity;
	p->accel = projectileAccel;

	p->width = 46;
	p->height = 22;

	p->hbWidth = 46;
	p->hbHeight = 22;

	p->origin = friendly;

	p->dir = c->currentDir;

	if (p->dir == Right) {
		p->x0 = c->x + c->hbWidth;
		p->y0 = c->y + projectileOffset;
	}
	else {
		p->x0 = c->x - p->hbWidth;
		p->y0 = c->y + projectileOffset;
	}

	p->damage = projectileDamage;
	p->type = c->selectedWeapon;

	p->x = p->x0;
	p->y = p->y0;

	c->isShooting = true;
	p->projectileTravel = true;
}

void tripleShot(projectile* p, entity* c) {
	int i;
	
	c->sprite = shooting;

	p->speed = 0.6;
	p->accel = 0.1;

	p->width = 46;
	p->height = 22;

	p->hbWidth = 46;
	p->hbHeight = 22;

	p->origin = friendly;

	p->dir = c->currentDir;

	if (p->dir == Right) {
		p->x0 = c->x + c->hbWidth;
		p->y0 = c->y + projectileOffset;
	}
	else {
		p->x0 = c->x - p->hbWidth;
		p->y0 = c->y + projectileOffset;
	}

	p->damage = 2;
	p->type = c->selectedWeapon;

	p->x = p->x0;
	p->y = p->y0;

	p->projectileTravel = true;
	c->isShooting = true;
}

void eShoot(projectile* p, entity* e, entity* c, int fc) {
	/*p->spriteBitmap = al_load_bitmap("Img/alienShot.bmp");
	al_convert_mask_to_alpha(p->spriteBitmap, al_map_rgb(255, 0, 255));*/
	e->shotFC = fc;
	p->speed = enemyProjectileVelocity;

	p->width = 20;
	p->height = 20;

	p->hbWidth = 20;
	p->hbHeight = 20;

	p->origin = foe;

	p->dir = e->currentDir;

	/*p->angle = atan2((-1.0 * e->y) + (-1.0 * c->hbY), (double)e->x + c->hbX);
	p->angle = p->angle * 180.0 / PI;
	p->angle = (float)p->angle;
	p->cos = cos(p->angle);
	p->sin = sin(p->angle);
	absF(&p->cos);
	absF(&p->sin);
	//absD(&angle);*/

	p->angle = atan2((double)e->y - c->y, (double)c->x - e->x);
	p->angle = p->angle * 180.0 / PI;
	p->angle = (float)p->angle;
	p->cos = cos(p->angle);
	p->sin = sin(p->angle);
	//absF(&p->cos);
	//absF(&p->sin);

	if (p->dir == Right) {
		p->x0 = e->x;// + e->hbWidth;
		p->y0 = e->y;// + projectileOffset;
	}
	else {
		p->x0 = e->x;// - p->hbWidth;
		p->y0 = e->y;// + projectileOffset;
	}

	p->damage = 0;// projectileDamage;
	p->type = 10;

	p->x = p->x0;
	p->y = p->y0;

	p->projectileTravel = true;
}

void refreshProjectileState(projectile p[], projectile e[], entity c, int* cpCount, int* epCount, int cx, int cy) {
	int j;

	for (j = 0; j < projectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].x < cx + al_get_display_width(display) && p[j].x > cx && p[j].y < cy + al_get_display_height(display) && p[j].y > cy) {
				if (p[j].dir == Right)
					p[j].x += p[j].speed;
				else
					p[j].x -= p[j].speed;
			}
			else {
				p[j].projectileTravel = false;
				*cpCount -= 1;
			}
			if (p[j].speed <= 20) {
				p[j].speed += p[j].accel;
			}
		}
	}

	for (j = 0; j < enemyProjectileMax; j++) {
		if (e[j].projectileTravel) {
			if (e[j].x < cx + al_get_display_width(display) && e[j].x > cx && e[j].y < cy + al_get_display_height(display) && e[j].y > cy) {
				if (e[j].dir == Right) {
					e[j].x += e[j].speed * -e[j].cos;
				}
				else {
					e[j].x += e[j].speed * e[j].cos;
				}
				e[j].y += e[j].speed * e[j].sin;
			}
			else {
				e[j].projectileTravel = false;
				*epCount -= 1;
			}
		}
	}
}

void refreshPlayerMovement(entity* p, tile t[], int** m) {
	int botTile, btID, upTile, upID, ltTile, ltID, rtTile, rtID;

	p->hbX = (p->x + (p->width / 2)) - p->hbWidth / 2;
	p->hbY = (p->y + (p->height - p->hbHeight));

	p->x0 = p->x;
	p->y0 = p->y;

	p->tileX = p->hbX / tileSize;
	p->tileY = p->hbY / tileSize;

	botTile = p->tileY + (p->hbHeight / tileSize);
	btID = m[botTile][p->tileX + 1];

	upTile = p->tileY;
	upID = m[upTile][p->tileX + 1];

	ltTile = p->tileX;
	ltID = m[p->tileY][ltTile];

	rtTile = p->tileX + (p->hbWidth / tileSize);
	rtID = m[p->tileY][rtTile];

	switch (p->dir) {
	case Right:
		if (!t[rtID].isSolid) {
			player.x += player.vel_x;
		}
		/*else {
			p->x = ((rtTile - 2.5) * tileSize) + 1;
		}*/
		break;
	case Left:
		if (!t[ltID].isSolid) {
			player.x -= player.vel_x;
		}
		/*else {
			p->x = ((ltTile + 0.5) * tileSize) - 1;
		}*/
		break;
	}

	if (t[upID].isSolid && p->y >= upTile + tileSize && !p->hitCeiling && p->vel_y < 0) {
		p->vel_y = 1;
		p->hitCeiling = true;
	}

	if (t[btID].isSolid && p->vel_y >= 0 && p->y >= botTile - tileSize) {
		p->onGround = true;
		p->hitCeiling = false;
		p->vel_y = 0;
		p->hbY -= (int)p->hbY % (tileSize * (p->hbHeight / tileSize));
		p->y = p->hbY - (p->height - p->hbHeight);
	}
	else {
		p->onGround = false;
		p->vel_y += gravity;
		p->y += p->vel_y;
	}
}

/*void refreshEnemyProjectile(projectile p[], int* pCount, int cx) {
	int j;

	for (j = 0; j < enemyProjectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].x < cx + al_get_display_width(display) && p[j].x > cx&& p[j].y < windowHeight + p[j].height && p[j].y > 0) {
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
}*/

void refreshEnemyMovement(entity* e, entity* p) {
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
}

void refreshCamera(float* cx, float* cy, entity p) {
	*cx = (player.x + player.width / 2) - al_get_display_width(display) / 2;
	*cy = (player.y + player.height / 2) - al_get_display_height(display) / 2;

	if (*cx < 0) {
		*cx = 0;
	}
	if (*cy < 0) {
		*cy = 0;
	}
}

void hitboxDetection(projectile* a, entity e[], entity* p, objective* kc, int* hitI, int* pCount, int* iFC, int* edFC, int* fc) {
	float xAxisPivotA, yAxisPivotA, xAxisPivotB, yAxisPivotB, rightA, leftA, downA, upA, rightB, leftB, downB, upB;
	int i, j;
	if (a[0].origin == friendly) {
		for (j = 0; j < enemyMax; j++) {
			for (i = 0; i < projectileMax; i++) {
				if (a[i].projectileTravel) {
					xAxisPivotA = a[i].x + a[i].width / 2;
					yAxisPivotA = a[i].y + a[i].height / 2;
					xAxisPivotB = e[j].x + e[j].width / 2;
					yAxisPivotB = e[j].y + e[j].height / 2;

					rightA = xAxisPivotA + a[i].hbWidth / 2;
					leftA = xAxisPivotA - a[i].hbWidth / 2;
					downA = yAxisPivotA + a[i].hbHeight / 2;
					upA = yAxisPivotA - a[i].hbHeight / 2;

					rightB = xAxisPivotB + e[j].hbWidth / 2;
					leftB = xAxisPivotB - e[j].hbWidth / 2;
					downB = yAxisPivotB + e[j].hbHeight / 2;
					upB = yAxisPivotB - e[j].hbHeight / 2;

					if ((rightA > leftB && rightA < rightB) || (leftA > leftB && leftA < rightB)) {
						if ((upA < downB && upA > upB) || (downA > upB && downA < downB)) {
							a[i].projectileTravel = false;
							a[i].x = 0;
							a[i].y = 0;

							if (e[j].selectedWeapon == playerShot[i].type) {
								e[j].life -= playerShot[i].damage + playerShot[i].damage * (randombytes_uniform(16) / 100.0);
							}

							if (e[j].life <= 0) {
								e[j].alive = false;
								e[j].y = 0;
								e[j].vel_x = 0;
								e[j].vel_y = 0;
								e[j].life = 0;
								edFC[j] = *fc;
								if (e[j].attack == kc->type) {
									kc->count -= 1;
								}
							}
							*pCount -= 1;
							hitI[projectileI] = i;
							hitI[enemyI] = j;
						}
					}
				}
			}
		}
	}
	else {
		for (i = 0; i < enemyProjectileMax; i++) {
			if (a[i].projectileTravel) {
				xAxisPivotA = a[i].x + a[i].width / 2;
				yAxisPivotA = a[i].y + a[i].height / 2;
				xAxisPivotB = p->x + p->width / 2;
				yAxisPivotB = p->y + p->height / 2;

				rightA = xAxisPivotA + a[i].hbWidth / 2;
				leftA = xAxisPivotA - a[i].hbWidth / 2;
				downA = yAxisPivotA + a[i].hbHeight / 2;
				upA = yAxisPivotA - a[i].hbHeight / 2;

				rightB = xAxisPivotB + p->hbWidth / 2;
				leftB = xAxisPivotB - p->hbWidth / 2;
				downB = yAxisPivotB + p->hbHeight / 2;
				upB = yAxisPivotB - p->hbHeight / 2;
				if (!p->immortality) {
					if ((rightA > leftB && rightA < rightB) || (leftA > leftB && leftA < rightB)) {
						if ((upA < downB && upA > upB) || (downA > upB && downA < downB)) {
							a[i].projectileTravel = false;
							a[i].x = 0;
							a[i].y = 0;

							*iFC = *fc;
							p->life -= a[i].damage;
							p->immortality = true;
						}
					}
				}
			}
		}
	}
}

void colisionDetection(entity* e, entity* p, int* iFC, int* fc) {
	float xAxisPivotP, yAxisPivotP, xAxisPivotE, yAxisPivotE, rightP, leftP, downP, upP, rightE, leftE, downE, upE;
	int i, j;

	for (j = 0; j < enemyMax; j++) {
		for (i = 0; i < projectileMax; i++) {

			xAxisPivotP = p[i].x + p[i].width / 2;
			yAxisPivotP = p[i].y + p[i].height / 2;
			xAxisPivotE = e[j].x + e[j].width / 2;
			yAxisPivotE = e[j].y + e[j].height / 2;

			rightP = xAxisPivotP + p[i].hbWidth / 2;
			leftP = xAxisPivotP - p[i].hbWidth / 2;
			downP = yAxisPivotP + p[i].hbHeight / 2;
			upP = yAxisPivotP - p[i].hbHeight / 2;

			rightE = xAxisPivotE + e[j].hbWidth / 2;
			leftE = xAxisPivotE - e[j].hbWidth / 2;
			downE = yAxisPivotE + e[j].hbHeight / 2;
			upE = yAxisPivotE - e[j].hbHeight / 2;

			if (!p->immortality) {
				if ((rightP > leftE && rightP < rightE) || (leftP > leftE && leftP < rightE)) {
					if ((upP < downE && upP > upE) || (downP > upE && downP < downE)) {
						
						*iFC = *fc;
						
						for (i = 0; i > -8; i--) {
							e->vel_y--;
						}
						
						p->life -= projectileDamage;
						p->immortality = true;
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
		*exit = true;
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
/*
void createTileSet(int **mat) {
	int i, j;

	mat = (int**)malloc(mapSize * sizeof(int*));
	if (mat) {
		for (i = 0; i < mapSize; ++i) {
			mat[i] = (int*)malloc(mapSize * sizeof(int));
		}

		FILE* file;
		fopen_s(&file, "Tiles/tilemap.txt", "r");

		for (i = 0; i < mapSize; i++)
		{
			for (j = 0; j < mapSize; j++)
			{
				fscanf_s(file, "%d", &mat[i][j]);
				//if (!fscanf_s(file, "%d", &mat[i][j]))
					//break;
				printf("%d\n", mat[i][j]);
			}

		}
	fclose(file);
	}
}*/
/*
void createTileSet(int* mat) {
	int i, j;

	mat = (int*)malloc(mapSize * mapSize * sizeof(int));
	if (mat) {

		FILE* file;
		fopen_s(&file, "Tiles/tilemap.txt", "r");

		for (i = 0; i < mapSize; i++)
		{
			for (j = 0; j < mapSize; j++)
			{
				fscanf_s(file, "%d", &mat[i * mapSize + j]);
				//if (!fscanf_s(file, "%d", &mat[i][j]))
					//break;
				printf("%d\n", mat[i * mapSize + j]);
			}

		}
		fclose(file);
	}
}*/



int main() {
	sfx_hit = al_load_sample("Audio/hit.wav");
	int i, j, projectileCount = 0, stageSelect = 1, enemyProjectileCount = 0, enemyDmgGauge = 0, hit = 0, hitI[2] = { 0, 0 }, hitII = 0, frameCount = 0, immortalityFC = 0, enemyDeadFC[enemyMax] = { 0, 0 };
	int** tileset = NULL;
	float cx = 0, cy = 0, w = 0, h = 0;
	char debugInput[2] = "", debugTest[6] = "debug", enemyLifeGauge[5], ptx[8], pty[8], objText[25];
	bool gameLoop = false, menuLoop = true, toggleStartText = false, exit = false, devMode = false, modDown = false, levelEditor = false;
	queue devChecker;

	initialize();
	initQueue(&devChecker);
	//devChecker->inicio = 0;
	//devChecker->fim = 0;
	//devChecker->total = 0;
	//devChecker->tamanho = queueSize;
	//initplayer(&player, playerSprites);
	//initenemy(enemy, &enemySprite);
	//createTileAtlas();

	tiles[ground].isSolid = true;
	tiles[ground].id = ground;

	tiles[air].isSolid = false;
	tiles[air].id = air;

	stage[backgroundL1] = al_load_bitmap("Img/bg.jpg");
	stage[backgroundL2] = al_load_bitmap("Img/backgroundLayer2.bmp");
	stage[foreground] = al_load_bitmap("Img/foreground.bmp");
	al_convert_mask_to_alpha(stage[foreground], al_map_rgb(255, 0, 255));

	al_clear_to_color(al_map_rgb(255, 255, 255));

	while (!exit) {
		while (menuLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			

			if (event.type == ALLEGRO_EVENT_KEY_CHAR) {
				debugInput[0] = event.keyboard.unichar;

				if (isQueueFull(&devChecker)) {
					dequeue(&devChecker);
					enqueue(&devChecker, debugInput);
				}
				else {
					enqueue(&devChecker, debugInput);
				}
				j = 0;

				for (i = devChecker.inicio; j < devChecker.total; i++) {
					if (i >= devChecker.tamanho) {
						i = 0;
					}
					if (devChecker.fila[i] != debugTest[j])	{
						break;
					}
					if (j == 4) {
						devMode = true;
					}
					j++;
				}
			}
			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_LSHIFT:
				case ALLEGRO_KEY_RSHIFT:
					modDown = true;
					break;
				case ALLEGRO_KEY_ENTER:
					switch (stageSelect) {
					case 1:
						killCount.count = 5;
						killCount.type = shooter;
						menuLoop = false;
						if (modDown && devMode) {
							player.x = 50 * tileSize;
							player.y = 50 * tileSize;
							levelEditor = true;
							gameLoop = false;
						}
						else {
							gameLoop = true;
							levelEditor = false;
						}
						break;
					case 2:
						endurance.count = 600;
						menuLoop = false;
						if (modDown && devMode) {
							player.x = 50 * tileSize;
							player.y = 50 * tileSize;
							levelEditor = true;
							gameLoop = false;
						}
						else {
							gameLoop = true;
							levelEditor = false;
						}
						break;
					}
					break;
				case ALLEGRO_KEY_UP:
					stageSelect--;
					if (stageSelect < 1) {
						stageSelect = 2;
					}
					break;
				case ALLEGRO_KEY_DOWN:
					stageSelect++;
					if (stageSelect > 2) {
						stageSelect = 1;
					}
					break;
				}
			}

			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				if (event.keyboard.keycode == ALLEGRO_KEY_LSHIFT || event.keyboard.keycode == ALLEGRO_KEY_RSHIFT) {
					modDown = false;
				}
			}

			if (event.type == ALLEGRO_EVENT_TIMER) {
				frameCount++;
			}
			exitGame(event, &menuLoop, &exit);

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(0, 0, 0));
				if (frameCount % (FPS / 2) == 0) {
					toggleStartText = !toggleStartText;
				}
				if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}
				if (toggleStartText) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, windowHeight / 2 - 12, ALLEGRO_ALIGN_CENTER, "Select mission!!");
				}
				al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, windowHeight / 2, ALLEGRO_ALIGN_CENTER, "1 - kill shooters");
				al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, windowHeight / 2 + 12, ALLEGRO_ALIGN_CENTER, "2 - endurance");
				switch (stageSelect) {
				case 1:
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2 - (10 * 12), windowHeight / 2, ALLEGRO_ALIGN_CENTER, "->");
					break;
				case 2:
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2 - (9 * 12), windowHeight / 2 + 12, ALLEGRO_ALIGN_CENTER, "->");
					break;
				}
				al_flip_display();
			}

		}

		tileset = (int**)malloc(mapSize * sizeof(int*));
		if (tileset) {
			for (i = 0; i < mapSize; ++i) {
				tileset[i] = (int*)malloc(mapSize * sizeof(int));
			}

			FILE* file;
			fopen_s(&file, "Tiles/tilemap1.txt", "r");

			switch (stageSelect) {
			case 1:
				break;
			case 2:
				fopen_s(&file, "Tiles/tilemap2.txt", "r");
				break;
			}

			for (i = 0; i < mapSize; i++) {
				for (j = 0; j < mapSize; j++) {
					fscanf_s(file, "%d", &tileset[i][j]);
				}
			}
			fclose(file);
		}

		al_play_sample(sample, 0.025, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, NULL);

		while (levelEditor) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
				player.tileX = event.mouse.x / tileSize;
				player.tileY = event.mouse.y / tileSize;
			}

			if (event.type == ALLEGRO_EVENT_TIMER) {
				if (player.x <= 0 + (al_get_display_width(display) / 2) || player.x >= (mapSize * tileSize) - (al_get_display_width(display) / 2)) {
					player.vel_x = 0;
				}

				if (player.y <= 0 + (al_get_display_height(display) / 2) || player.y >= (mapSize * tileSize) - (al_get_display_height(display) / 2)) {
					player.vel_y = 0;
				}

				player.x += player.vel_x;
				player.y += player.vel_y;

				refreshCamera(&cx, &cy, player);

				frameCount++;
			}

			exitGame(event, &menuLoop, &exit);

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
				case ALLEGRO_KEY_W:
					player.vel_y += -8;
					break;

				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_A:
					player.vel_x += -8;
					break;

				case ALLEGRO_KEY_DOWN:
				case ALLEGRO_KEY_S:
					player.vel_y += 8;
					break;

				case ALLEGRO_KEY_RIGHT:
				case ALLEGRO_KEY_D:
					player.vel_x += 8;
					break;

				case ALLEGRO_KEY_Q:
					player.selectedWeapon--;
					if (player.selectedWeapon < 0) {
						player.selectedWeapon = 1;
					}
					break;

				case ALLEGRO_KEY_E:
					player.selectedWeapon++;
					if (player.selectedWeapon > 1) {
						player.selectedWeapon = 0;
					}
					break;

				case ALLEGRO_KEY_X:
					al_play_sample(shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
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
				case ALLEGRO_KEY_UP:
				case ALLEGRO_KEY_W:
				case ALLEGRO_KEY_DOWN:
				case ALLEGRO_KEY_S:
					player.vel_y = 0;
					break;

				case ALLEGRO_KEY_LEFT:
				case ALLEGRO_KEY_A:
				case ALLEGRO_KEY_RIGHT:
				case ALLEGRO_KEY_D:
					player.vel_x = 0;
					break;
				}
			}

			exitGame(event, &gameLoop, &exit);

			if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
				al_acknowledge_resize(display);
			}

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(255, 255, 255));

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.05, -cy * 0.05);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL1], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL1], al_get_bitmap_width(stage[backgroundL1]), 0, ALLEGRO_FLIP_HORIZONTAL);

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx, -cy);
				al_use_transform(&camera);

				for (i = 0; i < mapSize; i++) {
					for (j = 0; j < mapSize; j++) {
						switch (tileset[i][j]) {
						case ground:
							al_draw_filled_rectangle(j * tileSize, i * tileSize, j * tileSize + tileSize, i * tileSize + tileSize, al_map_rgb(255, 0, 0));
							break;
						case air:
							//al_draw_rectangle(j* tileSize, i* tileSize, j* tileSize + tileSize, i* tileSize + tileSize, al_map_rgb(255, 0, 0), 2);
							break;
						case roof:
							break;
						}
					}
				}

				al_identity_transform(&camera);
				al_use_transform(&camera);

				switch (stageSelect) {
				case 1:
					sprintf_s(objText, sizeof(objText), "Stage %d", stageSelect);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				case 2:
					sprintf_s(objText, sizeof(objText), "Stage %d", stageSelect);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				}

				switch (player.selectedWeapon) {
				case 0:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = air");
					break;
				case 1:
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 28, 0, "Selected Tile = ground");
					break;
				}

				if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}

				sprintf_s(ptx, sizeof(ptx), "X = %d", player.tileX);
				al_draw_text(font, al_map_rgb(255, 255, 255), 10, 50, 0, ptx);
				sprintf_s(pty, sizeof(pty), "Y = %d", player.tileY);
				al_draw_text(font, al_map_rgb(255, 255, 255), 100, 50, 0, pty);

				al_flip_display();
			}
		}

		initplayer(&player, playerSprites);
		initenemy(enemy, &enemySprite);

		while (gameLoop) {
			ALLEGRO_EVENT event;
			al_wait_for_event(evQueue, &event);

			if (event.type == ALLEGRO_EVENT_TIMER) {
				refreshPlayerMovement(&player, tiles, tileset);
				refreshEnemyMovement(enemy, &player);
				refreshProjectileState(playerShot, enemyShot, player, &projectileCount, &enemyProjectileCount, cx, cy);
				refreshProjectileState(playerTripleShot, enemyShot, player, &projectileCount, &enemyProjectileCount, cx, cy);

				refreshCamera(&cx, &cy, player);

				if (player.isShooting) {
					if (player.spriteChange >= FPS / 2) {
						player.spriteChange = 0;
						player.sprite = neutral;
						player.isShooting = false;
					}
					else player.spriteChange++;
				}

				for (j = 0; j < enemyMax; j++) {
					if (enemy[j].attack == shooter) {
						for (i = 0; i < enemyProjectileMax; i++) {
							if (frameCount - enemy[j].shotFC >= FPS) {
								if (!enemyShot[i].projectileTravel) {
									enemyProjectileCount++;
									eShoot(&enemyShot[i], &enemy[j], &player, frameCount);
									break;
								}
							}
						}
					}
				}

				//Bullet hell lmao
				/*for (j = 0; j < enemyMax; j++) {
					if (enemy[j].attack == shooter) {
						for (i = 0; i < enemyProjectileMax; i++) {
							if (frameCount - enemy[i].shotFC >= FPS) {
								if (!enemyShot[i].projectileTravel) {
									enemyProjectileCount++;
									eShoot(&enemyShot[i], &enemy[j], &player, frameCount);
									break;
								}
							}
						}
					}
				}*/

				hitboxDetection(playerShot, enemy, &player, &killCount, hitI, &projectileCount, &immortalityFC, enemyDeadFC, &frameCount);
				hitboxDetection(enemyShot, enemy, &player, &killCount, hitI, &projectileCount, &immortalityFC, enemyDeadFC, &frameCount);
				hitboxDetection(playerTripleShot, enemy, &player, &killCount, hitI, &projectileCount, &immortalityFC, enemyDeadFC, &frameCount);
				colisionDetection(enemy, &player, &immortalityFC, &frameCount);

				/*while (hit > 0) {
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
						objKillCount++;
					}
				}
				*/
				for (i = 0; i < enemyMax; i++) {
					if (!enemy[i].alive && frameCount - enemyDeadFC[i] >= FPS) {
						enemyRandomizer(&enemy[i]);
						enemyDmgGauge = 0;
						enemy[i].alive = true;
					}
				}

				if (player.life <= 0) {
					player.alive = false;
					menuLoop = true;
					gameLoop = false;
				}
				if (player.immortality && frameCount - immortalityFC >= (int)(1.5 * FPS)) {
					player.immortality = false;
				}

				//anglePE = atan2((-1.0 * enemy.y) - (-1.0 * player.y), (double) enemy.x - player.x);
				//anglePE = anglePE * 180.0 / PI;
				//anglePE = (float) anglePE;
				//cosPE = cos(anglePE);
				//sinPE = sin(anglePE);
				//absF(&cosPE);
				//absF(&sinPE);
				//absD(&anglePE);

				if (enemyDmgGauge < enemy[hitI[enemyI]].maxLife - enemy[hitI[enemyI]].life && frameCount % 2 == 0) {
					enemyDmgGauge++;
				}


				switch (stageSelect) {
				case 1:
					if (killCount.count == 0) {
						menuLoop = true;
						gameLoop = false;
					}
					break;
				case 2:
					endurance.count--;
					if (endurance.count == 0) {
						menuLoop = true;
						gameLoop = false;
					}
					break;
				}

				setSpriteColor(&player);

				frameCount++;
				al_flip_display();
			}

			if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
				switch (event.keyboard.keycode) {
				case ALLEGRO_KEY_UP:
					
					if (player.onGround) {
						al_play_sample(sfx_jump, 0.25, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
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
					if (player.selectedWeapon > 2) {
						player.selectedWeapon = 0;
					}
					break;

				case ALLEGRO_KEY_X:
					al_play_sample(shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
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
				

				case ALLEGRO_KEY_C:
					al_play_sample(shot, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
					if (projectileCount < projectileMax) {
						for (i = 0; i < projectileMax; i++) {
							if (!playerShot[i].projectileTravel) {

								projectileCount++;
								player.spriteChange = 0;
								tripleShot(&playerTripleShot[i], &player);
								
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

			if (al_is_event_queue_empty(evQueue)) {
				al_clear_to_color(al_map_rgb(255, 255, 255));

				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.05, -cy * 0.05);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL1], 0, 0, 0);
				al_draw_bitmap(stage[backgroundL1], al_get_bitmap_width(stage[backgroundL1]), 0, ALLEGRO_FLIP_HORIZONTAL);

				/*al_identity_transform(&camera);
				al_translate_transform(&camera, -cx * 0.35, -cy * 0.35);
				al_use_transform(&camera);

				al_draw_bitmap(stage[backgroundL2], 0, 240, 0);
				al_draw_bitmap(stage[backgroundL2], al_get_bitmap_width(stage[backgroundL2]), 240, 0);
				*/
				al_identity_transform(&camera);
				al_translate_transform(&camera, -cx, -cy);
				al_use_transform(&camera);

				/*al_draw_bitmap(stage[foreground], 0, 226, 0);
				al_draw_bitmap(stage[foreground], al_get_bitmap_width(stage[foreground]), 226, ALLEGRO_FLIP_HORIZONTAL);
				al_draw_bitmap(stage[foreground], 2 * al_get_bitmap_width(stage[foreground]), 226, 0);*/

				for (i = 0; i < mapSize; i++) {
					for (j = 0; j < mapSize; j++) {
						switch (tileset[i][j]) {
						case ground:
							al_draw_filled_rectangle(j * tileSize, i * tileSize, j * tileSize + tileSize, i * tileSize + tileSize, al_map_rgb(255, 0, 0));
							break;
						case air:
							//al_draw_rectangle(j* tileSize, i* tileSize, j* tileSize + tileSize, i* tileSize + tileSize, al_map_rgb(255, 0, 0), 2);
							break;
						case roof:
							break;
						}
					}
				}

				if (player.currentDir == Right) {
					al_draw_bitmap(playerSprites[player.sprite], player.x, player.y, ALLEGRO_FLIP_HORIZONTAL);
					//al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hbWidth, player.hbY + player.hbHeight, al_map_rgba(0, 0, 255, 50));
				}
				else {
					al_draw_bitmap(playerSprites[player.sprite], player.x, player.y, 0);
					//al_draw_filled_rectangle(player.hbX, player.hbY, player.hbX + player.hbWidth, player.hbY + player.hbHeight, al_map_rgba(0, 0, 255, 50));
				}

				for (i = 0; i < enemyMax; i++) {
					if (enemy[i].alive) {
						if (enemy[i].x < player.x) {
							al_draw_bitmap(enemySprite[enemy[i].selectedWeapon], enemy[i].x, enemy[i].y, ALLEGRO_FLIP_HORIZONTAL);
						}
						else {
							al_draw_bitmap(enemySprite[enemy[i].selectedWeapon], enemy[i].x, enemy[i].y, 0);
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

				for (i = 0; i < projectileMax; i++) {
					if (playerTripleShot[i].projectileTravel) {
						setProjectileColor(&playerTripleShot[i]);
						if (playerTripleShot[i].dir == Right) {
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y, ALLEGRO_FLIP_HORIZONTAL);
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y + 50, ALLEGRO_FLIP_HORIZONTAL);
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y - 50, ALLEGRO_FLIP_HORIZONTAL);
						}
						else {
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y, 0);
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y + 50, 0);
							al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(playerTripleShot[i].r, playerTripleShot[i].g, playerTripleShot[i].b), playerTripleShot[i].x, playerTripleShot[i].y - 50, 0);
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

				switch ((int)player.life / 10) {
				case 3:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0));
					break;
				case 2:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_filled_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0));
					al_draw_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0), 2);
					break;
				case 1:
					al_draw_filled_rectangle(10, 35, 20, 45, al_map_rgb(255, 0, 0));
					al_draw_rectangle(30, 35, 40, 45, al_map_rgb(255, 0, 0), 2);
					al_draw_rectangle(50, 35, 60, 45, al_map_rgb(255, 0, 0), 2);
					break;
				}

				al_draw_filled_rectangle(windowWidth - (2 * (enemy[hitI[enemyI]].maxLife - enemyDmgGauge) + 50), 50, windowWidth - (enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(255, 0, 0));
				al_draw_filled_rectangle(windowWidth - 50, 50, windowWidth - (2 * enemy[hitI[enemyI]].life + 50), 62, al_map_rgb(0, 128, 0));
				sprintf_s(enemyLifeGauge, sizeof(enemyLifeGauge), "%.0f", enemy[hitI[enemyI]].life);
				al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth - 48, 42, 0, enemyLifeGauge);

				switch (stageSelect) {
				case 1:
					sprintf_s(objText, sizeof(objText), "Shooters left = %d", killCount.count);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				case 2:
					sprintf_s(objText, sizeof(objText), "survive = %d", endurance.count);
					al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, objText);
					break;
				}

				if (devMode) {
					al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth / 2, 0, ALLEGRO_ALIGN_CENTER, "Dev mode!!");
				}

				sprintf_s(ptx, sizeof(ptx), "X = %d", player.tileX);
				al_draw_text(font, al_map_rgb(255, 255, 255), 10, 50, 0, ptx);
				sprintf_s(pty, sizeof(pty), "Y = %d", player.tileY);
				al_draw_text(font, al_map_rgb(255, 255, 255), 100, 50, 0, pty);

				al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(player.r, player.g, player.b), al_get_bitmap_width(playerShotTemplate) + 20, 30, ALLEGRO_FLIP_HORIZONTAL);

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
	al_destroy_bitmap(enemySprite[antiBiotic]);
	al_destroy_bitmap(enemySprite[antiMycotic]);
	al_destroy_bitmap(enemySprite[antiVirus]);
	al_destroy_bitmap(enemyShooterSprite);
	al_destroy_bitmap(stage[backgroundL1]);
	al_destroy_bitmap(stage[backgroundL2]);
	al_destroy_bitmap(stage[foreground]);
	al_destroy_bitmap(playerShotTemplate);
	al_destroy_bitmap(enemyShotTemplate);
	al_destroy_sample(sample);
	free(tileset);

	return 0;
}

//font by everiux365

