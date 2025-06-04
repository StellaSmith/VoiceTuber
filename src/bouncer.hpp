#pragma once

#include "audio-level.hpp"
#include "node.hpp"
#include <cereal/access.hpp>
#include <imgui.h>

class Bouncer final : public Node
{
  static constexpr const char *className = "Bouncer";
  Bouncer(Lib &, Undo &, class AudioIn &);
  Bouncer(class App &);

private:
  float strength = 100.f;
  ImVec4 clearColor = ImVec4(123.f / 256.f, 164.f / 256.f, 119.f / 256.f, 1.00f);
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
