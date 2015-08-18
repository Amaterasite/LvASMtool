
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

// (y/n)の返答を待つ
// yのときtrueを返す
bool waitResponse() {
	string res;
	cin >> res;
	return (res=="y" || res=="Y" || res=="yes" || res=="Yes" || res=="YES");
}

string getFullPath(string str) {
	// ダブルクォーテーションを除去
	if(str[0]=='\"' && str[str.length()-1]=='\"') {
		str.erase(0,1);
		str.erase(str.length()-1,1);
	}

	// ファイル名をフルパスに変更
	if(str.size() <=1 || !(str[1]==':' || (str[0]=='\\' && str[1]=='\\'))) {
		str = ".\\" + str;
	}
	return str;
}

// TODO:もうLunarDLLに頼る必要なくね？PC<=>SNES変換メソッド用意して削っちゃえ
int main(int argc,char** argv) {
	char romPathBuf[256];
	char listPathBuf[256];

	// .iniファイルが見つからない場合生成する
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
				cout << "rom名を入力:";
				cin >> romFileName;
				romFileName = getFullPath(romFileName);
			}
			romFile.open(romFileName.c_str());
			if(!romFile) {
				cout << "Error: romを開けませんでした。" << endl;
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
				cout << "list名を入力:";
				cin >> listFileName;
			}
			if(listFileName == "Uninstall") {
				break;
			}
			listFileName = getFullPath(listFileName);
			listFile.open(listFileName.c_str());
			if(!listFile) {
				cout << "Error: listを開けませんでした。" << endl;
				continue;
			}
		}
		while(!listFile);

		romFile.close();
		listFile.close();

		LvASMtool LvASM(romFileName,listFileName,infoLevel);

		// LevelASM アンインストールモード
		if(listFileName == "Uninstall") {
			switch(LvASM.checkLvASMver()) {
			case LvASMtool::ELvASMver::_nothing:	// 未挿入
				throw string("LevelASM未挿入です。\n");
				break;
			case LvASMtool::ELvASMver::_new:		// rom側のバージョンの方が新しい
				printf("警告:rom側のLevelASMのバージョンの方が新しいです。\n");
				printf("rom ver:0x%04X tool ver:0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());
				printf("正常にアンインストールできない可能\性がありますが、続行しますか？（y/n)\n");
				if(waitResponse()) {
					LvASM.deleteLevelASM();
					LvASM.deleteLevelASMcode();
					LvASM.rewindHijackCode();
					LvASM.writeRomFile();
				}
				else {
					throw string("アンインストールを中止しました。\n");
				}
				break;
			case LvASMtool::ELvASMver::_old:
			case LvASMtool::ELvASMver::_now:
				printf("LevelASMをアンインストールしますか？(y/n)\n");
				if(waitResponse()) {
					LvASM.deleteLevelASM();
					LvASM.deleteLevelASMcode();
					LvASM.rewindHijackCode();
					LvASM.writeRomFile();
				}
				else {
					throw string("アンインストールを中止しました。\n");
				}
				break;
			}
			printf("アンインストールを完了しました。\n");
		}
		else {
			// LevelASM バージョン確認
			int addr;
			switch(LvASM.checkLvASMver()) {
			case LvASMtool::ELvASMver::_nothing:	// 新規挿入
				printf("LevelASMをインストールします。\n");
				addr = LvASM.insertLevelASMcode();
				printf("挿入成功 PCaddr:0x%06X\n",addr);
				break;
			case LvASMtool::ELvASMver::_old:		// バージョンアップ
				printf("LevelASMをバージョンアップします。\n"
						"version:0x%04X -> 0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());

				// 一旦アンインストール
				LvASM.rewindHijackCode();
				addr = LvASM.deleteLevelASMcode();

				LvASM.insertLevelASMcode(addr);
				break;
			case LvASMtool::ELvASMver::_new:		// rom側のバージョンの方が新しい
				printf("警告:rom側のLevelASMのバージョンの方が新しいです。\n");
				printf("rom ver:0x%04X tool ver:0x%04X\n",LvASM.getRomLvASMver(),LvASM.getToolLvASMver());
				printf("挿入を続行しますか？(y/n)\n");
				if(!waitResponse()) throw string("挿入を中止しました。\n");

				LvASM.rewindHijackCode();
				addr = LvASM.deleteLevelASMcode();

				LvASM.insertLevelASMcode(addr);
				break;
			case LvASMtool::ELvASMver::_now:		// 同じバージョン 一応書いとく～
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
	printf("完了\n");
	system("pause");
	return 0;
}
