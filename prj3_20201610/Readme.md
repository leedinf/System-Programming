# 시스템 프로그래밍 프로젝트 3 보고서

## 1. 개발 목표

본 프로젝트는 클라이언트로부터 주식 거래 요청 (매수, 매도, 조회)을 받아 처리하는 서버를 구현하는 것을 목표로 합니다. Event-driven 방식 (select 함수 활용)과 Thread-based 방식 (pthread 라이브러리 활용)을 모두 구현하고, 각 방식의 성능을 비교 분석합니다. Producer-Consumer, Reader-Writer 문제 등이 발생하지 않도록 동시성 제어에 신경 썼습니다.

주식 정보는 이진 트리를 사용하여 효율적으로 관리하며, 조회, 매수, 매도 작업 모두 이진 트리를 통해 접근합니다.

## 2. 개발 범위 및 내용

### A. 개발 범위

#### Task 1: Event-driven Approach (select 함수 활용)

*   **I/O Multiplexing:** `select` 함수를 사용하여 다중 클라이언트의 요청을 비동기적으로 처리합니다. 서버는 listen 소켓을 열어두고, `select` 함수를 통해 여러 소켓을 감시합니다. 이벤트 발생 시 해당 클라이언트의 요청을 처리합니다. 클라이언트 정보는 pool 형태로 관리됩니다.

*   **Epoll과의 차이점:** `select` 함수는 매번 모든 파일 디스크립터를 확인하는 반면, `epoll`은 상태 변화가 있는 파일 디스크립터만 확인합니다. 파일 디스크립터 수가 많아질수록 `select`의 성능이 `epoll`에 비해 떨어질 수 있습니다. 본 프로젝트에서는 파일 디스크립터 수가 크지 않아 성능 차이가 크지 않을 것으로 예상되지만, 일반적으로 Linux 환경에서는 `epoll`이 더 효율적입니다.

#### Task 2: Thread-based Approach (pthread 라이브러리 활용)

*   **Master Thread의 Connection 관리:** Master Thread는 클라이언트의 연결 요청을 수락하고, Worker Thread Pool에서 Worker Thread를 할당하여 클라이언트의 요청을 처리합니다.

*   **Worker Thread Pool 관리:** Worker Thread Pool은 클라이언트 요청을 처리하는 Worker Thread들의 집합입니다. 최대 Thread 수를 제한하고, `sbuf` 패키지를 사용하여 Thread Pool에 대한 동시 접근 문제를 해결합니다. Worker Thread는 클라이언트 연결 종료 시 Pool로 반환됩니다. Semaphore를 사용하여 여러가지 problem을 제어합니다.

#### Task 3: Performance Evaluation

*   Task 1과 Task 2의 성능을 비교 분석합니다.

*   **Metric 정의 및 측정 방법:**

    *   **요청 클라이언트 수:** 성능 평가의 기본 단위.
    *   **Thread 수:** Thread-based 방식에서 성능에 미치는 영향 분석.
    *   **총 처리 시간:** 전체 작업 완료 시간.
    *   **전체 처리량:** 클라이언트 수 \* 클라이언트 당 요청 수 (클라이언트 당 요청 수는 10으로 고정).
    *   **동시 처리율:** 전체 처리량 / 총 처리 시간.

*   **예상 결과:** Thread-based 방식이 Event-driven 방식보다 CPU 코어를 더 효율적으로 활용하여 더 높은 성능을 보일 것으로 예상됩니다.

### B. 개발 내용

#### Task 1 (Event-driven Approach with select())

*   `select` 함수를 이용한 I/O Multiplexing 구현.

*   클라이언트 연결 수락, 데이터 처리, 연결 종료 처리 구현.

*   클라이언트 정보를 pool 형태로 관리.

#### Task 2 (Thread-based Approach with pthread)

*   Master Thread: 클라이언트 연결 수락 및 Worker Thread 할당.

*   Worker Thread Pool: 클라이언트 요청 동시 처리.

*   `sbuf` 패키지를 사용한 Thread Pool 관리 및 동시성 제어.

*   Semaphore를 활용한 Reader-Writer 문제 해결.

#### Task 3 (Performance Evaluation)

*   다양한 클라이언트 수, Thread 수, 워크로드 (매수, 매도, 조회)에 따른 성능 측정 및 분석.

*   `gettimeofday` 함수를 사용하여 시간 측정.

### C. 개발 방법 (요약)

*   **Item 구조체:** 주식 정보 (주식 번호, 이름, 가격, 수량) 저장.
*   **TreeNode 구조체:** 이진 트리 노드 (Item 포인터, 좌/우 자식 노드 포인터).
*   **Load\_stock 함수:** `stock.txt` 에서 주식 정보 로딩.
*   **Sell, Buy 함수:** 주식 매매 처리 및 정보 업데이트.
*   **Print\_stock 함수:** 주식 정보 클라이언트로 전송.
*   **Save\_stock 함수:** 주식 정보 `stock.txt` 에 저장.
*   **Task 1 Pool 구조체 및 관련 함수:** 클라이언트 정보 관리.
*   **Task 2 Sbuf\_t 구조체, 스레드 풀 관련 함수, Mutex:** 스레드 풀 관리 및 동시성 제어.

## 3. 구현 결과

*   Event-driven 방식: 단일 스레드로 Concurrent한 프로세스 요청 처리 가능.
*   Thread-based 방식: Item 단위 semaphore를 사용하여 클라이언트 간 충돌 방지.
*   **개선점:** 현재 이진 트리는 순차적인 데이터 입력 시 비효율적이므로, 레드-블랙 트리 등을 사용하여 개선 가능.

## 4. 성능 평가 결과 (Task 3)

*   **측정 환경:** cspro9 (서버), cspro (클라이언트).
*   **측정 시점:** 2024-06-01 오전 11시경.
*   **고정 값:** 클라이언트 당 요청 처리 수 = 10, multiclient sleep 시간 = 100000.

### 결과 요약

*   Thread 개수가 증가함에 따라 동시 처리율이 증가하지만, 특정 개수 이상에서는 성능 향상이 미미하거나 오히려 감소하는 현상이 발생했습니다.

*   병목 현상 해결을 위한 추가적인 최적화 (e.g., Connection Pool, Lock-free 자료구조)가 필요합니다.

*   이진 트리의 효율성 개선 (e.g., 레드-블랙 트리)을 통해 전반적인 성능 향상을 기대할 수 있습니다.

## 5. 결론 및 향후 개선 방향

*   Thread-based 방식이 Event-driven 방식보다 더 나은 성능을 보였습니다.

*   향후 병목 현상을 해결하고, 자료구조를 개선하여 성능을 더욱 향상시킬 수 있습니다.
