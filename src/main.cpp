#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Window title"

int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
	SetTargetFPS(60);
	rlImGuiSetup(true);

	while (!WindowShouldClose())
	{
		BeginDrawing();
		rlImGuiBegin();

		ClearBackground(RAYWHITE);

		const char* text = "OMG! IT WORKS!";
		const Vector2 text_size = MeasureTextEx(GetFontDefault(), text, 20, 1);
		DrawText(text, SCREEN_WIDTH / 2 - text_size.x / 2, SCREEN_HEIGHT / 2, 20, BLACK);

		bool open = true;
		ImGui::ShowDemoWindow(&open);

		rlImGuiEnd();
		EndDrawing();
	}
	rlImGuiShutdown();
	CloseWindow();

	return 0;
}