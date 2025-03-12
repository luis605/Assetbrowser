#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include <string>
#include <filesystem>
#include <imgui_internal.h>
#include <iostream>

namespace fs = std::filesystem;

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define WINDOW_TITLE "Asset Browser"

std::string DEMO_PATH = "D:/Projekte/RayCity/assets";

static fs::path currentDir(DEMO_PATH);
static std::string selectedFile;

RenderTexture2D itemPreviewTexture;
Model itemPreviewModel;
Camera3D itemPreviewCamera;

typedef std::tuple<std::string, Texture> preview_item_t;
std::vector<preview_item_t> folderPreviewItems;

Camera3D folderPreviewCamera;

void on_folder_change(const fs::path& path)
{
	for (auto& item : folderPreviewItems)
	{
		UnloadTexture(std::get<1>(item));
	}
	folderPreviewItems.clear();

	currentDir = path;
	std::cout << "Folder changed to: " << currentDir.string() << std::endl;

	// iterate over all files in the directory
	for (const auto& entry : fs::directory_iterator(currentDir))
	{
		if (!entry.is_directory() && entry.path().extension() == ".obj")
		{
			RenderTexture2D texture = LoadRenderTexture(64, 64);
			folderPreviewItems.emplace_back(std::make_tuple(entry.path().string(), texture.texture));

			Model model = LoadModel(entry.path().string().c_str());

			BeginTextureMode(texture);
			ClearBackground(Color(0,0,0,0));
			BeginMode3D(folderPreviewCamera);

			DrawModel(model, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

			EndMode3D();
			EndTextureMode();
			UnloadModel(model);
		}
	}

}

void on_file_select(const fs::path& path)
{
	selectedFile = path.string();
	itemPreviewModel = LoadModel(selectedFile.c_str());

	std::cout << "File selected: " << path.filename().string() << std::endl;
}

void render_item_preview()
{
	if (GetMouseWheelMove() == 0 || ImGui::IsWindowHovered())
		UpdateCamera(&itemPreviewCamera, CAMERA_ORBITAL);
	BeginTextureMode(itemPreviewTexture);
	ClearBackground(Color(0,0,0,0));
	BeginMode3D(itemPreviewCamera);
	DrawModel(itemPreviewModel, Vector3{ 0.0f, 0.0f, 0.0f }, 1.0f, WHITE);

	EndMode3D();
	EndTextureMode();
}


#ifndef _DEBUG

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#endif
int main(void)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(60);
	rlImGuiSetup(true);

	itemPreviewTexture = LoadRenderTexture(256, 256);
	itemPreviewCamera.position = Vector3{ 0.0f, 1.0f, 1.0f };
	itemPreviewCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	itemPreviewCamera.up = Vector3{ 0.0f, -1.0f, 0.0f };
	itemPreviewCamera.fovy = 45.0f;
	itemPreviewCamera.projection = CAMERA_PERSPECTIVE;

	folderPreviewCamera.position = Vector3{ 0.0f, 1.0f, 1.0f };
	folderPreviewCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	folderPreviewCamera.up = Vector3{ 0.0f, -1.0f, 0.0f };
	folderPreviewCamera.fovy = 45.0f;
	folderPreviewCamera.projection = CAMERA_PERSPECTIVE;

	// Enable docking in ImGui
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// This flag ensures we build the default docking layout only once
	static bool firstDock = true;

	while (!WindowShouldClose())
	{
		BeginDrawing();

		ClearBackground(BLACK);
		rlImGuiBegin();

		// Create a full-screen window that will host the dockspace
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(io.DisplaySize);
		ImGui::Begin("DockSpace Window", nullptr,
					 ImGuiWindowFlags_NoTitleBar |
						 ImGuiWindowFlags_NoCollapse |
						 ImGuiWindowFlags_NoResize |
						 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoBringToFrontOnFocus);

		// Create dockspace
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

		// Set up default layout once
		if (firstDock)
		{
			firstDock = false;
			ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
			ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
			ImGui::DockBuilderSetNodeSize(dockspace_id, io.DisplaySize);

			// Split dockspace into left and right nodes
			ImGuiID dock_left, dock_right;
			ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.7f, &dock_left, &dock_right);

			// Split left node vertically into two: top ("Asset View") and bottom ("Folder Preview")
			ImGuiID dock_asset, dock_folder;
			ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.5f, &dock_asset, &dock_folder);

			// Dock our windows
			ImGui::DockBuilderDockWindow("Asset View", dock_asset);
			ImGui::DockBuilderDockWindow("Folder Preview", dock_folder);
			ImGui::DockBuilderDockWindow("File Browser", dock_right);

			ImGui::DockBuilderFinish(dockspace_id);
		}

		ImGui::End(); // End DockSpace Window

		// Dockable window: Asset View (top left)
		ImGui::Begin("Asset View");
		if (!selectedFile.empty())
		{
			ImGui::Text("Selected File: %s", selectedFile.c_str());

			auto currHeight = itemPreviewTexture.texture.height;
			auto currWidth = itemPreviewTexture.texture.width;

			auto spaceAvailable = ImGui::GetContentRegionAvail();

			if (currHeight != spaceAvailable.y || currWidth != spaceAvailable.x)
			{
				UnloadRenderTexture(itemPreviewTexture);
				itemPreviewTexture = LoadRenderTexture(spaceAvailable.x, spaceAvailable.y);
			}

			if (!selectedFile.empty())
			{
				render_item_preview();
			}

			rlImGuiImage(&itemPreviewTexture.texture);
		}
		ImGui::End();

		// Dockable window: Folder Preview (bottom left)
		ImGui::Begin("Folder Preview");
		ImGui::Text("Folder Preview for: %s", currentDir.filename().string().c_str());

		int currX = 0;
		int availableWidth = static_cast<int>(ImGui::GetContentRegionAvail().x);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 8));
		for (int i = 0; i < folderPreviewItems.size(); i++)
		{
			auto& item = folderPreviewItems[i];
			auto& path = std::get<0>(item);
			auto& texture = std::get<1>(item);
			auto name = fs::path(path).filename().string();

			if (rlImGuiImageButtonSize(name.c_str(), &texture, Vector2(64, 64)))
			{
				on_file_select(fs::path(path));
			}

			currX += 72 + 8;

			if (currX + 72 > availableWidth)
			{
				currX = 0;
			} else
			{
				ImGui::SameLine();
			}
		}
		ImGui::PopStyleVar();

		ImGui::End();

		// Dockable window: File Browser (right)
		ImGui::Begin("File Browser");
		ImGui::Text("Current Directory: %s", currentDir.string().c_str());
		if (currentDir != fs::path(DEMO_PATH))
		{
			if (ImGui::Button(".."))
			{
				on_folder_change(currentDir.parent_path());
			}
		}

		try
		{
			std::vector<fs::directory_entry> dirs, files;
			for (const auto &entry : fs::directory_iterator(currentDir))
			{
				if (entry.is_directory())
					dirs.push_back(entry);
				else
					files.push_back(entry);
			}

			// Display directories
			for (const auto &dir : dirs)
			{
				if (ImGui::Selectable(dir.path().filename().string().c_str()))
				{
					on_folder_change(dir.path());
				}
			}

			// Display files
			for (const auto &file : files)
			{
				if (file.path().extension() == ".obj")
				{
					if (ImGui::Selectable(file.path().filename().string().c_str()))
					{
						on_file_select(file.path());
					}
				}
			}
		}
		catch (const fs::filesystem_error &e)
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
		}
		ImGui::End();

		rlImGuiEnd();
		EndDrawing();
	}

	rlImGuiShutdown();
	CloseWindow();
	return 0;
}
