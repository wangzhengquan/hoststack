## hoststack

hoststack是一个类似与docker的容器，但是hoststack是以计算机本地操作系统的文件作为的镜像运行的，也就是说hoststack是宿主机操作系统的化身，但是因为实现了进程和文件的隔离所以在hoststack里面的任何操作都不会污染宿主机。hoststack只实现了与宿主机的进程和文件的隔离没有网络隔离，操作命令基本保持了与docker一致。当下hoststack只能运行在linux系统上。


## 使用说明
以下所有的命令都要以root用户执行。

## run

说明
```
Usage: hoststack run [OPTIONS] [COMMAND] [ARG...]

Run a command in a new container

Options:

  -d, --detach                         Run container in background and print container ID
  -v, --volume list                    Bind mount a volume
  -n, --name string                    Assign a name to the container
```
实例

```bash
 sudo hoststack run -v/home/wzq/wk/shmqueue:/app --name "hoststack1"
```
以上命令以默认的可交互的模式运行一个容器，名字设为hoststack1， 挂载本机目录v/home/wzq/wk/shmqueue到容器的/app目录

## attach

说明
```
Usage: hoststack attach CONTAINER

Attach local standard input, output, and error streams to a running container.
```

实例
如果run一个容器的时候加了`-d`选项，例如
```bash
 sudo hoststack run -v/home/wzq/wk/shmqueue:/app -d --name "hoststack1"
```
容器会在后台运行，这时如果要进入容器实例并与之交互就要用到attach命令，例如attach到上面的以后台运行的方式运行的名字为hoststack1的容器

```bash
 sudo hoststack attach hoststack1
```

## stop
说明
```
Usage:	docker  stop  CONTAINER [CONTAINER...]

Stop one or more running containers

```

停止正在运行的容器用stop命令，例如停止前面已经运行起来的名字为hoststack1的容器
```
sudo hoststack stop hoststack1
```
除了这种方式，还可以在交互模式中直接输入‘exit’命令也可以停止容器。  
注意：关闭terminal后容器转为后台运行，并不会停止容器。

## start
说明
```
Usage: hoststack start [OPTIONS] CONTAINER

Start a stopped container

Options:

  -d, --detach                         Start container in background and print container ID
```
实例
假若要重新启动上面已经停止的容器hoststack1，可以使用如下命令

```
sudo hoststack start hoststack1
```

## ps
说明
```
Usage: hoststack ps [OPTIONS]

List containers

Options:

  -a, --all             Show all containers (default shows just running)

```
实例
如果要查看当前容器的状态可以用这个命令。例如查看当前正在运行的容器列表，可以用如下命令
```
sudo hoststack ps
```
加-a选项可以查看所有的容器，包括正在运行和已经停止的。

## rm
```
Usage:	hoststack container rm [OPTIONS] CONTAINER [CONTAINER...]

Remove one or more containers

Options:

  -f, --force     Force the removal of a running container (uses SIGKILL)
```
如果某个容器已经彻底废弃不要了可以用rm命令删除。例如,删除名字为hoststack1的容器：
```
sudo hoststack rm hoststack1
```
加-f选项可以强制删除正在运行的容器。



## 原理

### 进程隔离
为什么说容器是一个进程。其实线程 进程 容器是同一类东西，只是它们对应了不同的隔离级别。线程没有任何隔离，进程隔离的是内存空间和文件描述符，容器是最高级别的隔离，比如docker它的进程 文件 内存 网络 用户全部都是隔离的，所以看起来象一个单独的操作系统。

### 文件隔离
实现文件隔离与进程隔离一样是很简单的事情，都是内核支持的。只需要在调用clone方法的时候加入文件隔离的标识，然后chroot改变根文件目录。容器中会用到一种特殊的文件系统叫UnionFilesystem，它的实现有很多比如overlay aufs等。这种文件系统本身与文件隔离没有任何关系，但是它是实现一个容器必不可少的驱动。为什么需要这种文件系统呢？先看一下overlay的基本原理。下图是docker官网给出的一张overlay的示意图。
 ![](./doc/img/overlay_constructs.jpg)
这个示意图有两层，第一层是Image layer，第二层是Container layer. 这两层通过mount命令mount到container mount，这一层就是我们正常能看到的那一层. 第一层Image layer是只读的，可以看到它有三个文件file1 file2 file3.假如我现在修改file2.overlay会拷贝file2 到container layer 然后在这个copy上进行修改。假如我新增一个文件file4，overlay发现Image layer没有这个文件，就直接在Container layer上新增这个文件。可以看到修改文件对原始的第一层不会有任何改动（这种行为有一个术语叫copy on write.）原始的第一层iamge layer 加第二层container layer里记录的差异（diff)组成了最终给用户看到的第三层container mount.overlay2可以有很多这样的层，这里面的每一层就好像git里的提交点，在docker里这每一层打成一个包就叫镜像。docker用这种文件系统是为了节约空间，因为每一层都可以共用。我们的hoststack第一也是为了节约空间，第二是为了修改容器里的文件不会对本机文件有影响。

### 其他技术
容器本身是一个大杂烩，要让它跑起来可用需要许多其他技术的支持。例如实现后台运行和attach功能，需要虚拟终端技术。容器的关闭需要对信号的处理。这些东西的资料比较多，也相对容易理解，这个文档就不解释了。

