//
// Created by PinkySmile on 18/09/2021
//

#ifndef BATTLE_ACHARACTER_HPP
#define BATTLE_ACHARACTER_HPP


#include <list>
#include "AObject.hpp"
#include "../Inputs/IInput.hpp"

namespace Battle
{
	enum CharacterActions {
		/*  0  */ ACTION_IDLE,
		/*  1  */ ACTION_CROUCHING,
		/*  2  */ ACTION_CROUCH,
		/*  3  */ ACTION_STANDING_UP,
		/*  4  */ ACTION_WALK_FORWARD,
		/*  5  */ ACTION_WALK_BACKWARD,
		/*  6  */ ACTION_FORWARD_DASH,
		/*  7  */ ACTION_BACKWARD_DASH,
		/*  8  */ ACTION_NEUTRAL_JUMP,
		/*  9  */ ACTION_FORWARD_JUMP,
		/* 10  */ ACTION_BACKWARD_JUMP,
		/* 11  */ ACTION_NEUTRAL_HIGH_JUMP,
		/* 12  */ ACTION_FORWARD_HIGH_JUMP,
		/* 13  */ ACTION_BACKWARD_HIGH_JUMP,
		/* 14  */ ACTION_FALLING,
		/* 15  */ ACTION_LANDING,
		/* 16  */ ACTION_AIR_DASH_1,
		/* 17  */ ACTION_AIR_DASH_2,
		/* 18  */ ACTION_AIR_DASH_3,
		/* 19  */ ACTION_AIR_DASH_4,
		/* 20  */ ACTION_AIR_DASH_6,
		/* 21  */ ACTION_AIR_DASH_7,
		/* 22  */ ACTION_AIR_DASH_8,
		/* 23  */ ACTION_AIR_DASH_9,
		/* 24  */ ACTION_AIR_TRANSFORM,
		/* 25  */ ACTION_GROUND_HIGH_NEUTRAL_BLOCK,
		/* 26  */ ACTION_GROUND_HIGH_SPIRIT_BLOCK,
		/* 27  */ ACTION_GROUND_HIGH_MATTER_BLOCK,
		/* 28  */ ACTION_GROUND_HIGH_VOID_BLOCK,
		/* 29  */ ACTION_GROUND_HIGH_HIT,
		/* 30  */ ACTION_GROUND_LOW_NEUTRAL_BLOCK,
		/* 31  */ ACTION_GROUND_LOW_SPIRIT_BLOCK,
		/* 32  */ ACTION_GROUND_LOW_MATTER_BLOCK,
		/* 33  */ ACTION_GROUND_LOW_VOID_BLOCK,
		/* 34  */ ACTION_GROUND_LOW_HIT,
		/* 35  */ ACTION_AIR_NEUTRAL_BLOCK,
		/* 36  */ ACTION_AIR_SPIRIT_BLOCK,
		/* 37  */ ACTION_AIR_MATTER_BLOCK,
		/* 38  */ ACTION_AIR_VOID_BLOCK,
		/* 39  */ ACTION_AIR_HIT,
		/* 40  */ ACTION_BEING_KNOCKED_DOWN,
		/* 41  */ ACTION_KNOCKED_DOWN,
		/* 42  */ ACTION_NEUTRAL_TECH,
		/* 43  */ ACTION_BACKWARD_TECH,
		/* 44  */ ACTION_FORWARD_TECH,
		/* 45  */ ACTION_FALLING_TECH,
		/* 46  */ ACTION_UP_AIR_TECH,
		/* 47  */ ACTION_DOWN_AIR_TECH,
		/* 48  */ ACTION_FORWARD_AIR_TECH,
		/* 49  */ ACTION_BACKWARD_AIR_TECH,
		/* 50  */ ACTION_AIR_TECH_LANDING_LAG,
		/* 51  */ ACTION_UNTRANSFORM,
		/* 52  */ ACTION_GROUND_SLAM,
		/* 53  */ ACTION_WALL_SLAM,

		/* 100 */ ACTION_5N = 100,
		/* 101 */ ACTION_6N,
		/* 102 */ ACTION_8N,
		/* 103 */ ACTION_3N,
		/* 104 */ ACTION_2N,
		/* 105 */ ACTION_214N,
		/* 106 */ ACTION_236N,
		/* 107 */ ACTION_421N,
		/* 108 */ ACTION_623N,
		/* 109 */ ACTION_41236N,
		/* 110 */ ACTION_63214N,
		/* 111 */ ACTION_6321469874N,
		/* 112 */ ACTION_j5N,
		/* 113 */ ACTION_j6N,
		/* 114 */ ACTION_j8N,
		/* 115 */ ACTION_j3N,
		/* 116 */ ACTION_j2N,
		/* 117 */ ACTION_j214N,
		/* 118 */ ACTION_j236N,
		/* 119 */ ACTION_j421N,
		/* 120 */ ACTION_j623N,
		/* 121 */ ACTION_j41236N,
		/* 122 */ ACTION_j63214N,
		/* 123 */ ACTION_j6321469874N,
		/* 124 */ ACTION_t5N,
		/* 125 */ ACTION_t6N,
		/* 126 */ ACTION_t8N,
		/* 127 */ ACTION_t2N,

		/* 150 */ ACTION_5M = 150,
		/* 151 */ ACTION_6M,
		/* 152 */ ACTION_8M,
		/* 153 */ ACTION_3M,
		/* 154 */ ACTION_2M,
		/* 155 */ ACTION_214M,
		/* 156 */ ACTION_236M,
		/* 157 */ ACTION_421M,
		/* 158 */ ACTION_623M,
		/* 159 */ ACTION_41236M,
		/* 160 */ ACTION_63214M,
		/* 161 */ ACTION_6321469874M,
		/* 162 */ ACTION_j5M,
		/* 163 */ ACTION_j6M,
		/* 164 */ ACTION_j8M,
		/* 165 */ ACTION_j3M,
		/* 166 */ ACTION_j2M,
		/* 167 */ ACTION_j214M,
		/* 168 */ ACTION_j236M,
		/* 169 */ ACTION_j421M,
		/* 170 */ ACTION_j623M,
		/* 171 */ ACTION_j41236M,
		/* 172 */ ACTION_j63214M,
		/* 173 */ ACTION_j6321469874M,
		/* 174 */ ACTION_t5M,
		/* 175 */ ACTION_t6M,
		/* 176 */ ACTION_t8M,
		/* 177 */ ACTION_t2M,

		/* 200 */ ACTION_5S = 200,
		/* 201 */ ACTION_6S,
		/* 202 */ ACTION_8S,
		/* 203 */ ACTION_3S,
		/* 204 */ ACTION_2S,
		/* 205 */ ACTION_214S,
		/* 206 */ ACTION_236S,
		/* 207 */ ACTION_421S,
		/* 208 */ ACTION_623S,
		/* 209 */ ACTION_41236S,
		/* 210 */ ACTION_63214S,
		/* 211 */ ACTION_6321469874S,
		/* 212 */ ACTION_j5S,
		/* 213 */ ACTION_j6S,
		/* 214 */ ACTION_j8S,
		/* 215 */ ACTION_j3S,
		/* 216 */ ACTION_j2S,
		/* 217 */ ACTION_j214S,
		/* 218 */ ACTION_j236S,
		/* 219 */ ACTION_j421S,
		/* 220 */ ACTION_j623S,
		/* 221 */ ACTION_j41236S,
		/* 222 */ ACTION_j63214S,
		/* 223 */ ACTION_j6321469874S,
		/* 224 */ ACTION_t5S,
		/* 225 */ ACTION_t6S,
		/* 226 */ ACTION_t8S,
		/* 227 */ ACTION_t2S,

		/* 250 */ ACTION_5V = 250,
		/* 251 */ ACTION_6V,
		/* 252 */ ACTION_8V,
		/* 253 */ ACTION_3V,
		/* 254 */ ACTION_2V,
		/* 255 */ ACTION_214V,
		/* 256 */ ACTION_236V,
		/* 257 */ ACTION_421V,
		/* 258 */ ACTION_623V,
		/* 259 */ ACTION_41236V,
		/* 260 */ ACTION_63214V,
		/* 261 */ ACTION_6321469874V,
		/* 262 */ ACTION_j5V,
		/* 263 */ ACTION_j6V,
		/* 264 */ ACTION_j8V,
		/* 265 */ ACTION_j3V,
		/* 266 */ ACTION_j2V,
		/* 267 */ ACTION_j214V,
		/* 268 */ ACTION_j236V,
		/* 269 */ ACTION_j421V,
		/* 270 */ ACTION_j623V,
		/* 271 */ ACTION_j41236V,
		/* 272 */ ACTION_j63214V,
		/* 273 */ ACTION_j6321469874V,
		/* 274 */ ACTION_t5V,
		/* 275 */ ACTION_t6V,
		/* 276 */ ACTION_t8V,
		/* 277 */ ACTION_t2V,

		/* 300 */ ACTION_5A = 300,
		/* 301 */ ACTION_6A,
		/* 302 */ ACTION_8A,
		/* 303 */ ACTION_3A,
		/* 304 */ ACTION_2A,
		/* 305 */ ACTION_214A,
		/* 306 */ ACTION_236A,
		/* 307 */ ACTION_421A,
		/* 308 */ ACTION_623A,
		/* 309 */ ACTION_41236A,
		/* 310 */ ACTION_63214A,
		/* 311 */ ACTION_6321469874A,
		/* 312 */ ACTION_j5A,
		/* 313 */ ACTION_j6A,
		/* 314 */ ACTION_j8A,
		/* 315 */ ACTION_j3A,
		/* 316 */ ACTION_j2A,
		/* 317 */ ACTION_j214A,
		/* 318 */ ACTION_j236A,
		/* 319 */ ACTION_j421A,
		/* 320 */ ACTION_j623A,
		/* 321 */ ACTION_j41236A,
		/* 322 */ ACTION_j63214A,
		/* 323 */ ACTION_j6321469874A,
		/* 324 */ ACTION_t5A,
		/* 325 */ ACTION_t6A,
		/* 326 */ ACTION_t8A,
		/* 327 */ ACTION_t2A,
	};

	class ACharacter : public AObject {
	private:
		struct LastInput {
			unsigned nbFrames : 28;
			unsigned h : 2;
			unsigned v : 2;
		};

		ACharacter *_opponent;
		std::shared_ptr<IInput> _input;
		std::list<LastInput> _lastInputs;
		unsigned _blockStun = 0;
		unsigned _jumpsUsed = 0;
		bool _hasJumped = false;
		bool _restand = false;
		union SpecialInputs {
			unsigned short _value = 0;
			struct {
				bool _236: 1;
				bool _214: 1;
				bool _623: 1;
				bool _421: 1;
				bool _624: 1;
				bool _426: 1;
				bool _6314: 1;
				bool _4136: 1;
				bool _624684: 1;
				bool _6314684: 1;
				bool _6246974: 1;
				bool _63146974: 1;
			};
		} _specialInputs;

		unsigned _maxJumps = 0;

		void _processInput(const InputStruct &input);
		bool _executeAirborneMoves(const InputStruct &input);
		bool _executeGroundMoves(const InputStruct &input);
		void _onMoveEnd(FrameData &lastData) override;
		bool _canStartMove(unsigned action, const FrameData &data) override;
		bool _isBlocking();
		bool _canCancel(unsigned int action);
		bool _isGrounded() const override;
		void _forceStartMove(unsigned action) override;
		int _getAttackTier(unsigned int action) const;

	public:
		ACharacter(const std::string &frameData, std::shared_ptr<IInput> input);
		~ACharacter() override = default;
		void setOpponent(ACharacter *opponent);
		void hit(IObject &other, const FrameData *data) override;
		void getHit(IObject &other, const FrameData *data) override;
		void render() const override;
		void update() override;
		void init(bool side, unsigned short maxHp, unsigned char maxJumps, Vector2f gravity);
		void consumeEvent(const sf::Event &event);
	};
}


#endif //BATTLE_ACHARACTER_HPP
