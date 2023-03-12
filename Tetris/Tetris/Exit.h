#pragma once
#include "Panel.h"

class Exit:
	public Panel
{
    Panel* panel;
    bool exit;

public:
    Exit(GameObject* confirmationPanel)
        : Panel("Exit", Position{ 20, 15 }, 5, 2, confirmationPanel), exit(false)
    {
        panel = new Panel{ "Exit", Position{22, 15}, 5, 2, confirmationPanel };
    }

    void draw() override
    {
    }

    void update() override
    {
        if (input->getMouseButtonDown(0))
        {
            Position mouse = screen2local(input->getMousePosition());
            if (mouse.x >= 1 && mouse.x <= 7 && mouse.y >= -1 && mouse.y <= 2)
                //parent->setActive(false);
                exit = true;
        }
    }

    bool getExit() { return exit; }

};

