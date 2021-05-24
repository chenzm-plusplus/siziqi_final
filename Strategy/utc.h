//
//  utc.h
//  Strategy
//
//  Created by chenzm on 2020/4/24.
//  Copyright © 2020 Yongfeng Zhang. All rights reserved.
//

#ifndef utc_h
#define utc_h

#include <iostream>
#include <ctime>// clock
#include <algorithm>
#include "Point.h"
#include <cmath>
#include <string.h>
#include "Strategy.h"
#include "Judge.h"
#include "common.h"
#include "utc_node.h"
#include "types.h"
//#include <conio.h>                //
//#include <atlstr.h>

using namespace std;

extern UCT_Node tree[upper_Case];

#define DEFAULT_TIME_LIMIT 2.85
#define DEFAULT_C 0
#define DEFAULT_C_S 0.7

class UCT {
    //---函数参数调整----------
    static const int ONE_ROUND_COUNTS = 2048;
    static const int WIN_ACCELERATE = 5;
    const float LAMDA = 0.01;


    //---存储棋盘信息，在节点搜索过程中保持不变-----------
    const int M;  //
    const int N;  //
    int** board;  //
    const int* top;  //

    const int noX;  //
    const int noY;
    
    const int lastX;
    const int lastY;
    
    //----存储搜索过程中的动态棋盘信息，在每次走步之后要进行更新，在重新搜索的时候要初始化---------------------
    Player cur_player;  //当前该谁走步了
    int** board_;  //
    int* top_;  //
    int last_x;  //
    int last_y;

    //-----存储树上分配了多少节点----------------
    int USED_TREE_NODE;  //
    UCT_Node* ROOT;
    
    inline float usedTime(const clock_t start_clock);
    
    inline bool inTimeLimit(const clock_t start_clock, const float time_limit);
    
    inline bool isValidPut(int** board_, int* top_, int cur_line) const;
    
    inline int getRow(int** board_, int* top_, int cur_line) const;
    
    /*
     落子
     注意！！！会更改当前棋盘状态
     落子时要改变：当前棋盘状态，当前该谁走步，上一步走在哪里
     */
    void putInLine(int** board_, int* top_,int cur_line);
    
    float getLamda(int line);

    
    /*
     在第line列落子之后将会必胜
     */
    inline bool willWin(int** board_,const int& cur_line);
    
    inline float getChildValue(UCT_Node* child, const float& log_sum = 0);
    
    inline Player nextPlayer(Player current_player);
    
    void swap(int& a,int& b);

    void createRandom(int* valid,const int N);
    
    UCT_Node* newNode(UCT_Node* parent);
public:
    //
    UCT(const int M, const int N, const int* _top, int** _board,
        const int noX, const int noY, const int _lastX, const int _lastY);
    
    ~UCT();
    
    Point* utcRoutine(float time_limit = DEFAULT_TIME_LIMIT);

private:
    float defautPolicy();
    
    void backUp(UCT_Node* node, float score);
    
    int decideSolution(float C = DEFAULT_C);
    
    Result JudgeStatus(int lastX,int lastY);
    
    UCT_Node* treePolicy();
    
    UCT_Node* expand(UCT_Node* root_, const int* feasible, const int feasible_num, int& chosed);
    
    void resetBoard();
    
    int selectChild(UCT_Node* root_, const int* feasible, const int feasible_num, float C= DEFAULT_C_S);
  
#ifdef DEBUG
    void printBoard_(){
        printf("\nboard_\n");
        for(int i = 0; i < M; i++) {
            for(int j = 0; j < N; j++) {
                printf("%d ", board_[i][j]);
            }
            printf("\n");
        }
        for(int j = 0; j < N; j++) {
            printf("%d ", top_[j]);
        }
        printf("\n==================\n");
    }
//    void printBoard(){
//        printf("\nboard\n");
//        for(int i = 0; i < M; i++) {
//            for(int j = 0; j < N; j++) {
//                printf("%d ", board[i][j]);
//            }
//            printf("\n");
//        }
//        for(int j = 0; j < N; j++) {
//            printf("%d ", top[j]);
//        }
//        printf("\n==================\n");
//
//    }
#endif
};


#endif /* utc_h */
