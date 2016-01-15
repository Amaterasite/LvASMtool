
#include "LvASMtool.h"

LvASMtool::LvASMtool() {}

LvASMtool::LvASMtool(string romName,string listName,int infoLevel) {

	LvASMtool::romName = romName;
	LvASMtool::listName = listName;
	LvASMtool::infoLevel = infoLevel;

	if(!romFile.open(romName)) {
		throw string("rom�t�@�C�����J���܂���ł����B");
	}
	romSize = romFile.getRomSize();
	headerSize = romFile.getHeader() ? 0x0200 : 0x0000;

	if(romSize < BASE_ADDR) {
		throw string("rom�e�ʂ��g�����Ă��������B\n");
	}
	if(romFile.findData((void*)"Lunar Magic",11,0x07F0A0) != 0x07F0A0) {
		throw string("Lunar Magic�ŉ������Ă��������B\n");
	}

	// LevelASM�}���\�̈��Bank0x40����
	insEndAddr = romSize > 0x200000 ? 0x200000 : romSize;

	insAddr = romFile.findData((void*)"LevelASM tool",13,BASE_ADDR) - 0x10;
	if(romFile.checkRATSdata(insAddr) < 0)	insAddr = -1;

	// �C���X�g�[���ς݂̏ꍇ�o�[�W�������擾
	if(insAddr>=0) {
		lvASMver = romFile.getRomData(insAddr+0x09)<<8 | romFile.getRomData(insAddr+0x08);
	}
	// edit1754����LevelASM���}���ς݂��m�F �}���ς݂� �}���A�h���X = -1754
	else {
		insAddr = romFile.findData((void*)"@LVLASM0",8,0) - 0x08;
		insAddr = (romFile.checkRATSdata(insAddr) >= 0) ? -1754 : -1;
	}
}

LvASMtool::~LvASMtool() {}

LvASMtool::ELvASMver LvASMtool::checkLvASMver() {
	if(insAddr == -1754)	return _edit1754;
	else if(insAddr<0)		return _nothing;

	if(lvASMver==LEVELASM_CODE_VERSION)		return _now;
	else if(lvASMver<LEVELASM_CODE_VERSION)	return _old;
	else									return _new;
}

// LevelASM MainCode�}��
int LvASMtool::insertLevelASMcode() {
	if(insAddr<0) {
		insAddr = romFile.findFreeSpace(BASE_ADDR,insEndAddr,0x1000);
		if(insAddr<0) throw string("LevelASM��}������󂫗̈悪������܂���ł����B");
	}
	return insertLevelASMcode(insAddr);
}

int LvASMtool::insertLevelASMcode(int insAddr) {

	// PC:0x001912(SNES:0x009712)�ɑ}������R�[�h
	uchar levelASM_InitLevelHijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// PC:0x0027B7(SNES:0x00A5B7)�ɑ}������R�[�h
	uchar levelASM_InitHijack0[] = {
			0x80,0x13
	};

	// PC:0x0027CC(SNES:0x00A5CC)�ɑ}������R�[�h
	uchar levelASM_InitHijack1[] = {
			0x20,0xEC,0xA5
	};

	// PC:0x0027E8(SNES:0x00A5E8)�ɑ}������R�[�h
	uchar levelASM_InitHijack2[] = {
			0x5C,0xFF,0xFF,0xFF,0x5C,0xFF,0xFF,0xFF}
	;

	// PC:0x002442(SNES:0x80A242)�ɑ}������R�[�h
	uchar levelASM_EarlyHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x0024EA(SNES:00A2EA)�ɑ}������R�[�h
	uchar levelASM_LaterHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA,0xEA
	};

	// PC]0x0003C1(SNES:0081C1)�ɑ}������R�[�h
	uchar levelASM_NMIHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x02DABB(SNES:0x05D8BB)�ɑ}������R�[�h
	uchar levelNum_Hijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// LevelASM ���s�R�[�h
	uchar levelASMmainCode[] = {
		//  __+0___+1___+2___+3___+4___+5___+6___+7___+8___+9___+A___+B___+C___+D___+E___+F
			0x22,0xDB,0xF6,0x80,0x5C,0xC0,0x81,0x3F,0x22,0xC8,0x81,0x3F,0x2C,0x9B,0x0D,0x30,	// 0x0000
			0x04,0x5C,0x60,0x98,0x80,0x5C,0x0C,0xA6,0x80,0xAE,0x01,0x07,0x8E,0x03,0x09,0xE2,	// 0x0010
			0x30,0x22,0xD0,0x81,0x3F,0x5C,0xF0,0xA5,0x80,0x22,0xD8,0x81,0x3F,0xAD,0xD4,0x13,	// 0x0020
			0xF0,0x04,0x5C,0x5B,0xA2,0x80,0x5C,0x8A,0xA2,0x80,0x68,0x85,0x1D,0x68,0x85,0x1C,	// 0x0030
			0x22,0xE0,0x81,0x3F,0x5C,0xF0,0xA2,0x80,0xA5,0x44,0x8D,0x30,0x21,0xAD,0x00,0x01,	// 0x0040
			0xC9,0x03,0x90,0x18,0xC9,0x0C,0x90,0x08,0xC9,0x10,0x90,0x10,0xC9,0x15,0xB0,0x0C,	// 0x0050
			0xD4,0x00,0x22,0xE8,0x81,0x3F,0x68,0x85,0x00,0x68,0x85,0x01,0x5C,0xC6,0x81,0x80,	// 0x0060
			0x8D,0x0B,0x01,0x0A,0x18,0x65,0x0E,0x6B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x0070
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x0080
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x0090
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x00A0
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x00B0
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x00C0
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x00D0
			0xC2,0x10,0x86,0x00,0xAE,0x0B,0x01,0xBF,0x00,0x82,0x3F,0xF0,0x1C,0xFA,0x4B,0xDA,	// 0x00E0
			0x48,0x48,0xAB,0xC2,0x20,0xAD,0x0B,0x01,0x0A,0xAA,0xBF,0x00,0x84,0x3F,0x18,0x65,	// 0x00F0
			0x00,0xAA,0xBD,0x00,0x00,0x48,0xE2,0x30,0x6B,0xE2,0x30,0x60,0x00,0x00,0x00,0x00,	// 0x0100
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x0110
			0xDA,0x22,0xAA,0x81,0x3F,0xFA,0x20,0x60,0x81,0x60,0xF4,0xBF,0xBF,0xAB,0xC2,0x31,	// 0x0120
			0x8A,0x69,0x00,0xC0,0xAA,0xBD,0x00,0x00,0x48,0xE2,0x30,0x6B,0x00,0x00,0x00,0x00,	// 0x0130
			0x8B,0xA2,0x00,0x20,0x60,0x81,0xAB,0x6B,0x8B,0xA2,0x02,0x20,0x60,0x81,0xAB,0x6B,	// 0x0140
			0x8B,0xA2,0x04,0x20,0x60,0x81,0xAB,0x6B,0x8B,0xA2,0x06,0x20,0x60,0x81,0xAB,0x6B,	// 0x0150
			0x8B,0xA2,0x08,0x20,0x60,0x81,0xAB,0x6B,0x8B,0xA2,0x0A,0x20,0x60,0x81,0xAB,0x6B,	// 0x0160
			0x8B,0xA2,0x0C,0x20,0x60,0x81,0xAB,0x6B,0x8B,0xA2,0x0E,0x20,0x60,0x81,0xAB,0x6B,	// 0x0170
	};

	int insSNESaddr = convPCtoSNES(insAddr) | 0x800000;

	writeLongAddr(levelASMmainCode,0x0005,insSNESaddr+0x01C0);	// JML InitLevel
	writeLongAddr(levelASMmainCode,0x0009,insSNESaddr+0x01C8);	// JSL InitEary
	writeLongAddr(levelASMmainCode,0x0022,insSNESaddr+0x01D0);	// JSL InitLater
	writeLongAddr(levelASMmainCode,0x002A,insSNESaddr+0x01D8);	// JSL MainEarly
	writeLongAddr(levelASMmainCode,0x0041,insSNESaddr+0x01E0);	// JSL MainLater
	writeLongAddr(levelASMmainCode,0x0063,insSNESaddr+0x01E8);	// JSL NMILevel

	writeLongAddr(levelASMmainCode,0x00E8,insSNESaddr+0x0200);	// LDA.l OffsetBank,x
	writeLongAddr(levelASMmainCode,0x00FB,insSNESaddr+0x0400);	// LDA.l OffsetAddr,x
	writeLongAddr(levelASMmainCode,0x0122,insSNESaddr+0x01AA);	// JSL AlwaysASM
	writeWordAddr(levelASMmainCode,0x0127,insSNESaddr+0x0160);	// JSR ExecuteLvASM

	for(int i=0;i<8;i++) {
		writeWordAddr(levelASMmainCode,0x0144+i*8,insSNESaddr+0x0160);	// JSR ExecuteLvASM
	}

	writeLongAddr(levelASM_InitLevelHijack,0x01,insSNESaddr+0x80);	// JSL InitLevel

	writeLongAddr(levelASM_InitHijack2,0x05,insSNESaddr+0x0088);		// JML CallInitEarly
	writeLongAddr(levelASM_InitHijack2,0x01,insSNESaddr+0x0099);		// JML CallInitLater

	writeLongAddr(levelASM_EarlyHijack,0x01,insSNESaddr+0x00A9);	// JML CallMainEarly

	writeLongAddr(levelASM_LaterHijack,0x01,insSNESaddr+0x00BA);	// JML CallMainLater

	writeLongAddr(levelASM_NMIHijack,0x01,insSNESaddr+0x00C8);		// JML CallNMILevel

	writeLongAddr(levelNum_Hijack,0x01,insSNESaddr+0x00F0);			// JSL LevelNum

	// �{�ƃR�[�h��������
	romFile.writeData(levelASM_InitLevelHijack,sizeof(levelASM_InitLevelHijack),0x001712);
	romFile.writeData(levelASM_InitHijack0,sizeof(levelASM_InitHijack0),0x0025B7);
	romFile.writeData(levelASM_InitHijack1,sizeof(levelASM_InitHijack1),0x0025CC);
	romFile.writeData(levelASM_InitHijack2,sizeof(levelASM_InitHijack2),0x0025E8);
	romFile.writeData(levelASM_EarlyHijack,sizeof(levelASM_EarlyHijack),0x002242);
	romFile.writeData(levelASM_LaterHijack,sizeof(levelASM_LaterHijack),0x0022EA);
	romFile.writeData(levelASM_NMIHijack,sizeof(levelASM_NMIHijack),0x0001C1);
	romFile.writeData(levelNum_Hijack,sizeof(levelNum_Hijack),0x02D8BB);

	// �w�b�_��������
	romFile.writeData((void*)LEVELASM_HEADER,0x70,insAddr+0x10);

	// ���s�R�[�h��������
	romFile.writeData((void*)levelASMmainCode,sizeof(levelASMmainCode),insAddr+0x80);

	// RTAS�^�O��������
	romFile.writeRATSdata(nullptr,0x0FF8,insAddr);

	uchar codeVer[] = {(uchar)(LEVELASM_CODE_VERSION),(uchar)(LEVELASM_CODE_VERSION>>8)};

	// �o�[�W������񏑂�����
	romFile.writeData((void*)codeVer,2,insAddr+0x08);

	return insAddr+0x0200;
}

// LevelASM��}��
int LvASMtool::insertLevelASM() {
	if(insAddr<0) return -1;

	uchar LevelPtrAddr[0x0402];
	uchar LevelPtrBank[0x0201];

	for(int i=0;i<0x402;i+=2) {
		writeWordAddr(LevelPtrAddr,i,0x0000);
	}
	for(int i=0;i<0x0201;i++) {
		LevelPtrBank[i] = 0;
	}

	// Level���̑}���t�@�C����
	ushort insFileNum[0x201];
	// Level ����}���t���O
	bool firstIns[0x201];

	// Level���̓o�^Offset��
	ushort insOffsetNum[0x201][8];
	//�@Level���̌��݂̑}���t�@�C���� 2nd pass�Ŏg����
	ushort nowInsOffsetNum[0x201][8];
	for(int i=0;i<0x201;i++) {
		insFileNum[i] = 0;
		firstIns[i] = true;
		for(int j=0;j<8;j++) {
			insOffsetNum[i][j] = 0;
			nowInsOffsetNum[i][j] = 0;
		}
	}

	string lineBuf;
	ifstream listFile;

	listline lineData;
	addr addrData;
	map<string,addr> addrList;

	printf("list���m�F\n");
	// 1st pass Level����offset�����m�F
	listFile.open(listName.c_str());
	if(!listFile) throw string("���X�g�t�@�C�����J���܂���ł����B\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);

		// 4�����ȉ��͖���
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		// ���Level�ɑ}���\��asm�̐���256�Ƃ���B
		if(++insFileNum[lineData.insnum] > 256) {
			throw string(
					"���Level�ɑ}������asm�̐���256�𒴂��܂����B\n"
					"�@ �@ �@ �@ ����ȁ@�[��Ɂ@�܂��J��\n"
					"�@ �@ �@ �@ �Ȃ�������ā@�ƁJ�������\n"
					"�@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ ��");
		}

		// �w�肳�ꂽASM�t�@�C�������ɑ}���ς݂��m�F
		map<string,addr>::const_iterator addrIndex = addrList.find(lineData.filename);
		if(addrIndex != addrList.end()) {
			// �}���ς݂Ȃ�f�[�^���g����
			addrData = addrIndex->second;
		} else {
			// ����}��
			Xkas Xkaser;
			Xkaser.setInsertRange(BASE_ADDR,0);
			Xkaser.allocSpace(&romFile,".\\LevelASM\\"+lineData.filename,18,0xFFF0);
			Xkaser.getLabelList("xkas.log");

			for(int i=0;i<8;i++) {
				addrData.labelData[i] = Xkaser.getOffset(LABEL_NAME[i]);

				// ���x���̕ʖ��𒲂ׂ� INIT(INIT_LATE) / MAIN(MAIN_EARLY)�p
				if(addrData.labelData[i] < 0) {
					addrData.labelData[i] = Xkaser.getOffset(LABEL_ALIAS[i]);
				}
			}
			addrList[lineData.filename] = addrData;
		}

		for(int i=0;i<8;i++) {
			if(addrData.labelData[i] >= 0) {
				insOffsetNum[lineData.insnum][i]++;
			}
		}
	}
	listFile.close();

	addrList.clear();

	// 2nd pass LevelASM�}��
	listFile.open(listName.c_str());
	if(!listFile) throw string("���X�g�t�@�C�����J���܂���ł����B\n");

	printf("LevelASM��}��\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		if(infoLevel > 0) {
			if(lineData.insnum>=0 && lineData.insnum<0x200) {
				printf("\nLevel %03X inserting:.\\LevelASM\\%s\n",lineData.insnum,lineData.filename.c_str());
			} else if(lineData.insnum==0x200) {
				printf("\nLevel ALL inserting:.\\LevelASM\\%s\n",lineData.filename.c_str());
			} else {
				throw string("�s����Level�ԍ���ASM��}�����悤�Ƃ��܂����B\n");
			}

		}
		// �w�肳�ꂽASM�t�@�C�������ɑ}���ς݂��m�F
		map<string,addr>::const_iterator addrIndex = addrList.find(lineData.filename);
		if(addrIndex != addrList.end()) {
			// �}���ς݂Ȃ�f�[�^���g����
			addrData = addrIndex->second;
		}
		else {
			// ����}��
			Xkas Xkaser;
			Xkaser.setInsertRange(BASE_ADDR,0);
			int xAddr = Xkaser.insertRatsASM(&romFile,".\\LevelASM\\"+lineData.filename,0x18,0,0xFFF0);
			int xSize = Xkaser.getASMsize()+0x20;
			if(Xkaser.isError()) {
				throw Xkaser.getSimpleErrMes();
			}

			// LevelASM�̎��ʎq����������
			romFile.writeData((void*)"LvASM_88",8,xAddr+0x08);

			// �e���x���ʒu���擾 ������Ȃ�����RTL�̃A�h���X�����蓖��
			if((addrData.RTLaddr =  Xkaser.getRTLaddr()) < 0) {
				throw string("�R�[�h�ɂ�RTL���K�v�ł��B\n");
			}
			addrData.RTLaddr |= 0x800000;

			uchar offsetData[0x10];
			addrData.base = convPCtoSNES(xAddr+0x10)|0x800000;
			bool worthless = true;
			for(int i=0;i<8;i++) {
				addrData.labelData[i] = Xkaser.getOffset(LABEL_NAME[i]);

				// ���x���̕ʖ��𒲂ׂ� INIT(INIT_LATE) / MAIN(MAIN_EARLY)�p
				if(addrData.labelData[i] < 0) {
					addrData.labelData[i] = Xkaser.getOffset(LABEL_ALIAS[i]);
				}

				// offset����`�̏ꍇRTL�̃A�h���X�֔�΂�
				if(addrData.labelData[i] < 0) {
					writeWordAddr(offsetData,i*2,addrData.RTLaddr-1);
				} else {
					writeWordAddr(offsetData,i*2,addrData.labelData[i]-1);
					worthless = false;
				}
			}
			// �S�Ă�offset������`�̏ꍇ�x����\������
			if(worthless) {
				printf("Warning!: �S�Ă�offset������`�ł��B\n          ����͈�،Ăяo����Ȃ����Ӗ��ȃR�[�h�ɂȂ�܂��I\n");
			}

			// LevelASM��offset���X�g����������
			romFile.writeData((void*)offsetData,0x10,xAddr+0x10);

			addrList[lineData.filename] = addrData;

			// infoLevel��1�ȏ�̏ꍇ LevelASM�̑}������\��
			if(infoLevel>0) {
				printf("Addr:0x%06X (SNES:0x%06X) Size:%dBytes (0x%0X)\n",xAddr,convPCtoSNES(xAddr)|0x800000,xSize,xSize);

				for(int i=0;i<8;i++) {
					if(addrData.labelData[i]<0) {
						if(infoLevel<2)	continue;
						printf("    %s : 0x%06X (SNES:0x%06X) (none) \n",OUTPUT_LABEL_NAME[i],convSNEStoPC(addrData.RTLaddr+1)+headerSize,addrData.RTLaddr+1);
					} else {
						printf("    %s : 0x%06X (SNES:0x%06X) \n",OUTPUT_LABEL_NAME[i],convSNEStoPC(addrData.labelData[i]+1)+headerSize,addrData.labelData[i]+1);
					}
				}
				printf("\n");
			}
		}

		// �Ώۂ�Level�ɑ}������ASM�̐�����̏ꍇ
		if(insFileNum[lineData.insnum] == 1) {
			LevelPtrBank[lineData.insnum] = addrData.base>>16;
			writeWordAddr(LevelPtrAddr,lineData.insnum*2,addrData.base);

		}
		// �Ώۂ�Level�ɑ}������ASM�̐�����������ꍇ�AJSL JML�����ō\�����ꂽ�R�[�h��������������B
		else if(insFileNum[lineData.insnum] > 1) {
			int asmInsAddr;
			uchar instBuf[4];
			// ����}���̏ꍇ �K�v�ȗ̈���m��
			if(firstIns[lineData.insnum]) {
				firstIns[lineData.insnum] = false;

				// �R�[�h�ɕK�v�ȃT�C�Y�v�Z
				int insSize = 0;
				for(int i=0;i<8;i++) {
					insSize += insOffsetNum[lineData.insnum][i]*4;
				}
				if(insSize == 0) {
					printf("Warning!: ���Ӗ��Ȏ��������R�[�h���쐬����܂��I\n");
				}
				insSize += 0x21;		// RATS�^�O / LEVELASM���ʎq / Offset���X�g / ����`�pRTL���̗e��

				asmInsAddr = romFile.findFreeSpace(BASE_ADDR,insEndAddr,insSize);
				if(asmInsAddr < 0)	throw string("LevelASM��}���ł��邾����rom�̈悪������܂���ł����B\n");
				if(infoLevel > 0) {
					printf("�̈�m�� Addr:%06X Size:%dBytes\n",convPCtoSNES(asmInsAddr)|0x800000,insSize);
				}

				romFile.writeRATSdata(nullptr,insSize-0x08,asmInsAddr);	// RATS�^�O��������
				romFile.writeData((void*)"LvASM_88",8,asmInsAddr+0x08);	// ���ʎq��������

				// 0x00�̂܂܂���FreeSpace�F����󂯂Ă��܂��̂Ŋm�ۂ����̈��RTL�Ŗ��߂�
				uchar RTL = 0x6B;
				romFile.writeReptData((void*)&RTL,1,insSize-0x20,asmInsAddr+0x20);

				// Offset���̊J�n�A�h���X����������
				int writeOffset = 0x20;
				uchar offsetAddr[0x10];
				for(int i=0;i<8;i++) {
					int addr = insOffsetNum[lineData.insnum][i] == 0 ? asmInsAddr+insSize-1 : asmInsAddr+writeOffset;

					if(infoLevel>2) {
						printf("    %s : %3d Addr:0x%06X (SNES 0x%06X)\n",OUTPUT_LABEL_NAME[i],(int)insOffsetNum[lineData.insnum][i],addr,convPCtoSNES(addr));
					}
					writeWordAddr(offsetAddr,i*2,convPCtoSNES(addr-1));
					writeOffset += insOffsetNum[lineData.insnum][i]*4;
				}
				romFile.writeData((void*)offsetAddr,0x10,asmInsAddr+0x10);

				LevelPtrBank[lineData.insnum] = (convPCtoSNES(asmInsAddr+0x10)>>16)|0x80;
				writeWordAddr(LevelPtrAddr,lineData.insnum*2,convPCtoSNES(asmInsAddr+0x10));
			} else {
				asmInsAddr = (LevelPtrAddr[lineData.insnum*2] | LevelPtrAddr[lineData.insnum*2+1]<<8 | LevelPtrBank[lineData.insnum]<<16) - 0x10;
				asmInsAddr = convSNEStoPC(asmInsAddr&0x7FFFFF);
			}

			int writeOffset = 0x20;
			// Bank�͑S���x������
			instBuf[3] = (convPCtoSNES(asmInsAddr)>>16)|0x80;

			for(int i=0;i<8;i++) {
				if(addrData.labelData[i] >= 0) {
					// �Ō�Ȃ�JML XXYYZZ ����ȊO�Ȃ�JSL XXYYZZ
					instBuf[0] = (nowInsOffsetNum[lineData.insnum][i]+1) == insOffsetNum[lineData.insnum][i] ? 0x5C : 0x22;

					// �R�[�� / �W�����v�揑������
					writeWordAddr(instBuf,1,addrData.labelData[i]);
					romFile.writeData(instBuf,4,asmInsAddr+writeOffset+nowInsOffsetNum[lineData.insnum][i]*4);

					nowInsOffsetNum[lineData.insnum][i]++;
				}
				writeOffset += insOffsetNum[lineData.insnum][i]*4;
			}
		}
		// �����̊ԈႢ�ő}������\��̖���Level���Ώۂ̏ꍇ
		else {
			throw string("�}���\\��̖���Level�ԍ��ɑ}�����������悤�Ƃ��܂����B\n");	//�_��������D����������
		}
	}

	romFile.writeData(LevelPtrBank,0x0200,insAddr+0x0200);
	romFile.writeData(LevelPtrAddr,0x0400,insAddr+0x0400);

	// �ǂ��ł�ASM��}�����Ă��邩�ۂ���LevelASM�̃R�[�h�̗����ύX����
	for(int i=0;i<8;i++) {
		ushort CallAddr = convPCtoSNES(insOffsetNum[0x200][i] == 0 ? insAddr+0x0160 : insAddr+0x01A0);
		uchar CallAddrLow = (uchar)CallAddr;
		uchar CallAddrHigh = (uchar)(CallAddr>>8);
		romFile.writeData(&CallAddrLow,1,insAddr+0x01C4+i*8);
		romFile.writeData(&CallAddrHigh,1,insAddr+0x01C5+i*8);
	}
	uchar PEAoperand[] = {
			LevelPtrBank[0x0200],
			LevelPtrBank[0x0200],
	};
	uchar ADCoperand[] = {
			LevelPtrAddr[0x0400],
			LevelPtrAddr[0x0401],
	};

	romFile.writeData(PEAoperand,0x02,insAddr+0x1AB);	// PEA $XXXX
	romFile.writeData(ADCoperand,0x02,insAddr+0x1B2);	// ADC #$XXXX
	return 0;
}

// �}���ς݂�LevelASM���폜
int LvASMtool::deleteLevelASM() {
	// �Ȃ�ƂȂ��p��
	int delCount = 0;
	int addr = BASE_ADDR;

	while(true) {
		if(lvASMver < 0x0110) {
			addr = romFile.findData((void*)"LEVELASM",8,addr);
		} else {
			addr = romFile.findData((void*)"LvASM_88",8,addr);
		}

		if(addr<0)	break;
		int size = romFile.eraseRATSdata(addr-0x08);
		if(size>0) {
			if(delCount++==0) {
				if(infoLevel > 0) printf("�}���ς݂�LevelASM���폜\n");
			}
			if(infoLevel > 0) printf("Addr:0x%06X Size:%dBytes\n",convPCtoSNES(addr)|0x800000,size);
		}
		addr++;
	}
	if(infoLevel==0 && delCount>0) printf("%d��LevelASM���폜\n",delCount);
	return delCount;
}

// LevelASM MainCode���폜
int LvASMtool::deleteLevelASMcode() {
	if(insAddr<0) {
		return -1;
	}
	romFile.eraseRATSdata(insAddr);
	return insAddr;
}

// ���������{�Ƃ̃R�[�h��߂�
void LvASMtool::rewindHijackCode() {
	// PC:0x0027CC(SNES:0x00A5CC)�ɑ}������R�[�h
	uchar rewindInitHijack[] = {
			0x20,0x60,0x98,0x20,0x2F,0x92,0x20,0x29,0x9F,0x20,0x1A,0x8E,0xC2,0x30,0x8B,0xA2,
			0x03,0x07,0xA0,0x05,0x09,0xA9,0xEF,0x01,0x54,0x00,0x00,0xAB,0xAE,0x01,0x07,0x8E,
			0x03,0x09,0xE2,0x30
	};

		// PC:0x002442(SNES:0x80A242)�ɑ}������R�[�h
	uchar rewindMainHijack[] = {
			0xAD,0xD4,0x13,0xF0,0x43
	};

	// PC:0x02DABB(SNES:0x05D8BB)�ɑ}������R�[�h
	uchar rewindLevelNumHijack[] = {
			0x0A,0x18,0x65,0x0E
	};

	// PC:0x0024EA(SNES:0x00A2EA)�ɑ}������R�[�h
	// CODE VER 0x0101�Œǉ�
	uchar rewindLaterHijack[] = {
			0x68,0x85,0x1D,0x68,0x85,0x1C
	};

	// PC:0x001912(SNES:0x009712)�ɑ}������R�[�h
	// CODE VER 0x0110�Œǉ�
	uchar rewindLevelHijack[] = {
			0x22,0xDB,0xF6,0x80
	};

	// PC]0x0003C1(SNES:0081C1)�ɑ}������R�[�h
	// CODE VER 0x0110�Œǉ�
	uchar rewindNMIHijack[] = {
			0xA5,0x44,0x8D,0x30,0x21
	};

	// ver 0x0101�ȑO INIT MAIN����
	if(lvASMver<0x0101) {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0110�ȑO INIT EARLY LATER����
	else if(lvASMver<0x0110) {
		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);

		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0112�ȑO
	else if(lvASMver<0x0112){
		romFile.writeData(rewindLevelHijack,sizeof(rewindLevelHijack),0x001712);
		romFile.writeData(rewindNMIHijack,sizeof(rewindNMIHijack),0x0001C1);

		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);

		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0112�ȍ~
	else {

		// PC:0x0027B7(SNES:0x00A5B7)�ɑ}������R�[�h
		uchar rewind_0112_InitHijack0[] = {
				0x80,0x16
		};

		// PC:0x0027CC(SNES:0x00A5CC)�ɑ}������R�[�h
		uchar rewind_0112_InitHijack1[] = {
				0x20,0x60,0x92
		};

		// PC:0x0027E8(SNES:0x00A5E8)�ɑ}������R�[�h
		uchar rewind_0112_InitHijack2[] = {
				0xAE,0x01,0x07,0x8E,0x03,0x09,0xE2,0x30
		};

		romFile.writeData(rewindLevelHijack,sizeof(rewindLevelHijack),0x001712);
		romFile.writeData(rewindNMIHijack,sizeof(rewindNMIHijack),0x0001C1);

		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);

		romFile.writeData(rewind_0112_InitHijack0,sizeof(rewind_0112_InitHijack0),0x0025B7);
		romFile.writeData(rewind_0112_InitHijack1,sizeof(rewind_0112_InitHijack1),0x0025CC);
		romFile.writeData(rewind_0112_InitHijack2,sizeof(rewind_0112_InitHijack2),0x0025E8);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);

	}

}

int LvASMtool::deleteEdit1754LevelASM() {

	// PC:0x0027EE(SNES:0x80A5EE)�ɑ}������R�[�h
	uchar rewindInitHijack[] = {
			0xE2,0x30,0x20,0x9B,0x91
	};

	// PC:0x002442(SNES:0x80A242)�ɑ}������R�[�h
	uchar rewindMainHijack[] = {
			0xAD,0xD4,0x13,0xF0,0x43
	};

	// PC:0x02DAB9(SNES:0x85D8B9)�ɑ}������R�[�h
	uchar rewindLevelNumHijack[] = {
			0xA5,0x0E,0x0A
	};

	uchar rewind_FFh[] = {
			0xFF
	};
	// �Ȃ�ƂȂ��p��
	int delCount = 0;
	int addr = 0;

	while(true) {
		addr = romFile.findData((void*)"@LVLASM0",8,addr);
		if(addr<0)	break;
		int size = romFile.eraseRATSdata(addr-0x08);
		if(size>0) {
			if(delCount++==0) {
				if(infoLevel > 0) printf("�}���ς݂�LevelASM���폜\n");
			}
			if(infoLevel > 0) printf("Addr:0x%06X Size:%dBytes\n",convPCtoSNES(addr)|0x800000,size);
		}
		addr++;
	}
	if(infoLevel==0 && delCount>0) printf("%d��LevelASM���폜\n",delCount-1);

	romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025EE);
	romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
	romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8B9);
	romFile.writeReptData(rewind_FFh,1,7,0x02DC46);
	return delCount-1;
}

void LvASMtool::writeRomFile() {
	romFile.writeRomFile();
}

// rom�ɑ}������Ă���LevelASM�̃o�[�W�������擾
ushort LvASMtool::getRomLvASMver() {
	return lvASMver;
}

// tool��LevelASM�̃o�[�W�������擾
ushort LvASMtool::getToolLvASMver() {
	return LEVELASM_CODE_VERSION;
}

int LvASMtool::convPCtoSNES(int addr) {
	if(addr<0) return -1;
	return ((addr&0xFFFF8000)<<1)|(addr&0x7FFF)|0x8000;
}

int LvASMtool::convSNEStoPC(int addr) {
	if(!(addr&0x8000)) return -1;
	addr &= 0x7FFFFF;
	return ((addr&0xFFFF0000)>>1)|(addr&0x7FFF);
}

// uchar�^�̔z���24bit�̃f�[�^���������ނ���
void LvASMtool::writeLongAddr(uchar* data,int offset,int addr) {
	data[offset] = (uchar)addr;
	data[offset+1] = (uchar)(addr>>8);
	data[offset+2] = (uchar)(addr>>16);
}

// uchar�^�̔z���16bit�̃f�[�^���������ނ���
void LvASMtool::writeWordAddr(uchar* data,int offset,int addr) {
	data[offset] = (uchar)addr;
	data[offset+1] = (uchar)(addr>>8);
}


// ���X�g�t�@�C��1�s�����
LvASMtool::listline LvASMtool::analyzeListLine(string listLine) {

	// �m�F�p
	string analyzeLine = listLine;

	listline lineData;

	// ;�ȍ~���폜
	if(analyzeLine.find(";")!=std::string::npos)	analyzeLine.erase(analyzeLine.find(";"),analyzeLine.length());

	if(analyzeLine.length() < 4) {
		lineData.filename = "";
		lineData.insnum = -1;
	}
	else {
		// �擪3������16�i���̐��l���`�F�b�N

		if(analyzeLine[3] != ' ' && analyzeLine[3] != '\t') {
			throw string("���X�g�t�@�C���̕��@���Ԉ���Ă܂�:" + listLine);
		}
		std::string lineNum = analyzeLine;
		lineNum.resize(3);

		char *lineNumChar = new char[4];
		lineNumChar = (char*)lineNum.c_str();

		// ALL��0x0200 �ǂ��ł�ASM�p
		if(toupper(lineNumChar[0]=='A') && toupper(lineNumChar[1]=='L') && toupper(lineNumChar[2]=='L')) {
			lineData.insnum = 0x0200;
		}
		else {
			if(!(isxdigit(lineNumChar[0]) && isxdigit(lineNumChar[1]) && isxdigit(lineNumChar[2]))) {
				throw string("���X�g�t�@�C���̕��@���Ԉ���Ă܂�:" + listLine);
			}
			lineData.insnum = (int)strtol(lineNumChar,nullptr,16);
			if(lineData.insnum >= 0x200)	throw "�w��ł���Level�ԍ���000-1FF�ł��B:" + listLine + "\n";
		}
		delete lineNumChar;

		// ��������t�@�C�����擾
		// �擪4�����폜
		analyzeLine.erase(0,4);

		if(analyzeLine.find(".asm") == std::string::npos)	throw ".asm�t�@�C�����w�肵�Ă��������B:" + listLine + "\n";

		analyzeLine.erase(analyzeLine.find(".asm")+4,analyzeLine.length());
		lineData.filename = analyzeLine;
	}
	return lineData;
}
