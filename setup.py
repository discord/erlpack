from setuptools import setup

have_cython = False
try:
    from Cython.Distutils import build_ext, Extension
    have_cython = True

except ImportError:
    from setuptools.command.build_ext import build_ext
    from setuptools.extension import Extension

if have_cython:
    packer = Extension(
        "erlpack._packer",
        cython_cplus=True,
        extra_compile_args=['-O3'],
        sources=["py/erlpack/_packer.pyx"]
    )
    unpacker = Extension(
        "erlpack._unpacker",
        cython_cplus=True,
        extra_compile_args=['-O3'],
        sources=["py/erlpack/_unpacker.pyx"]
    )
else:
    packer = Extension('erlpack._packer', sources=['py/erlpack/_packer.cpp'], extra_compile_args=['-O3'])
    unpacker = Extension('erlpack._unpacker', sources=['py/erlpack/_unpacker.cpp'], extra_compile_args=['-O3'])

ext_modules = [packer, unpacker]

setup(
    name='erlpack',
    version='0.3.2',
    author='Jake Heinz',
    author_email='jh@hammerandchisel.com',
    url="http://github.com/hammerandchisel/erlpack",
    description='A high performance erlang term encoder for Python.',
    license='Apache 2.0',
    cmdclass={'build_ext': build_ext},
    zip_safe=False,
    package_dir={'': 'py'},
    packages=['erlpack'],
    ext_modules=ext_modules,
    setup_requires=['pytest-runner'],
    tests_require=['pytest'],
)
