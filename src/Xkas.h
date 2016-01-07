
#include <string>
#include <map>

#include "Rom.h"
#include "windows.h"

#ifndef XKAS_H_
#define XKAS_H_

using namespace std;

class Xkas {
public:
	// Exceptionを発生させるほどの事でもないので列挙型でエラーを定義する
	enum EerrType {
		success,				// エラーなし
		unexecuted,				// アセンブル未実行
		asmfile_not_found,		// ASMファイルが見つからなかった時
		first_pass,				// 仮アセンブル段階の時
		tmpasm_create_failed,	// tmpasm.asmの作成に失敗
		tmpbin_create_failed,	// tmpasm.binの作成に失敗
		tmpbin_open_failed,		// tmpasm.binを開けなかった時
		tmplog_open_failed,		// xkas.logを開けなかった時
		assemble_error,			// アセンブルしたASMファイルにエラーが見つかった時
		assemble_size_zero,		// アセンブル後のサイズが0byte
		assemble_size_over,		// アセンブル後のサイズが限界をオーバー
		insert_failed,			// 挿入可能な空き領域が見つからなかった時
	};
	Xkas();
	Xkas(Rom* romData,string asmPath);
	Xkas(Rom* romData,string asmPath,int sizeOffset);
	Xkas(Rom* romData,string asmPath,int sizeOffset,int limitSize);
	~Xkas();

	int allocSpace(Rom* romData,string asmPath,int sizeOffset,int limitSize);

	int insertASM();
	int insertASM(int insertOffset);

	// RATSタグ込みでASMを挿入する
	int insertRatsASM(Rom* romData,string asmPath);
	int insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset);
	int insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset,int limitSize);

	// 挿入を許可する領域を設定
	void setInsertRange(int startAddr,int endAddr);

	// ラベルリストを手動で取得
	int getLabelList(string fileName);

	int getOffset(string labelName);

	int getRTLaddr();
	int getRTSaddr();

	int getAllocAddr();

	int getASMsize();

	EerrType getErrType();
	string getSimpleErrMes();
	string getSimpleErrMes(bool Japanese);
	bool isError();

private:
	map<string,int> labelData;

	// 確保するrom容量のオフセット、ヘッダ・フッタ追加用
	int sizeOffset = 0;

	// 割り当てられたアドレス
	int allocAddr = -1;

	// 割り当てるサイズ
	int allocSize = 0;

	// ASM単体のサイズ
	int asmSize = 0;

	// ASMの上限サイズ
	int limitSize = 0x8000;

	int startAddr = 0;
	int endAddr = 0;

	int RTLaddr = -1;
	int RTSaddr = -1;

	string asmPath = "";
	Rom* romData;

	EerrType xkasErr = EerrType::unexecuted;

	ifstream ASM;

	int convPCtoSNES(int addr);
	int convSNEStoPC(int addr);
};

#endif /* XKAS_H_ */
