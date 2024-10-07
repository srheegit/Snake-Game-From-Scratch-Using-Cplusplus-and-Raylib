#include <iostream>
#include <raylib.h>
#include <raymath.h>
#include <deque>

using namespace std;

/*
Using the raylib library, we create the classic snake game.

We shall do this in ten stages:
1. Create a blank canvas and game loop
2. Create the food
3. Create the snake
4. Move the snake
5. Make the snake eat the food
6. Make the snake grow longer
7. Check for collision with edges and tail
8. Add title and frame
9. Keep score
10. Add sounds

Any game has two stages: defining variables and running the game loop.

In our game, the game loop occurs in three steps:

1. Even handling: First, we need to check for any events that occur in the game, such as quitting the game, a key pressed on the keyboard, etc.
2. Updating positions: We update the positions of all game objects, such as the snake and food, based on the events we detected in step 1.
3. Drawing objects: We draw all the game objects in their ne wpositions on the screen. This step uses the raylib graphics functions to render the bojects in the display.

*/

// in raylib, the colors are stored as a struct called Color {r, g, b, a}, each being unsigned char (8 bytes = 256 bits)
// we shall use green and dark green
Color green = {173, 204, 96, 255};
Color darkGreen = {43, 51, 24, 255};

// We shall divide the 750x750 pixels of window into 25x25 cells, each of size 30x30 pixels
int cellSize = 30;
int cellCount = 25;
int offset = 75;

// the following variable will be used so that the snake updates every once in a while, and not every frame
// the reason we declare this variable is that the sanke will move way too fast if we update it every frame
double lastUpdateTime = 0;

// checks if a certain interval of time has passed since the last update
bool eventTriggered(double interval) {
    double currentTime = GetTime();
    if(currentTime - lastUpdateTime >= interval) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

// checks if an element is already in deque. This will be used in
// (1) ensuring that a new food generated will not be on top of the body of the snake, and
// (2) checking if a snake's head touches its own body
bool ElementInDeque(Vector2 element, deque<Vector2> deque) {
    for (unsigned int i = 0; i < deque.size(); ++i) {
        if (Vector2Equals(deque[i], element)) {
            return true;
        }
    }
    return false;
}

class Food {
    public:
        // use the Vector2 class provided by raylib library, to store the position of the food
        // the x and y coordinates can be accessed by calling position.x and position.y, respectively
        Vector2 position;

        // in raylib, a Texture2D is an optimized data type for GPU processing and is used for faster rendering of images and textures in various graphic applications like video games
        Texture2D texture;

        // constructor
        Food(deque<Vector2> snakeBody) {
            // in raylib, an image is a dat structure that contains the pixel data of a graphical image
            Image image = LoadImage("image/food.png");
            texture = LoadTextureFromImage(image);
            // unload the image to free some memory
            UnloadImage(image);
            position = GenerateRandomPos(snakeBody);
        }

        // destructor
        ~Food() {
            UnloadTexture(texture);
        }

        void Draw() {
            DrawTexture(texture, offset + position.x*cellSize, offset + position.y*cellSize, WHITE);
        }

        // function that generates random cell in the window
        Vector2 GenerateRandomCell() {
            float x = GetRandomValue(0, cellCount - 1);
            float y = GetRandomValue(0, cellCount - 1);
            return Vector2{x,y};
        }

        // function for generating random position. Ensures that the food never appears on top of the snake's body
        Vector2 GenerateRandomPos(deque<Vector2> snakeBody) {
            Vector2 position = GenerateRandomCell();
            while (ElementInDeque(position, snakeBody)) {
                position = GenerateRandomCell();
            }
            return position;
        }
};

class Snake {
    public:
        // we use deque for the body, since it's efficient to pop and push, for back and front
        // body.front() - i.e. the first element - is the head, while body.back() - i.e. the last element - is the tail
        deque<Vector2> body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
        // direction of the user input
        Vector2 direction = {1, 0};

        bool addSegment = false;

        void Draw() {
            for (unsigned int i = 0; i < body.size(); ++i) {
                float x = body[i].x;
                float y = body[i].y;
                Rectangle segment = Rectangle{offset + x*cellSize, offset + y*cellSize, (float)cellSize, (float)cellSize};
                DrawRectangleRounded(segment, 0.5, 6, darkGreen);
            }
        }

        // update the body of the snake; move it 1 to the direction if the snake didn't eat the food, and increase its length by 1 if it ate
        void Update() {

            body.push_front(Vector2Add(body.at(0), direction));
            if (addSegment == true) {
                addSegment = false;
            }
            else {
                body.pop_back();
            }
        }

        void Reset() {
            body = {Vector2{6,9}, Vector2{5,9}, Vector2{4,9}};
            direction = {1,0};
        }
};

class Game {
    public:
        Snake snake = Snake();
        Food food = Food(snake.body);
        // variable to check if the game is running
        bool running = true;
        int score = 0;
        int maxScore = 0;
        Sound eatSound;
        Sound wallSound;

        Game() {

            //initializes the audio device for playing sound in a game
            InitAudioDevice();
            eatSound = LoadSound("sounds/eat.mp3");
            wallSound = LoadSound("sounds/wall.mp3");
        }

        ~Game() {
            UnloadSound(eatSound);
            UnloadSound(wallSound);
            CloseAudioDevice();
        }

        void Draw() {
            food.Draw();
            snake.Draw();
        }
        void Update() {
            if (running) {
                snake.Update();
                CheckCollisionsWithFood();
                CheckCollisionsWithEdges();
                CheckCollisionWithTail();
            }
            
        }

        void CheckCollisionsWithFood() {
            if (Vector2Equals(snake.body[0], food.position)) {
                food.position = food.GenerateRandomPos(snake.body);
                // notify the snake object that it needs to grow in size
                snake.addSegment = true;
                score++;
                if (score > maxScore) {
                    maxScore++;
                }
                PlaySound(eatSound);
            }
        }

        void CheckCollisionsWithEdges() {
            if (snake.body.at(0).x == cellCount || snake.body.at(0).x == -1) {
                GameOver();
            }
            else if (snake.body.at(0).y == cellCount || snake.body.at(0).y == -1) {
                GameOver();
            }

        }

        void GameOver() {
            snake.Reset();
            food.position = food.GenerateRandomPos(snake.body);
            running = false;
            if (score > maxScore) {
                maxScore = score;
            }
            score = 0;
            PlaySound(wallSound);
        }

        void CheckCollisionWithTail() {
            deque<Vector2> headlessBody = snake.body;
            headlessBody.pop_front();
            if (ElementInDeque(snake.body.at(0), headlessBody)) {
                GameOver();
            }
        }
};

int main()
{
    // game start message
    cout << "Starting the game..." << endl;

    // Initalize the game window with certain width, height, and window title
    InitWindow(2*offset + cellSize*cellCount, 2*offset + cellSize*cellCount, "Snake Game");

    // Set the Frames Per Second for the game
    SetTargetFPS(60);

    Game game = Game();

    // game loop that runs while the window is open
    while(WindowShouldClose() == false) {
        BeginDrawing();

        if (eventTriggered(0.2)) {
            game.Update();
        }

        // change the direction whenever a key is pressed accordingly
        if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1) {
            game.snake.direction = Vector2{0,-1};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1) {
            game.snake.direction = Vector2{0, 1};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1) {
            game.snake.direction = Vector2{1, 0};
            game.running = true;
        }
        else if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1) {
            game.snake.direction = Vector2{-1, 0};
            game.running = true;
        }

        // Setting up the background after creating the game loop
        ClearBackground(green);

        // Draw the borders
        DrawRectangleLinesEx(Rectangle{(float)offset-5, (float)offset-5, (float)cellSize*cellCount + 10, (float)cellSize*cellCount + 10}, 5, darkGreen);

        // set title, score, and max score of the game
        DrawText("Classic Snake Game", offset - 5, 20, 40, darkGreen);
        DrawText("Score: ", offset - 5, offset + cellSize*cellCount + 10, 40, darkGreen);
        DrawText(TextFormat("%i", game.score), 230, offset + cellSize*cellCount + 10, 40, darkGreen);
        DrawText("Max Score: ", 500, offset + cellSize*cellCount + 10, 40, darkGreen);
        DrawText(TextFormat("%i", game.maxScore), 750, offset + cellSize*cellCount + 10, 40, darkGreen);

        // Draw the game
        game.Draw();


        EndDrawing();
    }





    //Close the game window
    CloseWindow();
    return 0;
}