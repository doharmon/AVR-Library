;---------------------------------------------------------------------------;
;
; Transmit
;
; State		Name		Description
;	0		tsTxIDLE	Idle: IR routine disabled
;	1		tsTxSTART	Add delay for start bit.
;  2-8		tsTxBIT0-6	Add delay for data bit.
;	9		tsTxBIT7	Add delay for data bit. OCR output high for stop bit.
;  10		tsTxSTOP	Add delay for stop bit.
;  11		tsTxCHKBUF	Check buffer for next byte.
;
; Data Byte: 0x9B 0b10011011
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   1
; State:		Idl Strt B0  B1  B2  B3  B4  B5  B6  B7 Stop  
;				+---+   +---+---+   +---+---+       +---+---+
;   				|   |       |   |       |       |
;	    			|   |       |   |       |       |
;	    			+---+       +---+       +---+---+
;
;			   	|   |   |   |   |   |   |   |   |   |   |   |
;			   	|   |   |   |   |   |   |   |   |   |   |   +-- tsTxCHKBUF
;			   	|   |   |   |   |   |   |   |   |   |   +-- tsTxSTOP
;			   	|   |   |   |   |   |   |   |   |   +-- tsTxBIT7
;			   	|   |   |   |   |   |   |   |   +-- tsTxBIT6
;			   	|   |   |   |   |   |   |   +-- tsTxBIT5
;			   	|   |   |   |   |   |   +-- tsTxBIT4
;			   	|   |   |   |   |   +-- tsTxBIT3
;			   	|   |   |   |   +-- tsTxBIT2
;			   	|   |   |   +-- tsTxBIT1
;			   	|   |   +-- tsTxBIT0
;			   	|   +-- tsTxSTART
;			   	+-- tsTxIDLE
;
;	NOTE:	Data bits are sent LSB first.
;			Any line level changes happen before IR routine is called.
;
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;
; Receive
;
; State		Name		Description
;	0		tsRxSTART	Waiting for line level to go low (0).
;  1-8		tsRxBIT0-7	Waiting for data bits.
;   9		tsRxSTOP	Waiting for stop bit. Set state to tsRxSTART.
;
; Data Byte: 0x9B 0b10011011
;
; Line Level:     1   0   1   1   0   1   1   0   0   1   1
; State:		    Strt B0  B1  B2  B3  B4  B5  B6  B7 Stop  
;				+---+   +---+---+   +---+---+       +---+---+
;   				|   |       |   |       |       |
;	    			|   |       |   |       |       |
;	    			+---+       +---+       +---+---+
;
;			   	    |   |   |   |   |   |   |   |   |   |
;			   	    |   |   |   |   |   |   |   |   |   +-- tsRxSTOP
;			   	    |   |   |   |   |   |   |   |   +-- tsRxBIT7
;			   	    |   |   |   |   |   |   |   +-- tsRxBIT6
;			   	    |   |   |   |   |   |   +-- tsRxBIT5
;			   	    |   |   |   |   |   +-- tsRxBIT4
;			   	    |   |   |   |   +-- tsRxBIT3
;			   	    |   |   |   +-- tsRxBIT2
;			   	    |   |   +-- tsRxBIT1
;			   	    |   +-- tsRxBIT0
;			   	    +-- tsRxSTART
;
;	NOTE:	Data bits are received LSB first.
;			The IR routine is called after the line level changes.
;
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;
; Transmit
;
; uint16_t  TXSerialBegin(uint32_t baud | void)
; void 		TXSerialEnd(void)
; void		XSerialWrite(uint8_t b)
; void 		XSerialFlushOutput(void)
; uint16_t 	XSerialPending(void)
;
; TX is T0, T1, T2, or T3 depending on which timer is used.
; 
; X is in the form T[Timer][OCR][Ascii]
;	Where:
;		Timer is 0, 1, 2, or 3
;		OCR is A, B, or C
;		Ascii appears only if port is ASCII only
;
; Examples:
;
; #define tsTX (tsconfigTIMER0 | tsconfigOCR_A | tsconfigOCR_B | tsconfigASCII)
;
; uint16_t	T0SerialBegin(void);
; void 		T0SerialEnd(void)
; void		T0AAsciiSerialWrite(uint8_t byte);
; void		T0BAsciiSerialWrite(uint8_t byte);
; void		T0AAsciiSerialFlushOutput(void);
; void		T0BAsciiSerialFlushOutput(void);
; uint16_t	T0AAsciiSerialPending(void);
; uint16_t	T0BAsciiSerialPending(void);
;
;
; #define tsTX (tsconfigTIMER2 | tsconfigOCR_A)
;
; uint16_t	T2SerialBegin(void);
; void 		T2SerialEnd(void)
; void		T2ASerialWrite(uint8_t byte);
; void		T2ASerialFlushOutput(void);
; uint16_t	T2ASerialPending(void);
;
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;
; Receive
;
; uint16_t 	XSerialRead(void)
; uint16_t 	XSerialPeek(void)
; uint16_t 	XSerialAvailable(void)
; void 		XSerialFlushInput(void)
; uint8_t 	XSerialOverflow(void)
;
; X is in the form T[Timer][OCR][PXY][Ascii]
;	Where:
;		Timer is 0, 1, 2, or 3
;		OCR is A, B, or C. Appears only if port is binary and not ASCII only.
;		PXY where:
;			X is A, B, C, or D depending on which port is used
;			Y is 0-7 depending on which port pin is used
;		Ascii appears only if port is ASCII only
;
; Examples:
;
; #define tsRX		(tsconfigTIMER0 | tsconfigPCPORTB | tsconfigASCII)
; #define tsRX_PINS	(tsconfigPCBIT0 | tsconfigPCBIT1)
;
; uint16_t	T0PB0AsciiSerialRead(void);
; uint16_t	T0PB0AsciiSerialPeek(void);
; uint16_t	T0PB0AsciiSerialAvailable(void);
; void		T0PB0AsciiSerialFlushInput(void);
; uint8_t	T0PB0AsciiSerialOverflow(void);
; uint16_t	T0PB1AsciiSerialRead(void);
; uint16_t	T0PB1AsciiSerialPeek(void);
; uint16_t	T0PB1AsciiSerialAvailable(void);
; void		T0PB1AsciiSerialFlushInput(void);
; uint8_t	T0PB1AsciiSerialOverflow(void);
;
;
; #define tsRX	(tsconfigTIMER2 | tsconfigOCR_B | tsconfigINT2)
;
; uint16_t	T2BSerialRead(void);
; uint16_t	T2BSerialPeek(void);
; uint16_t	T2BSerialAvailable(void);
; void		T2BSerialFlushInput(void);
; uint8_t	T2BSerialOverflow(void);
;
;---------------------------------------------------------------------------;

;---------------------------------------------------------------------------;
;
; Config				TX	RX 	RX_PINS
; tsconfigTIMER0-5		x	x
; tsconfigOCR_A-C		x	x
; tsconfigASCII			x	x
; tsconfigICP0-5			x
; tsconfigINT0-7			x
; tsconfigPCPORTA-D			x
; tsconfigPCBIT0-7				x
; 
;---------------------------------------------------------------------------;
