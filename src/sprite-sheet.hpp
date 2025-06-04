#pragma once
#include "lib.hpp"
#include "node.hpp"
#include <SDL_opengl.h>
#include <cereal/access.hpp>
#include <filesystem>
#include <sdlpp/sdlpp.hpp>

class SpriteSheet
{
public:
  SpriteSheet(Lib &, Undo &, const std::filesystem::path &path);
  SpriteSheet(class App &app, const std::filesystem::path &path);

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
  std::reference_wrapper<Undo> undo;
  int cols = 1;
  int rows = 1;
  int frame_ = 0;
  int numFrames_ = 1;
  std::shared_ptr<const Texture> texture;
};
