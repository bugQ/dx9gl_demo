/*
This file contains all of the function definitions for this example program
*/

// Header Files
//=============

#include "WindowsProgram.h"

#include "DebugMenu.h"

// Resource.h contains the #defines for the icon resources
// that the main window will use
#include "Resources/Resource.h"
// WindowsFunctions.h contains convenience functionality for Windows features;
// in this example program we just use it to get error messages
#include "../../Engine/Windows/WindowsFunctions.h"

// Graphics.h contains engine functions that perform all necessary
// graphics API calls during gameplay
#include "../../Engine/Graphics/Graphics.h"

#include "../../Engine/Physics/Terrain.h"
#include "../../Engine/Time/Time.h"
#include "../../Engine/UserInput/UserInput.h"

#include "Controller.h"
#include "FlyCam.h"
#include "Player.h"

#include "../../Engine/Debug_Runtime/UserOutput.h"
#include <sstream>

// Static Data Initialization
//===========================

using namespace eae6320;
using namespace eae6320::Graphics;

namespace
{
	// Instead of a pointer Windows provides a "handle"
	// to represent each window that is created
	// ("HWND" == "window handle").
	// In Windows every separate element
	// (buttons, text fields, scroll bars, etc.) is a "window",
	// and so a typical Windows program will have many different windows.
	// In our class, however, we will only have a single main window.
	HWND s_mainWindow = NULL;

	float camera_track_speed = 3.0f;
	float camera_pan_speed = 1.0f;

	/* begin hardcoded asset list.  whoops !!
	this will all be replaced with actual asset files, later. */

	// model_spec = mesh + effect indices
	typedef struct { size_t mesh_idx, mat_idx; Vector3 position; Vector3 scale; } model_spec;

	// sprite_spec = material + pixel rect + texel rect
	typedef struct { size_t mat_idx; Sprite::Rect xy; Sprite::Rect uv; } sprite_spec;

	Vector3 origin = Vector3(0.0f, 0.0f, 0.0f);
	Vector3 cm = Vector3(0.01f, 0.01f, 0.01f);

	Sprite::Rect standardUV = { 0.0f, 0.0f, 1.0f, 1.0f };

	const char * terrain_file = "data/ctf_collision.vib";
	const char * mesh_files[] =
	{ "data/ctf_ceiling.vib"
	, "data/ctf_cement.vib"
	, "data/ctf_floor.vib"
	, "data/ctf_metal.vib"
	, "data/ctf_railing.vib"
	, "data/ctf_walls.vib"
	};
	const char * material_files[] =
	{ "data/debug.mtb"
	, "data/ctf_cement.mtb"
	, "data/ctf_floor.mtb"
	, "data/ctf_metal.mtb"
	, "data/ctf_railing.mtb"
	, "data/ctf_walls.mtb"
	, "data/eae6320_a.mtb"
	, "data/numbers.mtb"
	};
	model_spec model_specs[] =
	{ { 0, 1, origin, cm }
	, { 1, 1, origin, cm }
	, { 2, 2, origin, cm }
	, { 3, 3, origin, cm }
	, { 4, 4, origin, cm }
	, { 5, 5, origin, cm }
	};
	sprite_spec sprite_specs[] =
	{ { 6,{ -1.2f, -0.4f, -0.6f, -1.0f }, standardUV }
	};
	/* end hardcoded asset list. */

	Mesh ** meshes;
	Material ** materials;
	Model ** models;
	size_t num_models;
	Sprite ** sprites;
	size_t num_sprites;
	Physics::Terrain * terrain;

	FlyCam * fly_cam;
	Player * player;

	Camera * active_cam;
	Controller * active_controller;

	Wireframe * wireframe;
	DebugMenu * debug_menu;

	std::string * fps_display;
	std::string * pos_display;

	struct {
		const float radius_min = 1.0f;
		const float radius_max = 5.0f;
		const float radius_default = 1.5f;
		float radius = radius_default;
		bool active;
		void reset() { radius = radius_default; }
		void draw(Wireframe* wireframe)
		{
			if (!active) return;
			wireframe->addSphere(Vector3(0, 0, 0), radius, 16, Vector4(1.0f, 0.7f, 0.2f, 1.0f));
		}
	} debug_sphere;

	void debug_sphere_reset() { debug_sphere.reset(); }
	void camera_reset()
	{
		fly_cam->fly_cam.position = Vector3(1.f, 1.f, 1.01f);
		fly_cam->fly_cam.rotation = Versor::rotation_y(3.14159265f);
	}

	// Window classes are almost always identified by name;
	// there is also a unique ATOM associated with them,
	// but in practice Windows expects to use the class name as an identifier.
	// If you don't change the name below from the default then
	// your program could have problems when it is run at the same time on the same computer
	// as one of your classmate's
	const char* s_mainWindowClass_name = "Bug's Main Window Class";
}

// Main Function
//==============

int CreateMainWindowAndReturnExitCodeWhenItCloses(const HINSTANCE i_thisInstanceOfTheProgram, const int i_initialWindowDisplayState)
{
	// Try to create the main window
	if (CreateMainWindow(i_thisInstanceOfTheProgram, i_initialWindowDisplayState))
	{
		// If the main window was successfully created wait for it to be closed
		const int exitCode = WaitForMainWindowToCloseAndReturnExitCode(i_thisInstanceOfTheProgram);
		return exitCode;
	}
	else
	{
		// If the main window wasn't created return a made-up error code
		return -1;
	}
}

// Helper Functions
//=================

template <typename T, size_t N> constexpr size_t countof(T(&)[N])
{
	return N;
}

template <size_t N> Mesh ** LoadMeshes(const char * (&spec)[N])
{
	Mesh ** out = new Mesh *[N];
	for (size_t i = 0; i < N; ++i) {
		Mesh::Data * data = Mesh::Data::FromBinFile(spec[i]);
		out[i] = new Mesh();
		if (!LoadMesh(*out[i], *data)) {
			delete out[i];
			out[i] = NULL;
		}
		delete data;
	}
	return out;
}

template <size_t N> Effect ** LoadEffects(const char * (&spec)[N])
{
	Effect ** out = new Effect *[N];
	Effect::Parent parent = Graphics::GetDevice();
	for (size_t i = 0; i < N; ++i)
		out[i] = Effect::FromFile(spec[i], parent);
	return out;
}

template <size_t N> Material ** LoadMaterials(const char * (&spec)[N])
{
	Material ** out = new Material *[N];
	Effect::Parent parent = Graphics::GetDevice();
	for (size_t i = 0; i < N; ++i)
		out[i] = Material::FromFile(spec[i], parent);
	return out;
}

template <size_t N> Model ** BuildModels(model_spec(&spec)[N],
	Mesh ** meshes, Material ** materials, size_t &num_models)
{
	Model ** out = new Model *[N];
	for (size_t i = 0; i < N; ++i)
		out[i] = new Model(
			*meshes[spec[i].mesh_idx],
			*materials[spec[i].mat_idx],
			spec[i].position,
			spec[i].scale);
	num_models = N;
	return out;
}

template <size_t N> Sprite ** BuildSprites(sprite_spec(&spec)[N],
	Material ** materials, size_t &num_sprites)
{
	Sprite ** out = new Sprite *[N];
	for (size_t i = 0; i < N; ++i)
		out[i] = new Sprite(
			*materials[spec[i].mat_idx],
			spec[i].xy,
			spec[i].uv);
	num_sprites = N;
	return out;
}

bool CreateMainWindow(const HINSTANCE i_thisInstanceOfTheProgram, const int i_initialWindowDisplayState)
{
	// Every window that Windows creates must belong to a "class".
	// Note that this is different than a C++ class (but similar in theory):
	// A windows class defines things that will be the same for every window
	// that belongs to the class.
	// (In other words, every window will have exactly one class,
	// but a windows class can have many windows.)
	// To create a new windows class it must be "registered" with Windows.
	ATOM mainWindowClass = RegisterMainWindowClass(i_thisInstanceOfTheProgram);
	if (mainWindowClass != NULL)
	{
		Vector3 maybej = Matrix4::rotation_q(Versor::rotation_z(1.57079633f)).predot0(Vector3::I).xyz();
		std::ostringstream oss;

		s_mainWindow = CreateMainWindowHandle(i_thisInstanceOfTheProgram, i_initialWindowDisplayState);
		if (s_mainWindow == NULL)
		{
			goto OnError;
		}

		if (!eae6320::Graphics::Initialize(s_mainWindow))
		{
			goto OnError;
		}

		meshes = LoadMeshes(mesh_files);
		materials = LoadMaterials(material_files);
		models = BuildModels(model_specs, meshes, materials, num_models);
		sprites = BuildSprites(sprite_specs, materials, num_sprites);

		terrain = Physics::Terrain::FromBinFile(terrain_file, cm);

		fly_cam = new FlyCam(Vector3(1.f, 1.f, 1.01f), 3.14159265f, camera_track_speed, camera_pan_speed);
		player = new Player(Vector3(0, 0, 0), 0, 1.5f, *terrain, 2.0f);

		/**
		oss << "(" << maybej.x << ", " << maybej.y << ", " << maybej.z << ")";
		UserOutput::Print(oss.str());
		/**/

		active_cam = &player->head_cam;
		active_controller = player;

		wireframe = new Wireframe(materials[0]);
		eae6320::Graphics::InitWireframe(*wireframe);

		fps_display = new std::string;
		pos_display = new std::string;

		debug_menu = new DebugMenu();
		debug_menu->add_text("fps", *fps_display);
		debug_menu->add_text("fly_cam.position", *pos_display);
		debug_menu->add_button("fly_cam.reset", camera_reset);
		debug_menu->add_checkbox("debug_sphere.active", debug_sphere.active);
		debug_menu->add_slider("debug_sphere.radius",
			debug_sphere.radius_min, debug_sphere.radius_max, debug_sphere.radius);
		debug_menu->add_button("debug_sphere.reset", debug_sphere_reset);

		return true;

	OnError:

		// Unregister the main window class
		{
			UnregisterMainWindowClass(i_thisInstanceOfTheProgram);
			mainWindowClass = NULL;
		}

		return false;
	}
	else
	{
		return false;
	}
}

int WaitForMainWindowToCloseAndReturnExitCode(const HINSTANCE i_thisInstanceOfTheProgram)
{
	// Wait for the main window to close
	int exitCode;
	bool wereThereErrors = WaitForMainWindowToClose(exitCode);

	// Clean up anything that was created/registered/initialized
	if (OnMainWindowClosed(i_thisInstanceOfTheProgram))
	{
		return exitCode;
	}
	else
	{
		if (wereThereErrors)
		{
			// If the program already had errors while waiting for the main window to close
			// the existing error exit code can be returned
			return exitCode;
		}
		else
		{
			// In a real program you might decide to define a special constant for this situation,
			// but for this example program -1 is a good arbitrary error code
			return -1;
		}
	}
}

// CreateMainWindow
//-----------------

HWND CreateMainWindowHandle(const HINSTANCE i_thisInstanceOfTheProgram, const int i_initialWindowDisplayState)
{
	// Create the main window
	HWND mainWindow;
	{
		// The window's "caption"
		// (The text that is displayed in the title bar)
		const char* windowCaption = "Bug's EAE6330 Game";
		// The window's style
		const DWORD windowStyle =
			// "Overlapped" is basically the same as "top-level"
			WS_OVERLAPPED
			// The caption is the title bar when in windowed-mode
			| WS_CAPTION
			// The window should never change dimensions, so only a minimize box is allowed
			| WS_MINIMIZEBOX
			// The system menu appears when you right-click the title bar
			| WS_SYSMENU;
		// The window's extended style
		const DWORD windowStyle_extended =
			// The following is a macro to make the extended style the default top-level look
			WS_EX_OVERLAPPEDWINDOW;
		// The width and height of the window.
		// A game cares about the width and height of the actual "client area",
		// which is the part of the window that doesn't include the borders and title bar;
		// this means that if we say that a game runs at a resolution of 800 x 600,
		// the actual window will be slightly bigger than that.
		// Initially, then, the window will be created with default values that Windows chooses
		// and then resized after creation)
		const int width = CW_USEDEFAULT;
		const int height = CW_USEDEFAULT;
		// The initial position of the window
		// (We don't care, and will let Windows decide)
		const int position_x = CW_USEDEFAULT;
		const int position_y = CW_USEDEFAULT;
		// Handle to the parent of this window
		// (Since this is our main window, it can't have a parent)
		const HWND hParent = NULL;
		// Handle to the menu for this window
		// (The main window won't have a menu)
		const HMENU hMenu = NULL;
		// Handle to the instance of the program that this window should be associated with
		const HINSTANCE hProgram = i_thisInstanceOfTheProgram;
		// Any arbitrary pointer can be associated with this window;
		// usually a program's own custom representation of the window will be used, like this:
		// void* userData = someCustomStructOrClass;
		// Since this example program is simple, though, no data will be associated:
		void* userData = NULL;

		// Ask Windows to create the specified window.
		// CreateWindowEx() will return a handle to the window,
		// which is what we'll use to communicate with Windows about this window
		mainWindow = CreateWindowEx(windowStyle_extended, s_mainWindowClass_name, windowCaption, windowStyle,
			position_x, position_y, width, height,
			hParent, hMenu, hProgram, userData);
		if (mainWindow == NULL)
		{
			const char* errorCaption = "No Main Window";
			std::string errorMessage("Windows failed to create the main window: ");
			errorMessage += eae6320::GetLastWindowsError();
			MessageBox(NULL, errorMessage.c_str(), errorCaption, MB_OK | MB_ICONERROR);
			return NULL;
		}
	}

	// Change the window's size based on the desired client area resolution
	{
		// In a real game these values would come from an external source
		// rather than be hard-coded
		const int desiredWidth = 1024;
		const int desiredHeight = 576;

		// Calculate how much of the window is coming from the "non-client area"
		// (the borders and title bar)
		RECT windowCoordinates;
		struct
		{
			long width;
			long height;
		} nonClientAreaSize;
		{
			// Get the coordinates of the entire window
			if (GetWindowRect(mainWindow, &windowCoordinates) == FALSE)
			{
				std::string errorMessage("Windows failed to get the coordinates of the main window: ");
				errorMessage += eae6320::GetLastWindowsError();
				MessageBox(NULL, errorMessage.c_str(), NULL, MB_OK | MB_ICONERROR);
				goto OnError;
			}
			// Get the dimensions of the client area
			RECT clientDimensions;
			if (GetClientRect(mainWindow, &clientDimensions) == FALSE)
			{
				std::string errorMessage("Windows failed to get the dimensions of the main window's client area: ");
				errorMessage += eae6320::GetLastWindowsError();
				MessageBox(NULL, errorMessage.c_str(), NULL, MB_OK | MB_ICONERROR);
				goto OnError;
			}
			// Get the difference between them
			nonClientAreaSize.width = (windowCoordinates.right - windowCoordinates.left) - clientDimensions.right;
			nonClientAreaSize.height = (windowCoordinates.bottom - windowCoordinates.top) - clientDimensions.bottom;
		}
		// Resize the window
		{
			BOOL shouldTheResizedWindowBeRedrawn = TRUE;
			if (MoveWindow(mainWindow, windowCoordinates.left, windowCoordinates.top,
				desiredWidth + nonClientAreaSize.width, desiredHeight + nonClientAreaSize.height,
				shouldTheResizedWindowBeRedrawn) == FALSE)
			{
				std::string errorMessage("Windows failed to resize the main window: ");
				errorMessage += eae6320::GetLastWindowsError();
				MessageBox(NULL, errorMessage.c_str(), NULL, MB_OK | MB_ICONERROR);
				goto OnError;
			}
		}
	}


	// Display the window in the initial state that Windows requested
	ShowWindow(mainWindow, i_initialWindowDisplayState);

	{
		std::string errorMessage;
		Time::Initialize(&errorMessage);
		if (!errorMessage.empty()) {
			MessageBox(NULL, errorMessage.c_str(), NULL, MB_OK | MB_ICONERROR);
			goto OnError;
		}
	}

	return mainWindow;

OnError:

	if (DestroyWindow(mainWindow) == FALSE)
	{
		std::string errorMessage("Windows failed to destroy the main window "
			"after an error in creation: ");
		errorMessage += eae6320::GetLastWindowsError();
		MessageBox(NULL, errorMessage.c_str(), NULL, MB_OK | MB_ICONERROR);
	}
	mainWindow = NULL;

	return NULL;
}

ATOM RegisterMainWindowClass(const HINSTANCE i_thisInstanceOfTheProgram)
{
	WNDCLASSEX wndClassEx = { 0 };
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.hInstance = i_thisInstanceOfTheProgram;

	// The class's style
	wndClassEx.style = 0;
#ifdef EAE6320_PLATFORM_GL
	// Reserve a unique device context for this window for OpenGL's use
	wndClassEx.style |= CS_OWNDC;
#endif
	// The function that will process all of the messages
	// that Windows will send to windows of this class
	wndClassEx.lpfnWndProc = OnMessageReceived;
	// Extra bytes can be set aside in the class for user data
	wndClassEx.cbClsExtra = 0;
	// Extra bytes can be set aside for each window of this class,
	// but this is usually specified for each window individually
	wndClassEx.cbWndExtra = 0;
	// The large and small icons that windows of this class should use
	// (These can be found in the Resources folder; feel free to change them)
	wndClassEx.hIcon = LoadIcon(i_thisInstanceOfTheProgram, MAKEINTRESOURCE(IDI_BIG));
	wndClassEx.hIconSm = LoadIcon(i_thisInstanceOfTheProgram, MAKEINTRESOURCE(IDI_SMALL));
	// The cursor that should display when the mouse pointer is over windows of this class
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	// The "brush" that windows of this class should use as a background
	// (Setting this is a bit confusing but not important,
	// so don't be alarmed if the next line looks scary)
	wndClassEx.hbrBackground = reinterpret_cast<HBRUSH>(IntToPtr(COLOR_BACKGROUND + 1));
	// A menu can be specified that all windows of this class would use by default,
	// but usually this is set for each window individually
	wndClassEx.lpszMenuName = NULL;
	// The class name (see comments where this is initialized)
	wndClassEx.lpszClassName = s_mainWindowClass_name;

	// Now all of the above information is given to Windows.
	// If all goes well, the class will be successfully registered
	// and it can be specified by name when creating the main window.
	const ATOM mainWindowClass = RegisterClassEx(&wndClassEx);
	if (mainWindowClass == NULL)
	{
		const char* errorCaption = "No Main Window Class";
		std::string errorMessage("Windows failed to register the main window's class: ");
		errorMessage += eae6320::GetLastWindowsError();
		MessageBox(NULL, errorMessage.c_str(), errorCaption, MB_OK | MB_ICONERROR);
	}
	return mainWindowClass;
}

// WaitForMainWindowToCloseAndReturnExitCode
//------------------------------------------

bool CleanupMainWindow()
{
	if (s_mainWindow != NULL)
	{
		if (DestroyWindow(s_mainWindow) != FALSE)
		{
			s_mainWindow = NULL;
		}
		else
		{
			const char* errorCaption = "Couldn't Destroy Main Window";
			std::string errorMessage("Windows failed to destroy the main window: ");
			errorMessage += eae6320::GetLastWindowsError();
			MessageBox(NULL, errorMessage.c_str(), errorCaption, MB_OK | MB_ICONERROR);
			return false;
		}

		for (size_t i = countof(model_specs); i > 0; --i)
			delete models[i - 1];
		delete[] models;
		delete[] materials;
		delete[] meshes;
		delete[] sprites;

		delete wireframe;
		delete debug_menu;
	}
	return true;
}

bool OnMainWindowClosed(const HINSTANCE i_thisInstanceOfTheProgram)
{
	bool wereThereErrors = false;

	if (!eae6320::Graphics::ShutDown())
	{
		wereThereErrors = true;
	};

	if (!CleanupMainWindow())
	{
		wereThereErrors = true;
	}

	if (!UnregisterMainWindowClass(i_thisInstanceOfTheProgram))
	{
		wereThereErrors = true;
	}

	return !wereThereErrors;
}

LRESULT CALLBACK OnMessageReceived(HWND i_window, UINT i_message, WPARAM i_wParam, LPARAM i_lParam)
{
	// DispatchMessage() will send messages to the main window here.
	// There are many messages that get sent to a window,
	// but games can ignore most of them
	// and let Windows handle them in the default way.

	// Process any messages that the game cares about
	// (any messages that are processed here should return a value
	// rather than letting the default processing function try to handle them a second time)
	switch (i_message)
	{
		// A key has been pressed down, and this is the translated character
	case WM_CHAR:
	{
		// This isn't usually the best way to handle keyboard input in a real-time game,
		// but it is a convenient way to handle an exit key
		// (The WPARAM input parameter indicates which key was pressed,
		// but this example program only cares about the escape key)
		if (i_wParam == VK_ESCAPE)
		{
			// You do _not_ need to do this in your game,
			// but this example program shows you how to display a "message box"
			// that asks the user for confirmation
			int result;
			{
				const char* caption = "Exit Program?";
				const char* message = "Are you sure you want to quit?";
				result = MessageBox(s_mainWindow, message, caption, MB_YESNO | MB_ICONQUESTION);
			}
			if (result == IDYES)
			{
				// Instruct Windows to send a WM_QUIT message
				{
					// The exit code is ignored,
					// but just as an example of what's possible a different exit code is sent here
					// than when the user closes the program in the usual way
					// (by clicking the X in the upper-right hand corner).
					int exitCode = 1;
					PostQuitMessage(exitCode);
				}

				// For WM_CHAR messages, return 0 to indicate that it was processed
				return 0;
			}
			else
			{
				// If the user doesn't select "Yes" to quit
				// then do nothing to keep running the program
			}
		}

		// If the key press wasn't handled pass it on to Windows to process in the default way
		break;
	}
	// The window's nonclient area is being destroyed
	case WM_NCDESTROY:
	{
		// This is the last message a window will receive
		// (Any child windows have already been destroyed).
		// After this message has been processed the window's handle will be invalid:
		s_mainWindow = NULL;

		// When the main window is destroyed
		// a WM_QUIT message should be sent
		// (if this isn't done the application would continue to run with no window).
		// This is where the exitCode in WaitForShutdown() comes from:
		int exitCode = 0;	// Arbitrary de facto success code
		PostQuitMessage(exitCode);	// This sends a WM_QUIT message

									// For WM_NCDESTROY messages, return 0 to indicate that it was processed
		return 0;
	}
	}

	// Pass any messages that weren't handled on to Windows
	return DefWindowProc(i_window, i_message, i_wParam, i_lParam);
}

bool UnregisterMainWindowClass(const HINSTANCE i_thisInstanceOfTheProgram)
{
	if (UnregisterClass(s_mainWindowClass_name, i_thisInstanceOfTheProgram) != FALSE)
	{
		return true;
	}
	else
	{
		const char* errorCaption = "Couldn't Unregister Main Window Class";
		std::string errorMessage("Windows failed to unregister the main window's class: ");
		errorMessage += eae6320::GetLastWindowsError();
		MessageBox(NULL, errorMessage.c_str(), errorCaption, MB_OK | MB_ICONERROR);
		return false;
	}
}

inline Vector4 color(bool b) { return b ? Vector4(0.7f, 1, 0.4f, 1) : Vector4(1, 0.4f, 0.7f, 1); }
inline float randf() { return (float)rand() / RAND_MAX - 0.5f; }

void Render()
{
	Clear();
	BeginFrame();

	for (size_t i = 0; i < num_models; ++i)
		if (!models[i]->mat->effect->render_state.alpha)
			DrawModel(*models[i], *active_cam);
	for (size_t i = 0; i < num_models; ++i)
		if (models[i]->mat->effect->render_state.alpha)
			DrawModel(*models[i], *active_cam);

	debug_sphere.draw(wireframe);
	for (size_t i = 0; i < terrain->num_triangles; ++i)
	{
		Triangle3 tri = terrain->triangles[i];
		wireframe->addLine(tri.a, Vector4::One, tri.b, Vector4::One);
		wireframe->addLine(tri.b, Vector4::One, tri.c, Vector4::One);
		wireframe->addLine(tri.c, Vector4::One, tri.a, Vector4::One);
	}
	/**
	for (size_t i = 0; i < 2048; ++i)
	{
		Vector3 n;
		Vector3 o = player->position + Versor::rotation_y(player->yaw).rotate(Vector3(0,0,0.0025f));
		Vector3 d = Versor::rotation_y(player->yaw).rotate(Vector3(randf() * 32, randf() * 18, -16));
		//Vector3 o(randf() * 6, randf() * 6, 0.1f);
		//Vector3 d(randf() * 0.05, randf() * 0.05, -0.2f);
		//Vector3 d(randf() * 6, randf() * 6, 4.f);
		bool collided = terrain->intersect_ray(o, d, &n) < 1;
		wireframe->addLine(o, color(collided), o + d, color(collided));
	}
	/**/
	eae6320::Graphics::DrawWireframe(*wireframe, *active_cam);
	wireframe->clear();

	for (size_t i = 0; i < num_sprites; ++i)
		if (sprites[i]->active)
			DrawSprite(*sprites[i]);

	fps_display->swap(std::to_string(Time::GetFramesPerSecond()));
	std::ostringstream oss;
	Vector3 pos = player->position;
	oss << "(" << pos.x << ", " << pos.y << ", " << pos.z << ")";
	pos_display->swap(oss.str());

	if(active_controller == debug_menu)
		debug_menu->Draw();

	EndFrame();
}

bool WaitForMainWindowToClose(int& o_exitCode)
{
	// Any time something happens that Windows cares about, it will send the main window a message.

	// One of the messages it sends is that the application should quit;
	// this can be sent when a user closes the window
	// (e.g. presses the X in the upper-right corner),
	// but we can also tell Windows to send a quit message if the user chooses to quit from within the game
	// (e.g. from an in-game menu)

	// Enter an infinite loop that will continue until a quit message (WM_QUIT) is received from Windows
	MSG message = { 0 };
	do
	{
		// To send us a message, Windows will add it to a queue.
		// Most Windows applications should wait until a message is received and then react to it.
		// Real-time programs, though, must continually draw new images to the screen as fast as possible
		// and only pause momentarily when there is a Windows message to deal with.

		// This means that the first thing that must be done every iteration of the game loop is to "peek" at the message queue
		// and see if there are any messages from Windows that need to be handled
		bool hasWindowsSentAMessage;
		{
			HWND getMessagesFromAnyWindowBelongingToTheCurrentThread = NULL;
			unsigned int getAllMessageTypes = 0;
			unsigned int ifAMessageExistsRemoveItFromTheQueue = PM_REMOVE;
			hasWindowsSentAMessage = PeekMessage(&message, getMessagesFromAnyWindowBelongingToTheCurrentThread,
				getAllMessageTypes, getAllMessageTypes, ifAMessageExistsRemoveItFromTheQueue) == TRUE;
		}
		if (!hasWindowsSentAMessage)
		{
			// Usually there will be no messages in the queue, and the game can run
			Time::OnNewFrame();
			float dt = Time::GetSecondsElapsedThisFrame();

			Vector2 joy_left = Vector2::Zero;
			Vector2 joy_right = Vector2::Zero;
			if (UserInput::IsKeyPressed('W')) joy_left += Vector2::J;
			if (UserInput::IsKeyPressed('S')) joy_left -= Vector2::J;
			if (UserInput::IsKeyPressed('A')) joy_left -= Vector2::I;
			if (UserInput::IsKeyPressed('D')) joy_left += Vector2::I;
			if (UserInput::IsKeyPressed(VK_UP))    joy_right += Vector2::J;
			if (UserInput::IsKeyPressed(VK_DOWN))  joy_right -= Vector2::J;
			if (UserInput::IsKeyPressed(VK_LEFT))  joy_right -= Vector2::I;
			if (UserInput::IsKeyPressed(VK_RIGHT)) joy_right += Vector2::I;
			Controller::Controls controls = { joy_left, joy_right };

#ifdef _DEBUG
			static int prevkey = -1;
			if (active_controller == debug_menu)
			{
				if (UserInput::IsKeyPressed(VK_OEM_3))
				{
					if (prevkey != VK_OEM_3)
					{
						if (active_cam == &player->head_cam)
							active_controller = player;
						else if (active_cam == &fly_cam->fly_cam)
							active_controller = fly_cam;
						prevkey = VK_OEM_3;
					}
				}
				else
					prevkey = -1;
			}
			else if (UserInput::IsKeyPressed(VK_OEM_3))
			{
				if (prevkey != VK_OEM_3)
				{
					active_controller = debug_menu;
					prevkey = VK_OEM_3;
				}
			}
			else if (active_controller == fly_cam)
			{
				if (UserInput::IsKeyPressed(VK_TAB))
				{
					if (prevkey != VK_TAB)
					{
						active_cam = &player->head_cam;
						active_controller = player;
						prevkey = VK_TAB;
					}
				}
				else
					prevkey = -1;
			}
			else if (UserInput::IsKeyPressed(VK_TAB))
			{
				if (prevkey != VK_TAB)
				{
					active_cam = &fly_cam->fly_cam;
					active_controller = fly_cam;
					prevkey = VK_TAB;
				}
			}
			else
				prevkey = -1;
#endif // _DEBUG

			active_controller->update(controls, dt);

			Render();
		}
		else
		{
			// If Windows _has_ sent a message, this iteration of the loop will handle it.
			// Note that Windows messages will take precedence over our game functionality;
			// this is because if we don't handle Windows messages the window can appear sluggish to the user
			// (if s/he tries to move it, for example, but we give too much precedence to our own game code).

			// First, the message must be "translated"
			// (Key presses are translated into character messages)
			TranslateMessage(&message);

			// Then, the message is sent on to the appropriate processing function.
			// This function is specified in the lpfnWndProc field of the WNDCLASSEX struct
			// used to register a class with Windows.
			// In the case of the main window in this example program
			// it will always be OnMessageReceived()
			DispatchMessage(&message);
		}
	} while (message.message != WM_QUIT);

	// The exit code for the application is stored in the WPARAM of a WM_QUIT message
	o_exitCode = static_cast<int>(message.wParam);

	return true;
}
