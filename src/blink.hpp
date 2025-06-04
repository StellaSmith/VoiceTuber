#pragma once
#include "image-list.hpp"
#include "node.hpp"
#include "sprite-sheet.hpp"
#include <cereal/access.hpp>
#include <chrono>
#include <filesystem>

template <typename S, typename ClassName>
class Blink final : public Node
{
public:
  Blink(class App &app, const std::filesystem::path &path);
  Blink(Lib &, Undo &, const std::filesystem::path &);

  constexpr static const char *className = ClassName::v;

private:
  enum class State {
    open,
    closed,
  };

  S sprite;
  int openEyes = 0;
  int closedEyes = 1;
  float blinkEvery = 3.5f;
  float blinkDuration = .25f;
  State state = State::open;
  std::chrono::high_resolution_clock::time_point nextEventTime;

  auto h() const -> float final;
  auto isTransparent(glm::vec2) const -> bool final;

  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;

  auto w() const -> float final;
  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend cereal::access;

  template <typename Archive>
  auto load(Archive &) -> void;
  template <typename Archive>
  auto save(Archive &) const -> void;
  template <typename Archive>
  static auto load_and_construct(Archive &archive, cereal::construct<Blink> &construct) -> void;
};

struct SpriteSheetBlinkClassName
{
  constexpr static const char *v = "SpriteSheetBlink";
};

struct ImageListBlinkClassName
{
  constexpr static const char *v = "ImageListBlink";
};

extern template class Blink<SpriteSheet, SpriteSheetBlinkClassName>;
extern template class Blink<ImageList, ImageListBlinkClassName>;

using SpriteSheetBlink = Blink<SpriteSheet, SpriteSheetBlinkClassName>;
using ImageListBlink = Blink<ImageList, ImageListBlinkClassName>;
