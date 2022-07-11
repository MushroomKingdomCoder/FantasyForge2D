#include "Engine.h"

Engine::Engine()
	:
	wnd(1024u, 576u, "FantasyForge2D", { { 1024u,576u } }),
	kbd(wnd.kbd),
	mouse(wnd.mouse),
	gfx(wnd.gfx()),
	AppClock()
{

}

void Engine::Go()
{
	gfx.NewFrame();
	UpdateModel();
	ComposeFrame();
	gfx.EndFrame();
}

void Engine::UpdateModel()
{

}

void Engine::ComposeFrame()
{

}