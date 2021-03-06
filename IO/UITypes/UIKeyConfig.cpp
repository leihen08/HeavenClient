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
#include "UIKeyConfig.h"

#include "../UI.h"

#include "../Components/MapleButton.h"
#include "../UITypes/UINotice.h"
#include "../UITypes/UILoginNotice.h"

#include <nlnx/nx.hpp>

namespace jrc
{
	UIKeyConfig::UIKeyConfig() : UIDragElement<PosKEYCONFIG>(Point<int16_t>())
	{
		keyboard = UI::get().get_keyboard();

		nl::node close = nl::nx::ui["Basic.img"]["BtClose"];

		nl::node KeyConfig = nl::nx::ui["StatusBar3.img"]["KeyConfig"];
		nl::node backgrnd = KeyConfig["backgrnd"];
		icon = KeyConfig["icon"];
		key = KeyConfig["key"];

		Texture bg = backgrnd;
		Point<int16_t> bg_dimensions = bg.get_dimensions();

		sprites.emplace_back(backgrnd);
		sprites.emplace_back(KeyConfig["backgrnd2"]);
		sprites.emplace_back(KeyConfig["backgrnd3"]);

		buttons[Buttons::CLOSE] = std::make_unique<MapleButton>(close, Point<int16_t>(bg_dimensions.x() - 12, 9));
		buttons[Buttons::CANCEL] = std::make_unique<MapleButton>(KeyConfig["button:Cancel"]);
		buttons[Buttons::DEFAULT] = std::make_unique<MapleButton>(KeyConfig["button:Default"]);
		buttons[Buttons::DELETE] = std::make_unique<MapleButton>(KeyConfig["button:Delete"]);
		buttons[Buttons::KEYSETTING] = std::make_unique<MapleButton>(KeyConfig["button:keySetting"]);
		buttons[Buttons::OK] = std::make_unique<MapleButton>(KeyConfig["button:OK"]);

		load_keys_pos();
		load_icons_pos();
		load_keys();
		load_icons();
		map_keys();

		dimension = bg_dimensions;
	}

	void UIKeyConfig::draw(float inter) const
	{
		UIElement::draw(inter);

		for (auto ficon : icons)
		{
			if (ficon.second)
			{
				if (std::find(found_actions.begin(), found_actions.end(), ficon.first) != found_actions.end())
				{
					int32_t maplekey = keyboard.get_maplekey(ficon.first);

					if (maplekey != -1)
					{
						KeyConfig::Key fkey = KeyConfig::actionbyid(maplekey);
						ficon.second->draw(position + keys_pos[fkey] - Point<int16_t>(2, 3));
					}
				}
				else
				{
					Point<int16_t> pos = icons_pos[ficon.first];

					for (auto uaction : updated_actions)
					{
						if (uaction.second == ficon.first)
						{
							pos = keys_pos[uaction.first] - Point<int16_t>(2, 3);
							break;
						}
					}

					ficon.second->draw(position + pos);
				}
			}
		}

		for (auto fkey : keys)
			fkey.second.draw(position + keys_pos[fkey.first]);
	}

	void UIKeyConfig::update()
	{
		UIElement::update();
	}

	void UIKeyConfig::send_key(int32_t keycode, bool pressed)
	{
		if (pressed && keycode == KeyAction::Id::ESCAPE)
			close();
	}

	Cursor::State UIKeyConfig::send_cursor(bool clicked, Point<int16_t> cursorpos)
	{
		Cursor::State dstate = UIDragElement::send_cursor(clicked, cursorpos);

		if (dragged)
			return dstate;

		KeyAction::Id icon_slot = icon_by_position(cursorpos);

		if (icon_slot != KeyAction::Id::LENGTH)
		{
			if (auto icon = icons[icon_slot].get())
			{
				if (clicked)
				{
					icon->start_drag(cursorpos - position - icons_pos[icon_slot]);
					UI::get().drag_icon(icon);

					return Cursor::State::GRABBING;
				}
				else
				{
					return Cursor::State::CANGRAB;
				}
			}
		}

		KeyConfig::Key key_slot = key_by_position(cursorpos);

		if (key_slot != KeyConfig::Key::LENGTH)
		{
			Keyboard::Mapping map = keyboard.get_maple_mapping(key_slot);
			KeyAction::Id action = KeyAction::actionbyid(map.action);

			if (auto icon = icons[action].get())
			{
				if (clicked)
				{
					icon->start_drag(cursorpos - position - keys_pos[key_slot]);
					UI::get().drag_icon(icon);

					return Cursor::State::GRABBING;
				}
				else
				{
					return Cursor::State::CANGRAB;
				}
			}
		}

		return Cursor::State::IDLE;
	}

	void UIKeyConfig::send_icon(const Icon& icon, Point<int16_t> cursorpos)
	{
		for (auto iter : icons_pos)
		{
			Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
				position + iter.second,
				position + iter.second + Point<int16_t>(32, 32)
				);

			if (icon_rect.contains(cursorpos))
				icon.drop_on_bindings(cursorpos, true);
		}

		KeyConfig::Key fkey = all_keys_by_position(cursorpos);

		if (fkey != KeyConfig::Key::LENGTH)
			icon.drop_on_bindings(cursorpos, false);
	}

	void UIKeyConfig::remove_key(KeyAction::Id action)
	{
		for (int i = 0; i < found_actions.size(); i++)
		{
			auto faction = found_actions.at(i);

			if (faction == action)
				found_actions.erase(found_actions.begin() + i);
		}
	}

	void UIKeyConfig::add_key(Point<int16_t> cursorposition, KeyAction::Id action)
	{
		KeyConfig::Key fkey = all_keys_by_position(cursorposition);
		int32_t okey = keyboard.get_maplekey(action);

		if (fkey == okey)
		{
			found_actions.emplace_back(action);
		}
		else
		{
			if (fkey != KeyConfig::Key::LENGTH)
				updated_actions.emplace_back(std::make_pair(fkey, action));
		}
	}

	Button::State UIKeyConfig::button_pressed(uint16_t buttonid)
	{
		switch (buttonid)
		{
		case Buttons::CLOSE:
		case Buttons::CANCEL:
			close();
			break;
		case Buttons::DEFAULT:
		{
			constexpr char* message = "Would you like to revert to default settings?";
			auto onok = [&]()
			{
				auto keysel_onok = [&](bool alternate)
				{
					std::cout << alternate << std::endl;

					reset();
				};

				UI::get().emplace<UIKeySelect>(keysel_onok, false);
			};

			UI::get().emplace<UIOk>(message, onok, UINotice::NoticeType::OK);
		}
		break;
		case Buttons::DELETE:
		{
			constexpr char* message = "Would you like to clear all key bindings?";
			auto onok = [&]()
			{
				clear();
			};

			UI::get().emplace<UIOk>(message, onok, UINotice::NoticeType::OK);
		}
		break;
		case Buttons::KEYSETTING:
			break;
		case Buttons::OK:
			break;
		default:
			break;
		}

		return Button::State::NORMAL;
	}

	void UIKeyConfig::close()
	{
		active = false;
		reset();
	}

	void UIKeyConfig::load_keys_pos()
	{
		int16_t slot_width = 33;
		int16_t slot_width_lg = 98;
		int16_t slot_height = 33;

		int16_t row_y = 126;
		int16_t row_special_y = row_y - slot_height - 5;

		int16_t row_quickslot_x = 535;

		int16_t row_one_x = 31;
		int16_t row_two_x = 80;
		int16_t row_three_x = 96;
		int16_t row_four_x = 55;
		int16_t row_five_x = 39;

		int16_t row_special_x = row_one_x;

		keys_pos[KeyConfig::Key::ESCAPE] = Point<int16_t>(row_one_x, row_special_y);

		row_special_x += slot_width * 2;

		for (size_t i = KeyConfig::Key::F1; i <= KeyConfig::Key::F12; i++)
		{
			KeyConfig::Key id = KeyConfig::actionbyid(i);

			keys_pos[id] = Point<int16_t>(row_special_x, row_special_y);

			row_special_x += slot_width;

			if (id == KeyConfig::Key::F4 || id == KeyConfig::Key::F8)
				row_special_x += 17;
		}

		keys_pos[KeyConfig::Key::SCROLL_LOCK] = Point<int16_t>(row_quickslot_x + (slot_width * 1), row_special_y);

		keys_pos[KeyConfig::Key::GRAVE_ACCENT] = Point<int16_t>(row_one_x + (slot_width * 0), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM1] = Point<int16_t>(row_one_x + (slot_width * 1), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM2] = Point<int16_t>(row_one_x + (slot_width * 2), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM3] = Point<int16_t>(row_one_x + (slot_width * 3), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM4] = Point<int16_t>(row_one_x + (slot_width * 4), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM5] = Point<int16_t>(row_one_x + (slot_width * 5), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM6] = Point<int16_t>(row_one_x + (slot_width * 6), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM7] = Point<int16_t>(row_one_x + (slot_width * 7), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM8] = Point<int16_t>(row_one_x + (slot_width * 8), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM9] = Point<int16_t>(row_one_x + (slot_width * 9), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::NUM0] = Point<int16_t>(row_one_x + (slot_width * 10), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::MINUS] = Point<int16_t>(row_one_x + (slot_width * 11), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::EQUAL] = Point<int16_t>(row_one_x + (slot_width * 12), row_y + (slot_height * 0));

		for (size_t i = KeyConfig::Key::Q; i <= KeyConfig::Key::RIGHT_BRACKET; i++)
		{
			KeyConfig::Key id = KeyConfig::actionbyid(i);

			keys_pos[id] = Point<int16_t>(row_two_x + (slot_width * (i - KeyConfig::Key::Q)), row_y + (slot_height * 1));
		}

		row_two_x += 9;

		keys_pos[KeyConfig::Key::BACKSLASH] = Point<int16_t>(row_two_x + (slot_width * 12), row_y + (slot_height * 1));

		for (size_t i = KeyConfig::Key::A; i <= KeyConfig::Key::APOSTROPHE; i++)
		{
			KeyConfig::Key id = KeyConfig::actionbyid(i);

			keys_pos[id] = Point<int16_t>(row_three_x + (slot_width * (i - KeyConfig::Key::A)), row_y + (slot_height * 2));
		}

		keys_pos[KeyConfig::Key::LEFT_SHIFT] = Point<int16_t>(row_four_x + (slot_width * 0), row_y + (slot_height * 3));

		row_four_x += 24;

		for (size_t i = KeyConfig::Key::Z; i <= KeyConfig::Key::PERIOD; i++)
		{
			KeyConfig::Key id = KeyConfig::actionbyid(i);

			keys_pos[id] = Point<int16_t>(row_four_x + (slot_width * (i - KeyConfig::Key::Z + 1)), row_y + (slot_height * 3));
		}

		row_four_x += 24;

		keys_pos[KeyConfig::Key::RIGHT_SHIFT] = Point<int16_t>(row_four_x + (slot_width * 11), row_y + (slot_height * 3));

		keys_pos[KeyConfig::Key::LEFT_CONTROL] = Point<int16_t>(row_five_x + (slot_width_lg * 0), row_y + (slot_height * 4));
		keys_pos[KeyConfig::Key::LEFT_ALT] = Point<int16_t>(row_five_x + (slot_width_lg * 1), row_y + (slot_height * 4));

		row_five_x += 24;

		keys_pos[KeyConfig::Key::SPACE] = Point<int16_t>(row_five_x + (slot_width_lg * 2), row_y + (slot_height * 4));

		row_five_x += 27;

		keys_pos[KeyConfig::Key::RIGHT_ALT] = Point<int16_t>(row_five_x + (slot_width_lg * 3), row_y + (slot_height * 4));

		row_five_x += 2;

		keys_pos[KeyConfig::Key::RIGHT_CONTROL] = Point<int16_t>(row_five_x + (slot_width_lg * 4), row_y + (slot_height * 4));

		keys_pos[KeyConfig::Key::INSERT] = Point<int16_t>(row_quickslot_x + (slot_width * 0), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::HOME] = Point<int16_t>(row_quickslot_x + (slot_width * 1), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::PAGE_UP] = Point<int16_t>(row_quickslot_x + (slot_width * 2), row_y + (slot_height * 0));
		keys_pos[KeyConfig::Key::DELETE] = Point<int16_t>(row_quickslot_x + (slot_width * 0), row_y + (slot_height * 1));
		keys_pos[KeyConfig::Key::END] = Point<int16_t>(row_quickslot_x + (slot_width * 1), row_y + (slot_height * 1));
		keys_pos[KeyConfig::Key::PAGE_DOWN] = Point<int16_t>(row_quickslot_x + (slot_width * 2), row_y + (slot_height * 1));
	}

	void UIKeyConfig::load_icons_pos()
	{
		int16_t row_x = 26;
		int16_t row_y = 307;

		int16_t slot_width = 36;
		int16_t slot_height = 36;

		icons_pos[KeyAction::Id::MAPLECHAT] = Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::TOGGLECHAT] = Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::WHISPER] = Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::MEDALS] = Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::BOSSPARTY] = Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::PROFESSION] = Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::EQUIPMENT] = Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::ITEMS] = Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::CHARINFO] = Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::MENU] = Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::QUICKSLOTS] = Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::PICKUP] = Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::SIT] = Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::ATTACK] = Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::JUMP] = Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::INTERACT_HARVEST] = Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 0));
		icons_pos[KeyAction::Id::SOULWEAPON] = Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 0));

		icons_pos[KeyAction::Id::SAY] = Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::PARTYCHAT] = Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::FRIENDSCHAT] = Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::ITEMPOT] = Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::EVENT] = Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::SILENTCRUSADE] = Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::STATS] = Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::SKILLS] = Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::QUESTLOG] = Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::CHANGECHANNEL] = Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::GUILD] = Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::PARTY] = Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::NOTIFIER] = Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::FRIENDS] = Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::WORLDMAP] = Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::MINIMAP] = Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 1));
		icons_pos[KeyAction::Id::KEYBINDINGS] = Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 1));

		icons_pos[KeyAction::Id::GUILDCHAT] = Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::ALLIANCECHAT] = Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::BATTLEANALYSIS] = Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::GUIDE] = Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::ENHANCEEQUIP] = Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::MONSTERCOLLECTION] = Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::MANAGELEGION] = Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 2));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::MAPLENEWS] = Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::CASHSHOP] = Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::MAINMENU] = Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::SCREENSHOT] = Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::PICTUREMODE] = Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 2));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 2));
		icons_pos[KeyAction::Id::MUTE] = Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 2));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 2));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 2));

		icons_pos[KeyAction::Id::FACE1] = Point<int16_t>(row_x + (slot_width * 0), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE2] = Point<int16_t>(row_x + (slot_width * 1), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE3] = Point<int16_t>(row_x + (slot_width * 2), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE4] = Point<int16_t>(row_x + (slot_width * 3), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE5] = Point<int16_t>(row_x + (slot_width * 4), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE6] = Point<int16_t>(row_x + (slot_width * 5), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::FACE7] = Point<int16_t>(row_x + (slot_width * 6), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::MAPLEACHIEVEMENT] = Point<int16_t>(row_x + (slot_width * 7), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::MONSTERBOOK] = Point<int16_t>(row_x + (slot_width * 8), row_y + (slot_height * 3));
		icons_pos[KeyAction::Id::TOSPOUSE] = Point<int16_t>(row_x + (slot_width * 9), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 10), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 11), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 12), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 13), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 14), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 15), row_y + (slot_height * 3));
		//icons_pos[KeyAction::Id::LENGTH] = Point<int16_t>(row_x + (slot_width * 16), row_y + (slot_height * 3));
	}

	void UIKeyConfig::load_keys()
	{
		keys[KeyConfig::Key::ESCAPE] = key[1];
		keys[KeyConfig::Key::NUM1] = key[2];
		keys[KeyConfig::Key::NUM2] = key[3];
		keys[KeyConfig::Key::NUM3] = key[4];
		keys[KeyConfig::Key::NUM4] = key[5];
		keys[KeyConfig::Key::NUM5] = key[6];
		keys[KeyConfig::Key::NUM6] = key[7];
		keys[KeyConfig::Key::NUM7] = key[8];
		keys[KeyConfig::Key::NUM8] = key[9];
		keys[KeyConfig::Key::NUM9] = key[10];
		keys[KeyConfig::Key::NUM0] = key[11];
		keys[KeyConfig::Key::MINUS] = key[12];
		keys[KeyConfig::Key::EQUAL] = key[13];

		keys[KeyConfig::Key::Q] = key[16];
		keys[KeyConfig::Key::W] = key[17];
		keys[KeyConfig::Key::E] = key[18];
		keys[KeyConfig::Key::R] = key[19];
		keys[KeyConfig::Key::T] = key[20];
		keys[KeyConfig::Key::Y] = key[21];
		keys[KeyConfig::Key::U] = key[22];
		keys[KeyConfig::Key::I] = key[23];
		keys[KeyConfig::Key::O] = key[24];
		keys[KeyConfig::Key::P] = key[25];
		keys[KeyConfig::Key::LEFT_BRACKET] = key[26];
		keys[KeyConfig::Key::RIGHT_BRACKET] = key[27];

		keys[KeyConfig::Key::LEFT_CONTROL] = key[29];
		keys[KeyConfig::Key::RIGHT_CONTROL] = key[29];

		keys[KeyConfig::Key::A] = key[30];
		keys[KeyConfig::Key::S] = key[31];
		keys[KeyConfig::Key::D] = key[32];
		keys[KeyConfig::Key::F] = key[33];
		keys[KeyConfig::Key::G] = key[34];
		keys[KeyConfig::Key::H] = key[35];
		keys[KeyConfig::Key::J] = key[36];
		keys[KeyConfig::Key::K] = key[37];
		keys[KeyConfig::Key::L] = key[38];
		keys[KeyConfig::Key::SEMICOLON] = key[39];
		keys[KeyConfig::Key::APOSTROPHE] = key[40];
		keys[KeyConfig::Key::GRAVE_ACCENT] = key[41];

		keys[KeyConfig::Key::LEFT_SHIFT] = key[42];
		keys[KeyConfig::Key::RIGHT_SHIFT] = key[42];

		keys[KeyConfig::Key::BACKSLASH] = key[43];
		keys[KeyConfig::Key::Z] = key[44];
		keys[KeyConfig::Key::X] = key[45];
		keys[KeyConfig::Key::C] = key[46];
		keys[KeyConfig::Key::V] = key[47];
		keys[KeyConfig::Key::B] = key[48];
		keys[KeyConfig::Key::N] = key[49];
		keys[KeyConfig::Key::M] = key[50];
		keys[KeyConfig::Key::COMMA] = key[51];
		keys[KeyConfig::Key::PERIOD] = key[52];

		keys[KeyConfig::Key::LEFT_ALT] = key[56];
		keys[KeyConfig::Key::RIGHT_ALT] = key[56];

		keys[KeyConfig::Key::SPACE] = key[57];

		keys[KeyConfig::Key::F1] = key[59];
		keys[KeyConfig::Key::F2] = key[60];
		keys[KeyConfig::Key::F3] = key[61];
		keys[KeyConfig::Key::F4] = key[62];
		keys[KeyConfig::Key::F5] = key[63];
		keys[KeyConfig::Key::F6] = key[64];
		keys[KeyConfig::Key::F7] = key[65];
		keys[KeyConfig::Key::F8] = key[66];
		keys[KeyConfig::Key::F9] = key[67];
		keys[KeyConfig::Key::F10] = key[68];

		keys[KeyConfig::Key::SCROLL_LOCK] = key[70];
		keys[KeyConfig::Key::HOME] = key[71];

		keys[KeyConfig::Key::PAGE_UP] = key[73];

		keys[KeyConfig::Key::END] = key[79];

		keys[KeyConfig::Key::PAGE_DOWN] = key[81];
		keys[KeyConfig::Key::INSERT] = key[82];
		keys[KeyConfig::Key::DELETE] = key[83];

		keys[KeyConfig::Key::F11] = key[87];
		keys[KeyConfig::Key::F12] = key[88];
	}

	void UIKeyConfig::load_icons()
	{
		icons[KeyAction::Id::EQUIPMENT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::EQUIPMENT), icon[0], -1);
		icons[KeyAction::Id::ITEMS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::ITEMS), icon[1], -1);
		icons[KeyAction::Id::STATS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::STATS), icon[2], -1);
		icons[KeyAction::Id::SKILLS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SKILLS), icon[3], -1);
		icons[KeyAction::Id::FRIENDS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FRIENDS), icon[4], -1);
		icons[KeyAction::Id::WORLDMAP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::WORLDMAP), icon[5], -1);
		icons[KeyAction::Id::MAPLECHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MAPLECHAT), icon[6], -1);
		icons[KeyAction::Id::MINIMAP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MINIMAP), icon[7], -1);
		icons[KeyAction::Id::QUESTLOG] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::QUESTLOG), icon[8], -1);
		icons[KeyAction::Id::KEYBINDINGS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::KEYBINDINGS), icon[9], -1);
		icons[KeyAction::Id::SAY] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SAY), icon[10], -1);
		icons[KeyAction::Id::WHISPER] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::WHISPER), icon[11], -1);
		icons[KeyAction::Id::PARTYCHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::PARTYCHAT), icon[12], -1);
		icons[KeyAction::Id::FRIENDSCHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FRIENDSCHAT), icon[13], -1);
		icons[KeyAction::Id::MENU] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MENU), icon[14], -1);
		icons[KeyAction::Id::QUICKSLOTS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::QUICKSLOTS), icon[15], -1);
		icons[KeyAction::Id::TOGGLECHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::TOGGLECHAT), icon[16], -1);
		icons[KeyAction::Id::GUILD] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::GUILD), icon[17], -1);
		icons[KeyAction::Id::GUILDCHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::GUILDCHAT), icon[18], -1);
		icons[KeyAction::Id::PARTY] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::PARTY), icon[19], -1);
		icons[KeyAction::Id::NOTIFIER] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::NOTIFIER), icon[20], -1);
		icons[KeyAction::Id::MAPLENEWS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MAPLENEWS), icon[21], -1);
		icons[KeyAction::Id::CASHSHOP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::CASHSHOP), icon[22], -1);
		icons[KeyAction::Id::ALLIANCECHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::ALLIANCECHAT), icon[23], -1);
		icons[KeyAction::Id::MANAGELEGION] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MANAGELEGION), icon[25], -1);
		icons[KeyAction::Id::MEDALS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MEDALS), icon[26], -1);
		icons[KeyAction::Id::BOSSPARTY] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::BOSSPARTY), icon[27], -1);
		icons[KeyAction::Id::PROFESSION] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::PROFESSION), icon[29], -1);
		icons[KeyAction::Id::ITEMPOT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::ITEMPOT), icon[30], -1);
		icons[KeyAction::Id::EVENT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::EVENT), icon[31], -1);
		icons[KeyAction::Id::SILENTCRUSADE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SILENTCRUSADE), icon[33], -1);
		//icons[KeyAction::Id::BITS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::BITS), icon[34], -1);
		icons[KeyAction::Id::BATTLEANALYSIS] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::BATTLEANALYSIS), icon[35], -1);
		icons[KeyAction::Id::GUIDE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::GUIDE), icon[39], -1);
		//icons[KeyAction::Id::VIEWERSCHAT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::VIEWERSCHAT), icon[40], -1);
		icons[KeyAction::Id::ENHANCEEQUIP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::ENHANCEEQUIP), icon[41], -1);
		icons[KeyAction::Id::MONSTERCOLLECTION] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MONSTERCOLLECTION), icon[42], -1);
		icons[KeyAction::Id::SOULWEAPON] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SOULWEAPON), icon[43], -1);
		icons[KeyAction::Id::CHARINFO] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::CHARINFO), icon[44], -1);
		icons[KeyAction::Id::CHANGECHANNEL] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::CHANGECHANNEL), icon[45], -1);
		icons[KeyAction::Id::MAINMENU] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MAINMENU), icon[46], -1);
		icons[KeyAction::Id::SCREENSHOT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SCREENSHOT), icon[47], -1);
		icons[KeyAction::Id::PICTUREMODE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::PICTUREMODE), icon[48], -1);
		icons[KeyAction::Id::MAPLEACHIEVEMENT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MAPLEACHIEVEMENT), icon[49], -1);
		icons[KeyAction::Id::PICKUP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::PICKUP), icon[50], -1);
		icons[KeyAction::Id::SIT] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SIT), icon[51], -1);
		icons[KeyAction::Id::ATTACK] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::ATTACK), icon[52], -1);
		icons[KeyAction::Id::JUMP] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::JUMP), icon[53], -1);
		icons[KeyAction::Id::INTERACT_HARVEST] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::INTERACT_HARVEST), icon[54], -1);
		icons[KeyAction::Id::FACE1] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE1), icon[100], -1);
		icons[KeyAction::Id::FACE2] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE2), icon[101], -1);
		icons[KeyAction::Id::FACE3] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE3), icon[102], -1);
		icons[KeyAction::Id::FACE4] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE4), icon[103], -1);
		icons[KeyAction::Id::FACE5] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE5), icon[104], -1);
		icons[KeyAction::Id::FACE6] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE6), icon[105], -1);
		icons[KeyAction::Id::FACE7] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::FACE7), icon[106], -1);
		//icons[KeyAction::Id::MAPLESTORAGE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MAPLESTORAGE), icon[200], -1);
		//icons[KeyAction::Id::SAFEMODE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::SAFEMODE), icon[201], -1);
		icons[KeyAction::Id::MUTE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MUTE), icon[202], -1);
		icons[KeyAction::Id::MONSTERBOOK] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::MONSTERBOOK), icon[1000], -1);
		icons[KeyAction::Id::TOSPOUSE] = std::make_unique<Icon>(std::make_unique<KeyIcon>(KeyAction::Id::TOSPOUSE), icon[1001], -1);
	}

	void UIKeyConfig::map_keys()
	{
		for (auto fkey : keys)
		{
			Keyboard::Mapping map = keyboard.get_maple_mapping(fkey.first);

			if (map.type != KeyType::Id::NONE)
			{
				KeyAction::Id action = KeyAction::actionbyid(map.action);

				if (action || action == KeyAction::Id::EQUIPMENT)
					found_actions.emplace_back(action);
			}
		}
	}

	void UIKeyConfig::clear()
	{
		found_actions.clear();
		updated_actions.clear();
	}

	void UIKeyConfig::reset()
	{
		clear();
		map_keys();
	}

	KeyAction::Id UIKeyConfig::icon_by_position(Point<int16_t> cursorpos) const
	{
		for (auto iter : icons_pos)
		{
			if (std::find(found_actions.begin(), found_actions.end(), iter.first) != found_actions.end())
				continue;

			Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
				position + iter.second,
				position + iter.second + Point<int16_t>(32, 32)
				);

			if (icon_rect.contains(cursorpos))
				return iter.first;
		}

		return KeyAction::Id::LENGTH;
	}

	KeyConfig::Key UIKeyConfig::key_by_position(Point<int16_t> cursorpos) const
	{
		for (auto iter : keys_pos)
		{
			Keyboard::Mapping map = keyboard.get_maple_mapping(iter.first);
			KeyAction::Id k = KeyAction::actionbyid(map.action);

			if (map.action != KeyType::Id::NONE)
			{
				if (std::find(found_actions.begin(), found_actions.end(), map.action) != found_actions.end())
				{
					Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
						position + iter.second,
						position + iter.second + Point<int16_t>(32, 32)
						);

					if (icon_rect.contains(cursorpos))
						return iter.first;
				}
			}
		}

		return KeyConfig::Key::LENGTH;
	}

	KeyConfig::Key UIKeyConfig::all_keys_by_position(Point<int16_t> cursorpos) const
	{
		for (auto iter : keys_pos)
		{
			Rectangle<int16_t> icon_rect = Rectangle<int16_t>(
				position + iter.second,
				position + iter.second + Point<int16_t>(32, 32)
				);

			if (icon_rect.contains(cursorpos))
				return iter.first;
		}

		return KeyConfig::Key::LENGTH;
	}

	UIKeyConfig::KeyIcon::KeyIcon(KeyAction::Id keyId)
	{
		source = keyId;
	}

	void UIKeyConfig::KeyIcon::drop_on_bindings(Point<int16_t> cursorposition, bool remove) const
	{
		auto keyconfig = UI::get().get_element<UIKeyConfig>();

		if (remove)
			keyconfig->remove_key(source);
		else
			keyconfig->add_key(cursorposition, source);
	}
}