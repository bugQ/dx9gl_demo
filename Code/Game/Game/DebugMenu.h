#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "Controller.h"

namespace eae6320
{
	class DebugMenu : public Controller
	{
#ifdef _DEBUG
		struct Widget
		{
		public:
			const std::string id;
			int draw(int x, int y) const;

			virtual void activate_left() = 0;
			virtual void activate_right() = 0;
			virtual const char* text_format() const = 0;

			Widget(char* id) : id(id) {}
			virtual ~Widget() {}
		};
		class Slider : public Widget
		{
			static const int resolution = 64;
			const float min, max;
			float &param;

			float normalized() const { return (param - min) / (max - min); }
			float denormalize(float x) const { return x * (max - min) + min; }
			float denormalize(int pos) const { return denormalize((float)pos / resolution); }
			int position() const { return (int) floor(normalized() * resolution); }

		public:
			Slider(char* id, float min, float max, float &param)
				: Widget(id), min(min), max(max), param(param) {}
			virtual ~Slider() {}

			virtual void activate_left()
			{
				param = denormalize(std::max(0, position() - 1));
			}
			virtual void activate_right()
			{
				param = denormalize(std::min(resolution, position() + 1));
			}

			virtual const char* text_format() const;
		};
		class CheckBox : public Widget
		{
			bool &param;

		public:
			CheckBox(char* id, bool &param) : Widget(id), param(param) {}
			virtual ~CheckBox() {}

			virtual void activate_left() { param = false; }
			virtual void activate_right() { param = true; }

			virtual const char* text_format() const
				{ return param ? "(off   -> on)" : "(off <-   on)"; }
		};
		class Text : public Widget
		{
			std::string &param;

		public:
			Text(char* id, std::string & param) : Widget(id), param(param) {}
			virtual ~Text() {}

			virtual void activate_left() {}
			virtual void activate_right() {}

			virtual const char* text_format() const { return param.c_str(); }
		};
		class Button : public Widget
		{
			void (&callback)(void);

		public:
			Button(char* id, void (&callback)(void)) : Widget(id), callback(callback) {}
			virtual ~Button() {}

			virtual void activate_left() {}
			virtual void activate_right() { callback(); }

			virtual const char* text_format() const { return "(press right)"; }
		};

		std::vector<Widget *> widgets;
		size_t cursor = 0;

#endif // _DEBUG
	public:
		enum Control { none, up, down, left, right };

		Control joy2dir(Vector2 joystick)
		{
			if (abs(joystick.y) > abs(joystick.x * 2))
			{
				if (joystick.y > 0.5f) return up;
				if (joystick.y < -0.5f) return down;
			}
			else if (abs(joystick.x) > abs(joystick.y * 2))
			{
				if (joystick.x > 0.5f) return right;
				if (joystick.x < -0.5f) return left;
			}
			return none;
		}

		DebugMenu() {}
		~DebugMenu() {}

#ifdef _DEBUG
		void add_slider(char* id, float min, float max, float& param)
			{ widgets.push_back(new Slider(id, min, max, param)); }
		void add_checkbox(char* id, bool &param)
			{ widgets.push_back(new CheckBox(id, param)); }
		void add_text(char* id, std::string &param)
			{ widgets.push_back(new Text(id, param)); }
		void add_button(char* id, void (&callback)(void))
			{ widgets.push_back(new Button(id, callback)); }

		void ControlCursor(Control c)
		{
			switch (c)
			{
			case up: cursor = cursor > 0 ? cursor - 1 : 0; break;
			case down: cursor = cursor < widgets.size() - 1 ? cursor + 1 : cursor; break;
			case left: if (widgets.size() > cursor) widgets[cursor]->activate_left(); break;
			case right: if (widgets.size() > cursor) widgets[cursor]->activate_right(); break;
			}
		}

		void Draw(int x = 0, int y = 0);

		void update(Controls controls, float dt)
		{
			static Control prev_dir = none;
			Control dir = joy2dir(controls.joy_right);
			if (prev_dir != dir)
			{
				ControlCursor(dir);

				if (dynamic_cast<Slider *>(widgets[cursor]) != NULL && (dir == left || dir == right))
					prev_dir = none;
				else
					prev_dir = dir;
			}
		}
#else // !_DEBUG
		void SetActive(bool) {}
		bool IsActive() { return false; }
		void ControlCursor(Control c) {}
		void add_slider(char* id, float min, float max, float& param) {}
		void add_checkbox(char* id, bool &param) {}
		void add_text(char* id, std::string &param) {}
		void add_button(char* id, void(&callback)(void)) {}
		void Draw(int x = 0, int y = 0) {}
		void update(Controls controls, float dt) {}
#endif // !_DEBUG
	};
}

