#include "prj-dialog.hpp"
#include <functional>
#include <imgui/imgui.h>

auto PrjDialog::draw() -> bool
{
  ImGui::Begin("New/Open Project");

  ImVec2 availableSpace = ImGui::GetContentRegionAvail();

  // Adjust the width of the ListBox
  ImVec2 listBoxSize(availableSpace.x, availableSpace.y - 25); // Adjust the width and height as needed
  if (dirs.empty())
  {
    const auto cwd = std::filesystem::current_path();
    for (auto &entry : std::filesystem::directory_iterator(cwd))
      if (entry.is_directory())
        dirs.push_back(entry.path());
    std::sort(std::begin(dirs), std::end(dirs));
  }

  auto ret = false;
  auto hasSelected = false;
  if (ImGui::BeginListBox("##dirs", listBoxSize))
  {
    std::function<void()> postponedAction = nullptr;
    if (ImGui::Selectable("..", ".." == selectedDir, ImGuiSelectableFlags_AllowDoubleClick))
      if (ImGui::IsMouseDoubleClicked(0))
        postponedAction = [this]() {
          dirs.clear();
          selectedDir = "";
          const auto cwd = std::filesystem::current_path();
          std::filesystem::current_path(cwd.parent_path());
        };

    for (auto &dir : dirs)
    {
      if (selectedDir == dir.filename())
        hasSelected = true;
      if (ImGui::Selectable(("> " + dir.filename().string()).c_str(),
                            selectedDir == dir.filename(),
                            ImGuiSelectableFlags_AllowDoubleClick))
      {
        if (ImGui::IsMouseDoubleClicked(0))
          postponedAction = [this, dir, &ret]() {
            std::filesystem::current_path(dir);
            dirs.clear();
            selectedDir = "";
            const auto projectFilePath = std::filesystem::path(selectedDir) / "prj.tpp";
            if (std::filesystem::exists(projectFilePath))
              ret = true;
          };
        else
          selectedDir = dir.filename().string();
      }
    }
    if (postponedAction)
      postponedAction();
    ImGui::EndListBox();
  }

  const auto btnWidth = 100.f;
  ImGui::PushItemWidth(availableSpace.x - btnWidth);
  char buf[1024];
  strcpy(buf, selectedDir.data());
  ImGui::InputText("##dirname", buf, sizeof(buf));
  selectedDir = buf;
  ImGui::PopItemWidth();

  ImGui::SameLine();
  if (hasSelected)
  {
    if (ImGui::Button("Open", ImVec2{btnWidth, 0}))
    {
      std::filesystem::current_path(selectedDir);
      dirs.clear();
      selectedDir = "";
      const auto projectFilePath = std::filesystem::path(selectedDir) / "prj.tpp";
      if (std::filesystem::exists(projectFilePath))
        ret = true;
    }
  }
  else
  {
    ImGui::BeginDisabled(selectedDir.empty());
    if (ImGui::Button("New", ImVec2{btnWidth, 0}))
    {
      if (std::filesystem::create_directories(selectedDir))
      {
        std::filesystem::current_path(selectedDir);
        dirs.clear();
        selectedDir = "";
        ret = true;
      }
    }
    ImGui::EndDisabled();
  }
  ImGui::End();
  return ret;
}
