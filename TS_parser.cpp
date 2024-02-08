#include "tsCommon.h"
#include "tsTransportStream.h"

int main(int argc, char *argv[], char *envp[]) {
    // Open the Transport Stream file for reading
    FILE* TransportStreamFile = fopen("../example_new.ts", "rb");

    // Open the Audio MP3 file for writing
    FILE* AudioMP2 = fopen("../Audio.mp2", "wb");

    // Check if the Transport Stream file was opened successfully
    if (TransportStreamFile == NULL) {
        printf("incorrect source of file\n");
        return EXIT_FAILURE;
    }
    
    // Buffer to store the Transport Stream packets
    uint8_t TS_PacketBuffer[xTS::TS_PacketLength];

    // Object to hold the Transport Stream packet header
    xTS_PacketHeader TS_PacketHeader;

    // Object to hold the Adaptation Field of the Transport Stream packet
    xTS_AdaptationField TS_AdaptationField;

    // Object to handle the PES assembly for PID 136
    xPES_Assembler PES_Assembler136;

    // Initialize the PES assembler for PID 136
    PES_Assembler136.Init(136);

    // Counter to track the Transport Stream packet ID
    int TS_PacketId = 0;

    // Read the Transport Stream packets until the end of the file
    while (!feof(TransportStreamFile)) {
        
        // Read a Transport Stream packet into the buffer
        size_t NumRead = fread(TS_PacketBuffer, 1, xTS::TS_PacketLength, TransportStreamFile);

        // If the number of bytes read is not equal to the length of a Transport Stream packet, break the loop
        if (NumRead != xTS::TS_PacketLength) {
            break;
        }

        // Reset the Transport Stream packet header
        TS_PacketHeader.Reset();

        // Parse the Transport Stream packet header from the buffer
        TS_PacketHeader.Parse(TS_PacketBuffer);

        // Reset the Adaptation Field
        TS_AdaptationField.Reset();

        // Check if the Transport Stream packet has the correct sync byte and PID 136
        if (TS_PacketHeader.getSyncByte() == 'G' && TS_PacketHeader.getPID() == 136) {
            //printf("%d ",TS_PacketId);
            // If the Transport Stream packet has an Adaptation Field, parse it
            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Parse(TS_PacketBuffer, TS_PacketHeader.getAdaptionField());
            }

            // Print the Transport Stream packet ID
            printf("%d ", TS_PacketId);

            // Print the Transport Stream packet header
            TS_PacketHeader.Print();

            // Print the Adaptation Field if present
            printf("\n");
            if (TS_PacketHeader.hasAdaptationField()) {
                TS_AdaptationField.Print();
            }

            // Assemble the PES packet for PID 136
            if (TS_PacketHeader.getPID() == 136) {
                PES_Assembler136.assemblerPes(TS_PacketBuffer, &TS_PacketHeader, &TS_AdaptationField, AudioMP2);
                
            }
        }

        // Increment the Transport Stream packet ID
        TS_PacketId++;
        //if(TS_PacketId>100)
         //   break;
        
    }
    
    // Close the Transport Stream file
    fclose(TransportStreamFile);

    // Close the Audio MP3 file
    fclose(AudioMP2);

    return 0;
}

