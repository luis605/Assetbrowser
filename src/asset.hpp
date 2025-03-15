#ifndef ASSET_HPP
#define ASSET_HPP

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include <string>
#include <filesystem>
#include <imgui_internal.h>
#include <iostream>
#include <vector>

namespace fs = std::filesystem;

constexpr int SCREEN_WIDTH = 800;
constexpr int SCREEN_HEIGHT = 450;
#define WINDOW_TITLE "Asset Browser"

typedef std::tuple<std::string, Texture> preview_item_t;

extern RenderTexture2D itemPreviewTexture;
extern Model itemPreviewModel;
extern Camera3D itemPreviewCamera;
extern Camera3D folderPreviewCamera;
extern std::vector<preview_item_t> folderPreviewItems;
extern fs::path currentDir;
extern std::string selectedFile;
extern ImGuiIO* io;

void Init();
void Render();
void Close();
void Dock();
void ConfigureDockLayout(ImGuiID dockspace_id);
void AssetViewer();
void PreviewFolder();
void FileBrowser();
void on_folder_change(const fs::path& path);
void on_file_select(const fs::path& path);
void render_item_preview();

#endif // ASSET_HPP