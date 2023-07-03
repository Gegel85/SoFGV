//
// Created by PinkySmile on 06/05/23.
//

#ifndef SOFGV_MATTERSHADOW_HPP
#define SOFGV_MATTERSHADOW_HPP


#include "Objects/Characters/VictoriaStar/Shadow.hpp"

namespace SpiralOfFate
{
	class MatterShadow : public Shadow {
	public:
		MatterShadow(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned hp,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned id,
			bool tint
		);

		static Shadow *create(
			const std::vector<std::vector<FrameData>> &frameData,
			unsigned int hp,
			bool direction,
			Vector2f pos,
			bool owner,
			class Character *ownerObj,
			unsigned int id,
			bool tint
		);

		void getHit(IObject &other, const FrameData *data) override;
	};
}


#endif //SOFGV_MATTERSHADOW_HPP
