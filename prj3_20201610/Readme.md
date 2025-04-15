시스템 프로그래밍 프로젝트 3 보고서 (README)
1. 개발 목표

본 프로젝트는 클라이언트로부터 주식 거래 요청(매수, 매도, 조회)을 받아 처리하는 서버를 구현하는 것을 목표로 합니다. Event-driven 방식(select 함수 활용)과 Thread-based 방식(pthread 라이브러리 활용)을 모두 구현하고, 각 방식의 성능을 비교 분석합니다. Producer-Consumer, Reader-Writer 문제 등이 발생하지 않도록 동시성 제어에 신경 썼습니다.

주식 정보는 이진 트리를 사용하여 효율적으로 관리하며, 조회, 매수, 매도 작업 모두 이진 트리를 통해 접근합니다.

2. 개발 범위 및 내용

A. 개발 범위

Task 1: Event-driven Approach (select 함수 활용)

I/O Multiplexing: select 함수를 사용하여 다중 클라이언트의 요청을 비동기적으로 처리합니다. 서버는 listen 소켓을 열어두고, select 함수를 통해 여러 소켓을 감시합니다. 이벤트 발생 시 해당 클라이언트의 요청을 처리합니다. 클라이언트 정보는 pool 형태로 관리됩니다.

Epoll과의 차이점: select 함수는 매번 모든 파일 디스크립터를 확인하는 반면, epoll은 상태 변화가 있는 파일 디스크립터만 확인합니다. 파일 디스크립터 수가 많아질수록 select의 성능이 epoll에 비해 떨어질 수 있습니다. 본 프로젝트에서는 파일 디스크립터 수가 크지 않아 성능 차이가 크지 않을 것으로 예상되지만, 일반적으로 Linux 환경에서는 epoll이 더 효율적입니다.

Task 2: Thread-based Approach (pthread 라이브러리 활용)

Master Thread의 Connection 관리: Master Thread는 클라이언트의 연결 요청을 수락하고, Worker Thread Pool에서 Worker Thread를 할당하여 클라이언트의 요청을 처리합니다.

Worker Thread Pool 관리: Worker Thread Pool은 클라이언트 요청을 처리하는 Worker Thread들의 집합입니다. 최대 Thread 수를 제한하고, sbuf 패키지를 사용하여 Thread Pool에 대한 동시 접근 문제를 해결합니다. Worker Thread는 클라이언트 연결 종료 시 Pool로 반환됩니다. Semaphore를 사용하여 여러가지 problem을 제어합니다.

Task 3: Performance Evaluation

Task 1과 Task 2의 성능을 비교 분석합니다.

Metric 정의 및 측정 방법:

요청 클라이언트 수: 성능 평가의 기본 단위.

Thread 수: Thread-based 방식에서 성능에 미치는 영향 분석.

총 처리 시간: 전체 작업 완료 시간.

전체 처리량: 클라이언트 수 * 클라이언트 당 요청 수 (클라이언트 당 요청 수는 10으로 고정).

동시 처리율: 전체 처리량 / 총 처리 시간.

예상 결과: Thread-based 방식이 Event-driven 방식보다 CPU 코어를 더 효율적으로 활용하여 더 높은 성능을 보일 것으로 예상됩니다.

B. 개발 내용

Task 1 (Event-driven Approach with select())

select 함수를 이용한 I/O Multiplexing 구현.

클라이언트 연결 수락, 데이터 처리, 연결 종료 처리 구현.

클라이언트 정보를 pool 형태로 관리.

Task 2 (Thread-based Approach with pthread)

Master Thread: 클라이언트 연결 수락 및 Worker Thread 할당.

Worker Thread Pool: 클라이언트 요청 동시 처리.

sbuf 패키지를 사용한 Thread Pool 관리 및 동시성 제어.

Semaphore를 활용한 Reader-Writer 문제 해결.

Task 3 (Performance Evaluation)

다양한 클라이언트 수, Thread 수, 워크로드(매수, 매도, 조회)에 따른 성능 측정 및 분석.

gettimeofday 함수를 사용하여 시간 측정.

C. 개발 방법

Task 1, 2 공통

Item 구조체: 주식 정보를 저장 (주식 번호, 이름, 가격, 수량).

TreeNode 구조체: 이진 트리 노드 (Item 포인터, 좌/우 자식 노드 포인터).

Load_stock 함수: stock.txt 파일에서 주식 정보를 읽어와 이진 트리에 저장.

Sell, Buy 함수: 입력받은 주문에 따라 이진 트리를 검색하여 해당 주식의 정보를 업데이트.

Print_stock 함수: 이진 트리의 주식 정보를 버퍼에 옮겨 클라이언트에게 전송.

Save_stock 함수: 이진 트리의 주식 정보를 stock.txt 파일에 저장.

Task 1

Pool 구조체: 클라이언트 정보를 저장하고 관리. (read/ready 디스크립터, 최대 디스크립터 수, 준비된 디스크립터 수, 디스크립터 배열, Rio_t 배열)

Init_pool 함수: Pool 초기화.

Add_client 함수: 새로운 클라이언트를 Pool에 추가.

Check_clients 함수: Pool 내 모든 클라이언트의 요청을 처리.

Task 2

Sbuf_t 구조체: Worker Thread Pool 관리를 위한 구조체 (버퍼, head, tail, 스레드 개수, 뮤텍스, 조건 변수).

Sbuf_init, Sbuf_insert, Sbuf_remove 함수: Thread Pool 초기화, 삽입, 제거 함수.

Worker Thread 함수: 클라이언트 요청 처리, detach 상태로 변경하여 종료 시 메모리 해제.

Mutex: Buy/Sell 시 Item 정보 수정 시 write mutex를 사용하여 동시 접근 제어.

Read/Write Mutex: Show/Save_stock 시 read mutex를 사용하여 Reader-Writer 문제 해결.

3. 구현 결과

Event-driven 방식: 단일 스레드로 Concurrent한 프로세스 요청 처리 가능.

Thread-based 방식: Item 단위 semaphore를 사용하여 클라이언트 간 충돌 방지.

개선점: 현재 이진 트리는 순차적인 데이터 입력 시 비효율적이며, 레드-블랙 트리 등을 사용하여 개선 가능.

A. 시행착오 (Sleep 시간의 영향)

초기 측정 시 Task 2의 수행 시간이 Task 1보다 오래 걸리는 문제 발생. Sleep 시간이 수행시간에 큰 영향을 미치는 것을 확인.

B. 결과 분석

Task 1 vs Task 2: Task 2 (Thread-based)가 Task 1 (Event-driven)보다 전반적으로 더 높은 동시 처리율과 짧은 수행 시간을 보였습니다. 특히, 처리량이 증가할수록 성능 차이가 커졌습니다.

워크로드 별: Task 1에서는 Show만 실행하는 경우 수행 시간이 증가했습니다. 재귀 함수 호출이 원인으로 추정됩니다. Task 2에서는 Buy/Sell을 실행하는 경우 수행 시간이 증가했습니다. Reader-Writer 문제 해결을 위한 Mutex 사용으로 인해 Reader의 동시 접근이 제한되어 발생하는 현상으로 분석됩니다.

클라이언트 개수 변화에 따른 동시 처리율 변화: 클라이언트 수가 증가함에 따라 동시 처리율은 로그 함수 형태에 가깝게 증가하는 경향을 보였습니다. 150 클라이언트 이상부터는 클라이언트 fork에 소요되는 시간이 증가하는 것으로 분석됩니다. Thread 수에 따른 뚜렷한 동시 처리율 차이는 관찰되지 않았습니다.

Thread 수 변화에 따른 동시 처리율 변화: 클라이언트 수가 증가함에 따라 Thread 수가 클라이언트 수와 같을 때, 각 Thread의 동시 처리율이 특정 값에 수렴하는 경향을 보였습니다. 이는 Thread 개수가 충분히 많아지면 병목 현상이 발생하여 더 이상의 성능 향상이 이루어지지 않음을 의미합니다. (위 sleep을 다시 넣은 실험 참고)

4. 종합 평가

Task 2 (Thread-based)가 Task 1 (Event-driven)보다 더 나은 성능을 보였습니다.

Thread 개수가 증가함에 따라 동시 처리율이 증가하지만, 특정 개수 이상에서는 성능 향상이 미미하거나 오히려 감소하는 현상이 발생했습니다.

병목 현상 해결을 위한 추가적인 최적화 (e.g., Connection Pool, Lock-free 자료구조)가 필요합니다.

이진 트리의 효율성 개선 (e.g., 레드-블랙 트리)을 통해 전반적인 성능 향상을 기대할 수 있습니다.
