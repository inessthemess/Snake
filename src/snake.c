#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "snakeAPI.h"
#include "arena.h"
#include "snake.h"




Snake* initSnake(int x, int y) {
    Snake* s = (Snake*)malloc(sizeof(Snake));
    s->x = x;
    s->y = y;
    s->length = 1;
    s->suivant = NULL;
    return s;
}


Snake* copySnake(Snake* original) {
    if (original == NULL) return NULL;

    Snake* copy = (Snake*)malloc(sizeof(Snake));
    copy->x = original->x;
    copy->y = original->y;
    copy->suivant = copySnake(original->suivant);

    return copy;
}




void freeSnake(Snake* snake){
    Snake* tmp = snake;
    Snake* suiv;

    while (tmp != NULL) {
        suiv = tmp->suivant;
        free(tmp);
        tmp = suiv;
    }
}



t_move sendMySnakeMove() {
    int moveInt;
    t_return_code moveSent;
    scanf("%d", &moveInt);
    t_move move = (t_move)moveInt;
    if (move == NORTH) {
        moveSent=sendMove(NORTH);
        return NORTH;
    } else if (move == EAST) {
        moveSent=sendMove(EAST);
        return EAST;
    } else if (move == SOUTH) {
        moveSent=sendMove(SOUTH);
        return SOUTH;
    } else if (move == WEST) {
        moveSent=sendMove(WEST);
        return WEST;
    }
    return -1; 
}



void moveSnake(Snake** head, t_move dir, int grow, Arena* arena) {
    int newX = (*head)->x;
    int newY = (*head)->y;

    switch (dir) {
        case NORTH: 
            newY--;
            break;
        case EAST: 
            newX++;
            break;
        case SOUTH: 
            newY++;
            break;
        case WEST: 
            newX--;
            break;
        default:
            return;
    }


        if (newX < 0 || newX >= arena->sizeX || newY < 0 || newY >= arena->sizeY ||
    arena->cells[newY][newX].snake ||
    (dir == NORTH && arena->cells[(*head)->y][(*head)->x].wallTop) ||
    (dir == EAST && arena->cells[(*head)->y][(*head)->x].wallRight) ||
    (dir == SOUTH && arena->cells[(*head)->y][(*head)->x].wallBottom) ||
    (dir == WEST && arena->cells[(*head)->y][(*head)->x].wallLeft)) {
        printf("Invalid move\n");
        return;
    }

    Snake* newHead = (Snake*)malloc(sizeof(Snake));
    newHead->x = newX;
    newHead->y = newY;
    newHead->suivant = *head;
    *head = newHead;

    arena->cells[newY][newX].snake = 1;

    if (!grow) {
        Snake* current = *head;
        Snake* prev = NULL;
        while (current->suivant != NULL) {
            prev = current;
            current = current->suivant;
        }
        arena->cells[current->y][current->x].snake = 0;
        free(current);
        if (prev != NULL)
            prev->suivant = NULL;
    }
}



//inspired by a code i did last year on bfs
void markAccessibleCells(Arena* arena,Snake* s,int** distance){
    for (int i = 0; i < arena->sizeY; i++) {
        for (int j = 0; j < arena->sizeX; j++) {
            distance[i][j] = -1; 
        }
    }
    Position* queue=(Position*)malloc(arena->sizeX*arena->sizeY*sizeof(Position));
    int front=0;
    int rear=0;
    Position p = {s->x,s->y};
    queue[rear++]=p;
    distance[p.y][p.x]=0;

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    while (front < rear){
        Position current = queue[front++];
        int curDist= distance[current.y][current.x];

        for (int i = 0; i < 4; i++){
            int nx=current.x+dx[i];
            int ny=current.y+dy[i];

            if (nx>=0 && nx<arena->sizeX && ny>=0 && ny<arena->sizeY){
                if (distance[ny][nx]==-1 && !arena->cells[ny][nx].snake &&
                    !((i==0 && arena->cells[current.y][current.x].wallTop) ||
                    (i==1 && arena->cells[current.y][current.x].wallRight) ||
                    (i==2 && arena->cells[current.y][current.x].wallBottom) ||
                    (i==3 && arena->cells[current.y][current.x].wallLeft))){
                        distance[ny][nx]=curDist+1;
                        queue[rear++]=(Position){nx,ny};
                }
            }
        }
    }
    free(queue);    
}


t_move decideNextMove(Arena* arena, Snake* s, int** distance){
    int bestDist = -1;
    t_move bestMove = NORTH;

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};


    printf("Snake position: %d %d\n", s->x, s->y);

    printf("Distance array: \n");
    for (int i = 0; i < arena->sizeY; i++) {
    for (int j = 0; j < arena->sizeX; j++) {
        printf("%d ", distance[i][j]);
    }
    printf("\n");
}

    for (int i=0; i<4;i++){
        int nx=s->x+dx[i];
        int ny=s->y+dy[i];

        printf("Checking move %d: (%d, %d) -> (%d, %d)\n", i, s->x, s->y, nx, ny);

        if (nx >= 0 && nx < arena->sizeX && ny >= 0 && ny < arena->sizeY &&
                distance[ny][nx] != -1 &&
                !arena->cells[ny][nx].snake &&
                !((i == 0 && arena->cells[s->y][s->x].wallTop) ||
                (i == 1 && arena->cells[s->y][s->x].wallRight) ||
                (i == 2 && arena->cells[s->y][s->x].wallBottom) ||
                (i == 3 && arena->cells[s->y][s->x].wallLeft))) {

                    if (distance[ny][nx]>bestDist){
                        bestDist=distance[ny][nx];
                        bestMove=(t_move)i;
                    }
        }
    }
    printf("bestDist: %d\n", bestDist);
    return bestMove;
}




int countAccessibleCells(Arena* arena, int** distance) {
    int accessibleCells = 0;
    for (int i = 0; i < arena->sizeY; i++) {
        for (int j = 0; j < arena->sizeX; j++) {
            if (distance[i][j] != -1) {
                accessibleCells++;
            }
        }
    }
    return accessibleCells;
}



int evaluateBoard(Arena* arena, Snake* mySnake, Snake* enemySnake, int** distanceMySnake, int** distanceEnemySnake) {
    markAccessibleCells(arena, mySnake, distanceMySnake);

    markAccessibleCells(arena, enemySnake, distanceEnemySnake);

    
    int myAccessibleCells = countAccessibleCells(arena, distanceMySnake);
    int enemyAccessibleCells = countAccessibleCells(arena, distanceEnemySnake);

    int score = myAccessibleCells - enemyAccessibleCells;


    int myProximityToEnemy = abs(mySnake->x - enemySnake->x) + abs(mySnake->y - enemySnake->y);
    int enemyProximityToCenter = abs(enemySnake->x - arena->sizeX / 2) + abs(enemySnake->y - arena->sizeY / 2);
    
    score += (arena->sizeX + arena->sizeY - myProximityToEnemy); 
    score -= (arena->sizeX + arena->sizeY - enemyProximityToCenter); 

    return score;
}



//interesting article about minimax algorithm in Snake game : https://coreyja.com/posts/BattlesnakeMinimax/Minimax%20in%20Battlesnake/
int minimax(Arena* arena, Snake* mySnake, Snake* enemySnake, int** distanceMySnake, int** distanceEnemySnake, int depth, int alpha, int beta, int maximizingPlayer, int grow) {
    if (depth == 0) {
        return evaluateBoard(arena, mySnake, enemySnake, distanceMySnake, distanceEnemySnake);
    }

    // Vecteurs de déplacement possibles (haut, droite, bas, gauche)
    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    if (maximizingPlayer) {
        int maxEval = -100000;
        for (int i = 0; i < 4; i++) {
            int nx = mySnake->x + dx[i];
            int ny = mySnake->y + dy[i];

            // on vérifie si la case est valide
            if ((nx >= 0 && nx < arena->sizeX && ny >= 0 && ny < arena->sizeY && 
                !arena->cells[ny][nx].snake) &&
                !((i == 0 && arena->cells[mySnake->y][mySnake->x].wallTop) ||
                (i == 1 && arena->cells[mySnake->y][mySnake->x].wallRight) ||
                (i == 2 && arena->cells[mySnake->y][mySnake->x].wallBottom) ||
                (i == 3 && arena->cells[mySnake->y][mySnake->x].wallLeft))) {

                // on fait une copie de l'arène et du snake pour simuler le mouvement
                Arena* arenaCopy = copyArena(arena);
                Snake* mySnakeCopy = copySnake(mySnake);

                moveSnake(&mySnakeCopy, (t_move)i, grow, arenaCopy);
                
                int eval = minimax(arenaCopy, mySnakeCopy, enemySnake, distanceMySnake, distanceEnemySnake, depth - 1, alpha, beta, 0, grow);
                // on libère la mémoire des copies
                freeArena(arenaCopy);
                freeSnake(mySnakeCopy);
                
                // on met à jour la valeur maximale
                maxEval = fmax(maxEval, eval);
                
                alpha = fmax(alpha, eval);
                // Coupure alpha-beta : si beta est inférieur ou égal à alpha, on arrête l'exploration
                // Cela signifie que le joueur minimisant a trouvé une meilleure option ailleurs
                if (beta <= alpha) {
                    break;
                }
            }
        }
        return maxEval;
    } else {
        int minEval = 100000;
        for (int i = 0; i < 4; i++) {
            int nx = enemySnake->x + dx[i];
            int ny = enemySnake->y + dy[i];

            if (nx >= 0 && nx < arena->sizeX && ny >= 0 && ny < arena->sizeY && 
                !arena->cells[ny][nx].snake &&
                !((i == 0 && arena->cells[enemySnake->y][enemySnake->x].wallTop) ||
                (i == 1 && arena->cells[enemySnake->y][enemySnake->x].wallRight) ||
                (i == 2 && arena->cells[enemySnake->y][enemySnake->x].wallBottom) ||
                (i == 3 && arena->cells[enemySnake->y][enemySnake->x].wallLeft))) {

                Arena* arenaCopy = copyArena(arena);
                Snake* enemySnakeCopy = copySnake(enemySnake);

                moveSnake(&enemySnakeCopy, (t_move)i, grow, arenaCopy);

                int eval = minimax(arenaCopy, mySnake, enemySnakeCopy, distanceMySnake, distanceEnemySnake, depth - 1, alpha, beta, 1, grow);
                freeArena(arenaCopy);
                freeSnake(enemySnakeCopy);

                minEval = fmin(minEval, eval);
                beta = fmin(beta, eval);
                if (beta <= alpha) {
                    break;
                }
            }
        }
        return minEval;
    }
}



t_move decideMinimaxMove(Arena* arena, Snake* mySnake, Snake* enemySnake, int** distanceMySnake, int** distanceEnemySnake, int depth, int grow) {
    int bestMove = NORTH;
    int bestValue = -100000;

    int dx[] = {0, 1, 0, -1};
    int dy[] = {-1, 0, 1, 0};

    for (int i = 0; i < 4; i++) {
        int nx = mySnake->x + dx[i];
        int ny = mySnake->y + dy[i];

        if ((nx >= 0 && nx < arena->sizeX && ny >= 0 && ny < arena->sizeY && 
            !arena->cells[ny][nx].snake) &&
            !((i == 0 && arena->cells[mySnake->y][mySnake->x].wallTop) ||
            (i == 1 && arena->cells[mySnake->y][mySnake->x].wallRight) ||
            (i == 2 && arena->cells[mySnake->y][mySnake->x].wallBottom) ||
            (i == 3 && arena->cells[mySnake->y][mySnake->x].wallLeft))) {

            Arena* arenaCopy = copyArena(arena);
            Snake* mySnakeCopy = copySnake(mySnake);

            moveSnake(&mySnakeCopy, (t_move)i, 0, arenaCopy);

             // Appel de la fonction minimax pour évaluer la valeur du plateau après le mouvement
            int boardValue = minimax(arenaCopy, mySnakeCopy, enemySnake, distanceMySnake, distanceEnemySnake, depth - 1, -100000, 100000, 0, grow);
            freeArena(arenaCopy);
            freeSnake(mySnakeCopy);

            // Mise à jour du meilleur mouvement si une meilleure valeur est trouvée
            if (boardValue > bestValue) {
                bestValue = boardValue;
                bestMove = (t_move)i;
            }
        }
    }
    return bestMove;
}

