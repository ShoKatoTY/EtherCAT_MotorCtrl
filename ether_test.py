import time
from pyethercat import Master

# EtherCAT�}�X�^��������
master = Master(interface="eth0")  # �g�p����C���^�[�t�F�[�X�����w��

# �X���[�u�f�o�C�X�i���[�^�[�h���C�o�j���擾
slave = master.slaves[0]  # ��ڂ̃X���[�u�f�o�C�X���g�p

# �g���N�w�߂ƈʒu�m�F�̂��߂�PDO�C���f�b�N�X��ݒ�
TORQUE_COMMAND_INDEX = 0x6071  # �g���N�w�߂̃C���f�b�N�X
POSITION_ACTUAL_INDEX = 0x6064  # ���݈ʒu�̃C���f�b�N�X

def set_torque(torque_value):
    # �g���N�w�߂�ݒ�
    slave.sdo_write(TORQUE_COMMAND_INDEX, torque_value)

def get_position():
    # ���݈ʒu���擾
    position = slave.sdo_read(POSITION_ACTUAL_INDEX)
    return position

try:
    # EtherCAT�ʐM���J�n
    master.start()

    # �g���N�w�߂𑗐M
    torque_value = 1000  # ��: 1000mNm�̃g���N��ݒ�
    set_torque(torque_value)
    print(f"Torque command set to: {torque_value}")

    # ��莞�ԑҋ@���Ă��猻�݈ʒu���擾
    time.sleep(1)  # 1�b�ҋ@
    position = get_position()
    print(f"Current motor position: {position}")

finally:
    # EtherCAT�ʐM���~
    master.stop()