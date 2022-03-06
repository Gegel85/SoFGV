//
// Created by Gegel85 on 29/09/2021.
//

#ifndef BATTLE_CHARACTERSELECT_HPP
#define BATTLE_CHARACTERSELECT_HPP


#include <memory>
#include <nlohmann/json.hpp>
#include "IScene.hpp"
#include "../Inputs/IInput.hpp"
#include "../Resources/FrameData.hpp"
#include "../Objects/Character.hpp"

namespace Battle
{
	struct CharacterEntry {
		nlohmann::json entry;
		int pos;
		unsigned _class;
		std::string name;
		std::string framedataPath;
		std::string subobjectDataPath;
		std::vector<std::vector<Color>> palettes;
		std::vector<Sprite> icon;
		std::map<unsigned, std::vector<std::vector<FrameData>>> data;

		CharacterEntry(const nlohmann::json &json);
		CharacterEntry(const CharacterEntry &entry);
		~CharacterEntry();
	};

	class CharacterSelect : public IScene {
	protected:

		sf::Font _font;
		mutable Sprite _randomSprite;
		std::shared_ptr<IInput> _leftInput;
		std::shared_ptr<IInput> _rightInput;
		mutable std::vector<CharacterEntry> _entries;
		int _leftPos = 0;
		int _rightPos = 0;
		int _leftPalette = 0;
		int _rightPalette = 1;
		bool _practice = false;

		Character *_createCharacter(int pos, int palette, std::shared_ptr<IInput> input);

	public:
		static Character *createCharacter(const CharacterEntry &entry, int pos, int palette, std::shared_ptr<IInput> input);
		CharacterSelect(std::shared_ptr<IInput> leftInput, std::shared_ptr<IInput> rightInput, bool practice = false);
		void render() const override;
		IScene *update() override;
		void consumeEvent(const sf::Event &event) override;
	};
}


#endif //BATTLE_CHARACTERSELECT_HPP
