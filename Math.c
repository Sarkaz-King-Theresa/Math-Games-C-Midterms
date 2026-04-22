#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <math.h>

#define MAX_USERS 200
#define USERNAME_LEN 50
#define PASSWORD_LEN 30
#define MAX_GHOST_LOG 200

// Changed to .txt so Notepad recognizes it easily as a text file
#define FILE_USERS "users.txt"
#define FILE_SCORES "scores.dat"
#define FILE_GHOST "ghost.dat"

typedef struct {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
} User;

typedef struct {
    char username[USERNAME_LEN];
    int totalScore;
    int scores[8];
} ScoreRecord;

typedef struct {
    char username[USERNAME_LEN];
    int ghostCurse[8];
    int lastCorrectCount[8];
} GhostRecord;

// Ui Helpers
void printHeader(const char* title) {
    int len = strlen(title) + 10;
    printf("\n+"); for(int i=0; i<len; i++) printf("-"); printf("+\n");
    printf("|     %s\n", title);
    printf("+"); for(int i=0; i<len; i++) printf("-"); printf("+\n");
}

void clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pressEnter() {
    printf("\n[#] Press ENTER to continue...");
    while (getchar() != '\n');
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int getRandomRange(int min, int max) {
    if (min >= max) return min;
    return (rand() % (max - min + 1)) + min;
}

void cleanString(char str[]) {
    str[strcspn(str, "\n")] = '\0';
    size_t len = strlen(str);
    if (len == 0) return;
    int start = 0, end = (int)len - 1;
    while (isspace((unsigned char)str[start])) start++;
    while (end >= start && isspace((unsigned char)str[end])) end--;
    if (start > 0 || end < (int)len - 1) {
        memmove(str, str + start, (size_t)(end - start + 1));
        str[end - start + 1] = '\0';
    }
}

//Error screens
void showLoginError(int type) {
    clear();
    printHeader("!!! ACCESS DENIED !!!");
    if (type == 1) printf("\n [!] ERROR: USERNAME NOT FOUND\n");
    else printf("\n [!] ERROR: INVALID PASSWORD\n");
    printf("\n------------------------------------------\n");
    pressEnter();
}

void showRegistrationError(int type) {
    clear();
    printHeader("!!! REGISTRATION FAILED !!!");
    if (type == 1) printf("\n [!] ERROR: USERNAME ALREADY EXISTS\n     Please choose a different name.\n");
    else printf("\n [!] ERROR: INVALID USERNAME\n     Username cannot be empty.\n");
    printf("\n------------------------------------------\n");
    pressEnter();
}

// Data
int loadUsers(User users[]) {
    FILE *fp = fopen(FILE_USERS, "r");
    int count = 0;
    if (fp == NULL) return 0;


    while (count < MAX_USERS && fscanf(fp, "%49s %29s", users[count].username, users[count].password) == 2) {
        count++;
    }
    fclose(fp);
    return count;
}

void saveUser(User newUser) {
    FILE *fp = fopen(FILE_USERS, "a"); // "a" for append in text mode
    if (fp) {
        fprintf(fp, "%s %s\n", newUser.username, newUser.password);
        fclose(fp);
    }
}

int loadScores(ScoreRecord scores[]) {
    FILE *fp = fopen(FILE_SCORES, "rb");
    int count = 0; if (fp == NULL) return 0;
    while (fread(&scores[count], sizeof(ScoreRecord), 1, fp)) count++;
    fclose(fp); return count;
}

void saveScores(ScoreRecord scores[], int count) {
    FILE *fp = fopen(FILE_SCORES, "wb");
    if (fp) { fwrite(scores, sizeof(ScoreRecord), count, fp); fclose(fp); }
}

// Registration
int registerUser() {
    User users[MAX_USERS];
    User newUser = {0};
    int count = loadUsers(users);

    clear();
    printHeader("NEW ACCOUNT");
    printf(" Desired Username: ");
    fgets(newUser.username, USERNAME_LEN, stdin);
    cleanString(newUser.username);

    if (strlen(newUser.username) == 0) {
        showRegistrationError(2);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, newUser.username) == 0) {
            showRegistrationError(1);
            return 0;
        }
    }

    printf(" Create Password: ");
    scanf("%29s", newUser.password);
    clearInputBuffer();

    saveUser(newUser);
    printf("\n[+] Account created successfully!\n");
    pressEnter();
    return 1;
}

// Login system
int loginUser(char loggedUser[]) {
    User users[MAX_USERS];
    int count = loadUsers(users);
    char u[USERNAME_LEN], p[PASSWORD_LEN];
    int userExists = 0;

    clear();
    printHeader("LOGIN");
    printf(" Username: ");
    fgets(u, USERNAME_LEN, stdin);
    cleanString(u);
    printf(" Password: ");
    scanf("%29s", p);
    clearInputBuffer();

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, u) == 0) {
            userExists = 1;
            if (strcmp(users[i].password, p) == 0) {
                strcpy(loggedUser, u);
                return 1;
            } else {
                showLoginError(2);
                return 0;
            }
        }
    }
    if (!userExists) showLoginError(1);
    return 0;
}

// Score and ghost mechanic
int findScoreIndex(ScoreRecord scores[], int count, char username[]) {
    for (int i = 0; i < count; i++) if (strcmp(scores[i].username, username) == 0) return i;
    return -1;
}

void bubbleSort(ScoreRecord scores[], int count, int op) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int v1 = (op == 0) ? scores[j].totalScore : scores[j].scores[op];
            int v2 = (op == 0) ? scores[j+1].totalScore : scores[j+1].scores[op];
            if (v1 < v2) { ScoreRecord t = scores[j]; scores[j] = scores[j+1]; scores[j+1] = t; }
        }
    }
}

void updateScore(ScoreRecord scores[], int *count, char username[], int op, int gameScore) {
    int idx = findScoreIndex(scores, *count, username);
    if (idx == -1) {
        idx = (*count)++;
        memset(&scores[idx], 0, sizeof(ScoreRecord));
        strcpy(scores[idx].username, username);
    }
    scores[idx].totalScore += gameScore;
    if (gameScore > scores[idx].scores[op]) scores[idx].scores[op] = gameScore;
}

void showTop10(ScoreRecord scores[], int count, int op) {
    bubbleSort(scores, count, op);
    const char* titles[] = {"OVERALL", "ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", "EXPONENTS", "ALGEBRA", "ULTRA HARD"};
    printf("\n--- %s TOP 10 ---\n", titles[op]);
    printf("%-5s | %-15s | %-10s\n", "RANK", "PLAYER", "SCORE");
    printf("----------------------------------\n");
    int limit = count < 10 ? count : 10;
    for (int i = 0; i < limit; i++) {
        int s = (op == 0) ? scores[i].totalScore : scores[i].scores[op];
        printf("#%-4d | %-15s | %-10d\n", i + 1, scores[i].username, s);
    }
}

int loadGhosts(GhostRecord ghosts[]) {
    FILE *fp = fopen(FILE_GHOST, "rb");
    int count = 0; if (fp == NULL) return 0;
    while (fread(&ghosts[count], sizeof(GhostRecord), 1, fp)) count++;
    fclose(fp); return count;
}

void saveGhosts(GhostRecord ghosts[], int count) {
    FILE *fp = fopen(FILE_GHOST, "wb");
    if (fp) { fwrite(ghosts, sizeof(GhostRecord), count, fp); fclose(fp); }
}

GhostRecord getGhostData(char username[]) {
    GhostRecord ghosts[MAX_GHOST_LOG]; int count = loadGhosts(ghosts);
    for (int i = 0; i < count; i++) if (strcmp(ghosts[i].username, username) == 0) return ghosts[i];
    GhostRecord g = {0}; strcpy(g.username, username); return g;
}

void updateGhostData(char username[], int correctCount, int operation) {
    GhostRecord ghosts[MAX_GHOST_LOG]; int count = loadGhosts(ghosts);
    int idx = -1;
    for (int i = 0; i < count; i++) if (strcmp(ghosts[i].username, username) == 0) idx = i;
    if (idx == -1 && count < MAX_GHOST_LOG) {
        idx = count++; memset(&ghosts[idx], 0, sizeof(GhostRecord)); strcpy(ghosts[idx].username, username);
    }
    if (idx != -1) {
        if (operation == 7) { if (correctCount == 3) ghosts[idx].ghostCurse[operation] += 1; }
        else {
            if (correctCount >= 8) ghosts[idx].ghostCurse[operation] += 2;
            else if (correctCount <= 4 && ghosts[idx].ghostCurse[operation] > 0) ghosts[idx].ghostCurse[operation] -= 1;
        }
        ghosts[idx].lastCorrectCount[operation] = correctCount; saveGhosts(ghosts, count);
    }
}

// Game Engine
void showHowToPlay(int operation, int difficulty, int timeLimit) {
    const char* modeNames[] = {"", "ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", "EXPONENTS", "ALGEBRA", "ULTRA HARD"};
    clear();
    printHeader("TUTORIAL");
    printf(" Mode: %s\n Difficulty: Level %d\n Time: %d sec/question\n", modeNames[operation], difficulty + 1, timeLimit);
    printf("------------------------------------------\n");
    printf(" >> Press ENTER to START...");
    getchar();
}

int playGame(char username[], int operation) {
    int rounds = (operation == 7) ? 3 : 10;
    int score = 0, correctCount = 0, hintUsed = 0;
    GhostRecord ghost = getGhostData(username);
    int difficulty = ghost.ghostCurse[operation];
    int timeLimit = (operation == 7) ? 60 : (operation == 6 ? 15 : (operation == 5 ? 10 : 7));

    showHowToPlay(operation, difficulty, timeLimit);

    for (int i = 1; i <= rounds; i++) {
        int a, b, correct, answer; char hintText[250] = "";
        clear();
        printf("\n==========================================\n");
        printf(" QUESTION %d / %d              TIME: %ds\n", i, rounds, timeLimit);
        printf("==========================================\n");

        if (operation == 1) { a = getRandomRange(1, 10 + (difficulty * 5)); b = getRandomRange(1, 10 + (difficulty * 5)); correct = a + b; printf("\n     %d + %d = ", a, b); }
        else if (operation == 2) { a = getRandomRange(20, 50 + (difficulty * 5)); b = getRandomRange(1, a - 1); correct = a - b; printf("\n     %d - %d = ", a, b); }
        else if (operation == 3) { a = getRandomRange(2, 12 + difficulty); b = getRandomRange(2, 12 + difficulty); correct = a * b; printf("\n     %d * %d = ", a, b); }
        else if (operation == 4) { b = getRandomRange(2, 10); correct = getRandomRange(1, 20); a = b * correct; printf("\n     %d / %d = ", a, b); }
        else if (operation == 5) { a = getRandomRange(2, 5); b = getRandomRange(2, 4); correct = (int)pow(a, b); printf("\n     %d ^ %d = ", a, b); }
        else if (operation == 6) { int x = getRandomRange(1, 15); a = getRandomRange(2, 5); b = getRandomRange(1, 20); int res = (a * x) + b; correct = x; printf("\n     Solve for X: %dx + %d = %d\n     X = ", a, b, res); }
        else if (operation == 7) {
            int sub = getRandomRange(1, 7);
            switch(sub) {
                case 1: a = getRandomRange(3, 8); b = getRandomRange(4, 10); correct = (a * b) / 2; strcpy(hintText, "(B * H) / 2"); printf("\n     Triangle Area (B:%d, H:%d): ", a, b); break;
                case 2: a = getRandomRange(1, 5); b = getRandomRange(1, 5); correct = (a > b) ? a : b; strcpy(hintText, "Roots: x^2 + (a+b)x + ab = 0"); printf("\n     Larger root: x^2 + (%d)x + (%d) = 0: ", -(a+b), a*b); break;
                case 3: a = getRandomRange(2, 4); b = getRandomRange(2, 4); correct = b; strcpy(hintText, "b^x = y"); printf("\n     log_%d(%d) = x: ", a, (int)pow(a, b)); break;
                case 4: a = getRandomRange(1, 5); b = getRandomRange(1, 5); correct = (a*a) + (b*b); strcpy(hintText, "Dist^2: a^2 + b^2"); printf("\n     Sq. Dist to (%d,%d): ", a, b); break;
                case 5: a = getRandomRange(10, 20); b = getRandomRange(20, 30); int c = getRandomRange(30, 40); correct = (a+b+c)/3; strcpy(hintText, "(A+B+C) / 3"); printf("\n     Mean of %d, %d, %d: ", a, b, c); break;
                case 6: a = getRandomRange(100, 400); b = getRandomRange(5, 20); correct = a/b; strcpy(hintText, "D / T"); printf("\n     Velocity for %dm in %ds: ", a, b); break;
                case 7: a = getRandomRange(2, 5); b = getRandomRange(2, 5); correct = a+(4*b); strcpy(hintText, "a1 + (n-1)d"); printf("\n     a5 for (a1:%d, d:%d): ", a, b); break;
            }
        }

        time_t start = time(NULL);
        while (1) {
            if (scanf("%d", &answer) != 1) { clearInputBuffer(); answer = -9999; } else { clearInputBuffer(); }
            if (answer == -1 && operation == 7 && !hintUsed) { hintUsed = 1; printf("\n     [HINT] %s\n     Answer: ", hintText); continue; }
            break;
        }
        time_t end = time(NULL);

        printf("\n==========================================\n");
        if (difftime(end, start) > timeLimit) printf(" [!] TIME UP! Answer: %d\n", correct);
        else if (answer == correct) { printf(" [+] CORRECT!\n"); score += (operation == 7 ? 50 : 10); correctCount++; }
        else printf(" [-] WRONG! Answer: %d\n", correct);
        printf("==========================================\n");
        pressEnter();
    }
    updateGhostData(username, correctCount, operation); return score;
}

// Math interface
int main() {
    srand((unsigned int)time(NULL));
    int mainChoice; char loggedUser[USERNAME_LEN];
    while (1) {
        clear(); printHeader("MATH MASTERY PRO");
        printf(" [1] Register\n [2] Login\n [3] Exit\n\n>> Selection: ");
        if (scanf("%d", &mainChoice) != 1) { clearInputBuffer(); continue; }
        clearInputBuffer();

        if (mainChoice == 1) registerUser();
        else if (mainChoice == 2) {
            if (loginUser(loggedUser)) {
                int gameChoice;
                while (1) {
                    clear(); char welcome[100]; sprintf(welcome, "PLAYER: %s", loggedUser); printHeader(welcome);
                    printf(" [1] Addition    [5] Exponents\n [2] Subtraction [6] Algebra\n [3] Multiply    [7] ULTRA HARD\n [4] Division    [8] Leaderboards\n [9] Logout\n\n>> ");
                    if (scanf("%d", &gameChoice) != 1) { clearInputBuffer(); continue; }
                    clearInputBuffer();

                    if (gameChoice >= 1 && gameChoice <= 7) {
                        int finalScore = playGame(loggedUser, gameChoice);
                        ScoreRecord sc[MAX_USERS]; int count = loadScores(sc);
                        updateScore(sc, &count, loggedUser, gameChoice, finalScore);
                        saveScores(sc, count);
                        clear(); printHeader("GAME SUMMARY");
                        printf(" Session Score: %d\n", finalScore);
                        showTop10(sc, count, gameChoice); pressEnter();
                    } else if (gameChoice == 8) {
                        ScoreRecord sc[MAX_USERS]; int count = loadScores(sc);
                        int b; clear(); printHeader("LEADERBOARDS");
                        printf(" [0] Overall [1-7] Specific Modes\n ID: ");
                        scanf("%d", &b); clearInputBuffer();
                        showTop10(sc, count, (b >= 0 && b <= 7) ? b : 0); pressEnter();
                    } else if (gameChoice == 9) break;
                }
            }
        } else if (mainChoice == 3) break;
    }
    return 0;
}
