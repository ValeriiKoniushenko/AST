import os
import sys
import shutil
import subprocess
import urllib.request
from scripts.prints import printError, printSuccess

def MakeScriptsExecutable(root_dir, ext):
    for dirpath, _, filenames in os.walk(root_dir):
        for filename in filenames:
            if filename.endswith(f".{ext}"):
                script_path = os.path.join(dirpath, filename)
                try:
                    subprocess.run(["chmod", "+x", script_path])
                except (subprocess.CalledProcessError, FileNotFoundError):
                    printError(f"Was met an error while trying to apply chmod +x to .{ext} files.")

def Install(preferedShellExt, dependenciesDir):
    # Change to script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    os.chdir(script_dir)

    # Change to dependencies directory
    dependency_folder = os.path.join("..", dependenciesDir)
    if not os.path.exists(dependency_folder):
        printError(f"Folder '{dependenciesDir}' not found")
        return False

    os.chdir(dependency_folder)

    # Download and extract boost if not exists
    boost_folder = "boost-1.86.0"

    if not os.path.exists(boost_folder):
        print("Downloading Boost...")
        boost_zip = "boost.zip"
        boost_url = "https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.zip"
        urllib.request.urlretrieve(boost_url, boost_zip)
        printSuccess("Boost was downloaded")
        
        print("Extracting Boost...")
        shutil.unpack_archive(boost_zip, ".")
        os.remove(boost_zip)
        printSuccess("Boost was unpacked")

    # Change to boost directory
    if not os.path.exists(boost_folder):
        printError("Can't find the unzipped boost directory")
        return False

    os.chdir(boost_folder)

    if sys.platform.startswith("linux"):
        MakeScriptsExecutable(os.getcwd(), preferedShellExt)        
        printSuccess(f"All boost/**/*.{preferedShellExt} was recursived marked as executable")

    installBootstrap = False
    while True:
        user_input = input(f"Do you want to run bootstrap.{preferedShellExt}? [Y/n]").strip().lower()
        if user_input in [ "y", "yes", "" ]:
            installBootstrap = True
            break
        elif user_input in [ "n", "no" ]:
            installBootstrap = False
            break
        else:
            printError("Invalid input. Please enter correct answer")

    if installBootstrap == True:
        subprocess.call(f"./bootstrap.{preferedShellExt}", shell=True)
        printSuccess("Bootstrap has finished!")

    installB2 = False
    while True:
        user_input = input(f"Do you want to run b2.{preferedShellExt}? [Y/n]").strip().lower()
        if user_input in [ "y", "yes", "" ]:
            installB2 = True
            break
        elif user_input in [ "n", "no" ]:
            installB2 = False
            break
        else:
            printError("Invalid input. Please enter correct answer")

    if installB2:
        subprocess.call("./b2 install", shell=True)
        printSuccess("b2 has finished!")

    return True
