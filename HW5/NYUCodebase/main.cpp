/*
Kevin To CS3113 Homework 5
Gundam Defenders Collision 

Create a simple Separated Axis Collision demo using colliding
rectangles or polygons.
(You will be provided with the SAT collision function).
It must have at least 3 objects colliding with each other and
responding to collisions. They must be rotated and scaled!
*/

#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <ctime>
#include <vector>
#include <algorithm>
#include <math.h>

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
enum GameState { STATE_MAIN_MENU, STATE_GAME_LEVEL };
bool gameRunning = true;
bool playerWins = false;
enum Type { PLAYER, VAYEATE };
int gameState;
ShaderProgram* program;

//Time Values
float lastFrameTicks = 0.0f;
float elapsed;

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


struct Vector {
	Vector() {

	}
	Vector(float X, float Y) {
		x = X;
		y = Y;
	}
	float x, y;

	float length() {
		float len = sqrtf(x*x + y*y);
		return len;
	}
};
bool testSATSeparationForEdge(float edgeX, float edgeY, const std::vector<Vector> &points1, 
	const std::vector<Vector> &points2, Vector &penetration) {
	float normalX = -edgeY;
	float normalY = edgeX;
	float len = sqrtf(normalX*normalX + normalY*normalY);
	normalX /= len;
	normalY /= len;

	std::vector<float> e1Projected;
	std::vector<float> e2Projected;

	for (int i = 0; i < points1.size(); i++) {
		e1Projected.push_back(points1[i].x * normalX + points1[i].y * normalY);
	}
	for (int i = 0; i < points2.size(); i++) {
		e2Projected.push_back(points2[i].x * normalX + points2[i].y * normalY);
	}

	std::sort(e1Projected.begin(), e1Projected.end());
	std::sort(e2Projected.begin(), e2Projected.end());

	float e1Min = e1Projected[0];
	float e1Max = e1Projected[e1Projected.size() - 1];
	float e2Min = e2Projected[0];
	float e2Max = e2Projected[e2Projected.size() - 1];

	float e1Width = fabs(e1Max - e1Min);
	float e2Width = fabs(e2Max - e2Min);
	float e1Center = e1Min + (e1Width / 2.0);
	float e2Center = e2Min + (e2Width / 2.0);
	float dist = fabs(e1Center - e2Center);
	float p = dist - ((e1Width + e2Width) / 2.0);

	if (p >= 0) {
		return false;
	}

	float penetrationMin1 = e1Max - e2Min;
	float penetrationMin2 = e2Max - e1Min;

	float penetrationAmount = penetrationMin1;
	if (penetrationMin2 < penetrationAmount) {
		penetrationAmount = penetrationMin2;
	}

	penetration.x = normalX * penetrationAmount;
	penetration.y = normalY * penetrationAmount;

	return true;
}

bool penetrationSort(Vector &p1, Vector &p2) {
	return p1.length() < p2.length();
}

bool checkSATCollision(const std::vector<Vector> &e1Points, const std::vector<Vector> &e2Points) {
	Vector penetration;
	std::vector<Vector> penetrations;
	for (int i = 0; i < e1Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e1Points.size() - 1) {
			edgeX = e1Points[0].x - e1Points[i].x;
			edgeY = e1Points[0].y - e1Points[i].y;
		}
		else {
			edgeX = e1Points[i + 1].x - e1Points[i].x;
			edgeY = e1Points[i + 1].y - e1Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);
		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}
	for (int i = 0; i < e2Points.size(); i++) {
		float edgeX, edgeY;

		if (i == e2Points.size() - 1) {
			edgeX = e2Points[0].x - e2Points[i].x;
			edgeY = e2Points[0].y - e2Points[i].y;
		}
		else {
			edgeX = e2Points[i + 1].x - e2Points[i].x;
			edgeY = e2Points[i + 1].y - e2Points[i].y;
		}
		Vector penetration;
		bool result = testSATSeparationForEdge(edgeX, edgeY, e1Points, e2Points, penetration);

		if (!result) {
			return false;
		}
		penetrations.push_back(penetration);
	}

	std::sort(penetrations.begin(), penetrations.end(), penetrationSort);
	penetration = penetrations[0];

	Vector e1Center;
	for (int i = 0; i < e1Points.size(); i++) {
		e1Center.x += e1Points[i].x;
		e1Center.y += e1Points[i].y;
	}
	e1Center.x /= (float)e1Points.size();
	e1Center.y /= (float)e1Points.size();

	Vector e2Center;
	for (int i = 0; i < e2Points.size(); i++) {
		e2Center.x += e2Points[i].x;
		e2Center.y += e2Points[i].y;
	}
	e2Center.x /= (float)e2Points.size();
	e2Center.y /= (float)e2Points.size();

	Vector ba;
	ba.x = e1Center.x - e2Center.x;
	ba.y = e1Center.y - e2Center.y;

	if ((penetration.x * ba.x) + (penetration.y * ba.y) < 0.0f) {
		penetration.x *= -1.0f;
		penetration.y *= -1.0f;
	}

	return true;
}

Vector rotate_position(float cx, float cy, float angle, Vector position)
{
	position.x -= cx;
	position.y -= cy;
	float x = position.x * cos(angle) - position.y * sin(angle);
	float y = position.x * sin(angle) + position.y * cos(angle);
	position.x = x + cx;
	position.y = y + cy;
	return position;
}


//Class Entity
class Entity {
public:
	float positionX, positionY;
	float top, bottom, left, right;
	float speedX, speedY;
	float accelerationX, accelerationY;
	float u, v, width, height;
	float sizeX, sizeY;
	std::vector<Vector> edges;
	float angle;
	Type type;
	Matrix entityMatrix;

	Entity() {}

	Entity(float x, float y, float spriteU, float spriteV, float spriteWidth, float spriteHeight, float dx, float dy, float sx, float sy,float ax, float ay, float rotate) {
		positionX = x;
		positionY = y;
		speedX = dx;
		speedY = dy;
		sizeX = sx;
		sizeY = sy;
		accelerationX = ax;
		accelerationY = ay;
		top = y + 0.05f * sizeY;
		bottom = y - 0.05f * sizeY;
		right = x + 0.05f * sizeX;
		left = x - 0.05f * sizeX;
		u = spriteU;
		v = spriteV;
		width = spriteWidth;
		height = spriteHeight;
		entityMatrix.identity();
		entityMatrix.Translate(x, y, 0);

		angle = rotate;
		edges.push_back(rotate_position(positionX, positionY, angle, Vector(left, top)));
		edges.push_back(rotate_position(positionX, positionY, angle, Vector(right, top)));
		edges.push_back(rotate_position(positionX, positionY, angle, Vector(left, bottom)));
		edges.push_back(rotate_position(positionX, positionY, angle, Vector(right, bottom)));

	}

	//Draw Object Funtion
	void draw() {
		entityMatrix.identity();
		entityMatrix.Translate(positionX, positionY, 0);
		entityMatrix.Rotate(angle);
		program->setModelMatrix(entityMatrix);

		std::vector<float> vertexData;
		std::vector<float> texCoordData;
		float texture_x = u;
		float texture_y = v;
		vertexData.insert(vertexData.end(), {
			(-0.1f * sizeX), 0.1f * sizeY,
			(-0.1f * sizeX), -0.1f * sizeY,
			(0.1f * sizeX), 0.1f * sizeY,
			(0.1f * sizeX), -0.1f * sizeY,
			(0.1f * sizeX), 0.1f * sizeY,
			(-0.1f * sizeX), -0.1f * sizeY,
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
	void updateY(float elapsed) {
			speedY += accelerationY;
			positionY += speedY * elapsed;
			top += speedY * elapsed;
			bottom += speedY * elapsed;
		
	}
	void updateX(float elapsed) {
			speedX += accelerationX;
			positionX += speedX * elapsed;
			left += speedX * elapsed;
			right += speedX * elapsed;
	}
	void updateXandY(float elapsed) {
		speedX += accelerationX;
		positionX += speedX * elapsed;
		left += speedX * elapsed;
		right += speedX * elapsed;

		speedY += accelerationY;
		positionY += speedY * elapsed;
		top += speedY * elapsed;
		bottom += speedY * elapsed;

		for (size_t i = 0; i < edges.size(); i++) {
			edges[i].x += speedX * elapsed;
			edges[i].y += speedY * elapsed;
		}
	}
};

//Global Entities
Entity player;
std::vector<Entity> vayeate;
std::vector<Entity> ground;

//Main Menu Text Settings and Rendering
void RenderMainMenu() {

	//Display Text
	modelMatrix.identity();
	modelMatrix.Translate(-2.8f, 1.0f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "Gundam Defenders", 0.4f, 0.0001f);


	modelMatrix.identity();
	modelMatrix.Translate(-1.8f, 0.5f, 0.0f);
	program->setModelMatrix(modelMatrix);
	DrawText(program, fontSheet, "Collision", 0.4f, 0.0001f);

	modelMatrix.identity();
	modelMatrix.Translate(-2.7f, -0.50f, 0.0f);
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
	DrawText(program, fontSheet, "SPACE TO STOP", 0.2f, 0.0001f);

}


//Game Menu Rendering
void RenderGameLevel() {
	player.draw();
	for (size_t i = 0; i < vayeate.size(); i++) {
		vayeate[i].draw();
	}
	for (size_t i = 0; i < ground.size(); i++) {
		ground[i].draw();
	}
}

//Game Update
void UpdateGameLevel(float elapsed) {

	//Heavyarms Movement and Stopping
	if (moveDown) {
		player.speedY = -1.25f;
	}
	if (moveUp) {
		player.speedY = 1.25f;
	}
	if (moveLeft) {
		player.speedX = -1.25f;
	}
	if (moveRight) {
		player.speedX = 1.25f;

	}
	if (shootBullet) {
		player.speedY = 0.0f;
		player.speedX = 0.0f;
	}

	//Vayeate Movement
	player.updateXandY(elapsed);
	for (size_t j = 0; j < ground.size(); j++) {
		if (checkSATCollision(player.edges, ground[j].edges)) {
			player.speedX = player.speedX * -1;
			player.speedY = player.speedY * -1;
		}
	}

	for (size_t i = 0; i < vayeate.size(); i++) {
		vayeate[i].updateXandY(elapsed);
		if (checkSATCollision(player.edges,vayeate[i].edges)) {
			if ((moveLeft && moveRight) == 0) {
				player.speedX = player.speedX * -1;
			}
			if ((moveUp && moveDown) == 0) {
				player.speedY = player.speedY * -1;
			}

			if (player.speedX == 0.0f) {
				vayeate[i].speedX = vayeate[i].speedX * -1;
			}
			else { vayeate[i].speedX = vayeate[i].speedX * -1; }
			if (player.speedY == 0.0f) {
				vayeate[i].speedY = vayeate[i].speedY * -1;
			}
			else { vayeate[i].speedY = vayeate[i].speedY * -1; }
		}
		
		for (size_t j = 0; j < ground.size(); j++) {
			if (checkSATCollision(vayeate[i].edges, ground[j].edges)) {
				vayeate[i].speedX = vayeate[i].speedX * -1;
				vayeate[i].speedY = vayeate[i].speedY * -1;
				}
			}
		for (size_t j = 0; j < vayeate.size(); j++) {
			if (checkSATCollision(vayeate[i].edges, vayeate[j].edges)) {
				vayeate[i].speedX = vayeate[i].speedX * -1;
				vayeate[i].speedY = vayeate[i].speedY * -1;
				vayeate[j].speedX = vayeate[j].speedX * -1;
				vayeate[j].speedY = vayeate[j].speedY * -1;
			}
		}
			
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
	player = Entity(0.0f, 1.50f, 0.0f / 1024.0f, 0.0f / 1024.0f, 93.0f / 1024.0f, 98.0f / 1024.0f, 
		0.0f, 0.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f);
	//initalize Vayeates
	for (int i = 0; i < 2; i++) {
		vayeate.push_back(Entity(-2.0f + (i * -1.0f) , 0.0f + (i * -1.0f),
			0.0f / 1024.0f, 100.0f / 1024.0f, 79.0f / 1024.0f, 100.0f / 1024.0f, -1.0f, 1.0f, 4.0f + (i * 1.0f),
			4.0f + (i * 1.0f), 0.0f, 0.0f, 0.0f + (i*.435f)));
		vayeate.push_back(Entity(2.0f + (i * 1.0f), 0.0f + (i * -1.0f),
			0.0f / 1024.0f, 100.0f / 1024.0f, 79.0f / 1024.0f, 100.0f / 1024.0f, 1.0f, 1.0f, 4.0f + (i * 1.0f),
			4.0f + (i * 1.0f), 0.0f, 0.0f, .435f + (i*.835f)));
	}
	//initalize Platforms
	ground.push_back(Entity(0.0f, -2.15f, 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 41.0f, 1.0f, 0.0f, 0.0f, 0.0f));
	ground.push_back(Entity(0.0f,  2.15f, 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 41.0f, 1.0f, 0.0f, 0.0f, 0.0f));
	ground.push_back(Entity(-3.90f, 0, 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 1.0f, 34.0f, 0.0f, 0.0f, 0.0f));
	ground.push_back(Entity(3.90, 0, 0.0f / 1024.0f, 258.0f / 1024.0f, 70.0f / 1024.0f, 70.0f / 1024.0f, 0.0f, 0.0f, 1.0f, 34.0f, 0.0f, 0.0f, 0.0f));

	
	Matrix space;
	SDL_Event event;
	bool done = false;
	while (!done) {
		//game controls
		while (SDL_PollEvent(&event)) {
			//closing
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE || event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
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
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN ) {
					moveDown = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) {
					moveLeft = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) {
					moveRight = true;
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_UP ) {
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
	displayWindow = SDL_CreateWindow("Gundam Defenders Platformer - Kevin To", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
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
	spriteSheet = LoadTexture(RESOURCE_FOLDER"robo2.png");
	background = LoadTexture(RESOURCE_FOLDER"space.png");

	runGame();

	SDL_Quit();
	return 0;
}
