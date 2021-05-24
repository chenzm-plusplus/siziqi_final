//
//  utc.cpp
//  Strategy
//
//  Created by chenzm on 2020/4/24.
//  Copyright © 2020 Yongfeng Zhang. All rights reserved.
//

#include "utc.h"

//===========================---初始化---======================================
UCT_Node tree[upper_Case];

UCT_Node* UCT::newNode(UCT_Node* parent) {
    auto new_node = &tree[USED_TREE_NODE++];
    new_node->parent=parent;
    new_node->score=0;
    new_node->searched_times=0;
    memset(new_node->children, 0, sizeof(new_node->children));
    return new_node;
}

/*
 初始化UTC
 */
UCT::UCT(const int M, const int N, const int* _top, int ** _board,
    const int noX, const int noY, const int _lastX, const int  _lastY):
        M(M),N(N),noX(noX),noY(noY),
        lastX(_lastX),lastY(_lastX),
    USED_TREE_NODE(0),last_x(-1),last_y(-1),
    top(_top),board(_board)
    {
    ROOT=newNode(nullptr);

    //开辟空间
    board_ = new int*[M];
    for(int i = 0; i < M; i++) {
        board_[i] = new int[N];
    }

    top_ = new int[N];
        //!!!!!!!!!不过其实不做这个初始化也不影响，但是为了安全还是做了吧
        resetBoard();
}

/*
 析构函数要保证内存不泄露
 */
UCT::~UCT(){
    delete[] top_;
    for(int i = 0; i < M; i++) {
        delete[] board_[i];
    }
    delete[] board_;
}

/*
 resetBoard：重新进行初始化
 */
void UCT::resetBoard() {
    cur_player = me_;
    for(int i = 0; i < M; i++) {
        for(int j = 0; j < N; j++){
            board_[i][j] = board[i][j];
        }
    }
    board_[noX][noY]=Board::no;
    copy(top, top+N, top_);
}

//=============================---private，简化函数逻辑---=====================================

float UCT::usedTime(const clock_t start_clock){
    return (float)(((float)clock()-(float)start_clock)/(float)CLOCKS_PER_SEC);
}

bool UCT::inTimeLimit(const clock_t start_clock, const float time_limit){
    return usedTime(start_clock)<time_limit;
}

Player UCT::nextPlayer(Player current_player){
    return (Player)(3-cur_player);
}

void UCT::putInLine(int **board_, int *top_, int cur_line){
    top_[cur_line]--;
    if(board_[top_[cur_line]][cur_line] == Board::no){
        top_[cur_line]--;
    }
    board_[top_[cur_line]][cur_line]=cur_player;
    last_x = top_[cur_line];
    last_y = cur_line;
}

bool UCT::isValidPut(int **board_, int *top_, int cur_line) const{
    return (top_[cur_line]>0 && (board_[top_[cur_line]-1][cur_line] == Board::empty))
    || top_[cur_line]>1 ;
}

int UCT::getRow(int **board_, int *top_, int cur_line)const{
    return board_[top_[cur_line]-1][cur_line]==Board::empty?top_[cur_line]-1:top_[cur_line]-2;
}

bool UCT::willWin(int **board_, const int& cur_line){
    bool must_win = false;
    int get_row = getRow(board_, top_, cur_line);
    board_[get_row][cur_line]=cur_player;
    if(JudgeStatus(get_row,cur_line)==Result::win_loss) {
        must_win=true;
    }
    board_[get_row][cur_line]=Board::empty;
    return must_win;
}

/*
 返回应当落子在第几列
 如果没合适的选择也会返回0，保证至少没有非法的落子
 */
int UCT::selectChild(UCT_Node* father, const int* validLines, const int valid_counts, float C) {
    float max_value = -FLT_MAX;
    int chosen_child = 0;
    float log_sum = C*2*log(father->searched_times);
    for(int i = 0; i < valid_counts; i++) {
        UCT_Node* tmp_child = father->children[validLines[i]];
        float tmp_value = getChildValue(tmp_child, log_sum);
        if(tmp_value>max_value) {
            chosen_child=validLines[i];
            max_value=tmp_value;
        }
    }
    return chosen_child;
}

int UCT::decideSolution(float C) {
    float max_value = -FLT_MAX;
    int chosen_child = 0;
    UCT_Node* tmp_child;
    float log_sum = C*2*log(ROOT->searched_times);
    for(int i = 0; i < N; i++) {
        if((tmp_child = ROOT->children[i])) {
            float tmp_value=(1+getLamda(i))*getChildValue(tmp_child,log_sum);
            if(tmp_value>max_value) {
                chosen_child=i;
                max_value=tmp_value;
            }
        }
    }
    return chosen_child;
}

float UCT::getChildValue(UCT_Node *child, const float& log_sum){
    return child->score/child->searched_times + sqrt(log_sum/child->searched_times);
}


Result UCT::JudgeStatus(int lastX,int lastY) {
    if(cur_player==Player::me_ && machineWin(lastX,lastY, M, N, board_)) {
        return Result::win_loss;
    }
    if(cur_player==Player::you_ && userWin(lastX,lastY, M, N, board_)) {
        return Result::win_loss;
    }
    if(isTie(N, top_))return Result::tie;
    return Result::not_end;
}

void UCT::swap(int& a,int& b){
    int t=a;a=b;b=t;
}

void UCT::createRandom(int* valid,const int N){
    for(int i=0;i<N;i++){
        valid[i]=i;
    }
    for(int j=N-1;j>=0;j--){
        int index = rand() % (j+1);
        swap(valid[j], valid[index]);
    }
}

float UCT::getLamda(int line){
    return max((float)(0),((float)abs(2-(line-lastY))/2)*LAMDA);
}

//=============================---函数主要逻辑---=====================================


Point* UCT::utcRoutine(float time_limit) {
//    cerr<<"in utcRoutine..."<<endl;
//    printBoard_();
    //有挺多要用到随机数的地方
    srand((unsigned)time(NULL));
    
    clock_t start_clock = clock();
    int round = 0;
    int cnt=0;
//    cerr<<"before while..."<<endl;
    while (USED_TREE_NODE<upper_Used&&round<ONE_ROUND_COUNTS) {
        //判断是否需要循环。为了减少clock()函数引发的系统调用次数，每运行2048次再进行一次时间判断
        round++;
        if(round>=ONE_ROUND_COUNTS){
            round=0;
            cnt++;
            if(!inTimeLimit(start_clock, time_limit)){
                break;
            }
        }
        resetBoard();
        auto node = treePolicy();
        float score = defautPolicy();
        backUp(node, score);
    }
#ifdef DEBUG
//    printf("used tree node = %d\n", USED_TREE_NODE);
//    printf("rounds = %d\n",cnt*ONE_ROUND_COUNTS);
#endif
//    printf("round = %d\n", round);
//    cerr<<"used tree node = "<<USED_TREE_NODE<<endl;
//    cerr<<"rounds = "<<cnt*ONE_ROUND_COUNTS<<endl;
    int y=decideSolution();
    int x=board[top[y]-1][y]==Board::empty?(top[y]-1):(top[y]-2);
    
    //返回一个point类
//    cerr<<"end utcRouting...(x,y)=("<<x<<","<<y<<")"<<endl;
    return new Point(x,y);
}

float UCT::defautPolicy() {
    const int ini_player = cur_player;
    int count = 0;
    int validLines[MAX_N]={0};
    //不断下棋
    while (true) {
        count++;
        auto status = JudgeStatus(last_x, last_y);
        switch (status) {
            case Result::win_loss:{
                //返回赢或输
                int score = count==1? WIN_ACCELERATE : 1;
                if (cur_player == ini_player) {
                    return score;
                }else{
                    return -score;
                }
                break;
            }
            case Result::tie:{
                return 0;
                break;
            }
            case Result::not_end:
            {
                /*
                bool will_win=false;
                int valid_counter=0;
                //查找可行点集
                int randomCheckLines[MAX_N];
                createRandom(randomCheckLines, N);
                for(int line = 0; line < N; line++) {
                    if(isValidPut(board_, top_, randomCheckLines[line])) {//如果第line列可以放子的话
                        validLines[valid_counter++]=randomCheckLines[line];//找到了一个可行点
                        //检查是否为必胜点，如果是必胜点的话就只下这个点
                        will_win = willWin(board_, randomCheckLines[line]);
                        if(will_win){
                            valid_counter = 1;
                            validLines[0]=randomCheckLines[line];
                            break;
                        }
                    }
                }
                */
                int valid_counter=0;
                //查找可行点集
                int randomCheckLines[MAX_N];
                createRandom(randomCheckLines, N);
                for(int line = 0; line < N; line++) {
                    if(isValidPut(board_, top_, randomCheckLines[line])) {//如果第line列可以放子的话
                        validLines[valid_counter++]=randomCheckLines[line];//找到了一个可行点
                        //检查是否为必胜点，如果是必胜点的话就只下这个点
                    }
                }
                
                //注意！！！！！因为是向下探索所以要先换人
                cur_player = nextPlayer(cur_player);
                int index=0;
                if(valid_counter>0)index = rand()%valid_counter;
                int chosen = validLines[index];
                putInLine(board_, top_, chosen);
            }
            default:
                break;
        }
    }
}


void UCT::backUp(UCT_Node* node, float score) {
//    cerr<<"backup"<<endl;
    float scores = score;
    UCT_Node* cur_node = node;
    while (cur_node) {
        cur_node->score += scores;
        cur_node->searched_times++;
        //由于必胜局会获得比较高的评分，但是并不希望这个评分会向上传递。
        //因此scores在向上传递的过程中会逐层-1,逐渐减少必胜局带来的影响
        scores = scores>1 ? (1-scores) : ((scores<-1) ? (-1-scores) : (-scores) );
        cur_node=cur_node->parent;
    }
//    cerr<<"end backup"<<endl;
}


/*
 返回一个搜索节点
 */
UCT_Node* UCT::treePolicy() {
//    cerr<<"in tree Policy"<<endl;
    UCT_Node* current_node = ROOT;
    cur_player=Player::me_; //
    
    int valid_counter;
    //查找所有可行落子点
    int validLines[MAX_N];

    int chosen_child = -1;
    //
    while(true) {
        bool will_win = false;
        valid_counter=0;
        //查找可行点集
        //生成随机数

        for(int line = 0; line < N; line++) {
            if(isValidPut(board_, top_, line)) {//如果第line列可以放子的话
                validLines[valid_counter++]=line;//找到了一个可行点
                //检查是否为必胜点，如果是必胜点的话就只下这个点
                will_win = willWin(board_, line);
                if(will_win){
                    valid_counter = 1;
                    validLines[0]=line;
                    break;
                }
            }
        }
        if(valid_counter==0) return current_node;//没有可落子点了
        
        //如果可落子点数>0
        //优先扩展新节点，如果没有新节点则选择已经扩展的节点
        UCT_Node* expand_new_node = expand(current_node, validLines, valid_counter, chosen_child);
        if(chosen_child==-1) chosen_child=selectChild(current_node,validLines, valid_counter);
        putInLine(board_, top_, chosen_child);
        
        if(expand_new_node) return expand_new_node;
        if(will_win) return current_node->children[validLines[0]];  //
        
        //如果以上条件都不满足，则继续向下搜索
        current_node=current_node->children[chosen_child];
        cur_player=nextPlayer(cur_player);
    }
}

/*
 返回值:
    >=0表示新扩展节点成功
    ==-1表示下面的节点已经都扩展过了
 */
UCT_Node* UCT::expand(UCT_Node* father, const int* validLines,
                  const int valid_counts, int& chosen_child) {
    for(int i = 0; i < valid_counts;i++) {
        if(!father->children[validLines[i]]) {
            chosen_child=validLines[i];
            father->children[chosen_child]=newNode(father);
            //新扩展一个节点立刻返回
            return father->children[chosen_child];
        }
    }
    chosen_child=-1;
    return nullptr;
}
