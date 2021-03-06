//////////////////////////////////////////////////////////////////////////////
// This file is part of the Journey MMORPG client                           //
// Copyright © 2015-2016 Daniel Allendorf                                   //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "../UIDragElement.h"

#include "../Graphics/Text.h"
#include "../Character/Charstats.h"

namespace jrc
{
	class UIMiniMap : public UIDragElement<PosMINIMAP>
	{
	public:
		static constexpr Type TYPE = MINIMAP;
		static constexpr bool FOCUSED = false;
		static constexpr bool TOGGLED = true;

		UIMiniMap(const CharStats& stats);

		void draw(float alpha) const override;
		void update() override;

		void send_key(int32_t keycode, bool pressed) override;

	private:
		static constexpr int16_t center_start_x = 64;
		const CharStats& stats;

		void update_buttons();
		void toggle_buttons();
		void update_text();

		enum Buttons
		{
			BT_MAP,
			BT_MAX,
			BT_MIN,
			BT_NPC,
			BT_SMALL
		};

		enum Type
		{
			MIN,
			NORMAL,
			MAX
		};

		int32_t mapid;
		int8_t type;
		int16_t combined_text_width;
		Point<int16_t> bt_map_pos;
		Point<int16_t> bt_max_pos;
		Point<int16_t> bt_min_pos;
		int16_t bt_min_width;
		int16_t bt_max_width;
		int16_t bt_map_width;
		int16_t final_pos;
		Animation min_left;
		Animation min_center;
		Animation min_right;
		std::vector<Sprite> normal_sprites;
		std::vector<Sprite> max_sprites;
		Text combined_text;
		Text region_text;
		Text town_text;
	};
}