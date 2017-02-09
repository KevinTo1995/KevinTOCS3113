
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
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
	displayWindow = SDL_CreateWindow("Heavyarms Attacking", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	// setup
	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	Matrix projectionMatrix;;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

	GLuint roboTexture = LoadTexture(RESOURCE_FOLDER"heavy.png");
	GLuint gatlingTexture = LoadTexture(RESOURCE_FOLDER"gat.png");
	GLuint miniTexture = LoadTexture(RESOURCE_FOLDER"miniGat.png");

	float roboPosition = 0.0f;
	float gatPosition = 0.0f;
	float miniPosition = 0.0f;

	float angle = 0.0f;

	glClearColor(0.529f, 0.808f, 0.980f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float last_frame_ticks = 0.0f;

	SDL_Event event;
	bool done = false;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - last_frame_ticks;
		last_frame_ticks = ticks;

		angle += (3.141592653f / 512);


		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program.programID);

		//Building Robot
		modelMatrix.identity();

		program.setModelMatrix(modelMatrix);

		float roboVertices[] = { -2.5, -2.0, 2.5, -2.0, 2.5, 2.0, -2.5, -2.0, 2.5, 2.0, -2.5, 2.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, roboVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float roboTexCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, roboTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, roboTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building Gatling top right
		modelMatrix.identity(); 
		modelMatrix.Translate(1.27, 0.30, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		float gatVertices[] = { -2.5, -2.0, 2.5, -2.0, 2.5, 2.0, -2.5, -2.0, 2.5, 2.0, -2.5, 2.0 };

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		float gatTexCoords[] = { 1.0, 1.0, 2.0, 1.0, 2.0, 0.0, 1.0, 1.0, 2.0, 0.0, 1.0, 0.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, gatlingTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building Gatling bottom right
		modelMatrix.identity();
		modelMatrix.Translate(1.27, -0.32, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, gatlingTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building Gatling top left
		modelMatrix.identity();
		modelMatrix.Translate(-1.48, 0.30, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, gatlingTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building Gatling bottom left
		modelMatrix.identity();
		modelMatrix.Translate(-1.48, -0.32, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, gatlingTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building mini Gatling top left
		modelMatrix.identity();
		modelMatrix.Translate(0.05, 0.16, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, miniTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);


		//Building mini Gatling top right
		modelMatrix.identity();
		modelMatrix.Translate(-.25, 0.16, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, miniTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building mini Gatling bottom left
		modelMatrix.identity();
		modelMatrix.Translate(-.47, 0.08, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, miniTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		//Building mini Gatling bottom right
		modelMatrix.identity();
		modelMatrix.Translate(.28, 0.08, 0.0);
		modelMatrix.Rotate(angle);
		modelMatrix.Translate(0.0, 0.0, 0.0);
		program.setModelMatrix(modelMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, gatVertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, gatTexCoords);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glBindTexture(GL_TEXTURE_2D, miniTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);


		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}