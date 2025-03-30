import os
import sys

def create_dpkg_file(path: str) -> int:
    print("DPKG package doesn't supported yet.")
    return -1

def create_pkg_file(path: str) -> int:
    print("PKG package doesn't supported yet.")
    return -1

def create_mpkg_file(path: str) -> int:
    print("MPKG package doesn't supported yet.")
    return -1

def create_exe_file(path: str) -> int:
    print("EXE package doesn't supported yet.")
    return -1

def create_packages() -> int:
    results = {
            'dpkg': {
                'status': create_dpkg_file('./')
            },
            'pkg': {
                'status': create_pkg_file('./')
            },
            'mpkg': {
                'status': create_mpkg_file('./')
            },
            'exe': {
                'status': create_exe_file('./')
            }
    }
    res = -1
    for key in results.keys():
        res += results[key]['status']
    return 0 if res == 0 else -1

if __name__ == '__main__':
    if create_packages() == 0:
        print("Done")
    else:
        print("Done with errors")

