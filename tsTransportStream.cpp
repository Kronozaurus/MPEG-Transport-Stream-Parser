#include "tsTransportStream.h"
#include <stdio.h>
#include <string.h>


//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_PacketHeader::Reset() {
     SB, E, S, T, PID, TSC, AFC, CC = 0;
}

int32_t xTS_PacketHeader::Parse(const uint8_t* Input) {
        uint32_t maskSB =   0b11111111000000000000000000000000;
        uint32_t maskE =    0b00000000100000000000000000000000;
        uint32_t maskS =    0b00000000010000000000000000000000;
        uint32_t maskT =    0b00000000001000000000000000000000;
        uint32_t maskPID =  0b00000000000111111111111100000000;
        uint32_t maskTSC =  0b00000000000000000000000011000000;
        uint32_t maskAFC =  0b00000000000000000000000000110000;
        uint32_t maskCC =   0b00000000000000000000000000001111;
        uint32_t Header = *((uint32_t*)Input);
        uint32_t HeaderData = xSwapBytes32(Header);
        SB = (HeaderData&maskSB)>>24;
        E = (HeaderData&maskE)>>23;
        S = (HeaderData&maskS)>>22;
        T = (HeaderData&maskT)>>21;
        PID = (HeaderData&maskPID)>>8;
        TSC = (HeaderData&maskTSC)>>6;
        AFC = (HeaderData&maskAFC)>>4;
        CC = HeaderData&maskCC;
}
void xTS_PacketHeader::Print() const {
    printf("TS: SB=%d E=%d S=%d T=%d PID=%d TSC=%d AFC=%d CC=%d", SB, E, S, T, PID, TSC, AFC, CC);
}

bool xTS_PacketHeader::hasAdaptationField() const {
    return ((AFC == 2) || (AFC == 3))? true : false;
}

//=============================================================================================================================================================================
// xTS_PacketHeader
//=============================================================================================================================================================================
void xTS_AdaptationField::Reset() {
    AFC, AF, DC, RA, SP, PCR, OR, SF, TP, EX = 0;
}

int32_t xTS_AdaptationField::Parse(const uint8_t* Input, uint8_t AdaptationFieldControl) {
    
    AFC = AdaptationFieldControl;
    if (AFC == 2 || AFC == 3) {
        AF = Input[4];
        Stuffing = AF - 1;
    }
    else {
        AF = 0;
        Stuffing = 0;
    }
   
	uint8_t var = Input[5];
	EX = var & 1;
	TP = (var >> 1) & 1;
	SF = (var >> 2) & 1;
	OR = (var >> 3) & 1;
	PCR = (var >> 4) & 1;
	SP = (var >> 5) & 1;
	RA = (var >> 6) & 1;
	DC = (var >> 7) & 1;

    /*
    int pointer = 6;
    if (PCR) {
        PCR = convertToUint64(0,0,0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        PCR_base >>= 7;

        PCR_reserved = (Input[pointer] & 0x7E);
        PCR_reserved >>= 1;
        PCR_extension = convertToUint16((Input[pointer] & 0x1),Input[++pointer]);
        Stuffing -= 6;
    }
    if (OR) {
        OPCR_base = convertToUint64(0, 0, 0, Input[pointer], Input[++pointer], Input[++pointer], Input[++pointer], Input[++pointer]);
        OPCR_base >>= 7;

        OPCR_reserved = (Input[pointer] & 0x7E);
        OPCR_reserved >>= 1;
        OPCR_extension = convertToUint16((Input[pointer] & 0x1), Input[++pointer]);
        Stuffing -= 6;
    }
    */
    return convertToUint32(AF, Input[5]);
}

void xTS_AdaptationField::Print() const {
    printf("AF: AF=%d DC=%d RA=%d SP=%d PR=%d OR=%d SF=%d TP=%d EX=%d\n",AF, DC, RA, SP,PCR, OR, SF, TP, TP);


    if(PCR){ 
        printf("\n\n\nPCR=%d ", (PCR_base * xTS::BaseToExtendedClockMultiplier + PCR_extension)); }
    if (OR) { 
        printf("\n\nOPCR=%d ", (OPCR_base * xTS::BaseToExtendedClockMultiplier + OPCR_extension)); }
}

//=============================================================================================================================================================================
// xPES_PacketHeader
//=============================================================================================================================================================================
void xPES_PacketHeader::Reset() {
    m_PacketStartCodePrefix, m_StreamId, m_PacketLength, 
        PTS_DTS_flags = 0, PTS, DTS = 0;
}

int32_t xPES_PacketHeader::Parse(const uint8_t* Input) {
    m_PacketStartCodePrefix = convertToUint24(Input[0], Input[1], Input[2]);
    m_StreamId = Input[3];
    m_PacketLength = convertToUint16(Input[4], Input[5]);

    if (!m_HeaderLength) {
        m_HeaderLength = 6;
    }

    if (m_StreamId == 0xBD || (m_StreamId >= 0xC0 && m_StreamId <= 0xEF)) {
        m_HeaderLength = 9;

        PTS_DTS_flags = (Input[7] & 0xC0) >> 6;
        

        if ((Input[7] & 0xC0) == 0xC0) {
            m_HeaderLength += 10;

            PTS = Input[9] & 0xE;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0xFE;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;

            DTS = Input[14] & 0xE;
            DTS <<= 7;
            DTS += Input[15];
            DTS <<= 8;
            DTS += Input[16] & 0xFE;
            DTS <<= 7;
            DTS += Input[17];
            DTS <<= 7;
            DTS += Input[18] >> 1;
        }
        else if ((Input[7] & 0x80) == 0x80) {
            m_HeaderLength += 5;

            PTS = Input[9] & 0xE;
            PTS <<= 7;
            PTS += Input[10];
            PTS <<= 8;
            PTS += Input[11] & 0xFE;
            PTS <<= 7;
            PTS += Input[12];
            PTS <<= 7;
            PTS += Input[13] >> 1;
        }
        

        if (Input[7] & 0x20) {
            m_HeaderLength += 6;
        }

        if (Input[7] & 0x10) {
            m_HeaderLength += 3;
        }

        if (Input[7] & 0x8) {
            m_HeaderLength += 0;
        }

        if (Input[7] & 0x4) {
            m_HeaderLength += 1;
        }

        if (Input[7] & 0x2) {
            m_HeaderLength += 2;
        }

        if (Input[7] & 0x1) {
            int point = m_HeaderLength;
            m_HeaderLength += 1;

            if (Input[point] & 0x80) {
                m_HeaderLength += 16;
            }

            if (Input[point] & 0x40) {
                m_HeaderLength += 1;
            }

            if (Input[point] & 0x20) {
                m_HeaderLength += 2;
            }

            if (Input[point] & 0x10) {
                m_HeaderLength += 2;
            }

            if (Input[point] & 0x1) {
                m_HeaderLength += 2;
            }
            
        }
    }

    return m_PacketStartCodePrefix;
}


void xPES_PacketHeader::Print() const {
    printf("PES: ,PSCP=%d ,SID=%d ,L=%d ",m_PacketStartCodePrefix,m_StreamId,m_PacketLength);
    
    /*

    if (PTS_DTS_flags == 0b10)
        printf("PTS=%d ", PTS);
       

    if (PTS_DTS_flags == 0b11) {
        printf("PTS=%d ", PTS);
        printf("DTS=%d ", DTS);
        
    }
    */
}


//=============================================================================================================================================================================
// xPES_Assembler
//=============================================================================================================================================================================
xPES_Assembler::xPES_Assembler() { }

xPES_Assembler::~xPES_Assembler() { delete[] m_Buffer; }

void xPES_Assembler::Init(int32_t PID) {
    m_PID = PID;
    m_Buffer = new uint8_t[100000];
    m_BufferSize = 0;
    m_LastContinuityCounter = 0;
    m_Started = false;
}
xPES_Assembler::eResult xPES_Assembler::AbsorbPacket(const uint8_t* TransportStreamPacket, const xTS_PacketHeader* PacketHeader, const xTS_AdaptationField* AdaptationField) {
    uint8_t TS_AdaptationLenght = 0;
    if (PacketHeader->hasAdaptationField()) {
        TS_AdaptationLenght = AdaptationField->getNumBytes();
    }

    uint32_t tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;


    if (PacketHeader->getPayloadUnitStartIndicator()) {
        tempSize = xTS::TS_PacketLength - xTS::TS_HeaderLength - TS_AdaptationLenght;
        uint32_t sizeToSkip = xTS::TS_PacketLength - tempSize;
        uint8_t* firstPacketBuffer = new uint8_t[188];

        for (int i = 0; i < tempSize; i++) {
            firstPacketBuffer[i] = TransportStreamPacket[sizeToSkip + i];
        }
        m_PESH.Parse(firstPacketBuffer);

        tempSize -= getHeaderLenght();

        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingStarted;
    }
    else if (PacketHeader->hasAdaptationField()) {
        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingFinished;
    }
    else {
        xPES_Assembler::xBufferAppend(TransportStreamPacket, tempSize);
        return eResult::AssemblingContinue;
    }
}

void xPES_Assembler::xBufferReset() {
    m_PESH.Reset();
    memset(m_Buffer, 0, sizeof(m_Buffer));
    m_BufferSize = 0;
}

void xPES_Assembler::xBufferAppend(const uint8_t* data, uint32_t size) {
    uint32_t sizeToSkip = xTS::TS_PacketLength - size;

    for (int i = 0; i < size; i++) {
        m_Buffer[m_BufferSize + i] = data[sizeToSkip + i];
    }
    m_BufferSize += size;
}

void xPES_Assembler::assemblerPes(const uint8_t* TS_PacketBuffer, const xTS_PacketHeader* TS_PacketHeader, 
    const xTS_AdaptationField* TS_AdaptationField, FILE* File) {

    xPES_Assembler::eResult result = AbsorbPacket(TS_PacketBuffer, TS_PacketHeader, TS_AdaptationField);
    switch (result)
    {
        case xPES_Assembler::eResult::AssemblingStarted: {
            printf("Assembling Started: \n"); 
            PrintPESH(); 
            break;
        }
        case xPES_Assembler::eResult::AssemblingContinue: {
            printf("Assembling Continue: "); 
            break; 
        }
        case xPES_Assembler::eResult::AssemblingFinished: {
            printf("Assembling Finished: \n"); 
            printf("PES: PcktLen=%d HeadLen=%d DataLen=%d\n", getNumPacketBytes(), getHeaderLenght(), getNumPacketBytes() - getHeaderLenght());
            saveBufferToFile(File);
            break;
        }
        case xPES_Assembler::eResult::StreamPackedLost: {
            printf("Packet Lost");
            break;
        }
        case xPES_Assembler::eResult::UnexpectedPID: {
            printf("Unexpected PID");
            break;
        }
        default: break;
    }
}

void xPES_Assembler::saveBufferToFile(FILE* AudioMP3) {
    fwrite(getPacket(), 1, getNumPacketBytes(), AudioMP3);
    xBufferReset();
}
/*Pobierane są wartości PacketStartCodePrefix, StreamId i PacketLength z odpowiednich pozycji wejściowych danych.

Jeśli HeaderLength nie zostało wcześniej ustawione, ustawiane jest na wartość 6.

Jeśli StreamId jest równy 0xBD lub mieści się w zakresie od 0xC0 do 0xEF, oznacza to, że nagłówek PES istnieje i należy go analizować. W takim przypadku ustawiana jest wartość HeaderLength na 9.

Odczytywane są flagi PTS_DTS_flags z pozycji 7 wejściowych danych.

Jeśli flaga PTS_DTS_flags ma wartość 0xC0, oznacza to, że obecne są zarówno pola PTS (Presentation Time Stamp) i DTS (Decoding Time Stamp). Odczytywane są wartości PTS i DTS z odpowiednich pozycji wejściowych danych za pomocą przesunięć bitowych i maskowania.

Jeśli flaga PTS_DTS_flags ma wartość 0x80, oznacza to, że obecne jest tylko pole PTS. Odczytywana jest wartość PTS z odpowiednich pozycji wejściowych danych za pomocą przesunięć bitowych i maskowania.

Następnie sprawdzane są kolejne flagi w celu ustalenia długości nagłówka (HeaderLength). Jeśli flaga jest ustawiona, odpowiednia wartość jest dodawana do HeaderLength.

Jeśli ostatnia flaga ma wartość 1, oznacza to, że obecne są dodatkowe pola rozszerzonego nagłówka. Odczytywane są kolejne wartości zależnie od ustawionych bitów w tym polu, a odpowiednie wartości są dodawane do HeaderLength.

*/
