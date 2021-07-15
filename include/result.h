#ifndef MP4_MANIPULATOR_ERROR_H_
#define MP4_MANIPULATOR_ERROR_H_

#include <cassert>
//#include <source_location>
#include <variant>

// Simple class for storing results. A result is either ok, or an error. If the
// result is an error then it must be handled prior to a result being destroyed
// via MarkErrorHandled or the result will assert. Supports moving, and moving
// the ok or error out of the result via GetOk and GetErr when called on
// rvalues.

template <typename V, typename E>
class Result {
 public:
  static Result Ok() {
    return Result{std::in_place_index<0>};
  }

  static Result Ok(V ok_value) {
    return Result{std::in_place_index<0>, std::move(ok_value)};
  }

  static Result Err(E error_value) {
    return Result<V, E>{std::in_place_index<1>, std::move(error_value)};
  }

  Result(Result&& other) noexcept : result_(std::move(other.result_)) {
    other.MarkErrorHandled();
  }

  Result& operator=(Result&& other) noexcept {
    if (this == &other) {
      // Handle self assignment.
      return *this;
    }
    result_ = std::move(other.result_);
    other.MarkErrorHandled();
    return *this;
  }

  ~Result() {
    // Ensure errors are handled by the time dtor is called.
    assert(IsOk() || is_error_handled_);
  }

  Result(Result&) = delete;
  Result& operator=(Result&) = delete;

  bool IsOk() {
    assert(result_.index() <= 1);
    return result_.index() == 0;
  }

  bool IsErr() {
    assert(result_.index() <= 1);
    return result_.index() == 1;
  }

  // Get a reference to the ok value.
  V& GetOk() & {
    assert(IsOk());
    return std::get<0>(result_);
  }

  // Get an rvalue reference to the ok value. Consumes the Result.
  V&& GetOk() && {
    assert(IsOk());
    return std::get<0>(std::move(result_));
  }

  // Get a reference to the error value.
  E& GetErr() & {
    assert(IsErr());
    return std::get<1>(result_);
  }

  // Get an rvalue reference to the error value. Consumes the Result.
  E&& GetErr() && {
    assert(IsErr());
    return std::get<1>(std::move(result_));
  }

  // Marks that this Result's error has been handled.
  void MarkErrorHandled() {
    assert(!is_error_handled_);  // Don't double handle.
    is_error_handled_ = true;
  }

 private:
  Result(std::in_place_index_t<0> index)
      : result_(index) {
    assert(IsOk());
  }

  Result(std::in_place_index_t<0> index, V ok_value)
      : result_(index, std::move(ok_value)) {
    assert(IsOk());
  }

  Result(std::in_place_index_t<1> index, E error_value)
      : result_(index, std::move(error_value)) {
    assert(IsErr());
  }

  // Has the error result been handled? Not used for the ok case. Used to
  // ensure error paths are handled.
  bool is_error_handled_ = false;
  std::variant<V, E> result_;
  // Clang doesn't have source_location support yet. Add this in once it does.
  //std::optional<std::source_location> error_location;
};

#endif  //  MP4_MANIPULATOR_ERROR_H_