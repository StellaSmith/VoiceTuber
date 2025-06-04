#pragma once

#include "chat.hpp"
#include <cereal/access.hpp>

class ChatV2 final : public Chat
{
public:
  ChatV2(class App &app, std::string name);
  ChatV2(Lib &, Undo &, uv::Uv &, class AudioSink &, std::string name);

  static constexpr const char *className = "Chat-v2";

private:
  auto do_clone() const -> std::shared_ptr<Node> final;

private:
  friend cereal::access;

  template <typename Archive>
  auto load(Archive &) -> void;
  template <typename Archive>
  auto save(Archive &) const -> void;
};
