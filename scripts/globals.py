import os
import subprocess
from . import prints
from . import installBoost


def CheckCommand(command, url):
    try:
        subprocess.run([command, '--version'], check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        prints.printSuccess(f"'{command}' was found!")
    except (subprocess.CalledProcessError, FileNotFoundError):
        prints.printWarning(f"{command} wasn't found. Install it ({url}) and try again.")
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
        prints.printError("Failed to retrieve Git safe directories.")
        return False

def CheckCommands():
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
                    prints.printError("Was met some errors while executing of git command.")
                    return False
                break
            elif user_input in ["n", "no"]:
                print("The project wasn't add to git safe dirs")
                break
            else:
                print("Invalid input. Please enter Y or N.")
    else:
        prints.printSuccess("The project is inside git safe directory.")

    return True

def ProcessGitSubmodules():
    dependency_folder = "dependencies"
    try:
        dependencies = os.listdir(dependency_folder)
    except FileNotFoundError:
        prints.printError("The folder '{dependency_folder}' does not exist.")
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
                    prints.printError("Was met some error while trying to update git submodules.")
                    return False
                break
            elif user_input in ["n", "no"]:
                print("Skipping git submodule update. You can update manually using 'git submodule update --init --force --remote'.")
                break
            else:
                print("Invalid input. Please enter Y or N.")
    else:
        prints.printSuccess("Git submodules don't need in update.")

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
            if installBoost.Install(PreferedShellExt(), "dependencies") == False:
                return False
            break
        elif user_input in ["n", "no"]:
            print("Skipping auto install of the boost library.")
            break
        else:
            print("Invalid input. Please enter Y or N.")

    return True
