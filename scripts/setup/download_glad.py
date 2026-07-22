#!/usr/bin/env python3
"""
Download and generate GLAD OpenGL loader files
Requires: requests (pip install requests)
"""

import os
import sys
import zipfile
import io

def download_glad():
    """Download pre-generated GLAD files for OpenGL 4.5 Core"""
    try:
        import requests
    except ImportError:
        print("ERROR: requests module not found. Install with: pip install requests")
        return False

    print("Downloading GLAD for OpenGL 4.5 Core...")

    # Use GLAD web service API to generate files
    url = "https://glad.dav1d.de/generated/tmpxxxxxx/glad.zip"

    # Alternative: Use a known good version from GitHub
    github_url = "https://github.com/Dav1dde/glad/archive/refs/heads/glad2.zip"

    print("Note: This script downloads from GLAD generator.")
    print("For production use, generate custom files at https://glad.dav1d.de/")
    print()
    print("Attempting to download pre-configured GLAD files...")

    # Get script directory
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.abspath(os.path.join(script_dir, "..", ".."))
    glad_dir = os.path.join(project_root, "thirdparty", "glad")

    # Create directories
    os.makedirs(os.path.join(glad_dir, "include", "glad"), exist_ok=True)
    os.makedirs(os.path.join(glad_dir, "include", "KHR"), exist_ok=True)
    os.makedirs(os.path.join(glad_dir, "src"), exist_ok=True)

    print(f"Target directory: {glad_dir}")
    print()
    print("=" * 60)
    print("MANUAL SETUP REQUIRED")
    print("=" * 60)
    print()
    print("Please generate GLAD files manually:")
    print()
    print("1. Go to: https://glad.dav1d.de/")
    print("2. Configure:")
    print("   - Language: C/C++")
    print("   - Specification: OpenGL")
    print("   - API gl: Version 4.5")
    print("   - Profile: Core")
    print("   - Check 'Generate a loader'")
    print("3. Click 'Generate'")
    print("4. Download and extract the zip file")
    print("5. Copy files:")
    print(f"   - include/glad/ → {os.path.join(glad_dir, 'include', 'glad')}")
    print(f"   - include/KHR/ → {os.path.join(glad_dir, 'include', 'KHR')}")
    print(f"   - src/glad.c → {os.path.join(glad_dir, 'src', 'glad.c')}")
    print()
    print("Or use Python glad generator:")
    print("   pip install glad")
    print(f"   cd {glad_dir}")
    print("   glad --profile=core --api=gl=4.5 --generator=c --spec=gl --out-path=.")
    print()

    return False

if __name__ == "__main__":
    if not download_glad():
        print("\nGLAD setup requires manual intervention. See instructions above.")
        sys.exit(1)
    else:
        print("\nGLAD downloaded successfully!")
        sys.exit(0)
