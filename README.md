# Project: mini-docker
Authors (team): Max Kutsenko, Zahar Kohut, Serhii Dmytryshyn, Maxym Kutsenko<br>
## Prerequisites

<mark>g++, cmake</mark>

### Compilation

```
./compile.sh
```

### Usage

Create container
```
./bin/mydocker create <config_path>
```

Delete container and cgroup
```
./bin/mydocker delete <container_id>
```

Up server
```
nx localhost 8080
```

Stop server
```
./bin/mydocker shutdow
```

List of all active containers
```
./bin/mydocker list
```

Run container by id
```
./bin/mydocker run <container_id>
```

Stop container by id
```
./bin/mydocker stop <container_id>
```
