#pragma once 

#include <iostream>
#include <variant>
#include <coroutine>
#include <type_traits>
#include <chrono>
#include <functional>

namespace jmvm::tasks
{
    /// <summary>
    /// General task template struct.
    /// </summary>
    template <typename T = void>
    struct task {
        struct promise_type;

        using coro = std::coroutine_handle<promise_type>;

        /// <summary>
        /// Promise type for void tasks
        /// </summary>
        struct void_promise_type {
            void return_void() const
            {
                if (std::holds_alternative<std::exception_ptr>(val_))
                {
                    throw std::get<std::exception_ptr>(val_);
                }
            }

            void unhandled_exception() noexcept
            {
                val_ = std::current_exception();
            }

            std::variant<std::monostate, std::exception_ptr> val_;
        };

        /// <summary>
        /// Promise type for non-void tasks
        /// </summary>
        template <typename U>
        struct type_promise_type {
            U&& value()
            {
                if (std::holds_alternative<U>(val_))
                {
                    return std::move(std::get<U>(val_));
                }
                if (std::holds_alternative<std::exception_ptr>(val_))
                {
                    throw std::get<std::exception_ptr>(val_);
                }
                throw std::exception("not return value");

            }

            void return_value(U&& val)
            {
                val_ = std::move(val);
            }

            void unhandled_exception() noexcept
            {
                val_ = std::current_exception();
            }

            std::variant<std::monostate, U, std::exception_ptr> val_;
        };

        /// <summary>
        /// Promise type template. Uses void promise type or type promise type depending on task template type T
        /// </summary>
        struct promise_type : std::conditional_t<std::is_same_v<T, void>, void_promise_type, type_promise_type<T>> {
            auto get_return_object()
            {
                return task{ coro::from_promise(*this), *this };
            }

            std::suspend_always initial_suspend() const noexcept
            {
                return {};
            }

            std::suspend_always final_suspend() const noexcept
            {
                return {};
            }

            task<T>* task_ptr_{ nullptr };
        };

        /// <summary>
        /// Runs coroutine once and returns true if it is done
        /// </summary>
        bool resume()
        {
            if (inner_task_ != nullptr)
            {
                if (inner_task_())
                {
                    inner_task_ = nullptr;
                    return handle_.done();
                }
                else
                {
                    return false;
                }
            }
            else if (!handle_.done())
            {
                handle_.resume();
                return handle_.done();
            }
            else
            {
                return true;
            }
        }

        bool await_ready()
        {
            return handle_.done();
        }

        void await_suspend(auto h)
        {
            h.promise().task_ptr_->inner_task_ = [t = handle_.promise().task_ptr_]()
            {
                return t->resume();
            };
        }

        /// <summary>
        /// Returns return_value or void depending on task template type T
        /// </summary>
        auto await_resume()
        {
            if constexpr (std::is_same_v<T, void>)
            {
                return;
            }
            else
            {
                return std::move(handle_.promise().value());
            }
        }

        task(coro&& handle, promise_type& promise): handle_{ std::move( handle ) }
        {
            promise.task_ptr_ = this;
        }

        ~task()
        {
            if (handle_)
            {
                handle_.destroy();
            }
        }
        coro handle_;
        std::function<bool()> inner_task_{ nullptr };

        static task<> delay(const std::chrono::milliseconds& time)
        {
            auto end_time = time + std::chrono::system_clock::now();
            co_await std::suspend_always{};
            while (end_time > std::chrono::system_clock::now())
            {
                co_await std::suspend_always{};
            }
        }
    };
}
