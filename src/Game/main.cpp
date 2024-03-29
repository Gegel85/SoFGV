#include <iostream>
#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <dbghelp.h>
#include <crtdbg.h>
#endif
#include <sys/stat.h>
#include <SoFGV.hpp>

#ifdef _WIN32
std::string getLastError(int err = GetLastError())
{
	char *s = nullptr;
	std::string str;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&s, 0, nullptr
	);
	str = s;
	LocalFree(s);
	return str;
#else
#define MessageBox(...) ((void)0)
std::string getLastError(int err = errno)
{
	return strerror(err);
#endif
}

#ifdef _WIN32
LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	char buf[MAX_PATH], buf2[MAX_PATH];

	time_t timer;
	char timebuffer[26];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, 26, "%Y%m%d%H%M%S", tm_info);
	sprintf(buf2, "crash_%s.dmp", timebuffer);

	HANDLE hFile = CreateFile(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, 0, 0);

		if (!win)
			sprintf(buf, "Le jeu a un peu crash en fait.\nMiniDumpWriteDump failed.\n%s: %s", getLastError().c_str(), buf2);
		else
			sprintf(buf, "Le jeu a un peu crash en fait.\nMinidump created %s", buf2);
		CloseHandle(hFile);
	} else
		sprintf(buf, "Le jeu a un peu crash en fait.\nCould not create file %s\n%s", buf2, getLastError().c_str());
	SpiralOfFate::game->logger.fatal(buf);
	Utils::dispMsg("Alors...", buf, MB_ICONERROR, &*SpiralOfFate::game->screen);
	exit(EXIT_FAILURE);
}
#endif

std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadPlayerInputs(std::ifstream &stream)
{
	std::map<SpiralOfFate::InputEnum, sf::Keyboard::Key> keyboardMap{
		{ SpiralOfFate::INPUT_LEFT,    sf::Keyboard::Left },
		{ SpiralOfFate::INPUT_RIGHT,   sf::Keyboard::Right },
		{ SpiralOfFate::INPUT_UP,      sf::Keyboard::Up },
		{ SpiralOfFate::INPUT_DOWN,    sf::Keyboard::Down },
		{ SpiralOfFate::INPUT_NEUTRAL, sf::Keyboard::W },
		{ SpiralOfFate::INPUT_MATTER,  sf::Keyboard::X },
		{ SpiralOfFate::INPUT_SPIRIT,  sf::Keyboard::C },
		{ SpiralOfFate::INPUT_VOID,    sf::Keyboard::Q },
		{ SpiralOfFate::INPUT_ASCEND,  sf::Keyboard::S },
		{ SpiralOfFate::INPUT_DASH,    sf::Keyboard::LShift },
		{ SpiralOfFate::INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<SpiralOfFate::InputEnum, std::pair<bool, int>> controllerMap{
		{ SpiralOfFate::INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ SpiralOfFate::INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ SpiralOfFate::INPUT_NEUTRAL, {false, 0} },
		{ SpiralOfFate::INPUT_MATTER,  {false, 2} },
		{ SpiralOfFate::INPUT_SPIRIT,  {false, 1} },
		{ SpiralOfFate::INPUT_VOID,    {false, 3} },
		{ SpiralOfFate::INPUT_ASCEND,  {true,  sf::Joystick::Z | (30 << 3)} },
		{ SpiralOfFate::INPUT_DASH,    {true,  sf::Joystick::Z | ((256 - 30) << 3)} },
		{ SpiralOfFate::INPUT_PAUSE,   {false, 7} }
	};
	std::map<SpiralOfFate::InputEnum, SpiralOfFate::ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, SpiralOfFate::InputEnum> realKeyboardMap;

	if (!stream.fail()) {
		for (auto &pair : keyboardMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		for (auto &pair : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
	for (auto &pair : keyboardMap)
		realKeyboardMap[pair.second] = pair.first;
	for (auto &pair : controllerMap) {
		realControllerMap[pair.first] = pair.second.first ?
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerAxis(
				0,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerButton(
				0,
				pair.second.second
			));
	}
	return {
		std::make_shared<SpiralOfFate::KeyboardInput>(realKeyboardMap),
		std::make_shared<SpiralOfFate::ControllerInput>(realControllerMap)
	};
}

std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> loadMenuInputs(std::ifstream &stream)
{
	std::map<SpiralOfFate::InputEnum, sf::Keyboard::Key> keyboardMap{
		{ SpiralOfFate::INPUT_LEFT,    sf::Keyboard::Left },
		{ SpiralOfFate::INPUT_RIGHT,   sf::Keyboard::Right },
		{ SpiralOfFate::INPUT_UP,      sf::Keyboard::Up },
		{ SpiralOfFate::INPUT_DOWN,    sf::Keyboard::Down },
		{ SpiralOfFate::INPUT_NEUTRAL, sf::Keyboard::W },
		{ SpiralOfFate::INPUT_SPIRIT,  sf::Keyboard::C },
		{ SpiralOfFate::INPUT_PAUSE,   sf::Keyboard::Tab }
	};
	std::map<SpiralOfFate::InputEnum, std::pair<bool, int>> controllerMap{
		{ SpiralOfFate::INPUT_LEFT,    {true,  sf::Joystick::Axis::X | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_RIGHT,   {true,  sf::Joystick::Axis::X | 30 << 3} },
		{ SpiralOfFate::INPUT_UP,      {true,  sf::Joystick::Axis::Y | (256 - 30) << 3} },
		{ SpiralOfFate::INPUT_DOWN,    {true,  sf::Joystick::Axis::Y | 30 << 3} },
		{ SpiralOfFate::INPUT_NEUTRAL, {false, 0} },
		{ SpiralOfFate::INPUT_SPIRIT,  {false, 1} },
		{ SpiralOfFate::INPUT_PAUSE,   {false, 7} }
	};
	std::map<SpiralOfFate::InputEnum, SpiralOfFate::ControllerKey *> realControllerMap;
	std::map<sf::Keyboard::Key, SpiralOfFate::InputEnum> realKeyboardMap;

	if (!stream.fail()) {
		for (auto &pair : keyboardMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
		for (auto &pair : controllerMap)
			stream.read(reinterpret_cast<char *>(&pair.second), sizeof(pair.second));
	}
	for (auto &pair : keyboardMap)
		realKeyboardMap[pair.second] = pair.first;
	for (auto &pair : controllerMap) {
		realControllerMap[pair.first] = pair.second.first ?
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerAxis(
				-1,
				static_cast<sf::Joystick::Axis>(pair.second.second & 7),
				(char)(pair.second.second >> 3)
			)) :
			static_cast<SpiralOfFate::ControllerKey *>(new SpiralOfFate::ControllerButton(
				-1,
				pair.second.second
			));
	}
	return {
		std::make_shared<SpiralOfFate::KeyboardInput>(realKeyboardMap),
		std::make_shared<SpiralOfFate::ControllerInput>(realControllerMap)
	};
}

void	saveSettings()
{
	std::ofstream stream{"settings.dat", std::istream::binary};

	SpiralOfFate::game->P1.first->save(stream);
	SpiralOfFate::game->P1.second->save(stream);
	SpiralOfFate::game->P2.first->save(stream);
	SpiralOfFate::game->P2.second->save(stream);
	SpiralOfFate::game->menu.first->save(stream);
	SpiralOfFate::game->menu.second->save(stream);
}

void	loadSettings()
{
	std::ifstream stream{"settings.dat", std::istream::binary};

	if (stream.fail() && errno != ENOENT)
		Utils::dispMsg("Cannot load settings", "Cannot open settings file: " + std::string(strerror(errno)), MB_ICONERROR);

	struct stat s;
	auto result = stat("settings.dat", &s);

	if (result == -1) {
		if (errno != ENOENT)
			Utils::dispMsg("Cannot load settings", "Cannot stat file: " + std::string(strerror(errno)),
				       MB_ICONERROR);
	} else if (s.st_size != 348)
		Utils::dispMsg("Cannot load settings", "Old settings or corrupted settings detected.\nYou might need to set your settings again in the menu.", MB_ICONWARNING);
	SpiralOfFate::game->P1 = loadPlayerInputs(stream);
	SpiralOfFate::game->P2 = loadPlayerInputs(stream);
	SpiralOfFate::game->menu = loadMenuInputs(stream);
}

void	run()
{
	sf::Event event;
	sf::Image icon;

	loadSettings();
#ifdef _WIN32
	std::string font = getenv("SYSTEMROOT") + std::string("\\Fonts\\comic.ttf");
#else
	std::string font = "/usr/share/fonts/TTF/DejaVuSerif.ttf";
#endif
	if (getenv("BATTLE_FONT"))
		font = getenv("BATTLE_FONT");
	SpiralOfFate::game->font.loadFromFile(font);
	SpiralOfFate::game->screen = std::make_unique<SpiralOfFate::Screen>("Spiral of Fate: Grand Vision | version " VERSION_STR);
	if (icon.loadFromFile("assets/gameIcon.png"))
		SpiralOfFate::game->screen->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	SpiralOfFate::game->screen->setFont(SpiralOfFate::game->font);
	SpiralOfFate::game->scene = std::make_unique<SpiralOfFate::TitleScreen>(SpiralOfFate::game->P1, SpiralOfFate::game->P2);
	while (SpiralOfFate::game->screen->isOpen()) {
		SpiralOfFate::IScene *newScene = SpiralOfFate::game->scene->update();

		SpiralOfFate::game->screen->clear(sf::Color::White);
		SpiralOfFate::game->scene->render();
		SpiralOfFate::game->screen->display();

		while (SpiralOfFate::game->screen->pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				SpiralOfFate::game->screen->close();
			SpiralOfFate::game->scene->consumeEvent(event);
		}
		if (newScene)
			SpiralOfFate::game->scene.reset(newScene);
		SpiralOfFate::game->networkMgr.update();
	}
	saveSettings();
}

int	main()
{
#ifdef _WIN32
	SetUnhandledExceptionFilter(UnhandledExFilter);
#endif

	#ifndef _DEBUG
	try {
	#endif
		new SpiralOfFate::Game();
		SpiralOfFate::game->logger.info("Starting game->");
		run();
		SpiralOfFate::game->logger.info("Goodbye !");
	#ifndef _DEBUG
	} catch (std::exception &e) {
		if (SpiralOfFate::game) {
			SpiralOfFate::game->logger.fatal(e.what());
			Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, &*SpiralOfFate::game->screen);
		} else
			Utils::dispMsg("Fatal error", e.what(), MB_ICONERROR, nullptr);
		delete SpiralOfFate::game;
		return EXIT_FAILURE;
	}
	#endif
	delete SpiralOfFate::game;
	return EXIT_SUCCESS;
}
