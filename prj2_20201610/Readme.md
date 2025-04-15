# RINUX Shell 모방 프로젝트 보고서

## 1. 개발 목표

본 프로젝트는 RINUX의 shell을 모방하여 C 언어와 내장 함수를 사용하여 최대한 유사하게 만드는 것을 목표로 합니다. Phase 1, 2, 3으로 나누어 간단한 입력문부터 백그라운드 실행, 파이프 등 전체적인 맥락의 쉘을 구현하고 테스트하며 실제 쉘과 가깝게 수정합니다. `fork()`를 사용하여 `exec` 계열 함수를 사용해도 현재 돌아가고 있는 `myshell` 이 종료되지 않으면서 새로운 child에서 명령을 실행할 수 있도록 합니다.

## 2. 개발 범위 및 내용

### A. 개발 범위

#### 1. Phase 1: 기본 Shell 기능 구현

*   `myshell` 을 실행했을 때 무한 루프가 돌며 매회 입력을 받기 시작합니다.
*   `cd`, `ls`, `exit` 등의 간단한 기초 쉘 입력문들을 받아 실제 쉘처럼 동작하며, 이 쉘에서 실행한 `mkdir`, `touch` 과 같은 실제 디렉토리에 영향을 주는 입력문들 또한 실제 디렉토리에 직접 반영할 수 있습니다.

#### 2. Phase 2: Redirection과 Pipe 구현

*   Redirection과 pipe를 구현합니다.
*   `(명령문 문장) | (명령문 문장) ….` 을 실행 시 앞의 명령문 문장의 출력 결과물이 뒤의 input으로 들어가게 됩니다. 예를 들어, `cat filename | grep "abc"` 를 실행했을 때, `filename` 의 내용들이 `cat` 으로 출력되는데, 이게 `stdout` 으로 출력되지 않고 뒤의 명령문 문장의 input으로 들어가 `abc` 가 포함된 내용을 찾아 출력해주게 됩니다. 여기서 `""` 의 유무 또한 걸러지게 돼서 모든 경우에 출력이 가능합니다.

#### 3. Phase 3: Background Process 구현

*   Background process를 구현합니다.

### B. 개발 내용

#### Phase 1 (fork & signal)

*   **`fork` 를 통해서 child process를 생성하는 부분에 대해서 설명**

    `builtin_command` 에 포함된 입력이 명령으로 들어올 경우, `fork` 하지 않고 `myshell` 에서 자체적으로 수행하여 출력합니다. 포함되지 않은 경우에 `fork()` 를 수행함과 동시에 `pid` 변수에 pid값을 저장해 줍니다. `pid == 0` 을 만족시킬 경우에 child process를 수행한다는 뜻이 되고, 반대의 경우에는 parent process가 수행되고 있다고 판단할 수 있습니다. `Fork` 하여 child process를 생성하면, 부모와 독립적인 새로운 프로그램이기 때문에 `execvp` 를 수행하여 여러 명령을 처리합니다. 부모는 `execvp` 를 수행하는 child를 reaping하기 위해 `wait` 합니다.

*   **Connection을 종료할 때 parent process에게 signal을 보내는 signal handling하는 방법 & flow**

    Connection을 종료할 때 child는 부모 프로세스에게 `sigchld` 시그널을 보냅니다. 메인 함수에서 `Signal(SIGCHLD,sigchld_handler);` 를 실행하여 signal handler를 통해 signal handling을 해 줍니다. `sigchld_handler` 는 `"sigchld"` Signal을 받았을 때 대기하고 있던 parent에게 `waitpid` 명령을 실행하도록 합니다.

#### Phase 2 (pipelining)

*   **Pipeline (`'|'`)을 구현한 부분에 대해서 간략히 설명 (design & implementation)**

    현재 명령문에서 `'|'` 가 존재할 경우, `eval` 함수에서 `|` 를 경계로 각각의 명령문 문장을 파싱합니다. 파싱된 문장은 2차원 문자열 배열 형태로 저장되며, `exec_pipe` 함수로 전달됩니다. 이 함수에서는 `fork` 를 해 주고, `pipe` 함수를 통해 `fd[2]` 에 파일 디스크립터를 배정해 줍니다. 마지막 명령문을 제외한 명령문들은 pipe input 부분을 닫아주고, `stdout` 대신에 pipe out을 `dup2` 함수를 통해 할당해 줍니다. 그리고 이 pipe out 부분은 닫아줍니다. Pipe out으로 전달된 출력은 부모의 pipe in으로 전달되며, 이것은 부모의 실행 부분에서 `in` 변수에 저장됩니다. 저장된 출력이 다음 생성된 child의 input으로 돌려주며 이를 반복하여 pipe 동작을 수행합니다.

*   **Pipeline 개수에 따라 어떻게 handling했는지에 대한 설명**

    Pipeline 개수에 따라 별도로 handling하지 않았습니다. 0개인 경우 기본적인 실행을 해주었고, 1개 이상이면 `pipe_exec` 함수를 실행했습니다. 파이프라인 명령문에서 마지막 실행문을 제외한 모든 경우에는 출력을 pipe의 `fd` 에 넣어주었고, 마지막의 경우에는 `stdout` 으로 출력할 수 있도록 설계했습니다.

#### Phase 3 (background process)

*   **Background (`'&'`) process를 구현한 부분에 대해서 간략히 설명**

    명령문의 마지막 문자에 `&` 이 입력으로 들어오면 그 부분을 공백으로 처리함과 동시에 `bg` 라는 flag를 1로 설정하였습니다. 그 아래의 모든 처리문 (`eval`, `pipe_exec`) 에서 `bg == 1` 이면 `wait` 을 사용하지 않고 곧바로 다음 반복문을 수행합니다 (부모).  `Bg == 0` 이면 포그라운드 실행으로, `sigchld` 를 unblock하고 `pause` 해주는 `Sigsuspend` 를 사용해서 child를 `wait` 해줍니다.

### C. 개발 방법

*   **B.의 개발 내용을 구현하기 위해 어느 소스코드에 어떤 요소를 추가 또는 수정할 것인지 설명. (함수, 구조체 등의 구현이나 수정을 서술)**

모든 구현은 `myshell.c` 에 추가 및 수정하였습니다.

#### Phase 1:

*   `main` 에서 `sigint`, `sigtstp`, `sigchld` 에 대한 시그널 핸들링을 추가합니다.
*   `Eval` 함수 내에서 `"cd"` 명령어에 대한 수행문을 추가하고 이 경우에 eval은 리턴됩니다 (`void`).
*   그리고 bulit-in 명령어가 아닌 경우, `fork()` 를 수행해 `fork == 0` 인 자식 프로세스에서 `execvp` 를 수행하여 명령을 수행합니다.
*   자식이 종료될 때 `sigchld` handling을 통해 종료되었다는 시그널을 받으면 `void sigchld_handler` 함수에서 부모의 `wait` 을 처리해 줍니다.

#### Phase 2:

*   `'|'` 문자가 입력으로 들어오면 `'|'` 를 경계로 파싱하여 2차원 문자열 배열을 만들고, 이를 인자로 받아 `exec_pipe` 가 실행하도록 추가했습니다.
*   `Void exec_pipe()` 함수는 앞의 명령문부터 순차적으로 처리하며 뒤로 전달하며 마지막 출력은 `stdout` 으로 출력되는 함수입니다. `pipe` 함수를 통해 `fd[2]` 에 파일 디스크립터를 배정해 줍니다. 마지막 명령문을 제외한 명령문들은 pipe input 부분을 닫아주고, `stdout` 대신에 pipe out을 `dup2` 함수를 통해 할당해 줍니다. 그리고 이 pipe out 부분은 닫아줍니다. Pipe out으로 전달된 출력은 부모의 pipe in으로 전달되며, 이것은 부모의 실행 부분에서 `in` 변수에 저장됩니다. 저장된 출력이 다음 생성된 child의 input으로 돌려주며 이를 반복하여 pipe 동작을 수행합니다.

#### Phase 3:

*   추가된 함수:
    *   `int get_max(proc *jobs);`
        *   현재 실행중인 process 중 가장 순번이 큰 process의 pid를 반환합니다.
    *   `int get_index(proc* , pid_t);`
        *   `pid` 에 해당하는 process 의 순번을 반환합니다.
    *   `int get_fg_pid(struct proc *jobs);`
        *   현재 포그라운드에서 실행중인 process의 pid를 반환합니다.
    *   `void sigint_handler(int sig);`
        *   `Ctrl+C` 핸들링. 현재 fg에 돌아가고 있는 process를 kill 하고 jobs에서 삭제합니다.
    *   `void sigtstp_handler(int sig);`
        *   `Ctrl+Z` 핸들링. 현재 fg에 돌아가고 있는 process를 stop 하고 state를 변경합니다.
    *   `void sigchld_handler(int sig);`
        *   child 상태 변화 핸들링. Child에 상태 변화에 따른 행동 추가. 종료되면 해당 pid를 0으로 만들어 jobs 목록에서 삭제합니다.

## 3. 결론 및 향후 개선 방향

*   RINUX shell을 모방한 myshell 구현을 통해 fork, signal, pipe, background process 등 시스템 프로그래밍 핵심 개념을 이해하고 적용할 수 있었습니다.
*   향후 리다이렉션 기능 추가, 오류 처리 강화, 사용자 편의성 향상 등을 통해 더욱 완성도 높은 shell을 만들 수 있을 것입니다.
