"""
Classes and Dataclasses — YOUR ATTEMPT

Write your implementations here, then run the tests against them:
    PRACTICE=1 uv run pytest ramp_up/python/06_classes_and_dataclasses -v

Peek at solution.py only after you've tried each drill.

Repr convention used throughout (tests check these exactly):
    Vector2D(x=1.0, y=2.0)        <- dataclass generates this for free
    Temperature(celsius=25.0)
    Stack([1, 2, 3])
    Task(priority=1, name='ship') <- dataclass generates this for free
    Point(x=3.0, y=4.0)           <- dataclass generates this for free
    Dog(name='Rex')
    Robot(model='R2')
"""
import math
from collections.abc import Iterator
from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class Vector2D:
    """
    Immutable 2D vector.

    JAVA:   an immutable POJO: final fields, constructor, getters, and
            hand-written equals()/hashCode(); arithmetic via method calls
            like BigDecimal — a.add(b).scale(2).
    PYTHON: @dataclass(frozen=True) generates __init__/__eq__/__repr__ AND
            __hash__ (the equals/hashCode contract, for free). Dunder
            methods make `v1 + v2` and `v1 - v2` just work.
    """

    x: float
    y: float

    def __add__(self, other: "Vector2D") -> "Vector2D":
        """
        JAVA:   public Vector2D add(Vector2D other)
        PYTHON: __add__ is what the + operator calls; return a NEW Vector2D.
        """
        return Vector2D(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2D") -> "Vector2D":
        """
        JAVA:   public Vector2D subtract(Vector2D other)
        PYTHON: __sub__ powers the - operator; return a NEW Vector2D.
        """
        return Vector2D(self.x - other.x, self.y - other.y)

    def scaled(self, k: float) -> "Vector2D":
        """
        JAVA:   public Vector2D scale(double k) — returns a new instance
                because the fields are final.
        PYTHON: same idea; frozen dataclasses can't mutate, so build and
                return a new Vector2D.
        """
        return Vector2D(self.x * k, self.y * k)

    def magnitude(self) -> float:
        """
        JAVA:   Math.sqrt(x * x + y * y) or Math.hypot(x, y).
        PYTHON: math.hypot(self.x, self.y).
        """
        return math.hypot(self.x, self.y)


class Temperature:
    """
    Stores celsius; exposes fahrenheit as a computed, settable attribute.

    JAVA:   private double celsius; getCelsius/setCelsius plus
            getFahrenheit/setFahrenheit doing the conversion.
    PYTHON: store a plain public attribute `self.celsius` (no getters "just
            in case"!). Expose fahrenheit via @property + @fahrenheit.setter
            so `t.fahrenheit = 212` converts and stores back into celsius —
            callers use attribute syntax either way.

    Conversions: F = C * 9/5 + 32, and C = (F - 32) * 5/9.
    """

    def __init__(self, celsius: float) -> None:
        self.celsius = celsius

    @property
    def fahrenheit(self) -> float:
        """Computed from self.celsius on every read."""
        return self.celsius * 9 / 5 + 32

    @fahrenheit.setter
    def fahrenheit(self, value: float) -> None:
        """Convert back and store into self.celsius."""
        self.celsius = (value - 32) * 5 / 9

    def __repr__(self) -> str:
        """Return 'Temperature(celsius=25.0)' style (like toString())."""
        return f"Temperature(celsius={self.celsius})"


class Stack:
    """
    LIFO stack wrapping a plain list.

    JAVA:   class Stack<T> implements Iterable<T> — you'd write size(),
            isEmpty(), iterator() returning an Iterator<T> object, and throw
            EmptyStackException from pop().
    PYTHON: define dunders and the whole language cooperates:
            __len__  -> len(stack) works
            __bool__ -> `if stack:` is "not empty" (len alone would give
                        this fallback; defining it makes the intent explicit)
            __iter__ -> for/list()/any() work; yield items TOP-FIRST
                        (pop order) — `iter(reversed-list)` is enough,
                        no Iterator class needed
            pop()/peek() on an empty stack: let the underlying list raise
            IndexError (EAFP) rather than pre-checking.
    """

    def __init__(self) -> None:
        self._items: list[Any] = []

    def push(self, item: Any) -> None:
        self._items.append(item)

    def pop(self) -> Any:
        """Remove and return the top item; IndexError if empty."""
        return self._items.pop()

    def peek(self) -> Any:
        """Return the top item without removing it; IndexError if empty."""
        return self._items[-1]

    def __len__(self) -> int:
        return len(self._items)

    def __bool__(self) -> bool:
        return bool(self._items)

    def __iter__(self) -> Iterator[Any]:
        return reversed(self._items)

    def __repr__(self) -> str:
        """Return 'Stack([1, 2, 3])' with items bottom-to-top."""
        return f"Stack({self._items!r})"


@dataclass
class Task:
    """
    A prioritized task; lower priority value = more urgent.

    JAVA:   class Task implements Comparable<Task> { public int compareTo... }
            so PriorityQueue and Collections.sort accept it.
    PYTHON: define __lt__ (just less-than — not a three-way compareTo) and
            both sorted() and heapq order Tasks directly. The dataclass
            already generated __eq__ and __repr__.
    """

    priority: int
    name: str

    def __lt__(self, other: "Task") -> bool:
        """Order by priority only (name is not part of the ordering)."""
        return self.priority < other.priority


@dataclass
class Point:
    """
    JAVA:   a static factory: public static Point fromString(String s).
    PYTHON: @classmethod receives the class as `cls` — construct with
            cls(...) so subclasses get instances of themselves.
    """

    x: float
    y: float

    @classmethod
    def from_string(cls, s: str) -> "Point":
        """
        Parse "3,4" (whitespace around numbers allowed, e.g. "3, 4")
        into Point(x=3.0, y=4.0).
        """
        x_str, y_str = s.split(",")
        return cls(float(x_str), float(y_str))


class Dog:
    """A speaker with no shared base class or interface — see describe()."""

    def __init__(self, name: str) -> None:
        self._name = name

    def speak(self) -> str:
        """Return '<name> says woof'."""
        return f"{self._name} says woof"

    def __repr__(self) -> str:
        """Return "Dog(name='Rex')" style."""
        return f"Dog(name={self._name!r})"


class Robot:
    """Also speaks; shares NO base class with Dog. That's the point."""

    def __init__(self, model: str) -> None:
        self._model = model

    def speak(self) -> str:
        """Return '<model> goes beep boop'."""
        return f"{self._model} goes beep boop"

    def __repr__(self) -> str:
        """Return "Robot(model='R2')" style."""
        return f"Robot(model={self._model!r})"


def describe(obj: Any) -> str:
    """
    Return '<ClassName>: <whatever obj.speak() returns>',
    e.g. describe(Dog("Rex")) == "Dog: Rex says woof".

    JAVA:   requires `interface Speaker { String speak(); }` and every class
            declaring `implements Speaker` before this method can accept it.
    PYTHON: duck typing — call obj.speak() on ANY object that has it. No
            interface, no cast, no common superclass. Use type(obj).__name__
            for the class name.
    """
    return f"{type(obj).__name__}: {obj.speak()}"
