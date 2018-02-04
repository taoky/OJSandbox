#!/usr/bin/env python3

import docker
client = docker.from_env()

# build ojs-alpha
client.images.build(path="../Sandbox_docker", tag="ojs-alpha")

# run container
volumes = {"../volume": {"bind": "/volume", "mode": "rw"}}
container = client.containers.run("ojs-alpha", detach=True, volumes=volumes)

# use backend
res = container.exec_run(["Backend/main", "-i", "/dev/null", "-o", "/dev/null", "--", "/volume/execTest0"], stderr=False)
print(res[0]) # exit_code
print(res[1]) # output (stdout)

# to be continued...