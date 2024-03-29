//
// Created by Gegel85 on 24/09/2021.
//

#ifndef BATTLE_TITLESCREEN_HPP
#define BATTLE_TITLESCREEN_HPP


#include <SFML/Graphics/Font.hpp>
#include <memory>
#include <thread>
#include "IScene.hpp"
#include "../Inputs/KeyboardInput.hpp"
#include "../Inputs/ControllerInput.hpp"

namespace SpiralOfFate
{
	class TitleScreen : public IScene {
	private:
		std::vector<sf::Texture> _inputs;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> _P1;
		std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> _P2;
		std::thread _thread;
		std::map<unsigned, std::map<sf::Joystick::Axis, int>> _oldStickValues;
		std::pair<unsigned, unsigned> _spec;
		unsigned _latestJoystickId = 0;
		unsigned _selectedEntry = 0;
		unsigned _leftInput = 0;
		unsigned _rightInput = 0;
		float _totalPing = 0;
		unsigned _nbPings = 0;
		unsigned _lastPing = 0;
		unsigned _peakPing = 0;
		unsigned _delay = 0;
		unsigned _specCount = 0;
		unsigned _netbellSound;
		bool _connecting = false;
		bool _changeInput = false;
		bool _askingInputs = false;
		bool _chooseSpecCount = false;
		bool _replaySelect = false;
		std::string _basePath;
		std::vector<std::pair<bool, std::string>> _replays;
		unsigned char _changingInputs = 0;
		unsigned char _cursorInputs = 0;
		IScene *_nextScene = nullptr;
		bool _connected = false;
		std::string _remote;
		std::string _oldRemote;

		void _loadReplay(const std::string &path);
		void _fetchReplayList();
		void _onInputsChosen();
		void _host(unsigned spec);
		void _connect();
		bool _onKeyPressed(sf::Event::KeyEvent ev);
		bool _onJoystickMoved(sf::Event::JoystickMoveEvent ev);
		bool _onJoystickPressed(sf::Event::JoystickButtonEvent ev);
		void _showAskInputBox() const;
		void _showHostMessage() const;
		void _showEditKeysMenu() const;
		void _showConnectMessage() const;
		void _showChooseSpecCount() const;
		void _onGoUp();
		void _onGoDown();
		void _onGoLeft();
		void _onGoRight();
		void _onCancel();
		void _onConfirm(unsigned stickId);
		void _onDisconnect(const std::string &address);
		void _onConnect(const std::string &address);
		void _pingUpdate(unsigned ping);
		void _specUpdate(std::pair<unsigned, unsigned> spec);

	public:
		TitleScreen(
			std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P1,
			std::pair<std::shared_ptr<SpiralOfFate::KeyboardInput>, std::shared_ptr<SpiralOfFate::ControllerInput>> P2
		);
		~TitleScreen();
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_TITLESCREEN_HPP
