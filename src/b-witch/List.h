#ifndef LIST_H
#define LIST_H

#include "Types.h"

template <typename NodeType>
class CDoubleLinkedNode
{
public:
	CDoubleLinkedNode()
	{
		this->pNext = (CDoubleLinkedNode<NodeType>*)0x0;
		this->pPrev = (CDoubleLinkedNode<NodeType>*)0x0;
	}

	NodeType node;
	CDoubleLinkedNode<NodeType>* pNext;
	CDoubleLinkedNode<NodeType>* pPrev;
};

template<typename NodeType>
class CDoubleLinkedList
{
public:
	CDoubleLinkedList()
		: pHead((CDoubleLinkedNode<NodeType>*)0x0)
		, pTail((CDoubleLinkedNode<NodeType>*)0x0)
	{

	}

	CDoubleLinkedNode<NodeType>* RemoveHead()
	{
		CDoubleLinkedNode<NodeType>* pHead;

		pHead = this->pHead;

		if (pHead != (CDoubleLinkedNode<NodeType>*)0x0) {
			if (pHead->pPrev == (CDoubleLinkedNode<NodeType>*)0x0) {
				this->pTail = (CDoubleLinkedNode<NodeType>*)0x0;
			}
			else {
				pHead->pPrev->pNext = (CDoubleLinkedNode<NodeType>*)0x0;
			}

			this->pHead = this->pHead->pPrev;
		}

		return pHead;
	}

	CDoubleLinkedNode<NodeType>* RemoveNode(CDoubleLinkedNode<NodeType>* pNode)
	{
		if (pNode->pNext == (CDoubleLinkedNode<NodeType>*)0x0) {
			this->pHead = pNode->pPrev;
		}
		else {
			pNode->pNext->pPrev = pNode->pPrev;
		}

		if (pNode->pPrev == (CDoubleLinkedNode<NodeType> *)0x0) {
			this->pTail = pNode->pNext;
		}
		else {
			pNode->pPrev->pNext = pNode->pNext;
		}

		return pNode;
	}

	void InsertFront(CDoubleLinkedNode<NodeType>* pNode)
	{
		pNode->pNext = (CDoubleLinkedNode<NodeType> *)0x0;
		pNode->pPrev = this->pHead;

		if (this->pHead == (CDoubleLinkedNode<NodeType> *)0x0) {
			this->pTail = pNode;
		}
		else {
			this->pHead->pNext = pNode;
		}

		this->pHead = pNode;

		return;
	}

	void InsertAfterQueue(CDoubleLinkedNode<NodeType>* pNode)
	{
		pNode->pNext = this->pTail;
		pNode->pPrev = (CDoubleLinkedNode<NodeType> *)0x0;

		if (this->pTail == (CDoubleLinkedNode<NodeType> *)0x0) {
			this->pHead = pNode;
		}
		else {
			this->pTail->pPrev = pNode;
		}

		this->pTail = pNode;

		return;
	}

	void InsertBefore(CDoubleLinkedNode<NodeType>* pNodeA, CDoubleLinkedNode<NodeType>* pNodeB)
	{
		if (pNodeB == (CDoubleLinkedNode<NodeType>*)0x0) {
			pNodeA->pNext = (CDoubleLinkedNode<NodeType>*)0x0;
			pNodeA->pPrev = this->pHead;

			if (this->pHead == (CDoubleLinkedNode<NodeType>*)0x0) {
				this->pTail = pNodeA;
			}
			else {
				this->pHead->pNext = pNodeA;
			}

			this->pHead = pNodeA;
		}
		else {
			if (pNodeB->pNext != (CDoubleLinkedNode<NodeType> *)0x0) {
				pNodeB->pNext->pPrev = pNodeA;
			}

			pNodeA->pNext = pNodeB->pNext;
			pNodeA->pPrev = pNodeB;
			pNodeB->pNext = pNodeA;

			if (this->pHead == pNodeB) {
				this->pHead = pNodeA;
			}
		}

		return;
	}

	void InsertBeforeHead(CDoubleLinkedNode<NodeType>* pNode)
	{
		pNode->pNext = (CDoubleLinkedNode<NodeType>*)0x0;
		pNode->pPrev = this->pHead;
		if (this->pHead == (CDoubleLinkedNode<NodeType>*)0x0) {
			this->pTail = pNode;
		}
		else {
			this->pHead->pNext = pNode;
		}

		this->pHead = pNode;

		return;
	}

	CDoubleLinkedNode<NodeType>* pHead;
	CDoubleLinkedNode<NodeType>* pTail;
};

template<typename NodeType>
class CSimpleLinkedNode
{
public:
	NodeType node;
	CSimpleLinkedNode<NodeType>* pNext;
};

template<typename NodeType>
class CSimpleLinkedList
{
public:
	CSimpleLinkedList()
	{
		this->pHead = (CSimpleLinkedNode<NodeType>*)0x0;
		this->pTail = (CSimpleLinkedNode<NodeType>*)0x0;

		return;
	}

	void InsertAfterQueue(CSimpleLinkedNode<NodeType>* pNode)
	{
		pNode->pNext = this->pTail;

		if (this->pTail == (CSimpleLinkedNode<NodeType>*)0x0) {
			this->pHead = pNode;
		}
		else {
			this->pTail->pNext = pNode;
		}

		this->pTail = pNode;

		return;
	}

	void RemoveHead()
	{
		if (this->pHead != (CSimpleLinkedNode<NodeType>*)0x0) {
			if (this->pHead->pNext == (CSimpleLinkedNode<NodeType>*)0x0) {
				this->pTail = (CSimpleLinkedNode<NodeType>*)0x0;
			}
			this->pHead = this->pHead->pNext;
		}

		return;
	}

	CSimpleLinkedNode<NodeType>* pHead;
	CSimpleLinkedNode<NodeType>* pTail;
};

#endif // LIST_H
