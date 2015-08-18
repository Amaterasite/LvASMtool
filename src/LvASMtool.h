
#ifndef LVASMTOOL_H_
#define LVASMTOOL_H_

#include <fstream>
#include <string>
#include <iostream>
#include <map>

#include "Rom.h"
#include "Xkas.h"

using namespace std;

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned int	uint;

class LvASMtool {
public:
	// new���\��ꂾ�����̖Y��Ă�
	// �A���_�[�o�[���Č떂����
	enum ELvASMver {
		_nothing,	// LevelASM���}��
		_now,		// rom���ƃc�[�����������o�[�W����
		_old,		// rom���̕����Â�
		_new,		// rom���̕����V����
	};
	LvASMtool();
	LvASMtool(string romName,string listName,int infoLevel);
	~LvASMtool();

	// LevelASM�̃o�[�W�������m�F
	ELvASMver checkLvASMver();

	// LevelASM MainCode��}��
	int insertLevelASMcode();
	int insertLevelASMcode(int insAddr);

	// LevelASM��}��
	int insertLevelASM();

	// LevelASM MainCode���폜
	int deleteLevelASMcode();

	// �}���ς�LevelASM���폜
	int deleteLevelASM();

	// rom�t�@�C���ɕҏW�����f�[�^����������
	void writeRomFile();

	// �}�������R�[�h��߂�
	void rewindHijackCode();

	ushort getRomLvASMver();

	// �萔�̃A�N�Z�b�T��݂���̂��Ăǂ��Ȃ́H
	ushort getToolLvASMver();

private:
	// ���X�g�t�@�C���p�\����
	struct listline {
		int insnum;			// �}������Level�ԍ�
		string filename;	// �}������asm�t�@�C���̖��O(�g���q����)
	};

	// �}�������A�h���X �����i�[����������Signed�Œ�`�`
	struct addr {
		short bank;
		int init;
		int early;
		int later;
	};

	string romName = "";
	string listName = "";

	ifstream listFile;
	Rom romFile;

	int romSize = 0;
	int insAddr = -1;
	int insEndAddr = 0;
	ushort lvASMver = 0;

	int infoLevel = 0;

	// Header Lorom�̂ݑΉ�
	int convPCtoSNES(int addr);
	int convSNEStoPC(int addr);

	// LeveASM ���s�R�[�h�̃o�[�W�����R�[�h
	// �c�[���{�̂̃o�[�W�����Ƃ͕�
	const ushort LEVELASM_CODE_VERSION = 0x0101;

	// LevelASM ���s�R�[�h�̐擪�ɑ}��������
	// "LevelASM tool"�̗L���œ����ς݂�����
	const char* LEVELASM_HEADER =
	//_______0123456789ABCDEF
			"LevelASM tool   "	// 0x00
			"Version:1.01    "	// 0x10
			"Date:2015/08/20 "	// 0x20
			"Author:88-CHAN /"	// 0x30
			" 33953YoShI     "	// 0x40
			"                "	// 0x50
			"                ";	// 0x60

	// ���ꖢ���̃A�h���X�ɂ̓f�[�^��}�����Ȃ�
	const int BASE_ADDR = 0x080000;

	void writeLongAddr(uchar* data,int offset,int addr);
	void writeWordAddr(uchar* data,int offset,int addr);
	listline analyzeListLine(string listLine);
};

#endif /* LVASMTOOL_H_ */
