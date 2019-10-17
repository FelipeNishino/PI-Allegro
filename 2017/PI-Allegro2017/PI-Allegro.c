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

#define FPS 60.0
#define windowWidth 800
#define windowHeight 450

// Felipe
#define projectileVelocity 0.6
#define projectileAccel 0.25
#define projectileOffset 10
#define projectileMax 3
#define projectileDamage 10
#define antiVirus 0
#define antiBiotic 1
#define antiMycotic 2
#define targetPracticeLife 20
#define PI 3.14159265
#define SODIUM_STATIC

// Helkson
#define Left 0
#define Right 1
#define gravity 0.275

struct sprite {
	// Felipe
	ALLEGRO_BITMAP *spriteBitmap;
	double x, y;
	float width, height;
	float life, maxLife;
	int hitboxWidth, hitboxHeight;
	int spriteChange;
	int r;
	int g;
	int b;
	int selectedWeapon;
	int currentDir;
	bool isShooting;
	bool jump;
	bool alive;


	int dir;
	float vel_x;
	float vel_y;
};

struct projectile {
	ALLEGRO_BITMAP *spriteBitmap;
	double x, y;
	float x0, y0;
	float width, height;
	float speed, accel;
	int hitboxWidth, hitboxHeight;
	float damage;
	int type;
	int dir;
	int r;
	int g;
	int b;
	bool projectileTravel;
	bool projectileHit;
};

ALLEGRO_DISPLAY * display = NULL;
ALLEGRO_FONT * font = NULL;
ALLEGRO_TIMER * timer = NULL;
ALLEGRO_SAMPLE *sample = NULL;
ALLEGRO_SAMPLE_INSTANCE *sampleInstance = NULL;
ALLEGRO_EVENT_QUEUE * queue = NULL;
ALLEGRO_BITMAP *playerShotTemplate;
ALLEGRO_BITMAP *backgroundL1;
ALLEGRO_BITMAP *backgroundL2;
ALLEGRO_BITMAP *foreground;
struct sprite player;
struct projectile playerShot[projectileMax];
struct sprite enemy;
struct projectile enemyShot[25];

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

double absD(double *x) {
	if (*x < 0) {
		*x = -1 * (*x);
	}
	return *x;
}

void setSpriteColor(struct sprite *a) {
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

void setProjectileColor(struct projectile *a) {
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

void enemyRandomizer(struct sprite *e) {
	int section;
	int lifeRandomizer = randombytes_uniform(16);

	//section = randombytes_uniform(2);
	section = 1;

	if (section == 1) {
		e->x = 550 + randombytes_uniform(201);
	}
	else {
		e->x = 50 + randombytes_uniform(201);
	}

	e->y = 250 + randombytes_uniform(101);

	e->selectedWeapon = randombytes_uniform(3);

	setSpriteColor(e);

	e->life = targetPracticeLife + lifeRandomizer;
	e->maxLife = targetPracticeLife + lifeRandomizer;

	e->alive = true;
}

int initplayer(struct sprite *c) {
	c->spriteBitmap = al_load_bitmap("Img/alienNeutral.bmp");
	if (!c->spriteBitmap) {
		fprintf(stderr, "Falha ao carregar imagem!\n");
		return -1;
	}
	al_convert_mask_to_alpha(c->spriteBitmap, al_map_rgb(255, 0, 255));

	c->x = 366;
	c->y = 50;
	c->hitboxWidth = 68;
	c->hitboxHeight = 45;
	c->vel_x = 4.5;
	c->dir = 5;
	c->currentDir = Right;
	c->selectedWeapon = 0;
	c->jump = false;

	return 0;
}

int initenemy(struct sprite *e) {
	e->spriteBitmap = al_load_bitmap("Img/miniufo.bmp");
	al_convert_mask_to_alpha(e->spriteBitmap, al_map_rgb(255, 0, 255));

	enemyRandomizer(e);

	e->vel_x = 0.5;
	e->vel_y = 0.5;

	e->width = 44;
	e->height = 38;

	e->hitboxWidth = 44;
	e->hitboxHeight = 38;

	return 0;
}


void actShoot(struct projectile *p, struct sprite *c) {
	p->spriteBitmap = al_load_bitmap("Img/tiro.bmp");
	al_convert_mask_to_alpha(p->spriteBitmap, al_map_rgb(255, 0, 255));
	c->spriteBitmap = al_load_bitmap("Img/alienShot.bmp");
	al_convert_mask_to_alpha(c->spriteBitmap, al_map_rgb(255, 0, 255));

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

int refreshProjectileState(struct projectile p[]) {
	int j, hitCount = 0;

	for (j = 0; j < projectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].speed <= 20) {
				p[j].speed += p[j].accel;
			}

			if ((p[j].x + p[j].width) >= 0 && p[j].x <= 800) {
				if (p[j].dir == Right)
					p[j].x += p[j].speed;
				else
					p[j].x -= p[j].speed;
			}
			else {
				p[j].projectileTravel = false;
				hitCount++;
			}
		}
	}
	return hitCount;
}

void refreshMovementState(struct sprite *p) {
	if (p->y >= 350) {
		p->y = 350;
		p->jump = false;
	}
	else {
		p->vel_y += gravity;
		p->y += p->vel_y;
	}

	if (p->x <= 0) {
		p->x = 1;
	}

	if (p->x >= 732) {
		p->x = 731;
	}
}

int hitboxDetection(struct projectile *a, struct sprite b, int *hitCount) {
	float xAxisPivotA, yAxisPivotA, xAxisPivotB, yAxisPivotB, rightA, leftA, downA, upA, rightB, leftB, downB, upB;
	int i, hitI = -1;

	for (i = 0; i < projectileMax; i++) {
		if (a[i].projectileTravel) {
			xAxisPivotA = a[i].x + a[i].width / 2;
			yAxisPivotA = a[i].y + a[i].height / 2;
			xAxisPivotB = b.x + b.width / 2;
			yAxisPivotB = b.y + b.height / 2;

			rightA = xAxisPivotA + a[i].hitboxWidth / 2;
			leftA = xAxisPivotA - a[i].hitboxWidth / 2;
			downA = yAxisPivotA + a[i].hitboxHeight / 2;
			upA = yAxisPivotA - a[i].hitboxHeight / 2;

			rightB = xAxisPivotB + b.hitboxWidth / 2;
			leftB = xAxisPivotB - b.hitboxWidth / 2;
			downB = yAxisPivotB + b.hitboxHeight / 2;
			upB = yAxisPivotB - b.hitboxHeight / 2;

			if ((rightA > leftB && rightA < rightB) || (leftA > leftB && leftA < rightB)) {
				if ((upA < downB && upA > upB) || (downA > upB && downA < downB)) {
					a[i].projectileTravel = false;
					a[i].x = 0;
					a[i].y = 0;
					*hitCount += 1;
					hitI = i;
				}
			}
		}
	}
	return hitI;
}

int main() {
	int i, projectileCount = 0, enemyDmgGauge = 0, hit = 0, hitI = 0, frameCount = 0, auxFrameCount = 0, killCount = 0;
	double anglePE;
	char enemyLifeGauge[5], kcText[15];
	bool gameLoop = false, menuLoop = true, toggleStartText = true;

	initialize();
	initplayer(&player);
	initenemy(&enemy);

	backgroundL1 = al_load_bitmap("Img/backgroundLayer1.bmp");
	backgroundL2 = al_load_bitmap("Img/backgroundLayer2.bmp");
	foreground = al_load_bitmap("Img/foreground2.bmp");
	al_convert_mask_to_alpha(foreground, al_map_rgb(255, 0, 255));

	al_clear_to_color(al_map_rgb(255, 255, 255));

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

		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || event.type == ALLEGRO_KEY_ESCAPE) {
			gameLoop = false;
		}

		if (al_is_event_queue_empty(queue)) {
			al_clear_to_color(al_map_rgb(0, 0, 0));
			if (frameCount % 60 == 0) {
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
			if (player.isShooting) {
				if (player.spriteChange >= 30) {
					player.spriteChange = 0;
					player.spriteBitmap = al_load_bitmap("Img/alienNeutral.bmp");
					al_convert_mask_to_alpha(player.spriteBitmap, al_map_rgb(255, 0, 255));
					player.isShooting = false;
				}
				else player.spriteChange++;
			}

			projectileCount -= refreshProjectileState(playerShot);

			hitI = hitboxDetection(playerShot, enemy, &hit);

			projectileCount -= hit;

			while (hit > 0) {
				if (enemy.selectedWeapon == playerShot[hitI].type) {
					enemy.life -= playerShot[0].damage + playerShot[0].damage * (randombytes_uniform(16) / 100.0);
				}
				hit--;

				if (enemy.life <= 0) {
					enemy.alive = false;
					enemy.y = 0;
					enemy.life = 0;
					auxFrameCount = frameCount;
					killCount++;
				}
			}

			if (!enemy.alive && frameCount - auxFrameCount >= 60) {
				enemyRandomizer(&enemy);
				enemyDmgGauge = 0;
				enemy.alive = true;
			}

			switch (player.dir)
			{
			case Right:
				if (player.x > 0) {
					player.x += player.vel_x;
				}
				break;
			case Left:
				if (player.x < 732) {
					player.x -= player.vel_x;
				}
				break;
			}

			refreshMovementState(&player);

			anglePE = atan2((-1 * enemy.y) - (-1 * player.y), enemy.x - player.x);
			anglePE = anglePE * 180 / PI;
			//absD(&anglePE);

			if (enemy.x != player.x) {
				if (enemy.x > player.x)	{
					enemy.x -= enemy.vel_x * cos(anglePE);
				}
				else {
					enemy.x += enemy.vel_x * cos(anglePE);
				}
			}
			if (enemy.y != player.y) {
				if (enemy.y > player.y) {
					enemy.y += enemy.vel_y * sin(anglePE);
				}
				else {
					enemy.y -= enemy.vel_y * sin(anglePE);
				}
			}

			if (enemyDmgGauge < enemy.maxLife - enemy.life && frameCount % 2 == 0) {
				enemyDmgGauge++;
			}

			setSpriteColor(&player);

			frameCount++;
			al_flip_display();
		}

		if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch (event.keyboard.keycode) {
			case ALLEGRO_KEY_UP:
				if (!player.jump) {
					player.vel_y = -7;
					player.y--;
					player.jump = true;
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
							actShoot(&playerShot[i], &player);
							break;
						}
					}
				}
				break;
			}
		}

		if (event.type == ALLEGRO_EVENT_KEY_UP)
		{
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

		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || event.type == ALLEGRO_KEY_ESCAPE) {
			gameLoop = false;
		}

		if (al_is_event_queue_empty(queue)) {
			al_clear_to_color(al_map_rgb(255, 255, 255));
			al_draw_bitmap(backgroundL1, 0, 0, 0);
			al_draw_bitmap(backgroundL2, 0, 240, 0);
			al_draw_bitmap(foreground, 0, 226, 0);

			if (enemy.alive) {
				if (enemy.x < 400) {
					al_draw_tinted_bitmap(enemy.spriteBitmap, al_map_rgb(enemy.r, enemy.g, enemy.b), enemy.x, enemy.y, ALLEGRO_FLIP_HORIZONTAL);
				}
				else {
					al_draw_tinted_bitmap(enemy.spriteBitmap, al_map_rgb(enemy.r, enemy.g, enemy.b), enemy.x, enemy.y, 0);
				}
			}

			if (player.currentDir == Right) {
				al_draw_bitmap(player.spriteBitmap, player.x, player.y, ALLEGRO_FLIP_HORIZONTAL);
			}
			else {
				al_draw_bitmap(player.spriteBitmap, player.x, player.y, 0);
			}

			al_draw_filled_rectangle(windowWidth - (2 * (enemy.maxLife - enemyDmgGauge) + 50), 50, windowWidth - (enemy.life + 50), 62, al_map_rgb(255, 0, 0));
			al_draw_filled_rectangle(windowWidth - 50, 50, windowWidth - (2 * enemy.life + 50), 62, al_map_rgb(0, 128, 0));
			sprintf_s(enemyLifeGauge, sizeof(enemyLifeGauge), "%.0f", enemy.life);
			al_draw_text(font, al_map_rgb(255, 255, 255), windowWidth - 48, 42, 0, enemyLifeGauge);
			sprintf_s(kcText, sizeof(kcText), "killcount = %d", killCount);
			al_draw_text(font, al_map_rgb(255, 255, 255), 10, 5, 0, kcText);

			al_draw_tinted_bitmap(playerShotTemplate, al_map_rgb(player.r, player.g, player.b), 0, 30, ALLEGRO_FLIP_HORIZONTAL);

			for (i = 0; i < projectileMax; i++) {
				if (playerShot[i].projectileTravel) {
					setProjectileColor(&playerShot[i]);
					if (playerShot[i].dir == Right) {
						al_draw_tinted_bitmap(playerShot[i].spriteBitmap, al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].x, playerShot[i].y, ALLEGRO_FLIP_HORIZONTAL);
					}
					else {
						al_draw_tinted_bitmap(playerShot[i].spriteBitmap, al_map_rgb(playerShot[i].r, playerShot[i].g, playerShot[i].b), playerShot[i].x, playerShot[i].y, 0);
					}
				}
			}

			al_flip_display();
		}
	}




	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_font(font);
	al_uninstall_keyboard();
	al_uninstall_mouse();
	al_destroy_bitmap(player.spriteBitmap);
	al_destroy_bitmap(enemy.spriteBitmap);
	al_destroy_bitmap(backgroundL1);
	al_destroy_bitmap(backgroundL2);
	al_destroy_bitmap(foreground);
	al_destroy_sample(sample);
	for (i = 0; i < projectileMax; i++) {
		al_destroy_bitmap(playerShot[i].spriteBitmap);
	}



	return 0;
}

//font by everiux365
