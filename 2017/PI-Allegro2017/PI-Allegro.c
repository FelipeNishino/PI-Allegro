#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
//#include <allegro5/allegro_native_dialog.h>
//#include <allegro5/allegro_primitives.h>

#define FPS 60.0
#define windowWidth 800
#define windowHeight 600

// Felipe
#define projectileVelocity 0.6
#define projectileAccel 0.25
#define projectileOffset 10
#define projectileMax 3

// Helkson
#define Up 0
#define Left 1
#define Right 2
#define gravity 0.1

struct sprite {
	// Felipe
	ALLEGRO_BITMAP *spriteBitmap;
	float x, y;
	float x0, y0;
	float xLenght, yLenght;
	float speed, accel;
	int spriteChange;
	bool isShooting;
	bool projectileTravel;

	//Helkson
	int dir;
	int vel_x;
	int vel_y;
	int vel_max;
	int limite;
};

ALLEGRO_DISPLAY * display = NULL;
ALLEGRO_FONT * font = NULL;
ALLEGRO_TIMER * timer = NULL;
ALLEGRO_EVENT_QUEUE * queue = NULL;
struct sprite player;
struct sprite projectile[projectileMax];


int initialize() {

	al_init();
	al_init_image_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	//al_init_primitives_addon();

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

int initplayer(struct sprite *c) {
	c->spriteBitmap = al_load_bitmap("Img/alienNeutral.bmp");
	if (!c->spriteBitmap) {
		fprintf(stderr, "Falha ao carregar imagem!\n");
		return -1;
	}

	c->x = 100;
	c->y = 500;
	c->xLenght = 68;
	c->yLenght = 45;
	c->vel_x = 3;
	c->vel_y = 3;
	c->limite = 600;
	c->dir = 5;

	return 0;
}

void actShoot(struct sprite *p, struct sprite *c, int n) {
	p->spriteBitmap = al_load_bitmap("Img/tiro.bmp");
	c->spriteBitmap = al_load_bitmap("Img/alienShot.bmp");

	p->speed = projectileVelocity;
	p->accel = projectileAccel;

	p->x0 = c->x + c->xLenght;
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

			if (p[j].x - p[j].x0 < 400) {
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

int main() {
	int i, projectileCount = 0, lastProjectileI = -1, keyPress = 0;
	bool gameLoop = true;

	initialize();
	initplayer(&player);

	al_clear_to_color(al_map_rgb(255, 255, 255));

	while (gameLoop) {
		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);

		/*while (player.y > player.limite)
		{
			player.vel_y = player.vel_y - gravity;
			player.y += player.vel_y;
		}*/

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

			switch (player.dir)
			{
			case Up:
				player.y -= player.vel_y;
				break;
			case Right:
				player.x += player.vel_x;
				break;
			case Left:
				player.x -= player.vel_x;
				break;
			}

			al_flip_display();
		}

		if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
			switch (event.keyboard.keycode) {
			case ALLEGRO_KEY_UP:
				player.dir = Up;
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
			
 			if (event.type == ALLEGRO_EVENT_KEY_UP) {
				player.dir = 5;
			}
		}

		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE || event.type == ALLEGRO_KEY_ESCAPE) {
			gameLoop = false;
		}

		if (al_is_event_queue_empty(queue)) {
			al_clear_to_color(al_map_rgb(255, 255, 255));
			al_draw_bitmap(player.spriteBitmap, player.x, player.y, ALLEGRO_FLIP_HORIZONTAL);

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