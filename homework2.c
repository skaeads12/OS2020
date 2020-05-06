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
int switchingTime = 0; // 스위칭 하는데 걸리는 시간

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

	int state;
	int count;
} Queue;

void GenerateBurstTime(Process *);
void GeneratePriority(Process *);

Process *CreateProcess(void)
{
	int pid = ++lastPid;

	Process *new = (Process *)malloc(sizeof(Process));
	new->pc = NULL;
	new->pid = pid;
	new->state = -1;
	new->priority = -1;
	new->burstTime = -1;

	return new;
}

Queue *CreateQueue(void)
{
	Queue *new = (Queue *)malloc(sizeof(Queue));

	new->front = NULL;
	new->rear = NULL;
	new->count = 0;

	return new;
}

int IsEmptyQueue(Queue *queue)
{
	return queue->count == 0;
}

void Enqueue_RR_FIFO(Queue *queue, Process *process)
{
	printf("PROCESS IN RR OR FIFO.\n");

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

	printf("PROCESS IN SJF.\n");

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
	
	printf("PROCESS IN PQ.\n");

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

Process *Dequeue(Queue *queue)
{
	Process *process = queue->front;
	queue->front = process->pc;

	return process;
}

void PrintQueue(Queue *queue)
{
	Process *process = queue->front;
	
	for(int i = 0; i < queue->count; i++)
	{
		printf("[%d]: %d\n", process->pid, process->burstTime);
		process = process->pc;
	}
}

void PrintQueue2(Queue *queue)
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

	printf("COEFFIENT: %f\n", coef);

	float result = coef * alpha + (1 - coef) * estimate;

	if(result < 0)
		result *= -1;
	else if(result == 0)
		result = 1;

	process->burstTime = (int) result;

	printf("GENERATED BURST TIME: %d\n", (int) result);
}

void GeneratePriority(Process *process)
{
	int random = rand() % 100;

	process->priority = random;

	printf("GENERATING PRIORITY: %d\n", random);
}

int Service(Process *process)
{
	process->burstTime--;
	if(!process->burstTime) // burstTime이 0?
	{
		serving = 0;
	}
}

int Select(Queue *queue) // 1: 여기서 선택, 0: 선택할 거 없음
{
	if(queue->front != NULL)
	{
		Process *servedProcess = queue->front;
		Service(queue->front);
		return 1;
	}
	else
		return 0;
}

int SwitchingContext(Process *exist, Process *new)
{
	
}

int main(void)
{
	Queue *queueRR = CreateQueue();
	Queue *queueSJF = CreateQueue();
	Queue *queuePQ = CreateQueue();
	Queue *queueFIFO = CreateQueue();

	Process *servingProcess;

	srand(time(NULL));

	for(int i = 0; i < 10; i++)
	{
		printf("------------\n[CYCLE TIME: %d]\n------------\n", i);

		if(rand() % 2 == 0)
		{
			printf("PROCESS CREATE.\n");
			switch(rand() % 4)
			{
				case 0:
					Enqueue_RR_FIFO(queueRR, CreateProcess());
					break;
				case 1:
					Enqueue_SJF(queueSJF, CreateProcess());
					break;
				case 2:
					Enqueue_PQ(queuePQ, CreateProcess());
					break;
				case 3:
					Enqueue_RR_FIFO(queueFIFO, CreateProcess());
					break;
			}
		}

		if(needSwitching) // 스위칭을 해야 한다면
		{
			if(switchingTime)
			{
				switchingTime--;
				continue;
			}
			else
			{
				needSwitching = 0;
			}
		}
		
		if(!serving)
		{
			if(!Select(queueRR))
				if(!Select(queueSJF))
					if(!Select(queuePQ))
						Select(queueFIFO)
		}
		else
		{
			Service(servingProcess);
		}
	}

	return 0;
}
