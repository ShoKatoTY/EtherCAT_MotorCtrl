#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ethercat.h"

// PDO���}�b�s���O�菇���֐���
void remap_pdo(uint16 slave) {
    uint16 index;
    uint8 subindex;
    int32 sdo_val;

    // 1. RPDO�̖�����
    index = 0x1600; // ��: 0x1600��RPDO�̃C���f�b�N�X
    subindex = 0x00;
    sdo_val = 0x0000;
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    // 2. RPDO�̃}�b�s���O
    subindex = 0x01;
    sdo_val = 0x60400010; // 0x6040: Controlword, 16bit
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    subindex = 0x02;
    sdo_val = 0x60710010; // 0x6071: Target Torque, 16bit
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    // 3. RPDO�̗L����
    subindex = 0x00;
    sdo_val = 0x02; // �}�b�s���O���ꂽ�I�u�W�F�N�g�̐�
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    // 4. TPDO�̖�����
    index = 0x1A00; // ��: 0x1A00��TPDO�̃C���f�b�N�X
    subindex = 0x00;
    sdo_val = 0x0000;
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    // 5. TPDO�̃}�b�s���O
    subindex = 0x01;
    sdo_val = 0x60410010; // 0x6041: Statusword, 16bit
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    subindex = 0x02;
    sdo_val = 0x60640020; // 0x6064: Position Actual Value, 32bit
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);

    // 6. TPDO�̗L����
    subindex = 0x00;
    sdo_val = 0x02; // �}�b�s���O���ꂽ�I�u�W�F�N�g�̐�
    ec_SDOwrite(slave, index, subindex, FALSE, sizeof(sdo_val), &sdo_val, EC_TIMEOUTRXM);
}

// �X���[�u�����s�\�ȏ�Ԃɂ���֐�
int set_slave_operational(uint16 slave) {
    uint16 controlword = 0x0006; // �X�C�b�`�I���f�B�X�G�[�u��
    uint16 statusword;

    // �X�C�b�`�I���f�B�X�G�[�u����Ԃɐݒ�
    *(uint16*)(ec_slave[slave].outputs) = controlword;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    // �X�e�[�^�X���[�h���m�F
    statusword = *(uint16*)(ec_slave[slave].inputs);
    if ((statusword & 0x004F) != 0x0040) {
        return 0; // �X�C�b�`�I���f�B�X�G�[�u���ɂȂ��Ă��Ȃ�
    }

    // ���f�B�g�D�X�C�b�`�I���ɐݒ�
    controlword = 0x0007;
    *(uint16*)(ec_slave[slave].outputs) = controlword;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    // �X�e�[�^�X���[�h���m�F
    statusword = *(uint16*)(ec_slave[slave].inputs);
    if ((statusword & 0x006F) != 0x0021) {
        return 0; // ���f�B�g�D�X�C�b�`�I���ɂȂ��Ă��Ȃ�
    }

    // �X�C�b�`�I���ɐݒ�
    controlword = 0x000F;
    *(uint16*)(ec_slave[slave].outputs) = controlword;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    // �X�e�[�^�X���[�h���m�F
    statusword = *(uint16*)(ec_slave[slave].inputs);
    if ((statusword & 0x006F) != 0x0023) {
        return 0; // �X�C�b�`�I���ɂȂ��Ă��Ȃ�
    }

    // �I�y���[�V�����C�l�[�u���ɐݒ�
    controlword = 0x001F;
    *(uint16*)(ec_slave[slave].outputs) = controlword;
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    // �X�e�[�^�X���[�h���m�F
    statusword = *(uint16*)(ec_slave[slave].inputs);
    if ((statusword & 0x006F) != 0x0027) {
        return 0; // �I�y���[�V�����C�l�[�u���ɂȂ��Ă��Ȃ�
    }

    return 1; // ����
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        // EtherCAT�l�b�g���[�N�̏�����
        if (ec_init(argv[1])) {
            printf("EtherCAT����������\n");

            // �X�L�����X���[�u
            if (ec_config_init(FALSE) > 0) {
                printf("%d�X���[�u��������܂���\n", ec_slavecount);

                // �X���[�u���Ƃ�PDO���}�b�s���O�����s
                for (int slave = 1; slave <= ec_slavecount; slave++) {
                    remap_pdo(slave);
                }

                // �}�X�^�[�̏�Ԃ��Z�[�t�I�y���[�V���i���ɐݒ�
                ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

                // �}�X�^�[�̏�Ԃ��I�y���[�V���i���ɐݒ�
                ec_slave[0].state = EC_STATE_OPERATIONAL;
                ec_writestate(0);
                ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE);

                if (ec_slave[0].state == EC_STATE_OPERATIONAL) {
                    printf("�}�X�^�[�̓I�y���[�V���i����Ԃł�\n");

                    // �X���[�u�����s�\�ȏ�Ԃɂ���
                    if (set_slave_operational(1)) {
                        printf("�X���[�u�͎��s�\��Ԃł�\n");

                        // �g���N�w�ߒl
                        int16 target_torque = 100; // ��: �g���N�w�ߒl
                        int32 position_actual_value;

                        // PDO�ʐM�̊J�n
                        ec_send_processdata();
                        ec_receive_processdata(EC_TIMEOUTRET);

                        // �g���N�w�߂̑��M�ƈʒu�̓ǂݏo�������[�v�ōs��
                        for (int i = 0; i < 100; i++) {
                            // �R���g���[�����[�h�͏o�̓f�[�^�̍ŏ���2�o�C�g
                            uint16* controlword_ptr = (uint16*)ec_slave[1].outputs;
                            // �g���N�w�߂͏o�̓f�[�^��3, 4�o�C�g�� (�o�C�g�I�t�Z�b�g2)
                            int16* target_torque_ptr = (int16*)(ec_slave[1].outputs + 2);

                            // �X�e�[�^�X���[�h�͓��̓f�[�^�̍ŏ���2�o�C�g
                            uint16* statusword_ptr = (uint16*)ec_slave[1].inputs;
                            // �ʒu�l�͓��̓f�[�^��3, 4, 5, 6�o�C�g�� (�o�C�g�I�t�Z�b�g2)
                            int32* position_actual_value_ptr = (int32*)(ec_slave[1].inputs + 2);

                            // �g���N�w�߂𑗐M
                            *target_torque_ptr = target_torque;

                            // PDO�ʐM�̑���M
                            ec_send_processdata();
                            ec_receive_processdata(EC_TIMEOUTRET);

                            // �ʒu�l�̓ǂݏo��
                            position_actual_value = *position_actual_value_ptr;

                            // �ʒu�l��\��
                            printf("���݂̈ʒu: %d\n", position_actual_value);

                            // 1�b�ҋ@
                            usleep(1000000);
                        }
                    } else {
                        printf("�X���[�u�͎��s�\��Ԃɓ���܂���ł���\n");
                    }
                } else {
                    printf("�}�X�^�[�̓I�y���[�V���i����Ԃɓ���܂���ł���\n");
                }
            } else {
                printf("�X���[�u��������܂���\n");
            }
        } else {
            printf("EtherCAT���������s\n");
        }
    } else {
        printf("�g�p�@: %s <�l�b�g���[�N�C���^�[�t�F�C�X>\n", argv[0]);
    }

    // EtherCAT�l�b�g���[�N�̕�
    ec_close();
    return 0;
}
