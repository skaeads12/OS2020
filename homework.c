#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RR 0
#define SJF 1
#define PQ 2
#define FIFO 3
#define SERVE 4


/* initial queue */
typedef struct Process
{
	int pid;
	int burstTime;
	int state;
	int priority; // 경우에 따라서 사용 x

	struct Process *next; // program counter
} Process;

/* new state queue and 4 ready queue */
typedef struct Queue
{
	Process *front;
	Process *rear;
	int count;
	int state;

	int isPreemptive;
	int quantum;
} Queue;

void InitQueue(Queue *);
void Enqueue(Queue *, int, int);
Process *Dequeue(Queue *);
void ServiceProcess(void);
int BurstTimeCal(int, int);
void CreateProcess(Queue *, int, int);
void SelectProcess(void);
void CheckQueue(Queue *);

int lastPid = 0;

void CreateProcess(Queue *queue, int alpha, int estimate)
{
	srand(time(NULL));

	int random = rand() % 2;

	if(random > -1)
	{
		int pid = ++lastPid;

		int burst = BurstTimeCal(alpha, estimate);
		Enqueue(queue, pid, burst);
	}
	else
	{
		return;
	}
}

int BurstTimeCal(int alpha, int estimate)
{
	float coef = 0.5;

	srand(time(NULL));
	int random = rand() % 10;
	random -= 5;

	float result = coef * (float) alpha + (1 - coef) * (float) estimate + (float) random;	

	int resultInt = (int) result;

	printf("[bustTimeCal] alpha: %.1f, estimate: %.1f, result: %.3f\n", (float) alpha, (float) estimate, result);

	return (int) result;
}

void InitQueue(Queue *queue)
{
	queue->front = queue->rear = NULL;
	queue->count = 0;
	queue->state = -1;
}

int IsEmpty(Queue *queue)
{
	return queue->count == 0;
}

void Enqueue(Queue *queue, int pid, int burstTime)
{
	Process *new = (Process *)malloc(sizeof(Process));
	new->pid = pid;
	new->burstTime = burstTime;
	new->next = NULL;
	new->state = queue->state;

	queue->count++;

	if(IsEmpty(queue))
	{
		queue->front = new;
		queue->rear = new;
		
		return;
	}

	Process *prev = NULL;
	Process *current = queue->front;

	switch(queue->state)
	{
		case RR:
			queue->rear->next = new;
			queue->rear = new;
			break;
		case SJF:
			while(current)
			{
				if(current->burstTime < new->burstTime)
				{
					prev = current;
					current = current->next;
				}
				else
					break;
			}

			if(prev == NULL)
			{
				new->next = current;
				queue->front = new;
			}
			else
			{
				new->next = current;
				prev->next = new;
			}
			printf("[ENQUEUE]\npid: %d, burstTime: %d, state: %d\n", new->pid, new->burstTime, new->state);

			break;
		case PQ:
			break;
		case FIFO:
			if(!IsEmpty(queue))
				queue->rear->next = new;
			queue->rear = new;
			break;
	}
}

Process *Dequeue(Queue *queue)
{
	int pid;
	Process *process;

	if(IsEmpty(queue))
		return 0;

	process = queue->front;
	queue->front = process->next;
	queue->count--;

	printf("[DEQUEUE]\npid: %d\n", process->pid);

	return process;
}

void PrintQueue(Queue *queue)
{
	Process *process = queue->front;

	printf("[PRINT]\n");

	for(int i = 0; i < queue->count; i++)
	{
		if(process != NULL)
		{
			printf("[pid: %d] = %d,", process->pid, process->burstTime);
			process = process->next;
		}
		else
			break;
	}
	printf("\n");
}

int main(void)
{
	int timeCycle;
	int quantumRR;
	int contextSwitchingTime;
	
	int preemptionSJF = -1;		//1 - yes
	int preemptionPQ = -1;		//0 - no

	/* alpha: previously measured value. */
	int alphaRR;
	int alphaSJF;
	int alphaPQ;
	int alphaFIFO;

	/* parameter for aging threshold. */
	int agingRR;
	int agingSJF;
	int agingPQ;
	int agingFIFO;

	/* previous estimated values used to estimate the following values. */
	int estimateRR;
	int estimateSJF;
	int estimatePQ;
	int estimateFIFO;

	printf("Enter the time cycles: ");
	scanf("%d", &timeCycle);
/*
	printf("Enter the vaule of time slot for RR: ");
	scanf("%d", &quantumRR);

	printf("Enter the context switching time: ");
	scanf("%d", &contextSwitchingTime);
*/
	/* if it's not 0 or 1, don't accept it. */
	while(preemptionSJF != 0 && preemptionSJF != 1)
	{
		printf("SJF with pre-emption (1-yes/0-no): ");
		scanf("%d", &preemptionSJF);

		if(preemptionSJF != 0 && preemptionSJF != 1)
			printf("Choose from 0 or 1.\n");
	}

	while(preemptionPQ != 0 && preemptionPQ != 1)
	{
		printf("PQ with pre-emption (1-yes/0-no): ");
		scanf("%d", &preemptionPQ);

		if(preemptionPQ != 0 && preemptionPQ != 1)
			printf("Choose from 0 or 1.\n");

	}

	/* alpha coefficient  */
	printf("Enter the alpha co-eff for RR: ");
	scanf("%d", &alphaRR);

	printf("Enter the alpha co-eff for SJF: ");
	scanf("%d", &alphaSJF);

	printf("Enter the alpha co-eff for PQ: ");
	scanf("%d", &alphaPQ);

	printf("Enter the alpha co-eff for FIFO: ");
	scanf("%d", &alphaFIFO);

	/* aging threshold */
/*	printf("Enter the aging time for RR: ");
	scanf("%d", &agingRR);

	printf("Enter the aging time for SJF: ");
	scanf("%d", &agingSJF);

	printf("Enter the aging time for PQ: ");
	scanf("%d", &agingPQ);

	printf("Enter the aging time for FIFO: ");
	scanf("%d", &agingFIFO);
*/
	/* previous estimated values */
	printf("Enter the initial estimated time for RR: ");
	scanf("%d", &estimateRR);

	printf("Enter the initial estimated time for SJF: ");
	scanf("%d", &estimateSJF);

	printf("Enter the initial estimated time for PQ: ");
	scanf("%d", &estimatePQ);

	printf("Enter the initial estimated time for FIFO: ");
	scanf("%d", &estimateFIFO);

	Queue queueRR;
	InitQueue(&queueRR);
	queueRR.quantum = quantumRR;
	queueRR.state = RR;

	Queue queueSJF;
	InitQueue(&queueSJF);
	queueSJF.isPreemptive = preemptionSJF;
	queueSJF.state = SJF;

	Queue queuePQ;
	InitQueue(&queuePQ);
	queuePQ.isPreemptive = preemptionPQ;
	queuePQ.state = PQ;

	Queue queueFIFO;
	InitQueue(&queueFIFO);
	queueFIFO.state = FIFO;

	srand((unsigned int) time(NULL));

	while(timeCycle > 0)
	{
		printf("time cycle: %d\n", timeCycle);	

		switch(rand() % 4) // The initial process enters a random queue.
		{
			case 0:
				printf("case 0 execution.\n");
				CreateProcess(&queueRR, alphaRR, estimateRR);
				break;
			case 1:
				printf("case 1 execution.\n");
				CreateProcess(&queueSJF, alphaSJF, estimateSJF);
				
				break;
			case 2:
				printf("case 2 execution.\n");
				CreateProcess(&queuePQ, alphaPQ, estimatePQ);
				
				break;
			case 3:
				printf("case 3 execution.\n");
				CreateProcess(&queueFIFO, alphaFIFO, estimateFIFO);
				
				break;
		}

		timeCycle--;
	}

	return 0;
}
