#!/usr/bin/env python3

"""
Use cURL and ImageMagick to download and extract a model zip-file.

Downloads the URL (which must be a zip-file) and extracts it to
model/<name>.obj and model/<name>.bmp.

If the zip-file contains a README, it is printed to standard output.
If the zip-file contains a PNG file, it is converted to BMP using
the convert command from ImageMagick.

If the zip-file contains multiple .obj or image files,
the last one is used.
"""

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
import zipfile


parser = argparse.ArgumentParser(
    description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
)
parser.add_argument("url")
parser.add_argument("name")


def main():
    args = parser.parse_args()
    with tempfile.NamedTemporaryFile() as fp:
        subprocess.check_call(("curl", "-Lo", fp.name, args.url))
        zf = zipfile.ZipFile(fp.name)
        for zi in zf.infolist():  # type: zipfile.ZipInfo
            if zi.filename.endswith(".obj"):
                targetpath = os.path.join("model", args.name + ".obj")
                with zf.open(zi) as source, open(targetpath, "wb") as target:
                    shutil.copyfileobj(source, target)
            elif "readme" in zi.filename.lower():
                with zf.open(zi) as source:
                    shutil.copyfileobj(source, sys.stdout.buffer)
                print("")
            elif zi.filename.endswith(".bmp"):
                targetpath = os.path.join("model", args.name + ".bmp")
                with zf.open(zi) as source, open(targetpath, "wb") as target:
                    shutil.copyfileobj(source, target)
            elif zi.filename.endswith(".png"):
                targetpath = os.path.join("model", args.name + ".bmp")
                target = open(targetpath, "wb")
                with target, zf.open(zi) as source:
                    proc = subprocess.Popen(
                        ("convert", "png:-", "bmp:-"),
                        stdin=subprocess.PIPE,
                        stdout=target,
                    )
                    with proc:
                        shutil.copyfileobj(source, proc.stdin)
                        proc.stdin.close()
                        proc.wait()


if __name__ == "__main__":
    main()
