#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#define FPS 60.0
#define windowWidth 800
#define windowHeight 600

// Felipe
#define projectileVelocity 0.6
#define projectileAccel 0.25
#define projectileOffset 10
#define projectileMax 3
#define projectileDamage 10
#define targetPracticeLife 20
#define SODIUM_STATIC

// Helkson
#define Left 0
#define Right 1
#define gravity 0.275

struct sprite {
	// Felipe
	ALLEGRO_BITMAP* spriteBitmap;
	float x, y;
	float x0, y0;
	float width, height;
	float speed, accel;
	float life;
	int hitboxWidth, hitboxHeight;
	int spriteChange;
	int damage;
	bool isShooting;
	bool jump;
	bool alive;
	bool projectileTravel;
	bool projectileHit;

	//Helkson
	int dir;
	float vel_x;
	float vel_y;
};

ALLEGRO_DISPLAY* display = NULL;
ALLEGRO_FONT* font = NULL;
ALLEGRO_TIMER* timer = NULL;
ALLEGRO_EVENT_QUEUE* queue = NULL;
struct sprite player;
struct sprite projectile[projectileMax];
struct sprite enemy;

int initialize() {

	sodium_init();

	al_init();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();

	timer = al_create_timer(1.0 / FPS);
	display = al_create_display(windowWidth, windowHeight);
	queue = al_create_event_queue();
	font = al_load_font("Fonts/arial.ttf", 12, 0);
	al_set_window_title(display, "Metal Slug 5");

	al_install_keyboard();
	al_install_mouse();

	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_keyboard_event_source());

	al_start_timer(timer);

	return 0;
}

int initplayer(struct sprite* c) {
	c->spriteBitmap = al_load_bitmap("Img/alienNeutral.bmp");
	if (!c->spriteBitmap) {
		fprintf(stderr, "Falha ao carregar imagem!\n");
		return -1;
	}

	c->x = 100;
	c->y = 500;
	c->vel_x = 7;
	c->vel_y = 0;
	c->hitboxWidth = 68;
	c->hitboxHeight = 45;
	c->vel_x = 3;
	c->vel_y = 3;
	c->dir = 5;
	c->jump = false;

	return 0;
}

int initenemy(struct sprite* e) {
	e->spriteBitmap = al_load_bitmap("Img/miniufo.bmp");

	e->x = 550 + randombytes_uniform(201);
	e->y = 400 + randombytes_uniform(101);
	e->life = targetPracticeLife;

	e->width = 44;
	e->height = 38;

	e->hitboxWidth = 44;
	e->hitboxHeight = 38;

	return 0;
}


void actShoot(struct sprite* p, struct sprite* c, int n) {
	p->spriteBitmap = al_load_bitmap("Img/tiro.bmp");
	c->spriteBitmap = al_load_bitmap("Img/alienShot.bmp");

	p->speed = projectileVelocity;
	p->accel = projectileAccel;

	p->width = 46;
	p->height = 22;

	p->hitboxWidth = 46;
	p->hitboxHeight = 22;

	p->x0 = c->x + c->hitboxWidth;
	p->y0 = c->y + projectileOffset;
	p->x = p->x0;
	p->y = p->y0;

	c->isShooting = true;
	p->projectileTravel = true;
}

int refreshProjectileState(struct sprite p[], int i, int n) {
	int j, aux;

	aux = n;

	for (j = 0; j < projectileMax; j++) {
		if (p[j].projectileTravel) {
			if (p[j].speed <= 20) {
				p[j].speed += p[j].accel;
			}

			if (p[j].x - p[j].x0 < windowWidth) {
				p[j].x += p[j].speed;
			}
			else {
				p[j].projectileTravel = false;
				n--;
			}
		}
	}

	if (aux - n <= aux) {
		return (aux - n);
	}
	else {
		return 0;
	}
}

void createTarget(struct sprite* e) {
	e->x = 550 + randombytes_uniform(201);
	e->y = 400 + randombytes_uniform(101);
	e->life = targetPracticeLife;
}

int hitboxDetection(struct sprite* a, struct sprite b) {
	float xAxisPivotA, yAxisPivotA, xAxisPivotB, yAxisPivotB, rightA, leftA, downA, upA, rightB, leftB, downB, upB;
	int i, hitCount = 0;

	for (i = 0; i < projectileMax; i++) {
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
				hitCount++;
			}
		}
	}

	return hitCount;
}

int main() {
	int i, projectileCount = 0, hit = 0, lastProjectileI = -1, keyPress = 0;
	bool gameLoop = true;

	initialize();
	initplayer(&player);
	initenemy(&enemy);

	al_clear_to_color(al_map_rgb(255, 255, 255));

	while (gameLoop) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);


		if (event.type == ALLEGRO_EVENT_TIMER) {
			if (player.isShooting) {
				if (player.spriteChange > 30) {
					player.spriteChange = 0;
					player.spriteBitmap = al_load_bitmap("Img/alienNeutral.bmp");
					player.isShooting = false;
				}
				else player.spriteChange++;
			}

			projectileCount = projectileCount - refreshProjectileState(projectile, lastProjectileI, projectileCount);

			hit = hitboxDetection(projectile, enemy);

			projectileCount = projectileCount - hit;

			if (hit > 0) {
				enemy.life -= projectileDamage;

				if (enemy.life <= 0) {
					createTarget(&enemy);
				}

				hit = 0;
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

			if (player.y >= 500) {
				player.y = 500;
				player.jump = false;
			}
			else {
				player.vel_y += gravity;
				player.y += player.vel_y;
			}

			if (player.x <= 0) {
				player.x = 1;
			}

			if (player.x >= 732) {
				player.x = 731;
			}

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
				break;

			case ALLEGRO_KEY_RIGHT:
				player.dir = Right;
				break;

			case ALLEGRO_KEY_SPACE:
				if (projectileCount <= 3) {
					projectileCount++;
					lastProjectileI = (lastProjectileI + 1) % 3;
					player.spriteChange = 0;
					actShoot(&projectile[lastProjectileI], &player, lastProjectileI);
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
			al_draw_bitmap(player.spriteBitmap, player.x, player.y, ALLEGRO_FLIP_HORIZONTAL);
			al_draw_bitmap(enemy.spriteBitmap, enemy.x, enemy.y, 0);

			for (i = 0; i < projectileMax; i++) {
				if (projectile[i].projectileTravel) {
					al_draw_bitmap(projectile[i].spriteBitmap, projectile[i].x, projectile[i].y, ALLEGRO_FLIP_HORIZONTAL);
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
	al_destroy_bitmap(projectile[0].spriteBitmap);
	al_destroy_bitmap(projectile[1].spriteBitmap);
	al_destroy_bitmap(projectile[2].spriteBitmap);

	return 0;
}