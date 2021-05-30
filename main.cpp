#include <SDL.h>
#include <iostream>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdlib.h>
#include <string>

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT (SCREEN_WIDTH * 3)
#define MS 10

SDL_Window* window;
SDL_Renderer* renderer;
bool gameOver = false;

//music and sounds
Mix_Music* music;
Mix_Chunk* sound;

//menu
SDL_Rect menuRect;
SDL_Texture* textScore;
SDL_Rect textScoreRect;
SDL_Texture* textLines;
SDL_Rect textLinesRect;
SDL_Texture* textSpeed;
SDL_Rect textSpeedRect;
SDL_Texture* textNext;
SDL_Rect textNextRect;
SDL_Texture* numberScore;
SDL_Rect numberScoreRect;
SDL_Texture* numberLines;
SDL_Rect numberLinesRect;
SDL_Texture* numberSpeed;
SDL_Rect numberSpeedRect;
TTF_Font* font;
int lines = 0;
int score = 0;
int speed = 0;

//board
const int totalX = 10;
const int totalY = 2 * totalX;
int sizeActual = SCREEN_WIDTH / totalX;
struct Square {
	int x; // pixels in x in the board
	int y; // pixels in y in the board
	bool active; // it indicates if the block is drawn in the board
	SDL_Color color;
};
struct Square matriz[totalY][totalX];

//figures
struct Figure {
	int blocks[8];
	int blocksX; //total of horizontal blocks 
	int bloclsY; //total of vertical blocks
	SDL_Color color;
	/*
	for example:
	figure:
	*
	* * *
	the coordinates are: 00,10,11,12: {0,0,1,0,1,1,1,2}
	*/
};
struct Figure figure[7];
int indexFigureBoard;
int indexFigureMenu;
int indexFigureRotate;
int actualFigure[8];//this has the coordinates of the actual figure on the board
int timeToUpdate = 0; //contador to move down the figure
int timeMax = 500; // time to move down the figure

//start
int timer = 0;
bool started = true;
SDL_Texture* textStart;
SDL_Rect startRect;
SDL_Texture* textIndication1;
SDL_Rect indication1Rect;
SDL_Texture* textIndication2;
SDL_Rect indication2Rect;
SDL_Texture* textIndication3;
SDL_Rect indication3Rect;
SDL_Texture* textIndication4;
SDL_Rect indication4Rect;

void generateIndications() {
	SDL_Surface* textSurface;
	SDL_Color white = { 255,255,255 };
	textSurface = TTF_RenderText_Solid(font, "Press Esc to exit", white);
	textIndication1 = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Press a,s,d to move the figure", white);
	textIndication2 = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Press R to restart", white);
	textIndication3 = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Press E to rotate the figure", white);
	textIndication4 = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
}

void changeTextTexture(std::string newText, SDL_Texture* &texture) {
	font = TTF_OpenFont("Oswald-Stencil.ttf", menuRect.h / 6);
	SDL_Surface* textSurface;
	SDL_Color white = { 255,255,255 };
	const char* t = newText.c_str();
	textSurface = TTF_RenderText_Solid(font, t, white);
	texture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
}

void changeTextStart(std::string newText) {
	font = TTF_OpenFont("Oswald-Stencil.ttf", (int)(SCREEN_WIDTH / 7.5));
	SDL_Surface* textSurface;
	SDL_Color white = { 255,255,255 };
	const char* t = newText.c_str();
	textSurface = TTF_RenderText_Solid(font, t, white);
	textStart = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
}

void start() {
	if (timer % 1000 == 0) {
		int number = timer / 1000 + 1;
		std::string num_str(std::to_string(number));
		changeTextStart(num_str);
	}
	timer += MS;
	if (timer >= 3000) {
		timer = 0;
		started = false;
	}
}

void generateMenu() {
	SDL_Color white = { 255,255,255 };
	SDL_Surface* textSurface;
	textSurface = TTF_RenderText_Solid(font, "Score", white);
	textScore = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Lines", white);
	textLines = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Speed", white);
	textSpeed = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "Next", white);
	textNext = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "0", white);
	numberScore = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "0", white);
	numberLines = SDL_CreateTextureFromSurface(renderer, textSurface);
	textSurface = TTF_RenderText_Solid(font, "0", white);
	numberSpeed = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_FreeSurface(textSurface);
}

void generateRectangle(SDL_Rect* rect, int x, int y, int w, int h) {
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}

void createBoard() {
	SDL_Color color = { 0,0,0 };
	for (int i = 0; i < totalX; i++) {
		for (int j = 0; j < totalY; j++) {
			matriz[j][i] = { i * sizeActual, j * sizeActual + menuRect.h, false, color };
		}
	}
}

//add the figure of the menu to the board, changes the matriz and update the actual figure
void addFigureToBoard() {
	//initial coordinate in x to start to draw the figure
	int initialX = totalX / 2 - figure[indexFigureBoard].blocksX / 2 - 1;
	for (int i = 0; i < 8; i += 2) {
		int posX = figure[indexFigureBoard].blocks[i];//position of the board x
		int posY = initialX + figure[indexFigureBoard].blocks[i + 1];//position of the board y
		matriz[posX][posY].color = figure[indexFigureMenu].color;
		matriz[posX][posY].active = true;
		actualFigure[i] = posX;
		actualFigure[i + 1] = posY;
	}
}

//change the images of the board and menu (index)
void changeFigures() {
	indexFigureBoard = indexFigureMenu;
	addFigureToBoard();
	indexFigureMenu = rand() % 7;
	indexFigureRotate = 0;
}

void initVariables() {
	srand((unsigned int)time(NULL));
	int random = rand() % 7;
	indexFigureMenu = random;
	indexFigureRotate = 0;

	generateRectangle(&menuRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 3);
	//menu
	generateRectangle(&textScoreRect, menuRect.w / 8, 0, menuRect.w / 4,
		menuRect.h / 6);
	generateRectangle(&textLinesRect, menuRect.w / 2 + menuRect.w / 8,
		textScoreRect.y, textScoreRect.w, textScoreRect.h);
	generateRectangle(&textSpeedRect, textScoreRect.x, menuRect.h / 2,
		textScoreRect.w, textScoreRect.h);
	generateRectangle(&textNextRect, textLinesRect.x,
		textSpeedRect.y, textScoreRect.w, textScoreRect.h);

	numberScoreRect.w = menuRect.w / 16;
	numberScoreRect.h = menuRect.h / 6;
	numberScoreRect.x = textScoreRect.x + (textScoreRect.w / 2) - (numberScoreRect.w / 2);
	numberScoreRect.y = textScoreRect.y + textScoreRect.h;

	numberLinesRect.w = numberScoreRect.w;
	numberLinesRect.h = numberScoreRect.h;
	numberLinesRect.x = numberScoreRect.x + menuRect.w / 2;
	numberLinesRect.y = numberScoreRect.y;

	numberSpeedRect.w = numberScoreRect.w;
	numberSpeedRect.h = numberScoreRect.h;
	numberSpeedRect.x = numberScoreRect.x;
	numberSpeedRect.y = textSpeedRect.y + textSpeedRect.h;

	startRect.w = SCREEN_WIDTH / 2;
	startRect.h = startRect.w;
	startRect.x = (SCREEN_WIDTH - startRect.w) / 2;
	startRect.y = menuRect.h + 3 * (SCREEN_HEIGHT - menuRect.h) / 4 - startRect.h / 2;//(SCREEN_HEIGHT + menuRect.h - startRect.h) / 2;

	indication1Rect.x = 0;
	indication1Rect.y = menuRect.h;
	indication1Rect.w = menuRect.w;
	indication1Rect.h = menuRect.h / 8;

	indication2Rect.x = 0;
	indication2Rect.y = indication1Rect.y + indication1Rect.h;
	indication2Rect.w = indication1Rect.w;
	indication2Rect.h = indication1Rect.h;

	indication3Rect.x = 0;
	indication3Rect.y = indication2Rect.y + indication2Rect.h;
	indication3Rect.w = indication1Rect.w;
	indication3Rect.h = indication1Rect.h;

	indication4Rect.x = 0;
	indication4Rect.y = indication3Rect.y + indication3Rect.h;
	indication4Rect.w = indication1Rect.w;
	indication4Rect.h = indication1Rect.h;

	figure[0] = { 0,0,1,0,2,0,3,0,1,4,{84, 153, 199} }; // vertical line
	figure[1] = { 0,0,0,1,1,0,1,1,2,2,{176, 58, 46} }; // square
	figure[2] = { 0,0,1,0,1,1,1,2,3,2,{33, 97, 140} };
	figure[3] = { 0,1,1,0,1,1,2,1,2,3,{247, 220, 111} };
	figure[4] = { 0,1,0,2,1,0,1,1,3,2,{142, 68, 173} };
	figure[5] = { 0,2,1,0,1,1,1,2,3,2,{34, 153, 84} };
	figure[6] = { 0,1,1,0,1,1,2,0,2,3,{243, 156, 18} };

	createBoard();
	changeFigures();

	font = TTF_OpenFont("Oswald-BoldItalic.ttf", menuRect.h / 6);
	generateIndications();
}



bool init() {

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", SDL_GetError(), NULL);
		SDL_Quit();
		std::cout << "SDL did not initialize " << std::endl;
		return false;
	}

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Image",
			"The image PNG is not working", NULL);
		return false;
	}

	if (TTF_Init() < 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Ttf",
			"The ttf is not working", NULL);
		return false;
	}
	if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE,
		&window, &renderer) < 0) {
		std::cout << "window or renderer are not initialized " << std::endl;
		return false;
	}
	SDL_SetWindowTitle(window, "Tetris");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Audio",
			"The audio is not working", NULL);
		return false;
	}
	return true;
}

void loadSound() {
	music = Mix_LoadMUS("batchbug-sweet-dreams.mp3");
	if (!music) {
		std::cout << "The music is not loaded" << std::endl;
	}
	sound = Mix_LoadWAV("hit.wav");
	if (!sound) {
		std::cout << "The sound is not loaded" << std::endl;
	}
	Mix_VolumeMusic(SDL_MIX_MAXVOLUME / 2);
	Mix_PlayMusic(music, -1);
}

bool detectRightColision() {
	for (int i = 0; i < 8; i += 2) {
		if (actualFigure[i + 1] >= totalX - 1) {
			return true;
		}
	}
	for (int i = 0; i < 8; i += 2) {
		if (matriz[actualFigure[i]][actualFigure[i + 1] + 1].active) {
			//the square below the figure is active
			bool figureColision = true;
			for (int j = 0; j < 8; j += 2) {
				if (actualFigure[i] == actualFigure[j] &&
					actualFigure[i + 1] + 1 == actualFigure[j + 1]) {
					figureColision = false;
					break;
				}
			}
			if (figureColision) {
				return true;
			}
		}
	}
	return false;
}

bool detectLeftColision() {
	for (int i = 0; i < 8; i += 2) {
		if (actualFigure[i + 1] <= 0) {
			return true;
		}
	}
	for (int i = 0; i < 8; i += 2) {
		if (matriz[actualFigure[i]][actualFigure[i + 1] - 1].active) {
			//the square below the figure is active
			bool figureColision = true;
			for (int j = 0; j < 8; j += 2) {
				if (actualFigure[i] == actualFigure[j] &&
					actualFigure[i + 1] - 1 == actualFigure[j + 1]) {
					figureColision = false;
					break;
				}
			}
			if (figureColision) {
				return true;
			}
		}
	}
	return false;
}

void moveLeftFigure() {
	if (!detectLeftColision()) {
		for (int i = 0; i < 8; i += 2) {
			//change color of the figure to black
			matriz[actualFigure[i]][actualFigure[i + 1]].color = { 0,0,0 };
			matriz[actualFigure[i]][actualFigure[i + 1]].active = false;
			//moving down the figure
			actualFigure[i + 1] -= 1;
		}
		for (int i = 0; i < 8; i += 2) {
			//painting the board with the correct color
			matriz[actualFigure[i]][actualFigure[i + 1]].color = figure[indexFigureBoard].color;
			matriz[actualFigure[i]][actualFigure[i + 1]].active = true;
		}
	}
}

void moveRightFigure() {
	if (!detectRightColision()) {
		for (int i = 0; i < 8; i += 2) {
			//change color of the figure to black
			matriz[actualFigure[i]][actualFigure[i + 1]].color = { 0,0,0 };
			matriz[actualFigure[i]][actualFigure[i + 1]].active = false;
			//moving down the figure
			actualFigure[i + 1] += 1;
		}
		for (int i = 0; i < 8; i += 2) {
			//painting the board with the correct color
			matriz[actualFigure[i]][actualFigure[i + 1]].color = figure[indexFigureBoard].color;
			matriz[actualFigure[i]][actualFigure[i + 1]].active = true;
		}
	}
}

//this method detects the colision of the figure with the blocks in the board and the 
//limits of the board
bool detectDownColision() {
	if (actualFigure[6] + 1 < totalY) {
		for (int i = 0; i < 8; i += 2) {
			if (matriz[actualFigure[i] + 1][actualFigure[i + 1]].active) {
				//the square below the figure is active
				bool figureColision = true;
				for (int j = 0; j < 8; j += 2) {
					if (actualFigure[i] + 1 == actualFigure[j] &&
						actualFigure[i + 1] == actualFigure[j + 1]) {
						figureColision = false;
						break;
					}
				}
				if (figureColision) {
					return true;
				}
			}
		}
		return false;
	}
	else {
		return true;
	}
}

bool completeLine(int coordinateY) {
	for (int i = 0; i < totalX; i++) {
		if (!matriz[coordinateY][i].active) {
			return false;
		}
	}
	//delete the complete line
	for (int i = 0; i < totalX; i++) {
		matriz[coordinateY][i].active = false;
		matriz[coordinateY][i].color = { 0,0,0 };
	}
	//move down the blocks on the board one position
	for (int i = coordinateY - 1; i >= 0; i--) {
		bool hasUp = false;
		for (int j = 0; j < totalX; j++) {
			if (matriz[i][j].active) {
				hasUp = true;
				matriz[i + 1][j].active = true;
				matriz[i + 1][j].color = matriz[i][j].color;
				matriz[i][j].active = false;
				matriz[i][j].color = { 0,0,0 };
			}
		}
		if (!hasUp)
			break;
	}
	lines++;
	score += 1;
	speed++;
	changeTextTexture(std::to_string(lines), numberLines);
	changeTextTexture(std::to_string(score), numberScore);
	changeTextTexture(std::to_string(speed), numberSpeed);
	if (timeMax > 100)
		timeMax -= 10;
	return true;
}

void restart() {
	for (int i = 0; i < totalY; i++) {
		for (int j = 0; j < totalX; j++) {
			if (matriz[i][j].active) {
				matriz[i][j].active = false;
				matriz[i][j].color = { 0,0,0 };
			}
		}
	}
	lines = 0;
	score = 0;
	speed = 0;
	timeMax = 500;
	changeTextTexture(std::to_string(lines), numberLines);
	changeTextTexture(std::to_string(score), numberScore);
	changeTextTexture(std::to_string(speed), numberSpeed);
	changeFigures();
	started = true;
}

void moveDownFigure() {
	if (!detectDownColision()) {
		for (int i = 0; i < 8; i += 2) {
			//change color of the figure to black
			matriz[actualFigure[i]][actualFigure[i + 1]].color = { 0,0,0 };
			matriz[actualFigure[i]][actualFigure[i + 1]].active = false;
			//moving down the figure
			actualFigure[i] += 1;
		}
		for (int i = 0; i < 8; i += 2) {
			//painting the board with the correct color
			matriz[actualFigure[i]][actualFigure[i + 1]].color = figure[indexFigureBoard].color;
			matriz[actualFigure[i]][actualFigure[i + 1]].active = true;
		}
	}
	else {
		//game over
		if (actualFigure[0] == figure[indexFigureBoard].blocks[0] &&
			actualFigure[2] == figure[indexFigureBoard].blocks[2] &&
			actualFigure[4] == figure[indexFigureBoard].blocks[4] &&
			actualFigure[6] == figure[indexFigureBoard].blocks[6]) {
			restart();
		}
		else {
			int k = 0;
			Mix_PlayChannel(1, sound, 0);
			//the figure is on a block below or on the end of the board
			int coordinateY = actualFigure[0];
			if (completeLine(coordinateY))
				k++;
			for (int i = 2; i < 8; i += 2) {
				if (coordinateY != actualFigure[i]) {
					coordinateY = actualFigure[i];
					if (completeLine(coordinateY)) {
						k++;
					}

				}
			}
			if (k > 1) {
				score += (k * (k - 1));
				changeTextTexture(std::to_string(score), numberScore);
			}
			changeFigures();
		}
	}
}

bool detectRotatedFigureColision(int* coordinates) {
	//disable the actual blocks of the figure
	for (int i = 0; i < 8; i += 2) {
		matriz[actualFigure[i]][actualFigure[i + 1]].active = false;
	}
	//check if there is no colision of the rotated figure
	for (int i = 0; i < 8; i += 2) {
		if (matriz[coordinates[i]][coordinates[i + 1]].active) {
			for (int j = 0; j < 8; j += 2) {
				matriz[actualFigure[j]][actualFigure[j + 1]].active = true;
			}
			return true;
		}
	}
	return false;
}

void rotateFigure(int* coordinates) {
	//it assign the black color to the normal figure, then updates the board with the
	//blocks of the rotated figure, activating the blocks and changing the respective color
	for (int i = 0; i < 8; i += 2) {
		matriz[actualFigure[i]][actualFigure[i + 1]].color = { 0,0,0 };
	}
	for (int i = 0; i < 8; i += 2) {
		actualFigure[i] = coordinates[i];
		actualFigure[i + 1] = coordinates[i + 1];
		matriz[coordinates[i]][coordinates[i + 1]].active = true;
		matriz[coordinates[i]][coordinates[i + 1]].color = figure[indexFigureBoard].
			color;
	}
}

void rotate() {
	switch (indexFigureBoard) {
	case 0:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[1] > 0 && actualFigure[1] < totalX - 2) {
				int coordinates[8] = {
					actualFigure[2], actualFigure[3] - 1,
					actualFigure[2],actualFigure[3],
					actualFigure[2],actualFigure[3] + 1,
					actualFigure[2],actualFigure[3] + 2
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[4] > 0 && actualFigure[4] < totalY - 2) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5],
					actualFigure[4] + 1, actualFigure[5],
					actualFigure[4] + 2, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[5] > 1 && actualFigure[5] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[4] , actualFigure[5] - 2,
					actualFigure[4] , actualFigure[5] - 1,
					actualFigure[4] , actualFigure[5],
					actualFigure[4] , actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			if (actualFigure[2] > 1 && actualFigure[2] < totalY - 1) {
				int coordinates[8] = {
					actualFigure[2] - 2 , actualFigure[3],
					actualFigure[2] - 1 , actualFigure[3],
					actualFigure[2] , actualFigure[3],
					actualFigure[2] + 1 , actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate = 0;
				}
			}
			break;
		}
		break;
	case 2:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[2] < totalY - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4] - 1, actualFigure[5] + 1,
					actualFigure[4], actualFigure[5],
					actualFigure[4] + 1, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[1] > 0) {
				int coordinates[8] = {
					actualFigure[4], actualFigure[5] - 1,
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1,
					actualFigure[4] + 1, actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[4] > 0) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5],
					actualFigure[4] + 1, actualFigure[5] - 1,
					actualFigure[4] + 1, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			if (actualFigure[3] > 1) {
				int coordinates[8] = {
					actualFigure[2] - 1, actualFigure[3] - 2,
					actualFigure[2], actualFigure[3] - 2,
					actualFigure[2], actualFigure[3] - 1,
					actualFigure[2], actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate = 0;
				}
			}
			break;
		}
		break;
	case 3:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[1] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5] - 1,
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[0] < totalY - 2) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1,
					actualFigure[4] + 1, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[1] > 0) {
				int coordinates[8] = {
					actualFigure[2], actualFigure[3] - 1,
					actualFigure[2], actualFigure[3],
					actualFigure[2], actualFigure[3] + 1,
					actualFigure[2] + 1, actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			if (actualFigure[1] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[2] - 1, actualFigure[3],
					actualFigure[2], actualFigure[3] - 1,
					actualFigure[2], actualFigure[3],
					actualFigure[2] + 1, actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate = 0;
				}
			}
			break;
		}
		break;
	case 4:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[6] < totalY - 1) {
				int coordinates[8] = {
					actualFigure[6] - 1, actualFigure[7],
					actualFigure[6], actualFigure[7],
					actualFigure[6], actualFigure[7] + 1,
					actualFigure[6] + 1, actualFigure[7] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[1] > 0) {
				int coordinates[8] = {
					actualFigure[2], actualFigure[3],
					actualFigure[2], actualFigure[3] + 1,
					actualFigure[2] + 1, actualFigure[3] - 1,
					actualFigure[2] + 1, actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[0] > 0) {
				int coordinates[8] = {
					actualFigure[0] - 1, actualFigure[1] - 1,
					actualFigure[0], actualFigure[1] - 1,
					actualFigure[0], actualFigure[1],
					actualFigure[0] + 1, actualFigure[1]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			if (actualFigure[5] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4] - 1, actualFigure[5] + 1,
					actualFigure[4], actualFigure[5] - 1,
					actualFigure[4], actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate = 0;
				}
			}
			break;
		}
		break;
	case 5:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[4] < totalY - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5],
					actualFigure[4] + 1, actualFigure[5],
					actualFigure[4] + 1, actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[3] > 0) {
				int coordinates[8] = {
					actualFigure[2], actualFigure[3] - 1,
					actualFigure[2], actualFigure[3],
					actualFigure[2], actualFigure[3] + 1,
					actualFigure[2] + 1, actualFigure[3] - 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[2] > 0) {
				int coordinates[8] = {
					actualFigure[2] - 1, actualFigure[3] - 1,
					actualFigure[2] - 1, actualFigure[3],
					actualFigure[2], actualFigure[3],
					actualFigure[2] + 1, actualFigure[3]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			if (actualFigure[3] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5] + 1,
					actualFigure[4], actualFigure[5] - 1,
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate = 0;
				}
			}
			break;
		}
		break;
	case 6:
		switch (indexFigureRotate) {
		case 0:
			if (actualFigure[5] < totalX - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5] - 1,
					actualFigure[4] - 1, actualFigure[5],
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 1:
			if (actualFigure[4] < totalY - 1) {
				int coordinates[8] = {
					actualFigure[4] - 1, actualFigure[5] + 1,
					actualFigure[4], actualFigure[5],
					actualFigure[4], actualFigure[5] + 1,
					actualFigure[4] + 1, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 2:
			if (actualFigure[3] > 0) {
				int coordinates[8] = {
					actualFigure[4], actualFigure[5] - 2,
					actualFigure[4], actualFigure[5] - 1,
					actualFigure[4] + 1, actualFigure[5] - 1,
					actualFigure[4] + 1, actualFigure[5]
				};
				if (!detectRotatedFigureColision(coordinates)) {
					rotateFigure(coordinates);
					indexFigureRotate++;
				}
			}
			break;
		case 3:
			int coordinates[8] = {
				actualFigure[2] - 1, actualFigure[3],
				actualFigure[2], actualFigure[3] - 1,
				actualFigure[2], actualFigure[3],
				actualFigure[2] + 1, actualFigure[3] - 1
			};
			if (!detectRotatedFigureColision(coordinates)) {
				rotateFigure(coordinates);
				indexFigureRotate = 0;
			}
			break;
		}
		break;
	}
}

void detectkeyExit() {
	SDL_Event event;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			gameOver = true;
		}
	}
}

//this method detects the keyboard and updates the action of a determined key
void detectKey() {
	SDL_Event event;
	const Uint8* keys = SDL_GetKeyboardState(NULL);
	while (SDL_PollEvent(&event)) {
		int typeEvent = event.type;
		if (typeEvent == SDL_QUIT) {
			gameOver = true;
		}
		else if (typeEvent == SDL_KEYDOWN) {
			if (keys[SDL_SCANCODE_ESCAPE]) {
				gameOver = true;
			}
			if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT]) {
				moveLeftFigure();
			}
			if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) {
				moveRightFigure();
			}
			if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) {
				moveDownFigure();
			}
			if (keys[SDL_SCANCODE_E]) {
				rotate();
			}
			if (keys[SDL_SCANCODE_R]) {
				restart();
			}
		}
	}
}

void update() {
	timeToUpdate += MS;
	if (timeToUpdate >= timeMax) {
		moveDownFigure();
		timeToUpdate = 0;
	}
}

void showStart() {
	SDL_RenderCopy(renderer, textStart, NULL, &startRect);
	SDL_RenderCopy(renderer, textIndication1, NULL, &indication1Rect);
	SDL_RenderCopy(renderer, textIndication2, NULL, &indication2Rect);
	SDL_RenderCopy(renderer, textIndication3, NULL, &indication3Rect);
	SDL_RenderCopy(renderer, textIndication4, NULL, &indication4Rect);
}

void showBoard() {
	SDL_Rect rect;
	rect.w = sizeActual;
	rect.h = sizeActual;
	for (int i = 0; i < totalY; i++) {
		for (int j = 0; j < totalX; j++) {
			rect.y = matriz[i][j].y;
			rect.x = matriz[i][j].x;
			SDL_SetRenderDrawColor(renderer, matriz[i][j].color.r, matriz[i][j].color.g,
				matriz[i][j].color.b, 0);
			SDL_RenderFillRect(renderer, &rect);
		}
	}
}

void showImageMenu() {
	int x = menuRect.w / 2 + menuRect.w / 4 - (sizeActual * figure[indexFigureMenu].blocksX) / 2;
	int y = menuRect.h / 2 + textSpeedRect.h + (menuRect.h - menuRect.h / 2 - textSpeedRect.h)
		/ 2 - (sizeActual * figure[indexFigureMenu].bloclsY) / 2;
	SDL_Rect square;
	square.w = sizeActual;
	square.h = sizeActual;
	SDL_SetRenderDrawColor(renderer, figure[indexFigureMenu].color.r,
		figure[indexFigureMenu].color.g, figure[indexFigureMenu].color.b, 0);
	for (int i = 0; i < 8; i = i + 2) {
		square.x = x + sizeActual * figure[indexFigureMenu].blocks[i + 1];
		square.y = y + sizeActual * figure[indexFigureMenu].blocks[i];
		SDL_RenderFillRect(renderer, &square);
	}
}

void showMenu() {
	SDL_SetRenderDrawColor(renderer, 50, 50, 50, 0);
	SDL_RenderFillRect(renderer, &menuRect);
	SDL_RenderCopy(renderer, textScore, NULL, &textScoreRect);
	SDL_RenderCopy(renderer, textLines, NULL, &textLinesRect);
	SDL_RenderCopy(renderer, textSpeed, NULL, &textSpeedRect);
	SDL_RenderCopy(renderer, textNext, NULL, &textNextRect);
	SDL_RenderCopy(renderer, numberScore, NULL, &numberScoreRect);
	SDL_RenderCopy(renderer, numberLines, NULL, &numberLinesRect);
	SDL_RenderCopy(renderer, numberSpeed, NULL, &numberSpeedRect);
	showImageMenu();
}

void show() {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);
	showMenu();
	if (started) {
		showStart();
	}
	else {
		showBoard();
	}
	SDL_RenderPresent(renderer);
}

void closeTextures() {
	SDL_DestroyTexture(textScore);
	SDL_DestroyTexture(textLines);
	SDL_DestroyTexture(textSpeed);
	SDL_DestroyTexture(textNext);
	SDL_DestroyTexture(numberScore);
	SDL_DestroyTexture(numberLines);
	SDL_DestroyTexture(numberSpeed);
	SDL_DestroyTexture(textStart);
	SDL_DestroyTexture(textIndication1);
	SDL_DestroyTexture(textIndication2);
	SDL_DestroyTexture(textIndication3);
	SDL_DestroyTexture(textIndication4);
}

void close() {
	closeTextures();
	Mix_FreeChunk(sound);
	Mix_FreeMusic(music);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_CloseFont(font);
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

int main(int argc, char* args[]) {
	if (!init())
		return -1;
	initVariables();
	loadSound();
	generateMenu();
	while (!gameOver) {
		if (started) {
			start();
			detectkeyExit();
		}
		else {
			detectKey();
			update();
		}
		show();
		SDL_Delay(MS);
	}
	close();
	return 0;
}

/*
Sweet Dreams by BatchBug | https://soundcloud.com/batchbug
Music promoted by https://www.chosic.com
Creative Commons Attribution 3.0 Unported License
https://creativecommons.org/licenses/by/3.0/deed.en_US
*/