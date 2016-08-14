#pragma once

#include <string>
#include <vector>
#include <algorithm>

namespace eae6320
{
	class DebugMenu
	{
#ifdef _DEBUG
		struct Widget
		{
		public:
			const std::string id;
			int draw(int x, int y) const;

			virtual void activate_left() = 0;
			virtual void activate_right() = 0;
			virtual char* text_format() const = 0;

			Widget(char* id) : id(id) {}
			virtual ~Widget() {}
		};
		class Slider : public Widget
		{
			static const int resolution = 64;
			const float min, max, range_recip;
			float &param;

			float normalized() const { return (param - min) * range_recip; }
			float denormalize(float x) const { return x * (max - min) + min; }
			int position() const { return (int) floor(normalized() * resolution); }

		public:
			Slider(char* id, float min, float max, float &param)
				: Widget(id), min(min), max(max), param(param), range_recip(1.0f / (max - min)) {}
			virtual ~Slider() {}

			virtual void activate_left()
			{
				param = denormalize((float) std::max(0, position() - 1));
			};
			virtual void activate_right()
			{
				param = denormalize((float) std::min(resolution, position() + 1));
			};

			virtual char* text_format() const;
		};
		class CheckBox : public Widget
		{
			bool &param;

		public:
			CheckBox(char* id, bool &param) : Widget(id), param(param) {}
			virtual ~CheckBox() {}

			virtual void activate_left() {}
			virtual void activate_right() { param = !param; }

			virtual char* text_format() const { return param ? "(#)" : "( )"; }
		};
		class Text : public Widget
		{
			char* &param;

		public:
			Text(char* id, char* &param) : Widget(id), param(param) {}
			virtual ~Text() {}

			virtual void activate_left() {}
			virtual void activate_right() {}

			virtual char* text_format() const { return param; }
		};

		std::vector<Widget *> widgets;
		bool active = false;

#endif // _DEBUG
	public:

		DebugMenu() {}
		~DebugMenu() {}

#ifdef _DEBUG
		void add_slider(char* id, float min, float max, float& param)
			{ widgets.push_back(new Slider(id, min, max, param)); }
		void add_checkbox(char* id, bool &param)
			{ widgets.push_back(new CheckBox(id, param)); }
		void add_text(char* id, char* &param)
			{ widgets.push_back(new Text(id, param)); }

		void SetActive(bool b) { active = b; }
		bool IsActive() { return active; }
		void Draw(int x = 0, int y = 0);
#else // !_DEBUG
		void SetActive(bool) {}
		bool IsActive() { return false; }
		void add_slider(char* id, float min, float max, float& param) {}
		void add_checkbox(char* id, bool &param) {}
		void add_text(char* id, char* &param) {}
		void Draw(int x = 0, int y = 0) {}
#endif // !_DEBUG
	};
}

