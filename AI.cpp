#include"AI.h"
#include<algorithm>
#include<sys/time.h>
#include<cmath>
const int Dx[4]={1,1,1,0};
const int Dy[4]={-1,0,1,1};
AI::Node* AI::newNode(int player)
{
    Node *node=&record_nodes[nodescnt++];
    node->player=player;
    node->total=0.0;
    node->UCB_a=0.0;
    node->UCB_b=0.0;
    node->winner=0;
    node->wins=0;
    for(int i=0;i<N;i++)
    {
        int x=M-1;
        while(x>=0&&board[x][i]!=0)
            x--;
        node->Childrens[i] = NULL;
        node->top[i]=x;
    }
    return node;
}

AI::~AI(){
    delete[] board;
    delete[] baseboard;
    delete[] record_nodes;
}

bool AI::JudgeWin(int x,int y){
    for(int i=0;i<4;i++)
    {
        int dx=Dx[i],dy=Dy[i];
        int cnt=1;
        for(int nx = x + dx,ny = y + dy;Reasonable(nx, ny);nx += dx,ny += dy){
            if(board[x][y]==board[nx][ny])
                cnt++;
            else
                break;
        }
        for(int nx = x - dx,ny = y - dy;Reasonable(nx, ny);nx -= dx,ny -= dy){
            if(board[x][y]==board[nx][ny])
                cnt++;
            else
                break;
        }
        if(cnt >= 4)
            return true;
    }
    return false;
}

bool AI::Reasonable(int x,int y)
{
    if(x>=0&&x<M&&y>=0&&y<N)
        return true;
    else
        return false;
}

void AI::GetInfo(const int M, const int N, const int *top, int **_board, const int lastX, const int lastY, const int noX, const int noY){
    this->M=M;
    this->N=N;
    board = new int *[M];
    baseboard = new int *[M];
    for(int i=0;i<M;i++)
    {
        board[i] =new int[N];
        baseboard[i]=new int[N];
    }
    
    for(int i=0;i<M;i++)
        for(int j=0;j<N;j++){
            board[i][j]=_board[i][j];
            baseboard[i][j]=_board[i][j];
        }
    board[noX][noY]=3;
    baseboard[noX][noY]=3;
    record_nodes=new Node[2*MAX_TIMES];
    for(int i=0;i<2*MAX_TIMES;i++){
        record_nodes[i].Childrens.resize(N);
        record_nodes[i].top.resize(N);
    }
    
}

double Time(){
    struct timeval tm;
    gettimeofday(&tm, NULL);
    double t=tm.tv_sec+1e-6*tm.tv_usec;
    return t;
}

AI::Node* AI::Selection(Node*root,std::vector<Node*>& path){
    Node *now = root;
    while(true){
        path.push_back(now);
        //        printf("path:%d \n",path.size());
        if(now->winner != 0){
            break;
        }
        for(int i = 0;i < N;i++){
            if(now->Childrens[i]!=NULL&&now->Childrens[i]->winner == now->player)
            {
//                printf("i:%d win!\n",i);
                now = now->Childrens[i];
                now -> total += 2;
                now -> wins +=2;
                return now;
            }
        }
        int expand_rank = -1;
        for(int i = 0;i < N;i++)
            if(now->top[i] >= 0 && now->Childrens[i] == NULL){
                expand_rank=i;
                break;
            }
        if(expand_rank!=-1){
            int x = now->top[expand_rank];
            //if(expand_rank==0)
            //printf("expand_rank:%d\n",expand_rank);
            int y = expand_rank;
            board[x][y] = now->player;
            int next_player = Enemy_player + My_player - now->player;
            //printf("createNew\n");
            now->Childrens[expand_rank] = newNode(next_player);
            now->Childrens[expand_rank]->total = 2;
            if(JudgeWin(x,y)){
                now->Childrens[expand_rank]->winner = now->player;
                now->Childrens[expand_rank]->wins = 2 * int(now->player == My_player);
            }
            else
                now = now->Childrens[expand_rank];
            break;
        }
        else{
            double p = 0.0;
            int road_rank = -1;
            Node *next = NULL;
            for(int i = 0;i < N;i++)
            {
                if(now->Childrens[i] != NULL && p < now->Childrens[i]->UCB_a + now->Childrens[i]->UCB_b * XL)
                {
                    p = now->Childrens[i]->UCB_a + now->Childrens[i]->UCB_b * XL;
                    next = now->Childrens[i];
                    road_rank = i;
                }
            }
            //printf("this choice:%d\n",road_rank);
            if(next == NULL){
                //    printf("expand_rank:%d\n",expand_rank);
                return next;
            }
            else{
                //  printf("new:%d\n",expand_rank);
                int x = now->top[road_rank];
                int y = road_rank;
                board[x][y] = now->player;
                now = next;
            }
        }
    }
    return now;
}

int AI::Simulation(Node* node){
    if(node == NULL)
        return 1;
    if(node->winner != 0){
        node->wins = 2 * int(node->winner == My_player);
        return int(node->wins);
    }
    Node *Nodenow = newNode(node->player);
    //    Nodenow->player = node->player;
    //    for(int i=0;i<N;i++)
    //        Nodenow->top[i]=node->top[i];
    while(true){
        int choice = 0;
        int setx[M],sety[N];
        for(int i = 0;i < N;i++)
        {
            if(Nodenow->top[i] >= 0){
                setx[choice]=Nodenow->top[i];
                sety[choice]=i;
                choice++;
            }
        }
        if(choice){
            //            printf("%d\n",choice);
            int x = -1;
            int y = -1;
            for(int i = 0;i < choice && x == -1;i++)
            {
                board[setx[i]][sety[i]]=Nodenow->player;
                if(JudgeWin(setx[i], sety[i])){
                    int result = int(Nodenow->player == My_player);
                    //delete Nodenow;
                    node->wins = result;
                    return result;
                }
                board[setx[i]][sety[i]]=0;
            }
            for(int i = 0;i < choice && x == -1;i++)
            {
                board[setx[i]][sety[i]] = My_player + Enemy_player - Nodenow->player;
                if(JudgeWin(setx[i], sety[i]))
                {
                    x = setx[i];
                    y = sety[i];
                }
                board[setx[i]][sety[i]]=0;
            }
            if(x == -1){
                int t = rand()%choice;
                x = setx[t];
                y = sety[t];
            }
            board[x][y] = Nodenow->player;
            //            for(int i=0;i<M;i++){
            //                for(int j=0;j<N;j++)
            //                    printf("%d ",board[i][j]);
            //                printf("\n");
            //            }
            //            printf("\n\n");
            if(JudgeWin(x,y))
            {
                int result = int(Nodenow->player == My_player);
                // delete Nodenow;
                node->wins = result;
                return result;
            }
            Nodenow->player = My_player + Enemy_player - Nodenow->player;
            while(x >= 0 && board[x][y] != 0) x--;
            Nodenow->top[y] = x;
            
        }
        else{
            //delete Nodenow;
            //printf("tie!\n");
            node->wins = 1;
            return 1;
        }
    }
}

void AI::CalculateParameters(Node *node){
    if(node->player == Enemy_player)
    {
        node->UCB_a = double(node->wins) / double(node->total);
        node->UCB_b = sqrt(1.0 / node->total);
        //       printf("UCB_a:%f,UCB_b:%f\n",node->UCB_a,node->UCB_b);
    }
    else
    {
        node->UCB_a = double(node->total - node->wins) / double(node->total);
        node->UCB_b = sqrt(1.0/node->total);
        //      printf("UCB_a:%f,UCB_b:%f\n",node->UCB_a,node->UCB_b);
    }
}

void AI::Backpropagation(std::vector<Node *> &path, int result){
    for(int i = 0; i < path.size() ; i++)
    {
        path[i]->wins += result;
        path[i]->total += 2;
        //        printf("path:%d\n",path[i]->total);
        CalculateParameters(path[i]);
    }
    
}

Point AI::MCST(){
    Node *root = newNode(My_player);
    double start_time=Time();
    for(int i = 0;i < MAX_TIMES; i++){
        if(Time() - start_time > 1.5)
            break;
        for(int i = 0;i < M; i++)
            for(int j = 0;j < N; j++)
                board[i][j] = baseboard[i][j];
        XL = root->total <= 1 ? 0 : Con * sqrt(log(root->total));
        //        printf("XL:%f\n",XL);
        std::vector<Node*> path;
        Node *pos = Selection(root,path);
        int result;
        if((pos && pos->winner == 0) || !pos)
            result = Simulation(pos);
        else
            result = 2 * int(pos->winner == My_player);
        //        printf("path:%lld\n",path.size());
        if(pos)
            CalculateParameters(pos);
        Backpropagation(path,result);
    }
    //printf("sictimes:%d\n",sic);
    Point rst(0,0);
    double p = -100.0;
//    printf("%f\n",p);
    for(int i = 0 ; i < N ; i++){
        if(root->Childrens[i] != NULL && p < root->Childrens[i]->UCB_a){
            p = root->Childrens[i]->UCB_a;
            rst.x = root->top[i];
            rst.y = i;
        }
    }
//    for(int i = 0;i < N;i++){
//        if(root->Childrens[i]!=NULL){
//            printf("i:%d\n",i);
//            printf("roottop:%d\n",root->top[i]);
//            printf("UCB:%f\n",root->Childrens[i]->UCB_a);
//            printf("wins:%lld\n",root->Childrens[i]->wins);
//            printf("total:%lld\n",root->Childrens[i]->total);
//        }
//    }
    printf("root:%lld\n",root->total);
//    printf("rst.x:%d rst.y:%d\n",rst.x,rst.y);
    return rst;

}

Point AI::GetAction(){
    Point p = MCST();
    return p;
}

