
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
	headerSize = romFile.getHeader() ? 0x0200 : 0x0000;

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

	// PC:0x001912(SNES:0x009712)に挿入するコード
	uchar levelASM_InitLevelHijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// PC:0x0027B7(SNES:0x00A5B7)に挿入するコード
	uchar levelASM_InitHijack0[] = {
			0x80,0x13
	};

	// PC:0x0027CC(SNES:0x00A5CC)に挿入するコード
	uchar levelASM_InitHijack1[] = {
			0x20,0xEC,0xA5
	};

	// PC:0x0027E8(SNES:0x00A5E8)に挿入するコード
	uchar levelASM_InitHijack2[] = {
			0x5C,0xFF,0xFF,0xFF,0x5C,0xFF,0xFF,0xFF}
	;

	// PC:0x002442(SNES:0x80A242)に挿入するコード
	uchar levelASM_EarlyHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x0024EA(SNES:00A2EA)に挿入するコード
	uchar levelASM_LaterHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC]0x0003C1(SNES:0081C1)に挿入するコード
	uchar levelASM_NMIHijack[] = {
			0x5C,0xFF,0xFF,0xFF,0xEA
	};

	// PC:0x02DABB(SNES:0x05D8BB)に挿入するコード
	uchar levelNum_Hijack[] = {
			0x22,0xFF,0xFF,0xFF
	};

	// LevelASM 実行コード
	uchar levelASMmainCode[] = {
		//  __+0___+1___+2___+3___+4___+5___+6___+7___+8___+9___+A___+B___+C___+D___+E___+F
			0x22,0xDB,0xF6,0x80,0x5C,0x90,0x81,0x3F,0x22,0x98,0x81,0x3F,0x2C,0x9B,0x0D,0x30,	// 0x0000
			0x04,0x5C,0x60,0x98,0x80,0x5C,0x0C,0xA6,0x80,0xAE,0x01,0x07,0x8E,0x03,0x09,0xE2,	// 0x0010
			0x30,0x22,0xA0,0x81,0x3F,0x5C,0xF0,0xA5,0x80,0x22,0xA8,0x81,0x3F,0xAD,0xD4,0x13,	// 0x0020
			0xF0,0x04,0x5C,0x5B,0xA2,0x80,0x5C,0x8A,0xA2,0x80,0x68,0x85,0x1D,0x68,0x85,0x1C,	// 0x0030
			0x22,0xB0,0x81,0x3F,0x5C,0x94,0x84,0x80,0xA5,0x44,0x8D,0x30,0x21,0xAD,0x00,0x01,	// 0x0040
			0xC9,0x03,0x90,0x1C,0xC9,0x0C,0x90,0x08,0xC9,0x10,0x90,0x10,0xC9,0x15,0xB0,0x0C,	// 0x0050
			0xD4,0x00,0x22,0xB8,0x81,0x3F,0x68,0x85,0x00,0x68,0x85,0x01,0x5C,0xC6,0x81,0x80,	// 0x0060
			0x9C,0x0B,0x01,0xA9,0x02,0x8D,0x0C,0x01,0x80,0xF2,0x8D,0x0B,0x01,0x0A,0x18,0x65,	// 0x0070
			0x0E,0x6B,0xC2,0x10,0x86,0x45,0xAE,0x0B,0x01,0xBF,0xFF,0xFF,0xFF,0xF0,0x1C,0xFA,	// 0x0080
			0x4B,0xDA,0x48,0x48,0xAB,0xC2,0x20,0xAD,0x0B,0x01,0x0A,0xAA,0xBF,0x00,0x84,0x3F,	// 0x0090
			0x18,0x65,0x45,0xAA,0xBD,0x00,0x00,0x48,0xE2,0x30,0x6B,0xE2,0x30,0x60,0xC2,0x21,	// 0x00A0
			0x29,0xFF,0x00,0x69,0xFF,0xFF,0x48,0xE2,0x20,0x60,0xDA,0x22,0xFF,0xFF,0xFF,0xFA,	// 0x00B0
			0x20,0x02,0x81,0x60,0xF4,0xBF,0xBF,0xAB,0xC2,0x31,0x8A,0x69,0x00,0xC0,0xAA,0xBD,	// 0x00C0
			0x00,0x00,0x48,0xE2,0x30,0x6B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	// 0x00D0
	};

	int insSNESaddr = convPCtoSNES(insAddr) | 0x800000;

	writeLongAddr(levelASMmainCode,0x0005,insSNESaddr+0x0190);	// JML InitLevel
	writeLongAddr(levelASMmainCode,0x0009,insSNESaddr+0x0198);	// JSL InitEary
	writeLongAddr(levelASMmainCode,0x0022,insSNESaddr+0x01A0);	// JSL InitLater
	writeLongAddr(levelASMmainCode,0x002A,insSNESaddr+0x01A8);	// JSL MainEarly
	writeLongAddr(levelASMmainCode,0x0041,insSNESaddr+0x01B0);	// JSL MainLater
	writeLongAddr(levelASMmainCode,0x0063,insSNESaddr+0x01B8);	// JSL NMILevel

	writeLongAddr(levelASMmainCode,0x008A,insSNESaddr+0x01F0);	// LDA.l OffsetBank,x
	writeLongAddr(levelASMmainCode,0x009D,insSNESaddr+0x0400);	// LDA.l OffsetAddr,x
	writeLongAddr(levelASMmainCode,0x00BC,insSNESaddr+0x0144);	// JSL AlwaysASM
	writeWordAddr(levelASMmainCode,0x00C1,insSNESaddr+0x0102);	// JSR ExecuteLvASM
	writeWordAddr(levelASMmainCode,0x00B4,insSNESaddr+0x018F);	// ADC.w !CodeOffset+$018F


	writeLongAddr(levelASM_InitLevelHijack,0x01,insSNESaddr+0x80);	// JSL InitLevel

	writeLongAddr(levelASM_InitHijack2,0x05,insSNESaddr+0x0088);	// JML CallInitEarly
	writeLongAddr(levelASM_InitHijack2,0x01,insSNESaddr+0x0099);	// JML CallInitLater

	writeLongAddr(levelASM_EarlyHijack,0x01,insSNESaddr+0x00A9);	// JML CallMainEarly

	writeLongAddr(levelASM_LaterHijack,0x01,insSNESaddr+0x00BA);	// JML CallMainLater
	writeLongAddr(levelASM_LaterHijack,0x05,insSNESaddr+0x012E);	// JML CallLvASM_API

	writeLongAddr(levelASM_NMIHijack,0x01,insSNESaddr+0x00C8);		// JML CallNMILevel

	writeLongAddr(levelNum_Hijack,0x01,insSNESaddr+0x00FA);			// JSL LevelNum

	// 本家コード書き換え
	romFile.writeData(levelASM_InitLevelHijack,sizeof(levelASM_InitLevelHijack),0x001712);
	romFile.writeData(levelASM_InitHijack0,sizeof(levelASM_InitHijack0),0x0025B7);
	romFile.writeData(levelASM_InitHijack1,sizeof(levelASM_InitHijack1),0x0025CC);
	romFile.writeData(levelASM_InitHijack2,sizeof(levelASM_InitHijack2),0x0025E8);
	romFile.writeData(levelASM_EarlyHijack,sizeof(levelASM_EarlyHijack),0x002242);
	romFile.writeData(levelASM_LaterHijack,sizeof(levelASM_LaterHijack),0x0022EA);
	romFile.writeData(levelASM_NMIHijack,sizeof(levelASM_NMIHijack),0x0001C1);
	romFile.writeData(levelNum_Hijack,sizeof(levelNum_Hijack),0x02D8BB);

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

	uchar LevelPtrAddr[0x0402];
	uchar LevelPtrBank[0x0201];

	for(int i=0;i<0x402;i+=2) {
		writeWordAddr(LevelPtrAddr,i,0x0000);
	}
	for(int i=0;i<0x0201;i++) {
		LevelPtrBank[i] = 0;
	}

	// Level毎の挿入ファイル数
	short insFileNum[0x201];
	// Level 初回挿入フラグ
	bool firstIns[0x201];

	// 全てのLevelの登録Offset数
	int allInsOffsetNum[LABEL_NUM];

	// Level毎の登録Offset数
	short** insOffsetNum = new short* [0x201];
	short** nowInsOffsetNum = new short* [0x201];

	for(int i=0;i<0x201;i++) {
		insOffsetNum[i] = new short[LABEL_NUM];
		nowInsOffsetNum[i] = new short[LABEL_NUM];

		insFileNum[i] = 0;
		firstIns[i] = true;
		for(int j=0;j<LABEL_NUM;j++) {
			insOffsetNum[i][j] = 0;
			nowInsOffsetNum[i][j] = 0;
		}
	}
	for(int i=0;i<LABEL_NUM;i++) {
		allInsOffsetNum[i] = 0;
	}

	string lineBuf;
	ifstream listFile;

	listline lineData;
	addr addrData;
	map<string,addr> addrList;

	printf("listを確認\n");
	// 1st pass Level毎のoffset数を確認
	listFile.open(listName.c_str());
	if(!listFile) throw string("リストファイルを開けませんでした。\n");
	while(!listFile.eof()) {
		getline(listFile,lineBuf);

		// 4文字以下は無視
		if(lineBuf.length()<4)	continue;
		lineData = analyzeListLine(lineBuf);

		if(lineData.insnum<0 || lineData.filename=="")	continue;

		// 一つのLevelに挿入可能なasmの数は256個とする。
		if(++insFileNum[lineData.insnum] > 256) {
			throw string(
					"一つのLevelに挿入するasmの数が256を超えました。\n"
					"　 　 　 　 こんな　つーるに　まし゛に\n"
					"　 　 　 　 なっちゃって　と゛うするの\n"
					"　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 　 完");
		}

		// 指定されたASMファイルが既に挿入済みか確認
		map<string,addr>::const_iterator addrIndex = addrList.find(lineData.filename);
		if(addrIndex != addrList.end()) {
			// 挿入済みならデータを使い回し
			addrData = addrIndex->second;
		} else {
			// 初回挿入
			Xkas Xkaser;
			Xkaser.setInsertRange(BASE_ADDR,0);
			Xkaser.allocSpace(&romFile,".\\LevelASM\\"+lineData.filename,18,0xFFF0);
			Xkaser.getLabelList("xkas.log");

			for(int i=0;i<LABEL_NUM;i++) {
				addrData.labelData[i] = Xkaser.getOffset(LABEL_NAME[i]);

				// ラベルの別名を調べる INIT(INIT_LATE) / MAIN(MAIN_EARLY)用
				if(addrData.labelData[i] < 0) {
					addrData.labelData[i] = Xkaser.getOffset(LABEL_ALIAS[i]);
				}
			}
			addrList[lineData.filename] = addrData;
		}

		for(int i=0;i<LABEL_NUM;i++) {
			if(addrData.labelData[i] >= 0) {
				insOffsetNum[lineData.insnum][i]++;

				allInsOffsetNum[i]++;
			}
		}
	}
	listFile.close();

	addrList.clear();

	// 自動生成LevelASM中間コードの基礎サイズを算出
	int initTableSize = 0x11;	// RATSタグ(8bytes) + LevelASM識別子(8bytes) + RTL(1bytes)
	if(infoLevel>2)	printf("\nOffset Entries:\n");
	for(int i=0;i<LABEL_NUM;i++) {
		if(infoLevel>2)	printf("    %s :%4d\n",OUTPUT_LABEL_NAME[i],allInsOffsetNum[i]);

		if(allInsOffsetNum[i]<1) continue;
		initTableSize +=2 ;		// offset1種あたり2bytes追加
	}
	if(infoLevel>2) printf("LevelASM HeaderSize:%d Bytes (0x%X)\n",initTableSize-1,initTableSize-1);

	// LevelASM合計サイズ
	int totalSize = 0;

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
			if(lineData.insnum>=0 && lineData.insnum<0x200) {
				printf("\nLevel %03X inserting:.\\LevelASM\\%s\n",lineData.insnum,lineData.filename.c_str());
			} else if(lineData.insnum==0x200) {
				printf("\nLevel ALL inserting:.\\LevelASM\\%s\n",lineData.filename.c_str());
			} else {
				throw string("不明なLevel番号にASMを挿入しようとしました。\n");
			}

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
			int xAddr = Xkaser.insertRatsASM(&romFile,".\\LevelASM\\"+lineData.filename,initTableSize-0x09,0,0xFFF0);
			int xSize = Xkaser.getASMsize()+initTableSize-1;
			if(Xkaser.isError()) {
				throw Xkaser.getSimpleErrMes();
			}

			// LevelASMの識別子を書き込む
			romFile.writeData((void*)"LvASM_88",8,xAddr+0x08);

			// 各ラベル位置を取得 見つからない物はRTLのアドレスを割り当て
			if((addrData.RTLaddr =  Xkaser.getRTLaddr()) < 0) {
				throw string("コードにはRTLが必要です。\n");
			}
			addrData.RTLaddr |= 0x800000;

			uchar* offsetData = new uchar[initTableSize-0x11];
			addrData.base = convPCtoSNES(xAddr+0x10)|0x800000;
			bool worthless = true;

			// offsetList用
			int addrOfs = 0;
			for(int i=0;i<LABEL_NUM;i++) {
				// offset挿入数が1以上の物のみを対象とする
				if(allInsOffsetNum[i] > 0) {
					addrData.labelData[i] = Xkaser.getOffset(LABEL_NAME[i]);

					// ラベルの別名を調べる INIT(INIT_LATE) / MAIN(MAIN_EARLY)用
					if(addrData.labelData[i] < 0) {
						addrData.labelData[i] = Xkaser.getOffset(LABEL_ALIAS[i]);
					}

					// offset未定義の場合RTLのアドレスへ飛ばす
					if(addrData.labelData[i] < 0) {
						writeWordAddr(offsetData,addrOfs,addrData.RTLaddr-1);
					} else {
						writeWordAddr(offsetData,addrOfs,addrData.labelData[i]-1);
						worthless = false;
					}
					addrOfs += 2;
				}
			}
			// 全てのoffsetが未定義の場合警告を表示する
			if(worthless) {
				printf("Warning!: 全てのラベルが未定義です。\n          これは一切呼び出されない無意味なコードになります！\n");
			}

			// LevelASMのoffsetリストを書き込む
			romFile.writeData((void*)offsetData,initTableSize-0x11,xAddr+0x10);

			delete[] offsetData;
			addrList[lineData.filename] = addrData;

			// 合計サイズ加算
			totalSize += xSize;

			// infoLevelが1以上の場合 LevelASMの挿入情報を表示
			if(infoLevel>0) {
				printf("Addr:0x%06X (SNES:0x%06X) Size:%d Bytes (0x%X)\n",xAddr+headerSize,convPCtoSNES(xAddr)|0x800000,xSize,xSize);

				for(int i=0;i<LABEL_NUM;i++) {
					if(addrData.labelData[i]<0) {
						// infoLevel2 未挿入offsetの情報も表示
						if(infoLevel<2)	continue;
						if(allInsOffsetNum[i]>0) {
							printf("    %s : 0x%06X (SNES:0x%06X) (none) \n",OUTPUT_LABEL_NAME[i],convSNEStoPC(addrData.RTLaddr)+headerSize,addrData.RTLaddr);
						}
					} else {
						printf("    %s : 0x%06X (SNES:0x%06X) \n",OUTPUT_LABEL_NAME[i],convSNEStoPC(addrData.labelData[i])+headerSize,addrData.labelData[i]);
					}
				}
				printf("\n");
			}
		}

		// 対象のLevelに挿入するASMの数が一つの場合
		if(insFileNum[lineData.insnum] == 1) {
			LevelPtrBank[lineData.insnum] = addrData.base>>16;
			writeWordAddr(LevelPtrAddr,lineData.insnum*2,addrData.base);

		}
		// 対象のLevelに挿入するASMの数が複数ある場合、JSL JMLだけで構成されたコードを自動生成する。
		else if(insFileNum[lineData.insnum] > 1) {
			int asmInsAddr;
			uchar instBuf[4];
			// 初回挿入の場合 必要な領域を確保
			if(firstIns[lineData.insnum]) {
				firstIns[lineData.insnum] = false;

				// コードに必要なサイズ計算
				int insSize = 0;
				for(int i=0;i<LABEL_NUM;i++) {
					insSize += insOffsetNum[lineData.insnum][i]*4;
				}
				if(insSize == 0) {
					printf("Warning!: 無意味な自動生成コードが作成されます！\n");
				}
				insSize += initTableSize;		// RATSタグ / LEVELASM識別子 / Offsetリスト / 未定義用RTL分の容量

				asmInsAddr = romFile.findFreeSpace(BASE_ADDR,insEndAddr,insSize);
				if(asmInsAddr < 0)	throw string("LevelASMを挿入できるだけのrom領域が見つかりませんでした。\n");
				if(infoLevel > 0) {
					printf("Intermediate code created.\nAddr:0x%06X (SNES:0x%06X) Size:%d Bytes (0x%X)\n",asmInsAddr+headerSize,convPCtoSNES(asmInsAddr)|0x800000,insSize,insSize);
				}

				// 合計サイズ加算
				totalSize += insSize;

				romFile.writeRATSdata(nullptr,insSize-0x08,asmInsAddr);	// RATSタグ書き込み
				romFile.writeData((void*)"LvASM_88",8,asmInsAddr+0x08);	// 識別子書き込み

				// 0x00のままだとFreeSpace認定を受けてしまうので確保した領域をRTLで埋める
				uchar RTL = 0x6B;
				romFile.writeReptData((void*)&RTL,1,insSize-initTableSize,asmInsAddr+initTableSize);

				// Offset毎の開始アドレスを書き込む
				int writeOffset = initTableSize-1;

				// offsetList用
				int addrOfs = 0;
				uchar *offsetAddr = new uchar[initTableSize-0x11];
				for(int i=0;i<LABEL_NUM;i++) {
					if(allInsOffsetNum[i] > 0) {
						int addr = insOffsetNum[lineData.insnum][i] == 0 ? asmInsAddr+insSize-1 : asmInsAddr+writeOffset;

						if(infoLevel>1) {
							printf("    %s : %3d Addr:0x%06X (SNES 0x%06X)\n",OUTPUT_LABEL_NAME[i],(int)insOffsetNum[lineData.insnum][i],addr+headerSize,convPCtoSNES(addr));
						}
						writeWordAddr(offsetAddr,addrOfs,convPCtoSNES(addr-1));
						writeOffset += insOffsetNum[lineData.insnum][i]*4;
						addrOfs += 2;
					}
				}
				romFile.writeData((void*)offsetAddr,initTableSize-0x11,asmInsAddr+0x10);

				LevelPtrBank[lineData.insnum] = (convPCtoSNES(asmInsAddr+0x10)>>16)|0x80;
				writeWordAddr(LevelPtrAddr,lineData.insnum*2,convPCtoSNES(asmInsAddr+0x10));

				delete[] offsetAddr;
			} else {
				asmInsAddr = (LevelPtrAddr[lineData.insnum*2] | LevelPtrAddr[lineData.insnum*2+1]<<8 | LevelPtrBank[lineData.insnum]<<16) - 0x10;
				asmInsAddr = convSNEStoPC(asmInsAddr&0x7FFFFF);
			}

			int writeOffset = initTableSize-1;

			for(int i=0;i<LABEL_NUM;i++) {
				if(addrData.labelData[i] >= 0) {
					// 最後ならJML XXYYZZ それ以外ならJSL XXYYZZ
					instBuf[0] = (nowInsOffsetNum[lineData.insnum][i]+1) == insOffsetNum[lineData.insnum][i] ? 0x5C : 0x22;

					// コール / ジャンプ先書き込み
					writeLongAddr(instBuf,1,addrData.labelData[i]|0x800000);
					romFile.writeData(instBuf,4,asmInsAddr+writeOffset+nowInsOffsetNum[lineData.insnum][i]*4);

					nowInsOffsetNum[lineData.insnum][i]++;
				}
				writeOffset += insOffsetNum[lineData.insnum][i]*4;
			}
		}
		// 何かの間違いで挿入する予定の無いLevelが対象の場合
		else {
			throw string("挿入予\定の無いLevel番号に挿入処理をしようとしました。\n");	//ダメ文字大好き結婚して
		}
	}
	if(infoLevel>0)	printf("LevelASM TotalSize:%d Bytes (0x%X)\n\n",totalSize,totalSize);

	romFile.writeData(LevelPtrBank,0x0200,insAddr+0x01F0);				// Bankデータ
	romFile.writeData((void*)"\0\0ヤヤなるは正義",0x10,insAddr+0x03F0);		// LevelASM未実行用Bank / LevelASM挿入状態 / Padding
	romFile.writeData(LevelPtrAddr,0x0400,insAddr+0x0400);				// Addrデータ

	// 未定義
	uchar usingUndefLabel = 0x00;
	for(int i=6;i<LABEL_NUM;i++) {
		if(allInsOffsetNum[i]>0) {
			usingUndefLabel |= (1<<(i-6));
		}
	}
	romFile.writeData(&usingUndefLabel,1,insAddr+0x03F1);

	uchar callLvASM[] = {
			0x8B,0xA2,0x00,0x20,0xFF,0xFF,0xAB,0x6B
	};
	// Xの値
	uchar loadX = 0x00;
	// ラベル毎のLevelASM実行コードを生成
	for(int i=0;i<LABEL_NUM;i++) {
		// ラベル登録数0の場合先頭をRTLに置き換え
		callLvASM[0] = allInsOffsetNum[i] == 0 ? 0x6B : 0x8B;

		// LDX #$XX
		callLvASM[2] = loadX;

		// 現在のラベルがどこでもASMに登録されている場合コードの流れを変更
		ushort CallAddr = convPCtoSNES(insOffsetNum[0x200][i] == 0 ? insAddr+0x0102 : insAddr+0x013A);
		writeWordAddr(callLvASM,4,CallAddr);

		// 実行コード書き込み
		romFile.writeData(callLvASM,0x08,insAddr+0x190+i*8);
		if(allInsOffsetNum[i] > 0) {
			loadX += 2;
		}
	}

	uchar PEAoperand[] = {
			LevelPtrBank[0x0200],
			LevelPtrBank[0x0200],
	};
	uchar ADCoperand[] = {
			LevelPtrAddr[0x0400],
			LevelPtrAddr[0x0401],
	};

	romFile.writeData(PEAoperand,0x02,insAddr+0x145);	// PEA $XXXX
	romFile.writeData(ADCoperand,0x02,insAddr+0x14C);	// ADC #$XXXX
	return 0;
}

// 挿入済みのLevelASMを削除
int LvASMtool::deleteLevelASM() {
	// なんとなく用意
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

	// PC:0x001912(SNES:0x009712)に挿入するコード
	// CODE VER 0x0110で追加
	uchar rewindLevelHijack[] = {
			0x22,0xDB,0xF6,0x80
	};

	// PC]0x0003C1(SNES:0081C1)に挿入するコード
	// CODE VER 0x0110で追加
	uchar rewindNMIHijack[] = {
			0xA5,0x44,0x8D,0x30,0x21
	};

	// ver 0x0101以前 INIT MAIN方式
	if(lvASMver<0x0101) {
		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0110以前 INIT EARLY LATER方式
	else if(lvASMver<0x0110) {
		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);

		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0112以前
	else if(lvASMver<0x0112){
		romFile.writeData(rewindLevelHijack,sizeof(rewindLevelHijack),0x001712);
		romFile.writeData(rewindNMIHijack,sizeof(rewindNMIHijack),0x0001C1);

		romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);

		romFile.writeData(rewindInitHijack,sizeof(rewindInitHijack),0x0025CC);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
		romFile.writeData(rewindLevelNumHijack,sizeof(rewindLevelNumHijack),0x02D8BB);
	}
	// ver 0x0112以降
	else {

		// PC:0x0027B7(SNES:0x00A5B7)に挿入するコード
		uchar rewind_0112_InitHijack0[] = {
				0x80,0x16
		};

		// PC:0x0027CC(SNES:0x00A5CC)に挿入するコード
		uchar rewind_0112_InitHijack1[] = {
				0x20,0x60,0x98
		};

		// PC:0x0027E8(SNES:0x00A5E8)に挿入するコード
		uchar rewind_0112_InitHijack2[] = {
				0xAE,0x01,0x07,0x8E,0x03,0x09,0xE2,0x30
		};

		romFile.writeData(rewindLevelHijack,sizeof(rewindLevelHijack),0x001712);
		romFile.writeData(rewindNMIHijack,sizeof(rewindNMIHijack),0x0001C1);

		if(lvASMver<0x120) {
			romFile.writeData(rewindLaterHijack,sizeof(rewindLaterHijack),0x0022EA);
		}
		// ver 0x0120以降
		else {
			// PC:0x0024EA(SNES:0x00A2EA)に挿入するコード
			uchar rewind_0120_LaterHijack[] = {
					0x68,0x85,0x1D,0x68,0x85,0x1C,0x4C,0x94,0x84
			};
			romFile.writeData(rewind_0120_LaterHijack,sizeof(rewind_0120_LaterHijack),0x0022EA);
		}

		romFile.writeData(rewind_0112_InitHijack0,sizeof(rewind_0112_InitHijack0),0x0025B7);
		romFile.writeData(rewind_0112_InitHijack1,sizeof(rewind_0112_InitHijack1),0x0025CC);
		romFile.writeData(rewind_0112_InitHijack2,sizeof(rewind_0112_InitHijack2),0x0025E8);
		romFile.writeData(rewindMainHijack,sizeof(rewindMainHijack),0x002242);
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
	addr &= 0x7FFFFF;
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

		char *lineNumChar = new char[4];
		lineNumChar = (char*)lineNum.c_str();

		// ALLで0x0200 どこでもASM用
		if(toupper(lineNumChar[0]=='A') && toupper(lineNumChar[1]=='L') && toupper(lineNumChar[2]=='L')) {
			lineData.insnum = 0x0200;
		}
		else {
			if(!(isxdigit(lineNumChar[0]) && isxdigit(lineNumChar[1]) && isxdigit(lineNumChar[2]))) {
				throw string("リストファイルの文法が間違ってます:" + listLine);
			}
			lineData.insnum = (int)strtol(lineNumChar,nullptr,16);
			if(lineData.insnum >= 0x200)	throw "指定できるLevel番号は000-1FFです。:" + listLine + "\n";
		}
		delete lineNumChar;

		// ここからファイル名取得
		// 先頭4文字削除
		analyzeLine.erase(0,4);

		if(analyzeLine.find(".asm") == std::string::npos)	throw ".asmファイルを指定してください。:" + listLine + "\n";

		analyzeLine.erase(analyzeLine.find(".asm")+4,analyzeLine.length());
		lineData.filename = analyzeLine;
	}
	return lineData;
}

bool LvASMtool::checkUsingUndefLabel() {
	if(insAddr<0) return false;
	uchar usingUndefLabel;
	romFile.readData(&usingUndefLabel,1,insAddr+0x03F1);
	return (usingUndefLabel&0x3F) != 0x00;
}

void LvASMtool::showUnsingUndefLabel() {
	uchar usingUndefLabel;
	romFile.readData(&usingUndefLabel,1,insAddr+0x03F1);
	usingUndefLabel &= 0x3F;
	for(int i=6;i<LABEL_NUM;i++) {
		if((usingUndefLabel&0x01)==0x01) {
			printf("%s\n",OUTPUT_LABEL_NAME[i]);
		}
		usingUndefLabel = usingUndefLabel>>1;
	}
}
