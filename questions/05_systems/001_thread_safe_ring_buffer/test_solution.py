import threading

import pytest

from solution import RingBuffer


class TestBasics:
    def test_push_pop_fifo(self):
        rb = RingBuffer(3)
        assert rb.push(1) and rb.push(2) and rb.push(3)
        assert rb.pop() == 1
        assert rb.pop() == 2
        assert rb.pop() == 3

    def test_push_when_full_returns_false(self):
        rb = RingBuffer(2)
        assert rb.push("a") is True
        assert rb.push("b") is True
        assert rb.push("c") is False
        assert len(rb) == 2
        assert rb.pop() == "a"          # "c" was rejected, not queued

    def test_pop_empty_returns_none(self):
        rb = RingBuffer(1)
        assert rb.pop() is None

    def test_len_empty_full(self):
        rb = RingBuffer(2)
        assert rb.is_empty() and not rb.is_full() and len(rb) == 0
        rb.push(1)
        assert not rb.is_empty() and not rb.is_full() and len(rb) == 1
        rb.push(2)
        assert not rb.is_empty() and rb.is_full() and len(rb) == 2

    def test_invalid_capacity(self):
        with pytest.raises(ValueError):
            RingBuffer(0)


class TestWraparound:
    def test_indices_wrap(self):
        rb = RingBuffer(3)
        rb.push(1); rb.push(2); rb.push(3)
        assert rb.pop() == 1
        assert rb.push(4)                 # writes into the recycled slot
        assert [rb.pop(), rb.pop(), rb.pop()] == [2, 3, 4]

    def test_many_cycles(self):
        rb = RingBuffer(4)
        out = []
        for i in range(100):
            rb.push(i)
            out.append(rb.pop())
        assert out == list(range(100))    # steady-state through many wraps


class TestOverwrite:
    def test_overwrite_evicts_oldest(self):
        rb = RingBuffer(3)
        for i in (1, 2, 3):
            rb.push_overwrite(i)
        rb.push_overwrite(4)              # evicts 1
        assert len(rb) == 3
        assert [rb.pop(), rb.pop(), rb.pop()] == [2, 3, 4]

    def test_overwrite_below_capacity_behaves_like_push(self):
        rb = RingBuffer(3)
        rb.push_overwrite("x")
        assert len(rb) == 1 and rb.pop() == "x"

    def test_sensor_stream_keeps_freshest(self):
        rb = RingBuffer(5)
        for frame in range(100):          # camera outruns the model
            rb.push_overwrite(frame)
        got = [rb.pop() for _ in range(5)]
        assert got == [95, 96, 97, 98, 99]


class TestThreadSafety:
    def test_concurrent_producers_no_loss_no_dupes(self):
        rb = RingBuffer(16)
        n_producers, per_producer = 4, 500
        popped = []
        done = threading.Event()

        def produce(base):
            for i in range(per_producer):
                item = base + i
                while not rb.push(item):   # retry while full
                    pass

        def consume():
            while not done.is_set() or len(rb):
                item = rb.pop()
                if item is not None:
                    popped.append(item)

        consumer = threading.Thread(target=consume)
        consumer.start()
        producers = [
            threading.Thread(target=produce, args=(k * per_producer,))
            for k in range(n_producers)
        ]
        for t in producers:
            t.start()
        for t in producers:
            t.join()
        done.set()
        consumer.join(timeout=10)
        assert not consumer.is_alive(), "consumer hung"

        expected = set(range(n_producers * per_producer))
        assert len(popped) == len(expected), "items lost or duplicated"
        assert set(popped) == expected
