#pragma once
#include "Panel.h"
#include "Exit.h"
#include "Continue.h"

class ConfirmationPanel :
    public Panel
{
    Panel* panel;
    Exit* exit;
    Continue* myContinue;
    bool myExit;

public:
    ConfirmationPanel(GameObject* tetris) //parent(tetris)를 매개변수로 받는다
        : Panel("", Position{ 10, 0 }, 20, 10, tetris), myExit(false)
    {
        panel = new Panel{ "", Position{20, 10}, 20, 10, this }; //새로운 panel을 child로 설정
        exit = new Exit(this); //Exit 버튼을 child로 설정
        myContinue = new Continue(this);

        this->setActive(false);
        
    }

    void draw() override
    {
        //for (auto child : children) child->setActive(true);
    }
    void update() override
    {
        if (exit->getExit())
            myExit = true;
    }
    bool getMyExit() { return myExit; }
};

