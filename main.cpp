#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <random>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const float GROUND_HEIGHT = 50.0f;

class Dino {
public:
    Dino() : isJumping(false), isFalling(false), velocity(0.0f, 0.0f), jumpDuration(0.5f), jumpHeight(250.0f), jumpElapsedTime(0.0f) {
        shape.setSize(sf::Vector2f(50.0f, 50.0f));
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(100.0f, WINDOW_HEIGHT - GROUND_HEIGHT - shape.getSize().y);
        jumpVelocity = -2 * jumpHeight / jumpDuration;
        gravity = 2 * jumpHeight / (jumpDuration * jumpDuration);
    }

    void handleInput() {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && !isJumping && !isFalling) {
            isJumping = true;
            jumpElapsedTime = 0.0f;
            clock.restart();
            velocity.y = jumpVelocity;
        }
    }

    void update(float deltaTime) {
        if (isJumping || isFalling) {
            jumpElapsedTime += deltaTime;

            if (isJumping) {
                velocity.y += gravity * deltaTime;
                if (jumpElapsedTime >= jumpDuration) {
                    isJumping = false;
                    isFalling = true;
                }
            }
            else if (isFalling) {
                velocity.y += gravity * deltaTime;
            }

            shape.move(0, velocity.y * deltaTime);

            if (shape.getPosition().y >= WINDOW_HEIGHT - GROUND_HEIGHT - shape.getSize().y) {
                shape.setPosition(shape.getPosition().x, WINDOW_HEIGHT - GROUND_HEIGHT - shape.getSize().y);
                isJumping = false;
                isFalling = false;
                velocity.y = 0.0f;
            }
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    bool isJumping;
    bool isFalling;
    float jumpVelocity;
    float gravity;
    sf::Clock clock;
    float jumpDuration;
    float jumpHeight;
    float jumpElapsedTime;
};

class Obstacle {
public:
    Obstacle(float startX) : isOffScreen(false), speed(-200.0f) {
        shape.setSize(sf::Vector2f(20.0f, 50.0f));
        shape.setFillColor(sf::Color::Green);
        shape.setPosition(startX, WINDOW_HEIGHT - GROUND_HEIGHT - shape.getSize().y);
    }

    void update(float deltaTime) {
        shape.move(speed * deltaTime, 0);
        if (shape.getPosition().x + shape.getSize().x < 0) {
            isOffScreen = true;
        }
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);
    }

    sf::FloatRect getBounds() const {
        return shape.getGlobalBounds();
    }

    bool isOffScreen;

private:
    sf::RectangleShape shape;
    float speed;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Red Square");

    // Load background texture
    sf::Texture backgroundTexture;
    if (!backgroundTexture.loadFromFile("background.png")) {
        return -1; // Error loading texture
    }
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    Dino dino;
    std::vector<Obstacle> obstacles;

    sf::RectangleShape ground;
    ground.setSize(sf::Vector2f(WINDOW_WIDTH, GROUND_HEIGHT));
    ground.setFillColor(sf::Color(150, 75, 0));
    ground.setPosition(0, WINDOW_HEIGHT - GROUND_HEIGHT);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 3);

    sf::Clock spawnClock;
    float nextSpawnTime = dist(gen);

    sf::Clock gameClock;
    bool gameOver = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (!gameOver) {
            dino.handleInput();
            float deltaTime = gameClock.restart().asSeconds();
            dino.update(deltaTime);

            if (spawnClock.getElapsedTime().asSeconds() > nextSpawnTime) {
                obstacles.emplace_back(WINDOW_WIDTH);
                nextSpawnTime = dist(gen);
                spawnClock.restart();
            }

            for (auto& obstacle : obstacles) {
                obstacle.update(deltaTime);
            }

            obstacles.erase(std::remove_if(obstacles.begin(), obstacles.end(), [](Obstacle& obstacle) {
                return obstacle.isOffScreen;
                }), obstacles.end());

            // Check for collisions
            for (auto& obstacle : obstacles) {
                if (dino.getBounds().intersects(obstacle.getBounds())) {
                    gameOver = true;
                    break;
                }
            }
        }

        window.clear();
        window.draw(backgroundSprite); // Draw background
        dino.draw(window);
        for (auto& obstacle : obstacles) {
            obstacle.draw(window);
        }
        window.draw(ground);
        window.display();
    }

    return 0;
}
