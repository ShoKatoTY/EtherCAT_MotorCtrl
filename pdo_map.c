// PDO�}�b�s���O��\������֐�
void print_pdo_mapping(uint16 slave)
{
    int i, j;
    uint16 index;
    int psize;
    uint8 bitlength;
    
    // RxPDO�}�b�s���O�̕\��
    printf("RxPDO Mapping:\n");
    for (i = 0; i < ec_slave[slave].Obytes; i++)
    {
        index = ec_slave[slave].SM[2].StartAddr + i;
        psize = ec_slave[slave].SM[2].SMlength;
        bitlength = ec_slave[slave].Osize[i];
        printf("  Index: 0x%04X, BitLength: %d\n", index, bitlength);
    }

    // TxPDO�}�b�s���O�̕\��
    printf("TxPDO Mapping:\n");
    for (i = 0; i < ec_slave[slave].Ibytes; i++)
    {
        index = ec_slave[slave].SM[3].StartAddr + i;
        psize = ec_slave[slave].SM[3].SMlength;
        bitlength = ec_slave[slave].Isize[i];
        printf("  Index: 0x%04X, BitLength: %d\n", index, bitlength);
    }
}