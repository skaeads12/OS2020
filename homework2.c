#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RR 0
#define SJF 1
#define PQ 2
#define FIFO 3
#define SERVE 4

int lastPid = 0;
float alpha = 5;
float estimate = 10;
int serving = 0; // cpu에서 작업중인 프로세스 pid, 없으면 0
int needSwitching = 0; // 스위칭이 필요하면 1
int switchingTime = 0; // 스위칭 하는데 남은 시간

typedef struct Process
{
	int pid;
	int state;
	int priority;
	int burstTime;
	int arrivalTime;

	struct Process *pc;
} Process;

typedef struct Queue
{
	Process *front;
	Process *rear;

	int quantum;
	int state;
	int count;
	int preemptive;
} Queue;

void GenerateBurstTime(Process *);
void GeneratePriority(Process *);
void Terminate(Process *);

Process *CreateProcess(void)
{
	int pid = ++lastPid;

	Process *new = (Process *)malloc(sizeof(Process));
	new->pc = NULL;
	new->pid = pid;
	new->state = -1;
	new->priority = -1;
	new->burstTime = -1;
	new->arrivalTime = -1;

	return new;
}

Queue *CreateQueue(void)
{
	Queue *new = (Queue *)malloc(sizeof(Queue));

	new->front = NULL;
	new->rear = NULL;
	new->count = 0;
	new->preemptive = -1;
	new->state = -1;

	return new;
}

int IsEmptyQueue(Queue *queue)
{
	return queue->count == 0;
}

void Enqueue_RR_FIFO(Queue *queue, Process *process)
{
	process->state = queue->state;

	if(process->burstTime == -1)
		GenerateBurstTime(process);

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		queue->rear->pc = process;
		queue->rear = process;
		queue->count++;
		return;
	}
}

void Enqueue_SJF(Queue *queue, Process *process)
{
	if(process->burstTime == -1)
		GenerateBurstTime(process);

	process->state = queue->state;

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		Process *current = queue->front;
		Process *prev = NULL;

		while(current)
		{
			if(current->burstTime < process->burstTime)
			{
				prev = current;
				current = current->pc;
			}
			else
				break;
		}

		if(prev == NULL)
		{
			process->pc = current;
			queue->front = process;
		}
		else
		{
			process->pc = current;
			prev->pc = process;
		}
		queue->count++;
	}
}

void Enqueue_PQ(Queue *queue, Process *process)
{
	if(process->priority == -1)
		GeneratePriority(process);

	process->state = queue->state;

	if(process->burstTime == -1)
		GenerateBurstTime(process);

	if(IsEmptyQueue(queue))
	{
		queue->front = process;
		queue->rear = process;
		queue->count++;
		return;
	}
	else
	{
		Process *current = queue->front;
		Process *prev = NULL;

		while(current)
		{
			if(current->priority < process->priority)
			{
				prev = current;
				current = current->pc;
			}
			else
				break;
		}

		if(prev == NULL)
		{
			process->pc = current;
			queue->front = process;
		}
		else
		{
			process->pc = current;
			prev->pc = process;
		}
		queue->count++;
	}
}

void Enqueue(Queue *queue, Process *process)
{
	if(queue->state == SJF)
		Enqueue_SJF(queue, process);
	else if(queue->state == PQ)
		Enqueue_PQ(queue, process);
	else
		Enqueue_RR_FIFO(queue, process);
}

Process *Dequeue(Queue *queue)
{
	Process *process = queue->front;
	queue->front = process->pc;
	queue->count--;

	return process;
}

void PrintQueue(Queue *queue) // 코드 완성되면 지울것
{
	Process *process = queue->front;
	
	for(int i = 0; i < queue->count; i++)
	{
		printf("[%d]: %d\n", process->pid, process->burstTime);
		process = process->pc;
	}
}

void PrintQueue2(Queue *queue) // 테스트용 2
{
	Process *process = queue->front;

	for(int i = 0; i< queue->count; i++)
	{
		printf("[%d]: %d\n", process->pid, process->priority);
		process = process->pc;
	}
}

void GenerateBurstTime(Process *process)
{

	float coef = rand() / (float) RAND_MAX;

	float result = coef * alpha + (1 - coef) * estimate;

	if(result < 0)
		result *= -1;
	else if(result == 0)
		result = 1;

	process->burstTime = (int) result;

	printf("Process execution time = %d\n\n", (int) result);
}

void GeneratePriority(Process *process)
{
	int random = rand() % 100;

	process->priority = random;

}

/* 프로세스를 서비스하는 함수, burstTime이 줄어듬.     */
/* 만약 0이 되면 needSwitching이 올라가고 Terminate함. */
void Service(Process *process)
{
	process->burstTime--;

	if(!process->burstTime) // 서비스 종료
	{
		serving = 0;
		needSwitching = 1;
		Terminate(process);
	}
}

/* 큐에서 새로운 프로세스를 선택하는 함수, 없다면 NULL */
/* 선택할 때 serving이 pid가 됨, 마찬가지로 없다면 그대로 */
Process *Select(Queue *queue)
{
	if(!IsEmptyQueue(queue))
	{
		Process *servedProcess = Dequeue(queue);
		serving = servedProcess->pid;
		
		return servedProcess;
	}
	else
		return NULL;
}

void Terminate(Process *process)
{
	
	free(process);
}

/* currentProcess 변경함 */
Process *ContextSwitching(Queue *queue, Process *currentProcess)
{

	if(IsEmptyQueue(queue))
	{
		return currentProcess;
	}
	else
	{
		Process *new = Select(queue);
		Enqueue(queue, currentProcess);
		return new;
	}
}

int main(void)
{
	Queue *queueRR = CreateQueue();
	queueRR->state = RR;
	Queue *queueSJF = CreateQueue();
	queueSJF->state = SJF;
	Queue *queuePQ = CreateQueue();
	queuePQ->state = PQ;
	Queue *queueFIFO = CreateQueue();
	queueFIFO->state = FIFO;

	Process *currentProcess = CreateProcess();

	int initSwitchingTime = 1;
	int initQuantum = 20;

	queueRR->quantum = initQuantum;

	int preemptionSJF = 1;
	int preemptionPQ = 1;

	needSwitching = 1;
	switchingTime = 0;

	srand(time(NULL));

	queueRR->quantum = initQuantum;

	for(int i = 0; i < 100; i++)
	{
		printf("-------------------------------------\n[CYCLE TIME: %d]\n----------------------------------\n", i);

		if(rand() % 2 == 0 || i == 0) // 50% 프로세스 생성
		{
			switch(rand() % 4)
			{
				case 0:
					Enqueue_RR_FIFO(queueRR, CreateProcess());
					printf("Process appended to RR queue.\n");
					break;
				case 1:
					Enqueue_SJF(queueSJF, CreateProcess());
					if(currentProcess->state == SJF && currentProcess->burstTime > queueSJF->front->burstTime && preemptionSJF == 0)
						needSwitching = 1;
					printf("Process appended to SJF queue.\n");
					break;
				case 2:
					Enqueue_PQ(queuePQ, CreateProcess());
					if(currentProcess->state == PQ && currentProcess->priority > queuePQ->front->burstTime && preemptionPQ == 0)
						needSwitching = 1;
					printf("Process appended to PQ queue.\n");
					break;
				case 3:
					Enqueue_RR_FIFO(queueFIFO, CreateProcess());
					printf("Process appended to FIFO queue\n");
					break;
			}
		}
		else
		{
			printf("no new process was generated.\n\n");
		}

		if(switchingTime > 0) // 스위칭 중이면 작업없이 다음 사이클로
		{
			switchingTime--;
			
			continue;
		}

		if((serving == 0 && needSwitching == 1) || i == 0) // 작업중이던 프로세스 TERMINATED.
		{
			currentProcess = currentProcess->pc;
			if(!currentProcess) // 다음 작업할 프로세스 선택
			{
				if(IsEmptyQueue(queueRR))
					if(IsEmptyQueue(queueSJF))
						if(IsEmptyQueue(queuePQ))
							currentProcess = Select(queueFIFO);
						else
							currentProcess = Select(queuePQ);
					else
						currentProcess = Select(queueSJF);
				else
				{
					currentProcess = Select(queueRR);
					queueRR->quantum = initQuantum;
				}
			}

			if(currentProcess) // 프로세스가 선택되었다면
			{
				switchingTime = initSwitchingTime;
			}
			
		}
		else if(needSwitching == 1) // 컨텍스트 스위칭이 필요할때
		{
			printf("STATE: %d\n", currentProcess->state);
			int state = currentProcess->state;
			switch(state)
			{
				case SJF:
					currentProcess = ContextSwitching(queueSJF, currentProcess);
					break;
				case PQ:
					currentProcess = ContextSwitching(queuePQ, currentProcess);
					break;
			}

			needSwitching = 0;
		}
		else // 그냥 계속 서비스
		{
			Service(currentProcess);
			if(currentProcess->state == RR)
			{
				queueRR->quantum--;
				if(queueRR->quantum == 0)
				{
					if(IsEmptyQueue(queueRR))
						queueRR->quantum = initQuantum;
					else
						currentProcess = ContextSwitching(queueRR, currentProcess);
				}
			}
		}

		printf("# of jobs in RR = %d\n\n", queueRR->count);
		printf("# of jobs in SJF = %d\n\n", queueSJF->count);
		printf("# of jobs in PRQ = %d\n\n", queuePQ->count);
		printf("# of jobs in FIFO = %d\n\n", queueFIFO->count);
	}



	return 0;
}
