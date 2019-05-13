#pragma once
#include"Point.h"
#include<vector>
#define MAX_TIMES 1400000
#define My_player 2
#define Enemy_player 1
#define Con 0.707
class AI
{
private:
    struct Node{
        int player;
        int winner;
        long long wins;
        long long total;
        double UCB_a,UCB_b;
        std::vector<int> top;
        std::vector<Node*> Childrens;
        Node(){};
    };
    
    int nodescnt=0;
    int **board;
    int **baseboard;
    int M,N;
    double XL=0.0;
    Node *record_nodes;
    Point MCST();
    bool JudgeWin(int x,int y);
    bool Reasonable(int x,int y);
    int Simulation(Node* node);
    void CalculateParameters(Node* node);
    Node* Selection(Node* root,std::vector<Node*>& path);
    void Backpropagation(std::vector<Node*>& path,int result);
    Node* newNode(int player);
public:
    AI(){};
    void GetInfo(const int M, const int N, const int* top, int** _board,const int lastX, const int lastY, const int noX, const int noY);
    Point GetAction();
    ~AI();
};
