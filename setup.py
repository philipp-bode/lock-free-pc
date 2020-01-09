import os
import sys
import subprocess

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=''):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    def run(self):
        try:
            subprocess.check_output(['cmake', '--version'])
        except OSError:
            raise RuntimeError(
                'CMake must be installed to build the following extensions: ' +
                ', '.join(e.name for e in self.extensions)
            )

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(
            self.get_ext_fullpath(ext.name)))
        cmake_args = [
            f'-DCMAKE_LIBRARY_OUTPUT_DIRECTORY={extdir}',
            f'-DPYTHON_EXECUTABLE={sys.executable}',
        ]

        cfg = 'Debug' if self.debug else 'Release'
        build_args = ['--config', cfg]

        cmake_args += [f'-DCMAKE_BUILD_TYPE={cfg}']
        build_args += ['--', '-j2']

        env = os.environ.copy()
        env['CXXFLAGS'] = (
            f'{env.get("CXXFLAGS", "")}'
            f' -DVERSION_INFO=\\"{self.distribution.get_version()}\\"'
        )
        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(
            ['cmake', ext.sourcedir] + cmake_args,
            cwd=self.build_temp, env=env
        )
        subprocess.check_call(
            ['cmake', '--build', '.'] + build_args, cwd=self.build_temp)


setup(
    name='lockfreepc',
    version='0.0.1',
    author='Philipp Bode',
    author_email='philipp.bode@student.hpi.de',
    url='http://github.com/philipp-bode/lock-free-pc',
    description='A wrapper for highly parallelized causal structure learning.',
    long_description='',
    packages=[
        'lockfreepc'
    ],
    ext_modules=[CMakeExtension('_lockfreepc')],
    cmdclass=dict(build_ext=CMakeBuild),
    zip_safe=False,
    install_requires=[
       'pandas==0.25.3',
       'networkx==2.4',
    ],
)
