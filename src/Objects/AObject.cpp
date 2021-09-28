//
// Created by PinkySmile on 18/09/2021
//

#include "AObject.hpp"
#include "../Resources/Game.hpp"
#include "../Logger.hpp"

namespace Battle
{
	void AObject::render() const
	{
		auto &data = *this->getCurrentFrameData();
		auto scale = Vector2f{
			static_cast<float>(data.size.x) / data.textureBounds.size.x,
			static_cast<float>(data.size.y) / data.textureBounds.size.y
		};
		auto result = this->_position + Vector2f{ data.offset.x * this->_dir, static_cast<float>(data.offset.y) };
		auto realPos = this->_position;

		realPos.y *= -1;
		result.y *= -1;
		result += Vector2f{
			-this->_dir * data.size.x / 2,
			-static_cast<float>(data.size.y)
		};
		this->_sprite.setPosition(result);
		this->_sprite.setScale(this->_dir * scale.x, scale.y);
		this->_sprite.textureHandle = data.textureHandle;
		this->_sprite.setTextureRect(data.textureBounds);
		game.textureMgr.render(this->_sprite);
		if (this->showBoxes) {
			sf::RectangleShape rect;

			rect.setOutlineThickness(1);
			rect.setOutlineColor(sf::Color{0x00, 0xFF, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0x00, 0xFF, 0x00, 0x60});
			for (auto &hurtBox : data.hurtBoxes) {
				auto box = this->_applyModifiers(hurtBox);

				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOutlineColor(sf::Color{0xFF, 0x00, 0x00, 0xFF});
			rect.setFillColor(sf::Color{0xFF, 0x00, 0x00, 0x60});
			for (auto &hitBox : data.hitBoxes) {
				auto box = this->_applyModifiers(hitBox);

				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			if (data.collisionBox) {
				auto box = this->_applyModifiers(*data.collisionBox);

				rect.setOutlineColor(sf::Color{0xFF, 0xFF, 0x00, 0xFF});
				rect.setFillColor(sf::Color{0xFF, 0xFF, 0x00, 0x60});
				rect.setPosition(box.pos + realPos);
				rect.setSize(box.size);
				game.screen->draw(rect);
			}

			rect.setOutlineThickness(2);
			rect.setOutlineColor(sf::Color::White);
			rect.setFillColor(sf::Color::Black);
			rect.setPosition(realPos - Vector2f{4, 4});
			rect.setSize({9, 9});
			game.screen->draw(rect);
		}
	}

	void AObject::update()
	{
		auto *data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];

		this->_animationCtr++;
		while (this->_animationCtr >= data->duration) {
			this->_animationCtr = 0;
			this->_animation++;
			this->_hasHit &= this->_animation < this->_moves.at(this->_action)[this->_actionBlock].size();
			if (this->_animation == this->_moves.at(this->_action)[this->_actionBlock].size())
				this->_onMoveEnd(this->_moves.at(this->_action)[this->_actionBlock].back());
			data = &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
			this->_hasHit &= !data->oFlag.resetHits;
		}
		if (data->oFlag.resetSpeed)
			this->_speed = {0, 0};
		this->_speed += Vector2f{this->_dir * data->speed.x, static_cast<float>(data->speed.y)};
		this->_position += this->_speed;
		if (!this->_isGrounded()) {
			this->_speed *= 0.99;
			this->_speed += this->_gravity;
		} else
			this->_speed *= 0.75;
	}

	void AObject::reset()
	{
		this->_rotation = this->_baseRotation;
		this->_gravity = this->_baseGravity;
		this->_hp = this->_baseHp;
		this->_airDrag = this->_baseAirDrag;
		this->_groundDrag = this->_baseGroundDrag;
	}

	bool AObject::isDead() const
	{
		return this->_dead;
	}

	void AObject::hit(IObject &other, const FrameData *)
	{
		char buffer[36];

		sprintf(buffer, "0x%08llX has hit 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
		this->_hasHit = true;
	}

	void AObject::getHit(IObject &other, const FrameData *)
	{
		char buffer[38];

		sprintf(buffer, "0x%08llX is hit by 0x%08llX", (unsigned long long)this, (unsigned long long)&other);
		logger.debug(buffer);
	}

	bool AObject::hits(IObject &other) const
	{
		auto *oData = other.getCurrentFrameData();
		auto *mData = this->getCurrentFrameData();

		if (!mData || !oData || this->_hasHit)
			return false;

		auto asAObject = dynamic_cast<AObject *>(&other);

		if (asAObject && asAObject->_team == this->_team)
			return false;

		for (auto &hurtBox : oData->hurtBoxes)
			for (auto &hitBox : mData->hitBoxes) {
				auto _hitBox = this->_applyModifiers(hitBox);
				auto _hurtBox = asAObject->_applyModifiers(hurtBox);

				_hitBox.pos.x += this->_position.x;
				_hitBox.pos.y -= this->_position.y;
				_hurtBox.pos.x += asAObject->_position.x;
				_hurtBox.pos.y += asAObject->_position.y;
				if (
					static_cast<float>(_hurtBox.pos.x)                   < static_cast<float>(_hitBox.pos.x) + _hitBox.size.x &&
					static_cast<float>(_hurtBox.pos.y)                   < static_cast<float>(_hitBox.pos.y) + _hitBox.size.y &&
					static_cast<float>(_hurtBox.pos.x) + _hurtBox.size.x > static_cast<float>(_hitBox.pos.x)                  &&
					static_cast<float>(_hurtBox.pos.y) + _hurtBox.size.y > static_cast<float>(_hitBox.pos.y)
				)
					return true;
			}
		return false;
	}

	const FrameData *AObject::getCurrentFrameData() const
	{
		return &this->_moves.at(this->_action)[this->_actionBlock][this->_animation];
	}

	Box AObject::_applyModifiers(Box box) const
	{
		if (this->_direction)
			return box;

		return Box{
			{static_cast<int>(-box.pos.x - box.size.x), box.pos.y},
			box.size
		};
	}

	void AObject::_applyNewAnimFlags()
	{
		auto data = this->getCurrentFrameData();

		if (!data)
			return;
		this->_hasHit &= data->oFlag.resetHits;
		if (data->dFlag.resetRotation)
			this->_rotation = 0;
	}

	bool AObject::_hasMove(unsigned action) const
	{
		return this->_moves.find(action) != this->_moves.end();
	}

	bool AObject::_startMove(unsigned int action)
	{
		if (!this->_hasMove(action))
			return false;

		auto &data = this->_moves.at(action)[0][0];

		if (!this->_canStartMove(action, data))
			return false;
		this->_forceStartMove(action);
		return true;
	}

	bool AObject::_canStartMove(unsigned action, const FrameData &data)
	{
		return true;
	}

	void AObject::_forceStartMove(unsigned int action)
	{
		this->_action = action;
		this->_actionBlock = 0;
		this->_animationCtr = 0;
		this->_animation = 0;
		this->_hasHit = false;
		this->_applyNewAnimFlags();
	}

	void AObject::_onMoveEnd(FrameData &)
	{
		this->_animation = 0;
		this->_applyNewAnimFlags();
	}

	bool AObject::_isGrounded() const
	{
		auto data = this->getCurrentFrameData();

		return !data || !data->dFlag.airborne;
	}
}
