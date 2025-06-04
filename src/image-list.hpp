#pragma once
#include "dialog.hpp"
#include "lib.hpp"
#include "node.hpp"
#include "undo.hpp"
#include <cereal/access.hpp>
#include <filesystem>

class ImageList : public virtual enable_shared_from_this
{
public:
  ImageList(Lib &, Undo &, const std::filesystem::path &path);
  auto frame() const -> int;
  auto frame(int) -> void;
  auto h() const -> float;
  auto isTransparent(glm::vec2) const -> bool;

  auto numFrames() const -> int;
  auto render() -> void;
  auto renderUi() -> void;

  auto w() const -> float;

private:
  friend cereal::access;

  template <typename Archive>
  auto load(Archive &) -> void;
  template <typename Archive>
  auto save(Archive &) const -> void;

private:
  std::reference_wrapper<Lib> lib;
  std::reference_wrapper<Undo> undo;
  int frame_ = 0;
  std::vector<std::shared_ptr<const Texture>> textures;
  std::shared_ptr<Dialog> dialog = nullptr;
};
