# Terminal Snake

A terminal game in C using the course's ncurses rendering framework and trace-based tests.

## Design

The snake stores row and column coordinates in two linked lists, with the head at the front. The project exercises heap allocation, linked-list updates, board parsing, collision handling, and cleanup.

Optional features in [src/common.h](src/common.h) include growth per food, bot behavior, and alternate rendering. They are disabled by default. Rebuild with `make clean` followed by `make` after changing these compile-time settings. The bot is a simple game heuristic, not a general path-planning algorithm.

## Build and run

Inside the Linux course container, from this directory:

```bash
make
./snake 1
make check
```

Use the arrow keys to steer. The argument `1` enables growth; `0` disables it. The build requires ncurses development libraries and Python 3 for the existing 57-test harness. See [../VALIDATION.md](../VALIDATION.md) for observed results.
