# Project: mini-docker
Authors (team): Zahar Kohut, Serhii Dmytryshyn, Maxym Kutsenko, Bohdan Ozarko<br>

## Prerequisites

g++, cmake, boost

### Compilation

```
./compile.sh
```

### Usage

Start server:
```
sudo ./bin/mydocker2 <port>
```
<mark>sudo is important! it is necessary for cgroups to work. <br></mark>
Connect to server:
```
nc localhost <port>
```

Next commadns are used after connection to server.<br>
Create container:
```
mydocker create <config_path>
```

Delete container:
```
mydocker delete <container_id>
```

Stop server:
```
mydocker shutdow
```

List of all available containers:
```
mydocker list
```

Run container by id:
```
mydocker run <container_id>
```
After run, container might be used as well as "mydocker" commands are available.<br>
After run, container must always be stopped. <br>
Stop container by id:
```
mydocker stop <container_id>
```

### Config file example
```
id = 3
n_pr = 10
max_memory = 10000000
lclfld = <path>,<path>,...
image = alp_minifs
```
ID and image fields are necessary, other are optional.<br>
Images available for now are "alp_minifs" and "alp_minifs_extended".<br>
It is better to avoid very low memory limits (<1MB) to avoid any unwanted behaviour. 
