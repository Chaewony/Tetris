#pragma once
#include "Panel.h"

class Continue:
    public Panel
{
    Panel* panel;

public:
    Continue(GameObject* confirmationPanel)
        : Panel("Continue", Position{ 25, 15 }, 5, 2, confirmationPanel)
    {
        panel = new Panel{ "Continue", Position{30, 15}, 5, 2, confirmationPanel };
    }

    void draw() override
    {
    }
    void update() override
    {
        if (input->getMouseButtonDown(0))
        {
            Position mouse = screen2local(input->getMousePosition());
            if (mouse.x >= 4 && mouse.x <= 10 && mouse.y >= -1 && mouse.y <= 2)
                parent->setActive(false);

        }
    }
};

