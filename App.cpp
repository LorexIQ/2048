#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <Windows.h>
#include <conio.h>
#include <map>

using namespace std;

class Game {
private:
	vector<vector<int>> data = { // массив поля
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
	};
	int score = 0; // текущий счёт
	int high_score = 0; // лучший счёт
	int run = true; // состояние приложения
	bool win_lock = true; // локер срабатывания тригера победы
	bool root_generate = false;
	bool dev_mode = false;
	
	map<int, int> console_colors = { // маппинг цветов консоли
		{0, 240},
		{2, 143},
		{4, 207}, {8, 207},
		{16, 111}, {32, 111}, {64, 111},
		{128, 95}, {256, 95}, {512, 95},
		{1024, 159}, {2048, 159},
		{-1, 47}
	};

	// поиск нулей на поле
	vector<vector<int>::iterator> searchZeros() {
		vector<vector<int>::iterator> zeros_index; // вектор для итераторов нулей
		// поиск нулей на поле
		for (auto it = data.begin(); it != data.end(); ++it)
			for (auto jt = (*it).begin(); jt != (*it).end(); ++jt)
				if (*jt == 0) zeros_index.push_back(jt);
		return zeros_index;
	}

	// создание новой плитки
	bool randGenerateTile() {
		vector<vector<int>::iterator> zeros = searchZeros(); 
		
		if (zeros.size() == 0) return false; // если нулей нет

		int pos_tile = rand() % zeros.size(); // выбор случайно клетки из 0

		*zeros[pos_tile] = (rand() % 9 < 8) ? 2 : 4; // создание новой плитки
		return true;
	}

	// проверка наличия шагов
	bool checkMotions() {
		int motions = 0;

		// пробег по всему полю в поисках одинаковых соседних клеток
		for (int r = 0; r < 4; r++) {
			for (int c = 0; c < 4; c++) {
				if (this->data[r][c] == 0 ||
					r != 0 && this->data[r][c] == this->data[r - 1][c] ||
					r != 3 && this->data[r][c] == this->data[r + 1][c] ||
					c != 0 && this->data[r][c] == this->data[r][c - 1] ||
					c != 3 && this->data[r][c] == this->data[r][c + 1])
					motions++; // плюс к кол-ву шагов
				// если клетка = 2048 и отключен лок победы
				if (this->data[r][c] == 2048 && this->win_lock) if (!this->winGame()) return false;
				if (this->data[r][c] == 8192) { // если клетка = 8192, завершаем игру
					this->completeGame();
					return false;
				}
			}
		}

		// если шагов не осталось - заканчиваем игру
		if (motions == 0) {
			this->endGame();
			return false;
		}
		return true;
	}

	// сохранение данных и закрытие приложения
	bool saveAndExit() {
		ofstream file("save.bin", ios_base::binary); // открытие файла сохранения
		
		if (!file.is_open()) return false; // проверка, открылся ли

		// сохранение каждого элемента данных в файл
		for (int r = 0; r < 4; r++)
			for (int c = 0; c < 4; c++)
				file.write((char*)&this->data[r][c], sizeof this->data[r][c]);
		file.write((char*)&this->win_lock, sizeof this->win_lock);
		file.write((char*)&this->score, sizeof this->score);
		file.write((char*)&this->high_score, sizeof this->high_score);
	
		file.close(); // закрытие файла
		return true;
	}

	// чтение сохранения
	bool openSave() {
		ifstream file("save.bin", ios_base::binary); // открытие файла сохранения
		
		if (!file.is_open()) return false; // проверка, открылся ли

		// чтение всех элементов данных из файла
		for (int r = 0; r < 4; r++)
			for (int c = 0; c < 4; c++)
				file.read((char*)&this->data[r][c], sizeof this->data[r][c]);
		file.read((char*)&this->win_lock, sizeof this->win_lock);
		file.read((char*)&this->score, sizeof this->score);
		file.read((char*)&this->high_score, sizeof this->high_score);

		file.close(); // закрытие файла
		return true;
	}

	// проверка сохранения
	bool checkSave() {
		ifstream file("save.bin", ios_base::binary); // открытие файла сохранения
		
		if (!file.is_open()) return false; // если файла нет

		file.seekg(0, ios_base::end); // перемещаем указатель в конец файла
		int size_file = file.tellg(); // получаем размер
		
		file.seekg(-4, ios_base::end); // перемещаем указатель на ячейку счёта
		file.read((char*)&this->high_score, sizeof this->high_score); // читаем лучший счёт

		file.close(); // закрываем файл
		
		if (size_file == 4) return false; // если в сохранении только лучший счёт
		return true;
	}

	// сохранение лучшего счёта
	void writeHighScore() {
		ofstream file("save.bin", ios_base::binary); // открытие файла сохранения

		if (!file.is_open()) return; // проверка, открыт ли

		file.seekp((int)file.tellp(), ios_base::end); // указатель перед последней ячейкой
		file.write((char*)&this->high_score, sizeof this->high_score); // записываем данные

		file.close(); // закрываем сайт
	}

	// вывод плиток игрового поля на экран
	void printTiles() {
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		int key_color = 0;

		SetConsoleOutputCP(866); // DOS кодировка
		cout << "\xC9" << setw(26) << setfill('\xCD') << "\xBB\n"; // верхняя рамка
		for (auto it = data.begin(); it != data.end(); ++it) {
			cout << '\xBA'; // левая рамка строки
			for (auto jt = (*it).begin(); jt != (*it).end(); ++jt) {
				key_color = (*jt > 2048) ? -1 : *jt; // код цвета для словаря
				// установка цвета консоли для ячейки числа
				SetConsoleTextAttribute(hConsole, this->console_colors[key_color]);
				int len_number = (int)to_string(*jt).size(); // символьный размер числа
				// вывод числа
				cout << setw(3 - (int)len_number / 2) << setfill(' ') << "" 
					<< (*jt)
					<< setw(2 - (int)len_number / 3) << "";
				SetConsoleTextAttribute(hConsole, 240); // возврат цвета консоли
			}
			cout << "\xBA\n"; // правая рамка строки
			// добавление отступов между строк
			if (it != data.end() - 1) {
				for (int i = 0; i < 2; ++i)
					cout << "\xBA" << setw(26) << "\xBA\n";
			}
		}
		cout << "\xC8" << setw(26) << setfill('\xCD') << "\xBC\n"; // нижняя рамка
		SetConsoleOutputCP(1251); // возвращаем кодировку
	}

	// вывод счёта
	void scoreBoard() {
		cout << " Score: " << this->score << '\n'
			<< " High score: " << this->high_score << '\n'; 
	}

	// вывод горячих клавиш
	void hotKeys() {
		cout << setfill(' ') << " W - up" << setw(15) << "A - left\n"
			<< " S - down" << setw(15) << "D - right\n\n"
			<< " Q - save and exit\n N - new game\n";
	}

	void highScoreView() {
		cout << setw(20 - to_string(this->score).size()) << setfill(' ') << " YOUR SCORE: " << this->score << '\n';
		if (this->high_score < this->score) { // если счёт игры больше лучшегго
			cout << "  This is new RECORD!!!\n";
			this->high_score = this->score; // устанавливаем новый лучший счёт
			this->score = 0; // обнуляем
		}
	}

	// конец игры
	void endGame() {
		system("cls"); // очистка консоли
		this->printTiles(); // вывод игрового поля
		this->highScoreView(); // вывод карточки счёта всей игры
		cout << "\n   The end... You LOSE!\n";

		this->run = false; // остановка приложения
		this->writeHighScore(); // сохранение лучшего счёта
	}

	bool winGame() {
		system("cls"); // очистка консоли
		this->win_lock = false; // разблокируем победу
		this->printTiles(); // вывод игрового поля
		this->highScoreView(); // вывод карточки счёта всей игры
		cout << "\n         You WIN!\n"
			<< "\nDo you want to continue the game? [y/n] ";
		int key;
		while (true) {
			key = _getch();
			if (key == 121 || key == 173) return true; // н/y - продолжаем игру
			if (key == 110 || key == 226) { // т/n - окончание игры
				this->run = false; // остановка приложения
				this->writeHighScore(); // сохранение лучшего счёта 
				return false;
			} 
		}
	}

	// принудительное завершение игры
	void completeGame() {
		system("cls"); // очистка консоли
		this->printTiles(); // вывод игрового поля
		this->highScoreView(); // вывод карточки счёта всей игры
		cout << "\n   You complete game!!!\n"
			<< "        The END...\n";

		this->run = false; // остановка приложения
		this->writeHighScore(); // сохранение лучшего счёта 
	}

	// новая игра
	bool newGame() {
		system("cls"); // очистка консоли
		this->scoreBoard(); // панель со счётом
		this->printTiles(); // вывод игрового поля
		cout << "\n Do you really want to start a new game? [y/n] ";
		int key;
		while (true) {
			key = _getch();
			if (key == 121 || key == 173) break; // н/y - перезагружаем игру
			if (key == 110 || key == 226) {
				this->root_generate = true;
				return true;
			} // т/n - продолжаем игру
		}

		this->data = { // очищаем игровое поле
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0},
		{0, 0, 0, 0}
		};
		this->score = 0; // сбразываем текущий счёт
		return true;
	}

	// вывод игрового поля на экран
	void printGameField() {
		system("cls"); // очистка консоли
		this->scoreBoard(); // вывод колонок счёта
		this->printTiles(); // вывод плиток
		this->hotKeys(); // вывод горячих клавиш
	}

	// сдвиг вверх
	bool shiftUp() {
		// копируем поля
		vector<vector<int>> data_copy(this->data.size());
		copy(this->data.begin(), this->data.end(), data_copy.begin());

		for (int c = 0; c < 4; c++) { // цикл по столбикам
			for (int r = 1; r < 4; r++) { // цикл по строкам
				int connects = 0; // количество сложений в одном столбце 
				// ^ для избежания тройного сложения в столбце, состоящем
				// из плиток одинаковых значений
				if (this->data[r][c] == 0) continue; // если клетка = 0 - пропускаем
				else if (this->data[r - 1][c] == 0) { // если предыдущая клетка = 0
					this->data[r - 1][c] = this->data[r][c]; // перезаписываем
					this->data[r][c] = 0; // очищаем текущую
					r = connects; // начинаем итерацию полей с начала
				}
				// если предыдущая клетка = текущей
				else if (this->data[r - 1][c] == this->data[r][c]) {
					this->data[r - 1][c] *= 2; // предыдущую удваиваем по значению
					this->score += this->data[r - 1][c]; // добавление очков
					this->data[r][c] = 0; // очищаем текущую
					connects = 1; // добавляем связь
					r = connects; // начинаем итерацию полей с начала
				}
			}
		}
		// если поля отличаются, считаем ход
		if (data_copy != this->data) return true;
		return false;
	}

	// сдвиг влево
	bool shiftLeft() {
		// копируем поля
		vector<vector<int>> data_copy(this->data.size());
		copy(this->data.begin(), this->data.end(), data_copy.begin());

		for (int r = 0; r < 4; r++) { // цикл по строкам
			for (int c = 1; c < 4; c++) { // цикл по столбикам
				int connects = 0; // количество сложений в одном столбце 
				// ^ для избежания тройного сложения в столбце, состоящем
				// из плиток одинаковых значений
				if (this->data[r][c] == 0) continue; // если клетка = 0 - пропускаем
				else if (this->data[r][c - 1] == 0) { // если предыдущая клетка = 0
					this->data[r][c - 1] = this->data[r][c]; // перезаписываем
					this->data[r][c] = 0; // очищаем текущую
					c = connects; // начинаем итерацию полей с начала
				}
				// если предыдущая клетка = текущей
				else if (this->data[r][c - 1] == this->data[r][c]) {
					this->data[r][c - 1] *= 2; // предыдущую удваиваем по значению
					this->score += this->data[r][c - 1]; // добавление очков
					this->data[r][c] = 0; // очищаем текущую
					connects = 1; // добавляем связь
					c = connects; // начинаем итерацию полей с начала
				}
			}
		}
		// если поля отличаются, считаем ход
		if (data_copy != this->data) return true;
		else return false;
	}

	// сдвиг вниз
	bool shiftDown() {
		// копируем поля
		vector<vector<int>> data_copy(this->data.size());
		copy(this->data.begin(), this->data.end(), data_copy.begin());

		for (int c = 0; c < 4; c++) { // цикл по столбикам
			for (int r = 2; r >= 0; r--) { // цикл по строкам
				int connects = 3; // количество сложений в одном столбце 
				// ^ для избежания тройного сложения в столбце, состоящем
				// из плиток одинаковых значений
				if (this->data[r][c] == 0) continue; // если клетка = 0 - пропускаем
				else if (this->data[r + 1][c] == 0) { // если предыдущая клетка = 0
					this->data[r + 1][c] = this->data[r][c]; // перезаписываем
					this->data[r][c] = 0; // очищаем текущую
					r = connects; // начинаем итерацию полей с начала
				}
				// если предыдущая клетка = текущей
				else if (this->data[r + 1][c] == this->data[r][c]) {
					this->data[r + 1][c] *= 2; // предыдущую удваиваем по значению
					this->score += this->data[r + 1][c]; // добавление очков
					this->data[r][c] = 0; // очищаем текущую
					connects = 2; // добавляем связь
					r = connects; // начинаем итерацию полей с начала
				}
			}
		}
		// если поля отличаются, считаем ход
		if (data_copy != this->data) return true;
		else return false;
	}

	// сдвиг вправо
	bool shiftRight() {
		// копируем поля
		vector<vector<int>> data_copy(this->data.size());
		copy(this->data.begin(), this->data.end(), data_copy.begin());

		for (int r = 0; r < 4; r++) { // цикл по строкам
			for (int c = 2; c >= 0; c--) { // цикл по столбикам
				int connects = 3; // количество сложений в одном столбце 
				// ^ для избежания тройного сложения в столбце, состоящем
				// из плиток одинаковых значений
				if (this->data[r][c] == 0) continue; // если клетка = 0 - пропускаем
				else if (this->data[r][c + 1] == 0) { // если предыдущая клетка = 0
					this->data[r][c + 1] = this->data[r][c]; // перезаписываем
					this->data[r][c] = 0; // очищаем текущую
					c = connects; // начинаем итерацию полей с начала
				}
				// если предыдущая клетка = текущей
				else if (this->data[r][c + 1] == this->data[r][c]) {
					this->data[r][c + 1] *= 2; // предыдущую удваиваем по значению
					this->score += this->data[r][c + 1]; // добавление очков
					this->data[r][c] = 0; // очищаем текущую
					connects = 2; // добавляем связь
					c = connects; // начинаем итерацию полей с начала
				}
			}
		}
		// если поля отличаются, считаем ход
		if (data_copy != this->data) return true;
		else return false;
	}
public:
	Game() {};
	Game(bool dev) : dev_mode(dev) { // dev режим
		if (dev) {
			this->data = { // массив поля
				{0, 0, 4096, 4096},
				{0, 0, 0, 0},
				{1024, 0, 0, 0},
				{1024, 0, 0, 0}
			};
		}
	};
	void start() {
		srand((int)time(0)); // рандом зависящий от времени
		int key;

		if (this->checkSave() && !this->dev_mode) { // если сохранение найдено. Отключено в dev режиме
			cout << " Saving found. Select an action:\n  S - load save\n  N - new game\n";
			while (!this->root_generate) { // пока не выбрана опция
				key = _getch();
				// если файл прочитался, выходим из цикла
				if (key == 115 || key == 235) if (this->openSave()) this->root_generate = true; // н/y
				if (key == 110 || key == 226) break; // т/n - начинаем новую игру
			}
		}

		while (this->run) { // цикл игры
			// в первом цикле после созранения, не будет генерироваться новая клетка
			if (!this->root_generate) this->randGenerateTile();
			else this->root_generate = false;

			if (this->checkMotions()) { // если имеются вариации ходов
				this->printGameField(); // выводим игровое поле с информацией
				while (true) { // считываем нужную клавишу
					key = _getch();
					if (key == 119 || key == 230) if (this->shiftUp()) break; // ц/w - вверх
					if (key == 97 || key == 228) if (this->shiftLeft()) break; // ф/a - влево
					if (key == 115 || key == 235) if (this->shiftDown()) break; // ы/s - вниз
					if (key == 100 || key == 162) if (this->shiftRight()) break; // в/d - вправо
					if (key == 113 || key == 169) if (this->saveAndExit()) return; // й/q - сохранить и выйти
					if (key == 110 || key == 226) if (this->newGame()) break; // т/n - новая игра
				}
			}
		}
		
	}
};

int main() {
	Game test; // создание объекта игры. true в параметрах, для dev режима
	test.start(); // запуск игры

	return 0;
}
