#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>

#include "ethercat.h"

// IOデータバッファ
static char IOmap[4096];
// 期待されるWKC値
static int expectedWKC;

// 開く
int soem_open(char* nif)
{
    int ret = ec_init(nif);
    return ret;
}

// 閉じる
void soem_close(void)
{
    ec_close();
}

#define ALL_SLAVES_OP_STATE 0
#define NO_SLAVES_FOUND     1
#define NOT_ALL_OP_STATE    2

// コンフィグする
int soem_config(void)
{
    int oloop, iloop, chk;

    if ( ec_config_init(FALSE) > 0 )
    {
        printf("%d slaves found and configured.\n",ec_slavecount);

        ec_config_map(&IOmap);
        ec_configdc();

        printf("Slaves mapped, state to SAFE_OP.\n");

        ec_statecheck(0, EC_STATE_SAFE_OP,  EC_TIMEOUTSTATE * 4);
        oloop = ec_slave[0].Obytes;
        if ((oloop == 0) && (ec_slave[0].Obits > 0)) oloop = 1;
        if (oloop > 8) oloop = 8;
        iloop = ec_slave[0].Ibytes;
        if ((iloop == 0) && (ec_slave[0].Ibits > 0)) iloop = 1;
        if (iloop > 8) iloop = 8;
        printf("segments : %d : %d %d %d %d\n",
            ec_group[0].nsegments,
            ec_group[0].IOsegment[0],
            ec_group[0].IOsegment[1],
            ec_group[0].IOsegment[2],
            ec_group[0].IOsegment[3]);
        printf("Request operational state for all slaves\n");
        expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
        printf("Calculated workcounter %d\n", expectedWKC);

        ec_slave[0].state = EC_STATE_OPERATIONAL;
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_writestate(0);
        chk = 40;

        do
        {
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);
            ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
        }
        while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

        if (ec_slave[0].state == EC_STATE_OPERATIONAL )
        {
            return ALL_SLAVES_OP_STATE;
        }
        else
        {
            return NOT_ALL_OP_STATE;
        }
    }
    else
    {
        return NO_SLAVES_FOUND;
    }
}

// スレーブの数を取得
int soem_getSlaveCount(void)
{
    return ec_slavecount;
}

// スレーブの状態を更新
int soem_updateState(void)
{
    int ret = ec_readstate();
    return ret;
}

// スレーブの状態を取得
int soem_getState(int slave)
{
    return ec_slave[slave].state;
}

// スレーブのALステータスコードを取得
int soem_getALStatusCode(int slave)
{
    return ec_slave[slave].ALstatuscode;
}

// スレーブのALステータスの説明を取得
void soem_getALStatusDesc(int slave, char* desc)
{
    snprintf(desc, 31, "%s", ec_ALstatuscode2string( ec_slave[slave].ALstatuscode ));
}

// スレーブの状態変更を要求
void soem_requestState(int slave, int state)
{
    ec_slave[slave].state = state;
    ec_writestate(slave);
}

// スレーブの名前を取得
void soem_getName(int slave, char* name)
{
    snprintf(name, 41, "%s", ec_slave[slave].name );
}

// スレーブのベンダ番号/製品番号/バージョン番号を取得
void soem_getId(int slave, unsigned long* id)
{
    id[0] = ec_slave[slave].eep_man;
    id[1] = ec_slave[slave].eep_id;
    id[2] = ec_slave[slave].eep_rev;
}

// PDO転送する
int soem_transferPDO(void)
{
    ec_send_processdata();
    int wkc = ec_receive_processdata(EC_TIMEOUTRET);

    if(wkc >= expectedWKC){
        return 1;
    }else{
        return 0;
    }
}

// 汎用スレーブPDO入力
uint8_t soem_getInputPDO(int slave, int offset)
{
    uint8_t ret = 0;

    if(slave <= ec_slavecount)
    {
        ret = ec_slave[slave].inputs[offset];
    }
    return ret;
}

// 汎用スレーブPDO出力
void soem_setOutPDO(int slave, int offset, uint8_t value)
{
    if(slave <= ec_slavecount)
    {
        ec_slave[slave].outputs[offset] = value;
    }
}

// 汎用スレーブPDO出力 (16ビット)
void soem_setOutPDO16(int slave, int offset, uint16_t value)
{
    if(slave <= ec_slavecount)
    {
        ec_slave[slave].outputs[offset] = value & 0xFF;
        ec_slave[slave].outputs[offset+1] = (value >> 8) & 0xFF;
    }
}

// メイン関数
int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        char *ifname = argv[1];

        int result = soem_open(ifname);
        if(result == 0)
        {
            printf("can not open network adapter!\n");
            return (1);
        }
        result = soem_config();
        if(result == NO_SLAVES_FOUND)
        {
            printf("slaves not found!\n");
            return (2);
        }
        if(result == NOT_ALL_OP_STATE)
        {
            printf("at least one slave can not reach OP state!\n");
            return (3);
        }

        uint16_t target_torque = 1000; // 目標トルクの値 (例: 1000)
        while (1)
        {
            // 1個目のスレーブのOUTの0〜1バイトめに目標トルクの値を設定
            soem_setOutPDO16(1, 0, target_torque);
            soem_transferPDO();

            // 必要に応じて、スレーブからの入力データを読み取ることができます
            // 例: uint8_t status = soem_getInputPDO(1, 0);
        }

        soem_requestState(0, EC_STATE_INIT);
        soem_close();
    }
    else
    {
        printf("Available adapters\n");
        ec_adaptert *adapter = NULL;
        adapter = ec_find_adapters();
        while (adapter != NULL)
        {
            printf("Description : %s, Device to use for wpcap: %s\n",
                adapter->desc,
                adapter->name);
            adapter = adapter->next;
        }
    }
    return (0);
}
