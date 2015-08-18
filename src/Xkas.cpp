
#include "Xkas.h"

Xkas::Xkas() {
	romData = nullptr;
}

Xkas::Xkas(Rom* romData,string asmPath) {
	allocSpace(romData,asmPath,0,0x8000);
}

Xkas::Xkas(Rom* romData,string asmPath,int sizeOffset) {
	allocSpace(romData,asmPath,sizeOffset,0x8000);
}

Xkas::Xkas(Rom* romData,string asmPath,int sizeOffset,int limitSize) {
	allocSpace(romData,asmPath,sizeOffset,limitSize);
}

Xkas::~Xkas() {
}

int Xkas::allocSpace(Rom* romData,string asmPath,int sizeOffset,int limitSize) {

	Xkas::romData = romData;
	Xkas::asmPath = asmPath;
	Xkas::sizeOffset = sizeOffset;
	Xkas::limitSize = limitSize;

	ASM.open(asmPath.c_str(),ios::in|ios::binary);
	if(!ASM) {
		xkasErr = EerrType::asmfile_not_found;
		return -1;
	}

	fstream tmpbin("tmpasm.bin",ios::out|ios::trunc);
	if(!tmpbin) {
		xkasErr = EerrType::tmpbin_create_failed;
		return -1;
	}
	tmpbin.close();

	fstream tmpasm("tmpasm.asm",ios::in|ios::out|ios::trunc);
	if(!tmpasm) {
		xkasErr = EerrType::tmpasm_create_failed;
	}

	ASM.seekg(0,std::ios::end);	int ASMfileSize = ASM.tellg();	ASM.seekg(0);

	char* tmpContents = new char[ASMfileSize];
	char xkasOrgLine[] = "lorom\x0D\x0Aorg $008000\x0D\x0A";
	ASM.read(tmpContents,ASMfileSize);
	tmpasm.write(xkasOrgLine,sizeof(xkasOrgLine)-1);
	tmpasm.write(tmpContents,ASMfileSize);
	tmpasm.close();
	delete[] tmpContents;

	// 1��� - ���A�Z���u��
	// �A�Z���u����̗e�ʂ𒲂ׁA�}���ɕK�v�ȗ̈���m�ۂ���B
	system("xkas.exe tmpasm.asm tmpasm.bin > xkas.log");

	tmpbin.open("tmpasm.bin",std::ios::in | std::ios::out | std::ios::binary);
	if(!tmpbin)	{
		xkasErr = EerrType::tmpbin_open_failed;
		return -1;
	}

	// asm�̗e�ʂ��m�F
	tmpbin.seekg(0,std::ios::end);
	asmSize = tmpbin.tellg();
	allocSize = asmSize + sizeOffset;
	tmpbin.seekg(0);
	tmpbin.close();

	if(allocSize == 0) {
		xkasErr = EerrType::assemble_size_zero;
		return -1;
	}
	else if(allocSize >= 0xFFF0) {
		xkasErr = EerrType::assemble_size_over;
		return -1;
	}

	// rom�̋󂫗̈���m�� ������Ȃ��ꍇ�G���[���o���ďI��
	allocAddr = romData->findFreeSpace(startAddr,romData->getRomSize()-endAddr,allocSize);

	if(allocAddr<0) {
		xkasErr = EerrType::insert_failed;
		return -1;
	}
	xkasErr = EerrType::first_pass;
	return allocAddr;
}

int Xkas::insertASM() {
	return insertASM(0);
}

int Xkas::insertASM(int insertOffset) {
	if(xkasErr!=EerrType::first_pass) return -1;

	int insertAddr = allocAddr+insertOffset;
	int insertSNESaddr = convPCtoSNES(insertAddr)|0x800000;

	char xkasOrgLine[] = "lorom\x0D\x0Aorg $008000\x0D\x0A";
	const char* HEX_DIGIT = "0123456789ABCDEF";

	xkasOrgLine[12] = HEX_DIGIT[(insertSNESaddr>>20)&0x0F];
	xkasOrgLine[13] = HEX_DIGIT[(insertSNESaddr>>16)&0x0F];
	xkasOrgLine[14] = HEX_DIGIT[(insertSNESaddr>>12)&0x0F];
	xkasOrgLine[15] = HEX_DIGIT[(insertSNESaddr>>8)&0x0F];
	xkasOrgLine[16] = HEX_DIGIT[(insertSNESaddr>>4)&0x0F];
	xkasOrgLine[17] = HEX_DIGIT[(insertSNESaddr)&0x0F];

	fstream tmpasm("tmpasm.asm",std::ios::in | std::ios::out | std::ios::binary);	tmpasm.seekg(0);
	tmpasm.write(xkasOrgLine,sizeof(xkasOrgLine)-1);
	tmpasm.close();

	// 2��� - �A�Z���u��
	system("xkas.exe tmpasm.asm tmpasm.bin > xkas2.log");

	std::ifstream tmplog("xkas2.log");
	if(!tmplog)	{
		xkasErr = EerrType::tmplog_open_failed;
		return -1;
	}

	std::string word;
	int offset;
	while(tmplog>>word) {
		if(word == "error:") {
			xkasErr = EerrType::assemble_error;
			tmplog.close();
			return -1;
		}
		tmplog.setf(std::ios::hex,std::ios::basefield);
		tmplog>>offset;
		labelData[word] = offset;
	}
	tmplog.close();

	// tmpasm.bin�̃f�[�^���o�b�t�@�ɏ�������
	fstream tmpbin("tmpasm.bin",std::ios::in | std::ios::out | std::ios::binary);
	if(!tmpbin) {
		xkasErr = EerrType::tmpbin_open_failed;
		return -1;
	}

	unsigned char* binContents = new unsigned char[asmSize];
	tmpbin.seekg(insertAddr);
	for(int i=0;i<asmSize;i++) {
		binContents[i] = tmpbin.get();
	}
	tmpbin.close();
	romData->writeData(binContents,asmSize,insertAddr);

	xkasErr = EerrType::success;
	delete[] binContents;
	return insertAddr;
}

int Xkas::insertRatsASM(Rom* romData,string asmPath) {
	return insertRatsASM(romData,asmPath,0,0,0x8000);
}

int Xkas::insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset) {
	return insertRatsASM(romData,asmPath,insertOffset,sizeOffset,0x8000);
}

/* RATS�^�O���݂�ASM��}������
* Rom*		romData			�}������rom�t�@�C��
* string	asmPath			�}������asm�t�@�C��
* int		insertOffset	asm��}������A�h���X�̍��� �ʏ��0
* int		sizeOffset		�m�ۂ���̈�̍��� �ʏ��0
* int		limitSize		asm�̏���T�C�Y �ʏ��0x8000bytes
*/
int Xkas::insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset,int limitSize) {
	int addr = allocSpace(romData,asmPath,sizeOffset+insertOffset+8,limitSize);
	insertASM(insertOffset+8);
	romData->writeRATSdata(nullptr,allocSize-8,addr);
	return addr;
}

// �}����������̈��ݒ�
// startAddr:�J�n�A�h���X
// endAddr:�I���A�h���X�܂ł̋���
void Xkas::setInsertRange(int startAddr,int endAddr) {
	Xkas::startAddr = startAddr;
	Xkas::endAddr = endAddr;
}

// �ȒP�ȃG���[���b�Z�[�W��Ԃ��B
string Xkas::getSimpleErrMes() {
	return getSimpleErrMes(true);
}

string Xkas::getSimpleErrMes(bool Japanese) {
	if(Japanese) {
		switch(xkasErr) {
		case EerrType::unexecuted:
			return "�A�Z���u�������s�ł��B\n";
		case EerrType::first_pass:
			return "���A�Z���u���i�K�ł��B\n";
		case EerrType::asmfile_not_found:
			return asmPath + ": asm�t�@�C����������܂���ł����B";
		case EerrType::tmpasm_create_failed:
			return "tmpasm.asm�̍쐬�Ɏ��s���܂����B\n";
		case EerrType::tmpbin_create_failed:
			return "tmpasm.bin�̍쐬�Ɏ��s���܂����B\n";
		case EerrType::tmpbin_open_failed:
			return "tmpasm.bin���J���܂���ł����B\n";
		case EerrType::tmplog_open_failed:
			return "xkas.log���J���܂���ł����B\n";
		case EerrType::assemble_size_zero:
			return "�A�Z���u����̃T�C�Y��0byte�ł��B\n";
		case EerrType::assemble_size_over:
			return "�A�Z���u����̃T�C�Y�����e�ʂ��I�[�o�[���܂����B\n"
					"���R�[�h��org���܂܂�Ă���\\��������܂��B\n";
		case EerrType::insert_failed:
			return "asm�t�@�C����}������X�y�[�X��������܂���ł����B\n";
		case EerrType::assemble_error:
			return asmPath + ": asm�ɃG���[������܂����B\n";

		case EerrType::success:
				return "asm�̑}���ɐ������܂����B\n";
		}
	}
	// TODO:�p��̃G���[���b�Z�[�W���p�ӂ��悤�ˁB
	else {

	}
	return "";
}

Xkas::EerrType Xkas::getErrType() {
	return xkasErr;
}

bool Xkas::isError() {
	return xkasErr != EerrType::success;
}

// �m�ۂ����A�h���X���擾
int Xkas::getAllocAddr() {
	return allocAddr;
}

// ASM�̃T�C�Y���擾
int Xkas::getASMsize() {
	return asmSize;
}

// ���x����offset���擾
int Xkas::getOffset(string labelName) {
	map<string,int>::const_iterator n = labelData.find(labelName);
	if(n == labelData.end()) return -1;
	return n->second;
}

int Xkas::convPCtoSNES(int addr) {
	if(addr<0) return -1;
	return ((addr&0xFFFF8000)<<1)|(addr&0x7FFF)|0x8000;
}

int Xkas::convSNEStoPC(int addr) {
	if(!(addr&0x8000)) return -1;
	return ((addr&0xFFFF0000)>>1)|(addr&0x7FFF);
}
