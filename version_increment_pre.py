import datetime
import os
import requests # type: ignore


Import("env") # type: ignore


VERSION_FILE = 'version'
VERSION_HEADER = 'Version.h'


version_url = "http://192.168.0.18:3000/prout-o-metre/firmwares/new-build-version"
resp = requests.get(version_url)
resp.raise_for_status()
VERSION = resp.json()["version"]


with open(VERSION_FILE, 'w+') as FILE:
    FILE.write(VERSION)
    print('Build number: {}'.format(VERSION))

HEADER_FILE = """
// AUTO GENERATED FILE, DO NOT EDIT
#ifndef VERSION
    #define VERSION "{}"
#endif
#ifndef BUILD_TIMESTAMP
    #define BUILD_TIMESTAMP "{}"
#endif
""".format(VERSION, datetime.datetime.now())

VERSION_HEADER = "include" + os.sep + VERSION_HEADER

with open(VERSION_HEADER, 'w+') as FILE:
    FILE.write(HEADER_FILE)

