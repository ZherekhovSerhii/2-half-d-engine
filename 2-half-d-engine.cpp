#include <iostream>
#include <chrono>
#include <Windows.h>

#include <vector>
#include<algorithm>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapWidth = 16;
int nMapHeight = 16;

float fFOV = 3.14159 / 4.0;
float fDepth = 16.0f;

int main(int argc, char* argv[]) {
	//Створення буферу
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	std::wstring map;
	map += L"###########....#";
	map += L"#..............#";
	map += L"#...........####";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#######........#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"#..............#";
	map += L"################";

	auto tp1 = std::chrono::system_clock::now();
	auto tp2 = std::chrono::system_clock::now();

	//Основний цикл
	while (true) {
		tp2 = std::chrono::system_clock::now();
		std::chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();

		//Контролі
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += (0.8f) * fElapsedTime;

		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'E') & 0x8000) {
			fPlayerX += sin(fPlayerA + 1.571) * 5.0f * fElapsedTime;
			fPlayerY += cos(fPlayerA + 1.571) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sin(fPlayerA + 1.571) * 5.0f * fElapsedTime;
				fPlayerY -= cos(fPlayerA + 1.571) * 5.0f * fElapsedTime;
			}
		}

		if (GetAsyncKeyState((unsigned short)'Q') & 0x8000) {
			fPlayerX -= sin(fPlayerA + 1.571) * 5.0f * fElapsedTime;
			fPlayerY -= cos(fPlayerA + 1.571) * 5.0f * fElapsedTime;

			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sin(fPlayerA + 1.571) * 5.0f * fElapsedTime;
				fPlayerY += cos(fPlayerA + 1.571) * 5.0f * fElapsedTime;
			}
		}

		for (int x = 0; x < nScreenWidth; ++x) {
			//Підрахування довжини променю (ray) для кожного стовпчику консолі
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0.0f;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle); //Кут для кожного променю що виходить з точки де знаходиться player 
			float fEyeY = cosf(fRayAngle);

			while (!bHitWall) {
				fDistanceToWall += 0.1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//Перевірка чи не вийшов промень за карту
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;			//Перевірка на пересічення зі стіною
					fDistanceToWall = fDepth;
				}
				else {
					//Якщо є пересічення, то вираховуємо дистанцію
					if (map.c_str()[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;

						std::vector<std::pair<float, float>> p; 

						for(int tx=0; tx<2; ++tx)
							for (int ty = 0; ty < 2; ++ty) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(std::make_pair(d, dot));
							}

						std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) {return left.first < right.first; });

						float fBound = 0.005;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;
					}
				}
			}

			//Вирахування дистанції до стелі і полу
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0) nShade = 0x2588;		//Дуже близько
			else if (fDistanceToWall < fDepth / 3.0) nShade = 0x2593;
			else if (fDistanceToWall < fDepth / 2.0) nShade = 0x2592;
			else if (fDistanceToWall < fDepth / 1.0) nShade = 0x2591;
			else									  nShade = ' ';		//Далеко

			if (bBoundary) nShade = ' ';

			for (int y = 0; y < nScreenHeight; ++y) {
				if (y < nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else {
					short nShadeFloor = ' ';
					//Замальовуємо засновуючись на дистанції
					float b = 1.0f - (((float)y - nScreenHeight / 2.0f) / ((float)nScreenHeight / 2.0f));
					if (b < 0.25)		nShadeFloor = '#';
					else if (b < 0.5)   nShadeFloor = 'x';
					else if (b < 0.75)  nShadeFloor = '.';
					else if (b < 0.9)   nShadeFloor = '-';
					else				nShadeFloor = ' ';
					screen[y * nScreenWidth + x] = nShadeFloor;
				}
			}
		}

		//Дані про позицію, ФПС, кут зору
		swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f, FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f / fElapsedTime);

		//Мапа
		for(int nx=0; nx<nMapWidth; ++nx)
			for (int ny = 0; ny < nMapHeight; ++ny) {
				screen[(ny + 1) * nScreenWidth + nx] = map[ny * nMapWidth + (nMapWidth - nx - 1)];
			}

		//Player на мапі
		screen[((int)fPlayerY + 1) * nScreenWidth + (int)(nMapWidth - fPlayerX)] = 'P';

		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0, 0 }, &dwBytesWritten);
	}
	std::cin.get();
	return 0;
}