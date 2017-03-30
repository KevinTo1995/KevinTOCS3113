/*
Kevin To CS3113 Homework 3
Gundam Defenders "Space Invaders"
• Make Spac`e Invaders
• It must have 2 states: TITLE SCREEN, GAME
• It must display text
• It must use sprite sheets
• You can use any graphics you want (it doesn’thave to be in space! :)

Has three states: Main Menu State, Game State and Game Over State.
Player Instructions:
Press Space to Start game.
Use Arrow Keys to move UP,DOWN,LEFT, and Right.
Press Space to shoot.
Eliminate all Vayeates to win.

*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <ctime>
#include <vector>

#include "Matrix.h"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

// SDL,Object Matrix, and Global values
SDL_Window* displayWindow;

GLuint fontSheet;
GLuint spriteSheet;
GLuint background;

Matrix projectionMatrix;
Matrix viewMatrix;
Matrix modelMatrix;

float globalTextureCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL,STATE_GAME_OVER };
bool gameRunning = true;
bool playerWins = false;
enum Type { PLAYER, VAYEATE };
int gameState;
ShaderProgram* program;

//Time Values
float lastFrameTicks = 0.0f;
float elapsed;
float playerLastShot = 0.0f;
float enemyLastShot = 0.0f;

//Control Bools
bool moveUp = false;
bool moveDown = false;
bool moveLeft = false;
bool moveRight = false;
bool shootBullet = false;

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return retTexture;
}

//Draw Text Function
void DrawText(ShaderProgram* program, int fontTexture, std::string text, float size, float space) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {

		int spriteIndex = (int)text[i];

		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + space) * i) + (-0.5f * size), 0.5f * size,
			((size + space) * i) + (-0.5f * size), -0.5f * size,
			((size + space) * i) + (0.5f * size), 0.5f * size,
			((size + space) * i) + (0.5f * size), -0.5f * size,
			((size + space) * i) + (0.5f * size), 0.5f * size,
			((size + space) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glUseProgram(program->programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Class Entity
class Entity {
public:
	float positionX, positionY;
	float top, bottom, left, right;
	float speedX, speedY;
	float u, v, width, height;
	float size;
	Type type;
	Matrix entityMatrix;

	Entity() {}

	Entity(float x, float y, float spriteU, float spriteV, float spriteWidth, float spriteHeight, float dx, float dy, float sz) {
		positionX = x;
		positionY = y;
		speedX = dx;
		speedY = dy;
		size = sz;
		top = y + 0.1f * size;
		bottom = y - 0.1f * size;
		right = x + 0.05f * size;
		left = x - 0.05f * size;
		u = spriteU;
		v = spriteV;
		width = spriteWidth;
		height = spriteHeight;
		entityMatrix.identity();
		entityMatrix.Translate(x, y, 0);
	}

	//Draw Object Funtion
	void draw() {
		entityMatrix.identity();
		entityMatrix.Translate(positionX, positionY, 0);
		program->setModelMatrix(entityMatrix);

		std::vector<float> vertexData;
		std::vector<float> texCoordData;
		float texture_x = u;
		float texture_y = v;
		vertexData.insert(vertexData.end(), {
			(-0.1f * size), 0.1f * size,
			(-0.1f * size), -0.1f * size,
			(0.1f * size), 0.1f * size,
			(0.1f * size), -0.1f * size,
			(0.1f * size), 0.1f * size,
			(-0.1f * size), -0.1f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + height,
			texture_x + width, texture_y,
			texture_x + width, texture_y + height,
			texture_x + width, texture_y,
			texture_x, texture_y + height,
		});

		glUseProgram(program->programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, spriteSheet);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
	}

};

//Global Entities
Entity player;
std::vector<Entity> vayeate;
std::vector<Entity> ground;
std::vector<Entity> bullets;
std::vector<Entity> lasers;

//Main Menu Text Settings and Rendering
void RenderMainMenu() {

	//Display Text
	modelMatrix.identity();
	modelMatrix.Translate(-2.8f, 1.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "Gundam Defenders", 0.4f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-2.7f, 0.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "PRESS SPACE TO START", 0.3f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-2.2f, -1.3f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "USE UP/DOWN/LEFT/RIGHT ", 0.2f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-1.8f, -1.5f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "ARROW KEYS TO MOVE", 0.2f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-1.5f, -2.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "SPACE TO FIRE", 0.2f, 0.0001f);
}

//Game Over Menu Text Settings and Rendering
void RenderGameOver() {

	//Display Text
	modelMatrix.identity();
	modelMatrix.Translate(-1.8f, 1.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	if (playerWins == true) {
		DrawText(program, fontSheet, " You Win!", 0.4f, 0.0001f);
	}
	else {
		DrawText(program, fontSheet, "Game Over", 0.4f, 0.0001f);
	}

	modelMatrix.identity();
	modelMatrix.Translate(-3.0f, 0.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "RESTART TO PLAY AGAIN", 0.3f, 0.0001f);
	gameRunning = false;
}

//Game Menu Rendering
void RenderGameLevel() {
	player.draw();
	for (size_t i = 0; i < vayeate.size(); i++) {
		vayeate[i].draw();
	}
	for (size_t i = 0; i < bullets.size(); i++) {
		bullets[i].draw();
	}
	for (size_t i = 0; i < lasers.size(); i++) {
		lasers[i].draw();
	}
}

//Game Update
void UpdateGameLevel(float elapsed) {

	//Heavyarms Movement and Shooting
	if (moveDown) {
		player.positionY -= player.speedY * elapsed;
		player.top -= player.speedY * elapsed;
		player.bottom -= player.speedY * elapsed;
	}
	if (moveUp) {
		player.positionY += player.speedY * elapsed;
		player.top += player.speedY * elapsed;
		player.bottom += player.speedY * elapsed;
	}
	if (moveLeft) {
		player.positionX -= player.speedX * elapsed;
		player.left -= player.speedX * elapsed;
		player.right -= player.speedX * elapsed;
	}
	if (moveRight) {
		player.positionX += player.speedX * elapsed;
		player.left += player.speedX * elapsed;
		player.right += player.speedX * elapsed;
	}
	//settign values and making bullets
	if (shootBullet) {
		if (playerLastShot > 0.3f) {
			playerLastShot = 0;
			bullets.push_back(Entity(player.positionX, player.positionY, 0.0f / 1024.0f, 
				234.0f / 1024.0f, 16.0f / 1024.0f, 10.0f / 1024.0f, 4.0f, 0, 0.5f));
		}
	}

	//Vayeate Movement
	for (size_t i = 0; i < vayeate.size(); i++) {
		vayeate[i].positionX -= vayeate[i].speedX * elapsed;
		vayeate[i].left -= vayeate[i].speedX * elapsed;
		vayeate[i].right -= vayeate[i].speedX * elapsed;

		vayeate[i].positionY += vayeate[i].speedY * elapsed;
		vayeate[i].top += vayeate[i].speedY * elapsed;
		vayeate[i].bottom += vayeate[i].speedY * elapsed;

		if ((vayeate[i].top > 2.0f && vayeate[i].speedY > 0) || (vayeate[i].bottom < -2.0f && vayeate[i].speedY < 0)) {
			for (size_t i = 0; i < vayeate.size(); i++) {
				vayeate[i].speedY = -vayeate[i].speedY;
			}
		}
		//if vayeate touches heavyarms, game ends
		if (vayeate[i].bottom < player.top &&
			vayeate[i].top > player.bottom &&
			vayeate[i].left < player.right &&
			vayeate[i].right > player.left) {
			gameState = STATE_GAME_OVER;
		}
		//if any vayeate reaches the endof left side, game ends
		if (vayeate[i].left < -4.00) {
			gameState = STATE_GAME_OVER;
		}

	}

	//Bullets Hitting vayeate
	std::vector<int> removeBullets;
	for (size_t i = 0; i < bullets.size(); i++) {
		bullets[i].positionX += bullets[i].speedX * elapsed;
		bullets[i].right += bullets[i].speedX * elapsed;
		bullets[i].left += bullets[i].speedX * elapsed;

		for (size_t j = 0; j < vayeate.size(); j++) {
			if (vayeate[j].bottom < bullets[i].top &&
				vayeate[j].top > bullets[i].bottom &&
				vayeate[j].left < bullets[i].right &&
				vayeate[j].right > bullets[i].left) {
				vayeate.erase(vayeate.begin() + j);
				removeBullets.push_back(i);
			}
		}
	}

	//removing bullets
	for (int i = 0; i < removeBullets.size(); i++) {
		bullets.erase(bullets.begin() + removeBullets[i] - i);
	}

	//setting values for lasers
	if (enemyLastShot > 0.5f) {
		enemyLastShot = 0;
		int randomLasers = rand() % vayeate.size();
		lasers.push_back(Entity(vayeate[randomLasers].positionX, vayeate[randomLasers].positionY,
			0.0f / 1024.0f, 202.0f / 1024.0f, 32.0f / 1024.0f, 30.0f / 1024.0f, -2.0f, 0, 1.0f));
	}

	//if laser hits player, game ends
	for (size_t i = 0; i < lasers.size(); i++) {
		lasers[i].positionX += lasers[i].speedX * elapsed;
		lasers[i].left += lasers[i].speedX * elapsed;
		lasers[i].right += lasers[i].speedX * elapsed;
		
		if (lasers[i].bottom < player.top &&
			lasers[i].top > player.bottom &&
			lasers[i].left < player.right &&
			lasers[i].right > player.left) {
			gameState = STATE_GAME_OVER;
		}
	}

	//Kill all vayeate, Win Game
	if (vayeate.size() == 0) {
		playerWins = true;
		gameState = STATE_GAME_OVER;
	}
		
}
//background for game
void gameBackground() {
	Matrix space;
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program->programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	modelMatrix.identity();
	program->setModelMatrix(space);

	float backgroundV[] = { -4.25f, -2.25f, 4.25f, -2.25f, 4.25f, 2.25f, 4.25f, 2.25f,  -4.25f, 2.25f,  -4.25f, -2.25f, };
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, backgroundV);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, globalTextureCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, background);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);

	float ticks = (float)SDL_GetTicks() / 1000.0f;
	elapsed = ticks - lastFrameTicks;
	lastFrameTicks = ticks;
	playerLastShot += elapsed;
	enemyLastShot += elapsed;
}

//render Menus and background
void Render() {

	glClear(GL_COLOR_BUFFER_BIT);
	gameBackground();
	switch (gameState) {
	case STATE_MAIN_MENU:
		RenderMainMenu();
		break;
	case STATE_GAME_LEVEL:
		RenderGameLevel();
		break;
	case STATE_GAME_OVER:
		RenderGameOver();
		break;
	}
	SDL_GL_SwapWindow(displayWindow);
}

//updates game level when active
void Update(float elapsed) {
	switch (gameState) {
	case STATE_GAME_LEVEL:
		UpdateGameLevel(elapsed);
		break;
	}
}

//Initializes Entities, controls, and starts game
void runGame() {
	//initialize Player
	player = Entity(-3.65f, 0.0f, 0.0f / 1024.0f, 0.0f / 1024.0f, 93.0f / 1024.0f, 98.0f / 1024.0f, 3.0f, 3.0f, 2.5f);
	//initalize Vayeates
	for (int i = 0; i < 25; i++) {
		vayeate.push_back(Entity(0.7 + (i % 5) * 0.7, 2.0 - (i / 5 * 0.7),
			0.0f / 1024.0f, 100.0f / 1024.0f, 79.0f / 1024.0f, 100.0f / 1024.0f, 0.03f, 1.0f,2.5f));
	}
	//initalize Ground
	for (int i = 0; i < 55; i++) {
		ground.push_back(Entity(-2.5f + (i) * 0.2f, 0.0f - (4 * 0.5f), 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f,1.0f));
	}

	for (int i = 0; i < 4; i++) {
		ground.push_back(Entity(-2.0f + (i)* 0.2f, 0.0f - (1 * 0.5f), 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 1.0f));
	}

	for (int i = 0; i < 4; i++) {
		ground.push_back(Entity(1.0f + (i)* 0.2f, -1.8f, , 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 1.0f));
	}

	Matrix space;
	SDL_Event event;
	bool done = false;
	while (!done) {
		//game controls
		while (SDL_PollEvent(&event)) {
			//closing
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			switch (event.type) {
			case SDL_KEYDOWN:
				// Starts game level or fires bullets
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					if (gameState == STATE_MAIN_MENU) {
						gameState = STATE_GAME_LEVEL;
					}
					else {
						shootBullet = true;
					}
				}
				//movements
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && player.bottom > -2.25f) {
					moveDown = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && player.left > -4.0f) {
					moveLeft = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && player.right < 4.0f) {
					moveRight = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_UP && player.top < 2.25f) {
					moveUp = true;
				}
				break;

			//To stop Player from moving due to global value usage
			case SDL_KEYUP:
				if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) {
					moveDown = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_UP) {
					moveUp = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					moveLeft = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					moveRight = false;
				}
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
					shootBullet = false;
				}
				break;
			}
		}

		if (gameRunning) {
			Update(elapsed);
			Render();
		}
	}
	return;
}

int main(int argc, char *argv[])
{
	srand(time(NULL));
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Gundam Defenders - Kevin To", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// setup
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	projectionMatrix.setOrthoProjection(-4.25f, 4.25f, -2.25f, 2.25f, -1.0f, 1.0f);
	program->setModelMatrix(modelMatrix);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);

	//load sprite sheets and textures
	fontSheet = LoadTexture(RESOURCE_FOLDER"font1.png");
	spriteSheet = LoadTexture(RESOURCE_FOLDER"sprites.png");
	background = LoadTexture(RESOURCE_FOLDER"space.png");

	runGame();

	SDL_Quit();
	return 0;
}