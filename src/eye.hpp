#pragma once
#include "anim-sprite.hpp"
#include "mouse-sink.hpp"
#include <cereal/access.hpp>

class Eye final : public AnimSprite, public MouseSink
{
public:
  Eye(class MouseTracking &, Lib &, Undo &, const std::filesystem::path &);
  ~Eye() final;

  static constexpr const char *className = "Eye";

private:
  float radius = 20.f;
  float followStrength = 4.f;
  glm::vec2 mouse;
  std::reference_wrapper<MouseTracking> mouseTracking;

  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;
  auto ingest(const glm::mat4 &projMat, glm::vec2 v) -> void final;
  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend class cereal::access;

  template <typename Archive>
  auto save(Archive &archive) const -> void;
  template <typename Archive>
  auto load(Archive &archive) -> void;
};
