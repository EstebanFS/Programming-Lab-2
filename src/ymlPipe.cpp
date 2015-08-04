/*Esteban Foronda Sierra
  Felipe Tovar Ospina
  Jesus Estiven Lopera
*/
using namespace std;
#include <iostream>
#include <sstream>
#include <cstring>
#include <string>
#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include "yaml-cpp/yaml.h"
#include <errno.h>
#include <map>
#include <vector>
#include <utility>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

template <class T> string toStr(const T &x)
{ stringstream s; s << x; return s.str(); }

/*Struct that saves all the job information of the yaml node.*/
struct Process{
  string name;
  string exec;
  string* args;
  int sizeArray;
  int fds[2];
};

/*Struct that saves all the Pipe information of the yaml node.*/
struct Pipe{
  string name;
  string* pipeArgs;
  string input;
  string output;
  int sizeArray;
  string auxFile;
};

#define msprocess map<string, Process>
#define mspipe map<string, Pipe>
#define msvprocess map<string, vector<Process> >
#define vprocess vector<Process>
#define msb map<string, bool>
#define psp pair<string, vector<Process> >
#define vpsp vector<psp>
#define mip map<int, string>
#define msi map<string, int>

/*Overloading operator >> in order to recieve a yaml node and a reference of
an array. This Overloading will save the information of the args yaml into the
array*/
void operator >> (const YAML::Node& node, string* a){
  int n = node.size();
  for(int i = 0; i < n; ++i) a[i] = node[i].as<string>();
}

/*Overloading operator >> in order to receive a yaml node and a reference of
the structure process. This overloading will save all the information of the
yaml node into a process structure*/
void operator >> (const YAML::Node& node, Process& p) {
  p.sizeArray = node["Args"].size();
  p.name = node["Name"].as<string>();
  p.exec = node["Exec"].as<string>();
  p.args = new string[p.sizeArray + 1];
  node["Args"] >> p.args;
}

/*Overloading operator >> in order to receive a yaml node and a reference of
the structure pipe. This overloading will save all the information of the
yaml node into a pipe structure*/
void operator >> (const YAML::Node& node, Pipe& p) {
  p.sizeArray = node["Pipe"].size();
  p.name = node["Name"].as<string>();
  p.pipeArgs = new string[p.sizeArray + 1];
  p.input = node["input"].as<string>();
  p.output = node["output"].as<string>();
  node["Pipe"] >> p.pipeArgs;
}

/*Function that changes the standar input and output for the auxFile*/
void changeStdPipe(Pipe& pipe){
  if(pipe.input != "stdin") freopen(pipe.input.c_str(), "r", stdin);
  if(pipe.auxFile != "stdout") freopen(pipe.auxFile.c_str(), "w", stdout);
}

/*Create the directory ./Temp, that is going to save all the temps files*/
bool createDirectory(){
  int success = mkdir("./temp", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  if(success == 0) return true;
  else {
    cout << "Directory could not be created" << endl;
    return false;
  }
}

/*Remove the directory ./Temp*/
bool removeDirectory(){
  int success = rmdir("./temp");
  if(success == 0) return true;
  else {
    cout << "Directory could not be removed" << endl;
    return false;
  }
}

/*Remove an specific file from ./Temp directory*/
bool removeFile(string file){
  int success = remove(file.c_str());
  if(success == 0) return true;
  else {
    cout << "File could not be removed" << endl;
    return false;
  }
}

/*Create a default pipe*/
Pipe createDefaultPipe(int sizePipes){
  Pipe defaultPipe;
  defaultPipe.name = "default";
  defaultPipe.input = "stdin";
  defaultPipe.output = "stdout";
  defaultPipe.auxFile = "./temp/temp";
  defaultPipe.auxFile += toStr(sizePipes);
  defaultPipe.auxFile += ".txt";
  return defaultPipe;
}

/*Print in stdout a success message*/
void printSuccesMessage(string name){
  printf("## %s finished Succesfully ##\n", name.c_str());
}

/*Print in stdout a unsuccess message*/
void printUnsuccesMessage(int status, string name){
  printf("## %s finished Unsuccesfully(Err:%d) ##\n", name.c_str(), status);
}

/*Check if a child die good or bad*/
void checkProcess(int status, string name){
  /*WIFSIGNALED return true if the child was signaled. If it was signaled
  we can know the real error with the macro WTERMSIG*/
  if(WIFSIGNALED(status)){
    printUnsuccesMessage(WTERMSIG(status), name);
  /*WIFEXITED return true if the child ends normally. If the child ends
  normally we have to check what is the real status, if the status is 0,
  the child finished successfully in other way the child ends unsuccessfully*/
  } else if(WIFEXITED(status)){
    if(!WEXITSTATUS(status))
    printSuccesMessage(name);
    else
    printUnsuccesMessage(WEXITSTATUS(status), name);
  }
}

/*Return the real errno from the child*/
int checkErrno(int status){
  /*WIFSIGNALED return true if the child was signaled. If it was signaled
  we can know the real error with the macro WTERMSIG*/
  if(WIFSIGNALED(status)){
    return WTERMSIG(status);
  /*WIFEXITED return true if the child ends normally. If the child ends
  normally we have to check what is the real status, if the status is 0,
  the child finished successfully in other way the child ends unsuccessfully*/
  } else if(WIFEXITED(status)){
    if(!WEXITSTATUS(status))
    return 0;
    else
    return WEXITSTATUS(status);
  }
}


void manageTempFiles(mip pidToPipeName, mspipe namePipeToPipe,
                 vpsp pipeWithProcess){
  int status;
  int currentPid = 0;
  //Wait for all the master's children
  while (currentPid != -1){
    currentPid = wait(&status);
    if(currentPid != -1){
      string line;
      string pipeName = pidToPipeName[currentPid];
      cout << "## Output " << pipeName << " ##" << endl;
      //Get the pipe associated with that name
      Pipe p = namePipeToPipe[pipeName];
      if(p.output == "stdout"){
        //create an input file stream that connect with the temp file, the
        //temp file has the result of that pipe
        ifstream auxFile (p.auxFile.c_str());
        if (auxFile.is_open()){
          //Getting each line in temp file and put them in stdout
          while (getline(auxFile,line)) cout << line << '\n';
          auxFile.close();
        }
        //Check what is the status of the pipe
        checkProcess(status, pipeName);
      } else {
        //Check what is the status of the pipe
        checkProcess(status, pipeName);
        //Create an input file stream that connect with the temp file, the
        //temp file has the result of that pipe
        ifstream auxFile (p.auxFile.c_str());
        //Create a output file stream that connect with the real file
        //output of the pipe
        ofstream outFile (p.output.c_str());
        if (auxFile.is_open()){
          while (getline(auxFile, line)) {
            //Print each line of the temp file in the real file output
            outFile << line.c_str() << "\n";
          }
          auxFile.close();
          outFile.close();
        }
      }
    }
  }
  //Remove all the temp files in ./Temp directory
  for(int i = 0; i < pipeWithProcess.size(); ++i){
    string namePipe = pipeWithProcess[i].first;
    Pipe p = namePipeToPipe[namePipe];
    removeFile(p.auxFile);
  }
  removeDirectory();
}

int main(int argc, const char* argv[]){
  //If the program can't create the directory, it can't continue with the
  //execution
  if(!createDirectory()) exit(0);
  YAML::Node yaml = YAML::LoadFile(argv[1]);
  int sizeProcesses = yaml["Jobs"].size();
  int sizePipes = yaml["Pipes"].size();
  //This estructure has associated the name of each job with the real job
  //structure, e.g. "nameJob" -> job
  msprocess nameProcessToProcess;
  //This structure has the name of the pipe and for each name the jobs that the
  //pipe has to execute, e.g "pipe1" -> [job1, job2, job3]
  vpsp pipeWithProcess;
  //This estructure has associated the name of each pipe with the real pipe
  //structure, e.g. "namePipe" -> pipe
  mspipe namePipeToPipe;
  //This structure said what job has been used in a pipe and what job hasn't
  msb isADefaultProcess;
  msi jobPosition;
  vprocess vectorJobs;
  mip positionToJob;
  //Iterate the number of jobs and save each job in the structures
  for(int i = 0; i < sizeProcesses; ++i){
    Process process;
    yaml["Jobs"][i] >> process;
    string name = process.name;
    nameProcessToProcess[name] = process;
    //All the jobs start with true, that means that all the job are
    //included in default pipe
    isADefaultProcess[name] = true;
    jobPosition[name] = i;
    positionToJob[i] = name;
    vectorJobs.push_back(process);
  }
  //Iterate the number of pipes and save each pipe in the structures
  for(int i = 0; i < sizePipes; ++i){
    Pipe pipe;
    yaml["Pipes"][i] >> pipe;
    //Create the route of temp file that saves the results of that pipe
    pipe.auxFile = "./temp/temp";
    pipe.auxFile += toStr(i);
    pipe.auxFile += ".txt";
    string name = pipe.name;
    namePipeToPipe[name] = pipe;
    vprocess processes;
    //Fill the vector of jobs that contains what jobs the pipe has to execute
    for(int i = 0; i < pipe.sizeArray; ++i){
      string nameProcess = pipe.pipeArgs[i];
      Process p = nameProcessToProcess[nameProcess];
      processes.push_back(p);
      //If the job is used in a pipe the value has to be changed to false,
      //that means that this job isn't part of default pipe anymore
      isADefaultProcess[nameProcess] = false;
    }
    psp pairPipeProcess = psp(name, processes);
    pipeWithProcess.push_back(pairPipeProcess);
  }
  //Create the default pipe with all the jobs with value equals true in
  //"isADefaultProcess" variable, that pipe isn't defined in yaml file
  Pipe defaultPipe = createDefaultPipe(sizePipes);
  namePipeToPipe["default"] = defaultPipe;
  //Iterate the map isADefaultProcess getting the jobs that have value true
  msb::iterator it;
  vprocess processes;
  vector<int> orderDefaultJobs;
  for (it = isADefaultProcess.begin(); it != isADefaultProcess.end(); ++it){
    if(it -> second == true){
      string name = it -> first;
      orderDefaultJobs.push_back(jobPosition[name]);
    }
  }
  //Sort the elements in order to obtein the same order of the yaml
  sort(orderDefaultJobs.begin(), orderDefaultJobs.end());
  for(int i = 0; i < orderDefaultJobs.size(); ++i){
    string nameJob = positionToJob[orderDefaultJobs[i]];
    Process p = nameProcessToProcess[nameJob];
    processes.push_back(p);
  }
  psp pairPipeProcess = psp("default", processes);
  pipeWithProcess.push_back(pairPipeProcess);
  mip pidToPipeName;
  //Iterate all the pipes and make fork for each pipe
  for(int x = 0; x < pipeWithProcess.size(); ++x){
    int pidPipe = fork();
    //Save the pid of that pipe that will use to know what pipe ends
    pidToPipeName[pidPipe] = pipeWithProcess[x].first;
    //If the fork fail, the program can't execute that pipe, finishing
    //that pipe and printing the errno
    if(pidPipe == -1) {
      printf("## Output %s ##\n",pipeWithProcess[x].first.c_str());
      printf("## %s finished Unsuccessful(Err:%d) ##\n",
              pipeWithProcess[x].first.c_str(), errno);
    } else if (pidPipe == 0){
      //This child will make fork for each job that are contained in the
      //vector of that pipe
      vector<Process> exes = pipeWithProcess[x].second;
      string namePipe = pipeWithProcess[x].first;
      Pipe currentPipe = namePipeToPipe[namePipe];
      //Change the stdin and stdout to a specific input file and temp output
      //file
      changeStdPipe(currentPipe);
      int pidJob = 0;
      //Iterate the jobs that contain that pipe
      for(int i = 0; i < exes.size() && pidJob != -1; ++i){
        if (pipe(exes[i].fds) == -1){
          pidJob = -1;
          continue;
        }
        pidJob = fork();
        //The first job and the last job are special cases, because the last
        //job doesn't put in other file descriptor, and the first one doesn't
        //receive from another file descriptor
        if (pidJob == -1){
        } else if (pidJob == 0){
          //The middles jobs duplicate the descriptor from the last job
          //in order to obtain the result of the last job, and
          //duplicate the the file descriptor of his output to put
          //his reult in that descriptor
          if(i > 0){
            close(0);
            dup2(exes[i - 1].fds[0], 0);
            close(exes[i - 1].fds[0]);
            close(exes[i - 1].fds[1]);
          }
          if(i < exes.size() - 1){
            close(1);
            dup2(exes[i].fds[1], 1);
            close(exes[i].fds[0]);
            close(exes[i].fds[1]);
          }
          //Convert the string in const char* in order to can execute the
          //function execvp
          int n = exes[i].sizeArray + 2;
          char* realExec[n];
          //Adding the first value in the array, that contain the exec of
          //the yaml
          realExec[0] = new char[exes[i].exec.length() + 1];
          strcpy(realExec[0], exes[i].exec.c_str());
          //Adding all the args contained in the yaml
          for(int j = 0; j < exes[i].sizeArray; ++j){
            realExec[j + 1] = new char[exes[i].args[j].length() + 1];
            strcpy(realExec[j + 1], exes[i].args[j].c_str());
          }
          //Adding a null value in the last position of the array
          realExec[n - 1] = NULL;
          execvp(realExec[0], realExec);
          //if execvp fails, ther errno is returned
          exit(errno);
        } else {
          //Close the file descriptors of the last child as a father
          if(i > 0){
            close(exes[i - 1].fds[0]);
            close(exes[i - 1].fds[1]);
          }
        }
      }
      int status;
      //Wait for the last job of that pipe, that supouse contain if the pipe
      //finish correctly or not.
      //If pidJOb is equals to -1 means that something was wrong in the fork
      //or pipe of each job and the pipe could not execute so the errno is
      //returned.
      //If pidJob is not equals to -1 mean that each fork and pipe work
      //correctly and the last job has the real status of the pipe
      if(pidJob != -1) {
        waitpid(pidJob, &status, 0);
        exit(checkErrno(status));
      }
      else exit(errno);
    }
  }
  //Call the function to put all the data in each respective output defined in
  //each pipe
  manageTempFiles(pidToPipeName, namePipeToPipe, pipeWithProcess);
  return 0;
}
