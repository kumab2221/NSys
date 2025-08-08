# CSVNodeEditor �v���O�C��

NSys�v���O�C���A�[�L�e�N�`�����g�p�����m�[�h�v���O���~���O�^CSV��̓c�[���ł��B

## �T�v

CSVNodeEditor�́AImGui�AImNodes�AImPlot���g�p���č\�z���ꂽ�m�[�h�x�[�X��CSV�f�[�^��̓v���O�C���ł��B�����I�ȃh���b�O&�h���b�v�C���^�[�t�F�[�X��CSV�f�[�^�̏����t���[���\�z�ł��܂��B

## �@�\

### ��{�@�\
- **CSV�t�@�C���̓ǂݍ��݁E�ۑ�**
- **�m�[�h�x�[�X�̃f�[�^�����t���[�\�z**
- **�����^�u�ł̕���ҏW**
- **Docking�E�B���h�E�Ή�**

### ���p�\�ȃm�[�h

#### �f�[�^����
- **CSV�ǂݍ��݃m�[�h**: CSV�t�@�C����ǂݍ��݁A�f�[�^���o��

#### �f�[�^����
- **�t�B���^�[�m�[�h**: �����Ɋ�Â��ăf�[�^���t�B���^�����O
- **�\�[�g�m�[�h**: �w�肵����Ńf�[�^���\�[�g
- **�W�v�m�[�h**: �O���[�v���ƏW�v�֐���K�p
- **�����m�[�h**: �����̃f�[�^�Z�b�g������

#### �f�[�^�o��
- **CSV�o�̓m�[�h**: �������ʂ�CSV�t�@�C���Ƃ��ĕۑ�

### UI�\��

- **���j���[�o�[**: �t�@�C������A�\���ݒ�A���s����
- **�^�u�o�[**: �����̏����t���[���Ǘ�
- **�m�[�h�p���b�g**: ���p�\�ȃm�[�h�̈ꗗ
- **�v���p�e�B�p�l��**: �I�����ꂽ�m�[�h�̐ݒ�
- **�f�[�^�v���r���[**: CSV�f�[�^�̓��e�\��
- **���O�p�l��**: �����󋵂̕\��

## �v���W�F�N�g�\��

```
CSVNodeEditor/
������ external/
��   ������ imgui/          # ImGui���C�u�����i�T�u���W���[���j
��   ������ imnodes/        # ImNodes���C�u�����i�T�u���W���[���j
��   ������ implot/         # ImPlot���C�u�����i�T�u���W���[���j
������ CSVNodeEditor.h     # ���C���N���X�w�b�_�[
������ CSVNodeEditor.cpp   # ���C���N���X����
������ NodeEditor.h        # �m�[�h�G�f�B�^�N���X
������ NodeEditor.cpp      # �m�[�h�G�f�B�^����
������ CSVData.h           # CSV�f�[�^�����N���X
������ CSVData.cpp         # CSV�f�[�^��������
������ NodeTypes.h         # �m�[�h�^�C�v��`
������ NodeTypes.cpp       # �m�[�h�^�C�v����
������ dllmain.cpp         # DLL�G���g���[�|�C���g
������ framework.h         # ���ʃw�b�_�[
������ README.md           # ���̃t�@�C��
```

## �r���h���@

### �O�����
- Visual Studio 2019�ȍ~
- Windows 10/11
- Git�i�T�u���W���[���Ǘ��p�j

### �r���h�菇

1. **�T�u���W���[���̏�����**
   ```bash
   git submodule update --init --recursive
   ```

2. **Visual Studio�Ńv���W�F�N�g���J��**
   - `CSVNodeEditor.vcxproj`���J��
   - �ݒ��Release x64�ɕύX

3. **�r���h���s**
   - �r���h �� �\�����[�V�����̃r���h

4. **�v���O�C���̔z�u**
   - �������ꂽDLL��`x64/Release/Plugins/CSVNodeEditor/`�t�H���_�ɔz�u

## �g�p���@

### ��{�I�ȃ��[�N�t���[

1. **CSV�t�@�C���̓ǂݍ���**
   - �m�[�h�p���b�g����uCSV�ǂݍ��݁v�m�[�h��ǉ�
   - �t�@�C���p�X���w�肵�ăf�[�^��ǂݍ���

2. **�f�[�^�����̒ǉ�**
   - �K�v�ȏ����m�[�h�i�t�B���^�[�A�\�[�g���j��ǉ�
   - �m�[�h�Ԃ�ڑ����ď����t���[���\�z

3. **���ʂ̏o��**
   - �uCSV�o�́v�m�[�h��ǉ�
   - �o�̓p�X���w�肵�Č��ʂ�ۑ�

### �m�[�h�̑�����@

- **�m�[�h�̒ǉ�**: �E�N���b�N���j���[�܂��̓m�[�h�p���b�g����
- **�m�[�h�̈ړ�**: �h���b�O&�h���b�v
- **�m�[�h�̐ڑ�**: ����/�o�̓s�����h���b�O���Đڑ�
- **�m�[�h�̍폜**: �m�[�h��I������Delete�L�[

### �^�u�Ǘ�

- **�V�K�^�u**: �^�u�o�[�́u+�v�{�^���ō쐬
- **�^�u�؂�ւ�**: �^�u���N���b�N���Đ؂�ւ�
- **�^�u����**: �^�u�́u�~�v�{�^���ŕ���

## �Z�p�d�l

### �g�p���C�u����
- **ImGui**: Immediate Mode GUI
- **ImNodes**: �m�[�h�G�f�B�^�@�\
- **ImPlot**: �f�[�^����

### �Ή��t�@�C���`��
- **����**: CSV�i�J���}��؂�j
- **�o��**: CSV�i�J���}��؂�j

### �p�t�H�[�}���X
- 10���s���x��CSV�f�[�^�������\
- �����������I�ȃf�[�^�Ǘ�

## ����̊g���\��

- **�ǉ��m�[�h�^�C�v**
  - �f�[�^�ϊ��m�[�h
  - ���v���̓m�[�h
  - �����m�[�h

- **�@�\�g��**
  - �v���W�F�N�g�t�@�C���̕ۑ�/�ǂݍ���
  - Undo/Redo�@�\
  - �o�b�`�����@�\

- **�t�@�C���`���Ή�**
  - Excel�t�@�C���Ή�
  - TSV�t�@�C���Ή�
  - JSON�t�@�C���Ή�

## �g���u���V���[�e�B���O

### �悭������

1. **�v���O�C�����ǂݍ��܂�Ȃ�**
   - DLL���������t�H���_�ɔz�u����Ă��邩�m�F
   - �ˑ����C�u�����������������N����Ă��邩�m�F

2. **CSV�t�@�C�����ǂݍ��߂Ȃ�**
   - �t�@�C���p�X�����������m�F
   - �t�@�C�������̃A�v���P�[�V�����ŊJ����Ă��Ȃ����m�F

3. **�m�[�h���\������Ȃ�**
   - ImNodes���C�u����������������������Ă��邩�m�F

## ���C�Z���X

���̃v���O�C���͓K�؂ȃ��C�Z���X�̉��Œ񋟂���Ă��܂��B�g�p����O�����C�u�����̃��C�Z���X�����炵�Ă��������B

## �v��

�v���W�F�N�g�ւ̍v�������}���܂��B�v�����N�G�X�g��C�V���[�̕񍐂����C�y�ɂ��肢���܂��B

## �Q�l����

- [ImGui Documentation](https://github.com/ocornut/imgui)
- [ImNodes Documentation](https://github.com/Nelarius/imnodes)
- [ImPlot Documentation](https://github.com/epezent/implot)
- [NSys ���C���v���W�F�N�g](../README.md)
