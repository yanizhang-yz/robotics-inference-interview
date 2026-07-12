"""
Classes and Dataclasses — reference solutions.

The theme: @dataclass is Lombok built into the language, dunder methods are
how you "implement interfaces" (Iterable, Comparable, toString) without
declaring any, and duck typing replaces the interface itself.
"""

import math
from collections.abc import Iterator
from dataclasses import dataclass
from typing import Any


@dataclass(frozen=True)
class Vector2D:
    # frozen=True: immutable, and __hash__ comes with __eq__ — the whole
    # equals()/hashCode() contract generated correctly, plus a repr that
    # reads like the constructor call: Vector2D(x=1.0, y=2.0).
    x: float
    y: float

    def __add__(self, other: "Vector2D") -> "Vector2D":
        # This is what `v1 + v2` calls — no .add() ceremony.
        return Vector2D(self.x + other.x, self.y + other.y)

    def __sub__(self, other: "Vector2D") -> "Vector2D":
        return Vector2D(self.x - other.x, self.y - other.y)

    def scaled(self, k: float) -> "Vector2D":
        return Vector2D(self.x * k, self.y * k)

    def magnitude(self) -> float:
        return math.hypot(self.x, self.y)


class Temperature:
    def __init__(self, celsius: float) -> None:
        # A plain public attribute — no getter/setter pair "just in case".
        self.celsius = celsius

    @property
    def fahrenheit(self) -> float:
        # Callers write `t.fahrenheit` — attribute syntax, computed value.
        return self.celsius * 9 / 5 + 32

    @fahrenheit.setter
    def fahrenheit(self, value: float) -> None:
        # `t.fahrenheit = 212` lands here and converts back to celsius.
        self.celsius = (value - 32) * 5 / 9

    def __repr__(self) -> str:
        return f"Temperature(celsius={self.celsius})"


class Stack:
    def __init__(self) -> None:
        self._items: list[Any] = []  # _prefix = "internal", by convention

    def push(self, item: Any) -> None:
        self._items.append(item)

    def pop(self) -> Any:
        # EAFP: no isEmpty() pre-check; the empty list raises IndexError.
        return self._items.pop()

    def peek(self) -> Any:
        return self._items[-1]

    def __len__(self) -> int:
        return len(self._items)

    def __bool__(self) -> bool:
        # __len__ alone would already make empty stacks falsy; defining
        # __bool__ makes the truthiness contract explicit.
        return bool(self._items)

    def __iter__(self) -> Iterator[Any]:
        # Top-first (pop order). No Iterator class — reversed() over the
        # list IS the iterator.
        return reversed(self._items)

    def __repr__(self) -> str:
        return f"Stack({self._items!r})"


@dataclass
class Task:
    priority: int
    name: str

    def __lt__(self, other: "Task") -> bool:
        # sorted() and heapq only ever need <, not a 3-way compareTo.
        return self.priority < other.priority


@dataclass
class Point:
    x: float
    y: float

    @classmethod
    def from_string(cls, s: str) -> "Point":
        # cls, not Point: a subclass calling from_string gets the subclass.
        x_str, y_str = s.split(",")
        return cls(float(x_str), float(y_str))


class Dog:
    def __init__(self, name: str) -> None:
        self.name = name

    def speak(self) -> str:
        return f"{self.name} says woof"

    def __repr__(self) -> str:
        return f"Dog(name={self.name!r})"


class Robot:
    def __init__(self, model: str) -> None:
        self.model = model

    def speak(self) -> str:
        return f"{self.model} goes beep boop"

    def __repr__(self) -> str:
        return f"Robot(model={self.model!r})"


def describe(obj: Any) -> str:
    # Duck typing: Dog and Robot share no base class and declare no
    # interface — having a speak() method is the entire contract.
    return f"{type(obj).__name__}: {obj.speak()}"
