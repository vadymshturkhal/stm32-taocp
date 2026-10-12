"""Tests of seal() / unseal() in line.py (the Pi side).

1. seal: the known line, refusing '\\r', '\\n' and non-ASCII bodies.
2. Round trip: unseal(seal(body)) == body.
3. LINE_CASES: the same table as LINE_CASES in tests/test_line_reader.cpp (keep the
   two in sync). There the MCU's LineReader + unseal must give the same answers,
   so both sides accept exactly the same lines.
4. Every single-byte change of a sealed line is rejected, without an exception.

Run (from tests/ or its parent):  python3 test_line.py
Exit status: 0 if every check passed.
"""
import pathlib
import sys

# The code is one folder up (pi_mcu_protocol/), the tests are in tests/
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent.parent))

from line import seal, unseal

# (name, bytes on the wire, accepted?) -- the same table as in test_line_reader.cpp
LINE_CASES = [
    ("a normal line",                  b"2 0 5 hello*EBDC\r\n",     True),
    ("'\\n' without '\\r'",            b"2 0 5 hello*EBDC\n",       True),
    ("two '\\r' before '\\n'",         b"2 0 5 hello*EBDC\r\r\n",   False),
    ("three '\\r' before '\\n'",       b"2 0 5 hello*EBDC\r\r\r\n", False),
    ("'\\r' before the line",          b"\r2 0 5 hello*EBDC\r\n",   False),
    ("'\\r' inserted in the body",     b"2 \r0 5 hello*EBDC\r\n",   False),
    ("two '\\r' inserted in the body", b"2 0 5 he\r\rllo*EBDC\r\n", False),
    ("'\\r' inserted in the CRC",      b"2 0 5 hello*EB\rDC\r\n",   False),
    ("a body byte replaced by '\\r'",  b"2 \r 5 hello*EBDC\r\n",    False),
    ("only '\\r's",                    b"\r\r\r\n",                 False),
]

failures = 0


def check(name, ok):
    """Print one result line and count it if it failed."""
    global failures
    print(f"{'PASS' if ok else 'FAIL'}  {name}")
    failures += not ok


def refuses(body):
    """True if seal(body) raises ValueError."""
    try:
        seal(body)
    except ValueError:
        return True
    return False


# 1. seal
check('seal("2 0 5 hello") == b"2 0 5 hello*EBDC\\r\\n"', seal("2 0 5 hello") == b"2 0 5 hello*EBDC\r\n")
check("seal refuses '\\r', '\\n' and non-ASCII", refuses("a\rb") and refuses("a\nb") and refuses("é"))

# 2. Round trip
bodies = ["", " hi", "hi ", "a*b*c", "2 0 5 hello", "  "]
check("round trip unseal(seal(b)) == b", all(unseal(seal(b)) == b for b in bodies))

# 3. The shared table
for name, wire, accepted in LINE_CASES:
    body = unseal(wire)
    ok = body == "2 0 5 hello" if accepted else body is None
    check(f"unseal: {name} -> {'accepted' if accepted else 'rejected'}", ok)

# 4. Every single-byte change is rejected
line = seal("2 0 5 hello")
wrongly_accepted = 0
for i in range(len(line)):
    for value in range(256):
        if value == line[i]:
            continue
        damaged = bytearray(line)
        damaged[i] = value
        body = unseal(bytes(damaged))
        if body is not None and body != "2 0 5 hello":
            wrongly_accepted += 1
check("every single-byte change gives None or the original body", wrongly_accepted == 0)

print("\nall passed" if not failures else "\nSOME TESTS FAILED")
sys.exit(1 if failures else 0)
