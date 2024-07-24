#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>
#include <SFML/System.hpp>
#include <Windows.h>
#include <mmsystem.h>
#include <fstream>

using namespace sf;
using namespace std;

//---------------------------------------------CLASSES--------------------------------------

class Player {
private:
    int HPmax = 10;
public:
    Sprite shape;
    Texture* texture;
    bool isAlive;
    int HP;
    float velocityY, velocityX;
    bool onGround;
    Texture textureLeft ;

    Player(Texture* texture) {
        this->HPmax = 10;
        this->HP = this->HPmax;
        this->texture = texture;
        this->shape.setTexture(*texture);
        this->isAlive = true;
        this->velocityX = 0.f;
        this->velocityY = 0.f;
        this->onGround = true;
        textureLeft.loadFromFile("Mario/PlayerLeft.png");
    }

    void update(float gravity, float groundLevel, float jumpStrength, Sound& jumpSound) {
        // Apply gravity
        if (shape.getPosition().y < groundLevel) {
            velocityY += gravity;
        } else {
            velocityY = 0.f;
            shape.setPosition(shape.getPosition().x, groundLevel);
            onGround = true;
        }

        // Handle jump
        if (Keyboard::isKeyPressed(Keyboard::Space) && onGround) {
            jumpSound.play();
            velocityY = -jumpStrength;
            onGround = false;
        }

        shape.move(0.f, velocityY);

        // Move left and right
         if (Keyboard::isKeyPressed(Keyboard::A)) {
            shape.move(-10.f, 0.f);
            shape.setTexture(textureLeft);
        } else if (Keyboard::isKeyPressed(Keyboard::D)) {
            shape.move(10.f, 0.f);
            shape.setTexture(*texture);
        } 
    }

    ~Player() {}
};


class Enemy {
public:
    Sprite shape;
    Texture* texture;
    float moveSpeed;
    float movementRange;
    bool movingRight;
    Vector2f startPosition;
    bool isAlive ;
    Texture textureRight ;
    Enemy(Texture* texture, float moveSpeed = 0.5f, float movementRange = 300.f) 
        : moveSpeed(moveSpeed), movementRange(movementRange), movingRight(true) {
        this->texture = texture;
        this->shape.setTexture(*texture);
        textureRight.loadFromFile("Mario/enemyRight.png");
        this->isAlive = true ;
    }

    void update(float deltaTime) {
        float currentX = shape.getPosition().x;

        if (movingRight) {
            shape.setTexture(*texture);
            shape.move(moveSpeed * deltaTime, 0.f);
            if (currentX >= startPosition.x + movementRange) {
                movingRight = false;
            }
        } else {
            shape.move(-moveSpeed * deltaTime, 0.f);
            shape.setTexture(textureRight);
            if (currentX <= startPosition.x - movementRange) {

                movingRight = true;
            }
        }
    }

    void setPosition(const Vector2f& position) {
        this->shape.setPosition(position);
        startPosition = position;
    }

    const Sprite& getSprite() const {
        return shape;
    }
};

class Collectible {
public:
    Sprite shape;
    Texture* texture;

    Collectible(Texture* texture) {
        this->texture = texture;
        this->shape.setTexture(*texture);
        this->shape.setScale(0.1f,0.1f);
    }
    void setPosition(const Vector2f& position) {
        this->shape.setPosition(position);
    }
    const Sprite& getSprite() const {
        return shape;
    }
};

class Brick {
public:
    Sprite shape;
    Texture* texture;
    Texture texture2, texture3;
    int HP;
    bool isBreaking;

    Brick(Texture* texture) : HP(3), isBreaking(false) {
        this->texture = texture;
        this->shape.setTexture(*texture);
        this->shape.setScale(0.7f, 0.7f);
        texture2.loadFromFile("Mario/brik2.png");
        texture3.loadFromFile("Mario/brik3.png");
    }

    void setPosition(const Vector2f& position) {
        this->shape.setPosition(position);
    }

    const Sprite& getSprite() const {
        return shape;
    }

    void updateTexture() {
        if (HP == 2) {
            shape.setTexture(texture2);
        } else if (HP == 1) {
            shape.setTexture(texture3);
        }
    }

    void startBreaking() {
        if (HP > 0) {
            isBreaking = true;
        }
    }

    bool operator==(const Brick& other) const {
        return this == &other;
    }

    ~Brick() {}
};

//---------------------------FUNCTIONS--------------------------------------------
Color darkenColor(const Color& color, float factor);
void handleMenuClick(int x, int y, Text choice1, Text choice2, Text choice3, bool &choice1Selected, bool &choice2Selected, bool &choice3Selected);
void Levleswin(RenderWindow &window);
void handleLevelClick(int x, int y, Sprite levelsSprites[], int &selectedLevel);
bool loadchosenlevel(int level, RenderWindow &mainwindow, Player &player);
bool checkCollision(Player& player, const Sprite& platform);
void handlePlayerBrickCollision(Player& player, vector<Brick>& bricks) ;
void handlePlayerEnemyCollision(Player& player, vector<Enemy>& enemies , Sound &dieSound );
void handlePlayerCollectibles(Player& player, vector<Collectible>& collectibles, int& score , Sound &coinSound );
void drawGrid(RenderWindow& window);
void saveLevel(const string& filename, const vector<Brick>& bricks , const vector<Enemy>& enemies , const vector<Collectible>& coins) ;
void loadLevel(const string& filename, vector<Brick>& bricks , Texture &bricktex , vector<Enemy>& enemies , Texture &enemtytex , vector<Collectible>& coins , Texture &cointex) ;
void LevelEditorwin(RenderWindow &window);
void LevelEditor(RenderWindow &window, int level);
void WinCol(Player& player, RenderWindow& window) ;
bool level1(RenderWindow &mainwindow, Player &player);
bool level2(RenderWindow &mainwindow, Player &player);
bool level3(RenderWindow &mainwindow, Player &player);
bool level4(RenderWindow &mainwindow, Player &player);
bool level5(RenderWindow &mainwindow, Player &player);
bool level6(RenderWindow &mainwindow, Player &player);
bool level7(RenderWindow &mainwindow, Player &player);
bool level8(RenderWindow &mainwindow, Player &player);
bool level9(RenderWindow &mainwindow, Player &player);


//---------------------------MAIN----------------------------------------------

int main() {
    // Init vars 
    srand(time(NULL));
    bool isPaused = false;
    bool PlaySelected = false;
    bool ExitSelected = false;
    bool EditorSelected = false;
    // Init window 
    RenderWindow window(VideoMode(1600, 900), "Super Mario", Style::Default);
    window.setFramerateLimit(60);
    //Icon
    Image icon;
    icon.loadFromFile("Mario/icon.png") ;

    // Set the icon
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    //MUSIC 
    Music backgroundMusic ;
    backgroundMusic.openFromFile("Mario/mainbackground.wav");
    backgroundMusic.play();
    // Fonts
    Font font1, font2, font3;
    font1.loadFromFile("Mario/BrutalBrushstrokes-Six.ttf");
    font2.loadFromFile("Mario/BrushBlitz-One.ttf");
    font3.loadFromFile("Mario/Carnotaurus-Three.ttf");

    // Main background
    Texture mainbacktex;
    mainbacktex.loadFromFile("Mario/mainback.png");
    Sprite mainbackground(mainbacktex);
    mainbackground.setScale(1.7f, 1.7f);

    // Main text
    Text supermariotxt;
    supermariotxt.setFont(font1);
    supermariotxt.setString("SUPER MARIO");
    supermariotxt.setCharacterSize(200);
    supermariotxt.setPosition(window.getSize().x / 2 - supermariotxt.getGlobalBounds().width / 2, 20.f);
    supermariotxt.setFillColor(Color::Red);
    supermariotxt.setOutlineColor(Color::Black);
    supermariotxt.setOutlineThickness(10.f);

    // Exit text
    Text Exittxt;
    Exittxt.setFont(font1);
    Exittxt.setString("EXIT");
    Exittxt.setCharacterSize(150);
    Exittxt.setPosition(window.getSize().x / 2 - Exittxt.getGlobalBounds().width / 2, window.getSize().y / 2 + 100.f);
    Color darkRed = darkenColor(Color::Red, 0.5f);
    Exittxt.setFillColor(darkRed);
    Exittxt.setOutlineColor(Color::Black);
    Exittxt.setOutlineThickness(10.f);

    // Play text
    Text Playtxt;
    Playtxt.setFont(font1);
    Playtxt.setString("PLAY");
    Playtxt.setCharacterSize(150);
    Playtxt.setPosition(window.getSize().x / 2 - Playtxt.getGlobalBounds().width / 2, window.getSize().y / 2 - 100.f);
    Playtxt.setFillColor(Color::Green);
    Playtxt.setOutlineColor(Color::Black);
    Playtxt.setOutlineThickness(10.f);
    
    Text Editortxt;
    Editortxt.setFont(font1);
    Editortxt.setString("LEVEL EDITOR");
    Editortxt.setCharacterSize(100);
    Editortxt.setPosition(window.getSize().x / 2 - Editortxt.getGlobalBounds().width / 2, window.getSize().y / 2 + 250.f);
    Editortxt.setFillColor(Color::Blue);
    Editortxt.setOutlineColor(Color::Black);
    Editortxt.setOutlineThickness(10.f);
    // Window controls 
    while (window.isOpen()) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
            if (event.KeyPressed && event.key.code == Keyboard::Escape) {
                window.close();
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Enter) {
                isPaused = !isPaused;
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                backgroundMusic.stop();
                handleMenuClick(event.mouseButton.x, event.mouseButton.y, Playtxt, Exittxt, Editortxt, PlaySelected, ExitSelected, EditorSelected);
            }
        }

        // Drawing
        window.clear();
        window.draw(mainbackground);
        window.draw(supermariotxt);
        window.draw(Exittxt);
        window.draw(Playtxt);
       // window.draw(Editortxt);
        window.display();

        // Choice selection
        if (PlaySelected) {
            Levleswin(window);
            PlaySelected = false;
        } else if (ExitSelected) {
            window.close();
        } else if (EditorSelected) {
            LevelEditorwin(window);
            EditorSelected = false;
        }
    }
    return 0;
}

//----------------------FUNCTIONS BODY---------------------------------

Color darkenColor(const Color& color, float factor) {
    if (factor < 0) factor = 0;
    if (factor > 1) factor = 1;

    Uint8 red = static_cast<Uint8>(color.r * factor);
    Uint8 green = static_cast<Uint8>(color.g * factor);
    Uint8 blue = static_cast<Uint8>(color.b * factor);

    return Color(red, green, blue, color.a);
}

void handleMenuClick(int x, int y, Text choice1, Text choice2, Text choice3, bool &choice1Selected, bool &choice2Selected, bool &choice3Selected) {
    if (choice1.getGlobalBounds().contains(x, y)) {
        choice1Selected = true;
    } else if (choice2.getGlobalBounds().contains(x, y)) {
        choice2Selected = true;
    } else if (choice3.getGlobalBounds().contains(x, y)) {
        choice3Selected = true;
    }
}

bool checkCollision(Player& player, const Sprite& platform) {
    return player.shape.getGlobalBounds().intersects(platform.getGlobalBounds());
}

void handlePlayerEnemyCollision(Player& player, vector<Enemy>& enemies, Sound& dieSound) {
    for (auto& enemy : enemies) {
        if (player.shape.getGlobalBounds().intersects(enemy.shape.getGlobalBounds())) {
            if (player.shape.getPosition().y + player.shape.getGlobalBounds().height <= enemy.shape.getPosition().y + 10) {
                enemy.isAlive = false;
            } else {
                player.HP--;
                if (player.HP <= 0) {
                    dieSound.play();
                    player.isAlive = false;
                }
            }
        }
    }

    enemies.erase(remove_if(enemies.begin(), enemies.end(), [](const Enemy& enemy) { return !enemy.isAlive; }), enemies.end());
}

void handlePlayerCollectibles(Player& player, vector<Collectible>& collectibles, int& score, Sound& coinSound) {
    for (auto it = collectibles.begin(); it != collectibles.end(); ) {
        if (player.shape.getGlobalBounds().intersects(it->shape.getGlobalBounds())) {
            score++;
            coinSound.play();
            it = collectibles.erase(it);
        } else {
            ++it;
        }
    }
}


void handlePlayerBrickCollision(Player& player, vector<Brick>& bricks) {
    for (auto it = bricks.begin(); it != bricks.end();) {
        Brick& brick = *it;
        if (checkCollision(player, brick.shape)) {
            FloatRect playerBounds = player.shape.getGlobalBounds();
            FloatRect brickBounds = brick.shape.getGlobalBounds();
            Vector2f initialPosition = player.shape.getPosition();
            float playerBottom = playerBounds.top + playerBounds.height;
            float brickTop = brickBounds.top;
            float playerTop = playerBounds.top;
            float brickBottom = brickBounds.top + brickBounds.height;
            float playerRight = playerBounds.left + playerBounds.width;
            float brickLeft = brickBounds.left;
            float playerLeft = playerBounds.left;
            float brickRight = brickBounds.left + brickBounds.width;

            bool collisionFromTop = playerBottom > brickTop && playerTop < brickTop;
            bool collisionFromBottom = playerTop < brickBottom && playerBottom > brickBottom;
            bool collisionFromLeft = playerRight > brickLeft && playerLeft < brickLeft;
            bool collisionFromRight = playerLeft < brickRight && playerRight > brickRight;

            if (collisionFromTop) {
                player.velocityY = 0.f;
                player.shape.setPosition(initialPosition.x, brickTop - playerBounds.height);
                player.onGround = true;
            } else if (collisionFromBottom) {
                player.velocityY = 0.f;
                player.shape.setPosition(initialPosition.x, brickBottom);
                if (!brick.isBreaking) {
                    brick.startBreaking();
                }
                brick.HP--;
                brick.updateTexture();
                if (brick.HP <= 0) {
                    it = bricks.erase(it);
                    continue;
                }
            } else if (collisionFromLeft) {
                player.shape.setPosition(brickLeft - playerBounds.width, initialPosition.y);
                player.velocityX = 0.f;
            } else if (collisionFromRight) {
                player.shape.setPosition(brickRight, initialPosition.y);
                player.velocityX = 0.f;
            }
        }
        ++it;
    }
}


void drawGrid(RenderWindow& window) {
    int gridSize = 32;
    int width = window.getSize().x *5;
    int height = window.getSize().y;

    for (int x = 0; x < width; x += gridSize) {
        Vertex line[] = {
            Vertex(Vector2f(x, 0), Color::Black),
            Vertex(Vector2f(x, height), Color::Black)
        };
        window.draw(line, 2, Lines);
    }

    for (int y = 0; y < height; y += gridSize) {
        Vertex line[] = {
            Vertex(Vector2f(0, y), Color::Black),
            Vertex(Vector2f(width, y), Color::Black)
        };
        window.draw(line, 2, Lines);
    }
}

void saveLevel(const string& filename, const vector<Brick>& bricks, const vector<Enemy>& enemies, const vector<Collectible>& coins) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for saving." << endl;
        return;
    }

    // Save coins
    for (const auto& coin : coins) {
        const auto& sprite2 = coin.shape;
        const auto& position2 = sprite2.getPosition();
        const auto& scale2 = sprite2.getScale();
        file << "coin " << position2.x << " " << position2.y << " " << scale2.x << " " << scale2.y << endl;
    }

    // Save bricks
    for (const auto& brick : bricks) {
        const auto& sprite = brick.shape;
        const auto& position = sprite.getPosition();
        const auto& scale = sprite.getScale();
        file << "brick " << position.x << " " << position.y << " " << scale.x << " " << scale.y << endl;
    }

    // Save enemies
    for (const auto& enemy : enemies) {
        const auto& sprite1 = enemy.shape;
        const auto& position1 = sprite1.getPosition();
        const auto& scale1 = sprite1.getScale();
        file << "enemy " << position1.x << " " << position1.y << " " << scale1.x << " " << scale1.y << endl;
    }

    file.close();
}


void loadLevel(const string& filename, vector<Brick>& bricks, Texture& bricktex, vector<Enemy>& enemies, Texture& enemytex, vector<Collectible>& coins, Texture& cointex) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file for loading." << endl;
        return;
    }

    bricks.clear();
    coins.clear();
    enemies.clear();

    string type;
    float x, y, scaleX, scaleY;

    while (file >> type >> x >> y >> scaleX >> scaleY) {
        if (type == "coin") {
            Collectible coin(&cointex);
            coin.setPosition(Vector2f(x, y));
            coin.shape.setScale(scaleX, scaleY);
            coins.push_back(coin);
        } else if (type == "brick") {
            Brick brick(&bricktex);
            brick.setPosition(Vector2f(x, y));
            brick.shape.setScale(scaleX, scaleY);
            bricks.push_back(brick);
        } else if (type == "enemy") {
            Enemy enemy(&enemytex);
            enemy.setPosition(Vector2f(x, y));
            enemy.shape.setScale(scaleX, scaleY);
            enemies.push_back(enemy);
        }
    }

    file.close();
}


void LevelEditorwin(RenderWindow &window) {
    bool levelSelected = false;
    int selectedLevel = 0;

    RenderWindow editorLevelsWin(VideoMode(1600, 900), "Level Editor", Style::Default);
    editorLevelsWin.setFramerateLimit(60);
    
    Font font;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");

    Texture Levelsbacktex;
    Levelsbacktex.loadFromFile("Mario/Levelsback.jpg");
    Sprite Levelsback(Levelsbacktex);
    Levelsback.setScale(1.7f, 1.7f);

    Texture levelsTexture[9];
    Sprite levelsSprites[9];

    for (int i = 0; i < 9; i++) {
        levelsTexture[i].loadFromFile("Mario/levels.jpg");
        levelsSprites[i].setTexture(levelsTexture[i]);
    }

    Text levelsText[9];
    for (int i = 0; i < 9; ++i) {
        levelsText[i].setFont(font);
        levelsText[i].setString("LVL " + to_string(i + 1));
        levelsText[i].setCharacterSize(75);
        levelsText[i].setFillColor(Color::Red);
    }

    float startX = 200.f, startY = 50.f;
    float gapX = 50.f, gapY = 50.f;
    int columns = 3;
    for (int i = 0; i < 9; ++i) { 
        int row = i / columns;
        int col = i % columns;
        levelsSprites[i].setPosition(startX + col * (levelsSprites[i].getGlobalBounds().width + gapX),
                                     startY + row * (levelsSprites[i].getGlobalBounds().height + gapY));
        levelsText[i].setPosition(levelsSprites[i].getPosition().x + levelsSprites[i].getGlobalBounds().width / 2 - levelsText[i].getGlobalBounds().width / 2,
                                  levelsSprites[i].getPosition().y + levelsSprites[i].getGlobalBounds().height / 2 - levelsText[i].getGlobalBounds().height / 2);
    }

    while (editorLevelsWin.isOpen()) {
        Event event;
        while (editorLevelsWin.pollEvent(event)) {
            if (event.type == Event::Closed) {
                editorLevelsWin.close();
                return;
            }
            if (event.KeyPressed && event.key.code == Keyboard::Escape) {
                editorLevelsWin.close();
                return;
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                handleLevelClick(event.mouseButton.x, event.mouseButton.y, levelsSprites, selectedLevel);
                if (selectedLevel != 0) {
                    levelSelected = true;
                    editorLevelsWin.close();
                }
            }
        }

        editorLevelsWin.clear();
        editorLevelsWin.draw(Levelsback);
        for (int i = 0; i < 9; i++) {
            editorLevelsWin.draw(levelsSprites[i]); 
            editorLevelsWin.draw(levelsText[i]);
        }
        editorLevelsWin.display();
    }

    if (levelSelected) {
        LevelEditor(window, selectedLevel);
    }
}

void handleLevelClick(int x, int y, Sprite levelsSprites[], int &selectedLevel) {
    for (int i = 0; i < 9; i++) {
        if (levelsSprites[i].getGlobalBounds().contains(x, y)) {
            selectedLevel = i + 1;
            break;
        }
    }
}
bool loadchosenlevel(int level, RenderWindow &mainwindow, Player &player) {
    switch (level) {
        case 1:
            return level1(mainwindow, player);
            break;
        case 2:
            return level2(mainwindow, player);
            break;
        case 3:
            return level3(mainwindow, player);
            break;
        case 4:
            return level4(mainwindow, player);
            break;
        case 5:
            return level5(mainwindow, player);
            break;
        case 6:
            return level6(mainwindow, player);
            break;
        case 7:
            return level7(mainwindow, player);
            break;
        case 8:
            return level8(mainwindow, player);
            break;
        case 9:
            return level9(mainwindow, player);
            break;
        default:
            return false;
            break;
    }
}

void LevelEditor(RenderWindow &window, int level) {
    // Load texture
    Texture bricktex , enemytex ,cointex;
    bricktex.loadFromFile("Mario/brik.png");
    enemytex.loadFromFile("Mario/enemy.png");
    cointex.loadFromFile("Mario/coin.png");
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible>coins ;
    loadLevel("level" + to_string(level) + ".txt", bricks , bricktex , enemies , enemytex , coins , cointex);

    // Set up view
    View view = window.getDefaultView();
    view.setCenter(window.getSize().x / 2.f, window.getSize().y / 2.f);

    // Main loop
    bool isDrawing = true;
    while (isDrawing) {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                return;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::S) {
                isDrawing = false;
                saveLevel("level" + to_string(level) + ".txt", bricks , enemies , coins);
            }
             Vector2f mousePos = window.mapPixelToCoords(Mouse::getPosition(window));
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                // Add brick on click
                Brick brick(&bricktex);  
                brick.setPosition(mousePos);
                bricks.push_back(brick);
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                  // Remove brick on right click
                 for (auto it = bricks.begin(); it != bricks.end(); ++it) {
                 if (it->getSprite().getGlobalBounds().contains(mousePos)) {
                     bricks.erase(it);
                     break;  
                  }
                 }
             }
             if (event.type == Event::KeyPressed && event.key.code == Keyboard::E) {
                // Add enemy on click
                Enemy enemy(&enemytex);  
                enemy.setPosition(mousePos);
                enemies.push_back(enemy);
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                  // Remove enemies on right click
                 for (auto it = enemies.begin(); it != enemies.end(); ++it) {
                 if (it->getSprite().getGlobalBounds().contains(mousePos)) {
                     enemies.erase(it);
                     break;  
                  }
                 }
             }
               if (event.type == Event::KeyPressed && event.key.code == Keyboard::C) {
                // Add coin on click
                Collectible coin (&cointex);  
                coin.setPosition(mousePos);
                coins.push_back(coin);
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Right) {
                  // Remove coins on right click
                 for (auto it = coins.begin(); it != coins.end(); ++it) {
                 if (it->getSprite().getGlobalBounds().contains(mousePos)) {
                     coins.erase(it);
                     break;  
                  }
                 }
             }

        }

        // Handle view movement
        if (Keyboard::isKeyPressed(Keyboard::Left)) {
            view.move(-5.f, 0.f);
        }
        if (Keyboard::isKeyPressed(Keyboard::Right)) {
            view.move(5.f, 0.f);
        }
        if (Keyboard::isKeyPressed(Keyboard::Up)) {
            view.move(0.f, -5.f);
        }
        if (Keyboard::isKeyPressed(Keyboard::Down)) {
            view.move(0.f, 5.f);
        }
        window.setView(view);

        // Drawing
        window.clear(Color::Cyan);
        drawGrid(window);
        for (const auto& brick : bricks) {
            window.draw(brick.shape);
        }
        for (const auto& enemy : enemies) {
            window.draw(enemy.shape);
        }
        for (const auto& coin : coins) {
            window.draw(coin.shape);
        }
        window.display();
    
}
}

void WinCol(Player& player, RenderWindow& window) {
    if (player.shape.getPosition().x <= 0) {
        player.shape.setPosition(0.f, player.shape.getPosition().y);
    }
     if (player.shape.getPosition().x >=  window.getSize().x * 5 ) {
        player.shape.setPosition(window.getSize().x * 5.f - 10.f, player.shape.getPosition().y);
    }
    
}

bool level1(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level1.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(2, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(1, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level2(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level2.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(3, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(2, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level3(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level3.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(4, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(3, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level4(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level4.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(5, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(4, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level5(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level5.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(6, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(5, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level6(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level6.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -1000.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(7, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(6, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level7(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level7.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(8, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(7, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}

bool level8(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level8.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -600.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(9, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(8, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}
 
bool level9(RenderWindow &mainwindow, Player &player) {
    mainwindow.setFramerateLimit(60);

    //load font 
    Font font, font1;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");
    font1.loadFromFile("Mario/BrushBlitz-One.ttf");

    // Load textures
    Texture playerTex, brickTex, enemyTex, coinTex;

    if (!playerTex.loadFromFile("Mario/Player.png")) {
        cerr << "Error loading player texture." << endl;
        return false;
    }

    if (!brickTex.loadFromFile("Mario/brik.png")) {
        cerr << "Error loading brick texture." << endl;
        return false;
    }

    if (!enemyTex.loadFromFile("Mario/enemy.png")) {
        cerr << "Error loading enemy texture." << endl;
        return false;
    }

    if (!coinTex.loadFromFile("Mario/coin.png")) {
        cerr << "Error loading coin texture." << endl;
        return false;
    }

    // Sounds
    SoundBuffer jumpBuffer, walkBuffer, coinBuffer, dieBuffer, winBuffer;
    Music backgroundMusic;

    Sound jumpSound, walkSound, coinSound, dieSound, winSound;

    // Load sound buffers
    if (!jumpBuffer.loadFromFile("Mario/jump.wav") ||
        !walkBuffer.loadFromFile("Mario/walk.wav") ||
        !coinBuffer.loadFromFile("Mario/coin.wav") ||
        !dieBuffer.loadFromFile("Mario/die.wav") ||
        !winBuffer.loadFromFile("Mario/win.wav")) {
        std::cerr << "Error loading sound files." << std::endl;
        return false;
    }

    // Load background music
    if (!backgroundMusic.openFromFile("Mario/background.wav")) {
        std::cerr << "Error loading background music." << std::endl;
        return false;
    }

    // Set sound buffers to sounds
    jumpSound.setBuffer(jumpBuffer);
    walkSound.setBuffer(walkBuffer);
    coinSound.setBuffer(coinBuffer);
    dieSound.setBuffer(dieBuffer);
    winSound.setBuffer(winBuffer);

    // Play background music
    backgroundMusic.play();

    // Create player
    player = Player(&playerTex);
    player.shape.setPosition(100.f, mainwindow.getSize().y - 400.f);
    Enemy enemy(&enemyTex);

    // Create bricks, enemies, collectibles
    vector<Brick> bricks;
    vector<Enemy> enemies;
    vector<Collectible> coins;
    loadLevel("level9.txt", bricks, brickTex, enemies, enemyTex, coins, coinTex);

    // Platform
    Texture platformtex;
    platformtex.loadFromFile("Mario/grass.png");
    Sprite platform(platformtex);
    platform.setPosition(-1000.f, mainwindow.getSize().y - 600.f);

    // Sky
    Texture skytex;
    skytex.loadFromFile("Mario/sky.png");
    Sprite sky(skytex);
    sky.setPosition(-1000.f, -1000.f);

    // Limit lines
    RectangleShape limitline(Vector2f(10.f, 6000.f));
    limitline.setFillColor(Color::Red);
    limitline.setPosition(mainwindow.getPosition().x, mainwindow.getPosition().y - 500);

    RectangleShape endlimitline(Vector2f(10.f, 6000.f));
    endlimitline.setFillColor(Color::Red);
    endlimitline.setPosition(mainwindow.getSize().x * 5, mainwindow.getPosition().y - 500);

    // Win Portal
    Texture winPortaltex;
    winPortaltex.loadFromFile("Mario/Portal.png");
    Sprite winPortal;
    winPortal.setTexture(winPortaltex);
    winPortal.setPosition(mainwindow.getSize().x * 5 - 100.f, mainwindow.getSize().y - 600);

    // Entry Portal
    Texture entryPortaltex;
    entryPortaltex.loadFromFile("Mario/EntryPortal.png");
    Sprite EntryPortal;
    EntryPortal.setTexture(entryPortaltex);
    EntryPortal.setPosition(mainwindow.getPosition().x - 100.f, mainwindow.getSize().y - 600);

    // Score text 
    Text scoretxt;
    scoretxt.setFont(font1);
    scoretxt.setCharacterSize(50);
    scoretxt.setFillColor(Color::Yellow);

    // HP text 
    Text HPtxt;
    HPtxt.setFont(font1);
    HPtxt.setCharacterSize(50);
    HPtxt.setFillColor(Color::Green);

    // Set up view
    Vector2f playerPos = player.shape.getPosition();
    View view = mainwindow.getDefaultView();
    view.setCenter(playerPos.x + mainwindow.getSize().x / 4, mainwindow.getSize().y / 2);

    // Game loop
    bool isPlaying = true;
    float gravity = 1.5f;
    float jumpStrength = 25.f;
    float groundLevel = mainwindow.getSize().y - 300.f;
    int score = 0;

    while (isPlaying) {
        Event event;
        while (mainwindow.pollEvent(event)) {
            if (event.type == Event::Closed) {
                mainwindow.close();
                return false;
            }
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape) {
                return false;
            }
        }

        // Update enemy
        for (auto& enemy : enemies) {
            enemy.update(5);
        }

        // Update player
        player.update(gravity, groundLevel, jumpStrength, jumpSound);
        WinCol(player, mainwindow);

        // Update view to follow player
        view.setCenter(player.shape.getPosition() + Vector2f(0.f, 0.f));
        mainwindow.setView(view);

        // Check collisions
        handlePlayerBrickCollision(player, bricks);
        handlePlayerEnemyCollision(player, enemies, dieSound);
        handlePlayerCollectibles(player, coins, score, coinSound);

        scoretxt.setString("Score: " + to_string(score));
        HPtxt.setString("HP: " + to_string(player.HP));

        // Drawing
        mainwindow.clear();
        mainwindow.draw(sky);
        mainwindow.draw(platform);

        for (const auto& brick : bricks) {
            mainwindow.draw(brick.shape);
        }
        mainwindow.draw(player.shape);
        scoretxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2 + 50);
        HPtxt.setPosition(view.getCenter().x - mainwindow.getSize().x / 2 + 50, view.getCenter().y - mainwindow.getSize().y / 2);

        for (const auto& enemy : enemies) {
            mainwindow.draw(enemy.shape);
        }
        for (const auto& collectible : coins) {
            mainwindow.draw(collectible.shape);
        }
        mainwindow.draw(HPtxt);
        mainwindow.draw(scoretxt);
        mainwindow.draw(limitline);
        mainwindow.draw(endlimitline);
        mainwindow.draw(winPortal);
        mainwindow.draw(EntryPortal);

        if (player.shape.getGlobalBounds().intersects(winPortal.getGlobalBounds())) {
            winSound.play();
            backgroundMusic.stop();
            RenderWindow secondarywindow(VideoMode(500, 400), "WIN", Style::Default);
            secondarywindow.setFramerateLimit(60);

            Text RePlaytxt;
            RePlaytxt.setFont(font);
            RePlaytxt.setCharacterSize(48);
            RePlaytxt.setString("Replay");
            RePlaytxt.setFillColor(Color::Blue);
            FloatRect textRect = RePlaytxt.getLocalBounds();
            RePlaytxt.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
            RePlaytxt.setPosition(secondarywindow.getSize().x / 2, secondarywindow.getSize().y / 2.0f);
            RePlaytxt.setOutlineThickness(5.f);

            Text Exittxt;
            Exittxt.setFont(font);
            Exittxt.setCharacterSize(48);
            Exittxt.setString("Exit");
            Exittxt.setFillColor(Color::Red);
            FloatRect textRect2 = Exittxt.getLocalBounds();
            Exittxt.setOrigin(textRect2.left + textRect2.width / 2.0f, textRect2.top + textRect2.height / 2.0f);
            Exittxt.setPosition(100, secondarywindow.getSize().y / 2.0f);
            Exittxt.setOutlineThickness(5.f);

            Text Nexttxt;
            Nexttxt.setFont(font);
            Nexttxt.setCharacterSize(48);
            Nexttxt.setString("Next ");
            Nexttxt.setFillColor(Color::Green);
            FloatRect textRect3 = Nexttxt.getLocalBounds();
            Nexttxt.setOrigin(textRect3.left + textRect3.width / 2.0f, textRect3.top + textRect3.height / 2.0f);
            Nexttxt.setPosition(secondarywindow.getSize().x / 2 + 150, secondarywindow.getSize().y / 2.0f);
            Nexttxt.setOutlineThickness(5.f);

            Texture lastbackTex ;
            lastbackTex.loadFromFile("Mario/lastback.png") ;
            Sprite Lastback(lastbackTex); 

            bool RePlaySelected = false;
            bool ExitSelected = false;
            bool NextSelected = false;

            while (secondarywindow.isOpen()) {
                Event event;
                while (secondarywindow.pollEvent(event)) {
                    if (event.type == Event::Closed) {
                        secondarywindow.close();
                        mainwindow.close();
                        return false;
                    }
                    if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                        handleMenuClick(event.mouseButton.x, event.mouseButton.y, RePlaytxt, Exittxt, Nexttxt, RePlaySelected, ExitSelected, NextSelected);
                    }
                }

                secondarywindow.clear();
                secondarywindow.draw(Lastback);
                secondarywindow.draw(Nexttxt);
                secondarywindow.draw(RePlaytxt);
                secondarywindow.draw(Exittxt);
                secondarywindow.display();

                if (NextSelected) {
                    secondarywindow.close();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(1, mainwindow2, player);
                    backgroundMusic.stop();
                    mainwindow.close();
                    
                } else if (ExitSelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    mainwindow.close();
                } else if (RePlaySelected) {
                    secondarywindow.close();
                    backgroundMusic.stop();
                    RenderWindow mainwindow2(VideoMode(1600, 900), "Main Window", Style::Default);
                    Texture playerTex;
                    playerTex.loadFromFile("Mario/Player.png");
                    Player player(&playerTex);
                    loadchosenlevel(9, mainwindow2, player);
                    mainwindow.close();
                }
            }
        }

        mainwindow.display();

        // Check if player is dead
        if (!player.isAlive) {
            dieSound.play();
            mainwindow.clear(Color::Black);
            isPlaying = false;
        }
    }

    return true;
}







void Levleswin(RenderWindow &window) {
    bool levelSelected = false;
    int selectedLevel = 0;

    RenderWindow levelswin(VideoMode(1600, 900), "Levels", Style::Default);
    levelswin.setFramerateLimit(60);

    Font font;
    font.loadFromFile("Mario/Carnotaurus-Three.ttf");

    Texture Levelsbacktex;
    Levelsbacktex.loadFromFile("Mario/Levelsback.jpg");
    Sprite Levelsback(Levelsbacktex);
    Levelsback.setScale(1.7f, 1.7f);

    Texture levelsTexture[9];
    Sprite levelsSprites[9];

    for (int i = 0; i < 9; i++) {
        levelsTexture[i].loadFromFile("Mario/levels.jpg");
        levelsSprites[i].setTexture(levelsTexture[i]);
    }

    Text levelsText[9];
    for (int i = 0; i < 9; ++i) {
        levelsText[i].setFont(font);
        levelsText[i].setString("LVL " + to_string(i + 1));
        levelsText[i].setCharacterSize(75);
        levelsText[i].setFillColor(Color::Red);
    }

    float startX = 200.f, startY = 50.f;
    float gapX = 50.f, gapY = 50.f;
    int columns = 3;
    for (int i = 0; i < 9; ++i) { 
        int row = i / columns;
        int col = i % columns;
        levelsSprites[i].setPosition(startX + col * (levelsSprites[i].getGlobalBounds().width + gapX),
                                     startY + row * (levelsSprites[i].getGlobalBounds().height + gapY));
        levelsText[i].setPosition(levelsSprites[i].getPosition().x + levelsSprites[i].getGlobalBounds().width / 2 - levelsText[i].getGlobalBounds().width / 2,
                                  levelsSprites[i].getPosition().y + levelsSprites[i].getGlobalBounds().height / 2 - levelsText[i].getGlobalBounds().height / 2);
    }

    while (levelswin.isOpen()) {
        Event event;
        while (levelswin.pollEvent(event)) {
            if (event.type == Event::Closed) {
                levelswin.close();
                return;
            }
            if (event.KeyPressed && event.key.code == Keyboard::Escape) {
                levelswin.close();
                return;
            }
            if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {
                handleLevelClick(event.mouseButton.x, event.mouseButton.y, levelsSprites, selectedLevel);
                if (selectedLevel != 0) {
                    levelSelected = true;
                    levelswin.close();
                }
            }
        }

        levelswin.clear();
        levelswin.draw(Levelsback);
        for (int i = 0; i < 9; i++) {
            levelswin.draw(levelsSprites[i]);
            levelswin.draw(levelsText[i]);
        }
        levelswin.display();
    }

    if (levelSelected) {
        RenderWindow mainwindow(VideoMode(1600, 900), "Main Window", Style::Default);
        Texture playerTex;
        playerTex.loadFromFile("Mario/Player.png");
        Player player(&playerTex); 
        if (!loadchosenlevel(selectedLevel, mainwindow, player)) {
            cerr << "Failed to load level " << selectedLevel << endl;
        }
    }
}
//                                                   END                                    