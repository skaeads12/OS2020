#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>


typedef struct Node
{
	pid_t data;
	struct Node *next;
} Node;

typedef struct Queue
{
	Node *front;
	Node *rear;
	int count;
} Queue;

void InitQueue(Queue *queue)
{
	queue->front = queue->rear = NULL;
	queue->count = 0;
}

int IsEmpty(Queue *queue)
{
	return queue->count == 0;
}

void Enqueue(Queue *queue, pid_t data)
{
	Node *new = (Node *)malloc(sizeof(Node));
	new->data = data;
	new->next = NULL;

	if(IsEmpty(queue))
	{
		queue->front = new;
	}
	else
	{
		queue->rear->next = new;
	}
	queue->rear = new;
	queue->count++;
}

pid_t Dequeue(Queue *queue)
{
	pid_t data;
	Node *node;

	if(IsEmpty(queue))
		return 0;

	node = queue->front;
	data = node->data;
	queue->front = node->next;
	free(node);
	queue->count--;

	printf("dequeue ==> pid: %ld\n", (long) getpid());

	return data;
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
/*	while(preemptionSJF != 0 && preemptionSJF != 1)
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
*/
	/* alpha coefficient  */
/*	printf("Enter the alpha co-eff for RR: ");
	scanf("%d", &alphaRR);

	printf("Enter the alpha co-eff for SJF: ");
	scanf("%d", &alphaSJF);

	printf("Enter the alpha co-eff for PQ: ");
	scanf("%d", &alphaPQ);

	printf("Enter the alpha co-eff for FIFO: ");
	scanf("%d", &alphaFIFO);
*/
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
/*	printf("Enter the initial estimated time for RR: ");
	scanf("%d", &estimateRR);

	printf("Enter the initial estimated time for SJF: ");
	scanf("%d", &estimateSJF);

	printf("Enter the initial estimated time for PQ: ");
	scanf("%d", &estimatePQ);

	printf("Enter the initial estimated time for FIFO: ");
	scanf("%d", &estimateFIFO);
*/
	while(timeCycle > 0)
	{
		printf("timeCycle: %d\n", timeCycle);
		
		timeCycle--;
	}

	Queue queueRR;
	InitQueue(&queueRR);

	pid_t pid = fork();
	if(pid == 0)
	{
		Enqueue(&queueRR, pid);

		printf("queue count: %d\n", queueRR.count);
		
		pid_t pid2 = Dequeue(&queueRR);
		printf("%ld\n",(long) getpid());
	}

	return 0;
}
