
#include "LvASMtool.h"

LvASMtool::LvASMtool() {}

LvASMtool::LvASMtool(string romName,string listName,int infoLevel) {

	LvASMtool::romName = romName;
	LvASMtool::listName = listName;
	LvASMtool::infoLevel = infoLevel;

	if(!romFile.open(romName)) {
		throw string("romファイルが開けませんでした。");
	}
	romSize = romFile.getRomSize();

	if(romSize < BASE_ADDR) {
		throw string("rom容量を拡張してください。\n");
	}
	if(romFile.findData((void*)"Lunar Magic",11,0x07F0A0) != 0x07F0A0) {
		throw string("Lunar Magicで改造してください。\n");
	}

	// LevelASM挿入可能領域はBank0x40未満
	insEndAddr = romSize > 0x200000 ? 0x200000 : romSize;

	insAddr = romFile.findData((void*)"LevelASM tool",13,BASE_ADDR) - 0x10;
	if(romFile.checkRATSdata(insAddr) < 0)	insAddr = -1;

	// インストール済みの場合バージョン情報取得
	if(insAddr>=0) {
		lvASMver = romFile.getRomData(insAddr+0x09)<<8 | romFile.getRomData(insAddr+0x08);
	}
	// edit1754氏のLevelASMが挿入済みか確認 挿入済みで 挿入アドレス = -1754
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

// LevelASM MainCode挿入
int LvASMtool::insertLevelASMcode() {
	if(insAddr<0) {
		insAddr = romFile.findFreeSpace(BASE_ADDR,insEndAddr,0x1000);
		if(insAddr<0) throw string("LevelASMを挿入する空き領域が見つかりませんでした。");
	}
	return insertLevelASMcode(insAddr);
}

int LvASMtool::insertLevelASMcode(int insAddr) {
	// PC:0x0027CC(SNES:0x00A5CC)に挿入するコード
	uchar levelASMinitHijack[] = {
			0x22,0xFF,0xFF,0xFF,0x20,0x60,0x98,0x20,0x2F,0x92,0x20,0x29,0x9F,0x20,0x1A,0x8E,
			0x22,0xFF,0xFF,0xFF,0x80,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00
	};

	// PC:0x002442(SNES:0x80A242)に挿入するコード
	uchar levelASMearlyHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x0024EA(SNES:00A2EA)に挿入するコード
	uchar levelASMlaterHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA,0xEA
	};

	// PC:0x02DABB(SNES:0x05D8BB)に挿入するコード
	uchar levelNumHijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// LevelASM 実行コード
	uchar levelASMmainCode[] = {
		//  __+0___+1___+2___+3___+4___+5___+6___+7___+8___+9___+A___+B___+C___+D___+E___+F
			0x6B,0xC2,0x30,0x4B,0x62,0x1C,0x00,0xAE,0x0B,0x01,0xBF,0xFF,0x81,0x3F,0x48,0x48,	// 0x00
			0xAD,0x0B,0x01,0x0A,0xAA,0xBF,0x00,0x84,0x3F,0x83,0x02,0x7B,0x8F,0x05,0xBD,0x7E,	// 0x10
			0xE2,0x30,0x68,0x6B,0x6B,0xC2,0x30,0x4B,0x62,0x17,0x00,0xAE,0x0B,0x01,0xBF,0xFF,	// 0x20
			0x81,0x3F,0x48,0x48,0xAD,0x0B,0x01,0x0A,0xAA,0xBF,0x00,0x88,0x3F,0x83,0x02,0xE2,	// 0x30
			0x30,0x68,0x6B,0xAD,0xD4,0x13,0xF0,0x04,0x5C,0x5B,0xA2,0x80,0x5C,0x8A,0xA2,0x80,	// 0x40
			0x68,0x85,0x1D,0x68,0x85,0x1C,0xA9,0x80,0x48,0xF4,0xEF,0xA2,0xC2,0x30,0xAE,0x0B,	// 0x50
			0x01,0xBF,0xFF,0x81,0x3F,0x48,0x48,0xAD,0x0B,0x01,0x0A,0xAA,0xBF,0x00,0x8C,0x3F,	// 0x60
			0x83,0x02,0xE2,0x30,0x68,0x6B,0x8D,0x0B,0x01,0x0A,0x18,0x65,0x0E,0x6B,0xC2,0x30,	// 0x70
			0x8B,0xA2,0x03,0x07,0xA0,0x05,0x09,0xA9,0xEF,0x01,0x54,0x00,0x00,0xAB,0xAE,0x01,	// 0x80
			0x07,0x8E,0x03,0x09,0xE2,0x30,0x6B,0x01,0x07,0x8E,0x03,0x09,0xE2,0x30,0x6B,0x00,	// 0x90
	};

	int insSNESaddr = convPCtoSNES(insAddr) | 0x800000;

	writeLongAddr(levelASMinitHijack,0x01,insSNESaddr+0x0081);	// JSL InitCode
	writeLongAddr(levelASMinitHijack,0x11,insSNESaddr+0x00FE);	// JSL CopyPal

	writeLongAddr(levelNumHijack,0x01,insSNESaddr+0x00F6);		// JSL LevelNum

	writeLongAddr(levelASMearlyHijack,0x01,insSNESaddr+0x00A5);	// JML EarlyMainCode
	writeLongAddr(levelASMlaterHijack,0x01,insSNESaddr+0x00D0);	// JML LaterMainCode

	writeLongAddr(levelASMmainCode,0x0B,insSNESaddr+0x01FF);	// LDA.l BankData-$01,x
	writeLongAddr(levelASMmainCode,0x2F,insSNESaddr+0x01FF);	// LDA.l BankData-$01,x
	writeLongAddr(levelASMmainCode,0x62,insSNESaddr+0x01FF);	// LDA.l BankData-$01,x

	writeLongAddr(levelASMmainCode,0x16,insSNESaddr+0x0400);	// LDA.l InitPtr,x
	writeLongAddr(levelASMmainCode,0x3A,insSNESaddr+0x0800);	// LDA.l EarlyPtr,x
	writeLongAddr(levelASMmainCode,0x6D,insSNESaddr+0x0C00);	// LDA.l LaterPtr,x

	// 本家コード書き換え
	romFile.writeData(levelASMinitHijack,sizeof(levelASMinitHijack),0x0025CC);
	romFile.writeData(levelASMearlyHijack,sizeof(levelASMearlyHijack),0x002242);
	romFile.writeData(levelASMlaterHijack,sizeof(levelASMearlyHijack),0x0022EA);
	romFile.writeData(levelNumHijack,sizeof(levelNumHijack),0x02D8BB);

	// ヘッダ書き込み
	romFile.writeData((void*)LEVELASM_HEADER,0x70,insAddr+0x10);

	// 実行コード書き込み
	romFile.writeData((void*)levelASMmainCode,sizeof(levelASMmainCode),insAddr+0x80);

	// RTASタグ書き込み
	romFile.writeRATSdata(nullptr,0x0FF8,insAddr);

	uchar codeVer[] = {(uchar)(LEVELASM_CODE_VERSION),(uchar)(LEVELASM_CODE_VERSION>>8)};

	// バージョン情報書き込み
	romFile.writeData((void*)codeVer,2,insAddr+0x08);

	return insAddr+0x0200;
}

// LevelASMを挿入
int LvASMtool::insertLevelASM() {
	if(insAddr<0) return -1;
	uchar InitPtrs[0x0400];
	uchar EarlyPtrs[0x0400];
	uchar LaterPtrs[0x0400];
	uchar PtrBank[0x0200];

	// INIT MAINをRTLで埋める
	ushort RTLaddr = convPCtoSNES(insAddr+0x7F);
	for(int i=0;i<0x400;i+=2) {
		writeWordAddr(InitPtrs,i,RTLaddr);
		writeWordAddr(EarlyPtrs,i,RTLaddr);
		writeWordAddr(LaterPtrs,i,RTLaddr);
	}
	uchar RTLbank = (convPCtoSNES(insAddr+0x7F)>>16)|0x80;
	for(int i=0;i<0x0200;i++) {
		PtrBank[i] = RTLbank;
	}

	// Level毎の挿入ファイル数 1st passでカウント、2nd passで使われる
	ushort insFileNum[0x200];
	//　Level毎の現在の挿入ファイル数 2nd passで使われる
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

	printf("listを確認\n");
	// 1st pass Level毎の挿入ファイル数チェック
	listFile.open(listName.c_str());
	if(!listFile) throw string("リストファイルを開けませんでした。\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);

		// 4文字以下は無視
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		// 一つのLevelに挿入可能なasmの数は、bank1個に挿入可能数の2,729個とする。
		if(++insFileNum[lineData.insnum] > 2729) {
			throw string(
					"一つのLevelに挿入するasmの数が2,729を超えました。\n"
					"　 　 　 　 こんな　つーるに　まし゛に\n"
					"　 　 　 　 なっちゃって　と゛うするの\n"
					"　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 完");
		}
	}
	listFile.close();

	// 2nd pass LevelASM挿入
	listFile.open(listName.c_str());
	if(!listFile) throw string("リストファイルを開けませんでした。\n");

	printf("LevelASMを挿入\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		if(infoLevel > 0) {
			printf("\nLv0x%03X inserting:.\\LevelASM\\%s\n",lineData.insnum,lineData.filename.c_str());
		}
		// 指定されたASMファイルが既に挿入済みか確認
		map<string,addr>::const_iterator addrIndex = addrList.find(lineData.filename);
		if(addrIndex != addrList.end()) {
			// 挿入済みならデータを使い回し
			addrData = addrIndex->second;
		}
		else {
			// 初回挿入
			Xkas Xkaser;
			Xkaser.setInsertRange(BASE_ADDR,0);
			int xAddr = Xkaser.insertRatsASM(&romFile,".\\LevelASM\\"+lineData.filename,8,0,0xFFF0);
			int xSize = Xkaser.getASMsize()+0x10;
			if(Xkaser.isError()) {
				throw Xkaser.getSimpleErrMes();
			}
			romFile.writeData((void*)"LEVELASM",8,xAddr+0x08);

			// 各ラベル位置を取得 見つからない物はRTLのアドレスを割り当て

			if(Xkaser.getRTLaddr() < 0) {
				throw string("コードにはRTLが必要です。\n");
			}

			addrData.init = Xkaser.getOffset("INIT");

			if(Xkaser.getOffset("MAIN") < 0) {
				addrData.early = Xkaser.getOffset("EARLY");
				addrData.later = Xkaser.getOffset("LATER");
			}
			else {
				addrData.early = Xkaser.getOffset("MAIN");
				addrData.later = -1;
			}

			if(addrData.init < 0)	addrData.init = Xkaser.getRTLaddr();
			if(addrData.early <0)	addrData.early = Xkaser.getRTLaddr();
			if(addrData.later <0)	addrData.later = Xkaser.getRTLaddr();

			addrData.init &= 0xFFFF;
			addrData.early &= 0xFFFF;
			addrData.later &= 0xFFFF;

			// もういらないけど表示用に残しておく？
			addrData.bank = (convPCtoSNES(xAddr)>>16)|0x80;

			addrList[lineData.filename] = addrData;
			if(infoLevel>0) {
				printf("Addr:0x%06X Size:%dBytes \n"
						"BANK:0x%02X   INIT:0x%04X EARLY:0x%04X LATER:0x%04X\n",
						convPCtoSNES(xAddr)|0x800000, xSize, addrData.bank, addrData.init, addrData.early, addrData.later);
			}
		}

		nowInsFileNum[lineData.insnum]++;

		// 対象のLevelに挿入するASMの数が一つの場合
		if(insFileNum[lineData.insnum] == 1) {
			// RTLでジャンプするので事前にデクリメントしておく。
			PtrBank[lineData.insnum] = addrData.bank;
			writeWordAddr(InitPtrs,lineData.insnum*2,addrData.init-1);
			writeWordAddr(EarlyPtrs,lineData.insnum*2,addrData.early-1);
			writeWordAddr(LaterPtrs,lineData.insnum*2,addrData.later-1);
		}
		// 対象のLevelに挿入するASMの数が複数ある場合、JSL JMLだけで構成されたコードを自動生成する。
		/* 詳細:
		 *  大体こんなコードを挿入しまーす。
		 *  db "STAR" dw insSize-1,insSize-1^$FFFF
		 *  db "LEVELASM"
		 *  print "INIT ",pc
		 *  JSL !AA_INIT
		 *  JML !BB_INIT
		 *  print "EARLY ",pc
		 *  JSL !AA_EARLY
		 *  JML !BB_EARLY
		 *  print "LATER ",pc
		 *  JSL !AA_LATER
		 *  JML !BB_LATER
		 * 挿入可能な数は32,768bytesからヘッダ16bytesを引き、
		 * 1個当たりのINIT EARLY LATERで消費される12bytesで割った数である2,729個です。
		 */
		else if(insFileNum[lineData.insnum] > 1) {
			int asmInsAddr;
			uchar instBuf[4];
			// 初回挿入の場合 必要な領域を確保
			if(nowInsFileNum[lineData.insnum] == 1) {
				int insSize = insFileNum[lineData.insnum]*12+0x10;
				asmInsAddr = romFile.findFreeSpace(BASE_ADDR,insEndAddr,insSize);
				if(asmInsAddr < 0)	throw string("LevelASMを挿入するだけのrom領域が見つかりませんでした。\n");
				if(infoLevel > 0) {
					printf("領域確保 Addr:%06X Size:%dBytes\n",convPCtoSNES(asmInsAddr)|0x800000,insSize);
				}

				romFile.writeRATSdata(nullptr,insSize-0x08,asmInsAddr);	// RATSタグ書き込み
				romFile.writeData((void*)"LEVELASM",8,asmInsAddr+0x08);	// 識別子書き込み

				// 0x00のままだとFreeSpace認定を受けてしまうので確保した領域をNOPで埋める
				uchar NOP = 0xEA;
				romFile.writeReptData((void*)&NOP,1,insSize-0x10,asmInsAddr+0x10);

				PtrBank[lineData.insnum] = (convPCtoSNES(asmInsAddr+0x0F)>>16)|0x80;
				writeWordAddr(InitPtrs,lineData.insnum*2,convPCtoSNES(asmInsAddr+0x0F));
				writeWordAddr(EarlyPtrs,lineData.insnum*2,convPCtoSNES(asmInsAddr+0x0F+insFileNum[lineData.insnum]*4));
				writeWordAddr(LaterPtrs,lineData.insnum*2,convPCtoSNES(asmInsAddr+0x0F+insFileNum[lineData.insnum]*8));
			}
			else {
				asmInsAddr = (InitPtrs[lineData.insnum*2] | InitPtrs[lineData.insnum*2+1]<<8 | PtrBank[lineData.insnum]<<16) - 0x0F;
				asmInsAddr = convSNEStoPC(asmInsAddr&0x7FFFFF);
			}
			// 最後ならJML XXYYZZ それ以外ならJSL XXYYZZ
			instBuf[0] = nowInsFileNum[lineData.insnum] == insFileNum[lineData.insnum] ? 0x5C : 0x22;

			// Bankは全ラベル共通
			instBuf[3] = addrData.bank;

			// INIT書き込み
			writeWordAddr(instBuf,1,addrData.init);
			romFile.writeData(instBuf,4,asmInsAddr+0x0C+nowInsFileNum[lineData.insnum]*4);

			// EARLY書き込み
			writeWordAddr(instBuf,1,addrData.early);
			romFile.writeData(instBuf,4,asmInsAddr+0x0C+(insFileNum[lineData.insnum]+nowInsFileNum[lineData.insnum])*4);

			// LATER書き込み
			writeWordAddr(instBuf,1,addrData.later);
			romFile.writeData(instBuf,4,asmInsAddr+0x0C+(insFileNum[lineData.insnum]*2+nowInsFileNum[lineData.insnum])*4);

		}
		// 何かの間違いで挿入する予定の無いLevelが対象の場合
		else {
			throw string("挿入予\定の無いLevel番号に挿入処理をしようとしました。\n");	//ダメ文字大好き結婚して
		}
	}
	romFile.writeData(PtrBank,	0x0200,insAddr+0x0200);
	romFile.writeData(InitPtrs,	0x0400,insAddr+0x0400);
	romFile.writeData(EarlyPtrs,0x0400,insAddr+0x0800);
	romFile.writeData(LaterPtrs,0x0400,insAddr+0x0C00);

	return 0;
}

// 挿入済みのLevelASMを削除
int LvASMtool::deleteLevelASM() {
	// なんとなく用意
	int delCount = 0;
	int addr = BASE_ADDR;

	while(true) {
		addr = romFile.findData((void*)"LEVELASM",8,addr);
		if(addr<0)	break;
		int size = romFile.eraseRATSdata(addr-0x08);
		if(size>0) {
			if(delCount++==0) {
				if(infoLevel > 0) printf("挿入済みのLevelASMを削除\n");
			}
			if(infoLevel > 0) printf("Addr:0x%06X Size:%dBytes\n",convPCtoSNES(addr)|0x800000,size);
		}
		addr++;
	}
	if(infoLevel==0 && delCount>0) printf("%d個のLevelASMを削除\n",delCount);
	return delCount;
}

// LevelASM MainCodeを削除
int LvASMtool::deleteLevelASMcode() {
	if(insAddr<0) {
		return -1;
	}
	romFile.eraseRATSdata(insAddr);
	return insAddr;
}

// 改造した本家のコードを戻す
void LvASMtool::rewindHijackCode() {
	// PC:0x0027CC(SNES:0x00A5CC)に挿入するコード
	uchar rewindInitHijack[] = {
			0x20,0x60,0x98,0x20,0x2F,0x92,0x20,0x29,0x9F,0x20,0x1A,0x8E,0xC2,0x30,0x8B,0xA2,
			0x03,0x07,0xA0,0x05,0x09,0xA9,0xEF,0x01,0x54,0x00,0x00,0xAB,0xAE,0x01,0x07,0x8E,
			0x03,0x09,0xE2,0x30
	};

	// PC:0x002442(SNES:0x80A242)に挿入するコード
	uchar rewindMainHijack[] = {
			0xAD,0xD4,0x13,0xF0,0x43
	};

	// PC:0x02DABB(SNES:0x05D8BB)に挿入するコード
	uchar rewindLevelNumHijack[] = {
			0x0A,0x18,0x65,0x0E
	};

	// PC:0x0024EA(SNES:0x00A2EA)に挿入するコード
	// CODE VER 0x0101で追加
	uchar rewindLaterHijack[] = {
			0x68,0x85,0x1D,0x68,0x85,0x1C
	};

	// ver 0x0101以前 INIT MAIN方式
	if(lvASMver<0x0101) {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0101以降 INIT EARLY LATER方式
	else {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);

	}

}

int LvASMtool::deleteEdit1754LevelASM() {

	// PC:0x0027EE(SNES:0x80A5EE)に挿入するコード
	uchar rewindInitHijack[] = {
			0xE2,0x30,0x20,0x9B,0x91
	};

	// PC:0x002442(SNES:0x80A242)に挿入するコード
	uchar rewindMainHijack[] = {
			0xAD,0xD4,0x13,0xF0,0x43
	};

	// PC:0x02DAB9(SNES:0x85D8B9)に挿入するコード
	uchar rewindLevelNumHijack[] = {
			0xA5,0x0E,0x0A
	};

	uchar rewind_FFh[] = {
			0xFF
	};
	// なんとなく用意
	int delCount = 0;
	int addr = 0;

	while(true) {
		addr = romFile.findData((void*)"@LVLASM0",8,addr);
		if(addr<0)	break;
		int size = romFile.eraseRATSdata(addr-0x08);
		if(size>0) {
			if(delCount++==0) {
				if(infoLevel > 0) printf("挿入済みのLevelASMを削除\n");
			}
			if(infoLevel > 0) printf("Addr:0x%06X Size:%dBytes\n",convPCtoSNES(addr)|0x800000,size);
		}
		addr++;
	}
	if(infoLevel==0 && delCount>0) printf("%d個のLevelASMを削除\n",delCount-1);

	romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025EE);
	romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
	romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8B9);
	romFile.writeReptData(rewind_FFh,1,7,0x02DC46);
	return delCount-1;
}

void LvASMtool::writeRomFile() {
	romFile.writeRomFile();
}

// romに挿入されているLevelASMのバージョンを取得
ushort LvASMtool::getRomLvASMver() {
	return lvASMver;
}

// toolのLevelASMのバージョンを取得
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

// uchar型の配列に24bitのデータを書き込むだけ
void LvASMtool::writeLongAddr(uchar* data,int offset,int addr) {
	data[offset] = (uchar)addr;
	data[offset+1] = (uchar)(addr>>8);
	data[offset+2] = (uchar)(addr>>16);
}

// uchar型の配列に16bitのデータを書き込むだけ
void LvASMtool::writeWordAddr(uchar* data,int offset,int addr) {
	data[offset] = (uchar)addr;
	data[offset+1] = (uchar)(addr>>8);
}


// リストファイル1行を解析
LvASMtool::listline LvASMtool::analyzeListLine(string listLine) {

	// 確認用
	string analyzeLine = listLine;

	listline lineData;

	// ;以降を削除
	if(analyzeLine.find(";")!=std::string::npos)	analyzeLine.erase(analyzeLine.find(";"),analyzeLine.length());

	if(analyzeLine.length() < 4) {
		lineData.filename = "";
		lineData.insnum = -1;
	}
	else {
		// 先頭3文字が16進数の数値かチェック

		if(analyzeLine[3] != ' ' && analyzeLine[3] != '\t') {
			throw string("リストファイルの文法が間違ってます:" + listLine);
		}
		std::string lineNum = analyzeLine;
		lineNum.resize(3);

		char* lineNumChar = new char[4];
		lineNumChar = (char*)lineNum.c_str();

		if(!(isxdigit(lineNumChar[0]) && isxdigit(lineNumChar[1]) && isxdigit(lineNumChar[2]))) {
			throw string("リストファイルの文法が間違ってます:" + listLine);
		}

		lineData.insnum = (int)strtol(lineNumChar,nullptr,16);
		if(lineData.insnum >= 0x200)	throw "指定できるLevel番号は000-1FFです。:" + listLine + "\n";

		// ここからファイル名取得
		// 先頭4文字削除
		analyzeLine.erase(0,4);

		if(analyzeLine.find(".asm") == std::string::npos)	throw ".asmファイルを指定してください。:" + listLine + "\n";

		analyzeLine.erase(analyzeLine.find(".asm")+4,analyzeLine.length());
		lineData.filename = analyzeLine;
	}
	return lineData;
}
