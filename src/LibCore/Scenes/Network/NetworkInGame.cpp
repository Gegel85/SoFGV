//
// Created by PinkySmile on 11/11/2022.
//

#include "NetworkInGame.hpp"
#include "Resources/Game.hpp"
#include "Scenes/CharacterSelect.hpp"
#include "Inputs/DelayInput.hpp"

namespace SpiralOfFate
{
	NetworkInGame::NetworkInGame(
		std::shared_ptr<RemoteInput> input,
		const InGame::GameParams &params,
		const std::vector<struct PlatformSkeleton> &platforms,
		const struct StageEntry &stage,
		Character *leftChr,
		Character *rightChr,
		unsigned int licon,
		unsigned int ricon,
		const nlohmann::json &lJson,
		const nlohmann::json &rJson
	) :
		InGame(params, platforms, stage, leftChr, rightChr, licon, ricon, lJson, rJson),
		_leftDInput(reinterpret_cast<DelayInput *>(&*game->battleMgr->getLeftCharacter()->getInput())),
		_rightDInput(reinterpret_cast<DelayInput *>(&*game->battleMgr->getRightCharacter()->getInput())),
		_rMachine(leftChr, rightChr),
		_input(std::move(input))
#ifdef _DEBUG
		,
		_leftChr(leftChr),
		_rightChr(rightChr)
#endif
	{
	}

	IScene *NetworkInGame::update()
	{
		if (this->_nextScene)
			return this->_nextScene;
		this->_input->refreshInputs();

		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		this->_leftDInput->fillBuffer();
		this->_rightDInput->fillBuffer();

		auto status = this->_rMachine.update(true, true);

		if (status == RollbackMachine::UPDATESTATUS_NO_INPUTS)
			return nullptr;
#ifdef _DEBUG
		this->_currentFrame++;
		my_assert(this->_currentFrame <= game->connection->_currentFrame + HARDCODED_CURRENT_DELAY);
#endif

		if (this->_moveList) {
			linput->update();
			rinput->update();
			this->_moveListUpdate((this->_paused == 1 ? linput : rinput)->getInputs());
		} else if (this->_paused)
			this->_pauseUpdate();
		if (status == RollbackMachine::UPDATESTATUS_GAME_ENDED) {
			this->_onGameEnd();
			return this->_nextScene;
		}
		if (!this->_paused) {
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		}
		return this->_nextScene;
	}

	void NetworkInGame::render() const
	{
		InGame::render();
		char buffer[500];

		sprintf(buffer, "Delay %u (%zi:%zi)", this->_leftDInput->getDelay(), this->_leftDInput->getBufferSize(), this->_rightDInput->getBufferSize());
		game->screen->borderColor(2, sf::Color::Black);
		game->screen->fillColor(sf::Color::White);
		game->screen->textSize(20);
		game->screen->displayElement(buffer, {-50, 50}, 145, Screen::ALIGN_LEFT);
		sprintf(buffer, "Rollback %zu/%zu", this->_rMachine.getBufferSize(), this->_rMachine.getMaxBufferSize());
		game->screen->displayElement(buffer, {-50, 75}, 145, Screen::ALIGN_LEFT);
		game->screen->textSize(30);
		game->screen->borderColor(0, sf::Color::Transparent);
#ifdef _DEBUG
		if (this->_displayInputs) {
			game->battleMgr->renderLeftInputs();
			game->battleMgr->renderRightInputs();
		}
#endif
	}

	void NetworkInGame::consumeEvent(const sf::Event &event)
	{
		this->_rMachine.consumeEvent(event);
		InGame::consumeEvent(event);
#ifdef _DEBUG
		if (event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::F2)
				this->_leftChr->showAttributes = this->_rightChr->showAttributes = !this->_rightChr->showAttributes;
			if (event.key.code == sf::Keyboard::F3)
				this->_leftChr->showBoxes = this->_rightChr->showBoxes = !this->_rightChr->showBoxes;
			if (event.key.code == sf::Keyboard::F4)
				this->_displayInputs = !this->_displayInputs;
		}
#endif
	}
}