#pragma once
#include "ImGui/imgui_internal.h"

namespace ImGui
{
	/* Color_t to ImVec4 */
	inline ImVec4 ColorToVec(Color_t color) {
		return {Color::TOFLOAT(color.r), Color::TOFLOAT(color.g), Color::TOFLOAT(color.b), Color::TOFLOAT(color.a)};
	}


	/* ImVec4 to Color_t */
	inline Color_t VecToColor(ImVec4 color) {
		return {
			static_cast<byte>(color.x * 256.0f > 255 ? 255 : color.x * 256.0f),
			static_cast<byte>(color.y * 256.0f > 255 ? 255 : color.y * 256.0f),
			static_cast<byte>(color.z * 256.0f > 255 ? 255 : color.z * 256.0f),
			static_cast<byte>(color.w * 256.0f > 255 ? 255 : color.w * 256.0f)
		};
	}

	__inline void HelpMarker(const char* desc)
	{
		if (IsItemHovered())
		{
			if (Vars::Menu::ModernDesign)
			{
				SetTooltip(desc);
			} else
			{
				F::Menu.FeatureHint = desc;
			}
		}
	}

	__inline bool IconButton(const char* icon)
	{
		PushFont(F::Menu.IconFont);
		TextUnformatted(icon);
		const bool pressed = IsItemClicked();
		PopFont();
		return pressed;
	}

	__inline void SectionTitle(const char* title, float yOffset = 0)
	{
		Dummy({ 0, yOffset });
		PushFont(F::Menu.SectionFont);
		const ImVec2 titleSize = CalcTextSize(title);
		SetCursorPosX((GetWindowSize().x - titleSize.x) * .5f);
		Text(title);
		PopFont();

		const auto widgetPos = GetCursorScreenPos();
		GradientRect(&F::Menu.MainGradient, { widgetPos.x, widgetPos.y - 2 }, GetColumnWidth(), 1.5);
	}

	__inline bool TableColumnChild(const char* str_id)
	{
		TableNextColumn();
		float contentHeight = GetWindowHeight() - (F::Menu.TabHeight + GetStyle().ItemInnerSpacing.y);
		return BeginChild(str_id, { GetColumnWidth(), contentHeight }, !Vars::Menu::ModernDesign);
	}

	__inline bool SidebarButton(const char* label, bool active = false)
	{
		if (active) { PushStyleColor(ImGuiCol_Button, ImColor(38, 38, 38).Value); }
		const bool pressed = Button(label, { GetWindowSize().x - 2 * GetStyle().WindowPadding.x, 22.f });
		if (active) { PopStyleColor(); }
		return pressed;
	}

	__inline bool TabButton(const char* label, bool active = false)
	{
		TableNextColumn();
		if (active) { PushStyleColor(ImGuiCol_Button, GetColorU32(ImGuiCol_ButtonActive)); }
		const bool pressed = Button(label, { GetColumnWidth(), F::Menu.TabHeight });
		if (active)
		{
			PopStyleColor();
		}
		return pressed;
	}

    __inline bool InputKeybind(const char* label, CVar<int>& output, bool bAllowNone = true, bool bAllowSpecial = false)
	{
		auto VK2STR = [&](const short key) -> const char* {
			switch (key) {
			case VK_LBUTTON: return "LMB";
			case VK_RBUTTON: return "RMB";
			case VK_MBUTTON: return "MMB";
			case VK_XBUTTON1: return "Mouse4";
			case VK_XBUTTON2: return "Mouse5";
			case VK_SPACE: return "Space";
			case 0x0: return "None";
			case VK_A: return "A";
			case VK_B: return "B";
			case VK_C: return "C";
			case VK_D: return "D";
			case VK_E: return "E";
			case VK_F: return "F";
			case VK_G: return "G";
			case VK_H: return "H";
			case VK_I: return "I";
			case VK_J: return "J";
			case VK_K: return "K";
			case VK_L: return "L";
			case VK_M: return "M";
			case VK_N: return "N";
			case VK_O: return "O";
			case VK_P: return "P";
			case VK_Q: return "Q";
			case VK_R: return "R";
			case VK_S: return "S";
			case VK_T: return "T";
			case VK_U: return "U";
			case VK_V: return "V";
			case VK_W: return "W";
			case VK_X: return "X";
			case VK_Y: return "Y";
			case VK_Z: return "Z";
			case VK_0: return "0";
			case VK_1: return "1";
			case VK_2: return "2";
			case VK_3: return "3";
			case VK_4: return "4";
			case VK_5: return "5";
			case VK_6: return "6";
			case VK_7: return "7";
			case VK_8: return "8";
			case VK_9: return "9";
			case VK_ESCAPE: return "Escape";
			case VK_SHIFT: return "Shift";
			case VK_LSHIFT: return "LShift";
			case VK_RSHIFT: return "RShift";
			case VK_CONTROL: return "Control";
			case VK_MENU: return "LAlt";
			case VK_PRIOR: return "Page Up";
			case VK_NEXT: return "Page Down";
			case VK_INSERT: return "Insert";
			case VK_DELETE: return "Delete";
			case VK_HOME: return "Home";
			case VK_END: return "End";
			case VK_F9: return "F9";
			case VK_F10: return "F10";
			case VK_F11: return "F11";
			case VK_F12: return "F12";
			default: break;
			}

			WCHAR output[16] = { L"\0" };
			if (const int result = GetKeyNameTextW(MapVirtualKeyW(key, MAPVK_VK_TO_VSC) << 16, output, 16)) {
				char outputt[128];
				sprintf(outputt, "%ws", output);
				return outputt;
			}

			return "Unknown";
		};

		const auto id = GetID(label);
		PushID(label);

		if (GetActiveID() == id) {
			Button("...", ImVec2(85, 20));

			static float time = I::EngineClient->Time();
			const float elapsed = I::EngineClient->Time() - time;
			static CVar<int>* curr = nullptr, * prevv = curr;
			if (curr != prevv) {
				time = I::EngineClient->Time();
				prevv = curr;
			}

			if (curr == nullptr && elapsed > 0.1f) {
				for (short n = 0; n < 256; n++) {
					if ((n > 0x0 && n < 0x7) ||
						(n > L'A' - 1 && n < L'Z' + 1) ||
						(n > L'0' - 1 && n < L'9' + 1) ||
						n == VK_LSHIFT ||
						n == VK_RSHIFT ||
						n == VK_SHIFT ||
						n == VK_ESCAPE ||
						n == VK_HOME ||
						n == VK_CONTROL ||
						n == VK_MENU ||
						n == VK_PRIOR ||
						n == VK_NEXT ||
						n == VK_DELETE ||
						n == VK_HOME ||
						n == VK_END ||
						bAllowSpecial && (
							n == VK_INSERT ||
							n == VK_F9 ||
							n == VK_F10 ||
							n== VK_F12
							)) {
						if ((!IsItemHovered() && GetIO().MouseClicked[0])) {
							ClearActiveID();
							break;
						}
						if (GetAsyncKeyState(n) & 0x8000)
						{
							if (n == VK_HOME || n == VK_INSERT) {
								break;
							}

							if (n == VK_ESCAPE && bAllowNone) {
								ClearActiveID();
								output.Value = 0x0;
								break;
							}

							output.Value = n;
							ClearActiveID();
							break;
						}
					} //loop
				}
			}

			if (curr != prevv) {
				time = I::EngineClient->Time();
				prevv = curr;
			}

			GetCurrentContext()->ActiveIdAllowOverlap = true;
			if ((!IsItemHovered() && GetIO().MouseClicked[0]))
			{
				ClearActiveID();
			}
		}
		else if (Button(VK2STR(output.Value), ImVec2(85, 20))) {
			SetActiveID(id, GetCurrentWindow());
		}

		SameLine();
		Text("%s", label);
		PopID();

		return true;
	}

	/* Combobox with multiple selectable items */
	__inline void MultiCombo(std::vector<const char*> titles, std::vector<bool*> options,
	                         const std::string& comboName) {
		if (titles.size() != options.size()) { return; }
		
		std::string preview = "<None>##";
		for (size_t i = 0; i < options.size(); i++) {
			if (*options[i]) {
				if (preview == "<None>##") { preview = ""; }
				preview += titles[i];
				preview.append(", ");
			}
		}
		preview.pop_back(); preview.pop_back(); // This is a stupid but easy way to remove the last comma

		PushItemWidth(F::Menu.ItemWidth);
		if (BeginCombo(comboName.c_str(), preview.c_str())) {
			for (size_t i = 0; i < titles.size(); i++) {
				Selectable((*options[i]) ? tfm::format("+ %s", titles[i]).c_str() : titles[i], options[i], ImGuiSelectableFlags_DontClosePopups);
			}

			EndCombo();
		}
		PopItemWidth();
	}
	
	__inline void MultiFlags(std::vector<const char*> flagNames, std::vector<int> flagValues, int* flagVar, const std::string& comboName)
	{
		if (flagNames.size() != flagValues.size()) { return; }

		std::string preview = "<Default>##";
		if (*flagVar == 0)
		{
			preview = "<None>##";
		} else
		{
			for (size_t i = 0; i < flagValues.size(); i++) {
				if (*flagVar & flagValues[i]) {
					if (preview == "<Default>##") { preview = ""; }
					preview += flagNames[i];
					preview.append(", ");
				}
			}
			preview.pop_back(); preview.pop_back();
		}

		PushItemWidth(F::Menu.ItemWidth);
		if (BeginCombo(comboName.c_str(), preview.c_str())) {
			for (size_t i = 0; i < flagNames.size(); i++) {
				const bool flagActive = *flagVar & flagValues[i];
				if (Selectable(flagActive ? tfm::format("+ %s", flagNames[i]).c_str() : flagNames[i], flagActive, ImGuiSelectableFlags_DontClosePopups))
				{
					if (flagActive)
					{
						*flagVar &= ~flagValues[i];
					} else
					{
						*flagVar |= flagValues[i];
					}
				}
			}

			EndCombo();
		}
		PopItemWidth();
	}

	__inline bool ColorPicker(const char* label, Color_t& color)
	{
		bool open = false;
		ImVec4 tempColor = ColorToVec(color);
		PushItemWidth(F::Menu.ItemWidth);
		if (ColorEdit4(label, &tempColor.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
		{
			color = VecToColor(tempColor);
			open = true;
		}
		PopItemWidth();
		HelpMarker(label);
		return open;
	}

	/* Inline color picker */
	__inline bool ColorPickerL(const char* label, Color_t& color, int num = 0)
	{
		SameLine(GetContentRegionMax().x - 20 - (num * 24));
		SetNextItemWidth(20);
		return ColorPicker(label, color);
	}

	__inline void TextCentered(const char* fmt)
	{
		const auto windowWidth = GetWindowSize().x;
		const auto textWidth = CalcTextSize(fmt).x;

		SetCursorPosX((windowWidth - textWidth) * 0.5f);
		Text("%s", fmt);
	}

    // Source: https://github.com/ocornut/imgui/issues/1537#issuecomment-355569554
	__inline bool ToggleButton(const char* label, bool* v)
    {
	    const auto p = GetCursorScreenPos();
        auto* drawList = GetWindowDrawList();
		auto& style = GetStyle();

        const float height = GetFrameHeight();
        const float width = height * 1.8f;
        const float radius = height * 0.50f;
        const ImVec2 labelSize = CalcTextSize(label, nullptr, true);

        InvisibleButton(label, ImVec2(width + style.ItemInnerSpacing.x + labelSize.x, height));
        if (IsItemClicked()) { *v = !*v; }

        float t = *v ? 1.0f : 0.0f;

        ImGuiContext& g = *GImGui;
        constexpr float ANIM_SPEED = 0.25f;
        if (g.LastActiveId == g.CurrentWindow->GetID(label))
        {
	        const float tAnim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
            t = *v ? (tAnim) : (1.0f - tAnim);
        }

        const ImU32 colBg = IsItemHovered() ? ImColor(60, 60, 60) : ImColor(50, 50, 50);
        const ImU32 colCircle = (*v) ? F::Menu.Accent : ImColor(180, 180, 180);

        drawList->AddRectFilled(p, ImVec2(p.x + width, p.y + height), colBg, height * 0.5f);
        drawList->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, colCircle);
		RenderText({ p.x + width + style.ItemInnerSpacing.x, p.y + (height / 2 - labelSize.y / 2) }, label);
		return *v;
    }

#pragma region Width Components
	__inline bool WCombo(const char* label, int* current_item, std::vector<const char*> items) {
		SetNextItemWidth(F::Menu.ItemWidth);
		return Combo(label, current_item, items.data(), items.size(), -1);
	}

	__inline bool WSlider(const char* label, float* v, float v_min, float v_max, const char* format = "%.2f", ImGuiSliderFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return SliderFloat(label, v, v_min, v_max, format, flags);
	}

	__inline bool WSlider(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return SliderInt(label, v, v_min, v_max, format, flags);
	}

	__inline bool WInputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputText(label, str, flags, callback, user_data);
	}

	__inline bool WInputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputTextWithHint(label, hint, str, flags, callback, user_data);
	}

	__inline bool WInputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0)
	{
		SetNextItemWidth(F::Menu.ItemWidth);
		return InputInt(label, v, step, step_fast, flags);
	}

	__inline bool WToggle(const char* label, bool* v)
	{
		bool result;
		if (Vars::Menu::ModernDesign)
		{
			result = ToggleButton(label, v);
		} else
		{
			result = ToggleButton(label, v);
		}
		return result;
	}
#pragma endregion
}
