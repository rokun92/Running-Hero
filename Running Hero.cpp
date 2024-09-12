#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <memory>

using namespace std;

int run = 1;
int score = 0, highScore = 0;
bool isJumping = false;
bool isFalling = false;
bool gameOver = false;
bool paused = false; // Paused state
float jumpSpeed = 0;
const float gravity = 0.6;
const float GROUND_LEVEL = 500.0f;
float speedMultiplier = 1.0f;

sf::RenderWindow window(sf::VideoMode(800, 600), "RUNNING HERO");

sf::Font font;
sf::Texture birdTexture; // Bird texture
sf::Sprite birdSprite;   // Bird sprite
sf::Texture backgroundTexture; // Background texture
sf::Sprite backgroundSprite;   // Background sprite

vector<unique_ptr<sf::Shape>> obstacles; // Obstacles using smart pointers
vector<bool> passedObstacles;            // Track passed obstacles

// Function to create a new obstacle
unique_ptr<sf::Shape> createObstacle() {
    int shapeType = rand() % 3;
    unique_ptr<sf::Shape> newObstacle;

    if (shapeType == 0) {
        // Rectangle
        int width = rand() % 40 + 20;
        int height = rand() % 40 + 30;
        newObstacle = make_unique<sf::RectangleShape>(sf::Vector2f(width, height));
    }
    else if (shapeType == 1) {
        // Circle
        int radius = rand() % 20 + 10;
        newObstacle = make_unique<sf::CircleShape>(radius);
    }
    else if (shapeType == 2) {
        // Triangle
        auto triangle = make_unique<sf::ConvexShape>(3);
        triangle->setPoint(0, sf::Vector2f(20, 0));
        triangle->setPoint(1, sf::Vector2f(0, 40));
        triangle->setPoint(2, sf::Vector2f(40, 40));
        newObstacle = move(triangle);
    }

    return newObstacle;
}

void resetGame() {
    birdSprite.setPosition(200, GROUND_LEVEL - birdSprite.getGlobalBounds().height);
    obstacles.clear();
    passedObstacles.clear();
    isJumping = false;
    isFalling = false;
    jumpSpeed = 0;
    score = 0;
    speedMultiplier = 1.5f; // Reset speed multiplier
}

void processInput() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            run = 0;

        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                run = 0;
            }
            if (event.key.code == sf::Keyboard::P) {
                paused = !paused;
            }

            if (event.key.code == sf::Keyboard::Space && !isJumping && !isFalling && !gameOver && !paused) {
                isJumping = true;
                jumpSpeed = -9 * speedMultiplier;
            }
            else if (event.key.code == sf::Keyboard::Space && gameOver && !paused) {
                resetGame();
                gameOver = false;
            }
        }
    }
}

void update() {
    if (gameOver || paused) return; // Do not update if the game is over or paused

    speedMultiplier = min(speedMultiplier + 0.001f, 3.0f); // Limit speed

    if (isJumping) {
        birdSprite.move(0, jumpSpeed);
        jumpSpeed += gravity * speedMultiplier;
        if (jumpSpeed >= 0) {
            isJumping = false;
            isFalling = true;
        }
    }
    else if (isFalling) {
        birdSprite.move(0, jumpSpeed);
        jumpSpeed += gravity * speedMultiplier;
        if (birdSprite.getPosition().y >= GROUND_LEVEL - birdSprite.getGlobalBounds().height) {
            birdSprite.setPosition(birdSprite.getPosition().x, GROUND_LEVEL - birdSprite.getGlobalBounds().height);
            isFalling = false;
            jumpSpeed = 0;
        }
    }

    // Obstacle logic
    for (size_t i = 0; i < obstacles.size(); ++i) {
        obstacles[i]->move(-5 * speedMultiplier, 0);

        // Check if the bird has passed the obstacle
        if (!passedObstacles[i] && birdSprite.getPosition().x > obstacles[i]->getPosition().x + obstacles[i]->getGlobalBounds().width) {
            passedObstacles[i] = true;
            score++;
        }
    }

    // Add new obstacles dynamically
    if (obstacles.empty() || obstacles.back()->getPosition().x < 600) {
        int minSpacing = 200;
        int maxSpacing = 400;
        int spacing = rand() % (maxSpacing - minSpacing) + minSpacing;

        if (!obstacles.empty()) {
            spacing += obstacles.back()->getGlobalBounds().width;
        }

        unique_ptr<sf::Shape> newObstacle = createObstacle();
        newObstacle->setPosition(800 + spacing, GROUND_LEVEL - newObstacle->getGlobalBounds().height);

        sf::Color obstacleColor(rand() % 256, rand() % 256, rand() % 256);
        newObstacle->setFillColor(obstacleColor);

        obstacles.push_back(move(newObstacle));
        passedObstacles.push_back(false); // Initially not passed
    }

    // Remove obstacles that go off-screen
    if (!obstacles.empty() && obstacles.front()->getPosition().x < -20) {
        obstacles.erase(obstacles.begin());
        passedObstacles.erase(passedObstacles.begin());
    }

    // Collision detection
    for (auto& obstacle : obstacles) {
        if (birdSprite.getGlobalBounds().intersects(obstacle->getGlobalBounds())) {
            gameOver = true;
            if (score > highScore) {
                highScore = score;
            }
            return;
        }
    }
}

void render() {
    window.clear();

    // Draw the background
    window.draw(backgroundSprite);

    // Draw the bird sprite
    window.draw(birdSprite);

    // Draw obstacles
    for (auto& obstacle : obstacles) {
        window.draw(*obstacle);
    }

    // Draw score
    string scr = "Score: " + to_string(score);
    sf::Text scoreText(scr, font, 20);
    scoreText.setFillColor(sf::Color::Black);
    scoreText.setPosition(10, 10);
    window.draw(scoreText);

    // Draw high score
    string hi = "High Score: " + to_string(highScore);
    sf::Text highScoreText(hi, font, 20);
    highScoreText.setFillColor(sf::Color::Black);
    highScoreText.setPosition(10, 40);
    window.draw(highScoreText);

    // Draw game over message
    if (gameOver) {
        sf::Text gameOverText("Game Over! Press Space to restart", font, 30);
        gameOverText.setFillColor(sf::Color::Red);
        sf::FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        gameOverText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
        window.draw(gameOverText);
    }

    // Draw paused message
    if (paused) {
        sf::Text pausedText("Game Paused! Press P to resume", font, 30);
        pausedText.setFillColor(sf::Color::Blue);
        sf::FloatRect textRect = pausedText.getLocalBounds();
        pausedText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        pausedText.setPosition(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
        window.draw(pausedText);
    }

    window.display();
}

void destroyWindow() {
    window.close();
}

int main() {
    if (!font.loadFromFile("C:/Users/ALPHA TECH/source/repos/Running Hero/arial-font/arial.ttf")) {
        cerr << "Error loading font" << endl;
        return -1;
    }

    if (!birdTexture.loadFromFile("C:/Users/ALPHA TECH/source/repos/Running Hero/hero.png")) {
        cerr << "Error loading bird image" << endl;
        return -1;
    }
    birdSprite.setTexture(birdTexture);
    birdSprite.setScale(0.3f, 0.3f); // Flip the sprite horizontally

    /*if (!backgroundTexture.loadFromFile("C:/Users/ALPHA TECH/source/repos/Running Hero/background.jpg")) {
        cerr << "Error loading background image" << endl;
        return -1;
    }*/
    backgroundSprite.setTexture(backgroundTexture);

    // Scale the background to fit the window
    sf::Vector2u backgroundSize = backgroundTexture.getSize();
    sf::Vector2u windowSize = window.getSize();
    backgroundSprite.setScale(
        static_cast<float>(windowSize.x) / backgroundSize.x,
        static_cast<float>(windowSize.y) / backgroundSize.y
    );

    srand(static_cast<unsigned int>(time(0)));

    resetGame();

    sf::Clock clock;
    const float frameDuration = 1.0f / 60.0f; // 60 FPS

    while (run) {
        processInput();
        update();
        render();

        sf::Time elapsed = clock.getElapsedTime();
        if (elapsed.asSeconds() < frameDuration) {
            sf::sleep(sf::seconds(frameDuration - elapsed.asSeconds()));
        }
        clock.restart();
    }

    destroyWindow();
    return 0;
}
