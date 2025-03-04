from scripts import globals

def Install():
    globals.CheckCommands()
    
    if globals.ProcessGitSafeDir() == False:
        exit(2)

    if globals.ProcessGitSubmodules() == False:
        exit(3)

    if globals.ProcessBoost() == False:
        exit(4)

    globals.prints.frameMessage("SUCCESS INSTALL!")

if __name__ == '__main__':
    Install()