#include "image-list.hpp"
#include "file-open.hpp"
#include "imgui-helpers.hpp"
#include "ui.hpp"
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <fmt/std.h>
#include <spdlog/spdlog.h>

ImageList::ImageList(Lib &aLib, Undo &aUndo, const std::filesystem::path &)
  : lib(aLib),
    undo(aUndo)
{
}

auto ImageList::frame() const -> int
{
  return frame_;
}

auto ImageList::frame(int v) -> void
{
  frame_ = v;
}

auto ImageList::h() const -> float
{
  if (textures.empty())
    return 100;
  return textures.front()->h();
}

auto ImageList::isTransparent(glm::vec2 v) const -> bool
{
  if (textures.empty())
    return false;

  auto &texture = textures[frame_ % textures.size()];

  if (texture->ch() == 3)
    return false;
  const auto x = static_cast<int>(v.x * texture->w() / w());
  const auto y = static_cast<int>(v.y * texture->h() / h());
  if (x < 0 || x >= texture->w() || y < 0 || y >= texture->h())
    return true;

  if (auto imageData = texture->imageData())
    return imageData[(x + y * texture->w()) * texture->ch() + 3] < 127;
  else
    return false;
}

template <typename Archive>
auto ImageList::load(Archive &archive) -> void
{
  std::vector<std::string> texturesForSaveLoad;
  archive(
    cereal::make_nvp("textures", texturesForSaveLoad));

  textures.clear();
  for (const auto &path : texturesForSaveLoad)
    textures.emplace_back(lib.get().queryTex(path));
}

template <typename Archive>
auto ImageList::save(Archive &archive) const -> void
{
  std::vector<std::string> texturesForSaveLoad;
  for (const auto &t : textures)
    texturesForSaveLoad.emplace_back(t->path());

  archive(
    cereal::make_nvp("textures", texturesForSaveLoad));
}

template void ImageList::save(cereal::JSONOutputArchive &) const;
template void ImageList::load(cereal::JSONInputArchive &);

auto ImageList::numFrames() const -> int
{
  return static_cast<int>(textures.size());
}

auto ImageList::render() -> void
{
  if (textures.empty())
    return;

  auto &texture = textures[frame_ % textures.size()];

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture->texture());
  glBegin(GL_QUADS);
  glColor4f(1.f, 1.f, 1.f, 1.f);
  glTexCoord2f(.0f, .0f);
  glVertex2f(.0f, .0f);
  glTexCoord2f(1.f, .0f);
  glVertex2f(w(), .0f);
  glTexCoord2f(1.f, 1.f);
  glVertex2f(w(), h());
  glTexCoord2f(.0f, 1.f);
  glVertex2f(.0f, h());
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

auto ImageList::renderUi() -> void
{
  if (dialog)
    if (!dialog->draw())
      dialog = nullptr;
  ImGui::TableNextColumn();
  Ui::textRj("Images");
  ImGui::TableNextColumn();
  auto n = 0;
  auto toDel = -1;
  for (auto &texture : textures)
  {
    auto const delStr = fmt::format("X##{} {}", n, texture->path());
    if (ImGui::Button(delStr.c_str()))
      toDel = n;
    ImGui::SameLine();
    ImGui::TextF("{} {}", n++, texture->path());
  }
  if (toDel >= 0)
    undo.get().record(
      [alive = weak_self(), toDel]() {
        if (auto self = alive.lock())
        {
          self->textures.erase(std::begin(self->textures) + toDel);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      },
      [alive = weak_self(), oldTextures = textures]() {
        if (auto self = alive.lock())
        {
          self->textures = std::move(oldTextures);
        }
        else
        {
          SPDLOG_INFO("this was destroyed");
        }
      });
  if (ImGui::Button("Add##Image"))
  {
    dialog = std::make_unique<FileOpen>(lib, "Add Image Dialog", [this](bool r, const auto &path) {
      if (r)
        undo.get().record(
          [alive = weak_self(), path]() {
            if (auto self = alive.lock())
            {
              self->textures.emplace_back(self->lib.get().queryTex([&]() {
                try
                {
                  if (!std::filesystem::exists(path.filename()))
                    std::filesystem::copy(path, path.filename());
                  return path.filename().string();
                }
                catch (std::runtime_error &e)
                {
                  SPDLOG_ERROR("{:t}", e);
                  return std::string{"engine:missing.png"};
                }
              }()));
            }
            else
            {
              SPDLOG_INFO("this was destroyed");
            }
          },
          [alive = weak_self()]() {
            if (auto self = alive.lock())
            {
              self->textures.pop_back();
            }
            else
            {
              SPDLOG_INFO("this was destroyed");
            }
          });
    });
  }
}

auto ImageList::w() const -> float
{
  if (textures.empty())
    return 100;
  return textures.front()->w();
}
