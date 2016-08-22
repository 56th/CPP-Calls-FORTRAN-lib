#include <iostream>

extern "C" // наша функция содержится в статической библиотеке
	// (фактически объектном файле) и будет подключена на этапе линковки
	// extern "C" говорит линковщику, что следует играть по правилам 
	// языка C (которые будут попроще, чем у C++ — например, в C нельзя
	// оверлоадить функции и т.д.)
	// компилятор ifort умеет делать совместимые с C статические библиотеки,
	// ну а мы программируем на C++, так что… extern "C"
	// подробнее тут: http://stackoverflow.com/questions/1041866/in-c-source-what-is-the-effect-of-extern-c
	void fortranSquare(size_t*, size_t*);

int main() {
	size_t a, b;
	std::cout << "size of size_t: " << sizeof(a) << '\n';
	std::cout << "enter integer N = "; 
	// 65 536 ^ 2 > UINT_MAX
	// на платформе Win32 size_t занимает 4 байта (как unsigned int), а на x64 — 8
	// поэтому, если ISO_C_BINDING не лжёт, в первом случае переполнение будет, а во втором — нет
	// (спойлер: на моей машине не лжёт)
	std::cin >> a;
	fortranSquare(&a, &b);
	std::cout << "N^2 = " << b << '\n';
	return 0;
}