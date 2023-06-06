#define __debug_all__
#include <iostream>
#include <coroutine>
#include <concepts>

struct Task {
	struct promise_type {
		Task get_return_object() { return { .h_ = std::coroutine_handle<promise_type>::from_promise(*this) }; }
		std::suspend_always initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void unhandled_exception() {}
		void return_void() {}
	};

	std::coroutine_handle<promise_type> h_;

	operator std::coroutine_handle<promise_type>() const { return h_; }
	operator std::coroutine_handle<>() const { return h_; }
};

template<typename T>
struct Generator {
	struct promise_type {
		T value_;
		Generator get_return_object() { return { .h_ = std::coroutine_handle<promise_type>::from_promise(*this) }; }
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void unhandled_exception() {}
		std::suspend_always yield_value(T value) {
			value_ = value;
			return {};
		}
		void return_void() {}
	};

	std::coroutine_handle<promise_type> h_;

	operator std::coroutine_handle<promise_type>() const { return h_; }
	operator std::coroutine_handle<>() const { return h_; }
};


Task teste() {
	for (int i = 0;; i++) {
		std::cout << "counter" << i << std::endl;
		co_await std::suspend_always{};
	}
}

Task testex() {
	for (int i = 0;; i++) {
		std::cout << "counter" << i << std::endl;
		co_await std::suspend_always{};
	}
}

Generator<int> teste2() {
	for (int i = 0;; i++) {
		co_yield i;
	}
}

int main(int argc, char *argv[]) {
	std::coroutine_handle<Task::promise_type> h = teste();
	h();
	h.destroy();

	auto h2 = teste2().h_;
	auto& promise = h2.promise();
	for (int i = 0;; i++) {
		
		std::cout << "counter" << promise.value_ << std::endl;
		h2();
	}
}