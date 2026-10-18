"""Digests of the runtime observed by directly executed checks."""

import json
import os
import re
import subprocess

from affected import GROUPS, canonical, digest, require


VERSION = 1
PROFILE = "ci-environment-key-boost-v1"
AMBIENT_KEYS = ("CXX", "CXXFLAGS", "CPATH", "CPLUS_INCLUDE_PATH", "LIBRARY_PATH", "LD_LIBRARY_PATH")
BOOST_PACKAGE = r"libboost[0-9]+(?:\.[0-9]+)+-dev"


def parsed_keys(environment_keys):
    """Hash exact ci.environment_key values without exposing their contents."""
    require(isinstance(environment_keys, dict) and set(environment_keys) == set(GROUPS),
            "Incomplete runtime groups")
    values = {}
    for group in GROUPS:
        require(isinstance(environment_keys[group], str), "Invalid runtime context")
        try:
            value = json.loads(environment_keys[group])
        except ValueError:
            raise ValueError("Invalid runtime context") from None
        require(isinstance(value, dict), "Invalid runtime context")
        values[group] = value
    return values


def runtime_platform(values):
    platforms = []
    for value in values.values():
        toolchain = value.get("toolchain", value)
        require(isinstance(toolchain, dict), "Invalid runtime toolchain")
        platform = toolchain.get("platform")
        require(isinstance(platform, list) and len(platform) == 3
                and all(isinstance(value, str) and value for value in platform), "Incomplete runtime platform")
        platforms.append(platform[0])
    require(all(value == platforms[0] for value in platforms)
            and platforms[0] in ("Linux", "Darwin"), "Unsupported runtime platform")
    return platforms[0]


def installed_boost(query=None):
    """Bind both the Debian Boost metapackage and its installed header package."""
    query = query or subprocess.check_output

    def package(name):
        try:
            fields = query(["dpkg-query", "--show", "--showformat=${db:Status-Status}\t${Version}\t${Depends}\n",
                            name], text=True, stderr=subprocess.DEVNULL).rstrip("\n").split("\t")
        except (OSError, subprocess.CalledProcessError):
            raise ValueError("Installed Boost identity unavailable") from None
        require(len(fields) == 3 and fields[0] == "installed"
                and re.fullmatch(r"[0-9A-Za-z.+:~_-]+", fields[1]), "Installed Boost identity unavailable")
        return fields[1], fields[2]

    version, dependency = package("libboost-dev")
    match = re.fullmatch("(" + BOOST_PACKAGE + r")(?: \([^()\r\n]+\))?", dependency)
    require(match is not None, "Installed Boost dependency unavailable")
    name = match[1]
    return {"libboost-dev": version, name: package(name)[0]}


def runtime_context(environment_keys, environment, *, system_packages):
    """Hash observed runtime and package versions without exposing raw values."""
    values = parsed_keys(environment_keys)
    platform = runtime_platform(values)
    require(isinstance(system_packages, dict), "Invalid system package identity")
    if platform == "Linux":
        require(len(system_packages) == 2 and "libboost-dev" in system_packages
                and any(re.fullmatch(BOOST_PACKAGE, name) for name in system_packages)
                and all(isinstance(value, str) and re.fullmatch(r"[0-9A-Za-z.+:~_-]+", value)
                        for value in system_packages.values()), "Incomplete installed Boost identity")
    else:
        require(not system_packages, "Unexpected system package identity")
    groups = {group: {"runtime_sha256": digest(canonical({
        "environment_key": values[group], "system_packages": system_packages}))} for group in GROUPS}
    ambient = {}
    for key in AMBIENT_KEYS:
        present = key in environment
        value = environment[key] if present else None
        require(not present or isinstance(value, str), "Invalid ambient context")
        ambient[key] = {"present": present, "sha256": digest(value.encode()) if present else None}
    return {"version": VERSION, "profile": PROFILE, "groups": groups, "ambient": ambient,
            "system_packages_sha256": digest(canonical(system_packages))}


def collect_runtime(*, package_query=None):
    """Collect only the current runtime; historical contexts cannot be inferred."""
    from ci import environment_key

    keys = environment_key()
    packages = installed_boost(package_query) if runtime_platform(parsed_keys(keys)) == "Linux" else {}
    return runtime_context(keys, os.environ, system_packages=packages)
