#ifdef _DEBUG

#include "DebugMenu.h"
#include "../../Engine/Graphics/Graphics.h"
#include <sstream>

namespace eae6320
{

int DebugMenu::Widget::draw(int x, int y) const
{
	std::ostringstream builder;
	builder << id << "\n  " << text_format();
	return Graphics::DrawDebugText(x, y, builder.str());
}

char* DebugMenu::Slider::text_format() const
{
	const size_t len = resolution / 4 + 2;
	char *gauge = new char[len + 1];

	int ticks = std::max(0, position());

	for (int i = 1; i < len - 1; ++i)
	{
		switch (ticks)
		{
		case 0:
			gauge[i] = ' ';
			break;
		case 1:
			gauge[i] = '-';
			ticks = 0;
			break;
		case 2:
			gauge[i] = '+';
			ticks = 0;
			break;
		case 3:
			gauge[i] = '=';
			ticks = 0;
			break;
		default:
			gauge[i] = '#';
			ticks -= 4;
		}
	}

	gauge[0] = param < min ? '<' : '[';
	gauge[len - 1] = param > max ? '>' : ']';
	gauge[len] = '\0';

	return gauge;
}

void DebugMenu::Draw(int x, int y)
{
	if (!active) return;

	/*
	std::vector<Widget *>::const_iterator it;
	for (it = widgets.begin(); it != widgets.end(); ++it)
		y += (*it)->draw(x, y);
	*/

	std::ostringstream builder;
	std::vector<Widget *>::const_iterator it;
	for (it = widgets.begin(); it != widgets.end(); ++it)
		builder << (*it)->id << "\n  " << (*it)->text_format() << "\n";
	Graphics::DrawDebugText(x, y, builder.str());
}

}

#endif // _DEBUG