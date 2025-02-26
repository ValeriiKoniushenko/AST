import os
import sys

def SupportsColor():
    return sys.stdout.isatty() and (os.environ.get("TERM") in ["xterm-color", "xterm-256color", "screen-256color"])

if SupportsColor():
    RED = "\033[91m"
    GREEN = "\033[92m"
    RESET = "\033[0m"
else:
    RED = ""
    GREEN = ""
    RESET = ""

def printError(msg):
    print(f"{RED}Error:{RESET} " + msg)

def printSuccess(msg):
    print(f"{GREEN}Success:{RESET} " + msg)

def frameMessage(msg, width = 50):
    msg = str(msg).upper().strip()
    if len(msg) % 2 != 0:
        msg += " "
    strGap = int((width - len(msg)) / 2)

    print(f"{GREEN}")
    print("╔" + "═" * width + "╗")
    print("║" + " " * strGap + msg + " " * strGap + "║")
    print("╚" + "═" * width + "╝" + f"{RESET}")

