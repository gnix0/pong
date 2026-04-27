#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define PADDLE_WIDTH 12
#define PADDLE_HEIGHT 72
#define PADDLE_MARGIN 32
#define BALL_SIZE 12
#define PADDLE_SPEED 360.0f
#define BALL_SPEED_X 260.0f
#define BALL_SPEED_Y 160.0f

typedef struct {
	float x;
	float y;
	float vy;
} Paddle;

typedef struct {
	float x;
	float y;
	float vx;
	float vy;
} Ball;

typedef struct {
	Paddle left_paddle;
	Paddle right_paddle;
	Ball ball;
	int left_score;
	int right_score;
} Game;

static float clamp_float(float value, float min, float max) {
	if (value < min) {
		return min;
	}

	if (value > max) {
		return max;
	}

	return value;
}

static void reset_ball(Game *game, int direction) {
	game->ball.x = (WINDOW_WIDTH - BALL_SIZE) / 2.0f;
	game->ball.y = (WINDOW_HEIGHT - BALL_SIZE) / 2.0f;
	game->ball.vx = BALL_SPEED_X * (float)direction;
	game->ball.vy = BALL_SPEED_Y;
}

static void init_game(Game *game) {
	game->left_paddle.x = PADDLE_MARGIN;
	game->left_paddle.y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f;
	game->left_paddle.vy = 0.0f;

	game->right_paddle.x = WINDOW_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH;
	game->right_paddle.y = (WINDOW_HEIGHT - PADDLE_HEIGHT) / 2.0f;
	game->right_paddle.vy = 0.0f;

	game->left_score = 0;
	game->right_score = 0;
	reset_ball(game, 1);
}

static SDL_Rect paddle_rect(Paddle paddle) {
	SDL_Rect rect = {(int)paddle.x, (int)paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT};
	return rect;
}

static SDL_Rect ball_rect(Ball ball) {
	SDL_Rect rect = {(int)ball.x, (int)ball.y, BALL_SIZE, BALL_SIZE};
	return rect;
}

static void update_window_title(SDL_Window *window, const Game *game) {
	char title[64];
	SDL_snprintf(title, sizeof(title), "Pong - %d : %d", game->left_score,
				 game->right_score);
	SDL_SetWindowTitle(window, title);
}

static void handle_input(Game *game, int *running) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			*running = 0;
		}
	}

	const Uint8 *keyboard = SDL_GetKeyboardState(NULL);
	if (keyboard[SDL_SCANCODE_ESCAPE]) {
		*running = 0;
	}

	game->left_paddle.vy = 0.0f;
	if (keyboard[SDL_SCANCODE_W]) {
		game->left_paddle.vy -= PADDLE_SPEED;
	}
	if (keyboard[SDL_SCANCODE_S]) {
		game->left_paddle.vy += PADDLE_SPEED;
	}

	game->right_paddle.vy = 0.0f;
	if (keyboard[SDL_SCANCODE_UP]) {
		game->right_paddle.vy -= PADDLE_SPEED;
	}
	if (keyboard[SDL_SCANCODE_DOWN]) {
		game->right_paddle.vy += PADDLE_SPEED;
	}
}

static void update_game(Game *game, float dt, SDL_Window *window) {
	game->left_paddle.y += game->left_paddle.vy * dt;
	game->right_paddle.y += game->right_paddle.vy * dt;

	game->left_paddle.y = clamp_float(game->left_paddle.y, 0.0f,
									  WINDOW_HEIGHT - PADDLE_HEIGHT);
	game->right_paddle.y = clamp_float(game->right_paddle.y, 0.0f,
									   WINDOW_HEIGHT - PADDLE_HEIGHT);

	game->ball.x += game->ball.vx * dt;
	game->ball.y += game->ball.vy * dt;

	if (game->ball.y <= 0.0f) {
		game->ball.y = 0.0f;
		game->ball.vy = -game->ball.vy;
	}

	if (game->ball.y + BALL_SIZE >= WINDOW_HEIGHT) {
		game->ball.y = WINDOW_HEIGHT - BALL_SIZE;
		game->ball.vy = -game->ball.vy;
	}

	SDL_Rect ball = ball_rect(game->ball);
	SDL_Rect left_paddle = paddle_rect(game->left_paddle);
	SDL_Rect right_paddle = paddle_rect(game->right_paddle);

	if (game->ball.vx < 0.0f && SDL_HasIntersection(&ball, &left_paddle)) {
		game->ball.x = game->left_paddle.x + PADDLE_WIDTH;
		game->ball.vx = -game->ball.vx;
	}

	if (game->ball.vx > 0.0f && SDL_HasIntersection(&ball, &right_paddle)) {
		game->ball.x = game->right_paddle.x - BALL_SIZE;
		game->ball.vx = -game->ball.vx;
	}

	if (game->ball.x + BALL_SIZE < 0.0f) {
		game->right_score++;
		reset_ball(game, 1);
		update_window_title(window, game);
	}

	if (game->ball.x > WINDOW_WIDTH) {
		game->left_score++;
		reset_ball(game, -1);
		update_window_title(window, game);
	}
}

static void render_game(SDL_Renderer *renderer, const Game *game) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_Rect left_paddle = paddle_rect(game->left_paddle);
	SDL_Rect right_paddle = paddle_rect(game->right_paddle);
	SDL_Rect ball = ball_rect(game->ball);

	SDL_RenderFillRect(renderer, &left_paddle);
	SDL_RenderFillRect(renderer, &right_paddle);
	SDL_RenderFillRect(renderer, &ball);

	SDL_RenderPresent(renderer);
}

int main(void) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_Log("SDL_Init error: %s", SDL_GetError());
		return 1;
	}

	SDL_Window *pwindow =
		SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
						 WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);

	if (!pwindow) {
		SDL_Log("SDL_CreateWindow error: %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	SDL_Renderer *prenderer =
		SDL_CreateRenderer(pwindow, -1, SDL_RENDERER_ACCELERATED);
	if (!prenderer) {
		SDL_Log("SDL_CreateRenderer error: %s", SDL_GetError());
		SDL_DestroyWindow(pwindow);
		SDL_Quit();
		return 1;
	}

	Game game;
	init_game(&game);
	update_window_title(pwindow, &game);

	int running = 1;
	Uint32 previous_ticks = SDL_GetTicks();

	while (running) {
		Uint32 current_ticks = SDL_GetTicks();
		float dt = (current_ticks - previous_ticks) / 1000.0f;
		previous_ticks = current_ticks;

		handle_input(&game, &running);
		update_game(&game, dt, pwindow);
		render_game(prenderer, &game);
	}

	SDL_DestroyRenderer(prenderer);
	SDL_DestroyWindow(pwindow);
	SDL_Quit();
	return 0;
}
