//
// Created by Gegel85 on 24/09/2021.
//

#include "InGame.hpp"
#include "../Objects/ACharacter.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	InGame::InGame(IInput *leftInput, IInput *rightInput)
	{
		sf::View view{{-50, -500, 1100, 618.75}};

		logger.info("InGame scene created");
		Battle::game.screen->setView(view);
		game.battleMgr = std::make_unique<BattleManager>(
			new ACharacter{
				"assets/characters/test/framedata.json",
				leftInput
			},
			new ACharacter{
				"assets/characters/test/framedata.json",
				rightInput
			}
		);
	}

	void InGame::render() const
	{
		Battle::game.battleMgr->render();
	}

	IScene *InGame::update()
	{
		Battle::game.battleMgr->update();
		return nullptr;
	}

	void InGame::consumeEvent(const sf::Event &event)
	{
		game.battleMgr->consumeEvent(event);
	}
}