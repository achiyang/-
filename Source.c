#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>

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
	system("title Mine Sweeper");
}

void CursorView(BOOL visible) {
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize = 1;
	cci.bVisible = visible;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cci);
}

void drawTitle() {
	GotoXY(0, 3);
	printf("           @@   @@  @@@  @   @  @@@@@   @@@@   @     @  @@@@@  @@@@@  @@@@   @@@@@  @@@@            ");
	GotoXY(0, 4);
	printf("           @ @ @ @   @   @@  @  @      @       @     @  @      @      @   @  @      @   @           ");
	GotoXY(0, 5);
	printf("           @  @  @   @   @ @ @  @@@@@   @@@@   @  @  @  @@@@@  @@@@@  @@@@   @@@@@  @@@@            ");
	GotoXY(0, 6);
	printf("           @     @   @   @  @@  @           @  @ @ @ @  @      @      @      @      @  @            ");
	GotoXY(0, 7);
	printf("           @     @  @@@  @   @  @@@@@   @@@@    @   @   @@@@@  @@@@@  @      @@@@@  @   @           ");
}

enum menu {
	START_GAME,
	END_GAME
};

enum menu mainMenu() {
	CursorView(FALSE);
	SetConsoleWindow(100, 30);
	system("cls");

	drawTitle();

	enum menu select = START_GAME;

	GotoXY(47, 17);
	printf("Start Game");
	GotoXY(47, 20);
	printf("End Game");
	GotoXY(43, 17);
	printf("☞");

	while (1) {
		int key = _getch();
		if (isupper(key)) key = tolower(key);
		if (key == 224) key = _getch();

		if (key == '\n' || key == ' ' || key == 13) {
			return select;
		}
		else if (key == 27 || key == 3) {
			return END_GAME;
		}
		else if (key == 'w' || key == 72 || key == 's' || key == 80) {
			select = 1 - select;
			GotoXY(43, 20 - select * 3);
			printf("  ");
			GotoXY(43, 17 + select * 3);
			printf("☞");
		}
	}
}

struct Difficulty {
	COORD boardSize;
	int numberMines;
};

struct Difficulty difficultyList[3] = {
	{{9, 9}, 10},
	{{16, 16}, 40},
	{{30, 16}, 99}
};

struct Difficulty customDifficulty() {
	CursorView(TRUE);

	struct Difficulty difficulty;

	while (1) {
		system("cls");
		drawTitle();
		GotoXY(41, 13);
		printf("Custom Difficulty");
		GotoXY(39, 16);
		printf("(board size >= 9 x 9)");
		GotoXY(38, 20);
		printf("Board cols:");
		GotoXY(37, 22);
		printf("Board lines:");
		GotoXY(36, 24);
		printf("Number mines:");

		GotoXY(50, 20);
		scanf("%hd", &difficulty.boardSize.X);
		GotoXY(50, 22);
		scanf("%hd", &difficulty.boardSize.Y);
		GotoXY(50, 24);
		scanf("%d", &difficulty.numberMines);

		if (difficulty.boardSize.X >= 10 &&
			difficulty.boardSize.Y >= 10 &&
			difficulty.numberMines < difficulty.boardSize.X * difficulty.boardSize.Y) break;
	}

	return difficulty;
}

enum {
	EASY,
	NORMAL,
	HARD,
	CUSTOM
};

int difficulty_cursor_X[4] = { 40, 38, 39, 44 };

struct Difficulty difficultySetting() {
	system("cls");
	drawTitle();

	int difficulty = EASY;

	GotoXY(45, 14);
	printf("Difficulty");
	GotoXY(43, 18);
	printf("Easy(9 x 9, 10)");
	GotoXY(41, 20);
	printf("Normal(16 x 16, 40)");
	GotoXY(42, 22);
	printf("Hard(30 x 16, 99)");
	GotoXY(47, 24);
	printf("Custom");
	GotoXY(40, 18);
	printf("☞");

	while (1) {
		int key = _getch();
		if (isupper(key)) key = tolower(key);
		if (key == 224) key = _getch();

		if (key == '\n' || key == ' ' || key == 13) {
			if (difficulty == CUSTOM) {
				return customDifficulty();
			}
			else {
				return difficultyList[difficulty];
			}
		}
		else if (key == 'w' || key == 72) {
			GotoXY(difficulty_cursor_X[difficulty], 18 + difficulty * 2);
			printf("  ");
			if (--difficulty < 0) difficulty += 4;
			GotoXY(difficulty_cursor_X[difficulty], 18 + difficulty * 2);
			printf("☞");
		}
		else if (key == 's' || key == 80) {
			GotoXY(difficulty_cursor_X[difficulty], 18 + difficulty * 2);
			printf("  ");
			if (++difficulty > 3) difficulty -= 4;
			GotoXY(difficulty_cursor_X[difficulty], 18 + difficulty * 2);
			printf("☞");
		}
	}
}

enum State {
	CLOSED,
	FLAGED,
	OPENED
};

typedef struct Cell {
	BOOL isMine;
	enum State state;
	int mineCount;
}Cell;

void setBoard(int numMine, Cell** board, COORD boardSize) {
	srand((unsigned int)time(NULL));
	int pos;
	int max = boardSize.X * boardSize.Y;
	for (int i = 0; i < numMine; i++) {
		do {
			pos = rand() % max;
		} while (board[pos / boardSize.X][pos % boardSize.X].isMine);
		board[pos / boardSize.Y][pos % boardSize.Y].isMine = TRUE;
	}

	for (int i = 0; i < boardSize.X; i++) {
		for (int j = 0; j < boardSize.Y; j++) {
			if (board[i][j].isMine == FALSE) {
				int count = 0;
				for (int k = -1; k <= 1; k++) {
					for (int l = -1; l <= 1; l++) {
						if (i + k >= 0 && i + k < boardSize.X &&
							j + l >= 0 && j + l < boardSize.Y &&
							board[i + k][j + l].isMine) {
							++count;
						}
					}
				}
				board[i][j].mineCount = count;
			}
		}
	}
}

int openGrid(Cell** board, COORD openPos, COORD boardSize) {
	if (board[openPos.X][openPos.Y].isMine) {
		return 0;
	}
	else if (board[openPos.X][openPos.Y].state == CLOSED) {
		board[openPos.X][openPos.Y].state = OPENED;
		if (board[openPos.X][openPos.Y].mineCount == 0) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (openPos.X + i >= 0 && openPos.X + i < boardSize.X &&
						openPos.Y + j >= 0 && openPos.Y + j < boardSize.Y) {
						openGrid(board, (COORD) { openPos.X + i, openPos.Y + j }, boardSize);
					}
				}
			}
		}
	}

	return 1;
}

const char* icon[] = { "  ", "①", "②", "③", "④", "⑤", "⑥", "⑦", "⑧" };

int gameLoop(struct Difficulty difficulty) {
	system("cls");
	CursorView(TRUE);
	SetConsoleWindow(difficulty.boardSize.X * 2 + 20, difficulty.boardSize.Y + 2);

	for (int i = 1; i <= difficulty.boardSize.Y; i++) {
		GotoXY(0, i);
		for (int j = 0; j < difficulty.boardSize.X; j++) {
			printf("■");
		}
	}

	int leftOver = difficulty.numberMines;

	GotoXY(difficulty.boardSize.X * 2 + 4, difficulty.boardSize.Y / 2 - 3);
	printf("leftover: %d", leftOver);
	GotoXY(difficulty.boardSize.X * 2 + 5, difficulty.boardSize.Y / 2);
	printf("spase / e");
	GotoXY(difficulty.boardSize.X * 2 + 3, difficulty.boardSize.Y / 2 + 2);
	printf("quit : q / esc");

	COORD cursorPos = { 0, 0 };
	GotoXY(cursorPos.X * 2, cursorPos.Y + 1);

	Cell** board = malloc(sizeof(Cell*) * difficulty.boardSize.X);
	if (board == NULL) {
		fprintf(stderr, "메모리 할당 실패\n");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < difficulty.boardSize.X; i++) {
		board[i] = calloc(difficulty.boardSize.Y, sizeof(Cell));
		if (board[i] == NULL) {
			fprintf(stderr, "메모리 할당 실패\n");
			exit(EXIT_FAILURE);
		}
	}

	setBoard(difficulty.numberMines, board, difficulty.boardSize);

	while (1) {
		int count = 0;
		for (int i = 0; i < difficulty.boardSize.X; i++) {
			for (int j = 0; j < difficulty.boardSize.Y; j++) {
				if (board[i][j].state != OPENED) {
					++count;
				}
			}
		}
		if (count == difficulty.numberMines) {
			CursorView(FALSE);
			GotoXY(difficulty.boardSize.X * 2 + 8, difficulty.boardSize.Y / 2 - 2);
			printf("Win!");
			break;
		}

		int key = _getch();
		if (isupper(key)) key = islower(key);
		if (key == 224) key = _getch();

		COORD newPos = cursorPos;
		if (key == ' ' && board[cursorPos.X][cursorPos.Y].state != FLAGED) {
			if (board[cursorPos.X][cursorPos.Y].state == CLOSED) {
				if (openGrid(board, cursorPos, difficulty.boardSize) == 0) {
					CursorView(FALSE);
					GotoXY(cursorPos.X * 2, cursorPos.Y + 1);
					printf("♨");
					GotoXY(difficulty.boardSize.X * 2 + 8, difficulty.boardSize.Y / 2 - 2);
					printf("BOOM!");
					break;
				}
				CursorView(FALSE);
				for (int i = 0; i < difficulty.boardSize.X; i++) {
					for (int j = 0; j < difficulty.boardSize.Y; j++) {
						if (board[i][j].state == OPENED) {
							GotoXY(i * 2, j + 1);
							printf("%s", icon[board[i][j].mineCount]);
						}
					}
				}
				CursorView(TRUE);
			}
			else if (board[cursorPos.X][cursorPos.Y].state == OPENED) {
				int flagCount = 0;
				for (int i = -1; i <= 1; i++) {
					for (int j = -1; j <= 1; j++) {
						if (cursorPos.X + i >= 0 && cursorPos.X + i < difficulty.boardSize.X &&
							cursorPos.Y + j >= 0 && cursorPos.Y + j < difficulty.boardSize.Y &&
							board[cursorPos.X + i][cursorPos.Y + j].state == FLAGED) {
							++flagCount;
						}
					}
				}
				if (flagCount == board[cursorPos.X][cursorPos.Y].mineCount) {
					int isGameOver = 0;
					for (int i = -1; i <= 1; i++) {
						for (int j = -1; j <= 1; j++) {
							if (cursorPos.X + i >= 0 && cursorPos.X + i < difficulty.boardSize.X &&
								cursorPos.Y + j >= 0 && cursorPos.Y + j < difficulty.boardSize.Y &&
								board[cursorPos.X + i][cursorPos.Y + j].state == CLOSED) {
								if (openGrid(board, (COORD) { cursorPos.X + i, cursorPos.Y + j }, difficulty.boardSize) == 0) {
									CursorView(FALSE);
									GotoXY((cursorPos.X + i) * 2, cursorPos.Y + j + 1);
									printf("♨");
									GotoXY(difficulty.boardSize.X * 2 + 8, difficulty.boardSize.Y / 2 - 2);
									printf("BOOM!");
									isGameOver = 1;
									break;
								}
								CursorView(FALSE);
								for (int i = 0; i < difficulty.boardSize.X; i++) {
									for (int j = 0; j < difficulty.boardSize.Y; j++) {
										if (board[i][j].state == OPENED) {
											GotoXY(i * 2, j + 1);
											printf("%s", icon[board[i][j].mineCount]);
										}
									}
								}
								CursorView(TRUE);
							}
						}
						if (isGameOver) break;
					}
					if (isGameOver) break;
				}
			}
		}
		else if (key == 'e' && board[cursorPos.X][cursorPos.Y].state != OPENED) {
			CursorView(FALSE);
			if (board[cursorPos.X][cursorPos.Y].state == CLOSED) {
				board[cursorPos.X][cursorPos.Y].state = FLAGED;
				printf("▣");
				--leftOver;
			}
			else if (board[cursorPos.X][cursorPos.Y].state == FLAGED) {
				board[cursorPos.X][cursorPos.Y].state = CLOSED;
				printf("■");
				++leftOver;
			}
			GotoXY(difficulty.boardSize.X * 2 + 4, difficulty.boardSize.Y / 2 - 3);
			printf("leftover: %d ", leftOver);
			CursorView(TRUE);
		}
		else if (key == 'q' || key == 27 || key == 3) {
			break;
		}
		else if (key == 'w' || key == 72) {
			--newPos.Y;
		}
		else if (key == 'a' || key == 75) {
			--newPos.X;
		}
		else if (key == 's' || key == 80) {
			++newPos.Y;
		}
		else if (key == 'd' || key == 77) {
			++newPos.X;
		}

		if (newPos.X >= 0 && newPos.X < difficulty.boardSize.X &&
			newPos.Y >= 0 && newPos.Y < difficulty.boardSize.Y) {
			cursorPos = newPos;
		}
		GotoXY(cursorPos.X * 2, cursorPos.Y + 1);
	}

	for (int i = 0; i < difficulty.boardSize.X; i++) {
		free(board[i]);
	}
	free(board);

	CursorView(FALSE);
	GotoXY(difficulty.boardSize.X * 2 + 5, difficulty.boardSize.Y / 2 + 4);
	printf("try again?");
	GotoXY(difficulty.boardSize.X * 2 + 7, difficulty.boardSize.Y / 2 + 5);
	printf("Y / N");
	while (1) {
		int key = _getch();
		if (isupper(key)) key = tolower(key);

		if (key == 'y' || key == ' ' || key == 13) {
			return 1;
		}
		else if (key == 'n' || key == 27 || key == 3 || key == 'q') {
			return 0;
		}
	}
}

int main() {
	while (1) {
		enum menu select = mainMenu();

		if (select == START_GAME) {
			struct Difficulty difficulty = difficultySetting();
			while (gameLoop(difficulty));
		}
		else if (select == END_GAME) {
			break;
		}
	}

	return 0;
}