2024/5/27�����iDS-402 & �A�v���P�[�V�����}�j���A���D�荞�ݍς�

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ecrt.h>

// EtherCAT�}�X�^�[�C���X�^���X
static ec_master_t *master = NULL;
static ec_domain_t *domain1 = NULL;
static ec_slave_config_t *sc = NULL;

// PDO entry offsets
static unsigned int off_control_word;
static unsigned int off_position_actual_value;
static unsigned int off_digital_inputs;
static unsigned int off_status_word;
static unsigned int off_target_torque;
static unsigned int off_mode_of_operation;

// ��`
#define VENDOR_ID 0x00000002
#define PRODUCT_ID 0x00030924
#define REVISION_NO 0x00000001

// �X�V�����i��F1ms�j
#define PERIOD_NS (1000000)

void cyclic_task(void)
{
    // Domain�f�[�^
    uint8_t *domain1_pd = ec_domain_data(domain1);

    // ���䃏�[�h�ƃg���N�w�߂�ݒ�
    EC_WRITE_U16(domain1_pd + off_control_word, 0x000F); // ���䃏�[�h�i��F�^�]�C�l�[�u���j
    EC_WRITE_S16(domain1_pd + off_target_torque, 100);  // �g���N�w�߁i��F100�j

    // �T�C�N���b�N�����g���N���[�h��ݒ�
    EC_WRITE_S8(domain1_pd + off_mode_of_operation, 0x0A); // ���[�h�ݒ�iCST�j

    // Process Data���X�V
    ecrt_domain_queue(domain1);
    ecrt_master_send(master);
    ecrt_master_receive(master);
    ecrt_domain_process(domain1);
}

int main(int argc, char **argv)
{
    // EtherCAT�}�X�^�[�C���X�^���X�̎擾
    master = ecrt_request_master(0);
    if (!master) {
        fprintf(stderr, "Failed to acquire master!\n");
        return -1;
    }

    // �h���C���̍쐬
    domain1 = ecrt_master_create_domain(master);
    if (!domain1) {
        fprintf(stderr, "Failed to create domain!\n");
        return -1;
    }

    // �X���[�u�̍\��
    sc = ecrt_master_slave_config(master, 0, VENDOR_ID, PRODUCT_ID);
    if (!sc) {
        fprintf(stderr, "Failed to configure slave!\n");
        return -1;
    }

    // PDO�G���g���̐ݒ�
    ec_pdo_entry_reg_t domain1_regs[] = {
        {0, VENDOR_ID, PRODUCT_ID, 0x6040, 0, &off_control_word},         // Control word
        {0, VENDOR_ID, PRODUCT_ID, 0x6064, 0, &off_position_actual_value}, // Position actual value
        {0, VENDOR_ID, PRODUCT_ID, 0x60FD, 0, &off_digital_inputs},       // Digital Inputs
        {0, VENDOR_ID, PRODUCT_ID, 0x6041, 0, &off_status_word},          // Status word
        {0, VENDOR_ID, PRODUCT_ID, 0x6071, 0, &off_target_torque},        // Target Torque
        {0, VENDOR_ID, PRODUCT_ID, 0x6060, 0, &off_mode_of_operation},    // Modes of Operation
        {}
    };

    if (ecrt_domain_reg_pdo_entry_list(domain1, domain1_regs)) {
        fprintf(stderr, "PDO entry registration failed!\n");
        return -1;
    }

    // �}�X�^�[�̃A�N�e�B�u��
    if (ecrt_master_activate(master)) {
        fprintf(stderr, "Master activation failed!\n");
        return -1;
    }

    // �h���C���v���Z�X�f�[�^�̎擾
    if (!(domain1_pd = ecrt_domain_data(domain1))) {
        fprintf(stderr, "Failed to get domain data pointer!\n");
        return -1;
    }

    // �f�o�C�X�������V�[�P���X
    EC_WRITE_U16(domain1_pd + off_control_word, 0x0080); // �G���[���Z�b�g
    usleep(10000); // 10ms�ҋ@
    EC_WRITE_U16(domain1_pd + off_control_word, 0x0006); // �X�C�b�`�I������
    usleep(10000); // 10ms�ҋ@
    EC_WRITE_U16(domain1_pd + off_control_word, 0x0007); // �X�C�b�`�I��
    usleep(10000); // 10ms�ҋ@
    EC_WRITE_U16(domain1_pd + off_control_word, 0x000F); // �^�]�C�l�[�u��
    usleep(10000); // 10ms�ҋ@

    // �T�C�N���^�X�N�̎��s
    while (1) {
        cyclic_task();
        usleep(PERIOD_NS / 1000);
    }

    return 0;
}

