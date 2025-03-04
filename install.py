from scripts.globals import *

if ProcessGitSafeDir() == False:
    exit(2)

if ProcessGitSubmodules() == False:
    exit(3)

if ProcessBoost() == False:
    exit(4)

prints.frameMessage("SUCCESS INSTALL!")
