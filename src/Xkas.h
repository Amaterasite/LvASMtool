
#include <string>
#include <map>

#include "Rom.h"
#include "windows.h"

#ifndef XKAS_H_
#define XKAS_H_

using namespace std;

class Xkas {
public:
	// Exception�𔭐�������قǂ̎��ł��Ȃ��̂ŗ񋓌^�ŃG���[���`����
	enum EerrType {
		success,				// �G���[�Ȃ�
		unexecuted,				// �A�Z���u�������s
		asmfile_not_found,		// ASM�t�@�C����������Ȃ�������
		first_pass,				// ���A�Z���u���i�K�̎�
		tmpasm_create_failed,	// tmpasm.asm�̍쐬�Ɏ��s
		tmpbin_create_failed,	// tmpasm.bin�̍쐬�Ɏ��s
		tmpbin_open_failed,		// tmpasm.bin���J���Ȃ�������
		tmplog_open_failed,		// xkas.log���J���Ȃ�������
		assemble_error,			// �A�Z���u������ASM�t�@�C���ɃG���[������������
		assemble_size_zero,		// �A�Z���u����̃T�C�Y��0byte
		assemble_size_over,		// �A�Z���u����̃T�C�Y�����E���I�[�o�[
		insert_failed,			// �}���\�ȋ󂫗̈悪������Ȃ�������
	};
	Xkas();
	Xkas(Rom* romData,string asmPath);
	Xkas(Rom* romData,string asmPath,int sizeOffset);
	Xkas(Rom* romData,string asmPath,int sizeOffset,int limitSize);
	~Xkas();

	int allocSpace(Rom* romData,string asmPath,int sizeOffset,int limitSize);

	int insertASM();
	int insertASM(int insertOffset);

	// RATS�^�O���݂�ASM��}������
	int insertRatsASM(Rom* romData,string asmPath);
	int insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset);
	int insertRatsASM(Rom* romData,string asmPath,int insertOffset,int sizeOffset,int limitSize);

	// �}����������̈��ݒ�
	void setInsertRange(int startAddr,int endAddr);

	// ���x�����X�g���蓮�Ŏ擾
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

	// �m�ۂ���rom�e�ʂ̃I�t�Z�b�g�A�w�b�_�E�t�b�^�ǉ��p
	int sizeOffset = 0;

	// ���蓖�Ă�ꂽ�A�h���X
	int allocAddr = -1;

	// ���蓖�Ă�T�C�Y
	int allocSize = 0;

	// ASM�P�̂̃T�C�Y
	int asmSize = 0;

	// ASM�̏���T�C�Y
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
