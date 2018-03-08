#!/usr/bin/env python3

import sys
import os
import stat
import json

configFileName = 'config.json'
installScriptFileName = 'install.sh'

class OJConfig:
    def __init__(self):
        self.maxProcess = 128
        self.outputSize = 16
        self.compilerRam = 256
        self.compilerTime = 5000
        self.memBonus = {
            '.java': 1.50000
        }
        self.timeBonus = {
            '.java': 1.50000
        }
        self.enabledLang = ['.c', '.cpp', '.py']

    def to_dict(self):
        return {
            'max-processes': self.maxProcess,
            'output-size': self.outputSize,
            'compile-memory': self.compilerRam,
            'compile-time': self.compilerTime,
            'mem-bonus': dict(self.memBonus),
            'time-bonus': dict(self.timeBonus),
            'enabled': list(self.enabledLang),
        }

    def dump(self, fp):
        return json.dump(self.to_dict(), fp)

def createInstallScript(config, fn):
    aptInstall = ['libseccomp-dev', 'libcap-dev']
    script = ['#!/bin/sh', '', 'sudo apt update']
    if '.java' in config.enabledLang:
        aptInstall.append('openjdk-8-jdk')
    if '.pas' in config.enabledLang:
        aptInstall.append('fp-compiler')
    script.append('sudo apt-get install -y ' + ' '.join(aptInstall))
    with open(fn, 'w') as f:
        f.write('\n'.join(script))
    os.chmod(fn, stat.S_IRWXU | stat.S_IRGRP | stat.S_IXGRP | stat.S_IROTH | stat.S_IXOTH)

def safeInput(dType=int, validate=lambda x: True, default=None):
    try:
        d = dType(input())
        if not validate(d):
            raise ValueError
        return d
    except ValueError:
        return default

def promptConfig(config):
    print('Maximum processes: [128] ', end='')
    config.maxProcesses = safeInput(int, lambda x: x > 1, 128)
    print('Maximum output file size in MiB: [16] ', end='')
    config.outputSize = safeInput(int, lambda x: x > 1, 16)
    print('Compiler time in ms: [5000] ', end='')
    config.compilerTime = safeInput(int, lambda x: x > 1, 5000)
    print('Compiler memory in MiB: [256] ', end='')
    config.compilerRam = safeInput(int, lambda x: x > 1, 256)
    print('Enable Java (require extra packages from APT): [Leave blank to set to False] ', end='')
    config.enableJava = safeInput(bool, default=False)
    if config.enableJava:
        config.enabledLang.append('.java')
    print('Enable Pascal (require extra packages from APT): [Leave blank to set to False] ', end='')
    config.enablePascal = safeInput(bool, default=False)
    if config.enablePascal:
        config.enabledLang.append('.pas')

if __name__ == '__main__':
    config = OJConfig()

    for opt in sys.argv[1:]:
        opt = opt.split('=')
        opt, vals = opt[0], opt[1:]
        if opt in ('autoconfig', 'writeconfig'):
            with open(configFileName, 'w') as f:
                config.dump(f)
            createInstallScript(config, installScriptFileName)
            exit(0)
        elif opt in ('compile-mem', 'compile-memory'):
            config.compilerRam = int(vals[0])
        elif opt in ('compile-time'):
            config.compilerTime = int(vals[0])
        elif opt in ('max-process'):
            config.maxProcess = int(vals[0])
        elif opt in ('output-size'):
            config.outputSize = int(vals[0])
        elif opt in ('enable-java'):
            if '.java' not in config.enabledLang:
                config.enabledLang.append('.java')
        elif opt in ('enable-pas', 'enable-pascal'):
            if '.pas' not in config.enabledLang:
                config.enabledLang.append('.pas')
        else:
            print('Unknown action "{}"'.format(opt))
            exit(1)

    promptConfig(config)
    with open(configFileName, 'w') as f:
        config.dump(f)
    createInstallScript(config, installScriptFileName)
    print('Created installer script "{}". Please run it to install dependencies.'.format(installScriptFileName))
