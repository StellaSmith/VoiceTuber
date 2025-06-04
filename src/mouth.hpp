#pragma once
#include "image-list.hpp"
#include "node.hpp"
#include "sprite-sheet.hpp"
#include "visemes-sink.hpp"
#include <cereal/access.hpp>
#include <chrono>
#include <filesystem>

template <typename S, typename ClassName>
class Mouth final : public Node, public VisemesSink
{
public:
  Mouth(class App &app, const std::filesystem::path &path);
  Mouth(class Wav2Visemes &, Lib &, Undo &, const std::filesystem::path &);

  ~Mouth() final;

  constexpr static const char *className = ClassName::v;

private:
  S sprite;
  std::map<Viseme, int> viseme2Sprite;
  Viseme viseme = Viseme{};
  std::chrono::high_resolution_clock::time_point freezeTime;
  std::reference_wrapper<Wav2Visemes> wav2Visemes;

  auto h() const -> float final;
  auto ingest(Viseme) -> void final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto w() const -> float final;
  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend cereal::access;

  template <typename Archive>
  auto save(Archive &archive) const -> void;
  template <typename Archive>
  auto load(Archive &archive) -> void;
};

struct SpriteSheetMouthClassName
{
  constexpr static const char *v = "Mouth";
};

struct ImageListMouthClassName
{
  constexpr static const char *v = "ImageListMouth";
};

extern template class Mouth<SpriteSheet, SpriteSheetMouthClassName>;
extern template class Mouth<ImageList, ImageListMouthClassName>;

using SpriteSheetMouth = Mouth<SpriteSheet, SpriteSheetMouthClassName>;
using ImageListMouth = Mouth<ImageList, ImageListMouthClassName>;
