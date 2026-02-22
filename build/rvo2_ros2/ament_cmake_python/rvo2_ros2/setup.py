from setuptools import find_packages
from setuptools import setup

setup(
    name='rvo2_ros2',
    version='0.0.1',
    packages=find_packages(
        include=('rvo2_ros2', 'rvo2_ros2.*')),
)
