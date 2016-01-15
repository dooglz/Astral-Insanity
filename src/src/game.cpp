#include "game.h"
#include "config.h"
#include "particles.h"
#include <SFML/Graphics.hpp>

Sprite *playerSprite;
Sprite *backgroundSprite;
Sprite *bulletsprite;
Sprite *powersprite;
Text *scoreText;
Text *pausedText;
ParticleSystem *ps;

extern RenderWindow *window;
extern Gamestates state;
extern Font *gameFont;
extern Texture *textures[TEX_COUNT];

int playerlives = 3;
static Sprite *enemies[MAX_ENEMIES];
 int PowerChance = 0;
static unsigned int currentEnemies = 0;
float playerMoveSpeed = 600.0f;
unsigned int score = 0;
float runTime = 0; // time in seconds that the game has been running

static Vector2f GetNewEnemyPos() { return Vector2f((float)(rand() % GAME_WORLD_Y), -128.0f); }

void LoadGameContent() {
  scoreText = new Text();
  scoreText->setFont(*gameFont);
  scoreText->setPosition(0, 0);
  scoreText->setCharacterSize(24);
  scoreText->setColor(Color::Red);

  playerSprite = new Sprite();
  playerSprite->setTexture(*textures[0]);
  playerSprite->setPosition(512, 256);
  bulletsprite = new Sprite();
  bulletsprite->setTexture(*textures[10]);
  bulletsprite->setPosition(0, -128.0f);

  powersprite = new Sprite();

  backgroundSprite = new Sprite();
  backgroundSprite->setTexture(*textures[11]);
  backgroundSprite->setPosition(GAME_WORLD_X, GAME_WORLD_Y);
  auto c = backgroundSprite->getColor();
  c.a = 100;
  backgroundSprite->setColor(c);
  for (auto &e : enemies) {
    e = new Sprite();
    e->setTexture(*textures[(rand() % 7) + 3]);
    e->setPosition(GetNewEnemyPos());
  }

  ps = new ParticleSystem(Vector2u(GAME_RESOULUTION));
  ps->setDissolutionRate(1);
  //prefuel
  ps->fuel(250, Vector2f(0, GAME_WORLD_Y), Vector2f(0, GAME_WORLD_X));
}
void UnLoadGameContent() {
  scoreText = new Text();
  scoreText->setFont(*gameFont);
  scoreText->setPosition(0, 0);
  scoreText->setCharacterSize(24);
  scoreText->setColor(Color::Red);
  for (auto &e : enemies) {
    delete e;
    e = nullptr;
  }
}
void FireBullet() {
  if (bulletsprite->getPosition().y <= -128.0f) {
    bulletsprite->setPosition(playerSprite->getPosition().x + 30,
                              playerSprite->getPosition().y - 1);
  }
}

void GameHandleEvent(Event &e) {
  if (e.type == Event::KeyPressed) {
    if (e.key.code == Keyboard::P) {
      state = Gamestates::Pause;
    }
  }
}

void Normalize(Vector2f &v) {
  float length = sqrt(v.x * v.x + v.y * v.y);
  if (length == 0.0f) {
    return;
  }
  v.x /= length;
  v.y /= length;
}

void GameUpdate(float deltaSeconds) {
	runTime += deltaSeconds;
  currentEnemies = min((int)ceil(runTime * 0.6f) + 1, MAX_ENEMIES);

  if (Keyboard::isKeyPressed(Keyboard::Space)) {
    FireBullet();
  }

  Vector2f moveDirection(0, 0);
  if (Keyboard::isKeyPressed(Keyboard::Up) || Keyboard::isKeyPressed(Keyboard::W)) {
    moveDirection += Vector2f(0, -1);
  }
  if (Keyboard::isKeyPressed(Keyboard::Down) || Keyboard::isKeyPressed(Keyboard::S)) {
    moveDirection += Vector2f(0, 1);
  }
  if (Keyboard::isKeyPressed(Keyboard::Left) || Keyboard::isKeyPressed(Keyboard::A)) {
    moveDirection += Vector2f(-1, 0);
  }
  if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::D)) {
    moveDirection += Vector2f(1, 0);
  }

  // Joystick input
  if (Joystick::isConnected(0)) {
    const float joystickX = Joystick::getAxisPosition(0, Joystick::X);
    const float joystickY = Joystick::getAxisPosition(0, Joystick::Y);

    if (abs(joystickX) > 40.0f) {
      moveDirection += Vector2f((float)((signbit(joystickX) * -2) + 1), 0);
    }
    if (abs(joystickY) > 40.0f) {
      moveDirection += Vector2f(0, (float)((signbit(joystickY) * -2) + 1));
    }

    if (Joystick::isButtonPressed(0, 2) && state == Gamestates::Game) {
      state = Gamestates::Pause;
    }
    if (Joystick::isButtonPressed(0, 0)) {
      FireBullet();
    }
  }
  if (playerSprite->getPosition().x < 0) {
    moveDirection.x = max(0.0f, moveDirection.x);
  }
  if (playerSprite->getPosition().y < 0) {
    moveDirection.y = max(0.0f, moveDirection.y);
  }
  if (playerSprite->getPosition().x > (GAME_WORLD_X - playerSprite->getLocalBounds().width)) {
    moveDirection.x = min(0.0f, moveDirection.x);
  }
  if (playerSprite->getPosition().y > (GAME_WORLD_Y - playerSprite->getLocalBounds().height)) {
    moveDirection.y = min(0.0f, moveDirection.y);
  }
  Normalize(moveDirection);
  moveDirection = (moveDirection * playerMoveSpeed) * deltaSeconds;

  playerSprite->setPosition(playerSprite->getPosition() + moveDirection);

  if (bulletsprite->getPosition().y > -128.0f) {
    bulletsprite->setPosition(bulletsprite->getPosition().x,
                              bulletsprite->getPosition().y - 1000.0f * deltaSeconds);
  }

  for (size_t i = 0; i < MAX_ENEMIES; i++) {
    if (i < currentEnemies) {
      // if not dead, move
      if (enemies[i]->getPosition().y < GAME_WORLD_X) {
        enemies[i]->setPosition(
            enemies[i]->getPosition() +
            Vector2f(sinf(runTime + i) * 100.0f * deltaSeconds, 50.0f * deltaSeconds));
        // collisions
        if (bulletsprite->getGlobalBounds().intersects(enemies[i]->getGlobalBounds())) {
          enemies[i]->setPosition(GetNewEnemyPos());
          score += 100;

		  PowerChance = rand() % 101;
		  if (PowerChance >= 30) {
			  powersprite->setPosition(enemies[i]->getPosition().x, enemies[i]->getPosition().y + 1);
			  powersprite->setTexture(*textures[13]);
		  }
		  else if (PowerChance >= 60) {
			  powersprite->setPosition(enemies[i]->getPosition().x, enemies[i]->getPosition().y + 1);
			  powersprite->setTexture(*textures[14]);
		  }
		  else if (PowerChance >= 90) {
			  powersprite->setPosition(enemies[i]->getPosition().x, enemies[i]->getPosition().y + 1);
			  powersprite->setTexture(*textures[12]);
		  }
        }
        if (playerSprite->getGlobalBounds().intersects(enemies[i]->getGlobalBounds())) {
          enemies[i]->setPosition(GetNewEnemyPos());
          playerlives -= 1;
        }
      } else {
        // enemy is offscreen, kill
        enemies[i]->setPosition(GetNewEnemyPos());
      }
    } else {
      // if alive
      if (enemies[i]->getPosition().y != -128.0f) {
        // kill
        enemies[i]->setPosition(GetNewEnemyPos());
      }
    }
  }

  if (playerlives == 0) {
    // state = Gamestates::Dead;
  }
  static int pcount = 0;
  ++pcount;
  if(pcount % 5 ==0){
  ps->fuel(1, Vector2f(0, 0), Vector2f(0, GAME_WORLD_X));
  pcount = 0;
  ps->fuel(1, Vector2f(0, 0), Vector2f(0, GAME_WORLD_X));
  }
 
  ps->update(deltaSeconds);
  scoreText->setString("Score =" + to_string(score + ceil(runTime)) + "\n\n"
                                                                      "lives = " +
                       to_string(playerlives));
}

void GameRender() {
  window->draw(*ps);
  window->draw(*playerSprite);
  window->draw(*bulletsprite);
  for (size_t i = 0; i < currentEnemies; i++) {
    window->draw(*enemies[i]);
  }
  if (PowerChance >= 30) {
	  window->draw(*powersprite);
  }

  window->draw(*scoreText);
}

void ResetGame() {
  score = 0;
  runTime = 0;
  currentEnemies = 0;
  playerSprite->setPosition(512, 256);
  for (auto e : enemies) {
   e->setPosition(GetNewEnemyPos());
  }
}

