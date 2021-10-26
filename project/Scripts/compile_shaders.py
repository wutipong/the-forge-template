import argparse
import sys
from pathlib import Path
import subprocess
import os


def initArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--destination',
                        help='output directory', required=True)
    parser.add_argument('-b', '--binaryDestination',
                        help='output directory', required=True)
    parser.add_argument('-s', '--solutionDir',
                        help='solution directory', required=True)

    parser.add_argument('-w', '--winSdkBin',
                        help='Windows SDK Bin path', required=True)
    parser.add_argument('input', help='input directory')
    args = parser.parse_args()

    return args


languages = "DIRECT3D11 DIRECT3D12 VULKAN"
args = initArgs()
fslPath = Path(args.solutionDir,
               'the-forge/Common_3/Tools/ForgeShadingLanguage/fsl.py')

files = Path(args.input).glob('**/*.fsl')
python = sys.executable

windowsSdKBinPathList = args.winSdkBin.split(';')
windowsSdkBinPath = ''

for sdkPath in windowsSdKBinPathList:
    path = Path(sdkPath, 'fxc.exe')
    if path.exists():
        windowsSdkBinPath = sdkPath
        break

for file in files:
    relatedPath = file.relative_to(args.input)
    destination = Path(args.destination, relatedPath).parent

    subprocess.run([python, fslPath, '-d', destination, '-b', args.binaryDestination, '--compile',
                    '-l', languages, file], env=dict(os.environ, FSL_COMPILER_FXC=windowsSdkBinPath))
