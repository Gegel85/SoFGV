//
// Created by PinkySmile on 01/03/2022.
//

#include "Stickman.hpp"
#include "../../Resources/Game.hpp"

#define MIN_RANDOM_FLAGS 6
#define NEW_FLAG_STEP 5
#define MAX_CHARGE 40
#define HITSTUN_RATIO 2/3
#define BLOCKSTUN_RATIO 2/3

namespace SpiralOfFate
{
	Stickman::Stickman(
		unsigned index,
		const std::string &frameData,
		const std::string &subobjFrameData,
		const std::pair<std::vector<SpiralOfFate::Color>,
		std::vector<SpiralOfFate::Color>> &palette,
		std::shared_ptr<IInput> input
	) :
		Character(index, frameData, subobjFrameData, palette, input)
	{
		this->_fakeFrameData.setSlave();
		game->logger.debug("Stickman class created");
	}

	unsigned int Stickman::getClassId() const
	{
		return 3;
	}

	const FrameData *Stickman::getCurrentFrameData() const
	{
		auto data = Character::getCurrentFrameData();

		if (!this->_buffTimer)
			return data;
		this->_fakeFrameData = *data;
		this->_allyBuffEffect(this->_fakeFrameData);
		return &this->_fakeFrameData;
	}

	void Stickman::_forceStartMove(unsigned int action)
	{
		Character::_forceStartMove(action);
		if (action == ACTION_5A && this->_buffTimer)
			this->_actionBlock = 1;
	}

	bool Stickman::_canStartMove(unsigned int action, const FrameData &data)
	{
		if (this->_action == ACTION_5A)
			return this->_buffTimer != 0 || this->_guardCooldown == 0;
		return Character::_canStartMove(action, data);
	}

	void Stickman::_applyNewAnimFlags()
	{
		Character::_applyNewAnimFlags();
	}

	void Stickman::_applyMoveAttributes()
	{
		Character::_applyMoveAttributes();
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A) {
			auto data = this->getCurrentFrameData();

			if (data->specialMarker == 1) {
				this->_buff = this->_rand(game->random);
				this->_buffTimer = timers[this->_buff].first;
				this->_guardCooldown = timers[this->_buff].second;
				this->_guardBar = timers[this->_buff].second;
			} else if (data->specialMarker == 2)
				this->_buffTimer = 0;
		}
	}

	void Stickman::_onMoveEnd(const FrameData &lastData)
	{
		if (this->_action >= ACTION_5A && this->_action <= ACTION_c64A && this->_actionBlock == 1) {
			this->_animation = 0;
			this->_applyNewAnimFlags();
			return;
		}
		if (this->_action == ACTION_WIN_MATCH2 && this->_actionBlock == 1) {
			this->_actionBlock++;
			assert(this->_moves.at(this->_action).size() != this->_actionBlock);
			return Character::_onMoveEnd(lastData);
		}
		Character::_onMoveEnd(lastData);
	}

	void Stickman::update()
	{
		Character::update();
		this->_buffTimer -= !!this->_buffTimer;
	}

	unsigned int Stickman::getBufferSize() const
	{
		return Character::getBufferSize() + sizeof(Data);
	}

	void Stickman::copyToBuffer(void *data) const
	{
		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		Character::copyToBuffer(data);
#ifdef _DEBUG
		game->logger.debug("Saving Stickman (Data size: " + std::to_string(sizeof(Data)) + ") @" + std::to_string((uintptr_t)dat));
#endif
		dat->_buff = this->_buff;
		dat->_time = this->_time;
		dat->_oldAction = this->_oldAction;
		dat->_buffTimer = this->_buffTimer;
	}

	void Stickman::restoreFromBuffer(void *data)
	{
		Character::restoreFromBuffer(data);

		auto dat = reinterpret_cast<Data *>((uintptr_t)data + Character::getBufferSize());

		this->_buff = dat->_buff;
		this->_time = dat->_time;
		this->_oldAction = dat->_oldAction;
		this->_buffTimer = dat->_buffTimer;
#ifdef _DEBUG
		game->logger.debug("Restored Stickman @" + std::to_string((uintptr_t)dat));
#endif
	}

	std::pair<unsigned int, std::shared_ptr<IObject>> Stickman::_spawnSubobject(unsigned int id, bool needRegister)
	{
		auto data = this->getCurrentFrameData();
		auto pos = this->_position + Vector2i{
			0,
			data->offset.y
		} + data->size / 2;

		if (id == 128)
			return game->battleMgr->registerObject<Projectile>(
				needRegister,
				this->_subObjectsData.at(id),
				this->_team,
				this->_direction,
				pos,
				this->_team,
				id,
				4
			);
		return Character::_spawnSubobject(id, needRegister);
	}

	void Stickman::onMatchEnd()
	{
		Character::onMatchEnd();
		this->_oldAction = this->_action;
		if (this->_oldAction <= ACTION_WIN_MATCH2)
			game->soundMgr.play(BASICSOUND_DASH);
	}

	bool Stickman::matchEndUpdate()
	{
		if (this->_oldAction < ACTION_WIN_MATCH1)
			return false;
		if (this->_action < ACTION_WIN_MATCH1)
			this->_forceStartMove(this->_oldAction);
		if (!this->_isGrounded())
			this->_speed.x = 0;
		if (std::abs(this->_position.x - reinterpret_cast<Stickman *>(&*this->_opponent)->_position.x) < 30 && this->_actionBlock == 0) {
			this->_actionBlock++;
			this->_animation = 0;
			this->_animationCtr = 0;
			this->_speed.x = 0;
		}
		this->_time += this->_actionBlock;
		return this->_time < 30;
	}

	void Stickman::_mutateHitFramedata(FrameData &framedata) const
	{
		this->_enemyBuffEffect(framedata);
	}

	void Stickman::_allyBuffEffect(FrameData &framedata) const
	{

	}

	void Stickman::_enemyBuffEffect(FrameData &framedata) const
	{

	}
}
