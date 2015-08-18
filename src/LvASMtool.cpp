
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

	if(romSize < BASE_ADDR) {
		throw string("rom�e�ʂ��g�����Ă��������B\n");
	}
	if(romFile.findData((void*)"Lunar Magic",11,0x07F0A0) != 0x07F0A0) {
		throw string("Lunar Magic�ŉ������Ă��������B\n");
	}


	insAddr = romFile.findData((void*)"LevelASM tool",13,BASE_ADDR) - 0x10;
	if(insAddr>=0) {
		lvASMver = romFile.getRomData(insAddr+0x09)<<8 | romFile.getRomData(insAddr+0x08);
	}
}

LvASMtool::~LvASMtool() {}

LvASMtool::ELvASMver LvASMtool::checkLvASMver() {
	if(insAddr<0) return _nothing;
	if(lvASMver==LEVELASM_CODE_VERSION)		return _now;
	else if(lvASMver<LEVELASM_CODE_VERSION)	return _old;
	else									return _new;
}

// LevelASM MainCode�}��
int LvASMtool::insertLevelASMcode() {
	if(insAddr<0) {
		insAddr = romFile.findFreeSpace(BASE_ADDR,romSize,0x1000);
		if(insAddr<0) throw string("LevelASM��}������󂫗̈悪������܂���ł����B");
	}
	return insertLevelASMcode(insAddr);
}

int LvASMtool::insertLevelASMcode(int insAddr) {
	// PC:0x0027CC(SNES:0x00A5CC)�ɑ}������R�[�h
	uchar levelASMinitHijack[] = {
			0x22,0xFF,0xFF,0xFF,0x20,0x60,0x98,0x20,0x2F,0x92,0x20,0x29,0x9F,0x20,0x1A,0x8E,
			0x22,0xFF,0xFF,0xFF,0x80,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00
	};

	// PC:0x002442(SNES0x80A242)�ɑ}������R�[�h
	uchar levelASMmainHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x02DABB(SNES0x05D8BB)�ɑ}������R�[�h
	uchar levelNumHijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// LevelASM ���s�R�[�h
	uchar levelASMmainCode[] = {
			0x6B,0xC2,0x30,0x4B,0x62,0x1B,0x00,0xAD,0x0B,0x01,0x0A,0x6D,0x0B,0x01,0xAA,0xBF,
			0xFF,0xFF,0xFF,0x48,0xBF,0xFF,0xFF,0xFF,0xEB,0x48,0x7B,0x8F,0x05,0xBD,0x7E,0xE2,
			0x30,0x68,0x6B,0x6B,0xC2,0x30,0x4B,0x62,0x16,0x00,0xAD,0x0B,0x01,0x0A,0x6D,0x0B,
			0x01,0xAA,0xBF,0xFF,0xFF,0xFF,0x48,0xBF,0xFF,0xFF,0xFF,0xEB,0x48,0xE2,0x30,0x68,
			0x6B,0xAD,0xD4,0x13,0xF0,0x04,0x5C,0x5B,0xA2,0x80,0x5C,0x8A,0xA2,0x80,0x8D,0x0B,
			0x01,0x0A,0x18,0x65,0x0E,0x6B,0xC2,0x30,0x8B,0xA2,0x03,0x07,0xA0,0x05,0x09,0xA9,
			0xEF,0x01,0x54,0x00,0x00,0xAB,0xAE,0x01,0x07,0x8E,0x03,0x09,0xE2,0x30,0x6B,0x00
	};

	int insSNESaddr = convPCtoSNES(insAddr) | 0x800000;

	writeLongAddr(levelASMinitHijack,0x01,insSNESaddr+0x0081);	// JSL InitCode
	writeLongAddr(levelASMinitHijack,0x11,insSNESaddr+0x00D6);	// JSL CopyPal

	writeLongAddr(levelNumHijack,0x01,insSNESaddr+0x00CE);		// JSL LevelNum

	writeLongAddr(levelASMmainHijack,0x01,insSNESaddr+0x00A4);	// JML MainCode

	writeLongAddr(levelASMmainCode,0x10,insSNESaddr+0x0401);	// LDA.l InitPtr+$01,x
	writeLongAddr(levelASMmainCode,0x15,insSNESaddr+0x0400);	// LDA.l InitPtr,x
	writeLongAddr(levelASMmainCode,0x33,insSNESaddr+0x0A01);	// LDA.l Mainptr+$01,x
	writeLongAddr(levelASMmainCode,0x38,insSNESaddr+0x0A00);	// LDA.l Mainptr,x

	// �{�ƃR�[�h��������
	romFile.writeData(levelASMinitHijack,sizeof(levelASMinitHijack),0x0025CC);
	romFile.writeData(levelASMmainHijack,sizeof(levelASMmainHijack),0x002242);
	romFile.writeData(levelNumHijack,sizeof(levelNumHijack),0x02D8BB);

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
	uchar InitPtrs[0x0600];
	uchar MainPtrs[0x0600];

	// INIT MAIN��RTL�Ŗ��߂�
	int RTLaddr = convPCtoSNES(insAddr+0x7F)|0x800000;
	for(int i=0;i<0x600;i+=3) {
		writeLongAddr(InitPtrs,i,RTLaddr);
		writeLongAddr(MainPtrs,i,RTLaddr);
	}

	// Level���̑}���t�@�C���� 1st pass�ŃJ�E���g�A2nd pass�Ŏg����
	ushort insFileNum[0x200];
	//�@Level���̌��݂̑}���t�@�C���� 2nd pass�Ŏg����
	ushort nowInsFileNum[0x200];
	for(int i=0;i<0x200;i++) {
		insFileNum[i] = 0;
		nowInsFileNum[i] = 0;
	}

	string lineBuf;
	ifstream listFile;

	listline lineData;
	addr addrData;
	map<string,addr> addrList;

	printf("list���m�F\n");
	// 1st pass Level���̑}���t�@�C�����`�F�b�N
	listFile.open(listName.c_str());
	if(!listFile) throw string("���X�g�t�@�C�����J���܂���ł����B\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);

		// 4�����ȉ��͖���
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		// ���Level�ɑ}���\��asm�̐��́ARATS�^�O�ŕی�\��8,190�Ƃ���B
		if(++insFileNum[lineData.insnum] > 8190) {
			throw string(
					"���Level�ɑ}������asm�̐���8,190�𒴂��܂����B\n"
					"�@ �@ �@ �@ ����ȁ@�[��Ɂ@�܂��J��\n"
					"�@ �@ �@ �@ �Ȃ�������ā@�ƁJ�������\n"
					"�@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ �@ ��");
		}
	}
	listFile.close();

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
			printf("\nLv0x%03X inserting:.\\LevelASM\\%s\n",lineData.insnum,lineData.filename.c_str());
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
			int xAddr = Xkaser.insertRatsASM(&romFile,".\\LevelASM\\"+lineData.filename,8,0,0xFFF0);
			int xSize = Xkaser.getASMsize()+0x10;
			if(Xkaser.isError()) {
				throw Xkaser.getSimpleErrMes();
			}
			romFile.writeData((void*)"LEVELASM",8,xAddr+0x08);

			addrData.init = Xkaser.getOffset("INIT");
			if(addrData.init < 0) throw lineData.filename + ": print \"INIT \",pc��������܂���ł����B\n";
			addrData.main = Xkaser.getOffset("MAIN");
			if(addrData.main < 0) throw lineData.filename + ": print \"MAIN \",pc��������܂���ł����B\n";
			addrData.init |= 0x800000;
			addrData.main |= 0x800000;
			addrList[lineData.filename] = addrData;
			if(infoLevel>0) {
				printf("Addr:0x%06X Size:%dBytes \nINIT:0x%06X MAIN:0x%06X\n",convPCtoSNES(xAddr)|0x800000,xSize,addrData.init,addrData.main);
			}
		}

		nowInsFileNum[lineData.insnum]++;

		// �Ώۂ�Level�ɑ}������ASM�̐�����̏ꍇ
		if(insFileNum[lineData.insnum] == 1) {
			// RTL�ŃW�����v����̂Ŏ��O�Ƀf�N�������g���Ă����B
			writeLongAddr(InitPtrs,lineData.insnum*3,addrData.init-1);
			writeLongAddr(MainPtrs,lineData.insnum*3,addrData.main-1);
		}
		// �Ώۂ�Level�ɑ}������ASM�̐�����������ꍇ�AJSL JML�����ō\�����ꂽ�R�[�h��������������B
		/* �ڍ�:
		 *  ��̂���ȃR�[�h��}�����܁[���B
		 *  db "STAR" dw insSize-1,insSize-1^$FFFF
		 *  db "LEVELASM"
		 *  print "INIT ",pc
		 *  JSL !AA_INIT
		 *  JML !BB_INIT
		 *  print "MAIN ",pc
		 *  JSL !AA_MAIN
		 *  JML !BB_MAIN
		 * �}���\�Ȑ���RATS�ŕی�\��65536bytes����w�b�_16bytes�������A
		 * 1�������INIT MAIN�ŏ�����8bytes�Ŋ��������ł���8190�ł��B
		 * 8190��LevelASM��}������Level ���Ă݂����ł��l�B
		 */
		else if(insFileNum[lineData.insnum] > 1) {
			int asmInsAddr;
			uchar instBuf[4];
			// ����}���̏ꍇ �K�v�ȗ̈���m��
			if(nowInsFileNum[lineData.insnum] == 1) {
				int insSize = insFileNum[lineData.insnum]*8+0x10;
				asmInsAddr = romFile.findFreeSpace(BASE_ADDR,insSize);
				if(asmInsAddr < 0)	throw string("LevelASM��}�����邾����rom�̈悪������܂���ł����B\n");
				if(infoLevel > 0) {
					printf("�̈�m�� Addr:%06X Size:%dBytes\n",convPCtoSNES(asmInsAddr)|0x800000,insSize);
				}

				romFile.writeRATSdata(nullptr,insSize-0x08,asmInsAddr);	// RATS�^�O��������
				romFile.writeData((void*)"LEVELASM",8,asmInsAddr+0x08);	// ���ʎq��������

				// 0x00�̂܂܂���FreeSpace�F����󂯂Ă��܂��̂Ŋm�ۂ����̈��NOP�Ŗ��߂�
				uchar NOP = 0xEA;
				romFile.writeReptData((void*)&NOP,1,insSize-0x10,asmInsAddr+0x10);

				writeLongAddr(InitPtrs,lineData.insnum*3,convPCtoSNES((asmInsAddr+0x0F))|0x800000);
				writeLongAddr(MainPtrs,lineData.insnum*3,convPCtoSNES((asmInsAddr+0x0F+insFileNum[lineData.insnum]*4))|0x800000);
			}
			else {
				asmInsAddr = (InitPtrs[lineData.insnum*3] | InitPtrs[lineData.insnum*3+1]<<8 | InitPtrs[lineData.insnum*3+2]<<16) - 0x0F;
				asmInsAddr = convSNEStoPC(asmInsAddr&0x7FFFFF);
			}
			// �Ō�Ȃ�JMLXXYYZZ ����ȊO�Ȃ�JSL XXYYZZ
			instBuf[0] = nowInsFileNum[lineData.insnum] == insFileNum[lineData.insnum] ? 0x5C : 0x22;

			// INIT��������
			writeLongAddr(instBuf,1,addrData.init);
			romFile.writeData(instBuf,4,asmInsAddr+0x0C+nowInsFileNum[lineData.insnum]*4);

			// MAIN��������
			writeLongAddr(instBuf,1,addrData.main);
			romFile.writeData(instBuf,4,asmInsAddr+0x0C+(insFileNum[lineData.insnum]+nowInsFileNum[lineData.insnum])*4);
		}
		// �����̊ԈႢ�ő}������\��̖���Level���Ώۂ̏ꍇ
		else {
			throw string("�}���\\��̖���Level�ԍ��ɑ}�����������悤�Ƃ��܂����B\n");	//�_��������D����������
		}
	}
	romFile.writeData(InitPtrs,0x600,insAddr+0x0400);
	romFile.writeData(MainPtrs,0x600,insAddr+0x0A00);

	return 0;
}

// �}���ς݂�LevelASM���폜
int LvASMtool::deleteLevelASM() {
	// �Ȃ�ƂȂ��p��
	int delCount = 0;
	int addr = BASE_ADDR;

	while(true) {
		addr = romFile.findData((void*)"LEVELASM",8,addr);
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

	if(lvASMver<0x0101) {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	else {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindMainHijack,sizeof(rewindLaterHijack),0x0022EA);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);

	}

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
	return ((addr&0xFFFF0000)>>1)|(addr&0x7FFF);
}

// uchar�^�̔z���24bit�̃f�[�^���������ނ���
void LvASMtool::writeLongAddr(uchar* data,int offset,int addr) {
	data[offset] = (uchar)addr;
	data[offset+1] = (uchar)(addr>>8);
	data[offset+2] = (uchar)(addr>>16);
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

		char* lineNumChar = new char[4];
		lineNumChar = (char*)lineNum.c_str();

		if(!(isxdigit(lineNumChar[0]) && isxdigit(lineNumChar[1]) && isxdigit(lineNumChar[2]))) {
			throw string("���X�g�t�@�C���̕��@���Ԉ���Ă܂�:" + listLine);
		}

		lineData.insnum = (int)strtol(lineNumChar,nullptr,16);
		if(lineData.insnum >= 0x200)	throw "�w��ł���Level�ԍ���000-1FF�ł��B:" + listLine + "\n";

		// ��������t�@�C�����擾
		// �擪4�����폜
		analyzeLine.erase(0,4);

		if(analyzeLine.find(".asm") == std::string::npos)	throw ".asm�t�@�C�����w�肵�Ă��������B:" + listLine + "\n";

		analyzeLine.erase(analyzeLine.find(".asm")+4,analyzeLine.length());
		lineData.filename = analyzeLine;
	}
	return lineData;
}
