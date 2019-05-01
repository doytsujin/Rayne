import sys
import json
import os
import errno
import subprocess
import platform

def main():
    if len(sys.argv) < 3:
        print 'Specify shader json file followed by output directory path [and optional resource folder relative path] as parameters'

    with open(sys.argv[1], 'r') as sourceJsonData:
        sourceJson = json.load(sourceJsonData)

    if not sourceJson:
        print('No data found.')
        return

    outFormats = ['dxil', 'spirv', 'metal'];
    #outFormats = ['spirv', 'metal'];

    outDirName = sys.argv[2]
    resourceRelativePath = sys.argv[3]
    if not os.path.exists(outDirName):
        try:
            os.makedirs(outDirName)
        except OSError as exc: # Guard against race condition
            if exc.errno != errno.EEXIST:
                raise

    if not resourceRelativePath:
        resourceRelativePath = ''

    jsonDirectory, jsonFileName = os.path.split(sys.argv[1])
    destinationJson = list()

    shaderConductorCmdPath = os.path.dirname(sys.argv[0])
    if platform.system() == 'Darwin':
        shaderConductorCmdPath = os.path.join(shaderConductorCmdPath, 'Vendor/ShaderConductor/Build/ninja-osx-clang-x64-Release/Bin/ShaderConductorCmd')
    else:
        print 'Script needs to be updated with ShaderConductor path for platform: ' + platform.system()
        return

    print shaderConductorCmdPath

    for shaderFile in sourceJson:
        if not 'file' in shaderFile or not 'shaders' in shaderFile:
            continue

        sourceFile = shaderFile['file']
        shaders = shaderFile['shaders']

        filePath, extension = os.path.splitext(sourceFile)
        filePath, fileName = os.path.split(filePath)
        sourceFile = os.path.join(jsonDirectory, sourceFile)

        for shader in shaders:
            if not 'name' in shader or not 'type' in shader:
                continue

            if shader['type'] == 'vertex':
                shaderType = 'vs'
            elif shader['type'] == 'fragment':
                shaderType = 'ps'

            shaderOptions = None
            if 'signature' in shader:
                shaderSignature = shader['signature']
                if 'options' in shaderSignature:
                    shaderOptions = shaderSignature['options']

            entryName = shader['name']

            destinationShaderList = list()
            destinationShader = dict()
            destinationShader['type'] = shader['type']
            destinationShader['name'] = entryName
            if shaderSignature:
                destinationShader['signature'] = shaderSignature;
            destinationShaderList.append(destinationShader)

            destinationShaderFile = dict()
            destinationShaderFile['shaders'] = destinationShaderList

            permutations = list()
            if shaderOptions:
                permutationCount = 2**len(shaderOptions)
                for i in range(0, permutationCount):
                    permutation = list()
                    for n, option in enumerate(shaderOptions):
                        permutation.append('-D')
                        permutationValue = '0'
                        if(i & (1 << n)) != 0:
                            permutationValue = '1'
                        permutation.append(option + '=' + permutationValue)
                    permutations.append(permutation)
            else:
                permutations.append(list())

            for outFormat in outFormats:
                outFile = os.path.join(outDirName, fileName + '.' + shaderType + '.' + outFormat)

                if outFormat == 'dxil':
                    compilerOutFormat = 'dxil'
                    destinationShaderFile['file~d3d12'] = os.path.join(resourceRelativePath, fileName + '.' + shaderType + '.' + outFormat)
                elif outFormat == 'spirv':
                    compilerOutFormat = 'spirv'
                    destinationShaderFile['file~vulkan'] = os.path.join(resourceRelativePath, fileName + '.' + shaderType + '.' + outFormat)
                elif outFormat == 'metal':
                    compilerOutFormat = 'msl_macos'
                    destinationShaderFile['file~metal'] = os.path.join(resourceRelativePath, fileName + '.' + shaderType + '.metallib')

                for permutationCounter, permutation in enumerate(permutations):
                    permutationOutFile = os.path.join(outDirName, fileName + '.' + shaderType + '.' + str(permutationCounter) + '.' + outFormat)
                    parameterList = [shaderConductorCmdPath, '-I', sourceFile, '-O', permutationOutFile, '-E', entryName, '-S', shaderType, '-T', compilerOutFormat]
                    if len(permutation) > 0:
                        parameterList.extend(permutation)
                    subprocess.call(parameterList)

                    if outFormat == 'metal' and platform.system() == 'Darwin':
                        bitcodeOutFile = permutationOutFile + '.air'
                        libOutFile = os.path.join(outDirName, fileName + '.' + shaderType + '.' + str(permutationCounter) + '.metallib')
                        subprocess.call(['xcrun', '-sdk', 'macosx', 'metal', '-c', permutationOutFile, '-o', bitcodeOutFile])
                        subprocess.call(['xcrun', '-sdk', 'macosx', 'metallib', bitcodeOutFile, '-o', libOutFile])
                        os.remove(permutationOutFile)
                        os.remove(bitcodeOutFile)

            destinationJson.append(destinationShaderFile)

        with open(os.path.join(outDirName, 'Shaders.json'), 'w') as destinationJsonData:
            json.dump(destinationJson, destinationJsonData, indent=4, sort_keys=True)

if __name__ == '__main__':
    main()

