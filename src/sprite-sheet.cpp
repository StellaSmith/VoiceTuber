#include "sprite-sheet.hpp"
#include "app.hpp"
#include "ui.hpp"
#include <cassert>
#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <filesystem>
#include <fmt/std.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

SpriteSheet::SpriteSheet(App &app, const std::filesystem::path &path)
  : SpriteSheet(app.getLib(), app.getUndo(), path)
{
}

SpriteSheet::SpriteSheet(Lib &lib, Undo &aUndo, const std::filesystem::path &path)
  : undo(aUndo), texture(lib.queryTex([&]() {
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
    }()))
{
}

auto SpriteSheet::render() -> void
{
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, texture->texture());
  glBegin(GL_QUADS);
  const auto fCols = static_cast<float>(cols);
  const auto fRows = static_cast<float>(rows);
  const auto i = frame_ % cols / fCols;
  const auto j = (fRows - 1.f - frame_ / cols) / fRows;
  glColor4f(1.f, 1.f, 1.f, 1.f);
  glTexCoord2f(.0f + i, .0f + j);
  glVertex2f(.0f, .0f);
  glTexCoord2f(1.f / fCols + i, .0f + j);
  glVertex2f(w(), .0f);
  glTexCoord2f(1.f / fCols + i, 1.f / fRows + j);
  glVertex2f(w(), h());
  glTexCoord2f(.0f + i, 1.f / fRows + j);
  glVertex2f(.0f, h());
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_TEXTURE_2D);
}

auto SpriteSheet::renderUi() -> void
{
  ImGui::TableNextColumn();
  Ui::textRj("Cols");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##Cols", cols);
  if (cols < 1)
    cols = 1;
  ImGui::TableNextColumn();
  Ui::textRj("Rows");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##Rows", rows);
  if (rows < 1)
    rows = 1;
  ImGui::TableNextColumn();
  Ui::textRj("NumFrames");
  ImGui::TableNextColumn();
  Ui::inputInt(undo, "##NumFrames", numFrames_);
  if (numFrames_ < 1)
    numFrames_ = 1;
}

#if 0
#define SER_PROP_LIST         \
  SER_PROP(cols);             \
  SER_PROP(rows);             \
  SER_PROP(depricatedFrame_); \
  SER_PROP(numFrames_);
  SER_DEF_PROPS()
#undef SER_PROP_LIST
#endif

template <typename Archive>
auto SpriteSheet::save(Archive &archive) const -> void
{
  archive(
    cereal::make_nvp("columns", this->cols),
    cereal::make_nvp("rows", this->rows),
    cereal::make_nvp("frames", this->numFrames_));
}

template <typename Archive>
auto SpriteSheet::load(Archive &archive) -> void
{
  archive(
    cereal::make_nvp("columns", this->cols),
    cereal::make_nvp("rows", this->rows),
    cereal::make_nvp("frames", this->numFrames_));
}

template void SpriteSheet::save(cereal::JSONOutputArchive &) const;
template void SpriteSheet::load(cereal::JSONInputArchive &);

auto SpriteSheet::w() const -> float
{
  return 1.f * texture->w() / cols;
}

auto SpriteSheet::h() const -> float
{
  return 1.f * texture->h() / rows;
}

auto SpriteSheet::isTransparent(glm::vec2 v) const -> bool
{
  if (texture->ch() == 3)
    return false;
  const auto x = static_cast<int>(v.x + (frame_ % numFrames_) % cols * w());
  const auto y = static_cast<int>(v.y + (rows - (frame_ % numFrames_) / cols - 1) * h());
  if (x < 0 || x >= texture->w() || y < 0 || y >= texture->h())
    return true;

  if (auto imageData = texture->imageData())
    return imageData[(x + y * texture->w()) * texture->ch() + 3] < 127;
  else
    return false;
}

auto SpriteSheet::frame(int v) -> void
{
  frame_ = v;
}

auto SpriteSheet::frame() const -> int
{
  return frame_;
}

auto SpriteSheet::numFrames() const -> int
{
  return numFrames_;
}
