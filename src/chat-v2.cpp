#include "chat-v2.hpp"
#include "app.hpp"
#include <cereal/details/helpers.hpp>
#include <cereal/types/base_class.hpp>
#include <utility>

ChatV2::ChatV2(App &app, std::string aName)
  : ChatV2(app.getLib(), app.getUndo(), app.getUv(), app.getAudioOut(), std::move(aName))
{
}

ChatV2::ChatV2(Lib &lib, Undo &aUndo, uv::Uv &uv, AudioSink &audioSink, std::string aName)
  : Chat(lib, aUndo, uv, audioSink, std::move(aName))
{
}

template <typename Archive>
auto ChatV2::load(Archive &archive) -> void
{
  archive(
    cereal::virtual_base_class<Chat>(this),
    cereal::make_nvp("hide_chat", this->hideChatSec));
}

template <typename Archive>
auto ChatV2::save(Archive &archive) const -> void
{
  archive(
    cereal::virtual_base_class<Chat>(this),
    cereal::make_nvp("hide_chat", this->hideChatSec));
}

auto ChatV2::do_clone() const -> std::shared_ptr<Node>
{
  return std::make_shared<ChatV2>(*this);
}
