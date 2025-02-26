import os
import subprocess
import sys
import scripts.installBoost

# Function to check if the terminal supports colors
def SupportsColor():
    return sys.stdout.isatty() and (os.environ.get("TERM") in ["xterm-color", "xterm-256color", "screen-256color"])

# ANSI escape code for red color
if SupportsColor():
    RED = "\033[91m"
    GREEN = "\033[92m"
    RESET = "\033[0m"
else:
    RED = ""
    GREEN = ""
    RESET = ""

# Function to check if a command is available in the system's PATH
def CheckCommand(command, url):
    try:
        subprocess.run([command, '--version'], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"{GREEN}Success:{RESET} '{command}' was found!")
    except (subprocess.CalledProcessError, FileNotFoundError):
        print(f"{RED}Error:{RESET} {command} wasn't found. Install it ({url}) and try again.")
        return False
    return True


def IsGitSafeDirectory(path):
    try:
        # Get the list of global safe directories
        result = subprocess.run(
            ["git", "config", "--global", "--get-all", "safe.directory"],
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        
        # Normalize the path for comparison
        normalized_path = os.path.abspath(path)

        # Check if the directory is in the list
        safe_directories = result.stdout.splitlines()
        return normalized_path in safe_directories

    except subprocess.CalledProcessError:
        print(f"{RED}Error:{RESET} Failed to retrieve Git safe directories.")
        return False

def CheckCommands():
    if CheckCommand("cppcheck", "https://sourceforge.net/projects/cppcheck/#download") == False:
        return False

    if CheckCommand("cmake", "https://cmake.org/download/") == False:
        return False

    if CheckCommand("git", "https://git-scm.com/downloads") == False:
        return False
    
    return True

def ProcessGitSafeDir():
    if IsGitSafeDirectory(os.getcwd()) == False:
        while True:
            user_input = input("This project wasn't added to safe dir inside your global Git config. Do thath automatically? [Y/n]: ").strip().lower()
            if user_input in ["y", "yes", ""]:
                git_config_command = ["git", "config", "--global", "--add", "safe.directory", os.getcwd()]
                try:
                    subprocess.run(git_config_command, check=True)
                except (subprocess.CalledProcessError, FileNotFoundError):
                    print(f"{RED}Error:{RESET} Was met some errors while executing of git command.")
                    return False
                break
            elif user_input in ["n", "no"]:
                print("The project wasn't add to git safe dirs")
                break
            else:
                print("Invalid input. Please enter Y or N.")
    
    return True

def ProcessGitSubmodules():
    dependency_folder = "dependencies"
    try:
        dependencies = os.listdir(dependency_folder)
    except FileNotFoundError:
        print(f"{RED}Error:{RESET} The folder '{dependency_folder}' does not exist.")
        return False

    count = len(dependencies)

    if count == 0:
        while True:
            user_input = input("No dependencies found. Do you want to update git submodules? [Y/n]: ").strip().lower()
            if user_input in ["y", "yes", ""]:
                git_submodule_command = ["git", "submodule", "update", "--init", "--force", "--remote"]
                try:
                    subprocess.run(git_submodule_command, check=True)
                    print("Git submodules updated successfully.")
                except subprocess.CalledProcessError:
                    print(f"{RED}Error:{RESET} Was met some error while trying to update git submodules.")
                    return False
                break
            elif user_input in ["n", "no"]:
                print("Skipping git submodule update. You can update manually using 'git submodule update --init --force --remote'.")
                break
            else:
                print("Invalid input. Please enter Y or N.")

    return True

def PreferedShellExt():
    while True:
        user_input = input("What shell scripts do you want to ues: \n\t(1) .sh\n\t(2) .bat\n> ").strip().lower()
        if user_input in [ "1", "sh", ".sh" ]:
            return "sh"
        elif user_input in [ "2", "bat", ".bat" ]:
            return "bat"
        else:
            print("Invalid input. Please enter correct answer")

def ProcessBoost():
    while True:
        user_input = input("Do you want to install boost? [Y/n]: ").strip().lower()
        if user_input in ["y", "yes", ""]:
            if scripts.installBoost.Install(PreferedShellExt()) == False:
                return False
            break
        elif user_input in ["n", "no"]:
            print("Skipping auto install of the boost library.")
            break
        else:
            print("Invalid input. Please enter Y or N.")

    return True

def SuccessMessage():
    print(f"{GREEN}")
    width = 50
    printStr = "SUCCESS INSTALL"
    if len(printStr) % 2 != 0:
        printStr += " "

    strGap = int((width - len(printStr)) / 2)
    print("╔" + "═" * width + "╗")
    print("║" + " " * strGap + printStr + " " * strGap + "║")
    print("╚" + "═" * width + "╝" + f"{RESET}")

SuccessMessage()
exit(0)

if CheckCommands() == False:
    exit(1)

if ProcessGitSafeDir() == False:
    exit(2)

if ProcessGitSubmodules() == False:
    exit(3)

if ProcessBoost() == False:
    exit(4)

SuccessMessage()
