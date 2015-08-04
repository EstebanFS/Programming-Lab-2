# Programming Lab 2

Programming lab 2 is the second project developed for the course
"Operating Systems"

### Description
The aim of the project is to develop a software that is going to receive a
series of tasks descriptions in YAML format and
a series of pipes that connect some specific jobs.
The software will try to execute the connected jobs and
it will show if the task could be completed or not.

In order to make a clear idea of the project, this will
execute the same task as if you run the command

```sh
echo hello_world | wc &
```

If the task can not be completed, the software will show a message with the
specific error that the system receive when tried to execute the task.
In other case the software will show the result of the task.

The YAML description will contain all the necessary information to execute
the task in a succesful way. The following code is a little example of a YAML
description.

```sh
Jobs :
  - Name : "job1"
    Exec : "ecsho"
    Args : ["you", "gotta","be","kidding"]
  - Name : "job2"
    Exec : "tr"
    Args : [" ","_"]
  - Name : "job3"
    Exec : "yes"
    Args : ["No","I’m", "not!"]
  - Name : "job4"
    Exec : "head"
    Args : []
  - Name : "job5"
    Exec : "echo"
    Args : ["No","I’m", "not!"]
  - Name : "job6"
    Exec : "tr"
    Args : [" ", "*"]
Pipes :
  - Name : "pipe1"
    Pipe : [ "job1","job2" ]
    input : "stdin"
    output : "out.txt"
  - Name : "pipe2"
    Pipe : [ "job5","job6" ]
    input : "stdin"
    output : "out2.txt"
```
The yaml file is composed by two sections.

The first one if Jobs, that contain all the individual jobs that can be
executed in the program.

Each job has to have the following information:

- Name: Is a descriptive name for the job.
- Exec: Is the name of the program that will be run, either with an absolute
or relative path
- Args: Is a list of arguments for the job.

The second one describe the pipes, each pipe will say
what jobs it executes and in what order.

Each pipe has to have the following information:

- Name: Is a descriptive name for the pipe.
- Jobs: The list of jobs in that pipe. note that the order in the list is the
actual redirection order.
- Args: Is a list of arguments for the job.
- Input: Whether to read from standard input (stdin) or from
a file.
-Output: Whether to write to standard output (stdout) or to
a file.

### Requirements
In order to run this software correctly, is necessary install some specific
free software or libraries before run it.

The following list are composed with those specific softwares or libraries

* [Libboost-all-dev]: Can be installed with sudo-apt-get install
  libboost-all-dev.
* [Cmake]: Can be installed with sudo apt-get install cmake.
* [Yaml-cpp]: Can be downloaded in the official page. This software use the
  version 0.5.1

### Executing
The proyect contain a MakeFile that have some options to use the proyect
easier, this options will help you to interact with the proyect.

- BUILD: Use that option if you want to compile the project and generate an
  executable into bin file, you can use "build" or "make" with out any param.
  Both will do the same action.
```sh
make build
```
```sh
make
```
- RUN: Use that option if you want to run the project with the examples into
examples file, unfortunately in this moment the project does not execute all
the examples automatically. So you have to modify the "makefile" a little bit
and add the example that you want to execute.
- The following code is the way that you can add an example to run, you can
  add how many examples you want.
- To add other example, there are a section called "run", you only have to add
the next line into that section.
```sh
@./bin/ymlRead examples/nameTest.yml
```
- After add the line the "run" section will look something like this:
```sh
run:
  @./bin/ymlRead examples/test.yml
  @./bin/ymlRead examples/testError.yml
  @./bin/ymlRead examples/nameTest.yml
```
- CLEAN: Use that option if you want to clean the project. This action will
erase all the executables generated into "bin" file.
```sh
make clean
```

### Development Team
This project was developed by a gruop of 3 people. All the group study System
Engeneer in EAFIT University. The name are presented next.
```sh
Esteban Foronda Sierra
```
```sh
Felipe Tovar Ospina
```
```sh
Jesús Estiven Lopera Jaramillo
```
[libboost-all-dev]:http://packages.ubuntu.com/precise/libboost-all-dev
[Yaml-cpp]:https://code.google.com/p/yaml-cpp/downloads/list
[Cmake]:https://launchpad.net/ubuntu/+source/cmake