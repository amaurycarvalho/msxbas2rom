#!/usr/bin/env python3
"""Send a host keyboard event to the openMSX window with XTEST.

Used by the msx-memory-debug skill to drive openMSX's keyjoystick pluggable when
an input script contains joystick events. Requires python-xlib and an X display.

Usage:
    xtest_key.py press KEY
    xtest_key.py release KEY
    xtest_key.py tap KEY [HOLD_SECONDS]
    xtest_key.py release_all
"""

import sys
import time

try:
    from Xlib import X, XK, display
    from Xlib.ext import xtest
except ImportError:  # pragma: no cover - optional host dependency
    print("WARNING: python-xlib is not installed; joystick input skipped", file=sys.stderr)
    sys.exit(0)

JOYSTICK_KEYSYMS = ("Left", "Right", "Up", "Down", "Control_L", "m")


def find_openmsx_window(disp):
    root = disp.screen().root
    stack = [root]
    while stack:
        window = stack.pop()
        try:
            name = window.get_wm_name()
            wm_class = window.get_wm_class() or ()
        except Exception:
            continue
        if name and "openmsx" in str(name).lower():
            return window
        if any("openmsx" in str(item).lower() for item in wm_class):
            return window
        try:
            stack.extend(window.query_tree().children)
        except Exception:
            continue
    return None


def focus_window(disp, window):
    try:
        window.configure(stack_mode=X.Above)
        window.set_input_focus(X.RevertToParent, X.CurrentTime)
        disp.sync()
        focused = disp.get_input_focus().focus
        if focused.id == window.id:
            return True
        # The focus may be on a child window of the same toplevel.
        parent = focused.query_tree().parent
        while parent is not None:
            if parent.id == window.id:
                return True
            parent = parent.query_tree().parent
    except Exception:
        pass
    return False


def send(disp, keysym_name, press):
    keysym = XK.string_to_keysym(keysym_name)
    if keysym == 0:
        print(f"WARNING: unknown X keysym: {keysym_name}", file=sys.stderr)
        return
    keycode = disp.keysym_to_keycode(keysym)
    event = X.KeyPress if press else X.KeyRelease
    xtest.fake_input(disp, event, keycode)
    disp.sync()


def main():
    if len(sys.argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    action = sys.argv[1].lower()
    disp = display.Display()

    if action == "release_all":
        for keysym_name in JOYSTICK_KEYSYMS:
            send(disp, keysym_name, False)
        return 0

    if len(sys.argv) < 3:
        print(__doc__.strip(), file=sys.stderr)
        return 2

    keysym_name = sys.argv[2]
    hold = float(sys.argv[3]) if len(sys.argv) > 3 else 0.1

    # Releases must work even after openMSX has exited: they only clear the
    # server-side key state. Presses must target openMSX, never the terminal.
    if action in {"release", "up"}:
        send(disp, keysym_name, False)
        return 0

    window = find_openmsx_window(disp)
    if window is None or not focus_window(disp, window):
        print(
            "WARNING: openMSX window not focused; keyboard event skipped",
            file=sys.stderr,
        )
        return 0

    if action in {"press", "down"}:
        send(disp, keysym_name, True)
    elif action == "tap":
        send(disp, keysym_name, True)
        time.sleep(hold)
        send(disp, keysym_name, False)
    else:
        print(f"WARNING: unknown action: {action}", file=sys.stderr)
    return 0


if __name__ == "__main__":
    sys.exit(main())
