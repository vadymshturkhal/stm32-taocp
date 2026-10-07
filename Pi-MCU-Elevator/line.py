from crc16 import crc16

def seal(body: str) -> bytes:
    """
    Encode the body with crc to bytes: b"<body>*XXXX\r\n"
    """
    if "\r" in body or "\n" in body:
        raise ValueError(r"\r or \n in the body")

    try:
        encoded = body.encode("ascii")
    except UnicodeError:
        # Added "from None" for hiding other errors
        raise ValueError(f"body must be ASCII: {body = }") from None

    crc = crc16(encoded)

    return encoded + b"*" + f"{crc:04X}".encode("ascii") + b"\r\n"

def unseal(line: bytes) -> str | None:
    """
    Return the body of the line or None if line is damaged 
    """

    # Get rid of \r\n
    line = line.removesuffix(b"\n").removesuffix(b"\r")

    # Check the length and body
    if len(line) < 5 or line[-5] != ord("*"):
        return None

    body, _, crc = line.rpartition(b"*")

    # Forbid "\r" and "\n" in the body
    if b"\r" in body or b"\n" in body:
        return None

    # Check crc
    if crc != (b"%04X" % crc16(body)):
        return None

    try:
        decoded = body.decode("ascii")
    except UnicodeError:
        return None

    return decoded


if __name__ == "__main__":
    # seal("2 0 5 hello") => b"2 0 5 hello*EBDC\r\n"
    str = "2 0 5 hello"
    sealed = seal(str)
    print(sealed)

    unsealed = unseal(sealed)
    print(unsealed)

    print(str == unsealed)
