from crc16 import crc16

def seal(body: str) -> bytes:
    """
    Encode the body to bytes: b"<body>*XXXX\r\n"
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


if __name__ == "__main__":
    # seal("2 0 5 hello") => b"2 0 5 hello*EBDC\r\n"
    print(seal("2 0 5 hello"))