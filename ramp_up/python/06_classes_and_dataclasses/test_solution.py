import heapq
import math

import pytest
from dataclasses import FrozenInstanceError
from solution import Dog, Point, Robot, Stack, Task, Temperature, Vector2D, describe


class TestVector2D:
    def test_add_operator(self):
        assert Vector2D(1.0, 2.0) + Vector2D(3.0, 4.0) == Vector2D(4.0, 6.0)

    def test_sub_operator(self):
        assert Vector2D(5.0, 5.0) - Vector2D(2.0, 3.0) == Vector2D(3.0, 2.0)

    def test_scaled_and_magnitude(self):
        v = Vector2D(3.0, 4.0)
        assert v.scaled(2) == Vector2D(6.0, 8.0)
        assert v.magnitude() == 5.0
        assert Vector2D(0.0, 0.0).magnitude() == 0.0  # edge: zero vector

    def test_frozen_means_immutable(self):
        v = Vector2D(1.0, 2.0)
        with pytest.raises(FrozenInstanceError):
            v.x = 99.0

    def test_equals_and_hashcode_for_free(self):
        # In Java you'd hand-write equals() and hashCode(); frozen=True
        # generates both, so Vector2D works in sets and as dict keys.
        assert Vector2D(1.0, 2.0) == Vector2D(1.0, 2.0)
        assert len({Vector2D(1.0, 2.0), Vector2D(1.0, 2.0)}) == 1

    def test_repr(self):
        assert repr(Vector2D(1.0, 2.0)) == "Vector2D(x=1.0, y=2.0)"


class TestTemperature:
    def test_fahrenheit_computed_from_celsius(self):
        assert Temperature(100.0).fahrenheit == 212.0
        assert Temperature(0.0).fahrenheit == 32.0

    def test_setting_fahrenheit_updates_celsius(self):
        t = Temperature(0.0)
        t.fahrenheit = 212.0  # attribute syntax, setter logic
        assert t.celsius == pytest.approx(100.0)

    def test_celsius_is_a_plain_attribute(self):
        t = Temperature(20.0)
        t.celsius = 25.0  # no setCelsius() — direct assignment is idiomatic
        assert t.fahrenheit == pytest.approx(77.0)

    def test_repr(self):
        assert repr(Temperature(25.0)) == "Temperature(celsius=25.0)"


class TestStack:
    def test_push_pop_peek_lifo(self):
        s = Stack()
        s.push(1)
        s.push(2)
        s.push(3)
        assert s.peek() == 3
        assert s.pop() == 3
        assert s.pop() == 2
        assert len(s) == 1

    def test_len_and_bool(self):
        s = Stack()
        assert len(s) == 0
        assert not s  # empty stack is falsy, like isEmpty()
        s.push("x")
        assert s  # non-empty is truthy
        assert len(s) == 1

    def test_iter_yields_top_first(self):
        s = Stack()
        s.push("a")
        s.push("b")
        s.push("c")
        assert list(s) == ["c", "b", "a"]  # pop order
        assert len(s) == 3  # iterating does not consume the stack

    def test_pop_and_peek_on_empty_raise(self):
        s = Stack()
        with pytest.raises(IndexError):
            s.pop()
        with pytest.raises(IndexError):
            s.peek()

    def test_repr(self):
        s = Stack()
        s.push(1)
        s.push(2)
        assert repr(s) == "Stack([1, 2])"


class TestTask:
    def test_sorted_orders_by_priority(self):
        tasks = [Task(3, "clean"), Task(1, "ship"), Task(2, "test")]
        assert [t.name for t in sorted(tasks)] == ["ship", "test", "clean"]

    def test_works_with_heapq_directly(self):
        heap: list[Task] = []
        heapq.heappush(heap, Task(2, "test"))
        heapq.heappush(heap, Task(1, "ship"))
        heapq.heappush(heap, Task(3, "clean"))
        assert heapq.heappop(heap).name == "ship"
        assert heapq.heappop(heap).name == "test"

    def test_dataclass_equality_for_free(self):
        assert Task(1, "ship") == Task(1, "ship")
        assert Task(1, "ship") != Task(2, "ship")

    def test_repr(self):
        assert repr(Task(1, "ship")) == "Task(priority=1, name='ship')"


class TestPointFromString:
    def test_basic_parse(self):
        assert Point.from_string("3,4") == Point(3.0, 4.0)

    def test_whitespace_and_floats(self):
        assert Point.from_string(" 1.5, -2.0 ") == Point(1.5, -2.0)

    def test_returns_subclass_instances(self):
        # Because the factory uses cls(...), subclasses come back as
        # themselves — like a static factory that respects `this` class.
        class LabeledPoint(Point):
            pass

        p = LabeledPoint.from_string("0,0")
        assert type(p) is LabeledPoint

    def test_repr(self):
        assert repr(Point(3.0, 4.0)) == "Point(x=3.0, y=4.0)"


class TestDescribe:
    def test_dog_speaks(self):
        assert describe(Dog("Rex")) == "Dog: Rex says woof"

    def test_robot_speaks(self):
        assert describe(Robot("R2")) == "Robot: R2 goes beep boop"

    def test_any_object_with_speak_works(self):
        # No interface, no inheritance: a class defined right here quacks
        # its way in. This would not compile in Java without `implements`.
        class Duck:
            def speak(self) -> str:
                return "quack"

        assert describe(Duck()) == "Duck: quack"

    def test_reprs(self):
        assert repr(Dog("Rex")) == "Dog(name='Rex')"
        assert repr(Robot("R2")) == "Robot(model='R2')"
