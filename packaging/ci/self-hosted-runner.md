# Running the packaging jobs on your own runner

The deb, rpm and AppImage jobs run anywhere, including GitLab's shared runners.
The Flatpak job does not, and this explains why and how to host it.

## Why the Flatpak job needs more than a normal job container

`flatpak-builder` sandboxes every build step with bubblewrap, so it has to
create a mount + user namespace *inside* the job container. Unprivileged job
containers have that capability dropped. Measured on a rootless podman host:

| container settings                        | `bwrap --dev-bind / / true` |
|-------------------------------------------|-----------------------------|
| default                                    | `Operation not permitted`  |
| `--security-opt seccomp=unconfined`        | `Operation not permitted`  |
| `--security-opt unmask=ALL`                | `Operation not permitted`  |
| `--cap-add=SYS_ADMIN`                      | works                      |
| `--privileged`                             | works                      |

Note `unshare -U` succeeds in all of them -- plain user namespaces are fine.
It is the combination bwrap needs that is blocked, and only the capability
restores it. `SYS_ADMIN` alone is sufficient; `privileged` is not required.

## Why SYS_ADMIN is acceptable on a rootless runner (and not otherwise)

On a **rootless** podman/docker socket, container root maps to the runner's own
unprivileged user:

    uid_map:  0 1000 1        # "root" in the container is uid 1000 outside

so `SYS_ADMIN` is scoped to that user namespace. It permits the nested sandbox
and still cannot mount host filesystems (`mount` fails even with the capability
granted) or reach host devices (`/dev/sda1` does not exist in the container).
Its ceiling is what the runner's own user account can already do.

On a **rootful** daemon this is not true -- container root is host root, and
`SYS_ADMIN` there is close to handing over the machine. Use a rootless runner.

## Setup

Rootless podman, socket already provided by systemd:

    systemctl --user enable --now podman.socket

Register a runner (GitLab -> project -> Settings -> CI/CD -> Runners -> New
project runner), then run it against that socket:

    podman run -d --name gitlab-runner --restart=always \
      -v /run/user/$(id -u)/podman/podman.sock:/var/run/docker.sock \
      -v gitlab-runner-config:/etc/gitlab-runner \
      gitlab/gitlab-runner:latest

    podman exec gitlab-runner gitlab-runner register \
      --non-interactive --url https://gitlab.com \
      --token "$RUNNER_TOKEN" \
      --executor docker --docker-image debian:testing \
      --docker-volumes /var/run/docker.sock:/var/run/docker.sock \
      --tag-list flatpak

Then add the capability in `/etc/gitlab-runner/config.toml`:

    [runners.docker]
      cap_add = ["SYS_ADMIN"]

## For a public repository: restrict who can trigger it

The isolation above bounds what a job can *do*; it does not decide *whose* code
runs. On a public project, mark the runner **protected** so it only picks up
pipelines on protected branches and tags -- merge requests from forks then
never reach it -- and give it a tag so only the job that needs it is routed
there, leaving the deb/rpm/AppImage jobs on shared runners.
