#include "dogcom.h"

#include <QDebug>
#include "constants.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "encrypt/md4.h"
#include "encrypt/md5.h"
#include "encrypt/sha1.h"

DogCom::DogCom(InterruptibleSleeper *s)
{
	sleeper = s;
	log = true;
}

void DogCom::Stop()
{
	sleeper->Interrupt();
}

void DogCom::FillConfig(QString a, QString p, QString m)
{
	account = a;
	password = p;
	mac_addr = m;
}

void DogCom::print_packet(const char* msg, const unsigned char *packet, int length)
{
	if (!log)
		return;
	QString x;
	for (int i = 0; i < length; i++)
	{
		x.append(QString::asprintf("%02x ", packet[i]));
	}
	qDebug("%s %s", msg, x.toStdString().c_str());
}

void DogCom::run()
{
	// 每一个ReportOffline后边都跟着一个return语句，防止程序逻辑混乱
	//    qDebug()<<"account:"<<account;
	//    qDebug()<<"password:"<<password;
	//    qDebug()<<"mac_addr:"<<mac_addr;
    qDebug() << Qt::endl;
	qDebug() << "Start dogcoming...";
	// 后台登录维持连接的线程
	srand((unsigned int)time(nullptr));
    DogcomSocket skt;
	try
	{
        skt.init();
	}
    catch (DogcomSocketException& e)
	{
		qCritical() << "dogcom socket init error"
					<< " msg: " << e.what();
		switch (e.errCode)
		{
		case DogcomError::WSA_START_UP:
			emit ReportOffline(OFF_WSA_STARTUP);
			return;
		case DogcomError::SOCKET:
			emit ReportOffline(OFF_CREATE_SOCKET);
			return;
		case DogcomError::BIND:
			emit ReportOffline(OFF_BIND_FAILED);
			return;
		case DogcomError::SET_SOCK_OPT_TIMEOUT:
			emit ReportOffline(OFF_SET_SOCKET_TIMEOUT);
			return;
		case DogcomError::SET_SOCK_OPT_REUSE:
			emit ReportOffline(OFF_SET_SOCKET_TIMEOUT);
			return;
		}
	}

	qInfo() << "Bind Success!";
	unsigned char seed[4];
	unsigned char auth_information[16];
    if (!dhcp_challenge(skt, seed))
	{
		qCritical() << "dhcp challenge failed";
		emit ReportOffline(OFF_CHALLENGE_FAILED);
		return;
	}
	else
	{
		//challenge成功 开始登录
		qDebug() << "trying to login...";
		sleeper->Sleep(200); // 0.2 sec
		qDebug() << "Wait for 0.2 second done.";
		int offLineReason;
        if ((offLineReason = dhcp_login(skt, seed, auth_information)) == -1)
		{
			// 登录成功
			emit ReportOnline();
			int keepalive_counter = 0;
			int first = 1;
			while (true)
			{
                if (!keepalive_1(skt, auth_information))
				{
					sleeper->Sleep(200); // 0.2 second
                    if (keepalive_2(skt, &keepalive_counter, &first))
					{
						continue;
					}
					qDebug() << "Keepalive in loop.";
					if (!sleeper->Sleep(20000))
					{
                        // 先注销再退出，这样DogcomSocket的析构函数一定会执行
                        // 窗口函数部分处理是用户注销还是用户退出
						qDebug() << "Interruptted by user";
						emit ReportOffline(OFF_USER_LOGOUT);
						return;
					}
				}
				else
				{
					qDebug() << "ReportOffline OFF_TIMEOUT caused by keepalive_1";
					emit ReportOffline(OFF_TIMEOUT);
					return;
				}
			}
		}
		else
		{
			// 登录失败，提示用户失败原因
			emit ReportOffline(offLineReason);
			return;
		}
	}
}

bool DogCom::dhcp_challenge(DogcomSocket &socket, unsigned char seed[])
{
	qDebug() << "Begin dhcp challenge...";
	unsigned char challenge_packet[20], recv_packet[1024];
	memset(challenge_packet, 0, 20);
	challenge_packet[0] = 0x01;
	challenge_packet[1] = 0x02;
	challenge_packet[2] = rand() & 0xff;
	challenge_packet[3] = rand() & 0xff;
	challenge_packet[4] = 0x68;

	qDebug() << "writing to dest...";
	if (socket.write((const char *)challenge_packet, 20) <= 0)
	{
		qCritical() << "Failed to send challenge";
		return false;
	}
	print_packet("[Challenge sent]", challenge_packet, 20);

	qDebug() << "reading from dest...";
	if (socket.read((char *)recv_packet) <= 0)
	{
		qCritical() << "Failed to recv";
		return false;
	}
	qDebug() << "read done";
	print_packet("[Challenge recv]", recv_packet, 76);

	if (recv_packet[0] != 0x02)
	{
		return false;
	}
	// recv_packet[20]到[24]是ip地址
	emit ReportIpAddress(recv_packet[20], recv_packet[21], recv_packet[22], recv_packet[23]);

	memcpy(seed, &recv_packet[4], 4);
	qDebug() << "End dhcp challenge";
	return true;
}

int DogCom::dhcp_login(DogcomSocket &socket, unsigned char seed[], unsigned char auth_information[])
{
	unsigned int login_packet_size;
	unsigned int length_padding = 0;
	int JLU_padding = 0;

	if (password.length() > 8)
	{
		length_padding = password.length() - 8 + (length_padding % 2);
		if (password.length() != 16)
		{
			JLU_padding = password.length() / 4;
		}
		length_padding = 28 + password.length() - 8 + JLU_padding;
	}
	login_packet_size = 338 + length_padding;
	unsigned char *login_packet = new unsigned char[login_packet_size];
	// checksum1[8]改为checksum1[16]
	unsigned char recv_packet[1024],
		MD5A[16], MACxorMD5A[6], MD5B[16], checksum1[16], checksum2[4];
	memset(login_packet, 0, login_packet_size);
	memset(recv_packet, 0, 100);

	// build login-packet
	login_packet[0] = 0x03;
	login_packet[1] = 0x01;
	login_packet[2] = 0x00;
	login_packet[3] = account.length() + 20;
	int MD5A_len = 6 + password.length();
	unsigned char *MD5A_str = new unsigned char[MD5A_len];
	MD5A_str[0] = 0x03;
	MD5A_str[1] = 0x01;
	memcpy(MD5A_str + 2, seed, 4);
	memcpy(MD5A_str + 6, password.toStdString().c_str(), password.length());
	MD5(MD5A_str, MD5A_len, MD5A);
	memcpy(login_packet + 4, MD5A, 16);
	memcpy(login_packet + 20, account.toStdString().c_str(), account.length());
	login_packet[56] = 0x20;
	login_packet[57] = 0x03;
	uint64_t sum = 0;
	uint64_t mac = 0;
	// unpack
	for (int i = 0; i < 6; i++)
	{
		sum = (int)MD5A[i] + sum * 256;
	}
	// unpack
	// 将QString转换为unsigned char
	unsigned char mac_binary[6];
	char __c1, __c2;
	__c1 = mac_addr.at(0).toLatin1();
	__c2 = mac_addr.at(1).toLatin1();
	int __i1, __i2;
	auto charToInt = [](char x) {
		if (x >= '0' && x <= '9')
			return x - '0';
		return x - 'A' + 10;
	};
	__i1 = charToInt(__c1);
	__i2 = charToInt(__c2);
	mac_binary[0] = (__i1 << 4) + __i2;
	for (int i = 1; i < 6; i++)
	{
		__c1 = mac_addr.at(i * 3).toLatin1();
		__c2 = mac_addr.at(i * 3 + 1).toLatin1();
		__i1 = charToInt(__c1);
		__i2 = charToInt(__c2);
		mac_binary[i] = (__i1 << 4) + __i2;
	}
	// 将QString转换为unsigned char结束
	for (int i = 0; i < 6; i++)
	{
		mac = mac_binary[i] + mac * 256;
	}
	sum ^= mac;
	// pack
	for (int i = 6; i > 0; i--)
	{
		MACxorMD5A[i - 1] = (unsigned char)(sum % 256);
		sum /= 256;
	}
	memcpy(login_packet + 58, MACxorMD5A, sizeof(MACxorMD5A));
	int MD5B_len = 9 + password.length();
	unsigned char *MD5B_str = new unsigned char[MD5B_len];
	memset(MD5B_str, 0, MD5B_len);
	MD5B_str[0] = 0x01;
	memcpy(MD5B_str + 1, password.toStdString().c_str(), password.length());
	memcpy(MD5B_str + password.length() + 1, seed, 4);
	MD5(MD5B_str, MD5B_len, MD5B);
	memcpy(login_packet + 64, MD5B, 16);
	login_packet[80] = 0x01;
	unsigned char host_ip[4] = {0};
	memcpy(login_packet + 81, host_ip, 4);
	unsigned char checksum1_str[101], checksum1_tmp[4] = {0x14, 0x00, 0x07, 0x0b};
	memcpy(checksum1_str, login_packet, 97);
	memcpy(checksum1_str + 97, checksum1_tmp, 4);
	MD5(checksum1_str, 101, checksum1);
	memcpy(login_packet + 97, checksum1, 8);
	login_packet[105] = 0x01;
	memcpy(login_packet + 110, "LIYUANYUAN", 10);
	unsigned char PRIMARY_DNS[4];
	PRIMARY_DNS[0] = 10;
	PRIMARY_DNS[1] = 10;
	PRIMARY_DNS[2] = 10;
	PRIMARY_DNS[3] = 10;
	memcpy(login_packet + 142, PRIMARY_DNS, 4);
	unsigned char dhcp_server[4] = {0};
	memcpy(login_packet + 146, dhcp_server, 4);
	unsigned char OSVersionInfoSize[4] = {0x94};
	unsigned char OSMajor[4] = {0x05};
	unsigned char OSMinor[4] = {0x01};
	unsigned char OSBuild[4] = {0x28, 0x0a};
	unsigned char PlatformID[4] = {0x02};
	OSVersionInfoSize[0] = 0x94;
	OSMajor[0] = 0x06;
	OSMinor[0] = 0x02;
	OSBuild[0] = 0xf0;
	OSBuild[1] = 0x23;
	PlatformID[0] = 0x02;
	unsigned char ServicePack[40] = {0x33, 0x64, 0x63, 0x37, 0x39, 0x66, 0x35, 0x32, 0x31, 0x32, 0x65, 0x38, 0x31, 0x37,
									 0x30, 0x61, 0x63, 0x66, 0x61, 0x39, 0x65, 0x63, 0x39, 0x35, 0x66, 0x31, 0x64, 0x37,
									 0x34, 0x39, 0x31, 0x36, 0x35, 0x34, 0x32, 0x62, 0x65, 0x37, 0x62, 0x31};
	unsigned char hostname[9] = {0x44, 0x72, 0x43, 0x4f, 0x4d, 0x00, 0xcf, 0x07, 0x68};
	memcpy(login_packet + 182, hostname, 9);
	memcpy(login_packet + 246, ServicePack, 40);
	memcpy(login_packet + 162, OSVersionInfoSize, 4);
	memcpy(login_packet + 166, OSMajor, 4);
	memcpy(login_packet + 170, OSMinor, 4);
	memcpy(login_packet + 174, OSBuild, 4);
	memcpy(login_packet + 178, PlatformID, 4);
	login_packet[310] = 0x68;
	login_packet[311] = 0x00;
	int counter = 312;
	unsigned int ror_padding = 0;
	if (password.length() <= 8)
	{
		ror_padding = 8 - password.length();
	}
	else
	{
		if ((password.length() - 8) % 2)
		{
			ror_padding = 1;
		}
		ror_padding = JLU_padding;
	}
	MD5(MD5A_str, MD5A_len, MD5A);
	login_packet[counter + 1] = password.length();
	counter += 2;
	for (int i = 0, x = 0; i < password.length(); i++)
	{
		x = (int)MD5A[i] ^ (int)password.at(i).toLatin1();
		login_packet[counter + i] = (unsigned char)(((x << 3) & 0xff) + (x >> 5));
	}
	counter += password.length();
	login_packet[counter] = 0x02;
	login_packet[counter + 1] = 0x0c;
	unsigned char *checksum2_str = new unsigned char[counter + 18]; // [counter + 14 + 4]
	memset(checksum2_str, 0, counter + 18);
	unsigned char checksum2_tmp[6] = {0x01, 0x26, 0x07, 0x11};
	memcpy(checksum2_str, login_packet, counter + 2);
	memcpy(checksum2_str + counter + 2, checksum2_tmp, 6);
	memcpy(checksum2_str + counter + 8, mac_binary, 6);
	sum = 1234;
	uint64_t ret = 0;
	for (int i = 0; i < counter + 14; i += 4)
	{
		ret = 0;
		for (int j = 4; j > 0; j--)
		{
			ret = ret * 256 + (int)checksum2_str[i + j - 1];
		}
		sum ^= ret;
	}
	sum = (1968 * sum) & 0xffffffff;
	for (int j = 0; j < 4; j++)
	{
		checksum2[j] = (unsigned char)(sum >> (j * 8) & 0xff);
	}
	memcpy(login_packet + counter + 2, checksum2, 4);
	memcpy(login_packet + counter + 8, mac_binary, 6);
	login_packet[counter + ror_padding + 14] = 0xe9;
	login_packet[counter + ror_padding + 15] = 0x13;
	login_packet[counter + ror_padding + 14] = 0x60;
	login_packet[counter + ror_padding + 15] = 0xa2;

	qDebug() << "login_packet_size:" << login_packet_size;
	socket.write((const char *)login_packet, login_packet_size);
	print_packet("[Login sent]", login_packet, login_packet_size);

	if (socket.read((char *)recv_packet) <= 0)
	{
		qCritical() << "Failed to recv data";
		return OFF_TIMEOUT;
	}

	if (recv_packet[0] != 0x04)
	{
		print_packet("[Login recv]", recv_packet, 100);
		qDebug() << "<<< Login failed >>>";
		if (recv_packet[0] == 0x05)
		{
			switch (recv_packet[4])
			{
			case LOGIN_CHECK_MAC:
				return OFF_CHECK_MAC;
			case LOGIN_SERVER_BUSY:
				return OFF_SERVER_BUSY;
			case LOGIN_WRONG_PASS:
				return OFF_WRONG_PASS;
			case LOGIN_NOT_ENOUGH:
				return OFF_NOT_ENOUGH;
			case LOGIN_FREEZE_UP:
				return OFF_FREEZE_UP;
			case LOGIN_NOT_ON_THIS_IP:
				return OFF_NOT_ON_THIS_IP;
			case LOGIN_NOT_ON_THIS_MAC:
				return OFF_NOT_ON_THIS_MAC;
			case LOGIN_TOO_MUCH_IP:
				return OFF_TOO_MUCH_IP;
				// 升级客户端这个密码错了就会弹出俩
			case LOGIN_UPDATE_CLIENT:
				return OFF_WRONG_PASS;
			case LOGIN_NOT_ON_THIS_IP_MAC:
				return OFF_NOT_ON_THIS_IP_MAC;
			case LOGIN_MUST_USE_DHCP:
				return OFF_MUST_USE_DHCP;
			default:
				return OFF_UNKNOWN;
			}
		}
		return OFF_UNKNOWN;
	}
	else
	{
		print_packet("[Login recv]", recv_packet, 100);
		qDebug() << "<<< Logged in >>>";
	}

	memcpy(auth_information, &recv_packet[23], 16);

	return -1;
}

int DogCom::keepalive_1(DogcomSocket &socket, unsigned char auth_information[])
{
	unsigned char keepalive_1_packet1[8] = {0x07, 0x01, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00};
	unsigned char recv_packet1[1024], keepalive_1_packet2[38], recv_packet2[1024];
	memset(keepalive_1_packet2, 0, 38);
	if (socket.write((const char *)keepalive_1_packet1, 8) <= 0)
	{
		qCritical() << "Failed to send data";
		return 1;
	}
	qDebug() << "[Keepalive1 sent]";
	//    print_packet("[Keepalive1 sent]",keepalive_1_packet1,42);
	while (1)
	{
		if (socket.read((char *)recv_packet1) <= 0)
		{
			qCritical() << "Failed to recv data";
			return 1;
		}
		else
		{
			qDebug() << "[Keepalive1 challenge_recv]";
			//            print_packet("[Keepalive1 challenge_recv]",recv_packet1,100);
			if (recv_packet1[0] == 0x07)
			{
				break;
			}
			else if (recv_packet1[0] == 0x4d)
			{
				qDebug() << "Get notice packet.";
				continue;
			}
			else
			{
				qDebug() << "Bad keepalive1 challenge response received.";
				return 1;
			}
		}
	}

	unsigned char keepalive1_seed[4] = {0};
	int encrypt_type;
	unsigned char crc[8] = {0};
	memcpy(keepalive1_seed, &recv_packet1[8], 4);
	encrypt_type = keepalive1_seed[0] & 3;
	gen_crc(keepalive1_seed, encrypt_type, crc);
	keepalive_1_packet2[0] = 0xff;
	memcpy(keepalive_1_packet2 + 8, keepalive1_seed, 4);
	memcpy(keepalive_1_packet2 + 12, crc, 8);
	memcpy(keepalive_1_packet2 + 20, auth_information, 16);
	keepalive_1_packet2[36] = rand() & 0xff;
	keepalive_1_packet2[37] = rand() & 0xff;

	if (socket.write((const char *)keepalive_1_packet2, 42) <= 0){
		qCritical()<<"Failed to send data";
		return 1;
	}

	if (socket.read((char *)recv_packet2) <= 0)
	{
		qCritical() << "Failed to recv data";
		return 1;
	}
	else
	{
		qDebug() << "[Keepalive1 recv]";
		//        print_packet("[Keepalive1 recv]",recv_packet2,100);

		if (recv_packet2[0] != 0x07)
		{
			qDebug() << "Bad keepalive1 response received.";
			return 1;
		}
	}
	return 0;
}

int DogCom::keepalive_2(DogcomSocket &socket, int *keepalive_counter, int *first)
{
	unsigned char keepalive_2_packet[40], recv_packet[1024], tail[4];

	if (*first)
	{
		// send the file packet
		memset(keepalive_2_packet, 0, 40);
		keepalive_2_packetbuilder(keepalive_2_packet, *keepalive_counter % 0xFF, *first, 1);
		(*keepalive_counter)++;

		if (socket.write((const char *)keepalive_2_packet, 40) <= 0){
			qCritical()<<"Failed to send data";
			return 1;
		}

		qDebug() << "[Keepalive2_file sent]";
		//        print_packet("[Keepalive2_file sent]",keepalive_2_packet,40);
		if (socket.read((char *)recv_packet) <= 0)
		{
			qCritical() << "Failed to recv data";
			return 1;
		}
		qDebug() << "[Keepalive2_file recv]";
		//        print_packet("[Keepalive2_file recv]",recv_packet,40);

		if (recv_packet[0] == 0x07)
		{
			if (recv_packet[2] == 0x10)
			{
				qDebug() << "Filepacket received.";
			}
			else if (recv_packet[2] != 0x28)
			{
				qDebug() << "Bad keepalive2 response received.";
				return 1;
			}
		}
		else
		{
			qDebug() << "Bad keepalive2 response received.";
			return 1;
		}
	}

	// send the first packet
	*first = 0;
	memset(keepalive_2_packet, 0, 40);
	keepalive_2_packetbuilder(keepalive_2_packet, *keepalive_counter % 0xFF, *first, 1);
	(*keepalive_counter)++;
	socket.write((const char *)keepalive_2_packet, 40);

	qDebug() << "[Keepalive2_A sent]";
	//    print_packet("[Keepalive2_A sent]",keepalive_2_packet,40);

	if (socket.read((char *)recv_packet) <= 0)
	{
		qCritical() << "Failed to recv data";
		return 1;
	}

	if (recv_packet[0] == 0x07)
	{
		if (recv_packet[2] != 0x28)
		{
			printf("Bad keepalive2 response received.\n");
			return 1;
		}
	}
	else
	{
		printf("Bad keepalive2 response received.\n");
		return 1;
	}
	memcpy(tail, &recv_packet[16], 4);

	memset(keepalive_2_packet, 0, 40);
	keepalive_2_packetbuilder(keepalive_2_packet, *keepalive_counter % 0xFF, *first, 3);
	memcpy(keepalive_2_packet + 16, tail, 4);
	(*keepalive_counter)++;
	socket.write((const char *)keepalive_2_packet, 40);

	qDebug() << "[Keepalive2_C sent]";
	//    print_packet("[Keepalive2_C sent]",keepalive_2_packet,40);

	if (socket.read((char *)recv_packet) <= 0)
	{
		qCritical() << "Failed to recv data";
		return 1;
	}
	qDebug() << "[Keepalive2_D recv]";
	//    print_packet("[Keepalive2_D recv]",recv_packet,40);

	if (recv_packet[0] == 0x07)
	{
		if (recv_packet[2] != 0x28)
		{
			qDebug() << "Bad keepalive2 response received.";
			return 1;
		}
	}
	else
	{
		qDebug() << "Bad keepalive2 response received.";
		return 1;
	}

	return 0;
}

void DogCom::gen_crc(unsigned char seed[], int encrypt_type, unsigned char crc[])
{
	if (encrypt_type == 0)
	{
		char DRCOM_DIAL_EXT_PROTO_CRC_INIT[4] = {(char)0xc7, (char)0x2f, (char)0x31, (char)0x01};
		char gencrc_tmp[4] = {0x7e};
		memcpy(crc, DRCOM_DIAL_EXT_PROTO_CRC_INIT, 4);
		memcpy(crc + 4, gencrc_tmp, 4);
	}
	else if (encrypt_type == 1)
	{
		unsigned char hash[32] = {0};
		MD5(seed, 4, hash);
		crc[0] = hash[2];
		crc[1] = hash[3];
		crc[2] = hash[8];
		crc[3] = hash[9];
		crc[4] = hash[5];
		crc[5] = hash[6];
		crc[6] = hash[13];
		crc[7] = hash[14];
	}
	else if (encrypt_type == 2)
	{
		unsigned char hash[32] = {0};
		MD4(seed, 4, hash);
		crc[0] = hash[1];
		crc[1] = hash[2];
		crc[2] = hash[8];
		crc[3] = hash[9];
		crc[4] = hash[4];
		crc[5] = hash[5];
		crc[6] = hash[11];
		crc[7] = hash[12];
	}
	else if (encrypt_type == 3)
	{
		unsigned char hash[32] = {0};
		SHA1(seed, 4, hash);
		crc[0] = hash[2];
		crc[1] = hash[3];
		crc[2] = hash[9];
		crc[3] = hash[10];
		crc[4] = hash[5];
		crc[5] = hash[6];
		crc[6] = hash[15];
		crc[7] = hash[16];
	}
}

void DogCom::keepalive_2_packetbuilder(unsigned char keepalive_2_packet[], int keepalive_counter, int filepacket, int type)
{
	keepalive_2_packet[0] = 0x07;
	keepalive_2_packet[1] = keepalive_counter;
	keepalive_2_packet[2] = 0x28;
	keepalive_2_packet[4] = 0x0b;
	keepalive_2_packet[5] = type;
	if (filepacket)
	{
		keepalive_2_packet[6] = 0x0f;
		keepalive_2_packet[7] = 0x27;
	}
	else
	{
		unsigned char KEEP_ALIVE_VERSION[2];
		KEEP_ALIVE_VERSION[0] = 0xDC;
		KEEP_ALIVE_VERSION[1] = 0x02;
		memcpy(keepalive_2_packet + 6, KEEP_ALIVE_VERSION, 2);
	}
	keepalive_2_packet[8] = 0x2f;
	keepalive_2_packet[9] = 0x12;
	if (type == 3)
	{
		unsigned char host_ip[4] = {0};
		memcpy(keepalive_2_packet + 28, host_ip, 4);
	}
}
