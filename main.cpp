#include <SDL.h>
#include <algorithm>
#include <cstdio>

const int WIDTH = 800;
const int HEIGHT = 500;
const float PADDLE_SPEED = 350.0f;
const float BALL_SPEED = 320.0f;

struct Game {
    SDL_FRect left = {30, 200, 14, 100};
    SDL_FRect right = {756, 200, 14, 100};
    SDL_FRect ball = {393, 243, 14, 14};
    float ballXSpeed = BALL_SPEED;
    float ballYSpeed = 170.0f;
    float serveWait = 0.7f;
    int leftScore = 0;
    int rightScore = 0;
    bool paused = false;
};

void resetBall(Game& game, float direction) {
    game.ball.x = (WIDTH - game.ball.w) / 2;
    game.ball.y = (HEIGHT - game.ball.h) / 2;
    game.ballXSpeed = BALL_SPEED * direction;
    game.ballYSpeed = 170.0f;
    game.serveWait = 0.7f;
}

void updateGame(Game& game, float dt, const Uint8* keys) {
    if (game.paused) return;

    if (keys[SDL_SCANCODE_W]) game.left.y -= PADDLE_SPEED * dt;
    if (keys[SDL_SCANCODE_S]) game.left.y += PADDLE_SPEED * dt;
    if (keys[SDL_SCANCODE_UP]) game.right.y -= PADDLE_SPEED * dt;
    if (keys[SDL_SCANCODE_DOWN]) game.right.y += PADDLE_SPEED * dt;

    game.left.y = std::clamp(game.left.y, 0.0f, HEIGHT - game.left.h);
    game.right.y = std::clamp(game.right.y, 0.0f, HEIGHT - game.right.h);

    // Give players a moment to get ready after each point.
    if (game.serveWait > 0) {
        game.serveWait -= dt;
        return;
    }

    game.ball.x += game.ballXSpeed * dt;
    game.ball.y += game.ballYSpeed * dt;

    if (game.ball.y < 0) {
        game.ball.y = 0;
        game.ballYSpeed = -game.ballYSpeed;
    } else if (game.ball.y + game.ball.h > HEIGHT) {
        game.ball.y = HEIGHT - game.ball.h;
        game.ballYSpeed = -game.ballYSpeed;
    }

    // Check direction too, so the ball cannot bounce repeatedly inside a paddle.
    if (game.ballXSpeed < 0 && SDL_HasIntersectionF(&game.ball, &game.left)) {
        game.ball.x = game.left.x + game.left.w;
        game.ballXSpeed = BALL_SPEED;
        float offset = (game.ball.y + game.ball.h / 2 -
                        (game.left.y + game.left.h / 2)) / (game.left.h / 2);
        game.ballYSpeed = offset * 250.0f;
    } else if (game.ballXSpeed > 0 && SDL_HasIntersectionF(&game.ball, &game.right)) {
        game.ball.x = game.right.x - game.ball.w;
        game.ballXSpeed = -BALL_SPEED;
        float offset = (game.ball.y + game.ball.h / 2 -
                        (game.right.y + game.right.h / 2)) / (game.right.h / 2);
        game.ballYSpeed = offset * 250.0f;
    }

    if (game.ball.x + game.ball.w < 0) {
        ++game.rightScore;
        resetBall(game, -1);
    } else if (game.ball.x > WIDTH) {
        ++game.leftScore;
        resetBall(game, 1);
    }
}

int main(int, char**) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL could not start: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Pong", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    if (!window) {
        std::fprintf(stderr, "Window could not open: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::fprintf(stderr, "Renderer could not start: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // These objects are reused. No frame history is collected in the game loop.
    Game game;
    SDL_Event event;
    bool running = true;
    Uint64 previousTime = SDL_GetPerformanceCounter();
    const double frequency = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running) {
        Uint32 frameStart = SDL_GetTicks();
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>((now - previousTime) / frequency);
        previousTime = now;
        // Limit movement after a stall so the ball cannot skip past a paddle.
        dt = std::min(dt, 0.033f);

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
            if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) game.paused = true;
            if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
                if (event.key.keysym.sym == SDLK_SPACE) game.paused = !game.paused;
                if (event.key.keysym.sym == SDLK_r) game = Game{};
            }
        }
        if (!running) break;

        updateGame(game, dt, SDL_GetKeyboardState(nullptr));

        char title[160];
        std::snprintf(title, sizeof(title),
            "Pong | %d : %d | W/S and Up/Down | Space: pause | R: restart%s",
            game.leftScore, game.rightScore, game.paused ? " | PAUSED" : "");
        SDL_SetWindowTitle(window, title);

        SDL_SetRenderDrawColor(renderer, 20, 20, 24, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 75, 75, 80, 255);
        for (int y = 0; y < HEIGHT; y += 25) {
            SDL_Rect dash = {WIDTH / 2 - 2, y, 4, 12};
            SDL_RenderFillRect(renderer, &dash);
        }
        SDL_SetRenderDrawColor(renderer, 240, 240, 240, 255);
        SDL_RenderFillRectF(renderer, &game.left);
        SDL_RenderFillRectF(renderer, &game.right);
        SDL_RenderFillRectF(renderer, &game.ball);
        SDL_RenderPresent(renderer);

        // Avoid running an unlimited loop that keeps the CPU busy.
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) SDL_Delay(16 - frameTime);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
