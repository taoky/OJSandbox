#!/usr/bin/env python3

import docker
from pathlib import Path
client = docker.from_env()

# build ojs-alpha
client.images.build(path="../Sandbox_docker", tag="ojs-alpha")
print("Build ojs-alpha done.")

# run container
volume_path = str(Path("../volume").resolve())
volumes = {volume_path: {"bind": "/volume", "mode": "rw"}}
container = client.containers.run("ojs-alpha", detach=True, volumes=volumes, stdin_open=True)
print("Container is running...")
print(container)

# use backend
res = container.exec_run(["/Backend/main", "-i", "/dev/null", "-o", "/dev/null", "--", "/volume/execTest0"], stderr=False)
print(res[0]) # exit_code
print(res[1]) # output (stdout)

# stop container
container.stop()