[中文说明](./README_CN.md)

hoststack is a container similar to docker, but hoststack runs as a mirror image of the computer's local operating system file, that is to say, hoststack is the incarnation of the host operating system, because it realizes the isolation of processes and files, so in hoststack Any operation will not pollute the host machine. Hoststack only realizes the isolation from the host process and files without network isolation, and the operation commands are basically consistent with docker. Currently hoststack can only run on linux systems.

## Install

###  Binary file
After the [download](https://github.com/wangzhengquan/hoststack/releases) is complete, decompress and run the hoststack file in the bin directory directly

### Source code compilation
```bash
git clone https://github.com/wangzhengquan/hoststack
cd hoststack
./build.sh
```


## ## Instructions for usage
 All the following commands must be executed as root user.

### run

```
Usage: hoststack run [OPTIONS] [COMMAND] [ARG...]

Run a command in a new container

Options:

  -d, --detach                         Run container in background and print container ID
  -v, --volume list                    Bind mount a volume
  -n, --name string                    Assign a name to the container
```
Demo:

```bash
 sudo hoststack run -v/home/wzq/wk/shmqueue:/app --name "hoststack1"
```
The above command runs a container in the default interactive mode, with the name set to hoststack1, and mounts the local directory v/home/wzq/wk/shmqueue to the /app directory of the container.

### attach

```
Usage: hoststack attach CONTAINER

Attach local standard input, output, and error streams to a running container.
```

Demo:

 If the `-d` option is added when running a container, for example
```bash
 sudo hoststack run -v/home/wzq/wk/shmqueue:/app -d --name "hoststack1"
```
The container will run in the background. At this time, if you want to enter and interact with the container instance, you need to use the attach command, for example, attach to the container named hoststack1

```bash
 sudo hoststack attach hoststack1
```

### stop

Stop a running container with the stop command

```
Usage:	hoststack  stop  CONTAINER [CONTAINER...]

Stop one or more running containers

```
Demo:

```bash
sudo hoststack stop hoststack1
```
The above command stops the previously running container named hoststack1. In addition to this method, you can also directly enter the 'exit' command in interactive mode to stop the container.
Note: Closing the terminal container directly to run in the background will not stop the container.

### start
```
Usage: hoststack start [OPTIONS] CONTAINER

Start a stopped container

Options:

  -d, --detach                         Start container in background and print container ID
```
Demo:

If you want to restart the above stopped container hoststack1, you can use the following command
```bash
sudo hoststack start hoststack1
```

### ps

 If you want to view the status of the current container, you can use this command.
```
Usage: hoststack ps [OPTIONS]

List containers

Options:

  -a, --all             Show all containers (default shows just running)

```
Demo:

```bash
sudo hoststack ps
```
The above command can view the list of currently running containers, and add the -a option to view all containers, including running and stopped.

### rm

Note If a container is completely abandoned, you can use the `rm` command to delete it.

```
Usage:	hoststack container rm [OPTIONS] CONTAINER [CONTAINER...]

Remove one or more containers

Options:

  -f, --force     Force the removal of a running container (uses SIGKILL)
```
Demo:

```bash
sudo hoststack rm hoststack1
```
The above command is to delete the container named hoststack1. If you add the -f option, you can forcefully delete the running container.

 


## Implementation

###  Process isolation
Why is it said that a container is a process. In fact, threads, processes, and containers are the same thing, but they correspond to different isolation levels. Threads do not have any isolation. Processes isolate memory space and file descriptors. Containers are the highest level of isolation. For example, docker’s processes, files, memory,  network and users are all isolated, so it looks like a separate operating system.

### File isolation
Implementing file isolation is as simple as process isolation, both of which are supported by the kernel. You only need to add the file isolation flag when calling the clone method, and then chroot to change the root file directory. A special file system called UnionFilesystem will be used in the container, and there are many implementations of it, such as overlay aufs and so on. This file system itself has nothing to do with file isolation, but it is an essential driver for implementing a container. Why is this file system needed? Let's first look at the basic principles of overlay. The figure below is a schematic diagram of an overlay given by the docker official website.
 ![](./doc/img/overlay_constructs.jpg)
This diagram has two layers, the first layer is "Image layer" and the second layer is "Container layer". These two layers are mounted to "container mount" through the mount command, and this layer is the layer we can see normally . The first layer“ Image layer ”is read-only, you can see that it has three files file1 / file2 and file3. If I modify file2 now, the overlay will copy file2 to the container layer and then modify it on this copy. If I add a new file file4, overlay finds that the Image layer does not have this file, and directly adds this file on the Container layer. It can be seen that the modified file will not have any changes to the original first layer (this behavior has a term called copy on write.) The difference recorded in the original first layer "iamge layer" and the second layer "container layer" (diff) constitutes the third layer "container mount" that is finally seen by the user. Overlay2 can have many such layers. Each layer is like a check point in git. In docker, each layer is packaged into a package called a mirror image. Docker uses this file system to save space, because each layer can be shared. Our hoststack also uses such a file system.

### Others
Containers are a hodgepodge by themselves, and many other technologies are required to make it usable. For example, to realize background operation and attach functions, virtual terminal technology is required. The closing of the container requires the handling of the signal.

## References
>https://draveness.me/docker/  
>https://coolshell.cn/articles/17061.html  
>https://coolshell.cn/articles/17010.html    
>https://coolshell.cn/articles/17049.html  
>https://coolshell.cn/articles/17998.html  
>https://github.com/xianlubird/mydocker  

