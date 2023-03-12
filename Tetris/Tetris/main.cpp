#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <conio.h> // console io
#include <cstring> // string.h
#include <cstdlib> // stdlib.h
#include <string>
#include <Windows.h>
#include "Utils.h"

using namespace std;

class Screen;
class GameObject;
class Input;

class Screen {
private:
	int		width; // visible width
	int		height;
	int		size;
	char* canvas;

	// constructor (생성자 함수) 메모리공간상에 적재되는 순간 호출되는
	Screen(int width = 10, int height = 10)
		: width(width), height(height), canvas(new char[(width + 1) * height])
	{
		bool faultyInput = false;
		if (this->width <= 0) {
			this->width = 10;
			faultyInput = true;
		}
		if (this->height <= 0) {
			this->height = 10;
			faultyInput = true;
		}
		size = (this->width + 1) * this->height;
		if (faultyInput == true) {
			delete canvas;
			canvas = new char[size];
		}
	}
	// destructor (소멸자 함수) 메모리공간상에서 없어지는 순간 호출되는 함수
	virtual ~Screen()
	{
		delete[] canvas;
		canvas = nullptr;
		width = 0; height = 0;
	}

	static Screen* Instance;

public:

	static Screen* GetInstance() {
		if (Instance == nullptr) {
			Instance = new Screen(20, 10);
		}
		return Instance;
	}

	int getWidth() const
	{
		return width;
	}

	int getHeight() const
	{
		return height;
	}

	void setWidth(int width)
	{
		this->width = width;
	}

	void clear()
	{
		memset(canvas, ' ', size);
	}

	int pos2Offset(const Position& pos) const
	{
		return (width + 1) * pos.y + pos.x;
	}

	void draw(const Position& pos, const char* shape, const Dimension& sz = Position{ 1, 1 })
	{
		int offset = pos2Offset(pos);
		for (int h = 0; h < sz.y; h++)
			strncpy(&canvas[offset + (width + 1) * h], &shape[h * sz.x], sz.x);
	}
	void render()
	{
		Borland::gotoxy(0, 0);
		for (int h = 0; h < height; h++)
			canvas[(width + 1) * (h + 1) - 1] = '\n';
		canvas[size - 1] = '\0';
		printf("%s", canvas);
	}

};

class Input {
	DWORD cNumRead, fdwMode, i;
	INPUT_RECORD irInBuf[128];
	int counter;
	HANDLE hStdin;
	DWORD fdwSaveOldMode;
	char blankChars[80];

	void errorExit(const char*);
	void keyEventProc(KEY_EVENT_RECORD);
	void mouseEventProc(MOUSE_EVENT_RECORD);
	void resizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

	static Input* Instance;

	Input()
	{
		memset(blankChars, ' ', 80);
		blankChars[79] = '\0';

		hStdin = GetStdHandle(STD_INPUT_HANDLE);
		if (hStdin == INVALID_HANDLE_VALUE)
			errorExit("GetStdHandle");
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode))
			errorExit("GetConsoleMode");
		/*
			   Step-1:
			   Disable 'Quick Edit Mode' option programmatically
		 */
		fdwMode = ENABLE_EXTENDED_FLAGS;
		if (!SetConsoleMode(hStdin, fdwMode))
			errorExit("SetConsoleMode");
		/*
		   Step-2:
		   Enable the window and mouse input events,
		   after you have already applied that 'ENABLE_EXTENDED_FLAGS'
		   to disable 'Quick Edit Mode'
		*/
		fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
		if (!SetConsoleMode(hStdin, fdwMode))
			errorExit("SetConsoleMode");

	}

	~Input() {
		SetConsoleMode(hStdin, fdwSaveOldMode);
	}

public:

	static Input* GetInstance()
	{
		if (Instance == nullptr) {
			Instance = new Input();
		}
		return Instance;
	}

	void readInputs()
	{
		if (!GetNumberOfConsoleInputEvents(hStdin, &cNumRead)) {
			cNumRead = 0;
			return;
		}
		if (cNumRead == 0) return;

		Borland::gotoxy(0, 14);
		printf("number of inputs %d\n", cNumRead);

		if (!ReadConsoleInput(
			hStdin,      // input buffer handle
			irInBuf,     // buffer to read into
			128,         // size of read buffer
			&cNumRead)) // number of records read
			errorExit("ReadConsoleInput");
		// Dispatch the events to the appropriate handler.

		for (int i = 0; i < cNumRead; i++)
		{
			switch (irInBuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input
				keyEventProc(irInBuf[i].Event.KeyEvent);
				break;

			case MOUSE_EVENT: // mouse input
				mouseEventProc(irInBuf[i].Event.MouseEvent);
				break;

			case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
				resizeEventProc(irInBuf[i].Event.WindowBufferSizeEvent);
				break;

			case FOCUS_EVENT:  // disregard focus events

			case MENU_EVENT:   // disregard menu events
				break;

			default:
				errorExit("Unknown event type");
				break;
			}
		}

		Borland::gotoxy(0, 0);
	}
	bool getKeyDown(WORD virtualKeyCode);
	bool getKey(WORD virtualKeyCode);
	bool getKeyUp(WORD virtualKeyCode);
};

Screen* Screen::Instance = nullptr;
Input* Input::Instance = nullptr;

class GameObject
{
private:
	char		blockShape[20];
	Position	pos;
	Position	secondPos;
	Dimension	dim;
	Screen* screen;
	Input* input;
	bool isPiledUp;
	bool isTop;
	float speed;

public:

	GameObject(const char* blockShape, const Position& pos, const Dimension& dim)
		: pos(pos), screen(Screen::GetInstance()), dim(dim), input(Input::GetInstance()), isTop(false), secondPos(Position(pos.x + 1, pos.y)), speed(500)
	{
		setBlockShape(blockShape);
	}
	virtual ~GameObject() {}

	virtual void draw() //draw에 필요 없는 코드 분리 해야함
	{
		//아래가 쌓여있으면
		if (isPiledUp)
		{
			screen->draw(pos, blockShape, dim);       //draw
			screen->draw(secondPos, blockShape, dim); //draw

			if (pos.y == 1 || secondPos.y == 1)       //쌓여있는데 y가 1이면 (어차피 드랍시 바로 아웃이니까)
				isTop = true;                         //게임오버 시켜주기 위해 isTop을 true로 해줌

			return; //바로 빠져나가기
		}

		//드랍
		if (pos.y + 1 < screen->getHeight() || secondPos.y + 1 < screen->getHeight())
		{
			pos.y = (pos.y + 1);                      //아래로 블럭하강
			secondPos.y = (secondPos.y + 1);          //아래로 블럭 하강
		}
		else isPiledUp = true;                        //쌓였음 (하강 중지)

		//그리기
		screen->draw(pos, blockShape, dim);
		screen->draw(secondPos, blockShape, dim);
	}

	virtual void update()
	{
		if (!isPiledUp)
		{
			//블럭 좌로 이동
			if (input->getKey(VK_LEFT)) {
				if (pos.x <= 0 || secondPos.x <= 0) return;
				pos.x = (pos.x - 1) % (screen->getWidth());
				secondPos.x = (secondPos.x - 1) % (screen->getWidth());
			}
			//블럭 우로 이동
			if (input->getKey(VK_RIGHT)) {
				if (pos.x >= (screen->getWidth() - 1) || secondPos.x >= (screen->getWidth() - 1)) return;
				pos.x = (pos.x + 1) % (screen->getWidth());
				secondPos.x = (secondPos.x + 1) % (screen->getWidth());
			}
			//rotate
			if (input->getKey(VK_UP)) {
				if (secondPos.x > pos.x && secondPos.y == pos.y)
					setSecondPos(Position{ pos.x, pos.y - 1 });
				else if (secondPos.x == pos.x && secondPos.y < pos.y)
					setSecondPos(Position{ pos.x - 1,pos.y });
				else if (secondPos.x < pos.x && secondPos.y == pos.y)
					setSecondPos(Position{ pos.x, pos.y + 1 });
				else if (secondPos.x == pos.x && secondPos.y > pos.y)
					setSecondPos(Position{ pos.x + 1,pos.y });

			}
			//하강 speed 빠르게 조정
			if (input->getKey(VK_DOWN)) {
				this->speed -= 100;
			}
		}
	}

	void Pile(Position pos)
	{
		screen->draw(pos, blockShape, dim);
		screen->draw(secondPos, blockShape, dim);
	}

	Position getPos() { return pos; } // getter function
	Position getSecondPos() { return secondPos; } // getter function
	void setSecondPos(const Position& pos) { this->secondPos = pos; } // setter function
	void setPos(const Position& pos) { this->pos = pos; } // setter function

	const char* getBlockShape() { return blockShape; }
	void setBlockShape(const char* blockShape) { strcpy(this->blockShape, blockShape); }

	Screen* getScreen() { return screen; }

	bool getIsPiledUp() { return isPiledUp; }
	void setIsPiledUp(bool isPiledUp) { this->isPiledUp = isPiledUp; }
	bool getIsTop() { return isTop; }
	float getSpeed() { return speed; }
};

int main()
{
	Screen* screen = Screen::GetInstance();
	Input* input = Input::GetInstance();

	Position pos{ screen->getWidth() / 2, 0 };
	const char shape[] = "*";
	Dimension sz{ (int)strlen(shape), 1 };
	GameObject* blocks[100];
	for (int i = 0; i < 100; i++)
	{
		blocks[i] = new GameObject{ shape, pos,  sz };
	}

	// Get the standard inp
	bool isLooping = true;

	int i = 0;
	int myLine0 = 0;
	int myLine1 = 0;
	int myLine2 = 0;
	int myLine3 = 0;
	int myLine4 = 0;
	int myLine5 = 0;
	int myLine6 = 0;
	int myLine7 = 0;
	int myLine8 = 0;
	int myLine9 = 0;

	while (isLooping) {

		//화면 클리어
		screen->clear();

		//화면 그리기
		for (int j = 0; j < i; j++)
			blocks[j]->draw();
		blocks[i]->draw();

		//키 인풋
		input->readInputs();

		//업데이트
		for (int j = 0; j < i; j++)
			blocks[j]->update();
		blocks[i]->update();

		//렌더
		screen->render();
		Sleep(blocks[i]->getSpeed());

		//쌓기
		for (int j = 0; j < i; j++) //for문 돌면서 블럭들 검사
		{
			if (blocks[j] == NULL) continue;

			Position piledBlockPos{ blocks[j]->getPos() }; //j가 원래 있었던 블럭
			Position piledBlockSecondPos{ blocks[j]->getSecondPos() }; //원래 있었던 블럭의 두번째 요소
			Position newBlockPos{ blocks[i]->getPos() }; //i가 새로 드랍된 블럭
			Position newBlockSecondPos{ blocks[i]->getSecondPos() }; //새로 드랍된 블럭의 두번째 요소

			//검사한 블럭중 포지션 겹치는 블럭이 있으면
			if (piledBlockPos.x == newBlockPos.x && piledBlockPos.y - 1 == newBlockPos.y)
			{
				blocks[i]->setIsPiledUp(true);
			}
			else if (piledBlockPos.x == newBlockSecondPos.x && piledBlockPos.y - 1 == newBlockSecondPos.y)
			{
				blocks[i]->setIsPiledUp(true);
			}
			else if (piledBlockSecondPos.x == newBlockPos.x && piledBlockSecondPos.y - 1 == newBlockPos.y)
			{
				blocks[i]->setIsPiledUp(true);
			}
		}

		//쌓인 블럭의 y값에 해당하는 라인 크기 키워주기
		if (blocks[i]->getIsPiledUp())
		{
			switch (blocks[i]->getPos().y)
			{
			case 0:
			{
				myLine0 += 2;
				break;
			}
			case 1:
			{
				myLine1 += 2;
				break;
			}
			case 2:
			{
				myLine2 += 2;
				break;
			}
			case 3:
			{
				myLine3 += 2;
				break;
			}
			case 4:
			{
				myLine4 += 2;
				break;
			}
			case 5:
			{
				myLine5 += 2;
				break;
			}
			case 6:
			{
				myLine6 += 2;
				break;
			}
			case 7:
			{
				myLine7 += 2;
				break;
			}
			case 8:
			{
				myLine8 += 2;
				break;
			}
			case 9:
			{
				myLine9 += 2;
				break;
			}
			default:
			{
				break;
			}
			}
		}

		//한 줄 채워지면 블럭 삭제하기 , 이런식으로 모든 줄 다 해야함
		Position deletePos{ 1,10 };
		if (myLine9 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 9)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 9)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine9 = 0; //초기화
		}
		if (myLine8 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 8)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 8)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine8 = 0; //초기화
		}
		if (myLine7 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 7)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 7)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine7 = 0; //초기화
		}
		if (myLine6 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 6)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 6)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine6 = 0; //초기화
		}
		if (myLine5 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 5)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 5)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine5 = 0; //초기화
		}
		if (myLine4 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 4)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 4)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine4 = 0; //초기화
		}
		if (myLine3 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 3)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 3)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine3 = 0; //초기화
		}
		if (myLine2 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 2)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 2)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine2 = 0; //초기화
		}
		if (myLine1 >= screen->getWidth())
		{
			for (int j = 0; j <= i; j++)
			{
				if (blocks[j]->getPos().y == 1)
				{
					blocks[j]->setPos(deletePos);
				}
				else
				{
					blocks[j]->setPos(Position{ blocks[j]->getPos().x,blocks[j]->getPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
				if (blocks[j]->getSecondPos().y == 1)
				{
					blocks[j]->setSecondPos(deletePos);
				}
				else
				{
					blocks[j]->setSecondPos(Position{ blocks[j]->getSecondPos().x,blocks[j]->getSecondPos().y + 1 }); //아닌애들은 한줄씩 내려줌
				}
			}
			myLine6 = 1; //초기화
		}

		//게임 루프 확인
		for (int j = 0; j < i; j++) //for문 돌면서 블럭들 검사
		{
			if (blocks[j] == NULL) continue;

			//검사한 블럭중 맨위에 있는 블럭이 있으면
			if (blocks[j]->getIsTop())
				isLooping = false; //게임루프 종료
		}

		//새로운 블럭 드랍
		if (blocks[i]->getIsPiledUp())
			i++;
	}
	printf("\nGame Over\n");

	return 0;
}

void Input::errorExit(const char* lpszMessage)
{
	fprintf(stderr, "%s\n", lpszMessage);

	// Restore input mode on exit.

	SetConsoleMode(hStdin, fdwSaveOldMode);

	ExitProcess(0);
}

bool Input::getKeyDown(WORD virtualKeyCode)
{
	// TODO: NOT FULLY IMPLEMENTED YET
	return getKey(virtualKeyCode);
}

bool Input::getKey(WORD virtualKeyCode)
{
	if (cNumRead == 0) return false;

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode &&
			irInBuf[i].Event.KeyEvent.bKeyDown == TRUE) {
			return true;
		}
	}
	return false;
}

bool Input::getKeyUp(WORD virtualKeyCode)
{
	if (cNumRead == 0) return false;

	for (int i = 0; i < cNumRead; i++)
	{
		if (irInBuf[i].EventType != KEY_EVENT) continue;

		if (irInBuf[i].Event.KeyEvent.wVirtualKeyCode == virtualKeyCode &&
			irInBuf[i].Event.KeyEvent.bKeyDown == FALSE) {
			return true;
		}
	}
	return false;
}


void Input::keyEventProc(KEY_EVENT_RECORD ker)
{
	Borland::gotoxy(0, 11);
	printf("%s\r", blankChars);
	switch (ker.wVirtualKeyCode) {
	case VK_LBUTTON:
		printf("left button ");
		break;
	case VK_BACK:
		printf("back space");
		break;
	case VK_RETURN:
		printf("enter key");
		break;
	case VK_LEFT:
		printf("arrow left");
		break;
	case VK_UP:
		printf("arrow up");
		break;
	default:
		if (ker.wVirtualKeyCode >= 0x30 && ker.wVirtualKeyCode <= 0x39)
			printf("Key event: %c ", ker.wVirtualKeyCode - 0x30 + '0');
		else printf("Key event: %c ", ker.wVirtualKeyCode - 0x41 + 'A');
		break;
	}

	Borland::gotoxy(0, 0);
}

void Input::mouseEventProc(MOUSE_EVENT_RECORD mer)
{
	Borland::gotoxy(0, 12);
	printf("%s\r", blankChars);
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
	printf("Mouse event: ");

	switch (mer.dwEventFlags)
	{
	case 0:
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			printf("left button press %d %d\n", mer.dwMousePosition.X, mer.dwMousePosition.Y);
		}
		else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
		{
			printf("right button press \n");
		}
		else
		{
			printf("button press\n");
		}
		break;
	case DOUBLE_CLICK:
		printf("double click\n");
		break;
	case MOUSE_HWHEELED:
		printf("horizontal mouse wheel\n");
		break;
	case MOUSE_MOVED:
		printf("mouse moved %d %d\n", mer.dwMousePosition.X, mer.dwMousePosition.Y);
		break;
	case MOUSE_WHEELED:
		printf("vertical mouse wheel\n");
		break;
	default:
		printf("unknown\n");
		break;
	}
	Borland::gotoxy(0, 0);
}

void Input::resizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
	Borland::gotoxy(0, 13);
	printf("%s\r", blankChars);
	printf("Resize event: ");
	printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
	Borland::gotoxy(0, 0);
}