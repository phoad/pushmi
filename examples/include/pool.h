#pragma once

// Copyright (c) 2018-present, Facebook, Inc.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <experimental/thread_pool>

#include <pushmi/executor.h>
#include <pushmi/trampoline.h>
#include <pushmi/time_single_sender.h>

#if __cpp_deduction_guides >= 201703
#define MAKE(x) x MAKE_
#define MAKE_(...) {__VA_ARGS__}
#else
#define MAKE(x) make_ ## x
#endif

namespace pushmi {

using std::experimental::static_thread_pool;
namespace execution = std::experimental::execution;

template<class Executor>
struct pool_executor {
  using properties = property_set<is_sender<>, is_executor<>, is_never_blocking<>, is_concurrent_sequence<>, is_single<>>;

  using e_t = Executor;
  e_t e;
  explicit pool_executor(e_t e) : e(std::move(e)) {}
  auto executor() { return *this; }
  PUSHMI_TEMPLATE(class Out)
    (requires Receiver<Out>)
  void submit(Out out) const {
    e.execute([e = *this, out = std::move(out)]() mutable {
      ::pushmi::set_value(out, e);
    });
  }
};

class pool {
  static_thread_pool p;
public:

  inline explicit pool(std::size_t threads) : p(threads) {}

  inline auto executor() {
    auto exec = execution::require(p.executor(), execution::never_blocking, execution::oneway);
    return pool_executor<decltype(exec)>{exec};
  }

  inline void stop() {p.stop();}
  inline void wait() {p.wait();}
};

} // namespace pushmi
