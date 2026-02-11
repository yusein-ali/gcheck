# Argument Generation in `gcheck`

This document explains how argument generation works in `gcheck` (`include/argument.h`), what distribution types are available, and how to use them effectively, especially for discrete test inputs.

## 1. Core Model

All generators implement `NextType<T>`:

1. `T& Next()` returns the next generated value.
2. `NextType<T>* Clone() const` returns a heap clone.

Most concrete generators derive from `Argument<T>`, which stores the current value in `value_`.

## 2. Distribution Types

`Random<T>` uses `Distribution<T>` internally.

### 2.1 `ChoiceDistribution<T>` (Discrete)

Uniformly selects from explicit choices.

```cpp
Random<int> x({0, 1, -1, 42});
Random<char> c(std::vector<char>{'a', 'b', 'c'});
```

Best for:

1. Edge-value focused tests.
2. Controlled finite domains.

### 2.2 `RangeDistribution<T>` (Numeric)

Uniform range sampling for integral/floating types.

```cpp
Random<int> r(1, 10);        // integers in [1,10]
Random<double> d(0.0, 1.0);  // reals in [0,1]
```

### 2.3 `RangeDistribution<T>` fallback (non-arithmetic)

For custom types supporting `end - start` and `start + offset`.

## 3. Main Generator Classes

### 3.1 `Argument<T>`

Constant/default generator. `Next()` returns the stored value.

```cpp
Argument<int> a(5);
a.Next(); // 5
```

### 3.2 `Random<T>`

Random generator over a chosen distribution.

```cpp
Random<int> by_range(10, 20);
Random<int> by_choices({1, 3, 5, 7}, 12345); // seeded
```

### 3.3 `SequenceArgument<T>`

Cycles through a fixed sequence.

```cpp
SequenceArgument<int> s({10, 20, 30});
// Next(): 10, 20, 30, 10, ...
```

### 3.4 `Generator<T, F>`

Wraps a callable:

1. `T f()`
2. `T f(size_t index)`

```cpp
auto g1 = Generator([] { return 7; });
auto g2 = Generator([](size_t i) { return int(i * i); });
```

### 3.5 `Container<T, ContainerT>`

Generates containers with dynamic/fixed size and element sources.

```cpp
Container<Random<int>> v(5, Random<int>(0, 9));
```

## 4. Composition Operators

## 4.1 `+` (combine sources)

`+` creates a `Combine<...>` source.

1. Discrete + discrete -> `Combine<DiscreteCombine,...>`
2. Continuous + continuous -> `Combine<ContinuousCombine,...>`

```cpp
auto mixed = Random<int>({0,1,2}) + Random<int>({100,101});
```

Selection is proportional to each component's `ChoiceLength()`.

## 4.2 `*` (join sources)

`*` creates a tuple-producing `Join<...>` source.

```cpp
auto args = Random<int>({1,2}) * Random<char>({'x','y'});
// Next() -> std::tuple<int, char>
```

## 5. `RandomSizeContainer(...)` Helpers

Convenience overloads for random-size container generation:

```cpp
auto c1 = RandomSizeContainer(1, 5, -10, 10);
auto c2 = RandomSizeContainer(std::vector<size_t>{0,1,3}, std::vector<int>{-1,0,1});
```

Recommended discrete pattern:

1. Use discrete size sets for meaningful lengths.
2. Use discrete value sets for edge values.

## 6. `advance(...)`

`advance(...)` calls `.Next()` only for `NextType`-derived objects; otherwise no-op.

```cpp
Random<int> r(0, 10);
int plain = 3;
advance(r, plain); // advances r, ignores plain
```

## 7. Effective Discrete Strategy

For robust discrete tests:

1. Build explicit value pools (valid, boundary, invalid-like representatives).
2. Seed random generators for reproducibility in CI.
3. Use `+` to mix semantic groups (e.g., small values + extreme values).
4. Use `*` to generate structured input tuples.
5. Use `RandomSizeContainer` with discrete size sets to include `0`, `1`, and larger sizes.

Example:

```cpp
auto ids = Random<int>({0, 1, -1, 999999}, 42);
auto flags = Random<bool>({false, true}, 42);
auto payloads = RandomSizeContainer(std::vector<size_t>{0,1,8}, std::vector<char>{'a','b','\n'});
auto input = ids * flags * payloads;
```

## 8. Integration with Tests

In `FunctionTest`/`IOTest` setup methods, use generated values to feed `SetArguments(...)` and expected outputs.

Typical flow:

1. Create generators as members/globals.
2. In each run, call `Next()` (or `advance(...)`).
3. Apply values via test setup APIs.
