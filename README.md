## kucker
kucker的运行环境是基于本地的环境，没有镜像的概念。kucker只有进程和文件的隔离。kucker基本命令的基本保持了与docker的一致。

## 命令说明
以下所有的命令都要以root用户执行。

## run

说明
```
Usage: kucker run [OPTIONS] [COMMAND] [ARG...]

Run a command in a new container

Options:

  -d, --detach                         Run container in background and print container ID
  -v, --volume list                    Bind mount a volume
  -n, --name string                    Assign a name to the container
```
实例

```bash
 sudo kucker run -v/home/wzq/wk/shmqueue:/app --name "kucker1"
```
以上命令以默认的可交互的模式运行一个容器，名字设为kucker1， 挂载本机目录v/home/wzq/wk/shmqueue到容器的/app目录

## attach

说明
```
Usage: kucker attach CONTAINER

Attach local standard input, output, and error streams to a running container.
```

实例
如果run一个容器的时候加了`-d`选项，例如
```bash
 sudo kucker run -v/home/wzq/wk/shmqueue:/app -d --name "kucker1"
```
容器会在后台运行，这时如果要进入容器实例并与之交互就要用到attach命令，例如attach到上面的以后台运行的方式运行的名字为kucker1的容器

```bash
 sudo kucker attach kucker1
```

## stop
说明
```
Usage:	docker  stop  CONTAINER [CONTAINER...]

Stop one or more running containers

```

停止正在运行的容器用stop命令，例如停止前面已经运行起来的名字为kucker1的容器
```
sudo kucker stop kucker1
```
除了这种方式，还可以在交互模式中直接输入‘exit’命令也可以停止容器。  
注意：关闭terminal后容器转为后台运行，并不会停止容器。

## start
说明
```
Usage: kucker start [OPTIONS] CONTAINER

Start a stopped container

Options:

  -d, --detach                         Start container in background and print container ID
```
实例
假若要重新启动上面已经停止的容器kucker1，可以使用如下命令

```
sudo kucker start kucker1
```

## ps
说明
```
Usage: kucker ps [OPTIONS]

List containers

Options:

  -a, --all             Show all containers (default shows just running)

```
实例
如果要查看当前容器的状态可以用这个命令。例如查看当前正在运行的容器列表，可以用如下命令
```
sudo kucker ps
```
加-a选项可以查看所有的容器，包括正在运行和已经停止的。

## rm
```
Usage:	kucker container rm [OPTIONS] CONTAINER [CONTAINER...]

Remove one or more containers

Options:

  -f, --force     Force the removal of a running container (uses SIGKILL)
```
如果某个容器已经彻底废弃不要了可以用rm命令删除。例如,删除名字为kucker1的容器：
```
sudo kucker rm kucker1
```
加-f选项可以强制删除正在运行的容器。


