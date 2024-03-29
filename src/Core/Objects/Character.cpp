//
// Created by PinkySmile on 18/09/2021
//

#ifdef _WIN32
#include <windows.h>
#endif
#include "Character.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"
#ifndef max
#define max(x, y) (x > y ? x : y)
#endif
#ifndef min
#define min(x, y) (x < y ? x : y)
#endif

#define COMBINATION_LENIENCY 4
#define MAX_FRAME_IN_BUFFER 60

#define WALL_SLAM_HITSTUN_INCREASE 30
#define GROUND_SLAM_HITSTUN_INCREASE 30
#define WALL_SLAM_THRESHOLD 15
#define GROUND_SLAM_THRESHOLD 20

#define SPECIAL_INPUT_BUFFER_PERSIST 10
#define DASH_BUFFER_PERSIST 6
#define HJ_BUFFER_PERSIST 6

#define NORMAL_BUFFER 4
#define HJ_BUFFER 15
#define DASH_BUFFER 15
#define QUARTER_CIRCLE_BUFFER 10
#define DP_BUFFER 15
#define HALF_CIRCLE_BUFFER 20
#define SPIRAL_BUFFER 30
#define CHARGE_PART_BUFFER 10
#define CHARGE_BUFFER 5
#define CHARGE_TIME 25

static const char *oFlags[] = {
	"grab",
	"airUnblockable",
	"unblockable",
	"voidElement",
	"spiritElement",
	"matterElement",
	"lowHit",
	"highHit",
	"autoHitPos",
	"canCounterHit",
	"hitSwitch",
	"cancelable",
	"jab",
	"resetHits",
	"resetOPSpeed",
	"restand",
	"super",
	"ultimate",
	"jumpCancelable",
	"transformCancelable",
	"unTransformCancelable",
	"dashCancelable",
	"backDashCancelable",
	"voidMana",
	"spiritMana",
	"matterMana"
};

static const char *dFlags[] = {
	"invulnerable",
	"invulnerableArmor",
	"superarmor",
	"grabInvulnerable",
	"voidBlock",
	"spiritBlock",
	"matterBlock",
	"neutralBlock",
	"airborne",
	"canBlock",
	"highBlock",
	"lowBlock",
	"karaCancel",
	"resetRotation",
	"counterHit",
	"flash",
	"crouch",
	"projectileInvul",
	"projectile",
	"landCancel",
	"dashCancel",
	"resetSpeed",
	"neutralInvul",
	"matterInvul",
	"spiritInvul",
	"voidInvul"
};

namespace SpiralOfFate
{
	std::function<bool (const Character::LastInput &)> Character::getInputN = [](const Character::LastInput &input) { return input.n; };
	std::function<bool (const Character::LastInput &)> Character::getInputM = [](const Character::LastInput &input) { return input.m; };
	std::function<bool (const Character::LastInput &)> Character::getInputS = [](const Character::LastInput &input) { return input.s; };
	std::function<bool (const Character::LastInput &)> Character::getInputV = [](const Character::LastInput &input) { return input.o; };
	std::function<bool (const Character::LastInput &)> Character::getInputD = [](const Character::LastInput &input) { return input.d; };
	std::function<bool (const Character::LastInput &)> Character::getInputA = [](const Character::LastInput &input) { return input.a; };

	const std::map<CharacterActions, std::string> actionNames{
		{ ACTION_IDLE,                           "Idle" },
		{ ACTION_CROUCHING,                      "Crouching" },
		{ ACTION_CROUCH,                         "Crouch" },
		{ ACTION_STANDING_UP,                    "Standing up" },
		{ ACTION_WALK_FORWARD,                   "Walk forward" },
		{ ACTION_WALK_BACKWARD,                  "Walk backward" },
		{ ACTION_FORWARD_DASH,                   "Forward dash" },
		{ ACTION_BACKWARD_DASH,                  "Backward dash" },
		{ ACTION_NEUTRAL_JUMP,                   "Neutral jump" },
		{ ACTION_FORWARD_JUMP,                   "Forward jump" },
		{ ACTION_BACKWARD_JUMP,                  "Backward jump" },
		{ ACTION_NEUTRAL_HIGH_JUMP,              "Neutral high jump" },
		{ ACTION_FORWARD_HIGH_JUMP,              "Forward high jump" },
		{ ACTION_BACKWARD_HIGH_JUMP,             "Backward high jump" },
		{ ACTION_FALLING,                        "Falling" },
		{ ACTION_LANDING,                        "Landing" },
		{ ACTION_AIR_DASH_1,                     "Up back air dash" },
		{ ACTION_AIR_DASH_2,                     "Up air dash" },
		{ ACTION_AIR_DASH_3,                     "Up forward air dash" },
		{ ACTION_AIR_DASH_4,                     "Back air dash" },
		{ ACTION_AIR_DASH_6,                     "Forward air dash" },
		{ ACTION_AIR_DASH_7,                     "Down back air dash" },
		{ ACTION_AIR_DASH_8,                     "Down air dash" },
		{ ACTION_AIR_DASH_9,                     "Down forward air dash" },
		{ ACTION_AIR_TRANSFORM,                  "Air transform" },
		{ ACTION_GROUND_HIGH_NEUTRAL_BLOCK,      "Ground high neutral block" },
		{ ACTION_GROUND_HIGH_NEUTRAL_PARRY,      "Ground high neutral parry" },
		{ ACTION_GROUND_HIGH_SPIRIT_PARRY,       "Ground high spirit parry" },
		{ ACTION_GROUND_HIGH_MATTER_PARRY,       "Ground high matter parry" },
		{ ACTION_GROUND_HIGH_VOID_PARRY,         "Ground high void parry" },
		{ ACTION_GROUND_HIGH_HIT,                "Ground high hit" },
		{ ACTION_GROUND_LOW_NEUTRAL_BLOCK,       "Ground low neutral block" },
		{ ACTION_GROUND_LOW_NEUTRAL_PARRY,       "Ground low neutral parry" },
		{ ACTION_GROUND_LOW_SPIRIT_PARRY,        "Ground low spirit parry" },
		{ ACTION_GROUND_LOW_MATTER_PARRY,        "Ground low matter parry" },
		{ ACTION_GROUND_LOW_VOID_PARRY,          "Ground low void parry" },
		{ ACTION_GROUND_LOW_HIT,                 "Ground low hit" },
		{ ACTION_AIR_NEUTRAL_BLOCK,              "Air neutral block" },
		{ ACTION_AIR_NEUTRAL_PARRY,              "Air neutral parry" },
		{ ACTION_AIR_SPIRIT_PARRY,               "Air spirit parry" },
		{ ACTION_AIR_MATTER_PARRY,               "Air matter parry" },
		{ ACTION_AIR_VOID_PARRY,                 "Air void parry" },
		{ ACTION_AIR_HIT,                        "Air hit" },
		{ ACTION_BEING_KNOCKED_DOWN,             "Being knocked down" },
		{ ACTION_KNOCKED_DOWN,                   "Knocked down" },
		{ ACTION_NEUTRAL_TECH,                   "Neutral tech" },
		{ ACTION_BACKWARD_TECH,                  "Backward tech" },
		{ ACTION_FORWARD_TECH,                   "Forward tech" },
		{ ACTION_FALLING_TECH,                   "Falling tech" },
		{ ACTION_UP_AIR_TECH,                    "Up air tech" },
		{ ACTION_DOWN_AIR_TECH,                  "Down air tech" },
		{ ACTION_FORWARD_AIR_TECH,               "Forward air tech" },
		{ ACTION_BACKWARD_AIR_TECH,              "Backward air tech" },
		{ ACTION_AIR_TECH_LANDING_LAG,           "Air tech landing lag" },
		{ ACTION_UNTRANSFORM,                    "Untransform" },
		{ ACTION_GROUND_SLAM,                    "Ground slam" },
		{ ACTION_WALL_SLAM,                      "Wall slam" },
		{ ACTION_HARD_LAND,                      "Hard land" },
		{ ACTION_NEUTRAL_AIR_JUMP,               "Neutral air jump" },
		{ ACTION_FORWARD_AIR_JUMP,               "Forward air jump" },
		{ ACTION_BACKWARD_AIR_JUMP,              "Backward air jump" },
		{ ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK,"Ground high neutral wrong block" },
		{ ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK, "Ground low neutral wrong block" },
		{ ACTION_AIR_NEUTRAL_WRONG_BLOCK,        "Air neutral wrong block" },
		{ ACTION_5N,                             "5n" },
		{ ACTION_6N,                             "6n" },
		{ ACTION_8N,                             "8n" },
		{ ACTION_3N,                             "3n" },
		{ ACTION_2N,                             "2n" },
		{ ACTION_214N,                           "214n" },
		{ ACTION_236N,                           "236n" },
		{ ACTION_623N,                           "623n" },
		{ ACTION_41236N,                         "41236n" },
		{ ACTION_63214N,                         "63214n" },
		{ ACTION_6321469874N,                    "6321469874n" },
		{ ACTION_j5N,                            "j5n" },
		{ ACTION_j6N,                            "j6n" },
		{ ACTION_j8N,                            "j8n" },
		{ ACTION_j3N,                            "j3n" },
		{ ACTION_j2N,                            "j2n" },
		{ ACTION_j214N,                          "j214n" },
		{ ACTION_j236N,                          "j236n" },
		{ ACTION_j623N,                          "j623n" },
		{ ACTION_j41236N,                        "j41236n" },
		{ ACTION_j63214N,                        "j63214n" },
		{ ACTION_j6321469874N,                   "j6321469874n" },
		{ ACTION_t5N,                            "t5n" },
		{ ACTION_t6N,                            "t6n" },
		{ ACTION_t8N,                            "t8n" },
		{ ACTION_t2N,                            "t2n" },
		{ ACTION_c28N,                           "[2]8n" },
		{ ACTION_c46N,                           "[4]6n" },
		{ ACTION_c64N,                           "[6]4n" },
		{ ACTION_5M,                             "5m" },
		{ ACTION_6M,                             "6m" },
		{ ACTION_8M,                             "8m" },
		{ ACTION_3M,                             "3m" },
		{ ACTION_2M,                             "2m" },
		{ ACTION_214M,                           "214m" },
		{ ACTION_236M,                           "236m" },
		{ ACTION_623M,                           "623m" },
		{ ACTION_41236M,                         "41236m" },
		{ ACTION_63214M,                         "63214m" },
		{ ACTION_6321469874M,                    "6321469874m" },
		{ ACTION_j5M,                            "j5m" },
		{ ACTION_j6M,                            "j6m" },
		{ ACTION_j8M,                            "j8m" },
		{ ACTION_j3M,                            "j3m" },
		{ ACTION_j2M,                            "j2m" },
		{ ACTION_j214M,                          "j214m" },
		{ ACTION_j236M,                          "j236m" },
		{ ACTION_j623M,                          "j623m" },
		{ ACTION_j41236M,                        "j41236m" },
		{ ACTION_j63214M,                        "j63214m" },
		{ ACTION_j6321469874M,                   "j6321469874m" },
		{ ACTION_t5M,                            "t5m" },
		{ ACTION_t6M,                            "t6m" },
		{ ACTION_t8M,                            "t8m" },
		{ ACTION_t2M,                            "t2m" },
		{ ACTION_c28M,                           "[2]8m" },
		{ ACTION_c46M,                           "[4]6m" },
		{ ACTION_c64M,                           "[6]4m" },
		{ ACTION_5S,                             "5s" },
		{ ACTION_6S,                             "6s" },
		{ ACTION_8S,                             "8s" },
		{ ACTION_3S,                             "3s" },
		{ ACTION_2S,                             "2s" },
		{ ACTION_214S,                           "214s" },
		{ ACTION_236S,                           "236s" },
		{ ACTION_623S,                           "623s" },
		{ ACTION_41236S,                         "41236s" },
		{ ACTION_63214S,                         "63214s" },
		{ ACTION_6321469874S,                    "6321469874s" },
		{ ACTION_j5S,                            "j5s" },
		{ ACTION_j6S,                            "j6s" },
		{ ACTION_j8S,                            "j8s" },
		{ ACTION_j3S,                            "j3s" },
		{ ACTION_j2S,                            "j2s" },
		{ ACTION_j214S,                          "j214s" },
		{ ACTION_j236S,                          "j236s" },
		{ ACTION_j623S,                          "j623s" },
		{ ACTION_j41236S,                        "j41236s" },
		{ ACTION_j63214S,                        "j63214s" },
		{ ACTION_j6321469874S,                   "j6321469874s" },
		{ ACTION_t5S,                            "t5s" },
		{ ACTION_t6S,                            "t6s" },
		{ ACTION_t8S,                            "t8s" },
		{ ACTION_t2S,                            "t2s" },
		{ ACTION_c28S,                           "[2]8s" },
		{ ACTION_c46S,                           "[4]6s" },
		{ ACTION_c64S,                           "[6]4s" },
		{ ACTION_5V,                             "5v" },
		{ ACTION_6V,                             "6v" },
		{ ACTION_8V,                             "8v" },
		{ ACTION_3V,                             "3v" },
		{ ACTION_2V,                             "2v" },
		{ ACTION_214V,                           "214v" },
		{ ACTION_236V,                           "236v" },
		{ ACTION_623V,                           "623v" },
		{ ACTION_41236V,                         "41236v" },
		{ ACTION_63214V,                         "63214v" },
		{ ACTION_6321469874V,                    "6321469874v" },
		{ ACTION_j5V,                            "j5v" },
		{ ACTION_j6V,                            "j6v" },
		{ ACTION_j8V,                            "j8v" },
		{ ACTION_j3V,                            "j3v" },
		{ ACTION_j2V,                            "j2v" },
		{ ACTION_j214V,                          "j214v" },
		{ ACTION_j236V,                          "j236v" },
		{ ACTION_j623V,                          "j623v" },
		{ ACTION_j41236V,                        "j41236v" },
		{ ACTION_j63214V,                        "j63214v" },
		{ ACTION_j6321469874V,                   "j6321469874v" },
		{ ACTION_t5V,                            "t5v" },
		{ ACTION_t6V,                            "t6v" },
		{ ACTION_t8V,                            "t8v" },
		{ ACTION_t2V,                            "t2v" },
		{ ACTION_c28V,                           "[2]8v" },
		{ ACTION_c46V,                           "[4]6v" },
		{ ACTION_c64V,                           "[6]4v" },
		{ ACTION_5A,                             "5a" },
		{ ACTION_6A,                             "6a" },
		{ ACTION_8A,                             "8a" },
		{ ACTION_3A,                             "3a" },
		{ ACTION_2A,                             "2a" },
		{ ACTION_214A,                           "214a" },
		{ ACTION_236A,                           "236a" },
		{ ACTION_421A,                           "421a" },
		{ ACTION_623A,                           "623a" },
		{ ACTION_41236A,                         "41236a" },
		{ ACTION_63214A,                         "63214a" },
		{ ACTION_6321469874A,                    "6321469874a" },
		{ ACTION_j5A,                            "j5a" },
		{ ACTION_j6A,                            "j6a" },
		{ ACTION_j8A,                            "j8a" },
		{ ACTION_j3A,                            "j3a" },
		{ ACTION_j2A,                            "j2a" },
		{ ACTION_j214A,                          "j214a" },
		{ ACTION_j236A,                          "j236a" },
		{ ACTION_j421A,                          "j421a" },
		{ ACTION_j623A,                          "j623a" },
		{ ACTION_j41236A,                        "j41236a" },
		{ ACTION_j63214A,                        "j63214a" },
		{ ACTION_j6321469874A,                   "j6321469874a" },
		{ ACTION_t5A,                            "t5a" },
		{ ACTION_t6A,                            "t6a" },
		{ ACTION_t8A,                            "t8a" },
		{ ACTION_t2A,                            "t2a" },
		{ ACTION_c28A,                           "[2]8a" },
		{ ACTION_c46A,                           "[4]6a" },
		{ ACTION_c64A,                           "[6]4a" },
		{ ACTION_214D,                           "214d"},
		{ ACTION_236D,                           "236d" },
		{ ACTION_421D,                           "421d" },
		{ ACTION_623D,                           "623d" },
		{ ACTION_41236D,                         "41236d" },
		{ ACTION_63214D,                         "63214d" },
		{ ACTION_6321469874D,                    "6321469874d" },
		{ ACTION_j214D,                          "j214d" },
		{ ACTION_j236D,                          "j236d" },
		{ ACTION_j421D,                          "j421d" },
		{ ACTION_j623D,                          "j623d" },
		{ ACTION_j41236D,                        "j41236d" },
		{ ACTION_j63214D,                        "j63214d" },
		{ ACTION_j6321469874D,                   "j6321469874d" },
		{ ACTION_t5D,                            "t5d" },
		{ ACTION_t6D,                            "t6d" },
		{ ACTION_t8D,                            "t8d" },
		{ ACTION_t2D,                            "t2d" },

		{ ACTION_NEUTRAL_OVERDRIVE,              "Neutral overdrive" },
		{ ACTION_MATTER_OVERDRIVE,               "Matter overdrive" },
		{ ACTION_SPIRIT_OVERDRIVE,               "Spirit overdrive" },
		{ ACTION_VOID_OVERDRIVE,                 "Void overdrive" },
		{ ACTION_NEUTRAL_AIR_OVERDRIVE,          "Neutral air overdrive" },
		{ ACTION_MATTER_AIR_OVERDRIVE,           "Matter air overdrive" },
		{ ACTION_SPIRIT_AIR_OVERDRIVE,           "Spirit air overdrive" },
		{ ACTION_VOID_AIR_OVERDRIVE,             "Void air overdrive" },
		{ ACTION_NEUTRAL_ROMAN_CANCEL,           "Neutral roman cancel" },
		{ ACTION_MATTER_ROMAN_CANCEL,            "Matter roman cancel" },
		{ ACTION_SPIRIT_ROMAN_CANCEL,            "Spirit roman cancel" },
		{ ACTION_VOID_ROMAN_CANCEL,              "Void roman cancel" },
		{ ACTION_NEUTRAL_AIR_ROMAN_CANCEL,       "Neutral air roman cancel" },
		{ ACTION_MATTER_AIR_ROMAN_CANCEL,        "Matter air roman cancel" },
		{ ACTION_SPIRIT_AIR_ROMAN_CANCEL,        "Spirit air roman cancel" },
		{ ACTION_VOID_AIR_ROMAN_CANCEL,          "Void air roman cancel" },
		{ ACTION_GROUND_HIGH_REVERSAL,           "Ground high reversal" },
		{ ACTION_GROUND_LOW_REVERSAL,            "Ground low reversal" },
		{ ACTION_AIR_REVERSAL,                   "Air reversal" },

		{ ACTION_WIN_MATCH1,                     "Win match1" },
		{ ACTION_WIN_MATCH2,                     "Win match2" },
		{ ACTION_WIN_MATCH3,                     "Win match3" },
		{ ACTION_WIN_MATCH4,                     "Win match4" },

		//{ ACTION_WIN_ROUND1,                     "Win round1" },
		//{ ACTION_WIN_ROUND2,                     "Win round2" },
		//{ ACTION_WIN_ROUND3,                     "Win round3" },
		//{ ACTION_WIN_ROUND4,                     "Win round4" },
		//{ ACTION_LOOSE_MATCH1,                   "Loose match1" },
		//{ ACTION_LOOSE_MATCH2,                   "Loose match2" },
		//{ ACTION_LOOSE_MATCH3,                   "Loose match3" },
		//{ ACTION_LOOSE_MATCH4,                   "Loose match4" },
		//{ ACTION_LOOSE_ROUND1,                   "Loose round1" },
		//{ ACTION_LOOSE_ROUND2,                   "Loose round2" },
		//{ ACTION_LOOSE_ROUND3,                   "Loose round3" },
		//{ ACTION_LOOSE_ROUND4,                   "Loose round4" },
	};

	static std::string actionToString(int action)
	{
		auto name = actionNames.find(static_cast<SpiralOfFate::CharacterActions>(action));

		return name == SpiralOfFate::actionNames.end() ? "Action #" + std::to_string(action) : name->second;
	}

	Character::Character()
	{
		this->_fakeFrameData.setSlave();
	}

	Character::Character(unsigned index, const std::string &frameData, const std::string &subobjFrameData, const std::pair<std::vector<Color>, std::vector<Color>> &palette, std::shared_ptr<IInput> input) :
		_input(std::move(input)),
		index(index)
	{
		this->_fakeFrameData.setSlave();
		this->_text.setFont(game->font);
		this->_text.setFillColor(sf::Color::White);
		this->_text.setOutlineColor(sf::Color::Black);
		this->_text.setOutlineThickness(2);
		this->_text.setCharacterSize(10);
		this->_text2.setFont(game->font);
		this->_text2.setFillColor(sf::Color::White);
		this->_text2.setOutlineColor(sf::Color::Black);
		this->_text2.setOutlineThickness(2);
		this->_text2.setCharacterSize(10);
		this->_limit.fill(0);
		this->_moves = FrameData::loadFile(frameData, palette);
		this->_subObjectsData = FrameData::loadFile(subobjFrameData, palette);
		this->_lastInputs.push_back(LastInput{0, false, false, false, false, false, false, 0, 0});
	}

	const FrameData *Character::getCurrentFrameData() const
	{
		auto data = Object::getCurrentFrameData();

		if (!this->_grabInvul)
			return data;
		this->_fakeFrameData = *data;
		this->_fakeFrameData.dFlag.grabInvulnerable = true;
		return &this->_fakeFrameData;
	}

	void Character::render() const
	{
		Object::render();
		if (this->showAttributes) {
			game->screen->draw(this->_text);
			game->screen->draw(this->_text2);
		}
		if (this->showBoxes) {
			if (isBlockingAction(this->_action))
				game->screen->displayElement({
					static_cast<int>(this->_position.x - this->_blockStun / 2),
					static_cast<int>(10 - this->_position.y),
					static_cast<int>(this->_blockStun),
					10
				}, sf::Color::White);
			else
				game->screen->displayElement(
					{static_cast<int>(this->_position.x - this->_blockStun / 2), static_cast<int>(10 - this->_position.y), static_cast<int>(this->_blockStun), 10},
					this->_isGrounded() ? sf::Color::Red : sf::Color{0xFF, 0x80, 0x00}
				);
		}
	}

	void Character::update()
	{
		auto limited = this->_limit[0] >= 100 || this->_limit[1] >= 100 || this->_limit[2] >= 100 || this->_limit[3] >= 100;
		auto input = this->_updateInputs();

		if (!this->_ultimateUsed) {
			if (this->_odCooldown) {
				this->_odCooldown--;
				if (!this->_odCooldown)
					game->soundMgr.play(BASICSOUND_OVERDRIVE_RECOVER);
			}
			if (this->_guardCooldown) {
				this->_guardCooldown--;
				if (!this->_guardCooldown)
					game->soundMgr.play(BASICSOUND_GUARD_RECOVER);
				this->_guardRegenCd = 0;
			} else if (this->_guardRegenCd)
				this->_guardRegenCd--;
			else if (this->_guardBar < this->_maxGuardBar)
				this->_guardBar++;
		} else {
			this->_barMaxOdCooldown = this->_maxOdCooldown;
			if (this->_odCooldown > 299 * this->_maxOdCooldown / 300)
				this->_odCooldown = this->_maxOdCooldown;
			else
				this->_odCooldown += this->_maxOdCooldown / 300;
			if (this->_guardCooldown > 299 * this->_maxGuardCooldown / 300)
				this->_guardCooldown = this->_maxGuardCooldown;
			else
				this->_guardCooldown += this->_maxGuardCooldown / 300;
		}

		for (auto &obj : this->_subobjects)
			if (obj.second && obj.second->isDead()) {
				obj.first = 0;
				obj.second.reset();
			}

		this->_tickMove();
		if (!this->_ultimateUsed) {
			this->_matterMana += (this->_matterManaMax - this->_matterMana) * this->_regen;
			this->_spiritMana += (this->_spiritManaMax - this->_spiritMana) * this->_regen;
			this->_voidMana += (this->_voidManaMax - this->_voidMana) * this->_regen;
		}

		if (this->_action == ACTION_FALLING_TECH && this->_isGrounded())
			this->_forceStartMove(ACTION_IDLE);
		if ((this->_action == ACTION_NEUTRAL_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_BACKWARD_TECH) && !this->_isGrounded())
			this->_forceStartMove(ACTION_FALLING_TECH);

		if (
			this->_action == ACTION_BEING_KNOCKED_DOWN ||
			this->_action == ACTION_KNOCKED_DOWN ||
			this->_action == ACTION_NEUTRAL_TECH ||
			this->_action == ACTION_FORWARD_TECH ||
			this->_action == ACTION_BACKWARD_TECH ||
			this->_action == ACTION_FALLING_TECH ||
			this->_action == ACTION_AIR_HIT ||
			this->_action == ACTION_GROUND_SLAM ||
			this->_action == ACTION_WALL_SLAM ||
			this->_action == ACTION_AIR_NEUTRAL_BLOCK ||
			this->_action == ACTION_AIR_NEUTRAL_WRONG_BLOCK ||
			this->_action == ACTION_GROUND_HIGH_NEUTRAL_BLOCK ||
			this->_action == ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK ||
			this->_action == ACTION_GROUND_LOW_NEUTRAL_BLOCK ||
			this->_action == ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK
		)
			this->_grabInvul = 6;
		else if (this->_grabInvul)
			this->_grabInvul--;

		if (this->_blockStun) {
			this->_blockStun--;
			if (this->_blockStun == 0) {
				if (!limited) {
					if (this->_isGrounded()) {
						if (this->_action != ACTION_AIR_HIT && this->_action != ACTION_GROUND_SLAM && this->_action != ACTION_WALL_SLAM)
							this->_forceStartMove(this->getCurrentFrameData()->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE);
					} else if (this->_restand || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT) {
						if (!this->_executeAirTech(input))
							this->_forceStartMove(ACTION_FALLING);
					}
				}
				this->_speedReset = false;
			}
		}

		if (
			(this->_action == ACTION_FORWARD_DASH || this->_action == ACTION_BACKWARD_DASH) &&
			this->_moves.at(this->_action).size() > 1 &&
			this->_actionBlock == 1 && (
			!this->_input->isPressed(
				(this->_direction ? this->_action == ACTION_BACKWARD_DASH : this->_action == ACTION_FORWARD_DASH) ?
				INPUT_LEFT :
				INPUT_RIGHT
			) || !this->_isGrounded()
		)) {
			auto data = this->getCurrentFrameData();

			this->_actionBlock++;
			if (this->_actionBlock == this->_moves.at(this->_action).size())
				//TODO : Create custom exceptions
				throw std::invalid_argument("Action " + actionToString(this->_action) + " is missing block 2");
			Object::_onMoveEnd(*data);
		}

		if (!this->_isGrounded() != this->getCurrentFrameData()->dFlag.airborne && this->getCurrentFrameData()->dFlag.landCancel) {
			if (this->_moves.at(this->_action).size() != this->_actionBlock + 1) {
				this->_actionBlock++;
				this->_animation = 0;
				this->_animationCtr = 0;
				this->_applyNewAnimFlags();
			} else
				this->_forceStartMove(this->_isGrounded() ? ACTION_IDLE : ACTION_FALLING);
		}

		if (this->_isGrounded()) {
			this->_airDashesUsed = 0;
			if (this->_action >= ACTION_UP_AIR_TECH && this->_action <= ACTION_BACKWARD_AIR_TECH)
				this->_forceStartMove(ACTION_AIR_TECH_LANDING_LAG);
			if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
				this->_forceStartMove(ACTION_HARD_LAND);
			else if (
				this->_speed.y <= 0 &&
				this->_action != ACTION_BEING_KNOCKED_DOWN &&
				this->_action != ACTION_KNOCKED_DOWN &&
				this->_action != ACTION_NEUTRAL_TECH &&
				this->_action != ACTION_FORWARD_TECH &&
				this->_action != ACTION_BACKWARD_TECH &&
				this->_action != ACTION_FALLING_TECH && (
					this->_action == ACTION_AIR_HIT ||
					this->_action == ACTION_GROUND_SLAM ||
					this->_action == ACTION_WALL_SLAM ||
					limited ||
					this->_hp <= 0
				)
			) {
				if (!this->_restand) {
					this->_blockStun = 0;
					game->soundMgr.play(BASICSOUND_KNOCKDOWN);
					this->_forceStartMove(ACTION_BEING_KNOCKED_DOWN);
				} else {
					this->_restand = false;
					this->_forceStartMove(ACTION_GROUND_HIGH_HIT);
					this->_actionBlock = 1;
				}
			} else if ((
				this->_action == ACTION_AIR_NEUTRAL_BLOCK ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY ||
				this->_action == ACTION_AIR_MATTER_PARRY ||
				this->_action == ACTION_AIR_VOID_PARRY ||
				this->_action == ACTION_AIR_SPIRIT_PARRY ||
				this->_action == ACTION_AIR_NEUTRAL_WRONG_BLOCK
			)) {
				this->_blockStun = 0;
				this->_forceStartMove(ACTION_IDLE);
			} else if (
				this->_action != ACTION_NEUTRAL_JUMP &&
				this->_action != ACTION_FORWARD_JUMP &&
				this->_action != ACTION_BACKWARD_JUMP &&
				this->_action != ACTION_NEUTRAL_HIGH_JUMP &&
				this->_action != ACTION_FORWARD_HIGH_JUMP &&
				this->_action != ACTION_BACKWARD_HIGH_JUMP
			)
				this->_jumpsUsed = 0;
		}
		if (!this->_blockStun)
			this->_processInput(input);
		else {
			if (this->_isGrounded())
				(this->_specialInputs._421n > 0 && this->_startMove(ACTION_NEUTRAL_OVERDRIVE)) ||
				(this->_specialInputs._421m > 0 && this->_startMove(ACTION_MATTER_OVERDRIVE)) ||
				(this->_specialInputs._421s > 0 && this->_startMove(ACTION_SPIRIT_OVERDRIVE)) ||
				(this->_specialInputs._421v > 0 && this->_startMove(ACTION_VOID_OVERDRIVE));
			else
				(this->_specialInputs._421n > 0 && this->_startMove(ACTION_NEUTRAL_AIR_OVERDRIVE)) ||
				(this->_specialInputs._421m > 0 && this->_startMove(ACTION_MATTER_AIR_OVERDRIVE)) ||
				(this->_specialInputs._421s > 0 && this->_startMove(ACTION_SPIRIT_AIR_OVERDRIVE)) ||
				(this->_specialInputs._421v > 0 && this->_startMove(ACTION_VOID_AIR_OVERDRIVE));
		}

		this->_applyMoveAttributes();
		this->_processGroundSlams();
		this->_calculateCornerPriority();
		this->_processWallSlams();
		if ((
			this->_action == ACTION_IDLE ||
			this->_action == ACTION_CROUCHING ||
			this->_action == ACTION_CROUCH ||
			this->_action == ACTION_STANDING_UP ||
			this->_action == ACTION_WALK_FORWARD ||
			this->_action == ACTION_WALK_BACKWARD ||
			this->_action == ACTION_FALLING
		) && this->_opponent) {
			if (this->_opponent->_position.x - this->_position.x != 0)
				this->_dir = std::copysign(1, this->_opponent->_position.x - this->_position.x);
			this->_direction = this->_dir == 1;
		}
	}

	void Character::init(
		bool side,
		unsigned short maxHp,
		unsigned char maxJumps,
		unsigned char maxAirDash,
		unsigned maxMMana,
		unsigned maxVMana,
		unsigned maxSMana,
		float manaRegen,
		unsigned maxGuardBar,
		unsigned maxGuardCooldown,
		unsigned odCd,
		float groundDrag,
		Vector2f airDrag,
		Vector2f gravity
	)
	{
		this->_dir = side ? 1 : -1;
		this->_direction = side;
		this->_team = !side;
		this->_baseHp = this->_hp = maxHp;
		this->_maxJumps = maxJumps;
		this->_maxAirDashes = maxAirDash;
		this->_baseGravity = this->_gravity = gravity;
		this->_voidManaMax   = maxVMana;
		this->_spiritManaMax = maxSMana;
		this->_matterManaMax = maxMMana;
		this->_voidMana   = maxVMana / 2.f;
		this->_spiritMana = maxSMana / 2.f;
		this->_matterMana = maxMMana / 2.f;
		this->_regen = manaRegen;
		this->_maxGuardCooldown = maxGuardCooldown;
		this->_guardBar = this->_maxGuardBar = maxGuardBar;
		this->_maxOdCooldown = odCd;
		this->_groundDrag = groundDrag;
		this->_airDrag = airDrag;
		if (side) {
			this->_position = {200, 0};
		} else {
			this->_position = {800, 0};
		}
	}

	void Character::consumeEvent(const sf::Event &event)
	{
		this->_input->consumeEvent(event);
	}

	void Character::_processInput(InputStruct input)
	{
		auto data = this->getCurrentFrameData();
		auto airborne =
			(this->_action == ACTION_BACKWARD_AIR_JUMP || this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP) ?
			data->dFlag.airborne :
			!this->_isGrounded();

		if (this->_atkDisabled || this->_inputDisabled) {
			input.n = 0;
			input.m = 0;
			input.v = 0;
			input.s = 0;
			input.a = 0;
			if (this->_inputDisabled) {
				input.horizontalAxis = 0;
				input.verticalAxis = 0;
				input.d = 0;
			}
		}
		if (
			(airborne && this->_executeAirborneMoves(input)) ||
			(!airborne && this->_executeGroundMoves(input))
		)
			return;
		if (this->_isGrounded())
			this->_startMove(this->_action == ACTION_CROUCH ? ACTION_STANDING_UP : ACTION_IDLE);
		else
			this->_startMove(ACTION_FALLING);
	}

	InputStruct Character::updateInputs()
	{
		return this->_updateInputs(false);
	}

	InputStruct Character::_updateInputs(bool tickBuffer)
	{
		this->_input->update();

		InputStruct input = this->_getInputs();

		if (
			!!input.n != this->_lastInputs.front().n ||
			!!input.m != this->_lastInputs.front().m ||
			!!input.s != this->_lastInputs.front().s ||
			!!input.v != this->_lastInputs.front().o ||
			!!input.d != this->_lastInputs.front().d ||
			!!input.a != this->_lastInputs.front().a ||
			std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis) != this->_lastInputs.front().h ||
			std::copysign(!!input.verticalAxis,   this->_dir * input.verticalAxis)   != this->_lastInputs.front().v
		)
			this->_lastInputs.push_front({
				0,
				!!input.n,
				!!input.m,
				!!input.s,
				!!input.v,
				!!input.d,
				!!input.a,
				static_cast<char>(std::copysign(!!input.horizontalAxis, this->_dir * input.horizontalAxis)),
				static_cast<char>(std::copysign(!!input.verticalAxis,   input.verticalAxis))
			});
		this->_lastInputs.front().nbFrames++;
		if (this->_lastInputs.front().nbFrames > MAX_FRAME_IN_BUFFER)
			this->_lastInputs.front().nbFrames = MAX_FRAME_IN_BUFFER;

		if (
			this->_replayData.empty() ||
			!!input.n != this->_replayData.back().n ||
			!!input.m != this->_replayData.back().m ||
			!!input.s != this->_replayData.back().s ||
			!!input.v != this->_replayData.back().v ||
			!!input.d != this->_replayData.back().d ||
			!!input.a != this->_replayData.back().a ||
			std::copysign(!!input.horizontalAxis, input.horizontalAxis) != this->_replayData.back()._h ||
			std::copysign(!!input.verticalAxis,   input.verticalAxis)   != this->_replayData.back()._v ||
			this->_replayData.back().time == 63
		)
			this->_replayData.push_back({
				!!input.n,
				!!input.m,
				!!input.v,
				!!input.s,
				!!input.a,
				!!input.d,
				static_cast<char>(std::copysign(!!input.horizontalAxis, input.horizontalAxis)),
				static_cast<char>(std::copysign(!!input.verticalAxis,   input.verticalAxis)),
				0
			});
		else
			this->_replayData.back().time++;

		this->_checkSpecialInputs(tickBuffer);
		this->_hasJumped &= input.verticalAxis > 0;
		this->_inputBuffer.horizontalAxis = input.horizontalAxis;
		this->_inputBuffer.verticalAxis = input.verticalAxis;
		for (unsigned i = 2; i < sizeof(this->_inputBuffer) / sizeof(int); i++) {
			if (((int *)&this->_inputBuffer)[i])
				((int *)&this->_inputBuffer)[i] -= tickBuffer;
			else if (std::abs(((int *)&input)[i]) == 1)
				((int *)&this->_inputBuffer)[i] = NORMAL_BUFFER;
		}
		return this->_inputBuffer;
	}

	bool Character::_executeAirborneMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_j5N));
		        this->_executeAirTech(input) ||

		        ((this->_specialInputs._624684n  || this->_specialInputs._6314684n)  && this->_startMove(ACTION_j6321469874N)) ||
		        ((this->_specialInputs._6246974n || this->_specialInputs._63146974n) && this->_startMove(ACTION_j6321469874N)) ||
		        ((this->_specialInputs._624684v  || this->_specialInputs._6314684v)  && this->_startMove(ACTION_j6321469874V)) ||
		        ((this->_specialInputs._6246974v || this->_specialInputs._63146974v) && this->_startMove(ACTION_j6321469874V)) ||
		        ((this->_specialInputs._624684s  || this->_specialInputs._6314684s)  && this->_startMove(ACTION_j6321469874S)) ||
		        ((this->_specialInputs._6246974s || this->_specialInputs._63146974s) && this->_startMove(ACTION_j6321469874S)) ||
		        ((this->_specialInputs._624684m  || this->_specialInputs._6314684m)  && this->_startMove(ACTION_j6321469874M)) ||
		        ((this->_specialInputs._6246974m || this->_specialInputs._63146974m) && this->_startMove(ACTION_j6321469874M)) ||
			((this->_specialInputs._624684d  || this->_specialInputs._6314684d)  && this->_startMove(ACTION_j6321469874D)) ||
			((this->_specialInputs._6246974d || this->_specialInputs._63146974d) && this->_startMove(ACTION_j6321469874D)) ||
			((this->_specialInputs._624684a  || this->_specialInputs._6314684a)  && this->_startMove(ACTION_j6321469874A)) ||
			((this->_specialInputs._6246974a || this->_specialInputs._63146974a) && this->_startMove(ACTION_j6321469874A)) ||

		        ((this->_specialInputs._624n || this->_specialInputs._6314n) && this->_startMove(ACTION_j63214N)) ||
		        ((this->_specialInputs._426n || this->_specialInputs._4136n) && this->_startMove(ACTION_j41236N)) ||
		        ((this->_specialInputs._624v || this->_specialInputs._6314v) && this->_startMove(ACTION_j63214V)) ||
		        ((this->_specialInputs._426v || this->_specialInputs._4136v) && this->_startMove(ACTION_j41236V)) ||
		        ((this->_specialInputs._624s || this->_specialInputs._6314s) && this->_startMove(ACTION_j63214S)) ||
		        ((this->_specialInputs._426s || this->_specialInputs._4136s) && this->_startMove(ACTION_j41236S)) ||
		        ((this->_specialInputs._624m || this->_specialInputs._6314m) && this->_startMove(ACTION_j63214M)) ||
		        ((this->_specialInputs._426m || this->_specialInputs._4136m) && this->_startMove(ACTION_j41236M)) ||
			((this->_specialInputs._624d || this->_specialInputs._6314d) && this->_startMove(ACTION_j63214D)) ||
			((this->_specialInputs._426d || this->_specialInputs._4136d) && this->_startMove(ACTION_j41236D)) ||
			((this->_specialInputs._624a || this->_specialInputs._6314a) && this->_startMove(ACTION_j63214A)) ||
			((this->_specialInputs._426a || this->_specialInputs._4136a) && this->_startMove(ACTION_j41236A)) ||

		        (this->_specialInputs._623n     && this->_startMove(ACTION_j623N)) ||
		        (this->_specialInputs._421n > 0 && this->_startMove(ACTION_NEUTRAL_AIR_ROMAN_CANCEL)) ||
		        (this->_specialInputs._623v     && this->_startMove(ACTION_j623V)) ||
		        (this->_specialInputs._421v > 0 && this->_startMove(ACTION_VOID_AIR_ROMAN_CANCEL)) ||
		        (this->_specialInputs._623s     && this->_startMove(ACTION_j623S)) ||
		        (this->_specialInputs._421s > 0 && this->_startMove(ACTION_SPIRIT_AIR_ROMAN_CANCEL)) ||
		        (this->_specialInputs._623m     && this->_startMove(ACTION_j623M)) ||
		        (this->_specialInputs._421m > 0 && this->_startMove(ACTION_MATTER_AIR_ROMAN_CANCEL)) ||
			(this->_specialInputs._623d     && this->_startMove(ACTION_j623D)) ||
			(this->_specialInputs._421d     && this->_startMove(ACTION_j421D)) ||
			(this->_specialInputs._623a     && this->_startMove(ACTION_j623A)) ||
			(this->_specialInputs._421a     && this->_startMove(ACTION_j421A)) ||

		        (this->_specialInputs._236n && this->_startMove(ACTION_j236N)) ||
		        (this->_specialInputs._214n && this->_startMove(ACTION_j214N)) ||
		        (this->_specialInputs._236v && this->_startMove(ACTION_j236V)) ||
		        (this->_specialInputs._214v && this->_startMove(ACTION_j214V)) ||
		        (this->_specialInputs._236s && this->_startMove(ACTION_j236S)) ||
		        (this->_specialInputs._214s && this->_startMove(ACTION_j214S)) ||
		        (this->_specialInputs._236m && this->_startMove(ACTION_j236M)) ||
		        (this->_specialInputs._214m && this->_startMove(ACTION_j214M)) ||
			(this->_specialInputs._236d && this->_startMove(ACTION_j236D)) ||
			(this->_specialInputs._214d && this->_startMove(ACTION_j214D)) ||
			(this->_specialInputs._236a && this->_startMove(ACTION_j236A)) ||
			(this->_specialInputs._214a && this->_startMove(ACTION_j214A)) ||

		        this->_executeAirParry(input) ||

		        (input.n && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8N)) ||
		        (input.n && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3N)) ||
		        (input.n &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6N)) ||
		        (input.n && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j2N)) ||
		        (input.n &&                                                                      this->_executeNeutralAttack(ACTION_j5N)) ||
		        (input.v && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8V)) ||
		        (input.v && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3V)) ||
		        (input.v &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6V)) ||
		        (input.v && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j2V)) ||
		        (input.v &&                                                                      this->_executeNeutralAttack(ACTION_j5V)) ||
		        (input.s && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8S)) ||
		        (input.s && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3S)) ||
		        (input.s &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6S)) ||
		        (input.s && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j2S)) ||
		        (input.s &&                                                                      this->_executeNeutralAttack(ACTION_j5S)) ||
		        (input.m && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8M)) ||
		        (input.m && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3M)) ||
		        (input.m &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6M)) ||
		        (input.m && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_j2M)) ||
		        (input.m &&                                                                      this->_executeNeutralAttack(ACTION_j5M)) ||
			(input.a && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_j8A)) ||
			(input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j3A)) ||
			(input.a &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_j6A)) ||
			(input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis == 0 &&  this->_executeDownAttack(ACTION_j2A)) ||
			(input.a &&                           this->_dir * input.horizontalAxis == 0 &&  this->_executeNeutralAttack(ACTION_j5A)) ||
			this->_executeAirDashes(input) ||
		        this->_executeAirJump(input);
	}

	bool Character::_executeGroundMoves(const InputStruct &input)
	{
		return  //(input.n && input.n <= 4 && this->_startMove(ACTION_5N)) ||

			((this->_specialInputs._624684n  || this->_specialInputs._6314684n)  && this->_startMove(ACTION_6321469874N)) ||
			((this->_specialInputs._6246974n || this->_specialInputs._63146974n) && this->_startMove(ACTION_6321469874N)) ||
			((this->_specialInputs._624684v  || this->_specialInputs._6314684v)  && this->_startMove(ACTION_6321469874V)) ||
			((this->_specialInputs._6246974v || this->_specialInputs._63146974v) && this->_startMove(ACTION_6321469874V)) ||
			((this->_specialInputs._624684s  || this->_specialInputs._6314684s)  && this->_startMove(ACTION_6321469874S)) ||
			((this->_specialInputs._6246974s || this->_specialInputs._63146974s) && this->_startMove(ACTION_6321469874S)) ||
			((this->_specialInputs._624684m  || this->_specialInputs._6314684m)  && this->_startMove(ACTION_6321469874M)) ||
			((this->_specialInputs._6246974m || this->_specialInputs._63146974m) && this->_startMove(ACTION_6321469874M)) ||
			((this->_specialInputs._624684d  || this->_specialInputs._6314684d)  && this->_startMove(ACTION_6321469874D)) ||
			((this->_specialInputs._6246974d || this->_specialInputs._63146974d) && this->_startMove(ACTION_6321469874D)) ||
			((this->_specialInputs._624684a  || this->_specialInputs._6314684a)  && this->_startMove(ACTION_6321469874A)) ||
			((this->_specialInputs._6246974a || this->_specialInputs._63146974a) && this->_startMove(ACTION_6321469874A)) ||

			((this->_specialInputs._624n || this->_specialInputs._6314n) && this->_startMove(ACTION_63214N)) ||
			((this->_specialInputs._426n || this->_specialInputs._4136n) && this->_startMove(ACTION_41236N)) ||
			((this->_specialInputs._624v || this->_specialInputs._6314v) && this->_startMove(ACTION_63214V)) ||
			((this->_specialInputs._426v || this->_specialInputs._4136v) && this->_startMove(ACTION_41236V)) ||
			((this->_specialInputs._624s || this->_specialInputs._6314s) && this->_startMove(ACTION_63214S)) ||
			((this->_specialInputs._426s || this->_specialInputs._4136s) && this->_startMove(ACTION_41236S)) ||
			((this->_specialInputs._624m || this->_specialInputs._6314m) && this->_startMove(ACTION_63214M)) ||
			((this->_specialInputs._426m || this->_specialInputs._4136m) && this->_startMove(ACTION_41236M)) ||
			((this->_specialInputs._624d || this->_specialInputs._6314d) && this->_startMove(ACTION_63214D)) ||
			((this->_specialInputs._426d || this->_specialInputs._4136d) && this->_startMove(ACTION_41236D)) ||
			((this->_specialInputs._624a || this->_specialInputs._6314a) && this->_startMove(ACTION_63214A)) ||
			((this->_specialInputs._426a || this->_specialInputs._4136a) && this->_startMove(ACTION_41236A)) ||

			(this->_specialInputs._c28n && this->_startMove(ACTION_c28N)) ||
			(this->_specialInputs._c46n && this->_startMove(ACTION_c46N)) ||
			(this->_specialInputs._c64n && this->_startMove(ACTION_c64N)) ||
			(this->_specialInputs._c28v && this->_startMove(ACTION_c28V)) ||
			(this->_specialInputs._c46v && this->_startMove(ACTION_c46V)) ||
			(this->_specialInputs._c64v && this->_startMove(ACTION_c64V)) ||
			(this->_specialInputs._c28s && this->_startMove(ACTION_c28S)) ||
			(this->_specialInputs._c46s && this->_startMove(ACTION_c46S)) ||
			(this->_specialInputs._c64s && this->_startMove(ACTION_c64S)) ||
			(this->_specialInputs._c28m && this->_startMove(ACTION_c28M)) ||
			(this->_specialInputs._c46m && this->_startMove(ACTION_c46M)) ||
			(this->_specialInputs._c64m && this->_startMove(ACTION_c64M)) ||
			(this->_specialInputs._c46a && this->_startMove(ACTION_c46A)) ||
			(this->_specialInputs._c64a && this->_startMove(ACTION_c64A)) ||

			(this->_specialInputs._623n     && this->_startMove(ACTION_623N)) ||
			(this->_specialInputs._421n > 0 && this->_startMove(ACTION_NEUTRAL_ROMAN_CANCEL)) ||
			(this->_specialInputs._623v     && this->_startMove(ACTION_623V)) ||
			(this->_specialInputs._421v > 0 && this->_startMove(ACTION_VOID_ROMAN_CANCEL)) ||
			(this->_specialInputs._623s     && this->_startMove(ACTION_623S)) ||
			(this->_specialInputs._421s > 0 && this->_startMove(ACTION_SPIRIT_ROMAN_CANCEL)) ||
			(this->_specialInputs._623m     && this->_startMove(ACTION_623M)) ||
			(this->_specialInputs._421m > 0 && this->_startMove(ACTION_MATTER_ROMAN_CANCEL)) ||
			(this->_specialInputs._623d     && this->_startMove(ACTION_623D)) ||
			(this->_specialInputs._421d     && this->_startMove(ACTION_421D)) ||
			(this->_specialInputs._623a     && this->_startMove(ACTION_623A)) ||
			(this->_specialInputs._421a     && this->_startMove(ACTION_421A)) ||

			(this->_specialInputs._236n && this->_startMove(ACTION_236N)) ||
			(this->_specialInputs._214n && this->_startMove(ACTION_214N)) ||
			(this->_specialInputs._236v && this->_startMove(ACTION_236V)) ||
			(this->_specialInputs._214v && this->_startMove(ACTION_214V)) ||
			(this->_specialInputs._236s && this->_startMove(ACTION_236S)) ||
			(this->_specialInputs._214s && this->_startMove(ACTION_214S)) ||
			(this->_specialInputs._236m && this->_startMove(ACTION_236M)) ||
			(this->_specialInputs._214m && this->_startMove(ACTION_214M)) ||
			(this->_specialInputs._236d && this->_startMove(ACTION_236D)) ||
			(this->_specialInputs._214d && this->_startMove(ACTION_214D)) ||
			(this->_specialInputs._236a && this->_startMove(ACTION_236A)) ||
			(this->_specialInputs._214a && this->_startMove(ACTION_214A)) ||

			this->_executeGroundParry(input) ||

			(input.n && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8N)) ||
			(input.n && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3N)) ||
			(input.n &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6N)) ||
			(input.n && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_2N)) ||
			(input.n &&                                                                      this->_executeNeutralAttack(ACTION_5N)) ||
			(input.v && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8V)) ||
			(input.v && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3V)) ||
			(input.v &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6V)) ||
			(input.v && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_2V)) ||
			(input.v &&                                                                      this->_executeNeutralAttack(ACTION_5V)) ||
			(input.s && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8S)) ||
			(input.s && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3S)) ||
			(input.s &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6S)) ||
			(input.s && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_2S)) ||
			(input.s &&                                                                      this->_executeNeutralAttack(ACTION_5S)) ||
			(input.m && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8M)) ||
			(input.m && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3M)) ||
			(input.m &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6M)) ||
			(input.m && input.verticalAxis < 0 &&                                            this->_executeDownAttack(ACTION_2M)) ||
			(input.m &&                                                                      this->_executeNeutralAttack(ACTION_5M)) ||
			(input.a && input.verticalAxis > 0 &&                                            this->_startMove(ACTION_8A)) ||
			(input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_3A)) ||
			(input.a &&                           this->_dir * input.horizontalAxis > 0 &&   this->_startMove(ACTION_6A)) ||
			(input.a && input.verticalAxis < 0 && this->_dir * input.horizontalAxis == 0 &&  this->_executeDownAttack(ACTION_2A)) ||
			(input.a &&                           this->_dir * input.horizontalAxis == 0 &&  this->_executeNeutralAttack(ACTION_5A)) ||
		        this->_executeGroundJump(input)   ||
		        this->_executeGroundDashes(input) ||
		        this->_executeCrouch(input)       ||
		        this->_executeWalking(input);
	}

	bool Character::_canStartMove(unsigned action, const FrameData &data)
	{
		if (this->_jumpCanceled && (
			this->_action == ACTION_NEUTRAL_JUMP ||
			this->_action == ACTION_FORWARD_JUMP ||
			this->_action == ACTION_BACKWARD_JUMP ||
			this->_action == ACTION_NEUTRAL_HIGH_JUMP ||
			this->_action == ACTION_FORWARD_HIGH_JUMP ||
			this->_action == ACTION_BACKWARD_HIGH_JUMP ||
			this->_action == ACTION_NEUTRAL_AIR_JUMP ||
			this->_action == ACTION_FORWARD_AIR_JUMP ||
			this->_action == ACTION_BACKWARD_AIR_JUMP
		))
			return false;
		if (isOverdriveAction(action)) {
			for (auto limit : this->_limit)
				if (limit >= 100)
					return false;
			return !this->_odCooldown;
		}
		if (isRomanCancelAction(action))
			return !this->_odCooldown && this->_action >= ACTION_5N && !isParryAction(this->_action);
		if (this->_hp <= 0 && this->_action == ACTION_KNOCKED_DOWN)
			return false;
		if (data.subObjectSpawn < 0 && data.subObjectSpawn >= -128 && this->_subobjects[-data.subObjectSpawn - 1].first)
			return false;
		if (data.oFlag.matterMana && this->_matterMana < data.manaCost)
			return false;
		if (data.oFlag.voidMana && this->_voidMana < data.manaCost)
			return false;
		if (data.oFlag.spiritMana && this->_spiritMana < data.manaCost)
			return false;
		if (action == ACTION_UP_AIR_TECH || action == ACTION_DOWN_AIR_TECH || action == ACTION_FORWARD_AIR_TECH || action == ACTION_BACKWARD_AIR_TECH) {
			for (auto limit : this->_limit)
				if (limit >= 100)
					return false;
			return (this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_SLAM || this->_action == ACTION_WALL_SLAM) && this->_blockStun == 0;
		}
		if (action == ACTION_NEUTRAL_TECH || action == ACTION_BACKWARD_TECH || action == ACTION_FORWARD_TECH)
			return this->_isGrounded() && this->_action == ACTION_KNOCKED_DOWN;
		if (action == ACTION_FALLING_TECH)
			return !this->_isGrounded() && (this->_action == ACTION_NEUTRAL_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_BACKWARD_TECH);
		if (action == ACTION_IDLE && this->_action == ACTION_STANDING_UP)
			return false;
		if (action == ACTION_CROUCHING && this->_action == ACTION_CROUCH)
			return false;
		if (this->_canCancel(action))
			return true;
		if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9)
			return this->_airDashesUsed < this->_maxAirDashes && this->_action == ACTION_FALLING;
		if ((action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP) || (action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP))
			return this->_jumpsUsed < this->_maxJumps && (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action == action)
			return false;
		if (isBlockingAction(action))
			return this->getCurrentFrameData()->dFlag.canBlock;
		if (isParryAction(action) && this->getCurrentFrameData()->dFlag.canBlock)
			return !this->_guardCooldown && !this->_blockStun && !isParryAction(this->_action);
		if (isBlockingAction(this->_action))
			return !this->_blockStun;
		if (action <= ACTION_WALK_BACKWARD || action == ACTION_FALLING || action == ACTION_LANDING)
			return (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING);
		if (this->_action <= ACTION_WALK_BACKWARD || this->_action == ACTION_FALLING || this->_action == ACTION_LANDING)
			return true;
		return false;
	}

	void Character::_onMoveEnd(const FrameData &lastData)
	{
		game->logger.debug(std::to_string(this->_action) + " ended");
		if (this->_action == ACTION_BEING_KNOCKED_DOWN) {
			this->_blockStun = 0;
			return this->_forceStartMove(ACTION_KNOCKED_DOWN);
		}

		if ((this->_action == ACTION_FORWARD_DASH || this->_action == ACTION_BACKWARD_DASH) && this->_moves.at(this->_action).size() > 1) {
			if (this->_actionBlock == 0)
				this->_actionBlock++;
			if (this->_actionBlock == 1)
				return Object::_onMoveEnd(lastData);
		}

		if ((this->_action == ACTION_FALLING_TECH || this->_blockStun) && !this->_actionBlock) {
			this->_actionBlock++;
			if (this->_moves.at(this->_action).size() == 1)
				//TODO: make proper exceptions
				throw std::invalid_argument("Action " + actionToString(this->_action) + " is missing block 1");
			Object::_onMoveEnd(lastData);
			return;
		}

		if (this->_action == ACTION_KNOCKED_DOWN) {
			if (this->_hp <= 0)
				return Object::_onMoveEnd(lastData);

			auto inputs = this->_getInputs();

			switch (this->_dummyGroundTech) {
			case GROUNDTECH_NONE:
				break;
			case GROUNDTECH_FORWARD:
				if (this->_startMove(ACTION_FORWARD_TECH))
					return;
				break;
			case GROUNDTECH_BACKWARD:
				if (this->_startMove(ACTION_BACKWARD_TECH))
					return;
				break;
			case GROUNDTECH_RANDOM:
				switch (game->random() % 3) {
				case 0:
					break;
				case 1:
					if (this->_startMove(ACTION_FORWARD_TECH))
						return;
					break;
				case 2:
					if (this->_startMove(ACTION_BACKWARD_TECH))
						return;
					break;
				}
				break;
			}
			if (this->_atkDisabled || this->_inputDisabled || (!inputs.a && !inputs.s && !inputs.d && !inputs.m && !inputs.n && !inputs.v) || !inputs.horizontalAxis)
				return this->_forceStartMove(ACTION_NEUTRAL_TECH);
			if (this->_startMove(inputs.horizontalAxis * this->_dir < 0 ? ACTION_BACKWARD_TECH : ACTION_FORWARD_TECH))
				return;
			return this->_forceStartMove(ACTION_NEUTRAL_TECH);
		}

		if (this->_action == ACTION_CROUCHING)
			return this->_forceStartMove(ACTION_CROUCH);

		auto idleAction = this->_isGrounded() ? (lastData.dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING;

		if (this->_action == ACTION_BACKWARD_AIR_TECH || this->_action == ACTION_FORWARD_AIR_TECH || this->_action == ACTION_UP_AIR_TECH || this->_action == ACTION_DOWN_AIR_TECH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_AIR_REVERSAL || this->_action == ACTION_GROUND_HIGH_REVERSAL || this->_action == ACTION_GROUND_LOW_REVERSAL)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_LANDING)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_BACKWARD_TECH || this->_action == ACTION_FORWARD_TECH || this->_action == ACTION_NEUTRAL_TECH)
			return this->_forceStartMove(idleAction);
		if (isParryAction(this->_action))
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_STANDING_UP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_AIR_TECH_LANDING_LAG)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_FORWARD_DASH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_BACKWARD_DASH)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_HARD_LAND)
			return this->_forceStartMove(idleAction);
		if (
			this->_action >= ACTION_5N ||
			this->_action == ACTION_LANDING
		)
			return this->_forceStartMove(idleAction);

		if (this->_action >= ACTION_AIR_DASH_1 && this->_action <= ACTION_AIR_DASH_9)
			return this->_forceStartMove(this->_isGrounded() ? ACTION_HARD_LAND : ACTION_FALLING);
		if (this->_action == ACTION_NEUTRAL_JUMP || this->_action == ACTION_FORWARD_JUMP || this->_action == ACTION_BACKWARD_JUMP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_NEUTRAL_AIR_JUMP || this->_action == ACTION_FORWARD_AIR_JUMP || this->_action == ACTION_BACKWARD_AIR_JUMP)
			return this->_forceStartMove(idleAction);
		if (this->_action == ACTION_NEUTRAL_HIGH_JUMP || this->_action == ACTION_FORWARD_HIGH_JUMP || this->_action == ACTION_BACKWARD_HIGH_JUMP)
			return this->_forceStartMove(idleAction);
		Object::_onMoveEnd(lastData);
	}

	void Character::hit(IObject &other, const FrameData *data)
	{
		Object::hit(other, data);
		this->_speed.x += data->pushBack * -this->_dir;
		if (data->oFlag.grab) {
			this->_actionBlock++;
			this->_animationCtr = 0;
			if (this->_moves.at(this->_action).size() <= this->_actionBlock)
				//TODO: make proper exceptions
				throw std::invalid_argument("Grab action " + actionToString(this->_action) + " is missing block " + std::to_string(this->_actionBlock));
			Object::_onMoveEnd(*data);
		}
	}

	void Character::getHit(IObject &other, const FrameData *data)
	{
		auto myData = this->getCurrentFrameData();
		char buffer[38];
		auto chr = dynamic_cast<Character *>(&other);

		sprintf(buffer, "0x%08llX is hit by 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		game->logger.debug(buffer);
		if (!data)
			return;
		if (chr) {
			if (data->oFlag.voidMana)
				chr->_voidMana += data->manaGain;
			if (data->oFlag.spiritMana)
				chr->_spiritMana += data->manaGain;
			if (data->oFlag.matterMana)
				chr->_matterMana += data->manaGain;
		}
		if (myData->dFlag.invulnerableArmor)
			return game->battleMgr->addHitStop(data->hitStop);
		this->_restand = data->oFlag.restand;
		if (
			this->_isBlocking() &&
			(!myData->dFlag.airborne || !data->oFlag.airUnblockable) &&
			!data->oFlag.unblockable &&
			!data->oFlag.grab
		)
			this->_blockMove(dynamic_cast<Object *>(&other), *data);
		else
			this->_getHitByMove(dynamic_cast<Object *>(&other), *data);
	}

	bool Character::_isBlocking() const
	{
		auto *data = this->getCurrentFrameData();

		if (this->_input->isPressed(this->_direction ? INPUT_LEFT : INPUT_RIGHT) && data->dFlag.canBlock)
			return true;
		if ((this->_forceBlock & 7) && ((this->_forceBlock & RANDOM_BLOCK) == 0 || game->random() % 8 != 0) && data->dFlag.canBlock)
			return true;
		return data->dFlag.neutralBlock || data->dFlag.spiritBlock || data->dFlag.matterBlock || data->dFlag.voidBlock;
	}

	void Character::_forceStartMove(unsigned int action)
	{
		auto anim = this->_moves.at(this->_action)[this->_actionBlock].size() == this->_animation ? this->_animation - 1 : this->_animation;

		this->_jumpCanceled = this->_moves.at(this->_action)[this->_actionBlock][anim].oFlag.jumpCancelable && (
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_HIGH_JUMP ||
			action == ACTION_FORWARD_HIGH_JUMP ||
			action == ACTION_BACKWARD_HIGH_JUMP ||
			action == ACTION_NEUTRAL_AIR_JUMP ||
			action == ACTION_FORWARD_AIR_JUMP ||
			action == ACTION_BACKWARD_AIR_JUMP
		);
		this->_opponent->_supersUsed += this->getAttackTier(action) >= 700;
		this->_opponent->_skillsUsed += this->getAttackTier(action) >= 400 && this->getAttackTier(action) < 600;
		game->logger.info("Starting action " + actionToString(action));
		if (isParryAction(action)) {
			unsigned loss = ((action == ACTION_AIR_NEUTRAL_PARRY || action == ACTION_GROUND_HIGH_NEUTRAL_PARRY || action == ACTION_GROUND_LOW_NEUTRAL_PARRY) + 1) * 60;

			this->_specialInputs._421n = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421m = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421s = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421v = -SPECIAL_INPUT_BUFFER_PERSIST;
			game->soundMgr.play(BASICSOUND_PARRY);
			this->_guardRegenCd = 120;
			if (this->_guardBar > loss)
				this->_guardBar -= loss;
			else {
				this->_guardBar = 0;
				this->_guardCooldown = this->_maxGuardCooldown;
				game->soundMgr.play(BASICSOUND_GUARD_BREAK);
			}
		}
		if (
			action == ACTION_IDLE ||
			action == ACTION_WALK_FORWARD ||
			action == ACTION_WALK_BACKWARD ||
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_HIGH_JUMP ||
			action == ACTION_FORWARD_HIGH_JUMP ||
			action == ACTION_BACKWARD_HIGH_JUMP
		) {
			if (this->_moves.at(this->_action)[this->_actionBlock][anim].dFlag.airborne) {
				game->soundMgr.play(BASICSOUND_LAND);
				if (action == ACTION_IDLE)
					return this->_forceStartMove(ACTION_LANDING);
			}
		}
		if (isOverdriveAction(action) || isRomanCancelAction(action)) {
			auto currentCd = this->_maxOdCooldown;

			if (isRomanCancelAction(action))
				currentCd /= 2;
			if (action == ACTION_NEUTRAL_OVERDRIVE || action == ACTION_NEUTRAL_ROMAN_CANCEL) {
				currentCd *= 3;
				currentCd /= 4;
			}
			this->_blockStun = 0;
			this->_odCooldown = this->_barMaxOdCooldown = currentCd;
		} else if (
			action == ACTION_NEUTRAL_JUMP ||
			action == ACTION_FORWARD_JUMP ||
			action == ACTION_BACKWARD_JUMP ||
			action == ACTION_NEUTRAL_AIR_JUMP ||
			action == ACTION_FORWARD_AIR_JUMP ||
			action == ACTION_BACKWARD_AIR_JUMP
		) {
			this->_jumpsUsed++;
			this->_hasJumped = true;
		} else if (action == ACTION_NEUTRAL_HIGH_JUMP || action == ACTION_FORWARD_HIGH_JUMP || action == ACTION_BACKWARD_HIGH_JUMP) {
			this->_jumpsUsed += 2;
			this->_hasJumped = true;
		} else if (action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9) {
			this->_airDashesUsed++;
			if (
				this->_action == ACTION_NEUTRAL_JUMP ||
				this->_action == ACTION_FORWARD_JUMP ||
				this->_action == ACTION_BACKWARD_JUMP ||
				this->_action == ACTION_NEUTRAL_AIR_JUMP ||
				this->_action == ACTION_FORWARD_AIR_JUMP ||
				this->_action == ACTION_BACKWARD_AIR_JUMP
			) {
				this->_jumpsUsed--;
				this->_hasJumped = false;
			}
		} else if (action >= ACTION_5N) {
			this->_hasJumped = true;
			this->_specialInputs._421n = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421m = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421s = -SPECIAL_INPUT_BUFFER_PERSIST;
			this->_specialInputs._421v = -SPECIAL_INPUT_BUFFER_PERSIST;
		}
		if (
			action != ACTION_AIR_HIT &&
			action != ACTION_GROUND_LOW_HIT &&
			action != ACTION_GROUND_HIGH_HIT &&
			action != ACTION_GROUND_SLAM &&
			action != ACTION_WALL_SLAM &&
			action != ACTION_UP_AIR_TECH &&
			action != ACTION_DOWN_AIR_TECH &&
			action != ACTION_FORWARD_AIR_TECH &&
			action != ACTION_BACKWARD_AIR_TECH &&
			action != ACTION_BEING_KNOCKED_DOWN &&
			action != ACTION_KNOCKED_DOWN
		) {
			this->_comboCtr = 0;
			this->_prorate = 1;
			this->_totalDamage = 0;
			this->_supersUsed = 0;
			this->_skillsUsed = 0;
			this->_limit.fill(0);
			this->_counter = false;
		}
		Object::_forceStartMove(action);
	}

	void Character::setOpponent(Character *opponent)
	{
		this->_opponent = opponent;
	}

	bool Character::_canCancel(unsigned int action)
	{
		auto currentData = this->getCurrentFrameData();

		if (!currentData->oFlag.cancelable)
			return false;
		if (!this->_hasHit && !currentData->dFlag.karaCancel)
			return false;
		if (currentData->oFlag.backDashCancelable && action == ACTION_BACKWARD_DASH)
			return true;
		if (currentData->oFlag.dashCancelable && action >= ACTION_AIR_DASH_1 && action <= ACTION_AIR_DASH_9 && this->_airDashesUsed < this->_maxAirDashes)
			return true;
		if (currentData->oFlag.dashCancelable && action == ACTION_FORWARD_DASH)
			return true;
		if (currentData->oFlag.jumpCancelable && action >= ACTION_NEUTRAL_AIR_JUMP && action <= ACTION_BACKWARD_AIR_JUMP && this->_jumpsUsed < this->_maxJumps)
			return true;
		if (currentData->oFlag.jumpCancelable && action >= ACTION_NEUTRAL_JUMP && action <= ACTION_BACKWARD_HIGH_JUMP)
			return true;
		if (action < 100)
			return false;
		if (action == this->_action && currentData->oFlag.jab)
			return true;
		if (this->getAttackTier(action) > this->getAttackTier(this->_action))
			return true;
		if (currentData->oFlag.hitSwitch && this->_action != action && this->getAttackTier(action) == this->getAttackTier(this->_action))
			return true;
		return false;
	}

	int Character::getAttackTier(unsigned int action) const
	{
		const FrameData *data;
		unsigned isTyped = (action >= ACTION_5M) * 100;

		if (action < 100)
			return -1;

		try {
			data = &this->_moves.at(action).at(0).at(0);
		} catch (...) {
			return -1;
		}
		if (data->priority)
			return *data->priority;
		if (data->oFlag.ultimate)
			return 800;
		if (data->oFlag.super)
			return 700;
		if (action == ACTION_AIR_REVERSAL || action == ACTION_GROUND_HIGH_REVERSAL || action == ACTION_GROUND_LOW_HIT)
			return 600;
		switch ((action % 50) + 100) {
		case ACTION_5N:
		case ACTION_2N:
		case ACTION_j5N:
			return 0 + isTyped;
		case ACTION_6N:
		case ACTION_8N:
		case ACTION_3N:
		case ACTION_j6N:
		case ACTION_j8N:
		case ACTION_j3N:
		case ACTION_j2N:
			return 200 + isTyped;
		case ACTION_c28N:
		case ACTION_c46N:
		case ACTION_c64N:
		case ACTION_214N:
		case ACTION_236N:
		case ACTION_421N:
		case ACTION_623N:
		case ACTION_41236N:
		case ACTION_63214N:
		case ACTION_6321469874N:
		case ACTION_j214N:
		case ACTION_j236N:
		case ACTION_j421N:
		case ACTION_j623N:
		case ACTION_j41236N:
		case ACTION_j63214N:
		case ACTION_j6321469874N:
			return 400 + isTyped;
		default:
			return -1;
		}
	}

	void Character::_checkPlatforms(Vector2f oldPos)
	{
		auto updatedY = this->_position.y;

		if (this->_specialInputs._22) {
			if (this->_isOnPlatform())
				this->_position.y -= 0.01;
			return;
		}
		this->_position.y = oldPos.y;

		auto plat = !this->_isOnPlatform();
		auto down = this->_input->isPressed(INPUT_DOWN);
		auto result = plat && down;

		this->_position.y = updatedY;
		if (result) {
			if (this->_isOnPlatform())
				this->_position.y -= 0.01;
			return;
		}
		Object::_checkPlatforms(oldPos);
	}

	void Character::_checkAllc28Input(bool tickBuffer)
	{
		if (this->_specialInputs._c28n)
			this->_specialInputs._c28n -= tickBuffer;
		else
			this->_specialInputs._c28n = this->_checkc28Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28m)
			this->_specialInputs._c28m -= tickBuffer;
		else
			this->_specialInputs._c28m = this->_checkc28Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28s)
			this->_specialInputs._c28s -= tickBuffer;
		else
			this->_specialInputs._c28s = this->_checkc28Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28v)
			this->_specialInputs._c28v -= tickBuffer;
		else
			this->_specialInputs._c28v = this->_checkc28Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28d)
			this->_specialInputs._c28d -= tickBuffer;
		else
			this->_specialInputs._c28d = this->_checkc28Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c28a)
			this->_specialInputs._c28a -= tickBuffer;
		else
			this->_specialInputs._c28a = this->_checkc28Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllc46Input(bool tickBuffer)
	{
		if (this->_specialInputs._c46n)
			this->_specialInputs._c46n -= tickBuffer;
		else
			this->_specialInputs._c46n = this->_checkc46Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46m)
			this->_specialInputs._c46m -= tickBuffer;
		else
			this->_specialInputs._c46m = this->_checkc46Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46s)
			this->_specialInputs._c46s -= tickBuffer;
		else
			this->_specialInputs._c46s = this->_checkc46Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46v)
			this->_specialInputs._c46v -= tickBuffer;
		else
			this->_specialInputs._c46v = this->_checkc46Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46d)
			this->_specialInputs._c46d -= tickBuffer;
		else
			this->_specialInputs._c46d = this->_checkc46Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c46a)
			this->_specialInputs._c46a -= tickBuffer;
		else
			this->_specialInputs._c46a = this->_checkc46Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllc64Input(bool tickBuffer)
	{
		if (this->_specialInputs._c64n)
			this->_specialInputs._c64n -= tickBuffer;
		else
			this->_specialInputs._c64n = this->_checkc64Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64m)
			this->_specialInputs._c64m -= tickBuffer;
		else
			this->_specialInputs._c64m = this->_checkc64Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64s)
			this->_specialInputs._c64s -= tickBuffer;
		else
			this->_specialInputs._c64s = this->_checkc64Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64v)
			this->_specialInputs._c64v -= tickBuffer;
		else
			this->_specialInputs._c64v = this->_checkc64Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64d)
			this->_specialInputs._c64d -= tickBuffer;
		else
			this->_specialInputs._c64d = this->_checkc64Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._c64a)
			this->_specialInputs._c64a -= tickBuffer;
		else
			this->_specialInputs._c64a = this->_checkc64Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll236Input(bool tickBuffer)
	{
		if (this->_specialInputs._236n)
			this->_specialInputs._236n -= tickBuffer;
		else
			this->_specialInputs._236n = this->_check236Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236m)
			this->_specialInputs._236m -= tickBuffer;
		else
			this->_specialInputs._236m = this->_check236Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236s)
			this->_specialInputs._236s -= tickBuffer;
		else
			this->_specialInputs._236s = this->_check236Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236v)
			this->_specialInputs._236v -= tickBuffer;
		else
			this->_specialInputs._236v = this->_check236Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236d)
			this->_specialInputs._236d -= tickBuffer;
		else
			this->_specialInputs._236d = this->_check236Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._236a)
			this->_specialInputs._236a -= tickBuffer;
		else
			this->_specialInputs._236a = this->_check236Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll214Input(bool tickBuffer)
	{
		if (this->_specialInputs._214n)
			this->_specialInputs._214n -= tickBuffer;
		else
			this->_specialInputs._214n = this->_check214Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214m)
			this->_specialInputs._214m -= tickBuffer;
		else
			this->_specialInputs._214m = this->_check214Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214s)
			this->_specialInputs._214s -= tickBuffer;
		else
			this->_specialInputs._214s = this->_check214Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214v)
			this->_specialInputs._214v -= tickBuffer;
		else
			this->_specialInputs._214v = this->_check214Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214d)
			this->_specialInputs._214d -= tickBuffer;
		else
			this->_specialInputs._214d = this->_check214Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._214a)
			this->_specialInputs._214a -= tickBuffer;
		else
			this->_specialInputs._214a = this->_check214Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll623Input(bool tickBuffer)
	{
		if (this->_specialInputs._623n)
			this->_specialInputs._623n -= tickBuffer;
		else
			this->_specialInputs._623n = this->_check623Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623m)
			this->_specialInputs._623m--;
		else
			this->_specialInputs._623m = this->_check623Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623s)
			this->_specialInputs._623s -= tickBuffer;
		else
			this->_specialInputs._623s = this->_check623Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623v)
			this->_specialInputs._623v -= tickBuffer;
		else
			this->_specialInputs._623v = this->_check623Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623d)
			this->_specialInputs._623d -= tickBuffer;
		else
			this->_specialInputs._623d = this->_check623Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._623a)
			this->_specialInputs._623a -= tickBuffer;
		else
			this->_specialInputs._623a = this->_check623Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll421Input(bool tickBuffer)
	{
		auto input = this->_input->getInputs();

		if (this->_specialInputs._421n)
			this->_specialInputs._421n -= std::copysign(tickBuffer, this->_specialInputs._421n);
		else
			this->_specialInputs._421n = this->_check421Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;
		if (this->_specialInputs._421n >= 0 && input.a && input.a < COMBINATION_LENIENCY && input.n && input.n < COMBINATION_LENIENCY)
			this->_specialInputs._421n = SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421m)
			this->_specialInputs._421m -= std::copysign(tickBuffer, this->_specialInputs._421m);
		else
			this->_specialInputs._421m = this->_check421Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;
		if (this->_specialInputs._421m >= 0 && input.a && input.a < COMBINATION_LENIENCY && input.m && input.m < COMBINATION_LENIENCY)
			this->_specialInputs._421m = SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421s)
			this->_specialInputs._421s -= std::copysign(tickBuffer, this->_specialInputs._421s);
		else
			this->_specialInputs._421s = this->_check421Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;
		if (this->_specialInputs._421s >= 0 && input.a && input.a < COMBINATION_LENIENCY && input.s && input.s < COMBINATION_LENIENCY)
			this->_specialInputs._421s = SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421v)
			this->_specialInputs._421v -= std::copysign(tickBuffer, this->_specialInputs._421v);
		else
			this->_specialInputs._421v = this->_check421Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;
		if (this->_specialInputs._421v >= 0 && input.a && input.a < COMBINATION_LENIENCY && input.v && input.v < COMBINATION_LENIENCY)
			this->_specialInputs._421v = SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421d)
			this->_specialInputs._421d -= tickBuffer;
		else
			this->_specialInputs._421d = this->_check421Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._421a)
			this->_specialInputs._421a -= tickBuffer;
		else
			this->_specialInputs._421a = this->_check421Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll624Input(bool tickBuffer)
	{
		if (this->_specialInputs._624n)
			this->_specialInputs._624n -= tickBuffer;
		else
			this->_specialInputs._624n = this->_check624Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624m)
			this->_specialInputs._624m -= tickBuffer;
		else
			this->_specialInputs._624m = this->_check624Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624s)
			this->_specialInputs._624s -= tickBuffer;
		else
			this->_specialInputs._624s = this->_check624Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624v)
			this->_specialInputs._624v -= tickBuffer;
		else
			this->_specialInputs._624v = this->_check624Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624d)
			this->_specialInputs._624d -= tickBuffer;
		else
			this->_specialInputs._624d = this->_check624Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624a)
			this->_specialInputs._624a -= tickBuffer;
		else
			this->_specialInputs._624a = this->_check624Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;

	}

	void Character::_checkAll426Input(bool tickBuffer)
	{
		if (this->_specialInputs._426n)
			this->_specialInputs._426n -= tickBuffer;
		else
			this->_specialInputs._426n = this->_check426Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426m)
			this->_specialInputs._426m -= tickBuffer;
		else
			this->_specialInputs._426m = this->_check426Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426s)
			this->_specialInputs._426s -= tickBuffer;
		else
			this->_specialInputs._426s = this->_check426Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426v)
			this->_specialInputs._426v -= tickBuffer;
		else
			this->_specialInputs._426v = this->_check426Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426d)
			this->_specialInputs._426d -= tickBuffer;
		else
			this->_specialInputs._426d = this->_check426Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._426a)
			this->_specialInputs._426a -= tickBuffer;
		else
			this->_specialInputs._426a = this->_check426Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll6314Input(bool tickBuffer)
	{
		if (this->_specialInputs._6314n)
			this->_specialInputs._6314n -= tickBuffer;
		else
			this->_specialInputs._6314n = this->_check6314Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314m)
			this->_specialInputs._6314m -= tickBuffer;
		else
			this->_specialInputs._6314m = this->_check6314Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314s)
			this->_specialInputs._6314s -= tickBuffer;
		else
			this->_specialInputs._6314s = this->_check6314Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314v)
			this->_specialInputs._6314v -= tickBuffer;
		else
			this->_specialInputs._6314v = this->_check6314Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314d)
			this->_specialInputs._6314d -= tickBuffer;
		else
			this->_specialInputs._6314d = this->_check6314Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314a)
			this->_specialInputs._6314a -= tickBuffer;
		else
			this->_specialInputs._6314a = this->_check6314Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll4136Input(bool tickBuffer)
	{
		if (this->_specialInputs._4136n)
			this->_specialInputs._4136n -= tickBuffer;
		else
			this->_specialInputs._4136n = this->_check4136Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._4136m)
			this->_specialInputs._4136m -= tickBuffer;
		else
			this->_specialInputs._4136m = this->_check4136Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._4136s)
			this->_specialInputs._4136s -= tickBuffer;
		else
			this->_specialInputs._4136s = this->_check4136Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._4136v)
			this->_specialInputs._4136v -= tickBuffer;
		else
			this->_specialInputs._4136v = this->_check4136Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._4136d)
			this->_specialInputs._4136d -= tickBuffer;
		else
			this->_specialInputs._4136d = this->_check4136Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._4136a)
			this->_specialInputs._4136a -= tickBuffer;
		else
			this->_specialInputs._4136a = this->_check4136Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll624684Input(bool tickBuffer)
	{
		if (this->_specialInputs._624684n)
			this->_specialInputs._624684n -= tickBuffer;
		else
			this->_specialInputs._624684n = this->_check624684Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684m)
			this->_specialInputs._624684m -= tickBuffer;
		else
			this->_specialInputs._624684m = this->_check624684Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684s)
			this->_specialInputs._624684s -= tickBuffer;
		else
			this->_specialInputs._624684s = this->_check624684Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684v)
			this->_specialInputs._624684v -= tickBuffer;
		else
			this->_specialInputs._624684v = this->_check624684Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684d)
			this->_specialInputs._624684d -= tickBuffer;
		else
			this->_specialInputs._624684d = this->_check624684Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._624684a)
			this->_specialInputs._624684a -= tickBuffer;
		else
			this->_specialInputs._624684a = this->_check624684Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll6314684Input(bool tickBuffer)
	{
		if (this->_specialInputs._6314684n)
			this->_specialInputs._6314684n -= tickBuffer;
		else
			this->_specialInputs._6314684n = this->_check6314684Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314684m)
			this->_specialInputs._6314684m -= tickBuffer;
		else
			this->_specialInputs._6314684m = this->_check6314684Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314684s)
			this->_specialInputs._6314684s -= tickBuffer;
		else
			this->_specialInputs._6314684s = this->_check6314684Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314684v)
			this->_specialInputs._6314684v -= tickBuffer;
		else
			this->_specialInputs._6314684v = this->_check6314684Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314684d)
			this->_specialInputs._6314684d -= tickBuffer;
		else
			this->_specialInputs._6314684d = this->_check6314684Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6314684a)
			this->_specialInputs._6314684a -= tickBuffer;
		else
			this->_specialInputs._6314684a = this->_check6314684Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll6246974Input(bool tickBuffer)
	{
		if (this->_specialInputs._6246974n)
			this->_specialInputs._6246974n -= tickBuffer;
		else
			this->_specialInputs._6246974n = this->_check6246974Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6246974m)
			this->_specialInputs._6246974m -= tickBuffer;
		else
			this->_specialInputs._6246974m = this->_check6246974Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6246974s)
			this->_specialInputs._6246974s -= tickBuffer;
		else
			this->_specialInputs._6246974s = this->_check6246974Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6246974v)
			this->_specialInputs._6246974v -= tickBuffer;
		else
			this->_specialInputs._6246974v = this->_check6246974Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6246974d)
			this->_specialInputs._6246974d -= tickBuffer;
		else
			this->_specialInputs._6246974d = this->_check6246974Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._6246974a)
			this->_specialInputs._6246974a -= tickBuffer;
		else
			this->_specialInputs._6246974a = this->_check6246974Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAll63146974Input(bool tickBuffer)
	{
		if (this->_specialInputs._63146974n)
			this->_specialInputs._63146974n -= tickBuffer;
		else
			this->_specialInputs._63146974n = this->_check63146974Input(getInputN) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._63146974m)
			this->_specialInputs._63146974m -= tickBuffer;
		else
			this->_specialInputs._63146974m = this->_check63146974Input(getInputM) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._63146974s)
			this->_specialInputs._63146974s -= tickBuffer;
		else
			this->_specialInputs._63146974s = this->_check63146974Input(getInputS) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._63146974v)
			this->_specialInputs._63146974v -= tickBuffer;
		else
			this->_specialInputs._63146974v = this->_check63146974Input(getInputV) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._63146974d)
			this->_specialInputs._63146974d -= tickBuffer;
		else
			this->_specialInputs._63146974d = this->_check63146974Input(getInputD) * SPECIAL_INPUT_BUFFER_PERSIST;

		if (this->_specialInputs._63146974a)
			this->_specialInputs._63146974a -= tickBuffer;
		else
			this->_specialInputs._63146974a = this->_check63146974Input(getInputA) * SPECIAL_INPUT_BUFFER_PERSIST;
	}

	void Character::_checkAllHJInput(bool tickBuffer)
	{
		if (this->_specialInputs._27)
			this->_specialInputs._27 -= tickBuffer;
		else
			this->_specialInputs._27 = this->_check27Input() * HJ_BUFFER_PERSIST;

		if (this->_specialInputs._28)
			this->_specialInputs._28 -= tickBuffer;
		else
			this->_specialInputs._28 = (this->_check28Input() || this->_dummyState == DUMMYSTATE_HIGH_JUMP) * HJ_BUFFER_PERSIST;

		if (this->_specialInputs._29)
			this->_specialInputs._29 -= tickBuffer;
		else
			this->_specialInputs._29 = this->_check29Input() * HJ_BUFFER_PERSIST;
	}

	void Character::_checkAllDashInput(bool tickBuffer)
	{
		if (this->_specialInputs._44)
			this->_specialInputs._44 -= tickBuffer;
		else
			this->_specialInputs._44 = this->_check44Input() * DASH_BUFFER_PERSIST;

		if (this->_specialInputs._66)
			this->_specialInputs._66 -= tickBuffer;
		else
			this->_specialInputs._66 = this->_check66Input() * DASH_BUFFER_PERSIST;
	}

	void Character::_checkSpecialInputs(bool tickBuffer)
	{
		if (this->_inputDisabled) {
			memset(this->_specialInputs._value, 0, sizeof(this->_specialInputs._value));
			return;
		}

		this->_clearLastInputs();
		if (
			this->_action == ACTION_AIR_HIT ||
			this->_action == ACTION_BEING_KNOCKED_DOWN ||
			this->_action == ACTION_KNOCKED_DOWN ||
			this->_action == ACTION_NEUTRAL_TECH ||
			this->_action == ACTION_FORWARD_TECH ||
			this->_action == ACTION_BACKWARD_TECH
		)
			this->_specialInputs._22 = false;
		else
			this->_specialInputs._22 = this->_check22Input();

		this->_checkAllDashInput(tickBuffer);
		this->_checkAllHJInput(tickBuffer);

		if (this->_atkDisabled) {
			memset(&this->_specialInputs._value[3], 0, sizeof(this->_specialInputs._value) - 3);
			return;
		}
		this->_checkAllc28Input(tickBuffer);
		this->_checkAllc46Input(tickBuffer);
		this->_checkAllc64Input(tickBuffer);
		this->_checkAll236Input(tickBuffer);
		this->_checkAll214Input(tickBuffer);
		this->_checkAll623Input(tickBuffer);
		this->_checkAll421Input(tickBuffer);
		this->_checkAll624Input(tickBuffer);
		this->_checkAll426Input(tickBuffer);
		this->_checkAll6314Input(tickBuffer);
		this->_checkAll4136Input(tickBuffer);
		this->_checkAll624684Input(tickBuffer);
		this->_checkAll6314684Input(tickBuffer);
		this->_checkAll6246974Input(tickBuffer);
		this->_checkAll63146974Input(tickBuffer);
	}

	bool Character::_check236Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found6 |= foundAtk && !input.v && input.h > 0;
			found3 |= found6 && input.v < 0 && input.h > 0;
			found2 |= found3 && input.v < 0 && !input.h;
			if (found2 && found3 && found6)
				return true;
			total += input.nbFrames;
			if (total > QUARTER_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check214Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4 |= foundAtk && !input.v && input.h < 0;
			found1 |= found4 && input.v < 0 && input.h < 0;
			found2 |= found1 && input.v < 0 && !input.h;
			if (found2 && found1 && found4)
				return true;
			total += input.nbFrames;
			if (total > QUARTER_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check623Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found3 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found3 |= foundAtk && input.v < 0 && input.h > 0;
			found2 |= found3 && input.v < 0 && !input.h;
			found6 |= found2 && !input.v && input.h > 0;
			if (found2 && found3 && found6)
				return true;
			total += input.nbFrames;
			if (total > DP_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check421Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found1 = false;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found1 |= foundAtk && input.v < 0 && input.h < 0;
			found2 |= found1 && input.v < 0 && !input.h;
			found4 |= found2 && !input.v && input.h < 0;
			if (found2 && found1 && found4)
				return true;
			total += input.nbFrames;
			if (total > DP_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check624Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4 |= foundAtk && !input.v && input.h < 0;
			found2 |= found4 && input.v < 0 && !input.h;
			found6 |= found2 && !input.v && input.h > 0;
			if (!input.v && input.h < 0 && found2 && found4)
				break;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check426Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found2 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found6 |= foundAtk && !input.v && input.h > 0;
			found2 |= found4 && input.v < 0 && !input.h;
			found4 |= found2 && !input.v && input.h < 0;
			if (!input.v && input.h > 0 && found2 && found6)
				break;
			if (found2 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check6314Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found1 = false;
		bool found3 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4 |= foundAtk && !input.v && input.h < 0;
			found1 |= found4 && input.v < 0 && input.h < 0;
			found3 |= found1 && input.v < 0 && input.h > 0;
			found6 |= found3 && !input.v && input.h > 0;
			if (found1 && found3 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check4136Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found1 = false;
		bool found3 = false;
		bool found4 = false;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found6 |= foundAtk && !input.v && input.h > 0;
			found3 |= found6 && input.v < 0 && input.h > 0;
			found1 |= found3 && input.v < 0 && input.h < 0;
			found4 |= found1 && !input.v && input.h < 0;
			if (found1 && found3 && found4 && found6)
				return true;
			total += input.nbFrames;
			if (total > HALF_CIRCLE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check624684Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found2 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found8 = false;
		bool found4_2 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4_2 |= foundAtk && !input.v && input.h < 0;
			found8   |= found4_2 && input.v > 0 && !input.h;
			found6_2 |= found8   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found2   |= found4_1 && input.v < 0 && !input.h;
			found6_1 |= found2   && !input.v && input.h > 0;
			if (found6_1 && found2 && found4_1 && found6_2 && found8 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check6314684Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found3 = false;
		bool found1 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found8 = false;
		bool found4_2 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4_2 |= foundAtk && !input.v && input.h < 0;
			found8   |= found4_2 && input.v > 0 && !input.h;
			found6_2 |= found8   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found1   |= found4_1 && input.v < 0 && input.h < 0;
			found3   |= found1   && input.v < 0 && input.h > 0;
			found6_1 |= found3   && !input.v && input.h > 0;
			if (found6_1 && found3 && found1 && found4_1 && found6_2 && found8 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check6246974Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found2 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found9 = false;
		bool found7 = false;
		bool found4_2 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4_2 |= foundAtk && !input.v && input.h < 0;
			found7   |= found4_2 && input.v > 0 && input.h < 0;
			found9   |= found7   && input.v > 0 && input.h > 0;
			found6_2 |= found9   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found2   |= found4_1 && input.v < 0 && !input.h;
			found6_1 |= found2   && !input.v && input.h > 0;
			if (found6_1 && found2 && found4_1 && found6_2 && found9 && found7 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check63146974Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned total = 0;
		bool found6_1 = false;
		bool found3 = false;
		bool found1 = false;
		bool found4_1 = false;
		bool found6_2 = false;
		bool found9 = false;
		bool found7 = false;
		bool found4_2 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4_2 |= foundAtk && !input.v && input.h < 0;
			found7   |= found4_2 && input.v > 0 && input.h < 0;
			found9   |= found7   && input.v > 0 && input.h > 0;
			found6_2 |= found9   && !input.v && input.h > 0;
			found4_1 |= found6_2 && !input.v && input.h < 0;
			found1   |= found4_1 && input.v < 0 && input.h < 0;
			found3   |= found1   && input.v < 0 && input.h > 0;
			found6_1 |= found3   && !input.v && input.h > 0;
			if (found6_1 && found3 && found1 && found4_1 && found6_2 && found9 && found7 && found4_2)
				return true;
			total += input.nbFrames;
			if (total > SPIRAL_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_check22Input()
	{
		unsigned total = 0;
		bool found2 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				return false;
			if (found2 && foundOther && input.v < 0)
				return true;
			found2 |= input.v < 0;
			foundOther |= found2 && input.v >= 0;
		}
		return false;
	}

	bool Character::_check44Input()
	{
		unsigned total = 0;
		bool found4 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
			if (input.h < 0 && input.v != 0)
				return false;
			if (found4 && foundOther && input.h < 0)
				return true;
			found4 |= input.h < 0;
			foundOther |= found4 && input.h >= 0;
		}
		return false;
	}

	bool Character::_check66Input()
	{
		unsigned total = 0;
		bool found6 = false;
		bool foundOther = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > DASH_BUFFER)
				break;
			if (input.h > 0 && input.v != 0)
				return false;
			if (found6 && foundOther && input.h > 0)
				return true;
			found6 |= input.h > 0;
			foundOther |= found6 && input.h <= 0;
		}
		return false;
	}

	bool Character::_check27Input()
	{
		unsigned total = 0;
		bool found7 = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found7 |= input.v > 0 && input.h < 0;
			if (input.v < 0 && found7)
				return true;
		}
		return false;
	}

	bool Character::_check28Input()
	{
		unsigned total = 0;
		bool found8 = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found8 |= input.v > 0;
			if (found8 && input.v < 0)
				return true;
		}
		return false;
	}

	bool Character::_check29Input()
	{
		unsigned total = 0;
		bool found9 = false;

		for (auto &input : this->_lastInputs) {
			total += input.nbFrames;
			if (total > HJ_BUFFER)
				break;
			found9 |= input.v > 0 && input.h > 0;
			if (found9 && input.v < 0)
				return true;
		}
		return false;
	}

	bool Character::_checkc28Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found8 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found8 |= foundAtk && input.v > 0;
			if (input.v < 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found8)
				return true;
			total += input.nbFrames;
			if ((!found8 || timer == 0) && total > CHARGE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_checkc46Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found6 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found6 |= foundAtk && input.h > 0;
			if (input.h < 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found6)
				return true;
			total += input.nbFrames;
			if ((!found6 || timer == 0) && total > CHARGE_BUFFER)
				break;
		}
		return false;
	}

	bool Character::_checkc64Input(const std::function<bool (const LastInput &)> &atkInput)
	{
		unsigned timer2 = 0;
		unsigned timer = 0;
		unsigned total = 0;
		bool found4 = false;
		bool foundAtk = false;

		for (auto &input : this->_lastInputs) {
			foundAtk |= atkInput(input);
			found4 |= foundAtk && input.h < 0;
			if (input.h > 0) {
				timer += input.nbFrames;
				timer2 = 0;
			} else {
				timer2 += input.nbFrames;
				if (timer2 >= CHARGE_PART_BUFFER)
					timer = 0;
			}
			if (timer >= CHARGE_TIME && found4)
				return true;
			total += input.nbFrames;
			if ((!found4 || timer == 0) && total > CHARGE_BUFFER)
				break;
		}
		return false;
	}

	void Character::_clearLastInputs()
	{
		auto it = this->_lastInputs.begin();
		unsigned total = 0;

		while (it != this->_lastInputs.end() && total < MAX_FRAME_IN_BUFFER) {
			total += it->nbFrames;
			it++;
		}
		this->_lastInputs.erase(it, this->_lastInputs.end());
	}

	bool Character::_executeAirDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_AIR_DASH_4))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_AIR_DASH_6))
			return true;
		if (!this->_input->isPressed(INPUT_DASH) || (!input.verticalAxis && !input.horizontalAxis))
			return false;
		return  (input.verticalAxis > 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_9)) ||
			(input.verticalAxis > 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_7)) ||
			(input.verticalAxis > 0 &&                                          this->_startMove(ACTION_AIR_DASH_8)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_3)) ||
			(input.verticalAxis < 0 && this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_AIR_DASH_1)) ||
			(input.verticalAxis < 0 &&                                          this->_startMove(ACTION_AIR_DASH_2)) ||
			(                          this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_AIR_DASH_6)) ||
			(                                                                   this->_startMove(ACTION_AIR_DASH_4));
	}

	bool Character::_executeAirParry(const InputStruct &input)
	{
		if (!this->_input->isPressed(INPUT_ASCEND) || input.horizontalAxis * this->_dir >= 0)
			return false;
		if (input.n) {
			if (this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY))
				return true;
		}
		if (input.m) {
			if (
				this->_action == ACTION_AIR_MATTER_PARRY || this->_startMove(ACTION_AIR_MATTER_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		if (input.s) {
			if (
				this->_action == ACTION_AIR_SPIRIT_PARRY || this->_startMove(ACTION_AIR_SPIRIT_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		if (input.v) {
			if (
				this->_action == ACTION_AIR_VOID_PARRY || this->_startMove(ACTION_AIR_VOID_PARRY) ||
				this->_action == ACTION_AIR_NEUTRAL_PARRY || this->_startMove(ACTION_AIR_NEUTRAL_PARRY)
			)
				return true;
		}
		return false;
	}

	bool Character::_executeAirJump(const InputStruct &input)
	{
		if (this->_hasJumped)
			return false;
		if (input.verticalAxis <= 0)
			return false;
		if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_AIR_JUMP))
			return true;
		if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_AIR_JUMP))
			return true;
		return this->_startMove(ACTION_NEUTRAL_AIR_JUMP);
	}

	bool Character::_executeGroundDashes(const InputStruct &input)
	{
		if (this->_specialInputs._44 && this->_startMove(ACTION_BACKWARD_DASH))
			return true;
		if (this->_specialInputs._66 && this->_startMove(ACTION_FORWARD_DASH))
			return true;
		if (!this->_input->isPressed(INPUT_DASH) || !input.horizontalAxis)
			return false;
		return this->_startMove(this->_dir * input.horizontalAxis > 0 ? ACTION_FORWARD_DASH : ACTION_BACKWARD_DASH);
	}

	bool Character::_executeGroundParry(const InputStruct &input)
	{
		if (!this->_input->isPressed(INPUT_ASCEND) || input.horizontalAxis * this->_dir >= 0)
			return false;
		if (input.n) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (this->_action == move || this->_startMove(move))
				return true;
		}
		if (input.m) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_MATTER_PARRY : ACTION_GROUND_HIGH_MATTER_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.s) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_SPIRIT_PARRY : ACTION_GROUND_HIGH_SPIRIT_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		if (input.v) {
			auto move = input.verticalAxis < 0 ? ACTION_GROUND_LOW_VOID_PARRY : ACTION_GROUND_HIGH_VOID_PARRY;
			auto move2 = input.verticalAxis < 0 ? ACTION_GROUND_LOW_NEUTRAL_PARRY : ACTION_GROUND_HIGH_NEUTRAL_PARRY;

			if (
				this->_action == move  || this->_startMove(move) ||
				this->_action == move2 || this->_startMove(move2)
			)
				return true;
		}
		return false;
	}

	bool Character::_executeGroundJump(const InputStruct &input)
	{
		if (this->_specialInputs._29 && this->_startMove(ACTION_FORWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._27 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP))
			return true;
		if (this->_specialInputs._28 && this->_startMove(ACTION_NEUTRAL_HIGH_JUMP))
			return true;
		if (input.verticalAxis <= 0)
			return false;
		if (this->_input->isPressed(INPUT_DASH)) {
			if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_HIGH_JUMP))
				return true;
			if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_HIGH_JUMP))
				return true;
			if (input.horizontalAxis == 0 && this->_startMove(ACTION_NEUTRAL_HIGH_JUMP))
				return true;
		}
		if (input.horizontalAxis * this->_dir > 0 && this->_startMove(ACTION_FORWARD_JUMP))
			return true;
		if (input.horizontalAxis * this->_dir < 0 && this->_startMove(ACTION_BACKWARD_JUMP))
			return true;
		return this->_startMove(ACTION_NEUTRAL_JUMP);
	}

	bool Character::_executeCrouch(const InputStruct &input)
	{
		if (input.verticalAxis >= 0)
			return false;
		this->_startMove(ACTION_CROUCHING);
		return this->_action == ACTION_CROUCHING || this->_action == ACTION_CROUCH;
	}

	bool Character::_executeWalking(const InputStruct &input)
	{
		if (!input.horizontalAxis)
			return false;
		this->_startMove(std::copysign(1, input.horizontalAxis) == std::copysign(1, this->_dir) ? ACTION_WALK_FORWARD : ACTION_WALK_BACKWARD);
		return true;
	}

	bool Character::_executeAirTech(const InputStruct &input)
	{
		switch (this->_dummyAirTech) {
		case AIRTECH_NONE:
			break;
		case AIRTECH_FORWARD:
			if (this->_startMove(ACTION_FORWARD_AIR_TECH))
				return true;
			break;
		case AIRTECH_BACKWARD:
			if (this->_startMove(ACTION_BACKWARD_AIR_TECH))
				return true;
			break;
		case AIRTECH_UP:
			if (this->_startMove(ACTION_UP_AIR_TECH))
				return true;
			break;
		case AIRTECH_DOWN:
			if (this->_startMove(ACTION_DOWN_AIR_TECH))
				return true;
			break;
		case AIRTECH_RANDOM:
			switch (game->random() % 4) {
			case 0:
				if (this->_startMove(ACTION_FORWARD_AIR_TECH))
					return true;
				break;
			case 1:
				if (this->_startMove(ACTION_BACKWARD_AIR_TECH))
					return true;
				break;
			case 2:
				if (this->_startMove(ACTION_UP_AIR_TECH))
					return true;
				break;
			case 3:
				if (this->_startMove(ACTION_DOWN_AIR_TECH))
					return true;
				break;
			}
			break;
		}
		if (!input.d & !input.n & !input.v & !input.m & !input.a & !input.s)
			return false;
		return  (input.verticalAxis > 0 &&                this->_startMove(ACTION_UP_AIR_TECH)) ||
			(input.verticalAxis < 0 &&                this->_startMove(ACTION_DOWN_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis > 0 && this->_startMove(ACTION_FORWARD_AIR_TECH)) ||
			(this->_dir * input.horizontalAxis < 0 && this->_startMove(ACTION_BACKWARD_AIR_TECH));
	}

	bool Character::hits(const IObject &other) const
	{
		auto otherChr = dynamic_cast<const Character *>(&other);

		if (otherChr) {
			for (auto limit : otherChr->_limit)
				if (limit >= 100)
					return false;

			auto odata = otherChr->getCurrentFrameData();
			auto mdata = this->getCurrentFrameData();

			if (
				(odata->oFlag.spiritElement && odata->oFlag.matterElement && odata->oFlag.voidElement) &&
				(this->_action == ACTION_NEUTRAL_OVERDRIVE                || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (
				(otherChr->_action == ACTION_SPIRIT_ROMAN_CANCEL || otherChr->_action == ACTION_SPIRIT_AIR_ROMAN_CANCEL || odata->oFlag.spiritElement) &&
				(this->_action == ACTION_MATTER_OVERDRIVE        || this->_action == ACTION_MATTER_AIR_OVERDRIVE        || this->_action == ACTION_NEUTRAL_OVERDRIVE || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return false;
			if (
				(otherChr->_action == ACTION_MATTER_ROMAN_CANCEL || otherChr->_action == ACTION_MATTER_AIR_ROMAN_CANCEL || odata->oFlag.matterElement) &&
				(this->_action == ACTION_VOID_OVERDRIVE          || this->_action == ACTION_VOID_AIR_OVERDRIVE          || this->_action == ACTION_NEUTRAL_OVERDRIVE || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return false;
			if (
				(otherChr->_action == ACTION_VOID_ROMAN_CANCEL || otherChr->_action == ACTION_VOID_AIR_ROMAN_CANCEL || odata->oFlag.voidElement) &&
				(this->_action == ACTION_SPIRIT_OVERDRIVE      || this->_action == ACTION_SPIRIT_AIR_OVERDRIVE      || this->_action == ACTION_NEUTRAL_OVERDRIVE || this->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return false;

			if (
				(mdata->oFlag.spiritElement && mdata->oFlag.matterElement && mdata->oFlag.voidElement) &&
				(otherChr->_action == ACTION_NEUTRAL_OVERDRIVE            || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return false;
			if (
				(this->_action == ACTION_SPIRIT_ROMAN_CANCEL || this->_action == ACTION_SPIRIT_AIR_ROMAN_CANCEL || mdata->oFlag.spiritElement) &&
				(otherChr->_action == ACTION_VOID_OVERDRIVE  || otherChr->_action == ACTION_VOID_AIR_OVERDRIVE  || otherChr->_action == ACTION_NEUTRAL_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (
				(this->_action == ACTION_MATTER_ROMAN_CANCEL  || this->_action == ACTION_MATTER_AIR_ROMAN_CANCEL  || mdata->oFlag.matterElement) &&
				(otherChr->_action == ACTION_SPIRIT_OVERDRIVE || otherChr->_action == ACTION_SPIRIT_AIR_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (
				(this->_action == ACTION_VOID_ROMAN_CANCEL    || this->_action == ACTION_VOID_AIR_ROMAN_CANCEL    || mdata->oFlag.voidElement) &&
				(otherChr->_action == ACTION_MATTER_OVERDRIVE || otherChr->_action == ACTION_MATTER_AIR_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_OVERDRIVE || otherChr->_action == ACTION_NEUTRAL_AIR_OVERDRIVE)
			)
				return Object::hits(other);
			if (isOverdriveAction(otherChr->_action))
				return false;
		}
		if (isRomanCancelAction(this->_action))
			return false;
		return Object::hits(other);
	}

	void Character::postUpdate()
	{
		if (this->_blockStun == 0 && (this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT) && this->_opponent->_action < ACTION_5N)
			this->_blockStun = 60;
		if (this->_position.x < 0)
			this->_position.x = 0;
		else if (this->_position.x > 1000)
			this->_position.x = 1000;
		if (this->_position.y < 0)
			this->_position.y = 0;
		else if (this->_position.y > 1000)
			this->_position.y = 1000;

		auto data = this->getCurrentFrameData();
		char buffer[8194];

		sprintf(
			buffer,
			"PositionX: %f\n"
			"PositionY: %f\n"
			"SpeedX: %f\n"
			"SpeedY: %f\n"
			"GravityX: %f\n"
			"GravityY: %f\n"
			"BlockStun: %i\n"
			"JumpUsed: %i/%i\n"
			"AirDashUsed: %i/%i\n"
			"Jumped: %s\n"
			"Restand: %s\n"
			"Action: %i\n"
			"ActionBlock: %i/%llu\n"
			"Animation: %i/%llu\n"
			"AnimationCtr: %i/%i\n"
			"Hp: %i/%i\n"
			"Rotation: %f\n"
			"HasHit: %s\n"
			"Direction: %s\n"
			"Dir: %f\n"
			"cornerPriority: %i\n"
			"justGotCorner: %s\n"
			"comboCtr: %u\n"
			"prorate: %f\n"
			"totalDamage: %u\n"
			"neutralLimit: %u\n"
			"voidLimit: %u\n"
			"matterLimit: %u\n"
			"spiritLimit: %u\n"
			"BaseGravityX: %f\n"
			"BaseGravityY: %f\n"
			"Overdrive CD: %u/%u\n"
			"SupersUsed %u\n"
			"SkillsUsed %u",
			this->_position.x,
			this->_position.y,
			this->_speed.x,
			this->_speed.y,
			this->_gravity.x,
			this->_gravity.y,
			this->_blockStun,
			this->_jumpsUsed,
			this->_maxJumps,
			this->_airDashesUsed,
			this->_maxAirDashes,
			this->_hasJumped ? "true" : "false",
			this->_restand ? "true" : "false",
			this->_action,
			this->_actionBlock,
			this->_moves.at(this->_action).size(),
			this->_animation,
			this->_moves.at(this->_action)[this->_actionBlock].size(),
			this->_animationCtr,
			this->_moves.at(this->_action)[this->_actionBlock][this->_animation].duration,
			this->_hp,
			this->_baseHp,
			this->_rotation,
			this->_hasHit ? "true" : "false",
			this->_direction ? "right" : "left",
			this->_dir,
			this->_cornerPriority,
			this->_justGotCorner ? "true" : "false",
			this->_comboCtr,
			this->_prorate,
			this->_totalDamage,
			this->_limit[0],
			this->_limit[1],
			this->_limit[2],
			this->_limit[3],
			this->_baseGravity.x,
			this->_baseGravity.y,
			this->_odCooldown,
			this->_maxOdCooldown,
			this->_supersUsed,
			this->_skillsUsed
		);
		this->_text.setString(buffer);
		this->_text.setPosition({static_cast<float>(this->_team * 850), -550});

		sprintf(
			buffer,
			"voidMana: %.2f/%u\n"
			"spiritMana: %.2f/%u\n"
			"matterMana: %.2f/%u\n"
			"specialMarker: %u\n"
			"blockStun: %u\n"
			"hitStun: %u\n"
			"prorate: %3.f\n"
			"neutralLimit: %u\n"
			"voidLimit: %u\n"
			"spiritLimit: %u\n"
			"matterLimit: %u\n"
			"pushBack: %i\n"
			"pushBlock: %i\n"
			"subObjectSpawn: %u\n"
			"manaGain: %u\n"
			"manaCost: %u\n"
			"hitStop: %u\n"
			"damage: %u\n",
			this->_voidMana,
			this->_voidManaMax,
			this->_spiritMana,
			this->_spiritManaMax,
			this->_matterMana,
			this->_matterManaMax,
			data->specialMarker,
			data->blockStun,
			data->hitStun,
			data->prorate,
			data->neutralLimit,
			data->voidLimit,
			data->spiritLimit,
			data->matterLimit,
			data->pushBack,
			data->pushBlock,
			data->subObjectSpawn,
			data->manaGain,
			data->manaCost,
			data->hitStop,
			data->damage
		);
		for (unsigned tmp = data->dFlag.flags, i = 0; tmp; tmp >>= 1, i++)
			if (tmp & 1) {
				strcat(buffer, dFlags[i]);
				strcat(buffer, "\n");
			}
		for (unsigned tmp = data->oFlag.flags, i = 0; tmp; tmp >>= 1, i++)
			if (tmp & 1) {
				strcat(buffer, oFlags[i]);
				strcat(buffer, "\n");
			}
		this->_text2.setString(buffer);
		this->_text2.setPosition({static_cast<float>(this->_team * 500 + 150) , -550});
	}

	unsigned char Character::_checkHitPos(const Object *other) const
	{
		if (!other)
			return 0;

		auto *oData = this->getCurrentFrameData();
		auto *mData = other->getCurrentFrameData();
		auto mCenter = other->_position;
		auto oCenter = this->_position;
		auto mScale = Vector2f{
			static_cast<float>(mData->size.x) / mData->textureBounds.size.x,
			static_cast<float>(mData->size.y) / mData->textureBounds.size.y
		};
		auto oScale = Vector2f{
			static_cast<float>(oData->size.x) / oData->textureBounds.size.x,
			static_cast<float>(oData->size.y) / oData->textureBounds.size.y
		};
		bool found = false;
		SpiralOfFate::Rectangle hurtbox;
		SpiralOfFate::Rectangle hitbox;

		mCenter.y *= -1;
		mCenter += Vector2f{
			mData->size.x / -2.f - mData->offset.x * !other->_direction * 2.f,
			-static_cast<float>(mData->size.y) + mData->offset.y
		};
		mCenter += Vector2f{
			mData->textureBounds.size.x * mScale.x / 2,
			mData->textureBounds.size.y * mScale.y / 2
		};
		oCenter.y *= -1;
		oCenter += Vector2f{
			oData->size.x / -2.f - oData->offset.x * !this->_direction * 2.f,
			-static_cast<float>(oData->size.y) + oData->offset.y
		};
		oCenter += Vector2f{
			oData->textureBounds.size.x * oScale.x / 2,
			oData->textureBounds.size.y * oScale.y / 2
		};

		for (auto &hurtBox : oData->hurtBoxes) {
			auto _hurtBox = this->_applyModifiers(hurtBox);
			SpiralOfFate::Rectangle __hurtBox;

			__hurtBox.pt1 = _hurtBox.pos.rotation(this->_rotation, oCenter)                                                      + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt2 = (_hurtBox.pos + Vector2f{0, static_cast<float>(_hurtBox.size.y)}).rotation(this->_rotation, oCenter) + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt3 = (_hurtBox.pos + _hurtBox.size).rotation(this->_rotation, oCenter)                                    + Vector2f{this->_position.x, -this->_position.y};
			__hurtBox.pt4 = (_hurtBox.pos + Vector2f{static_cast<float>(_hurtBox.size.x), 0}).rotation(this->_rotation, oCenter) + Vector2f{this->_position.x, -this->_position.y};
			hurtbox = __hurtBox;
			for (auto &hitBox : mData->hitBoxes) {
				auto _hitBox = other->_applyModifiers(hitBox);
				SpiralOfFate::Rectangle __hitBox;

				__hitBox.pt1 = _hitBox.pos.rotation(other->_rotation, mCenter)                                                     + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt2 = (_hitBox.pos + Vector2f{0, static_cast<float>(_hitBox.size.y)}).rotation(other->_rotation, mCenter) + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt3 = (_hitBox.pos + _hitBox.size).rotation(other->_rotation, mCenter)                                    + Vector2f{other->_position.x, -other->_position.y};
				__hitBox.pt4 = (_hitBox.pos + Vector2f{static_cast<float>(_hitBox.size.x), 0}).rotation(other->_rotation, mCenter) + Vector2f{other->_position.x, -other->_position.y};
				if (__hurtBox.intersect(__hitBox) || __hurtBox.isIn(__hitBox) || __hitBox.isIn(__hurtBox)) {
					hitbox = __hitBox;
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		my_assert(found);

		auto height = 0;
		auto pts = hurtbox.getIntersectionPoints(hitbox);
		auto center = this->_position.y + oData->offset.y + oData->size.y / 2;

		for (auto &arr : pts)
			for (auto &pt : arr)
				height += (pt.y > center) - (pt.y < center);
		if (height == 0)
			return 0;
		if (pts.size() == 1)
			return 0; // TODO: Handle this case
		return height > 0 ? 1 : 2;
	}

	void Character::_blockMove(Object *other, const FrameData &data)
	{
		auto myData = this->getCurrentFrameData();
		bool low = (
			this->_input->isPressed(INPUT_DOWN) ||
			(this->_forceBlock & 7) == LOW_BLOCK ||
			((this->_forceBlock & 7) == RANDOM_HEIGHT_BLOCK && game->random() % 2)
		);
		unsigned char height = data.oFlag.lowHit | (data.oFlag.highHit << 1);

		if (!isParryAction(this->_action))
			this->_guardRegenCd = 120;
		game->battleMgr->addHitStop(data.hitStop);
		if (data.oFlag.autoHitPos)
			height |= this->_checkHitPos(other);
		if ((this->_forceBlock & 7) == ALL_RIGHT_BLOCK)
			low = height & 1;
		if ((this->_forceBlock & 7) == ALL_WRONG_BLOCK)
			low = height & 2;

		bool wrongBlock =
			((height & 1) && !myData->dFlag.lowBlock && (!low || !myData->dFlag.canBlock)) ||
			((height & 2) && !myData->dFlag.highBlock && (low || !myData->dFlag.canBlock));

		if (
			(
				(
					(this->_forceBlock & 7) || this->_input->isPressed(this->_direction ? INPUT_LEFT : INPUT_RIGHT)
				) && myData->dFlag.canBlock
			) || isBlockingAction(this->_action)
		) {
			if (wrongBlock) {
				if (this->_guardCooldown)
					return this->_getHitByMove(other, data);
				if (this->_isGrounded())
					this->_forceStartMove(low ? ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_WRONG_BLOCK);
				this->_blockStun = data.blockStun * 5 / 3;
				game->soundMgr.play(BASICSOUND_WRONG_BLOCK);
			} else {
				if (this->_isGrounded())
					this->_forceStartMove(low ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK);
				else
					this->_forceStartMove(ACTION_AIR_NEUTRAL_BLOCK);
				this->_blockStun = data.blockStun;
				game->soundMgr.play(BASICSOUND_BLOCK);
			}
			this->_processGuardLoss(wrongBlock);
		} else if (wrongBlock)
			return this->_getHitByMove(other, data);
		else if (isParryAction(this->_action)) {
			this->_speed.x += data.pushBlock * -this->_dir;
			this->_parryEffect(other);
			return;
		} else if (data.oFlag.matterElement && data.oFlag.voidElement && data.oFlag.spiritElement) //TRUE NEUTRAL
			if (myData->dFlag.neutralBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else
				return this->_getHitByMove(other, data);
		else if (data.oFlag.matterElement) {
			if (myData->dFlag.voidBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.matterBlock || myData->dFlag.neutralBlock || myData->dFlag.spiritBlock)
				return this->_getHitByMove(other, data);
		} else if (data.oFlag.voidElement) {
			if (myData->dFlag.spiritBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.voidBlock || myData->dFlag.neutralBlock || myData->dFlag.matterBlock)
				return this->_getHitByMove(other, data);
		} else if (data.oFlag.spiritElement) {
			if (myData->dFlag.matterBlock)
				game->soundMgr.play(BASICSOUND_BLOCK);
			else if (myData->dFlag.spiritBlock || myData->dFlag.neutralBlock || myData->dFlag.voidBlock)
				return this->_getHitByMove(other, data);
		}
		this->_speed.x += data.pushBlock * -this->_dir;
		this->_hp -= data.chipDamage;
	}

	bool Character::_isOnPlatform() const
	{
		if (this->_specialInputs._22)
			return false;
		return Object::_isOnPlatform();
	}

	void Character::_getHitByMove(Object *obj, const FrameData &data)
	{
		if (data.oFlag.ultimate) {
			this->_supersUsed = 0;
			this->_skillsUsed = 0;
			this->_prorate = max(0.25, this->_prorate);
		}

		auto superRate = this->_supersUsed >= 2 ? min(1.f, max(0.f, (100.f - (10 << (this->_supersUsed - 2))) / 100.f)) : 1;
		auto skillRate = this->_skillsUsed >= 2 ? min(1.f, max(0.f, (100.f - (3 << (this->_skillsUsed - 2))) / 100.f)) : 1;
		auto myData = this->getCurrentFrameData();
		auto counter = this->_counterHit == 1;
		auto chr = dynamic_cast<Character *>(obj);
		float damage = data.damage * this->_prorate * skillRate * superRate;

		my_assert(!data.oFlag.ultimate || chr);
		counter &= this->_action != ACTION_AIR_HIT;
		counter &= this->_action != ACTION_WALL_SLAM;
		counter &= this->_action != ACTION_GROUND_SLAM;
		counter &= this->_action != ACTION_GROUND_LOW_HIT;
		counter &= this->_action != ACTION_GROUND_HIGH_HIT;
		if (data.oFlag.ultimate && chr->_actionBlock == 0) {
			chr->_actionBlock++;
			if (chr->_actionBlock == chr->_moves.at(chr->_action).size())
				//TODO : Create custom exceptions
				throw std::invalid_argument("Action " + actionToString(chr->_action) + " is missing block 1");
			chr->_animation = 0;
			chr->_applyNewAnimFlags();
		}
		if ((myData->dFlag.counterHit || counter) && data.oFlag.canCounterHit && this->_counterHit != 2 && !myData->dFlag.superarmor) {
			game->soundMgr.play(BASICSOUND_COUNTER_HIT);
			if (this->_isGrounded() && data.counterHitSpeed.y <= 0)
				this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
			else
				this->_forceStartMove(ACTION_AIR_HIT);
			this->_speedReset = data.oFlag.resetSpeed;
			damage *= 1.5;
			this->_totalDamage += damage;
			this->_counter = true;
			this->_comboCtr++;
			this->_blockStun = data.hitStun * 1.5;
			this->_speed.x = -data.counterHitSpeed.x * this->_dir;
			this->_speed.y =  data.counterHitSpeed.y;
			this->_prorate *= data.prorate / 100;
			this->_limit[0] += data.neutralLimit;
			this->_limit[1] += data.voidLimit;
			this->_limit[2] += data.matterLimit;
			this->_limit[3] += data.spiritLimit;
			game->battleMgr->addHitStop(data.hitStop * 1.5);
			game->logger.debug("Counter hit !: " + std::to_string(this->_blockStun) + " hitstun frames");
		} else {
			game->soundMgr.play(data.hitSoundHandle);
			if (!myData->dFlag.superarmor) {
				if (this->_isGrounded() && data.hitSpeed.y <= 0)
					this->_forceStartMove(myData->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
				else
					this->_forceStartMove(ACTION_AIR_HIT);
				this->_totalDamage += damage;
				this->_comboCtr++;
				this->_blockStun = data.hitStun;
				this->_speed.x = -data.hitSpeed.x * this->_dir;
				this->_speed.y =  data.hitSpeed.y;
				this->_prorate *= data.prorate / 100;
				this->_limit[0] += data.neutralLimit;
				this->_limit[1] += data.voidLimit;
				this->_limit[2] += data.matterLimit;
				this->_limit[3] += data.spiritLimit;
				this->_speedReset = data.oFlag.resetSpeed;
			}
			game->battleMgr->addHitStop(data.hitStop);
			game->logger.debug(std::to_string(this->_blockStun) + " hitstun frames");
		}
		this->_hp -= damage;
	}

	void Character::_processWallSlams()
	{
		if (this->_position.x < 0) {
			this->_position.x = 0;
			if (std::abs(this->_speed.x) >= WALL_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT)
			) {
				this->_blockStun += WALL_SLAM_HITSTUN_INCREASE;
				game->soundMgr.play(BASICSOUND_WALL_BOUNCE);
				this->_forceStartMove(ACTION_WALL_SLAM);
				this->_speed.x *= -0.15;
				this->_speed.y = 7.5;
			} else
				this->_speed.x = 0;
			return;
		}

		if (this->_position.x <= 1000)
			return;
		this->_position.x = 1000;
		if (std::abs(this->_speed.x) >= WALL_SLAM_THRESHOLD && (
			this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
		)) {
			this->_blockStun += WALL_SLAM_HITSTUN_INCREASE;
			game->soundMgr.play(BASICSOUND_WALL_BOUNCE);
			this->_forceStartMove(ACTION_WALL_SLAM);
			this->_speed.x *= -0.15;
			this->_speed.y = 7.5;
		} else
			this->_speed.x = 0;
	}

	void Character::_processGroundSlams()
	{
		if (this->_isGrounded()) {
			if (this->_position.y < 0)
				this->_position.y = 0;

			if (std::abs(this->_speed.y) >= GROUND_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
			)) {
				this->_speed.x *= 0.1;
				this->_speed.y *= -0.8;
				game->soundMgr.play(BASICSOUND_GROUND_SLAM);
				this->_forceStartMove(ACTION_GROUND_SLAM);
				this->_blockStun += GROUND_SLAM_HITSTUN_INCREASE;
			} else
				this->_speed.y = 0;
		} else if (this->_position.y > 1000) {
			this->_position.y = 1000;

			if (std::abs(this->_speed.y) >= GROUND_SLAM_THRESHOLD && (
				this->_action == ACTION_AIR_HIT || this->_action == ACTION_GROUND_HIGH_HIT || this->_action == ACTION_GROUND_LOW_HIT
			)) {
				this->_speed.x *= 0.1;
				this->_speed.y *= -0.8;
				game->soundMgr.play(BASICSOUND_GROUND_SLAM);
				this->_forceStartMove(ACTION_GROUND_SLAM);
				this->_blockStun += GROUND_SLAM_HITSTUN_INCREASE;
			} else
				this->_speed.y = 0;
		}
	}

	void Character::_calculateCornerPriority()
	{
		auto newPriority = (this->_position.x >= 1000) - (this->_position.x <= 0);

		this->_justGotCorner = newPriority && !this->_cornerPriority;
		if (newPriority && this->_justGotCorner) {
			if (newPriority == this->_opponent->_cornerPriority) {
				if (this->_team == 1 && this->_opponent->_justGotCorner && this->_opponent->_cornerPriority == 1) {
					this->_cornerPriority = newPriority;
					this->_opponent->_justGotCorner = false;
					this->_opponent->_cornerPriority = 0;
				} else
					this->_cornerPriority = 0;
			} else {
				this->_justGotCorner = newPriority && !this->_cornerPriority;
				this->_cornerPriority = newPriority;
			}
		} else if (!newPriority) {
			this->_justGotCorner = false;
			this->_cornerPriority = 0;
		}
	}

	void Character::_applyMoveAttributes()
	{
		auto data = this->getCurrentFrameData();

		Object::_applyMoveAttributes();
		if (!this->_hadUltimate && data->oFlag.ultimate) {
			game->soundMgr.play(BASICSOUND_ULTIMATE);
			this->_voidMana = 0;
			this->_matterMana = 0;
			this->_spiritMana = 0;
		}
		this->_hadUltimate = data->oFlag.ultimate;
		this->_ultimateUsed |= data->oFlag.ultimate;
		if (this->_speedReset)
			this->_speed = {0, 0};
		if (data->oFlag.voidMana)
			this->_voidMana -= data->manaCost;
		if (data->oFlag.spiritMana)
			this->_spiritMana -= data->manaCost;
		if (data->oFlag.matterMana)
			this->_matterMana -= data->manaCost;
		if (
			this->_voidMana < 0 ||
			this->_spiritMana < 0 ||
			this->_matterMana < 0
		) {
			this->_voidMana = this->_voidManaMax / 10;
			this->_spiritMana = this->_spiritManaMax / 10;
			this->_matterMana = this->_matterManaMax / 10;
			if (this->_isGrounded()) {
				this->_blockStun = 60;
				this->_forceStartMove(data->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
				this->_speed = {this->_dir * -1, 0};
			} else {
				this->_blockStun = 12000;
				this->_forceStartMove(ACTION_AIR_HIT);
				this->_speed = {this->_dir * -1, 20};
			}
			game->soundMgr.play(BASICSOUND_GUARD_BREAK);
		}
	}

	std::pair<unsigned, std::shared_ptr<IObject>> Character::_spawnSubobject(unsigned id, bool needRegister)
	{
		auto data = this->getCurrentFrameData();
		auto pos = this->_position + Vector2i{
			0,
			data->offset.y
		} + data->size / 2;

		try {
			return game->battleMgr->registerObject<Projectile>(
				needRegister,
				this->_subObjectsData.at(id),
				this->_team,
				this->_direction,
				pos,
				this->_team,
				id
			);
		} catch (std::out_of_range &e) {
			throw std::invalid_argument("Cannot find subobject id " + std::to_string(id));
		}
	}

	const std::shared_ptr<IInput> &Character::getInput() const
	{
		return this->_input;
	}

	std::shared_ptr<IInput> &Character::getInput()
	{
		return this->_input;
	}

	const std::map<unsigned, std::vector<std::vector<FrameData>>> &Character::getFrameData()
	{
		return this->_moves;
	}

	void Character::setAttacksDisabled(bool disabled)
	{
		this->_atkDisabled = disabled;
	}

	void Character::disableInputs(bool disabled)
	{
		this->_inputDisabled = disabled;
	}

	unsigned int Character::getBufferSize() const
	{
		return Object::getBufferSize() + sizeof(Data) + sizeof(LastInput) * this->_lastInputs.size() + this->_replayData.size() * sizeof(ReplayData);
	}

	void Character::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());
		size_t i = 0;

		Object::copyToBuffer(data);
#ifdef _DEBUG
		game->logger.debug("Saving Character (Data size: " + std::to_string(sizeof(Data) + sizeof(LastInput) * this->_lastInputs.size()) + ") @" + std::to_string((uintptr_t)dat));
#endif
		dat->_jumpCanceled = this->_jumpCanceled;
		dat->_hadUltimate = this->_hadUltimate;
		dat->_supersUsed = this->_supersUsed;
		dat->_skillsUsed = this->_skillsUsed;
		dat->_grabInvul = this->_grabInvul;
		dat->_ultimateUsed = this->_ultimateUsed;
		dat->_nbReplayInputs = this->_replayData.size();
		dat->_inputBuffer = this->_inputBuffer;
		dat->_speedReset = this->_speedReset;
		dat->_guardRegenCd = this->_guardRegenCd;
		dat->_odCooldown = this->_odCooldown;
		dat->_counter = this->_counter;
		dat->_blockStun = this->_blockStun;
		dat->_jumpsUsed = this->_jumpsUsed;
		dat->_airDashesUsed = this->_airDashesUsed;
		dat->_comboCtr = this->_comboCtr;
		dat->_totalDamage = this->_totalDamage;
		dat->_prorate = this->_prorate;
		dat->_atkDisabled = this->_atkDisabled;
		dat->_inputDisabled = this->_inputDisabled;
		dat->_hasJumped = this->_hasJumped;
		dat->_restand = this->_restand;
		dat->_justGotCorner = this->_justGotCorner;
		dat->_regen = this->_regen;
		dat->_voidMana = this->_voidMana;
		dat->_spiritMana = this->_spiritMana;
		dat->_matterMana = this->_matterMana;
		dat->_guardCooldown = this->_guardCooldown;
		dat->_guardBar = this->_guardBar;
		memcpy(dat->_specialInputs, this->_specialInputs._value, sizeof(dat->_specialInputs));
		dat->_nbLastInputs = this->_lastInputs.size();
		for (i = 0; i < this->_limit.size(); i++)
			dat->_limit[i] = this->_limit[i];
		i = 0;
		for (auto it = this->_lastInputs.begin(); it != this->_lastInputs.end(); it++, i++)
			((LastInput *)&dat[1])[i] = *it;
		for (i = 0; i < this->_subobjects.size(); i++) {
			if (this->_subobjects[i].first && this->_subobjects[i].second && !this->_subobjects[i].second->isDead())
				dat->_subObjects[i] = this->_subobjects[i].first;
			else
				dat->_subObjects[i] = 0;
		}
		memcpy(&((LastInput *)&dat[1])[dat->_nbLastInputs], this->_replayData.data(), this->_replayData.size() * sizeof(ReplayData));
	}

	void Character::restoreFromBuffer(void *data)
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Object::getBufferSize());

		Object::restoreFromBuffer(data);
		this->_jumpCanceled = dat->_jumpCanceled;
		this->_hadUltimate = dat->_hadUltimate;
		this->_supersUsed = dat->_supersUsed;
		this->_skillsUsed = dat->_skillsUsed;
		this->_grabInvul = dat->_grabInvul;
		this->_ultimateUsed = dat->_ultimateUsed;
		this->_inputBuffer = dat->_inputBuffer;
		this->_speedReset = dat->_speedReset;
		this->_guardRegenCd = dat->_guardRegenCd;
		this->_odCooldown = dat->_odCooldown;
		this->_counter = dat->_counter;
		this->_blockStun = dat->_blockStun;
		this->_jumpsUsed = dat->_jumpsUsed;
		this->_airDashesUsed = dat->_airDashesUsed;
		this->_comboCtr = dat->_comboCtr;
		this->_totalDamage = dat->_totalDamage;
		this->_prorate = dat->_prorate;
		this->_atkDisabled = dat->_atkDisabled;
		this->_inputDisabled = dat->_inputDisabled;
		this->_hasJumped = dat->_hasJumped;
		this->_restand = dat->_restand;
		this->_justGotCorner = dat->_justGotCorner;
		this->_regen = dat->_regen;
		this->_voidMana = dat->_voidMana;
		this->_spiritMana = dat->_spiritMana;
		this->_matterMana = dat->_matterMana;
		this->_guardCooldown = dat->_guardCooldown;
		this->_guardBar = dat->_guardBar;
		memcpy(this->_specialInputs._value, dat->_specialInputs, sizeof(dat->_specialInputs));
		this->_lastInputs.clear();
		for (size_t i = 0; i < dat->_nbLastInputs; i++)
			this->_lastInputs.push_back(((LastInput *)&dat[1])[i]);
		for (size_t i = 0; i < this->_limit.size(); i++)
			this->_limit[i] = dat->_limit[i];
		for (size_t i = 0; i < this->_subobjects.size(); i++) {
			this->_subobjects[i].first = dat->_subObjects[i];
			this->_subobjects[i].second.reset();
		}
		this->_replayData.clear();
		this->_replayData.reserve(dat->_nbReplayInputs);
		for (size_t i = 0; i < dat->_nbReplayInputs; i++)
			this->_replayData.push_back((
				(ReplayData *)(&(
					(LastInput *)&dat[1]
				)[dat->_nbLastInputs])
			)[i]);
	}

	void Character::resolveSubObjects(const BattleManager &manager)
	{
		for (auto &subobject : this->_subobjects)
			if (subobject.first)
				subobject.second = manager.getObjectFromId(subobject.first);
	}

	unsigned int Character::getClassId() const
	{
		return 1;
	}

	void Character::_removeSubobjects()
	{
		for (auto &obj : this->_subobjects)
			obj.second.reset();
	}

	void Character::_processGuardLoss(bool wrongBlock)
	{
		auto loss = this->_blockStun * 4;

		if (this->_guardCooldown)
			return;
		if (loss >= this->_guardBar) {
			game->soundMgr.play(BASICSOUND_GUARD_BREAK);
			this->_guardBar = this->_maxGuardBar;
			this->_guardCooldown = this->_maxGuardCooldown;
		} else
			this->_guardBar -= loss;
	}

	void Character::_parryEffect(Object *other)
	{
		auto data = this->getCurrentFrameData();
		auto oData = other->getCurrentFrameData();
		bool isStrongest = (oData->oFlag.matterElement && data->dFlag.voidBlock) ||
			(oData->oFlag.voidElement && data->dFlag.spiritBlock) ||
			(oData->oFlag.spiritElement && data->dFlag.matterBlock) ||
			(oData->oFlag.matterElement == oData->oFlag.voidElement && oData->oFlag.voidElement == oData->oFlag.spiritElement && data->dFlag.neutralBlock);

		if (data->dFlag.spiritBlock) {
			this->_blockStun += 5;
			other->_speed.x += this->_dir * (2 + isStrongest * 5);
			this->_speed.x -= this->_dir * (2 + isStrongest * 5);
		}
		if (data->dFlag.voidBlock)
			other->_speed.x -= this->_dir * (5 + isStrongest * 10);
		if (data->dFlag.matterBlock) {
			auto chr = dynamic_cast<Character *>(other);

			if (chr) {
				if (data->oFlag.voidMana)
					chr->_voidMana -= chr->getCurrentFrameData()->oFlag.voidElement * (50 + isStrongest * 50);
				if (data->oFlag.spiritMana)
					chr->_spiritMana -= chr->getCurrentFrameData()->oFlag.spiritElement * (50 + isStrongest * 50);
				if (data->oFlag.matterMana)
					chr->_matterMana -= chr->getCurrentFrameData()->oFlag.matterElement * (50 + isStrongest * 50);
				if (
					chr->_voidMana < 0 ||
					chr->_spiritMana < 0 ||
					chr->_matterMana < 0
				) {
					chr->_voidMana = chr->_voidManaMax / 10;
					chr->_spiritMana = chr->_spiritManaMax / 10;
					chr->_matterMana = chr->_matterManaMax / 10;
					if (this->_isGrounded()) {
						chr->_blockStun = 60;
						chr->_forceStartMove(data->dFlag.crouch ? ACTION_GROUND_LOW_HIT : ACTION_GROUND_HIGH_HIT);
						chr->_speed = {chr->_dir * -1, 0};
					} else {
						chr->_blockStun = 12000;
						chr->_forceStartMove(ACTION_AIR_HIT);
						chr->_speed = {chr->_dir * -1, 20};
					}
					game->soundMgr.play(BASICSOUND_GUARD_BREAK);
				}
			} else {
				other->_team = this->_team;
				other->_speed.x *= -1;
				other->_dir *= -1;
				other->_direction = !other->_direction;
			}
		}
		memset(&this->_specialInputs, 0, sizeof(this->_specialInputs));
		memset(&this->_inputBuffer, 0, sizeof(this->_inputBuffer));
		if (isStrongest)
			game->soundMgr.play(BASICSOUND_BEST_PARRY);
		if (data->dFlag.neutralBlock) {
			this->_forceStartMove(this->_getReversalAction());
			this->_blockStun = 0;
		} else if (isStrongest) {
			this->_forceStartMove(this->_isGrounded() ? (data->dFlag.crouch ? ACTION_CROUCH : ACTION_IDLE) : ACTION_FALLING);
			this->_blockStun = 0;
		} else {
			game->soundMgr.play(BASICSOUND_BLOCK);
			this->_forceStartMove(this->_isGrounded() ? (data->dFlag.crouch ? ACTION_GROUND_LOW_NEUTRAL_BLOCK : ACTION_GROUND_HIGH_NEUTRAL_BLOCK) : ACTION_AIR_NEUTRAL_BLOCK);
		}
	}

	unsigned Character::_getReversalAction()
	{
		return this->_isGrounded() ? (
			this->getCurrentFrameData()->dFlag.crouch ?
			ACTION_GROUND_LOW_REVERSAL :
			ACTION_GROUND_HIGH_REVERSAL
		) : ACTION_AIR_REVERSAL;
	}

	InputStruct Character::_getInputs()
	{
		auto result = this->_input->getInputs();

		if (this->_dummyState == DUMMYSTATE_JUMP)
			result.verticalAxis = 1;
		if (this->_dummyState == DUMMYSTATE_CROUCH)
			result.verticalAxis = -1;
		return result;
	}

	bool Character::isBlockingAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_HIGH_NEUTRAL_BLOCK:
		case ACTION_GROUND_LOW_NEUTRAL_BLOCK:
		case ACTION_AIR_NEUTRAL_BLOCK:
		case ACTION_GROUND_HIGH_NEUTRAL_WRONG_BLOCK:
		case ACTION_GROUND_LOW_NEUTRAL_WRONG_BLOCK:
		case ACTION_AIR_NEUTRAL_WRONG_BLOCK:
			return true;
		default:
			return false;
		}
	}

	bool Character::isParryAction(unsigned int action)
	{
		switch (action) {
		case ACTION_GROUND_HIGH_NEUTRAL_PARRY:
		case ACTION_GROUND_HIGH_SPIRIT_PARRY:
		case ACTION_GROUND_HIGH_MATTER_PARRY:
		case ACTION_GROUND_HIGH_VOID_PARRY:
		case ACTION_GROUND_LOW_NEUTRAL_PARRY:
		case ACTION_GROUND_LOW_SPIRIT_PARRY:
		case ACTION_GROUND_LOW_MATTER_PARRY:
		case ACTION_GROUND_LOW_VOID_PARRY:
		case ACTION_AIR_NEUTRAL_PARRY:
		case ACTION_AIR_SPIRIT_PARRY:
		case ACTION_AIR_MATTER_PARRY:
		case ACTION_AIR_VOID_PARRY:
			return true;
		default:
			return false;
		}
	}

	bool Character::isOverdriveAction(unsigned int action)
	{
		switch (action) {
		case ACTION_NEUTRAL_OVERDRIVE:
		case ACTION_MATTER_OVERDRIVE:
		case ACTION_SPIRIT_OVERDRIVE:
		case ACTION_VOID_OVERDRIVE:
		case ACTION_NEUTRAL_AIR_OVERDRIVE:
		case ACTION_MATTER_AIR_OVERDRIVE:
		case ACTION_SPIRIT_AIR_OVERDRIVE:
		case ACTION_VOID_AIR_OVERDRIVE:
			return true;
		default:
			return false;
		}
	}

	bool Character::isRomanCancelAction(unsigned int action)
	{
		switch (action) {
		case ACTION_NEUTRAL_ROMAN_CANCEL:
		case ACTION_VOID_ROMAN_CANCEL:
		case ACTION_SPIRIT_ROMAN_CANCEL:
		case ACTION_MATTER_ROMAN_CANCEL:
		case ACTION_NEUTRAL_AIR_ROMAN_CANCEL:
		case ACTION_VOID_AIR_ROMAN_CANCEL:
		case ACTION_SPIRIT_AIR_ROMAN_CANCEL:
		case ACTION_MATTER_AIR_ROMAN_CANCEL:
			return true;
		default:
			return false;
		}
	}

	const std::vector<Character::ReplayData> &Character::getReplayData() const
	{
		return this->_replayData;
	}

	bool Character::_executeNeutralAttack(unsigned int base)
	{
		//5X, 2X, 6X, 3X, 8X
		char tries[] = {0, 4, 1, 3, 2};

		for (auto tr : tries)
			if (this->_hasMove(base + tr))
				return this->_startMove(base + tr);
		return false;
	}

	bool Character::_executeDownAttack(unsigned int base)
	{
		return this->_startMove(base - !this->_hasMove(base));
	}

	void Character::_applyNewAnimFlags()
	{
		auto data = this->getCurrentFrameData();

		Object::_applyNewAnimFlags();
		if (!this->_ultimateUsed && data->oFlag.ultimate) {
			game->soundMgr.play(BASICSOUND_ULTIMATE);
			this->_voidMana = 0;
			this->_matterMana = 0;
			this->_spiritMana = 0;
		}
		this->_ultimateUsed |= data->oFlag.ultimate;
		if (data->subObjectSpawn > 0) {
			if (data->subObjectSpawn <= 64 && this->_subobjects[data->subObjectSpawn - 1].first)
				return;
			else if (data->subObjectSpawn <= 128 && this->_subobjects[data->subObjectSpawn - 1].first)
				this->_subobjects[data->subObjectSpawn - 1].second->kill();

			auto obj = this->_spawnSubobject(data->subObjectSpawn - 1);

			if (data->subObjectSpawn > 128)
				return;
			this->_subobjects[data->subObjectSpawn - 1] = obj;
		}
	}
}
