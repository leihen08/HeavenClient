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

#include "../Components/Slider.h"
#include "../Template/BoolPair.h"
#include "../Graphics/Text.h"
#include "../Graphics/SpecialText.h"

namespace jrc
{
	class UIEvent : public UIDragElement<PosEVENT>
	{
	public:
		static constexpr Type TYPE = EVENT;
		static constexpr bool FOCUSED = false;
		static constexpr bool TOGGLED = true;

		UIEvent();

		void draw(float inter) const override;
		void update() override;

		bool remove_cursor(bool clicked, Point<int16_t> cursorpos) override;
		Cursor::State send_cursor(bool clicked, Point<int16_t> cursorpos) override;
		void send_key(int32_t keycode, bool pressed) override;

	protected:
		Button::State button_pressed(uint16_t buttonid) override;

	private:
		void clear_tooltip();
		void close();
		std::string get_event_title(uint8_t id);
		std::string get_event_date(uint8_t id);
		int16_t row_by_position(int16_t y);
		int16_t col_by_position(int16_t x);
		void show_item(int16_t row, int16_t col);

		enum Buttons : uint16_t
		{
			CLOSE
		};

		int16_t offset;
		int16_t event_count;
		ShadowText event_title[3];
		Text event_date[3];
		Slider slider;
		Texture item_reward;
		Texture text_reward;
		Texture next;
		Texture label_on;
		Texture label_next;
		std::vector<BoolPair<bool>> events;
	};
}