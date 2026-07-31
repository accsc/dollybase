#!/usr/bin/env python3
"""
run_stress.py — Run dollybase stress tests via NcursesAgent (pexpect + pyte).

Each test is launched in a PTY. The agent reads the rendered screen through
pyte so we can assert on what's actually visible.

Interactive tests (INKEY, READ, WAIT) are driven by a driver map that sends
the right keystrokes at the right time.

Usage:
    python3 int/run_stress.py                          # run all stress_*.prg
    python3 int/run_stress.py int/stress/stress_basic.prg  # run one
"""

import gc
import glob
import os
import re
import sys

# Make sure the repo root is on the path so tui_agent is importable
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from tui_agent import NcursesAgent, ENTER, ESC

PRG = os.path.abspath(os.path.join(os.path.dirname(__file__), "prg"))


# Interactive test drivers: map test name -> list of (wait_for_text, keys_to_send)
# Each step waits for text on screen, then sends keys.
INTERACTIVE_DRIVERS = {
    "stress_inkey.prg": [
        # Test 3: waits for a key
        ("Press a key", "A"),
        # Test 4: waits for a key within 0.5s
        ("Press a key within", "B"),
        # Test 5: press keys, Enter to stop
        ("Press keys, Enter to stop", "C\r"),
    ],
    "stress_menu.prg": [
        # Select option 1, WAIT dismisses with any key
        ("Select an option", "1"),
        ("You selected", "x"),  # dismiss WAIT
        # Select option 0 to exit, WAIT dismisses with any key
        ("Select an option", "0"),
        ("Goodbye", "x"),  # dismiss WAIT after Goodbye
    ],
    "stress_ui.prg": [
        # 4 READs — each needs Esc to exit
        ("Enter name:", ESC),
        ("Enter age", ESC),
        ("Enter score", ESC),
        ("First:", ESC),
    ],
    "stress_rect.prg": [
        # WAIT at the end needs a keypress
        ("Press any key to finish", "x"),
    ],
}


def extract_expected(prg_path):
    """Find the last ? or @...SAY line and extract its first string literal.

    For tests with conditional output (IF/ELSE), returns a list of possible
    expected phrases.
    """
    with open(prg_path) as f:
        lines = f.readlines()
    expected = []
    for line in reversed(lines):
        line = line.strip()
        if line.startswith('?') or line.startswith('??'):
            m = re.search(r'"([^"]*)"', line)
            if m and m.group(1).strip():
                expected.append(m.group(1).strip())
        if line.startswith('@') and 'SAY' in line.upper():
            m = re.search(r'"([^"]*)"', line)
            if m and m.group(1).strip():
                expected.append(m.group(1).strip())
    # Return the last 2 expected phrases (handles IF/ELSE branches)
    if expected:
        return expected[-2:] if len(expected) >= 2 else expected
    return ["Done"]


def run_test(prg_path, max_wait=5):
    """
    Run a stress test. Return (passed, detail).

    For interactive tests, uses the INTERACTIVE_DRIVERS map to send keystrokes.
    For non-interactive tests, pumps until idle and checks output.
    """
    abs_path = os.path.abspath(prg_path)
    run_dir = os.path.dirname(abs_path)
    name = os.path.basename(abs_path)
    expected_list = extract_expected(prg_path)

    # Use absolute path for prg so it works after cd
    cmd = f'bash -c "cd {run_dir} && exec {PRG} {os.path.basename(abs_path)}"'

    agent = NcursesAgent(
        cmd,
        rows=500,
        cols=160,
        timeout=max_wait + 5,
    )

    try:
        driver = INTERACTIVE_DRIVERS.get(name)

        if driver:
            # Interactive test: drive step by step
            for wait_text, keys in driver:
                try:
                    agent.wait_for(wait_text, timeout=5, check_raw=True)
                except TimeoutError:
                    # If we can't find the prompt, try sending keys anyway
                    pass
                agent.send_key(keys)
                agent._pump(1, idle_limit=0.5)

            # Final pump to capture remaining output
            agent._pump(3, idle_limit=2.0)
        else:
            # Non-interactive: pump until idle
            agent._pump(max_wait, idle_limit=2.0)

        # Capture screen BEFORE sending 'q' (exit message overwrites ? output)
        screen_before = agent.display()
        raw_before = agent.raw_text()

        # Send 'q' to dismiss the "Press any key to continue..." exit prompt
        agent.send_key("q")
        agent._pump(2, idle_limit=1.0)

        # Also capture after (for WAIT-dismissed tests)
        screen_after = agent.display()
        raw_after = agent.raw_text()

        # Check both before and after
        combined = (screen_before + "\n" + raw_before + "\n" +
                     screen_after + "\n" + raw_after).lower()

        found_any = any(e.lower() in combined for e in expected_list)
        if found_any:
            detail = "found expected text"
            passed = True
        elif raw_before.strip() or screen_before.strip():
            detail = f"none of {expected_list} found; has output"
            passed = False
        else:
            detail = "empty screen"
            passed = False
    finally:
        try:
            agent.expect_exit(timeout=3)
        except:
            pass
        agent.close()
        del agent
        gc.collect()

    return passed, detail


def main():
    if len(sys.argv) < 2:
        targets = sorted(glob.glob(os.path.join(os.path.dirname(__file__), "stress", "stress_*.prg")))
    else:
        targets = list(sys.argv[1:])

    if not targets:
        print("No stress test files found.")
        sys.exit(1)

    passed = 0
    failed = 0

    for prg_path in targets:
        name = os.path.basename(prg_path)
        ok, detail = run_test(prg_path)

        if ok:
            print(f"  OK   {name}")
            passed += 1
        else:
            print(f" FAIL {name} — {detail}")
            failed += 1

    print(f"\n{passed} passed, {failed} failed out of {passed + failed}")
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
