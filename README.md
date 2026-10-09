# IPC Showcase

A Linux C project demonstrating communication between cooperating processes with
pipes, signals, System V shared memory and semaphores, and TCP/UDP sockets. The
`zadanie` program is the process coordinator; the individual `proc_*` programs
implement parts of the pipeline.

## Current state

The checked-in source is incomplete for an end-to-end run. `zadanie` expects to
execute `proc_pr`, `proc_s`, and `proc_serv1`, but their source files and build
targets are not present. The other included programs can be compiled, but the
coordinator cannot complete its normal startup until those three components
are supplied.

## Build

Requirements: Linux, a C compiler, Meson, and Ninja. A Nix development shell is
also defined in `flake.nix` and provides these tools.

```sh
meson setup build
meson compile -C build
```

Or enter the provided development shell first:

```sh
nix develop
meson setup build
meson compile -C build
```

Meson currently builds `zadanie`, `proc_p1`, `proc_p2`, `proc_t`, `proc_d`, and
`proc_serv2`, along with the shared `helpers` library.

## Intended use

The coordinator's argument format is:

```text
zadanie <tcp-port> <udp-port>
```

It starts child programs using paths relative to the current working directory
(`./proc_p1`, `./proc_pr`, and so on). Once the missing components are
available, run it from the build directory so those paths resolve. The input
files are also opened relative to the working directory:

- `p1.txt` supplies lines to `proc_p1`.
- `p2.txt` supplies lines to `proc_p2`.

Both producers wait for `SIGUSR1` before reading and forwarding a line. The
included `proc_t` reads from a pipe and writes to System V shared memory under
semaphore control. `proc_d` reads from a second shared-memory segment and sends
NUL-terminated messages over TCP to `127.0.0.1:<tcp-port>`. The included
`proc_serv2` listens for UDP datagrams on `127.0.0.1:<udp-port>` and records
received messages, one per line, in `serv2.txt` in the working directory. It
recreates that output file when it starts.

`zadanie` uses fixed System V IPC keys for its shared-memory segments and
semaphores. After starting its children, it waits for up to 15 seconds for a
signal, then stops the children and removes the IPC resources it created.

## Source layout

- `src/zadanie.c` — process launcher and IPC resource coordinator.
- `src/helpers.c` — line reading, pipe writing, semaphore, signal, and socket
  helpers.
- `src/proc_p1.c`, `src/proc_p2.c` — file-to-pipe producers.
- `src/proc_t.c` — pipe-to-shared-memory stage.
- `src/proc_d.c` — shared-memory-to-TCP stage.
- `src/proc_serv2.c` — UDP receiver and `serv2.txt` writer.
- `include/` — declarations and shared configuration for these programs.
- `meson.build` — build targets; `flake.nix` — optional Nix development shell.
