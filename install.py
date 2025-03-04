try:
    from .scripts.globals import *
except ImportError:
    from scripts.globals import *
    
def Install():
    CheckCommands()
    
    if ProcessGitSafeDir() == False:
        exit(2)

    if ProcessGitSubmodules() == False:
        exit(3)

    if ProcessBoost() == False:
        exit(4)

    prints.frameMessage("SUCCESS INSTALL!")

if __name__ == '__main__':
    Install()