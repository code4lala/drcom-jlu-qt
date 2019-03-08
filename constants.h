#ifndef CONSTANTS_H
#define CONSTANTS_H

enum {
    // 离线原因
    OFF_UNKNOWN,
    OFF_WRONG_PASS,
    OFF_WRONG_MAC,
    OFF_USER_LOGOUT,

    // 当前状态
    STATE_OFFLINE,
    STATE_LOGGING,
    STATE_ONLINE

};

#endif // CONSTANTS_H
