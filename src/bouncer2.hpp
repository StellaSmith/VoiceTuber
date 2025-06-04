#pragma once
#include "audio-level.hpp"
#include "node.hpp"
#include <cereal/access.hpp>

class Bouncer2 final : public Node
{
public:
  static constexpr const char *className = "Bouncer2";
  Bouncer2(Lib &, Undo &, class AudioIn &, std::string name);
  Bouncer2(class App &, std::string name);

private:
  float strength = 100.f;
  float easing = 50.f;
  AudioLevel audioLevel;

  auto render(float dt, Node *hovered, Node *selected) -> void final;
  auto renderUi() -> void final;

  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend cereal::access;

  template <typename Archive>
  auto save(Archive &) const -> void;
  template <typename Archive>
  auto load(Archive &) -> void;
};
