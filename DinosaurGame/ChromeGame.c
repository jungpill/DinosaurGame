#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define DINO_BOTTOM_Y 18
#define TREE_BOTTOM_Y 20
#define TREE_BOTTOM_X 45
#define JUMP_LIMIT 3
#define MAX_RANKINGS 5

typedef struct {
    char initials[4];
    int score;
    char date[20];
} Ranking;

Ranking rankings[MAX_RANKINGS] = { 0 };
int score = 0;
clock_t lastScoreTime = 0;

void GetCurrentDate(char* buffer, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buffer, size, "%04d-%02d-%02d %02d:%02d:%02d",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void SaveRanking(const char* initials, int score) {
    char currentDate[20];
    GetCurrentDate(currentDate, sizeof(currentDate));

    for (int i = 0; i < MAX_RANKINGS; i++) {
        if (rankings[i].score == 0) {
            snprintf(rankings[i].initials, sizeof(rankings[i].initials), "%s", initials);
            rankings[i].score = score;
            snprintf(rankings[i].date, sizeof(rankings[i].date), "%s", currentDate);
            break;
        }
    }

    for (int i = 0; i < MAX_RANKINGS - 1; i++) {
        for (int j = i + 1; j < MAX_RANKINGS; j++) {
            if (rankings[j].score > rankings[i].score) {
                Ranking temp = rankings[i];
                rankings[i] = rankings[j];
                rankings[j] = temp;
            }
        }
    }
}

void DisplayRankings() {
    system("cls");
    printf("\n===== Rankings =====\n");
    for (int i = 0; i < MAX_RANKINGS; i++) {
        if (rankings[i].score > 0) {
            printf("%d. %s - %d점 (%s)\n", i + 1, rankings[i].initials, rankings[i].score, rankings[i].date);
        }
    }
    printf("\n게임을 다시 시작하려면 r, 종료하려면 n을 누르세요: ");
}

void SetConsoleView() {
    system("mode con:cols=100 lines=25");
    system("title Google Dinosaurs. By BlockDMask");
}

void GotoXY(int x, int y) {
    COORD Pos;
    Pos.X = 2 * x;
    Pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int GetKeyDown() {
    if (_kbhit() != 0) {
        int key = _getch();
        if (key == 32) return key;
        else if (key == 0 || key == 224) {
            key = _getch();
            if (key == 72) return key;
        }
    }
    return 0;
}

void DrawDino(int dinoY) {
    GotoXY(0, dinoY);
    static bool legFlag = true;
    printf("   O   \n");
    printf("  /|\\ \n");
    printf(legFlag ? " /   \\\n" : "  \\ /\n");
    legFlag = !legFlag;
}

void ClearDino(int dinoY) {
    for (int i = 0; i < 3; i++) {
        GotoXY(0, dinoY + i);
        printf("       ");
    }
}

void DrawTree(int treeX, int height) {
    for (int i = 0; i < height; i++) {
        GotoXY(treeX, TREE_BOTTOM_Y - i);
        printf("$$$");
    }
}

void ClearTree(int treeX, int height) {
    for (int i = 0; i < height; i++) {
        GotoXY(treeX, TREE_BOTTOM_Y - i);
        printf("   ");
    }
}

bool CheckCollision(int dinoY, int treeX, int obstacleHeight) {
    return (treeX <= 2 && treeX >= 0 && dinoY >= TREE_BOTTOM_Y - obstacleHeight);
}

void ResetGame(int* dinoY, int* treeX, int* obstacleHeight) {
    score = 0; // 점수 초기화
    lastScoreTime = clock(); // 점수 계산 시간 초기화
    *dinoY = DINO_BOTTOM_Y; // 공룡 위치 초기화
    *treeX = TREE_BOTTOM_X; // 장애물 위치 초기화
    *obstacleHeight = 4 + (rand() % 4) * 2; // 장애물 높이 랜덤 설정
}

void MoveCursorTo(int x, int y) {
    COORD Pos;
    Pos.X = 2 * x;
    Pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

void UpdateScore() {
    clock_t currentTime = clock();

    // 0.5초 지났을 경우 점수 증가
    if ((double)(currentTime - lastScoreTime) / CLOCKS_PER_SEC >= 0.5) {
        score += 1;
        lastScoreTime = currentTime; // 마지막 점수 업데이트 시간 갱신
    }

    // 점수 출력
    MoveCursorTo(0, 22); // 출력 위치
    printf("Score: %d", score);
}


int main(void) {
    SetConsoleView();
    srand(time(NULL));

    bool isJumping = false;
    bool isBottom = true;
    const int gravity = 2;
    int jumpCount = 0;
    bool isDoubleJump = false;
    int obstacleHeight = 4 + (rand() % 4) * 2;

    int dinoY = DINO_BOTTOM_Y;
    int treeX = TREE_BOTTOM_X;

    while (true) {
        UpdateScore();
        int key = GetKeyDown();
        if ((key == 32 || key == 72) && (isBottom || isDoubleJump)) {
            if (isBottom) {
                isJumping = true;
                isBottom = false;
                isDoubleJump = true;
                jumpCount = 0;
            }
            else if (isDoubleJump) {
                isJumping = true;
                isDoubleJump = false;
                jumpCount = 0;
            }
        }

        if (isJumping && jumpCount < JUMP_LIMIT) {
            ClearDino(dinoY);
            dinoY -= gravity;
            jumpCount++;
        }
        else {
            isJumping = false;
            ClearDino(dinoY);
            dinoY += gravity;
        }

        if (dinoY >= DINO_BOTTOM_Y) {
            dinoY = DINO_BOTTOM_Y;
            isBottom = true;
            isDoubleJump = false;
        }

        ClearTree(treeX, obstacleHeight);
        treeX -= 2;
        if (treeX <= 0) {
            treeX = TREE_BOTTOM_X;
            obstacleHeight = 4 + (rand() % 4) * 2;
        }

        DrawDino(dinoY);
        DrawTree(treeX, obstacleHeight);

        if (CheckCollision(dinoY, treeX, obstacleHeight)) {
            MoveCursorTo(0, 23);
            printf("Game Over!\n");
            printf("이름 이니셜을 입력하세요 (최대 3글자): ");
            char initials[4];
            scanf("%3s", initials);
            SaveRanking(initials, score);

            DisplayRankings();

            char retry = _getch();
            if (retry == 'n') break;
            else if (retry == 'r') {
                ResetGame(&dinoY, &treeX, &obstacleHeight);
                system("cls");
            }
        }

        Sleep(80);
    }

    return 0;
}


