#include "edBankDebug.h"
#include "edBankBuffer.h"

edCBankDebug edBankDebug = { 1 };

void edCBankDebug::buffer_link(edCBankBuffer* pBankBuffer)
{
	if (pBankBuffer != (edCBankBuffer*)0x0) {
		if (this->bankCount == 1) {
			this->pNextBank->pNextBank = pBankBuffer;
			this->pNextBank->pPrevBank = pBankBuffer;

			pBankBuffer->pNextBank = this->pNextBank;
			pBankBuffer->pPrevBank = this->pNextBank;
		}
		else {
			if (this->bankCount == 0) {
				this->pNextBank = pBankBuffer;
				pBankBuffer->pNextBank = pBankBuffer;
				pBankBuffer->pPrevBank = pBankBuffer;
			}
			else {
				pBankBuffer->pNextBank = this->pNextBank;
				pBankBuffer->pPrevBank = this->pNextBank->pPrevBank;
				this->pNextBank->pPrevBank->pNextBank = pBankBuffer;
				this->pNextBank->pPrevBank = pBankBuffer;
			}
		}

		this->bankCount = this->bankCount + 1;
	}

	return;
}

void edCBankDebug::buffer_unlink(edCBankBuffer* pBankBuffer)
{
	if (pBankBuffer != (edCBankBuffer*)0x0) {
		this->bankCount = this->bankCount + -1;
		pBankBuffer->pNextBank->pPrevBank = pBankBuffer->pPrevBank;
		pBankBuffer->pPrevBank->pNextBank = pBankBuffer->pNextBank;

		if (this->bankCount == 0) {
			this->pNextBank = (edCBankBuffer*)0x0;
		}
		else {
			if (this->pNextBank == pBankBuffer) {
				this->pNextBank = pBankBuffer->pNextBank;
			}
		}

		pBankBuffer->pNextBank = (edCBankBuffer*)0x0;
		pBankBuffer->pPrevBank = (edCBankBuffer*)0x0;
	}

	return;
}