# 后端设计

## rlimit

`getrlimit`, `setrlimit`, `prlimit`

限制程序资源：最大虚拟内存、转储文件、CPU 时间、数据段、MEMLOCK、MSGQUEUE、进程优先级、最大文件描述符数量、用户最大进程数、RTPRIO、RTTIME、SIGPENDING、栈大小

## exec

https://stackoverflow.com/questions/5769734/what-are-the-different-versions-of-exec-used-for-in-c-and-c

1. **L vs V**: whether you want to pass the parameters to the exec'ed program as

   - **L**: individual parameters in the call **(variable argument list**): `execl()`, `execle()`, `execlp()`, and `execlpe()`
   - **V**: as **an array of char*** `execv()`, `execve()`, `execvp()`, and `execvpe()`

   The array format is useful when the number of parameters that are to be sent to the exec'ed process are variable -- as in not known in advance, so you can't put in a fixed number of parameters in a function call.

2. **E**: **The versions with an 'e' at the end let you additionally pass an array of char* that are a set of strings added to the spawned processes environment** before the exec'ed program launches. Yet another way of passing parameters, really.

3. **P**: **The versions with 'p' in there use the environment path variable** to search for the executable file named to execute. The versions without the 'p' require an absolute or relative file path to be prepended to the filename of the executable if it is not in the current working directory.