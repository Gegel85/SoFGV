//
// Created by PinkySmile on 13/09/2022.
//

#include "ClientInGame.hpp"
#include "Resources/Game.hpp"
#include "Scenes/CharacterSelect.hpp"
#include "ClientCharacterSelect.hpp"

namespace SpiralOfFate
{
	ClientInGame::ClientInGame(
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
		_rMachine(leftChr, rightChr),
		_input(std::move(input))
	{
	}

	IScene *ClientInGame::update()
	{
		this->_input->refreshInputs();

		auto status = this->_rMachine.update(true, true);

		if (this->_nextScene)
			return this->_nextScene;
		if (status == RollbackMachine::UPDATESTATUS_NO_INPUTS)
			return nullptr;

		auto linput = game->battleMgr->getLeftCharacter()->getInput();
		auto rinput = game->battleMgr->getRightCharacter()->getInput();

		if (this->_moveList) {
			linput->update();
			rinput->update();
			this->_moveListUpdate((this->_paused == 1 ? linput : rinput)->getInputs());
		} else if (this->_paused)
			this->_pauseUpdate();
		if (!this->_paused) {
			if (status == RollbackMachine::UPDATESTATUS_GAME_ENDED)
				return this->_nextScene;
			if (linput->getInputs().pause == 1)
				this->_paused = 1;
			else if (rinput->getInputs().pause == 1)
				this->_paused = 2;
		}
		return this->_nextScene;
	}

	void ClientInGame::consumeEvent(const sf::Event &event)
	{
		this->_rMachine.consumeEvent(event);
		InGame::consumeEvent(event);
	}
}