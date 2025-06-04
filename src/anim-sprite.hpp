#pragma once
#include "node.hpp"
#include "sprite-sheet.hpp"
#include <cereal/access.hpp>
#include <chrono>

class AnimSprite : public Node
{
public:
  AnimSprite(Lib &, Undo &, const std::filesystem::path &path);
  AnimSprite(class App &app, const std::filesystem::path &path);
  static constexpr const char *className = "AnimSprite";

protected:
  auto render(float dt, Node *hovered, Node *selected) -> void override;
  auto renderUi() -> void override;

protected:
  SpriteSheet sprite;

private:
  friend cereal::access;

  template <typename Archive>
  auto save(Archive &) const -> void;
  template <typename Archive>
  auto load(Archive &) -> void;
  template <typename Archive>
  static auto load_and_construct(Archive &archive, cereal::construct<AnimSprite> &construct) -> void;

private:
  float fps = 30.f;
  bool physics = false;
  glm::vec2 end = {0.f, 0.f};
  float force = 200.f;
  float damping = 1.f;
  float springiness = 2.f;
  std::chrono::high_resolution_clock::time_point startTime;
  float animRotV = 0.f;
  glm::vec2 lastProjPivot = {0.f, 0.f};
  glm::vec2 lastProjPivotV = {0.f, 0.f};
  std::shared_ptr<const Texture> arrowN;
  std::shared_ptr<const Texture> arrowNE;
  std::shared_ptr<const Texture> arrowE;
  std::shared_ptr<const Texture> arrowSE;
  std::shared_ptr<const Texture> arrowS;
  std::shared_ptr<const Texture> arrowSW;
  std::shared_ptr<const Texture> arrowW;
  std::shared_ptr<const Texture> arrowNW;
  std::shared_ptr<const Texture> center;

  auto h() const -> float final;
  auto isTransparent(glm::vec2) const -> bool final;
  auto w() const -> float final;
  auto do_clone() const -> std::shared_ptr<Node>;
};
