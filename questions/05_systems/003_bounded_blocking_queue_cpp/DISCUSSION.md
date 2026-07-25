# Quest 04 Notes

Use this file after your first attempt.

## What I Implemented

-

## Bugs I Hit

-

## Interview Explanation

- Where this queue fits:
- Why it is bounded:
- Why shutdown is hard:
- What I would measure:

## Follow-Up Questions To Review

- Why is `std::unique_lock` used with `std::condition_variable`?
- Why must the wait use a predicate?
- What happens if `notify_one()` is called before the waiting thread sleeps?
- What happens to blocked producers after `close()`?
- What happens to blocked consumers after `close()`?
