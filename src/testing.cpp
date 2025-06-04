#include "testing.hpp"

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <memory>
#include <sched.h>
#include <sys/wait.h>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <variant>

Test *Test::head = nullptr;

Test::Test(const char *name)
  : name(name), next(std::exchange(head, this))
{
}

struct Exited
{
  int exit_code;
};

struct Signaled
{
  int signal;
};

static std::variant<Exited, Signaled> wait_process(pid_t pid)
{
  int status;
  for (;;)
  {
    if (waitpid(pid, &status, 0) == -1)
    {
      int const error = errno;
      if (error == EINTR)
        continue;
      throw std::system_error{error, std::system_category(), "waitpid() failed"};
    }
    if (WIFEXITED(status))
      return Exited{WEXITSTATUS(status)};
    else // if (WIFSIGNALED(status))
      return Signaled{WTERMSIG(status)};
  }
}

static std::string_view signal_name(int signal)
{
  if (char const *ptr = sigabbrev_np(signal))
    return ptr;
  else
    throw std::invalid_argument{"invalid signal number"};
}

#include <fmt/color.h>

struct FileDescriptor
{
public:
  using native_handle_type = int;
  static constexpr native_handle_type invalid_handle = -1;

  constexpr FileDescriptor() noexcept = default;
  constexpr explicit FileDescriptor(native_handle_type handle) noexcept
    : m_handle(handle)
  {
  }

  void close()
  {
    if (m_handle != invalid_handle)
      ::close(std::exchange(m_handle, invalid_handle));
  }

  ~FileDescriptor()
  {
    try
    {
      close();
    }
    catch (...)
    {
    }
  }

  constexpr native_handle_type native_handle() const noexcept
  {
    return m_handle;
  }

private:
  native_handle_type m_handle = invalid_handle;
};

static std::size_t write_some(FileDescriptor &fd, void const *data, std::size_t size)
{
  for (;;)
  {
    ssize_t const written = ::write(fd.native_handle(), data, size);
    if (written == -1)
    {
      int const error = errno;
      if (error == EINTR)
        continue;
      throw std::system_error{error, std::system_category(), "write() failed"};
    }
    return written;
  }
}

static void write_all(FileDescriptor &fd, void const *data, std::size_t size)
{
  while (size != 0)
    size -= write_some(fd, data, size);
}

static std::size_t read_some(FileDescriptor &fd, void *data, std::size_t size)
{
  for (;;)
  {
    ssize_t const readed = ::read(fd.native_handle(), data, size);
    if (readed == -1)
    {
      int const error = errno;
      if (error == EINTR)
        continue;
      throw std::system_error{error, std::system_category(), "read() failed"};
    }
    return readed;
  }
}

[[maybe_unused]]
static void read_all(FileDescriptor &fd, void *data, std::size_t size)
{
  while (size != 0)
  {
    auto const readed = read_some(fd, data, size);
    if (readed == 0)
      throw std::runtime_error{"early EOF"};
    data = static_cast<char *>(data) + readed;
    size -= readed;
  }
}

#include <fcntl.h>
std::pair<FileDescriptor, FileDescriptor> create_pipe()
{
  int fds[2];
  if (::pipe2(fds, O_CLOEXEC) == -1)
  {
    int const error = errno;
    throw std::system_error{error, std::system_category(), "pipe2() failed"};
  }

  return {
    FileDescriptor{fds[0]},
    FileDescriptor{fds[1]},
  };
}

#include <memory>

template <typename F>
static auto fork_invoke(F &&func)
{
  using invocation_result = std::remove_cv_t<std::invoke_result_t<F>>;
  using result_type = std::variant<std::conditional_t<std::is_void_v<invocation_result>, std::monostate, invocation_result>, Exited, Signaled>;
  static_assert(std::is_void_v<invocation_result> || std::is_trivially_copyable_v<invocation_result>,
                "The return type of the function must be void or trivially copyable");

  auto [read_fd, write_fd] = create_pipe();
  pid_t pid;
  switch (pid = fork())
  {
  case -1: {
    int const error = errno;
    throw std::system_error{error, std::system_category(), "fork() failed"};
  }
  case 0:
    try
    {
      read_fd.close();
      if constexpr (std::is_void_v<invocation_result>)
      {
        std::invoke(std::forward<F>(func));
        auto const result = '\0';
        write_all(write_fd, (void const *)std::addressof(result), sizeof(result));
      }
      else
      {
        auto const result = std::invoke(std::forward<F>(func));
        write_all(write_fd, (void const *)std::addressof(result), sizeof(result));
      }
      write_fd.close();
      std::exit(0);
    }
    catch (...)
    {
      std::terminate();
    }
  default: {
    write_fd.close();
    auto const status = wait_process(pid);
    if (auto const *exited = std::get_if<Exited>(&status))
    {
      if (exited->exit_code == 0)
      {
        if constexpr (!std::is_void_v<invocation_result>)
        {
          alignas(invocation_result) char buffer[sizeof(invocation_result)];
          read_all(read_fd, buffer, sizeof(invocation_result));
          return result_type{std::in_place_index<0>, std::move(*reinterpret_cast<invocation_result *>(+buffer))};
        }
        else
        {
          alignas(char) char buffer[1];
          read_all(read_fd, buffer, sizeof(buffer));
          return result_type{std::in_place_index<0>};
        }
      }
      else
      {
        return result_type{std::in_place_index<1>, *exited};
      }
    }
    else
    {
      auto const &signaled = *std::get_if<Signaled>(&status);
      return result_type{std::in_place_index<2>, signaled};
    }
  }
  }
}

static auto invoke_test(Test &test)
{
  return fork_invoke([&test]() { return test.invoke(); });
}

int testing_main(int, char **)
{

  // Run all tests
  for (Test *test = Test::head; test; test = test->next)
  {
    fmt::print("{:?}: ", fmt::styled(test->name, fmt::emphasis::bold));
    std::fflush(nullptr);
    auto const status = invoke_test(*test);

    if (std::get_if<0>(&status))
      fmt::print("{}\n", fmt::styled("succeeded", fmt::fg(fmt::color::green)));
    else if (auto const *exited = std::get_if<1>(&status))
      fmt::print("{} with exit code {}\n", fmt::styled("failed", fmt::fg(fmt::color::red)), exited->exit_code);
    else
    {
      auto const &signaled = *std::get_if<2>(&status);
      fmt::print("{} with signal SIG{}\n", fmt::styled("failed", fmt::fg(fmt::color::red)), signal_name(signaled.signal));
    }
  }
  return 0;
}

TEST("throws")
{
  throw std::runtime_error{"This test is supposed to fail"};
}

TEST("aborts")
{
  std::fprintf(stderr, "This test is supposed to fail\n");
  std::abort();
}

TEST("succeeds")
{
  std::printf("This test is supposed to succeed\n");
}

TEST("exists zero")
{
  std::exit(0);
}

TEST("exists non-zero")
{
  std::exit(1);
}
