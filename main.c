#include <stdio.h>
#include <SDL.h>
#include <SDL_mixer.h>
#include "./cons.c"

int game_is_running;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* bg = NULL, *b_car = NULL, *r_car = NULL, *g_car = NULL;
SDL_Texture* texture1 = NULL, *texture2 = NULL, *texture3 = NULL, *texture4 = NULL;
Mix_Music* music = NULL;

int last_frame_time = 0;
int clicked_car_index = -1;
int lvl = 3, dth = 0, noc, win, restart = TRUE, sound = TRUE, coll, r=0, g=0, b=0, fu = -50;


struct car {
	float x, y, dx, dy, angle;
	int color, que;
}cars[15];

SDL_bool check_collision(SDL_Rect rect1, SDL_Rect rect2) {
	if (rect1.x < rect2.x + CAR_WIDTH &&
		rect1.x + CAR_WIDTH > rect2.x &&
		rect1.y < rect2.y + CAR_HEIGHT &&
		rect1.y + CAR_HEIGHT > rect2.y)
		return TRUE;
	return FALSE;
}

float distance(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

void parking(int x) {
	int temp = 0, j;
	for (int i = 0; i < noc; i++) {
		if (cars[i].color == x && cars[i].que != -1) {
			fu -= 50;
			cars[i].y = -200 + fu;
			cars[i].que = -2;
		}
		else if (cars[i].que > dth) {
			if (temp == 0) {
				cars[i].que = dth;
				int j = i;
				temp += 1;
			}
			else if(j < i)
				cars[i].que = dth - 1;
			else if(j > i)
				cars[i].que = dth + 1;


			switch (cars[i].que)
			{
			case(1):
				cars[i].x = 83;
				cars[i].y = 5;
				break;
			case(2):
				cars[i].x = 130;
				cars[i].y = 5;
				break;
			case(3):
				cars[i].x = 178;
				cars[i].y = 5;
				break;
			case(4):
				cars[i].x = 224;
				cars[i].y = 5;
				break;
			case(5):
				cars[i].x = 270;
				cars[i].y = 5;
				break;
			default:
				break;
			}
			cars[i].angle = 180;
		}
	}
}

int init_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		fprintf(stderr, "error tf\n");
		return FALSE;
	}

	window = SDL_CreateWindow(
		NULL, 
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		W_WIDTH,
		W_HEIGHT,
		SDL_WINDOW_BORDERLESS
		);
	if (!window) {
		fprintf(stderr, "error window\n");
		return FALSE;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		fprintf(stderr, "error renderer\n");
		return FALSE;
	}

	bg = SDL_LoadBMP("assets/parkingspot.bmp");
	b_car = SDL_LoadBMP("assets/car1.bmp");
	r_car = SDL_LoadBMP("assets/rcar.bmp");
	g_car = SDL_LoadBMP("assets/gcar.bmp");
	if (!bg || !b_car || !r_car || !g_car) {
		fprintf(stderr, "error image\n");
		return FALSE;
	}

	texture1 = SDL_CreateTextureFromSurface(renderer, bg);
	texture2 = SDL_CreateTextureFromSurface(renderer, b_car);
	texture3 = SDL_CreateTextureFromSurface(renderer, r_car);
	texture4 = SDL_CreateTextureFromSurface(renderer, g_car);
	if (!texture1 || !texture2 || !texture3 || !texture4) {
		fprintf(stderr, "error texture\n");
		return FALSE;
	}

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
	music = Mix_LoadMUS("assets/Death by Glamour.mp3");
	if (!music) {
		fprintf(stderr, "error music\n");
		return FALSE;
	}
	return TRUE;
}

void setup() {
	srand(time(NULL));
	r = 0;
	g = 0;
	b = 0;
	dth = 0;

	switch (lvl)
	{
	case(1):
		noc = 3;
		break;
	case(2):
		noc = 6;
		break;
	default:
		break;
	}
	if (lvl >= 3)
		noc = 15;
	win = noc;

	for (int i = 0; i < noc; i++) { 	// Arabalarýn baþlangýç pozisyonlarý
		do {
			coll = FALSE;
			cars[i].x = (W_WIDTH - CAR_WIDTH) / 2 + (rand() % 170 - 75); // starting x
			cars[i].y = (W_HEIGHT - CAR_HEIGHT) / 2 + (rand() % 360 - 130); // starting y

			for (int j = 0; j < i; j++) {     //üst üste araba çýkýþýný engelleme
				if (distance(cars[i].x, cars[i].y, cars[j].x, cars[j].y) < MIN_SPAWN_DISTANCE) {
					coll = TRUE;
					break;
				}
			}
		} while (coll);

		//renk belirleme
		if (b < 3) {
			cars[i].color = 3;  //blue
			b += 1;
		}
		else if (r < 3) {
			cars[i].color = 1;  //red
			r += 1;
		}
		else {
			cars[i].color = 2;  //green
			g += 1;
		}
		cars[i].que = -1;

		// Rastgele yönler ve hýz ayarlamasý
		float angle = (float)(rand() % 360) * (3.14159 / 180); // Radyan cinsinden
		cars[i].angle = (angle * (180/3.14159))-90;
		cars[i].dx = CAR_SPEED * cos(angle);
		cars[i].dy = -CAR_SPEED * sin(angle);

	}

	r = 0;
	g = 0;
	b = 0;
	restart = FALSE;
}

void process_input() {
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
			game_is_running = FALSE;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) {
				game_is_running = FALSE;
				break;
			}
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				int x = event.button.x;
				int y = event.button.y;

				if (x >= 304 && y <= 52) {  //ses aç kapa
					if (sound) {
						Mix_PauseMusic();
						sound = FALSE;
					}
					else {
						Mix_ResumeMusic();
						sound = TRUE;
					}
				}
				else if(y>=55){
					for (int i = 0; i < noc; i++) {
						if (x >= cars[i].x && x <= cars[i].x + CAR_WIDTH && y >= cars[i].y && y <= cars[i].y + CAR_HEIGHT) {
							clicked_car_index = i;
							break;
						}
					}
				}
			}
			break;
		}
	}
}

void update() {
	last_frame_time = SDL_GetTicks();
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
		SDL_Delay(time_to_wait);

	float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f; //arabalarýn fps yerine saniye ile hareket etmesi için bir deðiþken

	// Týklanan arabayý hareket ettir
	if (clicked_car_index != -1) {
		cars[clicked_car_index].x += cars[clicked_car_index].dx * delta_time;
		cars[clicked_car_index].y += cars[clicked_car_index].dy * delta_time;

		// Duvar çarpýþma kontrolü
		if (cars[clicked_car_index].x <= 0 || cars[clicked_car_index].x >= W_WIDTH - CAR_WIDTH ||
			cars[clicked_car_index].y <= 65 || cars[clicked_car_index].y >= W_HEIGHT - CAR_HEIGHT) {

			win -= 1;
			if (win == 0) {
				lvl += 1;
				printf("u win");
				restart = TRUE;
			}

			switch (cars[clicked_car_index].color)
			{
			case(1):
				r += 1;
				break;
			case(2):
				g += 1;
				break;
			case(3):
				b += 1;
				break;
			default:
				break;
			}
			dth += 1;

			if (r == 3)
				dth -= 3;
			else if (g == 3)
				dth -= 3;
			else if (b == 3)
				dth -= 3;

			cars[clicked_car_index].que = dth;

			if (dth == 5)
				printf("try again");
				restart = TRUE;

			//1. - 91,30 /  2. -138,30  / 3. - 186,30 / 4. - 232,30  / 5. - 278,30
			switch (cars[clicked_car_index].que)
			{
			case(1):
				cars[clicked_car_index].x = 83;
				cars[clicked_car_index].y = 5;
				break;
			case(2):
				cars[clicked_car_index].x = 130;
				cars[clicked_car_index].y = 5;
				break;
			case(3):
				cars[clicked_car_index].x = 178;
				cars[clicked_car_index].y = 5;
				break;
			case(4):
				cars[clicked_car_index].x = 224;
				cars[clicked_car_index].y = 5;
				break;
			case(5):
				cars[clicked_car_index].x = 270;
				cars[clicked_car_index].y = 5;
				break;
			default:
				break;
			}
			cars[clicked_car_index].angle = 180;

			if (r == 3) {  //araba yok etme çalýþmalarý
				parking(1);
				r -= 3;
			}
			else if (g == 3) {
				parking(2);
				g -= 3;
			}
			else if (b == 3) {
				parking(3);
				b -= 3;
			}

			printf("%d", dth);
			clicked_car_index = -1; // týklanmýþ arabayý býrakma
		}

		// Diðer arabalarla çarpýþma kontrolü
		for (int i = 0; i < noc; i++) {
			if (cars[i].que == -1) {
				SDL_Rect clicked_car_rect = { (int)cars[clicked_car_index].x, (int)cars[clicked_car_index].y, CAR_WIDTH, CAR_HEIGHT };
				if (i != clicked_car_index) {
					SDL_Rect other_car_rect = { (int)cars[i].x, (int)cars[i].y, CAR_WIDTH, CAR_HEIGHT };
					if (check_collision(clicked_car_rect, other_car_rect)) {
						clicked_car_index = -1;
						restart = TRUE;
						break;
					}
				}
			}
		}
	}
}


void render() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderCopy(renderer, texture1, NULL, NULL);
	SDL_Point center = { CAR_WIDTH / 2,CAR_HEIGHT / 2 };

	if (!sound)
		SDL_RenderDrawLine(renderer, 310, 8, 350, 52);

	for (int i = 0; i < noc; i++) {
		SDL_Rect car_rect = {
			(int)cars[i].x,
			(int)cars[i].y,
			CAR_WIDTH,
			CAR_HEIGHT
		};

		if (cars[i].color == 3)
			SDL_RenderCopyEx(renderer, texture2, NULL, &car_rect, -cars[i].angle, &center, SDL_FLIP_NONE);
		else if (cars[i].color == 2)
			SDL_RenderCopyEx(renderer, texture4, NULL, &car_rect, -cars[i].angle, &center, SDL_FLIP_NONE);
		else if(cars[i].color == 1)
			SDL_RenderCopyEx(renderer, texture3, NULL, &car_rect, -cars[i].angle, &center, SDL_FLIP_NONE);
	}

	SDL_RenderPresent(renderer);
}

void destroy() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture1);
	SDL_DestroyTexture(texture2);
	SDL_DestroyTexture(texture3);
	SDL_FreeSurface(bg);
	SDL_FreeSurface(r_car);
	SDL_FreeSurface(b_car);
	Mix_FreeMusic(music);
	SDL_Quit();
}

int main(int argc, char* args[]) {
	game_is_running = init_window();
	Mix_PlayMusic(music, -1);

	while (game_is_running) {
		if (restart)
			setup();
		process_input();
		update();
		render();
	}

	destroy();
	return 0;
}