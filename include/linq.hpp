#ifndef LINQ_HPP
#define LINQ_HPP

#include <utility>
#include <vector>
#include <algorithm>
#include <functional>

namespace linq {

    template<typename T, typename Iter>
    class range_enumerator;

    template<typename T>
    class drop_enumerator;

    template<typename T>
    class take_enumerator;

    template<typename T, typename U, typename F>
    class select_enumerator;

    template<typename T, typename F>
    class until_enumerator;

    template<typename T, typename F>
    class where_enumerator;

    template<typename T>
    class enumerator {
    public:

        enumerator(const enumerator &) = delete;

        enumerator &operator=(const enumerator &) = delete;

        enumerator &operator=(enumerator &&) = delete;

        enumerator(enumerator &&) noexcept = default;

        enumerator() = default;

        ~enumerator() = default;

        virtual T operator*() const = 0;

        virtual enumerator<T> &operator++() = 0;

        virtual explicit operator bool() const = 0;

        auto drop(int count) {
            return drop_enumerator<T>(*this, count);
        }

        auto take(int count) {
            return take_enumerator<T>(*this, count);
        }

        template<typename U = T, typename F>
        auto select(F func) {
            return select_enumerator<T, U, F>(*this, func);
        }

        template<typename F>
        auto until(F func) {
            return until_enumerator<T, F>(*this, func);
        }

        auto until_eq(const T &object) {
            return until_enumerator<T, std::function<bool(const T &)>>
                    (*this, [&object](const T &check) { return check == object; });
        }

        template<typename F>
        auto where(F func) {
            return where_enumerator<T, F>(*this, func);
        }

        auto where_neq(const T &object) {
            return where_enumerator<T, std::function<bool(const T &)>>
                    (*this, [&object](const T &check) { return check != object; });
        }

        std::vector<T> to_vector() {
            std::vector<T> result;
            while (*this) {
                result.push_back(**this);
                ++(*this);
            }
            return result;
        }

        template<typename Iter>
        void copy_to(Iter it) {
            while ((bool) *this) {
                *it = **this;
                ++(*this);
                ++it;
            }
        }

    };

    template<typename T, typename Iter>
    class range_enumerator final : public enumerator<T> {
    public:
        range_enumerator(Iter begin, Iter end) : begin_{begin}, end_{end} {
        }

        T operator*() const override { return *begin_; }

        enumerator<T> &operator++() override {
            ++begin_;
            return *this;
        }

        explicit operator bool() const override {
            return begin_ != end_;
        }

    private:
        Iter begin_, end_;
    };

    template<typename T>
    auto from(T begin, T end) {
        range_enumerator<typename std::iterator_traits<T>::value_type, T> result(begin, end);
        return result;
    }

    template<typename T>
    class drop_enumerator final : public enumerator<T> {
    public:
        drop_enumerator(enumerator<T> &parent, int count) : parent_{parent} {
            for (int i = 0; i < count; ++i) {
                ++parent_;
            }
        }

        T operator*() const override {
            return *parent_;
        }

        enumerator<T> &operator++() override {
            ++parent_;
            return *this;
        }

        explicit operator bool() const override {
            return (bool) parent_;
        }

    private:
        enumerator<T> &parent_;
    };

    template<typename T>
    class take_enumerator final : public enumerator<T> {
    public:
        take_enumerator(enumerator<T> &parent, int last) : parent_{parent}, last_{last} {
        }

        T operator*() const override {
            return *parent_;
        }

        enumerator<T> &operator++() override {
            --last_;
            if (last_ > 0) {
                ++parent_;
            }
            return *this;
        }

        explicit operator bool() const override {
            return (bool) parent_ && last_ > 0;
        }

    private:
        enumerator<T> &parent_;
        int last_;
    };

    template<typename T, typename U, typename F>
    class select_enumerator final : public enumerator<U> {
    public:
        select_enumerator(enumerator<T> &parent, F func) : parent_{parent}, func_{func} {
        }

        U operator*() const override {
            return func_(*parent_);
        }

        enumerator<U> &operator++() override {
            ++parent_;
            return *this;
        }

        explicit operator bool() const override {
            return (bool) parent_;
        }

    private:
        enumerator<T> &parent_;
        F func_;
    };

    template<typename T, typename F>
    class until_enumerator final : public enumerator<T> {

    public:
        until_enumerator(enumerator<T> &parent, F predicate) : parent_{parent}, predicate_{predicate} {
        }

        T operator*() const override {
            return *parent_;
        }

        enumerator<T> &operator++() override {
            if ((bool) *this) {
                ++parent_;
            }
            return *this;
        }

        explicit operator bool() const override {
            return (bool) parent_ && !predicate_(*parent_);
        }

    private:
        enumerator<T> &parent_;
        F predicate_;
    };

    template<typename T, typename F>
    class where_enumerator final : public enumerator<T> {
    public:
        where_enumerator(enumerator<T> &parent, F predicate) : parent_{parent}, predicate_{predicate} {
            if ((bool) parent_ && !predicate_(*parent_)) ++(*this);
        }

        T operator*() const override {
            return *parent_;
        }

        enumerator<T> &operator++() override {
            ++parent_;
            while ((bool) *this && !predicate_(*parent_)) {
                ++parent_;
            }
            return *this;
        }

        explicit operator bool() const override {
            return (bool) parent_;
        }

    private:
        enumerator<T> &parent_;
        F predicate_;
    };

} // namespace linq

#endif