#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define DINO_BOTTOM_Y 18  // 사람 위치를 장애물과 맞추기 위해 조정
#define TREE_BOTTOM_Y 20  // 장애물 위치
#define TREE_BOTTOM_X 45
#define JUMP_LIMIT 3      // 점프 높이 (1단 점프 기준)

//재시작 시 스코어 초기화 등 개선할점 있음

int score = 0;
clock_t lastScoreTime = 0;

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
        if (key == 32) {  // 스페이스바
            return key;
        }
        else if (key == 0 || key == 224) {  // 특수 키 (방향키 포함)
            key = _getch();
            if (key == 72) {  // 방향키 위
                return key;
            }
        }
    }
    return 0;
}

// 사람 모양 그리기
void DrawDino(int dinoY) {
    GotoXY(0, dinoY);
    static bool legFlag = true;
    printf("   O   \n");       // 머리
    printf("  /|\\ \n");       // 팔과 몸통
    if (legFlag) {
        printf(" /   \\\n");   // 움직이는 다리 1
        legFlag = false;
    }
    else {
        printf("  \\ /\n");   // 움직이는 다리 2
        legFlag = true;
    }
}

// 사람 위치 지우기
void ClearDino(int dinoY) {
    for (int i = 0; i < 3; i++) {
        GotoXY(0, dinoY + i);
        printf("       ");
    }
}

// 장애물 그리기
void DrawTree(int treeX, int height) {
    for (int i = 0; i < height; i++) {
        GotoXY(treeX, TREE_BOTTOM_Y - i);
        printf("$$$"); // 두께를 3으로 설정
    }
}

// 장애물 지우기 (두께 3에 맞춰 공백 출력)
void ClearTree(int treeX, int height) {
    for (int i = 0; i < height; i++) {
        GotoXY(treeX, TREE_BOTTOM_Y - i);
        printf("   "); // 두께에 맞춰 공백 출력
    }
}

// 충돌 감지 함수
bool CheckCollision(int dinoY, int treeX, int obstacleHeight) {
    // 사람의 위치와 장애물 위치가 겹치는지 확인
    return (treeX <= 2 && treeX >= 0 && dinoY >= TREE_BOTTOM_Y - obstacleHeight);
}

void UpdateScore() {
    clock_t currentTime = clock();

    // 0.5초 지났을 경우 점수 증가
    if ((double)(currentTime - lastScoreTime) / CLOCKS_PER_SEC >= 0.5) {
        score += 1;
        lastScoreTime = currentTime; // 마지막 점수 업데이트 시간 갱신

        // 점수 출력
        GotoXY(0, 22);  // 원하는 위치로 이동하여 점수 출력
        printf("Score: %d", score);
    }
}

int main(void) {
    SetConsoleView();
    srand(time(NULL)); // 난수 시드 설정 (프로그램 시작 시 한 번만 호출)

    bool isJumping = false;
    bool isBottom = true;
    const int gravity = 2; // 중력을 낮춰 점프 속도를 줄임
    int jumpCount = 0;     // 점프 높이 제한을 위한 카운트
    bool isDoubleJump = false; // 2단 점프 가능 여부
    int obstacleHeight = 4 + (rand() % 4) * 2; // 4, 6, 8, 또는 10의 높이로 랜덤 설정

    int dinoY = DINO_BOTTOM_Y;
    int treeX = TREE_BOTTOM_X;

    while (true) {
        UpdateScore();
        int key = GetKeyDown();
        if ((key == 32 || key == 72) && (isBottom || isDoubleJump)) {
            if (isBottom) { // 바닥에서 첫 점프
                isJumping = true;
                isBottom = false;
                isDoubleJump = true; // 2단 점프 가능
                jumpCount = 0;
            }
            else if (isDoubleJump) { // 공중에서 2단 점프
                isJumping = true;
                isDoubleJump = false; // 더 이상 점프 불가
                jumpCount = 0;
            }
        }

        // 점프 중이면 Y 감소 (위로 이동)
        if (isJumping && jumpCount < JUMP_LIMIT) {
            ClearDino(dinoY); // 현재 위치 지우기
            dinoY -= gravity; // 위로 이동
            jumpCount++;      // 점프 높이 증가
        }
        else {
            isJumping = false;
            ClearDino(dinoY); // 현재 위치 지우기
            dinoY += gravity; // 아래로 이동
        }

        // 바닥에 도달했을 때 위치 고정 및 상태 초기화
        if (dinoY >= DINO_BOTTOM_Y) {
            dinoY = DINO_BOTTOM_Y;
            isBottom = true;
            isDoubleJump = false; // 바닥에 닿으면 다시 2단 점프 가능
        }

        // 장애물 이동
        ClearTree(treeX, obstacleHeight); // 장애물 현재 위치 지우기
        treeX -= 2;
        if (treeX <= 0) {
            treeX = TREE_BOTTOM_X;
            obstacleHeight = 3 + (rand() % 4) * 2; // 4, 6, 8, 또는 10의 높이로 랜덤 설정
        }

        // 사람과 장애물 그리기
        DrawDino(dinoY);
        DrawTree(treeX, obstacleHeight);

        // 충돌 감지
        if (CheckCollision(dinoY, treeX, obstacleHeight)) {
            GotoXY(0, 23);
            printf("Game Over!\n"); // 게임 오버 메시지
            printf("게임을 종료하려면 n을 누르세요");
            char retry = _getch();
            if (retry == 'n') {
                break;
            }
            
        }

        Sleep(80);
    }

    system("pause");
    return 0;
}