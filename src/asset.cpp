#include "asset.hpp"

RenderTexture2D itemPreviewTexture;
Model itemPreviewModel;
Camera3D itemPreviewCamera;
Camera3D folderPreviewCamera;
std::vector<preview_item_t> folderPreviewItems;
fs::path currentDir;
std::string selectedFile;
ImGuiIO* io;

void Init()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(-1);
    rlImGuiSetup(true);

    itemPreviewTexture = LoadRenderTexture(256, 256);
    itemPreviewCamera.position = Vector3{ 0.0f, 6.0f, 6.0f };
    itemPreviewCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    itemPreviewCamera.up = Vector3{ 0.0f, -1.0f, 0.0f };
    itemPreviewCamera.fovy = 45.0f;
    itemPreviewCamera.projection = CAMERA_PERSPECTIVE;

    folderPreviewCamera.position = Vector3{ 0.0f, 6.0f, 6.0f };
    folderPreviewCamera.target = Vector3{ 0.0f, 0.0f, 0.0f };
    folderPreviewCamera.up = Vector3{ 0.0f, -1.0f, 0.0f };
    folderPreviewCamera.fovy = 45.0f;
    folderPreviewCamera.projection = CAMERA_PERSPECTIVE;

    // Enable docking in ImGui
    io = &ImGui::GetIO();
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Set up initial directory
    currentDir = fs::path("assets/");
}

void Dock()
{
    static bool firstTimeDock = true;
    static ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io->DisplaySize);

    ImGuiWindowFlags dockWindowFlags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoDocking;

    ImGui::Begin("DockSpace Window", nullptr, dockWindowFlags);

    ImGuiDockNodeFlags dockFlags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockFlags);

    if (firstTimeDock)
    {
        ConfigureDockLayout(dockspace_id);
        firstTimeDock = false;
    }

    ImGui::End();
}

void ConfigureDockLayout(ImGuiID dockspace_id)
{
    ImGui::DockBuilderRemoveNode(dockspace_id); // Clear previous layout
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, io->DisplaySize);

    // Split dockspace into left and right nodes with 70/30 ratio
    ImGuiID dock_left, dock_right;
    ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.7f, &dock_left, &dock_right);

    // Split left node vertically into asset view (top) and folder preview (bottom)
    ImGuiID dock_asset, dock_folder;
    ImGui::DockBuilderSplitNode(dock_left, ImGuiDir_Up, 0.5f, &dock_asset, &dock_folder);

    // Dock windows to their respective areas
    ImGui::DockBuilderDockWindow("Asset View", dock_asset);
    ImGui::DockBuilderDockWindow("Folder Preview", dock_folder);
    ImGui::DockBuilderDockWindow("File Browser", dock_right);

    ImGui::DockBuilderFinish(dockspace_id);
}

void AssetViewer()
{
    ImGui::Begin("Asset View");

    if (selectedFile.empty())
    {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No file selected.");
    }
    else
    {
        ImGui::Text("Selected File: %s", selectedFile.c_str());

        ImVec2 spaceAvailable = ImGui::GetContentRegionAvail();

        // Only resize render texture when needed to avoid performance issues
        if (itemPreviewTexture.texture.height != static_cast<int>(spaceAvailable.y) ||
            itemPreviewTexture.texture.width != static_cast<int>(spaceAvailable.x))
        {
            // Make sure dimensions are at least 1x1 to avoid rendering errors
            float width = fmaxf(1.0f, spaceAvailable.x);
            float height = fmaxf(1.0f, spaceAvailable.y);

            UnloadRenderTexture(itemPreviewTexture);
            itemPreviewTexture = LoadRenderTexture(width, height);
        }

        render_item_preview();

        if (IsRenderTextureValid(itemPreviewTexture))
        {
            rlImGuiImage(&itemPreviewTexture.texture);
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error loading preview");
        }
    }
    ImGui::End();
}

void PreviewFolder()
{
    ImGui::Begin("Folder Preview");
    ImGui::TextColored(ImVec4(0.2f, 0.6f, 1.0f, 1.0f), "Folder: %s", currentDir.filename().string().c_str());
    ImGui::Separator();

    float availableWidth = ImGui::GetContentRegionAvail().x;
    constexpr float itemSize = 64.0f;
    constexpr float itemSpacing = 8.0f;
    float itemTotalWidth = itemSize + itemSpacing;

    // Calculate the number of items per row
    int itemsPerRow = std::max(1, static_cast<int>(availableWidth / itemTotalWidth));

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(itemSpacing, itemSpacing));

    for (size_t i = 0; i < folderPreviewItems.size(); i++)
    {
        // Use structured binding for clarity (C++17)
        const auto& [path, texture] = folderPreviewItems[i];
        std::string name = fs::path(path).filename().string();

        ImGui::BeginGroup();
        if (rlImGuiImageButtonSize(name.c_str(), &texture, Vector2(itemSize, itemSize)))
        {
            on_file_select(fs::path(path));
        }

        // Truncate label if too wide
        std::string displayName = name;
        if (ImGui::CalcTextSize(displayName.c_str()).x > itemSize)
        {
            displayName = displayName.substr(0, 8) + "...";
        }
        float textWidth = ImGui::CalcTextSize(displayName.c_str()).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (itemSize - textWidth) * 0.5f);
        ImGui::TextWrapped("%s", displayName.c_str());
        ImGui::EndGroup();

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", name.c_str());
            ImGui::EndTooltip();
        }

        // Row wrapping logic
        if ((i + 1) % itemsPerRow != 0 && i < folderPreviewItems.size() - 1)
        {
            ImGui::SameLine();
        }
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void FileBrowser()
{
    ImGui::Begin("File Browser");
    ImGui::Text("Current Directory: %s", currentDir.string().c_str());

    if (ImGui::Button(".."))
    {
        on_folder_change(currentDir.parent_path());
    }

    try
    {
        std::vector<fs::directory_entry> dirs, files;
        for (const fs::directory_entry& entry : fs::directory_iterator(currentDir))
        {
            if (entry.is_directory())
                dirs.push_back(entry);
            else
                files.push_back(entry);
        }

        // Display directories
        for (const fs::directory_entry& dir : dirs)
        {
            if (ImGui::Selectable(dir.path().filename().string().c_str()))
            {
                on_folder_change(dir.path());
            }
        }

        // Display files with .obj extension
        for (const fs::directory_entry& file : files)
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
    catch (const fs::filesystem_error& e)
    {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
    }
    ImGui::End();
}

void Render()
{
    while (!WindowShouldClose())
    {
        BeginDrawing();

        ClearBackground(BLACK);
        rlImGuiBegin();

        Dock();
        AssetViewer();
        PreviewFolder();
        FileBrowser();

        rlImGuiEnd();
        EndDrawing();
    }
}

void Close()
{
    rlImGuiShutdown();
    CloseWindow();
}


void on_folder_change(const fs::path& path)
{
    for (preview_item_t& item : folderPreviewItems)
    {
        UnloadTexture(std::get<1>(item));
    }

    folderPreviewItems.clear();

    currentDir = path;
    std::cout << "Folder changed to: " << currentDir.string() << std::endl;

    // Process each .obj file in the new directory
    for (const fs::directory_entry& entry : fs::directory_iterator(currentDir))
    {
        if (!entry.is_directory() && entry.path().extension() == ".obj")
        {
            RenderTexture2D texture = LoadRenderTexture(64, 64);
            folderPreviewItems.emplace_back(std::make_tuple(entry.path().string(), texture.texture));

            Model model = LoadModel(entry.path().string().c_str());

            BeginTextureMode(texture);
            ClearBackground(Color{ 0, 0, 0, 0 });
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
    if (itemPreviewModel.meshCount > 0)
    {
        UnloadModel(itemPreviewModel);
    }

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
