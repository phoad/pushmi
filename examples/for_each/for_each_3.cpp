// Copyright (c) 2018-present, Facebook, Inc.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>

#include <for_each.h>

using namespace pushmi::aliases;

auto inline_bulk_target() {
  return [](
      auto init,
      auto selector,
      auto input,
      auto&& func,
      auto sb,
      auto se,
      auto out) {
        try {
          auto acc = init(input);
          for (decltype(sb) idx{sb}; idx != se; ++idx){
              func(acc, idx);
          }
          auto result = selector(std::move(acc));
          mi::set_value(out, std::move(result));
        } catch(...) {
          mi::set_error(out, std::current_exception());
        }
      };
}

int main()
{
  std::vector<int> vec(10);

  mi::for_each(inline_bulk_target(), vec.begin(), vec.end(), [](int& x){
    x = 42;
  });

  assert(std::count(vec.begin(), vec.end(), 42) == static_cast<int>(vec.size()));

  std::cout << "OK" << std::endl;
}
