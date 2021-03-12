#include "GameLogic.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <array>
#include <cmath>
#include <ctime>
#include <random>
#include <vector>

// Must be an odd number
constexpr int board_size{ 3 };
constexpr int difficulty{ 4 };
constexpr float speed_multiplier{ 1.5f };
constexpr float base_wait{ 1.5f };
constexpr float wait_multiplier{ 0.75f };

class Fireball
{
public:
    enum class Side
    {
        Top,
        Bottom,
        Left,
        Right
    };

    Fireball(Side side, const sf::Texture& texture, int speed)
        : side{ side }, sprite{ texture }, speed{ speed }
    {
        
    }

    Side side;
    sf::Time wait{ sf::seconds(base_wait) };
    sf::Sprite sprite;
    int speed;
};

int main()
{
    // Initalize our window to the correct size and prevent resizing
    sf::VideoMode desktop{ sf::VideoMode::getDesktopMode() };
    // Store size constants for future use
    const unsigned int window_size{ desktop.height * 2 / 3 };
    const float radius{ window_size / (board_size * 4.0f) };
    const int base_speed{ static_cast<int>(board_size * radius) };
    sf::RenderWindow window(sf::VideoMode(window_size, window_size), "Fire Hodge", sf::Style::Titlebar | sf::Style::Close);

    // Load in the character texture and sprite
    sf::Texture character_texture;
    if (!character_texture.loadFromFile("character.png"))
        return 1;
    sf::Sprite character{ character_texture };
    character.setScale((window_size / (board_size * 2.0f)) / character.getLocalBounds().width, (window_size / (board_size * 2.0f)) / character.getLocalBounds().height);
    character.setPosition(window_size / 2.0f - radius, window_size / 2.0f - radius);

    // Load in the spots
    std::array<sf::CircleShape, board_size * board_size> spots;
    for (int i{ 0 }; i < board_size; ++i)
    {
        for (int j{ 0 }; j < board_size; ++j)
        {
            spots[i * board_size + j] = sf::CircleShape{ radius };
            spots[i * board_size + j].setPosition(window_size / (board_size + 1.0f) * (j + 1) - radius, window_size / (board_size + 1.0f) * (i + 1) - radius);
        }
    }

    std::mt19937 twister{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
    std::uniform_int_distribution randomizer(1, board_size);

    // Load in the fireballs
    sf::Texture fireball_texture;
    if (!fireball_texture.loadFromFile("fireball.png"))
        return 2;
    std::vector<Fireball> fireballs;
    for (int i{ 0 }; i < 4 * (board_size / 2); ++i)
    {
        if (i % 4 == 0)
        {
            fireballs.push_back(Fireball(Fireball::Side::Top, fireball_texture, static_cast<int>(std::pow(speed_multiplier, difficulty) * base_speed)));
        }
        if (i % 4 == 1)
        {
            fireballs.push_back(Fireball(Fireball::Side::Bottom, fireball_texture, static_cast<int>(std::pow(speed_multiplier, difficulty) * base_speed)));
        }
        if (i % 4 == 2)
        {
            fireballs.push_back(Fireball(Fireball::Side::Left, fireball_texture, static_cast<int>(std::pow(speed_multiplier, difficulty) * base_speed)));
        }
        if (i % 4 == 3)
        {
            fireballs.push_back(Fireball(Fireball::Side::Right, fireball_texture, static_cast<int>(std::pow(speed_multiplier, difficulty) * base_speed)));
        }
        fireballs[i].sprite.setScale((window_size / (board_size * 2.0f)) / fireballs[i].sprite.getLocalBounds().width, (window_size / (board_size * 2.0f)) / fireballs[i].sprite.getLocalBounds().height);
    }

    //Load in the sounds
    sf::SoundBuffer bonk_buffer;
    if (!bonk_buffer.loadFromFile("death.wav"))
        return 3;
    sf::Sound bonk;
    bonk.setBuffer(bonk_buffer);
    sf::Music music;
    if (!music.openFromFile("battle.wav"))
        return 4;

    bool gameActive{ false };
    int charX{ board_size / 2 };
    int charY{ board_size / 2 };

    sf::Clock clock;
    while (window.isOpen())
    {
        while (!gameActive)
        {
            // Print everything to the screen
            window.clear(sf::Color::Cyan);
            for (auto& s : spots)
                window.draw(s);
            for (auto& f : fireballs)
            {
                if (f.wait < sf::seconds(base_wait * static_cast<float>(std::pow(wait_multiplier, difficulty))))
                    window.draw(f.sprite);
            }
            window.draw(character);
            window.display();

            sf::Event event;
            while (window.pollEvent(event))
            {
                switch (event.type)
                {
                case sf::Event::Closed:
                    window.close();
                    return 0;
                    break;
                    // Handle movement
                case sf::Event::KeyPressed:
                    if (event.key.code == sf::Keyboard::Space)
                    {
                        gameActive = true;
                        charX = board_size / 2;
                        charY = board_size / 2;
                        character.setPosition(window_size / 2.0f - radius, window_size / 2.0f - radius);

                        for (auto& f : fireballs)
                        {
                            if (f.side == Fireball::Side::Top)
                            {
                                f.wait = sf::seconds(base_wait);
                                f.sprite.setPosition(randomizer(twister) * (window_size / (board_size + 1.0f)) - radius, -radius);
                            }
                            if (f.side == Fireball::Side::Bottom)
                            {
                                f.wait = sf::seconds(base_wait);
                                f.sprite.setPosition(randomizer(twister) * (window_size / (board_size + 1.0f)) - radius, window_size - radius);
                            }
                            if (f.side == Fireball::Side::Left)
                            {
                                f.wait = sf::seconds(base_wait);
                                f.sprite.setPosition(-radius, randomizer(twister) * (window_size / (board_size + 1.0f)) - radius);
                            }
                            if (f.side == Fireball::Side::Right)
                            {
                                f.wait = sf::seconds(base_wait);
                                f.sprite.setPosition(window_size - radius, randomizer(twister) * (window_size / (board_size + 1.0f)) - radius);
                            }

                            music.play();
                        }
                    }
                    break;
                }
            }
        }


        while (gameActive)
        {
            // Timer to allow for constant speeds
            sf::Time elapsed{ clock.restart() };

            // Event Handler
            sf::Event event;
            while (window.pollEvent(event))
            {
                switch (event.type)
                {
                case sf::Event::Closed:
                    window.close();
                    return 0;
                    break;
                    // Handle movement
                case sf::Event::KeyPressed:
                    switch (event.key.code)
                    {
                    case sf::Keyboard::Up:
                        if (charY > 0)
                        {
                            character.move(0.0f, -(window_size / (board_size + 1.0f)));
                            --charY;
                        }
                        break;
                    case sf::Keyboard::Down:
                        if (charY < board_size - 1)
                        {
                            character.move(0.0f, window_size / (board_size + 1.0f));
                            ++charY;
                        }
                        break;
                    case sf::Keyboard::Left:
                        if (charX > 0)
                        {
                            character.move(-(window_size / (board_size + 1.0f)), 0.0f);
                            --charX;
                        }
                        break;
                    case sf::Keyboard::Right:
                        if (charX < board_size - 1)
                        {
                            character.move(window_size / (board_size + 1.0f), 0.0f);
                            ++charX;
                        }
                        break;
                    }
                    break;
                }
            }

            // Print everything to the screen
            window.clear(sf::Color::Cyan);
            for (auto& s : spots)
                window.draw(s);
            for (auto& f : fireballs)
            {
                if (f.wait < sf::seconds(base_wait * static_cast<float>(std::pow(wait_multiplier, difficulty))))
                    window.draw(f.sprite);
            }
            window.draw(character);
            window.display();

            for (auto& f : fireballs)
            {
                if (f.wait > sf::Time())
                    f.wait -= elapsed;

                if (f.wait <= sf::Time())
                {
                    switch (f.side)
                    {
                    case Fireball::Side::Top:
                        f.sprite.move(0.0f, f.speed * elapsed.asSeconds());
                        break;
                    case Fireball::Side::Bottom:
                        f.sprite.move(0.0f, -f.speed * elapsed.asSeconds());
                        break;
                    case Fireball::Side::Left:
                        f.sprite.move(f.speed * elapsed.asSeconds(), 0.0f);
                        break;
                    case Fireball::Side::Right:
                        f.sprite.move(-f.speed * elapsed.asSeconds(), 0.0f);
                        break;
                    }
                }

                std::uniform_real_distribution random_multiplier{ 0.5f, 3.0f };

                if (f.side == Fireball::Side::Top && f.sprite.getPosition().y > window_size)
                {
                    f.wait = sf::seconds(base_wait * random_multiplier(twister) * static_cast<float>(std::pow(wait_multiplier, difficulty)));
                    f.sprite.setPosition(randomizer(twister) * (window_size / (board_size + 1.0f)) - radius, -radius);
                }
                if (f.side == Fireball::Side::Bottom && f.sprite.getPosition().y < -radius * 2)
                {
                    f.wait = sf::seconds(base_wait * random_multiplier(twister) * static_cast<float>(std::pow(wait_multiplier, difficulty)));
                    f.sprite.setPosition(randomizer(twister) * (window_size / (board_size + 1.0f)) - radius, window_size - radius);
                }
                if (f.side == Fireball::Side::Left && f.sprite.getPosition().x > window_size)
                {
                    f.wait = sf::seconds(base_wait * random_multiplier(twister) * static_cast<float>(std::pow(wait_multiplier, difficulty)));
                    f.sprite.setPosition(-radius, randomizer(twister) * (window_size / (board_size + 1.0f)) - radius);
                }
                if (f.side == Fireball::Side::Right && f.sprite.getPosition().x < -radius * 2)
                {
                    f.wait = sf::seconds(base_wait * random_multiplier(twister) * static_cast<float>(std::pow(wait_multiplier, difficulty)));
                    f.sprite.setPosition(window_size - radius, randomizer(twister) * (window_size / (board_size + 1.0f)) - radius);
                }

                if (f.sprite.getGlobalBounds().intersects(character.getGlobalBounds()))
                {
                    music.stop();
                    bonk.play();
                    gameActive = false;
                }

                if (music.getStatus() == sf::Music::Status::Stopped)
                {
                    gameActive = false;
                }
            }
        }
    }

    return 0;
}