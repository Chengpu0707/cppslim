#pragma once
#include "SlimFixture.h"

// ---------------------------------------------------------------------------
// Fixture registration macros
//
// Usage in a fixture .cpp file:
//
//   SLIM_FIXTURE(MyClass)
//       SLIM_METHOD(methodA)
//       SLIM_METHOD(methodB)
//   SLIM_END
//
// Usage in Fixtures.cpp:
//
//   void AddFixtures(StatementExecutor* executor) {
//       SLIM_INCLUDE_FIXTURE(MyClass)
//       ...
//   }
// ---------------------------------------------------------------------------

// Open a registration function for fixture class T.
#define SLIM_FIXTURE(T)                                                         \
    void T##_Register(StatementExecutor* executor) {                           \
        using _FixtureType = T;                                                 \
        static const char* const _fixtureName = #T;                            \
        executor->registerFixture(_fixtureName,                                 \
            &SlimFixture<T>::create, &SlimFixture<T>::destroy);

// Register one method name → BoundMethod adapter inside a SLIM_FIXTURE block.
#define SLIM_METHOD(method)                                                     \
    executor->registerMethod(_fixtureName, #method,                            \
        &BoundMethod<_FixtureType, &_FixtureType::method>::call);

// Close the SLIM_FIXTURE block.
#define SLIM_END \
    }

// Add a fixture class to the global registry inside AddFixtures().
#define SLIM_INCLUDE_FIXTURE(T)                                                 \
    do {                                                                        \
        void T##_Register(StatementExecutor*);                                 \
        executor->addFixture(T##_Register);                                     \
    } while (0)
