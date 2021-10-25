import os
import argparse
import sys
import string
import subprocess


def initArgs():
    parser = argparse.ArgumentParser()
    parser.add_argument('-d', '--destination',
                        help='output directory', required=True)
    parser.add_argument('-b', '--binaryDestination',
                        help='output directory', required=True)
    parser.add_argument('-s', '--solutionDir',
                        help='solution directory', required=True)
    parser.add_argument('fsl_input', help='fsl file to generate from')
    args = parser.parse_args()

    return args


languages = "DIRECT3D11 DIRECT3D12 VULKAN"
args = initArgs()
fslPath = os.path.normpath(
    os.path.join(
        args.solutionDir,
        'the-forge/Common_3/Tools/ForgeShadingLanguage/fsl.py'
    )
)

files = os.listdir(args.fsl_input)
python = ''.join(['"', sys.executable, '"'])
print(python)
for file in files:
    filePath = os.path.normpath(
        os.path.join(args.fsl_input, file)
    )

    subprocess.run([sys.executable, fslPath, '-d', args.destination, '-b', args.binaryDestination, '--compile',
                    '-l', languages, filePath])
