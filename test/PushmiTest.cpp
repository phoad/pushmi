// Copyright (c) 2018-present, Facebook, Inc.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "catch.hpp"

#include <type_traits>

#include <chrono>
using namespace std::literals;

#include "pushmi/flow_single_sender.h"
#include "pushmi/o/empty.h"
#include "pushmi/o/from.h"
#include "pushmi/o/just.h"
#include "pushmi/o/on.h"
#include "pushmi/o/transform.h"
#include "pushmi/o/tap.h"
#include "pushmi/o/submit.h"
#include "pushmi/o/extension_operators.h"

#include "pushmi/trampoline.h"
#include "pushmi/new_thread.h"

using namespace pushmi::aliases;


SCENARIO( "empty can be used with tap and submit", "[empty][sender]" ) {

  GIVEN( "An empty sender" ) {
    auto e = op::empty();
    using E = decltype(e);

    REQUIRE( v::SenderTo<E, v::any_receiver<>, v::is_single<>> );

    WHEN( "tap and submit are applied" ) {
      int signals = 0;
      e |
        op::tap(
          [&](){ signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; }) |
        op::submit(
          [&](){ signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; });

      THEN( "the done signal is recorded twice" ) {
        REQUIRE( signals == 20 );
      }

      WHEN( "future_from is applied" ) {
        REQUIRE_THROWS_AS(v::future_from(e).get(), std::future_error);

        THEN( "future_from(e) returns std::future<void>" ) {
          REQUIRE( std::is_same<std::future<void>, decltype(v::future_from(e))>::value );
        }
      }
    }
  }

  GIVEN( "An empty int single_sender" ) {
    auto e = op::empty<int>();
    using E = decltype(e);

    REQUIRE( v::SenderTo<E, v::any_receiver<std::exception_ptr, int>, v::is_single<>> );

    WHEN( "tap and submit are applied" ) {

      int signals = 0;
      e |
        op::tap(
          [&](auto v){ signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; }) |
        op::submit(
          [&](auto v){ signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; });

      THEN( "the done signal is recorded twice" ) {
        REQUIRE( signals == 20 );
      }
    }
  }
}

SCENARIO( "just() can be used with transform and submit", "[just][sender]" ) {

  GIVEN( "A just int single_sender" ) {
    auto j = op::just(20);
    using J = decltype(j);

    REQUIRE( v::SenderTo<J, v::any_receiver<std::exception_ptr, int>, v::is_single<>> );

    WHEN( "transform and submit are applied" ) {
      int signals = 0;
      int value = 0;
      j |
        op::transform(
          [&](int v){ signals += 10000; return v + 1; },
          [&](auto v){ std:abort(); return v; }) |
        op::transform(
          [&](int v){ signals += 10000; return v * 2; }) |
        op::submit(
          [&](auto v){ value = v; signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; });

      THEN( "the transform signal is recorded twice, the value and done signals once and the result is correct" ) {
        REQUIRE( signals == 20110 );
        REQUIRE( value == 42 );
      }
    }

    WHEN( "future_from<int> is applied" ) {
      auto twenty = v::future_from<int>(j).get();

      THEN( "the value signal is recorded once and the result is correct" ) {
        REQUIRE( twenty == 20 );
        REQUIRE( std::is_same<std::future<int>, decltype(v::future_from<int>(j))>::value );
      }
    }
  }
}

SCENARIO( "from() can be used with transform and submit", "[from][sender]" ) {

  GIVEN( "A from int many_sender" ) {
    int arr[] = {0, 9, 99};
    auto m = op::from(arr);
    using M = decltype(m);

    REQUIRE( v::SenderTo<M, v::any_receiver<std::exception_ptr, int>, v::is_many<>> );

    WHEN( "transform and submit are applied" ) {
      int signals = 0;
      int value = 0;
      m |
        op::transform(
          [&](int v){ signals += 10000; return v + 1; },
          [&](auto v){ std:abort(); return v; }) |
        op::transform(
          [&](int v){ signals += 10000; return v * 2; }) |
        op::submit(
          [&](auto v){ value += v; signals += 100; },
          [&](auto e) noexcept { signals += 1000; },
          [&](){ signals += 10; });

      THEN( "the transform signal is recorded twice, the value signal once and the result is correct" ) {
        REQUIRE( signals == 60310 );
        REQUIRE( value == 222 );
      }
    }

  }
}
