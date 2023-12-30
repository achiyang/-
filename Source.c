#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>

enum {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	FLAG,
	ENTER,
	ESCAPE
};

char defaultControls[7] = {'w', 's', 'a', 'd', 'e', ' ', 'q'};
char currentControls[7] = {'w', 's', 'a', 'd', 'e', ' ', 'q'};

BOOL isCharInArray(char target, char array[], int size) {
	for (int i = 0; i < size; i++) {
		if (array[i] == target) {
			return TRUE;
		}
	}
	return FALSE;
}

BOOL isControls(int C) {
	return isCharInArray(C, currentControls, sizeof(currentControls));
}

void GotoXY(int x, int y) {
	COORD pos;
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SetConsoleWindow(int cols, int lines) {
	char command[40] = { 0 };
	sprintf(command, "mode con:cols=%d lines=%d", cols, lines);
	system(command);
}

void CursorView(BOOL visible) {
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize = 1;
	cci.bVisible = visible;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
}

void textColor(int colorNum) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

int getkey() {
	int key = _getch();
	if (isupper(key)) {
		key = tolower(key);
	}
	else if (key == 224 || key == 0) {
		key = _getch();
	}
	return key;
}

int getcontrols() {
	int key;
	do {
		key = getkey();
	} while (key != 27 && !isControls(key));
	return key;
}

void drawTitle() {
	textColor(14);
	GotoXY(0, 4);
	printf("           @@   @@  @@@  @   @  @@@@@   @@@@   @     @  @@@@@  @@@@@  @@@@   @@@@@  @@@@            ");
	GotoXY(0, 5);
	printf("           @ @ @ @   @   @@  @  @      @       @     @  @      @      @   @  @      @   @           ");
	GotoXY(0, 6);
	printf("           @  @  @   @   @ @ @  @@@@@   @@@@   @  @  @  @@@@@  @@@@@  @@@@   @@@@@  @@@@            ");
	GotoXY(0, 7);
	printf("           @     @   @   @  @@  @           @  @ @ @ @  @      @      @      @      @  @            ");
	GotoXY(0, 8);
	printf("           @     @  @@@  @   @  @@@@@   @@@@    @   @   @@@@@  @@@@@  @      @@@@@  @   @           ");
}

typedef struct ConsoleBuffer {
	CHAR_INFO* buffer;
	COORD bufferSize;
} ConsoleBuffer;

ConsoleBuffer saveConsoleBuffer(COORD bufferSize) {
	CHAR_INFO* buffer = (CHAR_INFO*)malloc(sizeof(CHAR_INFO) * bufferSize.X * bufferSize.Y);
	if (buffer == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		exit(EXIT_FAILURE);
	}

	COORD bufferCoord = { 0, 0 };
	SMALL_RECT readRegion = { 0, 0, bufferSize.X - 1, bufferSize.Y - 1 };

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	ReadConsoleOutput(console, buffer, bufferSize, bufferCoord, &readRegion);

	ConsoleBuffer consoleBuffer = { buffer, bufferSize };
	return consoleBuffer;
}

void restoreConsoleBuffer(ConsoleBuffer consoleBuffer) {
	if (consoleBuffer.buffer == NULL) {
		return;
	}
	SetConsoleWindow(consoleBuffer.bufferSize.X, consoleBuffer.bufferSize.Y);

	COORD bufferCoord = { 0, 0 };
	SMALL_RECT writeRegion = { 0, 0, consoleBuffer.bufferSize.X - 1, consoleBuffer.bufferSize.Y - 1 };

	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	WriteConsoleOutput(console, consoleBuffer.buffer, consoleBuffer.bufferSize, bufferCoord, &writeRegion);
}

void freeConsoleBuffer(ConsoleBuffer* consoleBuffer) {
	if (consoleBuffer->buffer == NULL) {
		return;
	}
	free(consoleBuffer->buffer);
	consoleBuffer->buffer = NULL;
}

void printSpecial(char c) {
	if (isdigit(c)) {
		printf("%c%c", -93, -80 + c - '0');
	}
	else if (islower(c)) printf("%c%c", -93, -63 + c - 'a');
	else if (c == 80) printf("%c%c", -95, -23);
	else if (c == 72) printf("%c%c", -95, -24);
	else if (c == 75) printf("%c%c", -95, -25);
	else if (c == 77) printf("%c%c", -95, -26);
	else if (c == '!') printf("£¡");
	else if (c == '?') printf("£¿");
	else if (c == ',') printf("£¬");
	else if (c == '.') printf("£®");
	else if (c == ':') printf("£º");
	else if (c == ';') printf("£»");
	else if (c == '\'') printf("£§");
	else if (c == '\"') printf("£¢");
	else if (c == '(') printf("£¨");
	else if (c == ')') printf("£©");
	else if (c == '[') printf("£Û");
	else if (c == ']') printf("£Ý");
	else if (c == '{') printf("£û");
	else if (c == '}') printf("£ý");
	else if (c == '<') printf("¡´");
	else if (c == '>') printf("¡µ");
	else if (c == '+') printf("£«");
	else if (c == '-') printf("£­");
	else if (c == '=') printf("£½");
	else if (c == '$') printf("£¤");
	else if (c == '%') printf("£¥");
	else if (c == '#') printf("££");
	else if (c == '&') printf("£¦");
	else if (c == '*') printf("£ª");
	else if (c == '@') printf("£À");
	else if (c == '^') printf("£Þ");
	else if (c == '|') printf("£ü");
	else if (c >= 1 && c <= 26) printf("^%c", c + 'A' - 1);
	else if (c == 32) printf("¢§");
	else printf("%c ", c);
}

void drawSquareBox(COORD pos, COORD size, int plusSizeX, BOOL clearContents) {
	GotoXY(pos.X, pos.Y);
	printf("¦£¦¡");
	for (int i = 0; i < size.X - 2; i++) {
		printf("¦¡¦¡");
	}
	for (int i = 0; i < plusSizeX; i++) {
		printf("¦¡");
	}
	printf("¦¤ ");
	for (int i = 1; i <= size.Y - 2; i++) {
		GotoXY(pos.X, pos.Y + i);
		printf("¦¢ ");
		if (clearContents) {
			for (int j = 0; j < size.X - 2; j++) {
				printf("  ");
			}
			for (int j = 0; j < plusSizeX; j++) {
				printf(" ");
			}
		}
		GotoXY(pos.X + (size.X - 1) * 2 + plusSizeX, pos.Y + i);
		printf("¦¢ ");
	}
	GotoXY(pos.X, pos.Y + size.Y - 1);
	printf("¦¦¦¡");
	for (int i = 0; i < size.X - 2; i++) {
		printf("¦¡¦¡");
	}
	for (int i = 0; i < plusSizeX; i++) {
		printf("¦¡");
	}
	printf("¦¥ ");
}

int quitKeySettings() {
	textColor(15);
	COORD boxPos = { 29, 12 };
	COORD boxSize = { 21, 11 };
	drawSquareBox(boxPos, boxSize, 0, TRUE);
	GotoXY(42, 15);
	printf("Quit with saving?");

	int selection = 0;
	while (1) {
		textColor(15);
		GotoXY(41, 19);
		printf("Yes    No    Cancle");
		textColor(11);
		switch (selection) {
		case 0:
			GotoXY(41, 19);
			printf("Yes");
			break;

		case 1:
			GotoXY(48, 19);
			printf("No");
			break;

		case 2:
			GotoXY(54, 19);
			printf("Cancle");
			break;
		}

		int key = getkey();
		if (key == 'y') {
			if (selection == 0) {
				return 0;
			}
			else {
				selection = 0;
			}
		}
		else if (key == 'n') {
			if (selection == 1) {
				return 1;
			}
			else {
				selection = 1;
			}
		}
		else if (key == 'c') {
			if (selection == 2) {
				return 2;
			}
			else {
				selection = 2;
			}
		}
		else if (key == currentControls[ENTER]) {
			break;
		}
		else if (key == currentControls[ESCAPE] || key == 27) {
			if (selection == 2) {
				break;
			}
			else {
				selection = 2;
			}
		}
		else if (key == currentControls[LEFT] && selection > 0) {
			selection--;
		}
		else if (key == currentControls[RIGHT] && selection < 2) {
			selection++;
		}
	}
	return selection;
}

enum {
	RESET = 7,
	QUIT = 8
};

void keySettings() {
	GotoXY(0, 9);
	for (int i = 0; i < 100 * 21; i++) {
		_putch(' ');
	}

	char tempControls[7];
	memcpy(tempControls, currentControls, sizeof(currentControls));
	COORD squareSize = { 3, 3 };

	COORD posArray[9] = {
		{ 25, 16 },
		{ 25, 19 },
		{ 19, 19 },
		{ 31, 19 },
		{ 53, 18 },
		{ 64, 18 },
		{ 75, 18 },
		{ 44, 26 },
		{ 52, 26 }
	};

	textColor(15);
	GotoXY(posArray[UP].X - 1, posArray[UP].Y - 2);
	printf("CONTROLS");

	GotoXY(posArray[FLAG].X + 1, posArray[FLAG].Y - 2);
	printf("FLAG");

	GotoXY(posArray[ENTER].X - 2, posArray[ENTER].Y - 2);
	printf("ENTER/OPEN");

	GotoXY(posArray[ESCAPE].X, posArray[ESCAPE].Y - 2);
	printf("ESCAPE");

	GotoXY(posArray[RESET].X, posArray[RESET].Y);
	printf("RESET");

	GotoXY(posArray[QUIT].X, posArray[QUIT].Y);
	printf("QUIT");

	for (int i = 0; i < 7; i++) {
		GotoXY(posArray[i].X + 2, posArray[i].Y + 1);
		printSpecial(tempControls[i]);
	}

	textColor(7);
	for (int i = 1; i < 7; i++) {
		drawSquareBox(posArray[i], squareSize, 0, FALSE);
	}

	GotoXY((posArray[RESET].X + 4 + posArray[QUIT].X) / 2, posArray[RESET].Y);
	printf("/");

	textColor(11);
	drawSquareBox(posArray[UP], squareSize, 0, FALSE);

	int current = UP;

	while (1) {
		int temp = current;
		int key = getcontrols();

		if (key == currentControls[ENTER] || key == currentControls[ESCAPE] || key == 27) {
			if (current == QUIT) {
				COORD consoleSize = { 100, 30 };
				ConsoleBuffer tempBuffer = saveConsoleBuffer(consoleSize);

				int selection = quitKeySettings();
				if (selection == 2) {
					restoreConsoleBuffer(tempBuffer);
					freeConsoleBuffer(&tempBuffer);
				}
				else {
					freeConsoleBuffer(&tempBuffer);
					if (selection == 0) {
						memcpy(currentControls, tempControls, sizeof(tempControls));
					}
					break;
				}
			}
			else {
				if (key == currentControls[ENTER]) {
					if (current == RESET) {
						memcpy(tempControls, defaultControls, sizeof(defaultControls));
						textColor(15);
						for (int i = 0; i < 7; i++) {
							GotoXY(posArray[i].X + 2, posArray[i].Y + 1);
							printSpecial(tempControls[i]);
						}
					}
					else {
						textColor(4);
						drawSquareBox(posArray[current], squareSize, 0, FALSE);

						int key;
						do {
							key = getkey();
						} while (key != 27 && key != tempControls[current] && isCharInArray(key, tempControls, sizeof(tempControls)));
						if (key != 27) {
							tempControls[current] = key;
						}

						textColor(15);
						GotoXY(posArray[current].X + 2, posArray[current].Y + 1);
						printSpecial(tempControls[current]);

						textColor(11);
						drawSquareBox(posArray[current], squareSize, 0, FALSE);
					}
				}
				else {
					current = QUIT;
				}
			}
		}
		else if (key == currentControls[UP]) {
			if (current == DOWN || current == LEFT || current == RIGHT) {
				current = UP;
			}
			else if (current == RESET) {
				current = RIGHT;
			}
			else if (current == QUIT) {
				current = FLAG;
			}
		}
		else if (key == currentControls[DOWN]) {
			if (current == UP) {
				current = DOWN;
			}
			else if (current == DOWN || current == LEFT || current == RIGHT) {
				current = RESET;
			}
			else if (current == FLAG || current == ENTER || current == ESCAPE) {
				current = QUIT;
			}
		}
		else if (key == currentControls[LEFT]) {
			if (current == FLAG || current == ENTER ||
				current == ESCAPE || current == QUIT) {
				current--;
			}
			else if (current == UP || current == DOWN) {
				current = LEFT;
			}
			else if (current == RIGHT) {
				current = DOWN;
			}
		}
		else if (key == currentControls[RIGHT]) {
			if (current == RIGHT || current == FLAG ||
				current == ENTER || current == RESET) {
				current++;
			}
			else if (current == UP || current == DOWN) {
				current = RIGHT;
			}
			else if (current == LEFT) {
				current = DOWN;
			}
		}

		if (current != temp) {
			textColor(11);
			if (current < 7) {
				drawSquareBox(posArray[current], squareSize, 0, FALSE);
			}
			else {
				GotoXY(posArray[current].X, posArray[current].Y);
				printf("%s", current == RESET ? "RESET" : "QUIT");
			}

			textColor(15);
			if (temp < 7) {
				textColor(7);
				drawSquareBox(posArray[temp], squareSize, 0, FALSE);
			}
			else {
				GotoXY(posArray[temp].X, posArray[temp].Y);
				printf("%s", temp == RESET ? "RESET" : "QUIT");
			}
			GotoXY(0, 0);
		}
	}
}

void optionsMenu() {
	GotoXY(0, 9);
	for (int i = 0; i < 100 * 21; i++) {
		_putch(' ');
	}

	drawSquareBox((COORD) { 41, 18 }, (COORD) { 8, 3 }, 1, FALSE);
	drawSquareBox((COORD) { 41, 24 }, (COORD) { 8, 3 }, 1, FALSE);

	getkey();

	keySettings();
}

typedef enum MenuSelection {
	PLAY,
	OPTIONS,
	EXIT
} MenuSelection;

void mainMenu() {
	COORD consoleSize = { 100, 30 };

	CursorView(FALSE);
	SetConsoleWindow(consoleSize.X, consoleSize.Y);
	system("cls");
	drawTitle();

	MenuSelection currentSelection = PLAY;

	while (1) {
		textColor(15);
		GotoXY(45, 19);
		printf("Play Game");
		drawSquareBox((COORD) { 42, 18 }, (COORD) { 7, 3 }, 1, FALSE);

		GotoXY(46, 22);
		printf("Options");
		drawSquareBox((COORD) { 42, 21 }, (COORD) { 7, 3 }, 1, FALSE);

		GotoXY(45, 25);
		printf("Exit Game");
		drawSquareBox((COORD) { 42, 24 }, (COORD) { 7, 3 }, 1, FALSE);

		GotoXY(0, 14);
		for (int i = 0; i < 100; i++) {
			_putch(' ');
		}

		switch (currentSelection) {
		case PLAY:
			textColor(8);
			GotoXY(37, 14);
			printf("select level & start game");
			textColor(11);
			GotoXY(45, 19);
			printf("Play Game");
			drawSquareBox((COORD) { 42, 18 }, (COORD) { 7, 3 }, 1, FALSE);
			break;

		case OPTIONS:
			textColor(8);
			GotoXY(42, 14);
			printf("change settings");
			textColor(11);
			GotoXY(46, 22);
			printf("Options");
			drawSquareBox((COORD) { 42, 21 }, (COORD) { 7, 3 }, 1, FALSE);
			break;

		case EXIT:
			textColor(8);
			GotoXY(42, 14);
			printf("exit to desktop");
			textColor(11);
			GotoXY(45, 25);
			printf("Exit Game");
			drawSquareBox((COORD) { 42, 24 }, (COORD) { 7, 3 }, 1, FALSE);
			break;
		}

		int key = getcontrols();

		if (key == currentControls[ENTER]) {
			if (currentSelection == PLAY) {

			}
			else if (currentSelection == OPTIONS) {
				ConsoleBuffer tempBuffer = saveConsoleBuffer(consoleSize);
				optionsMenu();
				restoreConsoleBuffer(tempBuffer);
				freeConsoleBuffer(&tempBuffer);
			}
			else if (currentSelection == EXIT) {
				break;
			}
		}
		else if (key == currentControls[ESCAPE] || key == 27) {
			if (currentSelection == EXIT) {
				break;
			}
			else {
				currentSelection = EXIT;
			}
		}
		else if (key == currentControls[UP]) {
			currentSelection = (currentSelection + 2) % 3;
		}
		else if (key == currentControls[DOWN]) {
			currentSelection = (currentSelection + 1) % 3;
		}
	}
}

int main() {
	system("title Minesweeper");

	mainMenu();

	return 0;
}