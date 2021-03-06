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
#include "UINotice.h"

#include "../UI.h"

#include "../Components/MapleButton.h"

#include "nlnx/nx.hpp"

namespace jrc
{
	UINotice::UINotice(std::string message, NoticeType t) : UIDragElement<PosNOTICE>(Point<int16_t>()), type(t)
	{
		nl::node src = nl::nx::ui["Basic.img"]["Notice6"];

		top = src["t"];
		center = src["c"];
		centerbox = src["c_box"];
		box = src["box"];
		box2 = src["box2"];
		bottom = src["s"];
		bottombox = src["s_box"];

		if (type == NoticeType::YESNO)
		{
			position.shift_y(-8);
			question = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE, message, 200);
		}
		else if (type == NoticeType::ENTERNUMBER)
		{
			position.shift_y(-16);
			question = Text(Text::Font::A12M, Text::Alignment::LEFT, Text::Color::WHITE, message, 200);
		}
		else if (type == NoticeType::OK || type == NoticeType::OKSMALL)
		{
			uint16_t maxwidth = 160;

			if (type == NoticeType::OK)
				maxwidth = top.width() - 6;

			if (type == NoticeType::OK)
				position.shift_y(-8);

			question = Text(Text::Font::A11M, Text::Alignment::CENTER, Text::Color::WHITE, message, maxwidth);
		}

		height = question.height();
		dimension = Point<int16_t>(top.width(), top.height() + height + bottom.height());
		position = Point<int16_t>(position.x() - dimension.x() / 2, position.y() - dimension.y() / 2);
	}

	void UINotice::draw(bool textfield) const
	{
		Point<int16_t> start = position;

		top.draw(start);
		start.shift_y(top.height());

		if (textfield)
		{
			center.draw(start);
			start.shift_y(center.height());
			centerbox.draw(start);
			start.shift_y(centerbox.height() - 1);
			box2.draw(start);
			start.shift_y(box2.height());
			box.draw(DrawArgument(start, Point<int16_t>(0, 29)));
			start.shift_y(29);

			question.draw(position + Point<int16_t>(13, 13));
		}
		else
		{
			center.draw(DrawArgument(start, Point<int16_t>(0, 32)));
			start.shift_y(32);
			centerbox.draw(start);
			start.shift_y(centerbox.height());
			box.draw(start);
			start.shift_y(box.height());

			question.draw(position + Point<int16_t>(131, 14));
		}

		bottombox.draw(start);
	}

	void UINotice::send_key(int32_t keycode, bool pressed)
	{
		if (pressed && (keycode == KeyAction::RETURN || keycode == KeyAction::ESCAPE))
		{
			if (type == NoticeType::OK || type == NoticeType::OKSMALL)
				UI::get().get_element<UIOk>()->send_key(keycode, pressed);
			else if (type == NoticeType::YESNO)
				UI::get().get_element<UIYesNo>()->send_key(keycode, pressed);
			else if (type == NoticeType::ENTERNUMBER)
				UI::get().get_element<UIEnterNumber>()->send_key(keycode, pressed);
		}
	}

	int16_t UINotice::box2offset() const
	{
		return top.height() + centerbox.height() + box.height() + 16;
	}

	UIYesNo::UIYesNo(std::string message, std::function<void(bool)> yh) : UINotice(message, NoticeType::YESNO)
	{
		yesnohandler = yh;

		int16_t belowtext = box2offset();

		nl::node src = nl::nx::ui["Basic.img"];

		buttons[YES] = std::make_unique<MapleButton>(src["BtOK4"], Point<int16_t>(156, belowtext));
		buttons[NO] = std::make_unique<MapleButton>(src["BtCancel4"], Point<int16_t>(198, belowtext));
	}

	void UIYesNo::draw(float alpha) const
	{
		UINotice::draw(false);
		UIElement::draw(alpha);
	}

	void UIYesNo::send_key(int32_t keycode, bool pressed)
	{
		if (keycode == KeyAction::RETURN)
		{
			yesnohandler(true);
			active = false;
		}
		else if (keycode == KeyAction::ESCAPE)
		{
			yesnohandler(false);
			active = false;
		}
	}

	Button::State UIYesNo::button_pressed(uint16_t buttonid)
	{
		switch (buttonid)
		{
		case YES:
			yesnohandler(true);
			break;
		case NO:
			yesnohandler(false);
			break;
		}

		active = false;

		return Button::State::PRESSED;
	}

	UIEnterNumber::UIEnterNumber(std::string message, std::function<void(int32_t)> nh, int32_t m, int32_t quantity) : UINotice(message, NoticeType::ENTERNUMBER)
	{
		numhandler = nh;
		max = m;

		int16_t belowtext = box2offset() - 21;
		int16_t pos_y = belowtext + 37;

		nl::node src = nl::nx::ui["Basic.img"];

		buttons[OK] = std::make_unique<MapleButton>(src["BtOK4"], 156, pos_y);
		buttons[CANCEL] = std::make_unique<MapleButton>(src["BtCancel4"], 198, pos_y);

		numfield = Textfield(Text::Font::A11M, Text::Alignment::LEFT, Text::Color::LIGHTGREY, Rectangle<int16_t>(24, 232, belowtext, belowtext + 20), 10);
		numfield.change_text(std::to_string(quantity));

		numfield.set_enter_callback(
			[&](std::string numstr)
			{
				handlestring(numstr);
			}
		);

		numfield.set_key_callback(
			KeyAction::Id::ESCAPE,
			[&]()
			{
				active = false;
			}
		);

		numfield.set_state(Textfield::State::FOCUSED);
	}

	void UIEnterNumber::draw(float alpha) const
	{
		UINotice::draw(true);
		UIElement::draw(alpha);

		numfield.draw(position);
	}

	void UIEnterNumber::update()
	{
		UIElement::update();

		numfield.update(position);
	}

	Cursor::State UIEnterNumber::send_cursor(bool clicked, Point<int16_t> cursorpos)
	{
		if (numfield.get_state() == Textfield::State::NORMAL)
		{
			Cursor::State nstate = numfield.send_cursor(cursorpos, clicked);

			if (nstate != Cursor::State::IDLE)
				return nstate;
		}

		return UIElement::send_cursor(clicked, cursorpos);
	}

	void UIEnterNumber::send_key(int32_t keycode, bool pressed)
	{
		if (keycode == KeyAction::RETURN)
		{
			handlestring(numfield.get_text());
			active = false;
		}
		else if (keycode == KeyAction::ESCAPE)
		{
			active = false;
		}
	}

	Button::State UIEnterNumber::button_pressed(uint16_t buttonid)
	{
		switch (buttonid)
		{
		case OK:
			handlestring(numfield.get_text());
			break;
		case CANCEL:
			active = false;
			break;
		}

		return Button::State::NORMAL;
	}

	void UIEnterNumber::handlestring(std::string numstr)
	{
		int num = -1;
		bool has_only_digits = (numstr.find_first_not_of("0123456789") == std::string::npos);

		auto okhandler = [&]()
		{
			numfield.set_state(Textfield::State::FOCUSED);
			buttons[OK]->set_state(Button::State::NORMAL);
		};

		if (!has_only_digits)
		{
			numfield.set_state(Textfield::State::DISABLED);
			UI::get().emplace<UIOk>("Only numbers are allowed.", okhandler, NoticeType::OKSMALL);
			return;
		}
		else
		{
			num = std::stoi(numstr);
		}

		if (num < 1)
		{
			numfield.set_state(Textfield::State::DISABLED);
			UI::get().emplace<UIOk>("You may only enter a number equal to or higher than 1.", okhandler, NoticeType::OKSMALL);
			return;
		}
		else if (num > max)
		{
			numfield.set_state(Textfield::State::DISABLED);
			UI::get().emplace<UIOk>("You may only enter a number equal to or lower than " + std::to_string(max) + ".", okhandler, NoticeType::OKSMALL);
			return;
		}
		else
		{
			numhandler(num);
			active = false;
		}

		buttons[OK]->set_state(Button::State::NORMAL);
	}

	UIOk::UIOk(std::string message, std::function<void()> oh, NoticeType type) : UINotice(message, type)
	{
		okhandler = oh;

		nl::node src = nl::nx::ui["Basic.img"];

		buttons[OK] = std::make_unique<MapleButton>(src["BtOK4"], 197, box2offset());
	}

	void UIOk::draw(float alpha) const
	{
		UINotice::draw(false);
		UIElement::draw(alpha);
	}

	void UIOk::send_key(int32_t keycode, bool pressed)
	{
		if (keycode == KeyAction::RETURN)
		{
			okhandler();
			active = false;
		}
		else if (keycode == KeyAction::ESCAPE)
		{
			active = false;
		}
	}

	Button::State UIOk::button_pressed(uint16_t buttonid)
	{
		switch (buttonid)
		{
		case OK:
			okhandler();
			break;
		}

		active = false;

		return Button::State::NORMAL;
	}
}