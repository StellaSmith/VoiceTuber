#include "file-open.hpp"
#include <functional>
#include <imgui/imgui.h>
#include <log/log.hpp>

#ifdef _WIN32
#include <Windows.h>
#endif

FileOpen::FileOpen(Lib &lib, std::string dialogName, Cb aCb)
  : Dialog(std::move(dialogName),
           [this, aCb = std::move(aCb)](bool r) { aCb(r, getSelectedFile()); }),
    cwd(std::filesystem::current_path()),
    upDir(lib.queryTex("engine:up-dir.png", true))
{
}

auto FileOpen::internalDraw() -> DialogState
{
  const auto sz = ImGui::GetFontSize();
  if (ImGui::ImageButton((void *)(intptr_t)upDir->texture(), ImVec2(sz, sz)))
  {
    files.clear();
    selectedFile = "";
#ifdef _WIN32
    if (cwd != cwd.parent_path())
      cwd = cwd.parent_path();
    else
      cwd = "";
#else
    cwd = cwd.parent_path();
#endif
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Go Up");
  ImGui::SameLine();
  ImGui::Text("%s", cwd.string().c_str());

  if (files.empty())
  {
    if (!cwd.string().empty())
    {
      try
      {
        for (auto &entry : std::filesystem::directory_iterator(cwd))
          files.push_back(entry.path());
      }
      catch (std::runtime_error&e)
      {
        LOG(e.what());
      }
      std::sort(files.begin(), files.end());
    }
    else
    {
#ifdef _WIN32
      char d = 'A';
      for (auto drives = GetLogicalDrives(); drives != 0; drives >>= 1, ++d)
      {
        if (drives & 0x1)
        {
          auto drive = d + std::string{":\\"};
          files.push_back(drive);
        }
      }
#endif
    }
  }
  // Populate the files in the list box
  if (ImGui::BeginListBox("##files", ImVec2(700, 400)))
  {
    std::function<void()> postponedAction = nullptr;
    for (auto &file : files)
    {
      try
      {
        const auto fileStr = [&file]() {
          if (file.filename().string().empty())
            return file.string();
          else
            return file.filename().string();
        }();
        if (fileStr.empty())
          continue;
        const auto isHidden = fileStr.front() == '.';
        if (isHidden)
          continue;
        const auto isDirectory = std::filesystem::is_directory(file);

        if (ImGui::Selectable(((isDirectory ? "> " : "  ") + fileStr).c_str(),
                              selectedFile == fileStr,
                              ImGuiSelectableFlags_AllowDoubleClick))
        {
          if (ImGui::IsMouseDoubleClicked(0))
          {
            if (isDirectory)
            {
              postponedAction = [this, file]() {
                cwd = cwd / file;
                files.clear();
                selectedFile = "";
              };
            }
            else
            {
              selectedFile = fileStr;
              ImGui::EndListBox();
              return DialogState::ok;
            }
          }
          else
          {
            selectedFile = fileStr;
          }
        }
      }
      catch (std::runtime_error &e)
      {
        LOG(e.what());
      }
    }
    if (postponedAction)
      postponedAction();
    ImGui::EndListBox();
  }

  // Show the selected file
  if (!selectedFile.empty())
    ImGui::Text("%s", selectedFile.c_str());
  else
    ImGui::Text("No file selected");

  const auto BtnSz = 90;
  ImGui::SameLine(700 - 2 * BtnSz - 10);
  ImGui::BeginDisabled(selectedFile.empty());
  if (ImGui::Button("Open", ImVec2(BtnSz, 0)))
  {
    if (!std::filesystem::is_directory(cwd / selectedFile))
    {
      ImGui::EndDisabled();

      return DialogState::ok;
    }
    else
    {
      cwd = cwd / selectedFile;
      files.clear();
      selectedFile = "";
    }
  }
  ImGui::EndDisabled();
  ImGui::SetItemDefaultFocus();
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(BtnSz, 0)))
    return DialogState::cancel;
  return DialogState::active;
}

auto FileOpen::getSelectedFile() const -> std::filesystem::path
{
  return cwd / selectedFile;
}
