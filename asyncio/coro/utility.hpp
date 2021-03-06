#pragma once

#include <experimental/coroutine>

#include <asyncio/common.hpp>
#include <asyncio/log.hpp>

#include "coro.hpp"

BEGIN_ASYNCIO_NAMESPACE;

template <typename ReturnType, typename CoroType = coro<ReturnType>,
          typename AllocatorType = DefaultAllocator>
class co_runner {
public:
  co_runner(CoroType &co) : _runner(run(co)) { _runner.await_suspend(nullptr); }
  co_runner(CoroType &&co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(co_runner &) = delete;
  co_runner(co_runner &&) = delete;
  std::future<ReturnType> get_future() { return std::move(_future); }

private:
  coro<void, AllocatorType> _runner;
  std::future<ReturnType> _future;

  coro<void, AllocatorType> run(CoroType &co) {
    std::promise<ReturnType> promise;
    this->_future = promise.get_future();
    try {
      ReturnType ret = co_await std::move(co);
      promise.set_value(ret);
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  }
};

template <typename CoroType, typename AllocatorType>
class co_runner<void, CoroType, AllocatorType> {
public:
  co_runner(CoroType &co) : _runner(run(co)) { _runner.await_suspend(nullptr); }
  co_runner(CoroType &&co) : _runner(run(co)) {
    _runner.await_suspend(nullptr);
  }
  co_runner(co_runner &) = delete;
  co_runner(co_runner &&) = delete;
  std::future<void> get_future() { return std::move(_future); }

private:
  coro<void, AllocatorType> _runner;
  std::future<void> _future;

  coro<void, AllocatorType> run(CoroType &co) {
    std::promise<void> promise;
    this->_future = promise.get_future();
    try {
      CoroType holder = std::move(co);
      co_await holder;
      promise.set_value();
    } catch (...) {
      promise.set_exception(std::current_exception());
    }
  }
};

class AWaitableBase {
public:
  AWaitableBase() : _ready(false), _caller(nullptr) {}

  bool await_ready() const noexcept { return _ready; }
  bool await_suspend(std::experimental::coroutine_handle<> caller) noexcept {
    if (!_ready) {
      _caller = caller;
    }
    return !_ready;
  }

  template <typename E> void raise(E e) {
    this->_exception = std::make_exception_ptr(e);
    this->setReady();
    this->resumeCaller();
  }

protected:
  void setReady() { _ready = true; }
  void resumeCaller() {
    if (_caller) {
      _caller.resume();
    }
  }
  void checkException() {
    if (this->_exception) {
      std::exception_ptr ptr = this->_exception;
      this->_exception = nullptr;
      std::rethrow_exception(ptr);
    }
  }

  bool _ready;
  std::experimental::coroutine_handle<> _caller;
  std::exception_ptr _exception;
};

template <typename T> class AWaitable : public AWaitableBase {
public:
  T await_resume() {
    this->checkException();
    return std::move(_value);
  }
  void resume(T &&value) {
    _value = std::move(value);
    this->setReady();
    this->resumeCaller();
  }
  void resume(T &value) {
    _value = value;
    this->setReady();
    this->resumeCaller();
  }

private:
  T _value;
};

template <> class AWaitable<void> : public AWaitableBase {
public:
  void await_resume() { this->checkException(); }
  void resume() {
    this->setReady();
    this->resumeCaller();
  }
};

END_ASYNCIO_NAMESPACE;