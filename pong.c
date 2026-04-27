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
#define BALL_SPEEDUP 1.06f
#define MAX_BALL_SPEED_X 560.0f
#define MAX_BALL_SPEED_Y 420.0f
#define WINNING_SCORE 5
#define CENTER_LINE_WIDTH 4
#define CENTER_LINE_HEIGHT 18
#define CENTER_LINE_GAP 12
#define DIGIT_WIDTH 30
#define DIGIT_HEIGHT 54
#define DIGIT_THICKNESS 6

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
	int game_over;
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
	game->ball.vy = BALL_SPEED_Y * (direction > 0 ? 1.0f : -1.0f);
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
	game->game_over = 0;
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

	if (game->game_over) {
		const char *winner =
			game->left_score > game->right_score ? "Left" : "Right";
		SDL_snprintf(title, sizeof(title), "Pong - %s wins - Space to restart",
					 winner);
	} else {
		SDL_snprintf(title, sizeof(title), "Pong - %d : %d", game->left_score,
					 game->right_score);
	}

	SDL_SetWindowTitle(window, title);
}

static void handle_input(Game *game, int *running, SDL_Window *window) {
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			*running = 0;
		}

		if (event.type == SDL_KEYDOWN &&
			event.key.keysym.scancode == SDL_SCANCODE_SPACE &&
			game->game_over) {
			init_game(game);
			update_window_title(window, game);
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

static void bounce_from_paddle(Ball *ball, const Paddle *paddle, int direction) {
	float paddle_center = paddle->y + PADDLE_HEIGHT / 2.0f;
	float ball_center = ball->y + BALL_SIZE / 2.0f;
	float hit_position = (ball_center - paddle_center) / (PADDLE_HEIGHT / 2.0f);
	float speed_x = ball->vx < 0.0f ? -ball->vx : ball->vx;

	hit_position = clamp_float(hit_position, -1.0f, 1.0f);
	speed_x = clamp_float(speed_x * BALL_SPEEDUP, BALL_SPEED_X, MAX_BALL_SPEED_X);

	ball->vx = speed_x * (float)direction;
	ball->vy = clamp_float(hit_position * MAX_BALL_SPEED_Y, -MAX_BALL_SPEED_Y,
						   MAX_BALL_SPEED_Y);
}

static void award_point(Game *game, int left_player_scored, SDL_Window *window) {
	if (left_player_scored) {
		game->left_score++;
		reset_ball(game, -1);
	} else {
		game->right_score++;
		reset_ball(game, 1);
	}

	if (game->left_score >= WINNING_SCORE || game->right_score >= WINNING_SCORE) {
		game->game_over = 1;
		game->ball.vx = 0.0f;
		game->ball.vy = 0.0f;
	}

	update_window_title(window, game);
}

static void update_game(Game *game, float dt, SDL_Window *window) {
	if (game->game_over) {
		return;
	}

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
		bounce_from_paddle(&game->ball, &game->left_paddle, 1);
	}

	if (game->ball.vx > 0.0f && SDL_HasIntersection(&ball, &right_paddle)) {
		game->ball.x = game->right_paddle.x - BALL_SIZE;
		bounce_from_paddle(&game->ball, &game->right_paddle, -1);
	}

	if (game->ball.x + BALL_SIZE < 0.0f) {
		award_point(game, 0, window);
	}

	if (game->ball.x > WINDOW_WIDTH) {
		award_point(game, 1, window);
	}
}

static void fill_rect(SDL_Renderer *renderer, int x, int y, int w, int h) {
	SDL_Rect rect = {x, y, w, h};
	SDL_RenderFillRect(renderer, &rect);
}

static void render_digit(SDL_Renderer *renderer, int digit, int x, int y,
						 int scale) {
	static const int segments[10][7] = {
		{1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0},
		{1, 1, 0, 1, 1, 0, 1}, {1, 1, 1, 1, 0, 0, 1},
		{0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
		{1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0},
		{1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 0, 1, 1},
	};
	int width = DIGIT_WIDTH * scale;
	int height = DIGIT_HEIGHT * scale;
	int thickness = DIGIT_THICKNESS * scale;

	if (digit < 0 || digit > 9) {
		return;
	}

	if (segments[digit][0]) {
		fill_rect(renderer, x + thickness, y, width - 2 * thickness, thickness);
	}
	if (segments[digit][1]) {
		fill_rect(renderer, x + width - thickness, y + thickness, thickness,
				  height / 2 - thickness);
	}
	if (segments[digit][2]) {
		fill_rect(renderer, x + width - thickness, y + height / 2, thickness,
				  height / 2 - thickness);
	}
	if (segments[digit][3]) {
		fill_rect(renderer, x + thickness, y + height - thickness,
				  width - 2 * thickness, thickness);
	}
	if (segments[digit][4]) {
		fill_rect(renderer, x, y + height / 2, thickness,
				  height / 2 - thickness);
	}
	if (segments[digit][5]) {
		fill_rect(renderer, x, y + thickness, thickness,
				  height / 2 - thickness);
	}
	if (segments[digit][6]) {
		fill_rect(renderer, x + thickness, y + height / 2 - thickness / 2,
				  width - 2 * thickness, thickness);
	}
}

static void render_center_line(SDL_Renderer *renderer) {
	int x = WINDOW_WIDTH / 2 - CENTER_LINE_WIDTH / 2;

	for (int y = 0; y < WINDOW_HEIGHT;
		 y += CENTER_LINE_HEIGHT + CENTER_LINE_GAP) {
		fill_rect(renderer, x, y, CENTER_LINE_WIDTH, CENTER_LINE_HEIGHT);
	}
}

static void render_score(SDL_Renderer *renderer, const Game *game) {
	render_digit(renderer, game->left_score % 10, WINDOW_WIDTH / 2 - 82, 24, 2);
	render_digit(renderer, game->right_score % 10, WINDOW_WIDTH / 2 + 22, 24, 2);
}

static void render_game_over(SDL_Renderer *renderer, const Game *game) {
	int winner_x = game->left_score > game->right_score ? WINDOW_WIDTH / 2 - 78
														: WINDOW_WIDTH / 2 + 18;

	fill_rect(renderer, winner_x, 92, 72, 6);
	fill_rect(renderer, winner_x, 92, 6, 34);
	fill_rect(renderer, winner_x + 66, 92, 6, 34);
	fill_rect(renderer, winner_x + 14, 112, 44, 6);

	fill_rect(renderer, WINDOW_WIDTH / 2 - 86, WINDOW_HEIGHT - 82, 172, 6);
	fill_rect(renderer, WINDOW_WIDTH / 2 - 86, WINDOW_HEIGHT - 58, 172, 6);
	fill_rect(renderer, WINDOW_WIDTH / 2 - 86, WINDOW_HEIGHT - 82, 6, 30);
	fill_rect(renderer, WINDOW_WIDTH / 2 + 80, WINDOW_HEIGHT - 82, 6, 30);
}

static void render_game(SDL_Renderer *renderer, const Game *game) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	render_center_line(renderer);
	render_score(renderer, game);

	SDL_Rect left_paddle = paddle_rect(game->left_paddle);
	SDL_Rect right_paddle = paddle_rect(game->right_paddle);
	SDL_Rect ball = ball_rect(game->ball);

	SDL_RenderFillRect(renderer, &left_paddle);
	SDL_RenderFillRect(renderer, &right_paddle);
	SDL_RenderFillRect(renderer, &ball);

	if (game->game_over) {
		render_game_over(renderer, game);
	}

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

		handle_input(&game, &running, pwindow);
		update_game(&game, dt, pwindow);
		render_game(prenderer, &game);
	}

	SDL_DestroyRenderer(prenderer);
	SDL_DestroyWindow(pwindow);
	SDL_Quit();
	return 0;
}
