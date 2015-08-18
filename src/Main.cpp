
#include <iostream>
#include <fstream>
#include <string>

#include "Rom.h"
#include "Xkas.h"
#include "LvASMtool.h"
#include "windows.h"

using namespace std;

#define LENGTH(ary) ((sizeof(ary))/sizeof((ary)[0]))

const char* WELCOME_MESSAGE =
		"****************************************\n"
		"* LevelASM tool version 1.01\n"
		"* usage:LvASMtool SMW.smc list.txt\n"
		"****************************************\n";

// (y/n)�̕ԓ���҂�
// y�̂Ƃ�true��Ԃ�
bool waitResponse() {
	string res;
	cin >> res;
	return (res=="y" || res=="Y" || res=="yes" || res=="Yes" || res=="YES");
}

string getFullPath(string str) {
	// �_�u���N�H�[�e�[�V����������
	if(str[0]=='\"' && str[str.length()-1]=='\"') {
		str.erase(0,1);
		str.erase(str.length()-1,1);
	}

	// �t�@�C�������t���p�X�ɕύX
	if(str.size() <=1 || !(str[1]==':' || (str[0]=='\\' && str[1]=='\\'))) {
		str = ".\\" + str;
	}
	return str;
}

// TODO:����LunarDLL�ɗ���K�v�Ȃ��ˁHPC<=>SNES�ϊ����\�b�h�p�ӂ��č�����Ⴆ
int main(int argc,char** argv) {
	char romPathBuf[256];
	char listPathBuf[256];

	// .ini�t�@�C����������Ȃ��ꍇ��������
	fstream configFile("LvASMtool.ini");
	if(!configFile) {
		configFile.open("LvASMtool.ini",ios::out|ios::trunc);
		configFile << "[LvASM]" << endl;
		configFile << "RomPath =" << endl;
		configFile << "ListPath =" << endl;
		configFile << "Info = 0" << endl;;
		configFile.close();
	}
	else {
		configFile.close();
	}

	int infoLevel = GetPrivateProfileInt("LvASM","Info",0,"./LvASMtool.ini");

	if(argc<3) {
		GetPrivateProfileString("LvASM","RomPath","null",romPathBuf,sizeof(romPathBuf),"./LvASMtool.ini");
		if(strcmp(romPathBuf,"null") != 0 && strlen(romPathBuf)>0) {
			argv[1] = romPathBuf;
			argc++;
		}

		GetPrivateProfileString("LvASM","ListPath","null",listPathBuf,sizeof(listPathBuf),"./LvASMtool.ini");
		if(strcmp(listPathBuf,"null") != 0 && strlen(romPathBuf)>0) {
			argv[2] = listPathBuf;
			argc++;
		}

		if(argc!=3) cout << WELCOME_MESSAGE << endl;
	}
	try {
		string romFileName = "";
		string listFileName = "";
		ifstream listFile;
		ifstream romFile;
		do {
			romFile.clear();
			romFileName = "";
			if(argc==3) {
				argc--;
				romFileName = argv[1];
			}
			else {
				cout << "rom�������:";
				cin >> romFileName;
				romFileName = getFullPath(romFileName);
			}
			romFile.open(romFileName.c_str());
			if(!romFile) {
				cout << "Error: rom���J���܂���ł����B" << endl;
				continue;
			}
		}
		while(!romFile);

		do {
			listFile.clear();
			listFileName = "";
			if(argc==2) {
				argc--;
				listFileName = argv[2];
			}
			else {
				cout << "list�������:";
				cin >> listFileName;
			}
			if(listFileName == "Uninstall") {
				break;
			}
			listFileName = getFullPath(listFileName);
			listFile.open(listFileName.c_str());
			if(!listFile) {
				cout << "Error: list���J���܂���ł����B" << endl;
				continue;
			}
		}
		while(!listFile);

		romFile.close();
		listFile.close();

		LvASMtool LvASM(romFileName,listFileName,infoLevel);

		// LevelASM �A���C���X�g�[�����[�h
		if(listFileName == "Uninstall") {
			switch(LvASM.checkLvASMver()) {
			case LvASMtool::ELvASMver::_nothing:	// ���}��
				throw string("LevelASM���}���ł��B\n");
				break;
			case LvASMtool::ELvASMver::_new:		// rom���̃o�[�W�����̕����V����
				printf("�x��:rom����LevelASM�̃o�[�W�����̕����V�����ł��B\n");
				printf("rom ver:0x%04X tool ver:0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());
				printf("����ɃA���C���X�g�[���ł��Ȃ��\\��������܂����A���s���܂����H�iy/n)\n");
				if(waitResponse()) {
					LvASM.deleteLevelASM();
					LvASM.deleteLevelASMcode();
					LvASM.rewindHijackCode();
					LvASM.writeRomFile();
				}
				else {
					throw string("�A���C���X�g�[���𒆎~���܂����B\n");
				}
				break;
			case LvASMtool::ELvASMver::_old:
			case LvASMtool::ELvASMver::_now:
				printf("LevelASM���A���C���X�g�[�����܂����H(y/n)\n");
				if(waitResponse()) {
					LvASM.deleteLevelASM();
					LvASM.deleteLevelASMcode();
					LvASM.rewindHijackCode();
					LvASM.writeRomFile();
				}
				else {
					throw string("�A���C���X�g�[���𒆎~���܂����B\n");
				}
				break;
			}
			printf("�A���C���X�g�[�����������܂����B\n");
		}
		else {
			// LevelASM �o�[�W�����m�F
			int addr;
			switch(LvASM.checkLvASMver()) {
			case LvASMtool::ELvASMver::_nothing:	// �V�K�}��
				printf("LevelASM���C���X�g�[�����܂��B\n");
				addr = LvASM.insertLevelASMcode();
				printf("�}������ PCaddr:0x%06X\n",addr);
				break;
			case LvASMtool::ELvASMver::_old:		// �o�[�W�����A�b�v
				printf("LevelASM���o�[�W�����A�b�v���܂��B\n"
						"version:0x%04X -> 0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());

				// ��U�A���C���X�g�[��
				LvASM.rewindHijackCode();
				addr = LvASM.deleteLevelASMcode();

				LvASM.insertLevelASMcode(addr);
				break;
			case LvASMtool::ELvASMver::_new:		// rom���̃o�[�W�����̕����V����
				printf("�x��:rom����LevelASM�̃o�[�W�����̕����V�����ł��B\n");
				printf("rom ver:0x%04X tool ver:0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());
				printf("�}���𑱍s���܂����H(y/n)\n");
				if(!waitResponse()) throw string("�}���𒆎~���܂����B\n");

				LvASM.rewindHijackCode();
				addr = LvASM.deleteLevelASMcode();

				LvASM.insertLevelASMcode(addr);
				break;
			case LvASMtool::ELvASMver::_now:		// �����o�[�W���� �ꉞ�����Ƃ��`
				break;
			}
			LvASM.deleteLevelASM();

			LvASM.insertLevelASM();
			LvASM.writeRomFile();
		}
	}
	catch(string str) {
		cout << "Error: " << str << endl;
		remove("xkas2.log");
		system("pause");
		return 0;
	}
	remove("xkas.log");
	remove("xkas2.log");
	remove("tmpasm.asm");
	remove("tmpasm.bin");
	printf("����\n");
	system("pause");
	return 0;
}
