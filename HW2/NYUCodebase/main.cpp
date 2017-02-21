
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <ctime>

#include "Matrix.h"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

// Paddle class
class Paddle {
public:
	Paddle(float lt, float rt, float tp, float bt) : left(lt), right(rt), top(tp), bottom(bt) {}

	float left;
	float right;
	float top;
	float bottom;
};

// Ball class
class Ball {
public:
	Ball(float posX, float posY, float vel, float spd, float acc, float dirX, float dirY) : positionX(posX), positionY(posY), speed(spd), accel(acc), directionX(dirX), directionY(dirY) {}
	Ball() {}

	float positionX = 0.0f;
	float positionY = 0.0f;
	float speed = 0.4f;
	float accel = 5.0f;
	float directionX = (float)(rand() % 5 + 1);
	float directionY = (float)(rand() % 5 + 1);

	void reset() {
		positionX = 0.0f;
		positionY = 0.0f;
		speed = 0.4f;
		accel = 5.0f;
		directionX = (float)(rand() % 5 + 1);
		directionY = (float)(rand() % 5 + 1);
	}

	void move(float elapsed) {
		positionX += (speed * directionX * elapsed);
		positionY += (speed * directionY * elapsed);
	}
};

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


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong, Kevin To", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// setup
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	Matrix projectionMatrix;;
	Matrix leftPaddleMatrix;
	Matrix rightPaddleMatrix;
	Matrix ballMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.6f, 3.6f, -2.25f, 2.25f, -1.0f, 1.0f);

	//Texture for Paddle and  Ball
	GLuint green = LoadTexture(RESOURCE_FOLDER"green.png");
	GLuint blue = LoadTexture(RESOURCE_FOLDER"blue.png");
	GLuint white = LoadTexture(RESOURCE_FOLDER"white.png");

	Paddle leftPaddle(-3.5f, -3.4f, 0.5f, -0.5f);
	Paddle rightPaddle(3.4f, 3.5f, 0.5f, -0.5f);
	Ball ball = Ball();

	float lastFrameTicks = 0.0f;

	SDL_Event event;
	bool done = false;
	bool gameRunning = false;
	while (!done) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//game controls
		while (SDL_PollEvent(&event)) {

			//closing
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
			if (event.type == SDL_KEYDOWN) {
				// Game Start
				if (event.key.keysym.scancode == SDL_SCANCODE_SPACE && !gameRunning)
					gameRunning = true;

				// Left Paddle
				if (event.key.keysym.scancode == SDL_SCANCODE_W && leftPaddle.top < 2.25f) {
					leftPaddle.top += 0.5f;
					leftPaddle.bottom += 0.5f;
					leftPaddleMatrix.Translate(0.0f, 0.5f, 0.0f);
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_S && leftPaddle.bottom > -2.25f) {
					leftPaddle.top -= 0.5f;
					leftPaddle.bottom -= 0.5f;
					leftPaddleMatrix.Translate(0.0f, -0.5f, 0.0f);
				}

				// Right Paddle
				if (event.key.keysym.scancode == SDL_SCANCODE_UP && rightPaddle.top < 2.25f) {
					rightPaddle.top += 0.5f;
					rightPaddle.bottom += 0.5f;
					rightPaddleMatrix.Translate(0.0f, 0.5f, 0.0f);
				}
				else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && rightPaddle.bottom > -2.25f) {
					rightPaddle.top -= 0.5f;
					rightPaddle.bottom -= 0.5f;
					rightPaddleMatrix.Translate(0.0f, -0.5f, 0.0f);
				}
			}
		}
		//Drawing Paddles and Ball

		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(program.programID);

		float globalTextureCoords[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f };

		//left paddle
		program.setModelMatrix(leftPaddleMatrix);
		float leftPaddleCoords[] = { -3.5f, -0.5f, -3.4f, -0.5f, -3.4f, 0.5f, -3.4f, 0.5f, -3.5f, 0.5f, -3.5f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddleCoords);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, globalTextureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, blue);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		// right paddle
		program.setModelMatrix(rightPaddleMatrix);
		float rightPaddleCoords[] = { 3.4f, -0.5f, 3.5f, -0.5f, 3.5f, 0.5f, 3.5f, 0.5f, 3.4f, 0.5f, 3.4f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddleCoords);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, globalTextureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, white);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		// ball
		program.setModelMatrix(ballMatrix);
		float ballCoords[] = { -0.1f, -0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 0.1f, 0.1f, -0.1f, 0.1f, -0.1f, -0.1f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballCoords);
		glEnableVertexAttribArray(program.positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, globalTextureCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, green);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);


		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		if (gameRunning)
		{
			// Paddle Collision
			if (ball.positionX <= leftPaddle.right && ball.positionY <= leftPaddle.top && ball.positionY >= leftPaddle.bottom ||
				ball.positionX >= rightPaddle.left && ball.positionY <= rightPaddle.top && ball.positionY >= rightPaddle.bottom)
			{
				ball.directionX *= -1;
				ball.speed += ball.accel * elapsed;
				ball.move(elapsed);
				ballMatrix.Translate((ball.speed * ball.directionX* elapsed), (ball.speed * ball.directionY* elapsed), 0.0f);
			}
			// Right side wins, screen turns blue
			else if (ball.positionX <= leftPaddle.left)
			{
				gameRunning = false;
				ballMatrix.Translate(-ball.positionX, -ball.positionY, 0.0f);
				ball.reset();
				glClearColor(0.0, 0.0, 0.5, 0.0);
			}

			// Left side wins, screen turns green
			else if (ball.positionX >= rightPaddle.right)
			{
				gameRunning = false;
				ballMatrix.Translate(-ball.positionX, -ball.positionY, 0.0f);
				ball.reset();
				glClearColor(0.0, 0.5, 0.0, 0.0);\
			}

			// Wall Collisions
			else if (ball.positionY + 0.1f >= 2.25f || ball.positionY - 0.1f <= -2.25f)
			{
				ball.directionY *= -1;
				ball.speed += ball.accel* elapsed;
				ball.move(elapsed);
				ballMatrix.Translate(ball.speed * ball.positionX* elapsed, ball.speed * ball.positionY* elapsed, 0.0f);
			}

			// Ball Movement
			else
			{
				ball.move(elapsed);
				ballMatrix.Translate((ball.speed * ball.directionX* elapsed), (ball.speed * ball.directionY* elapsed), 0.0f);
			}
		}
		SDL_GL_SwapWindow(displayWindow);

	}

	SDL_Quit();
	return 0;
}