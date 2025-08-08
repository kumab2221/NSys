# NSys

NSys�́AImGui���g�p����C++�A�v���P�[�V�����J���v���W�F�N�g�ł��B

## �v���W�F�N�g�\��

```
NSys/
������ NSys/                    # ���C���A�v���P�[�V����
��   ������ external/imgui/      # ImGui���C�u�����i�T�u���W���[���j
��   ������ ImGuiWindowManager.cpp
��   ������ ImGuiWindowManager.h
��   ������ main.cpp
������ PluginTest/              # �v���O�C���e�X�g�p�v���W�F�N�g
��   ������ external/imgui/      # ImGui���C�u�����i�T�u���W���[���j
��   ������ dllmain.cpp
��   ������ framework.h
��   ������ README.md
������ CSVNodeEditor/           # �m�[�h�v���O���~���O�^CSV��̓c�[��
��   ������ external/            # �O�����C�u�����i�T�u���W���[���j
��   ��   ������ imgui/          # ImGui���C�u����
��   ��   ������ imnodes/        # ImNodes���C�u����
��   ��   ������ implot/         # ImPlot���C�u����
��   ������ CSVNodeEditor.h     # ���C���N���X
��   ������ CSVNodeEditor.cpp   # ���C������
��   ������ NodeEditor.h        # �m�[�h�G�f�B�^
��   ������ NodeEditor.cpp      # �m�[�h�G�f�B�^����
��   ������ CSVData.h           # CSV�f�[�^����
��   ������ CSVData.cpp         # CSV�f�[�^��������
��   ������ NodeTypes.h         # �m�[�h�^�C�v��`
��   ������ NodeTypes.cpp       # �m�[�h�^�C�v����
��   ������ dllmain.cpp         # DLL�G���g���[�|�C���g
��   ������ framework.h         # ���ʃw�b�_�[
��   ������ README.md           # �v���O�C������
������ README.md
```

## �O�����

- Visual Studio 2019�ȍ~
- Git
- CMake�i�K�v�ɉ����āj

## �v���O�C��

### PluginTest
��{�I�ȃv���O�C���@�\�̃e�X�g�p�v���W�F�N�g�ł��BImGui�R���e�L�X�g�����L���ă��C���A�v���P�[�V�����ƘA�g���܂��B

### CSVNodeEditor
�m�[�h�v���O���~���O�^CSV��̓c�[���ł��B�ȉ��̋@�\��񋟂��܂��F

- **�m�[�h�x�[�X�̃f�[�^�����t���[�\�z**
- **CSV�t�@�C���̓ǂݍ��݁E�ۑ�**
- **�����^�u�ł̕���ҏW**
- **Docking�E�B���h�E�Ή�**
- **�e��f�[�^�����m�[�h**�i�t�B���^�[�A�\�[�g�A�W�v�A�������j

�ڍׂ� [CSVNodeEditor/README.md](CSVNodeEditor/README.md) ���Q�Ƃ��Ă��������B

## �Z�b�g�A�b�v�菇

### 1. ���|�W�g���̃N���[��

```bash
git clone <repository-url>
cd NSys
```

### 2. �T�u���W���[���̏�����

**�d�v**: ���̃v���W�F�N�g��ImGui���C�u�������T�u���W���[���Ƃ��Ďg�p���Ă��܂��B�ȉ��̎菇�Ő��������������Ă��������B

#### ���@1: �������@�i�V�K�N���[�����j

```bash
# ���|�W�g�����N���[������ۂɃT�u���W���[���������ɏ�����
git clone --recursive <repository-url>
cd NSys
```

#### ���@2: �����̃��|�W�g���ŃT�u���W���[����������

```bash
# �T�u���W���[���̏������ƍX�V
git submodule update --init --recursive
```

#### ���@3: �T�u���W���[�������������삵�Ȃ��ꍇ�̑Ώ��@

������L�̕��@�ŃT�u���W���[��������������������Ȃ��ꍇ�i��̃f�B���N�g�����쐬�����ꍇ�j�F

```bash
# 1. ���imgui�f�B���N�g�����폜
Remove-Item -Recurse -Force "NSys/external/imgui" -ErrorAction SilentlyContinue
Remove-Item -Recurse -Force "PluginTest/external/imgui" -ErrorAction SilentlyContinue

# 2. �T�u���W���[�����Ēǉ�
git submodule add -b docking https://github.com/ocornut/imgui.git NSys/external/imgui
git submodule add -b docking https://github.com/ocornut/imgui.git PluginTest/external/imgui

# 3. �T�u���W���[���̏�Ԃ��m�F
git submodule status
```

### 3. �����Z�b�g�A�b�v�i�����j

�v���W�F�N�g�̃Z�b�g�A�b�v������������X�N���v�g��񋟂��Ă��܂��F

#### Windows�p�o�b�`�X�N���v�g
```cmd
setup.bat
```

#### PowerShell�X�N���v�g�i���ڍׂȃ��O�o�́j
```powershell
.\setup.ps1
```

�����̃X�N���v�g�͈ȉ��������I�Ɏ��s���܂��F
- Git�̃C���X�g�[���m�F
- �T�u���W���[���̏�����
- ImGui�t�@�C���̑��݊m�F
- ��肪����ꍇ�̎����C��

### 4. �v���W�F�N�g�̃r���h

1. `NSys.sln` ��Visual Studio�ŊJ��
2. �K�v�Ȉˑ��֌W����������Ă��邱�Ƃ��m�F
3. �r���h�ݒ��I���iDebug/Release�j
4. �r���h�����s

## �T�u���W���[���̊Ǘ�

### �T�u���W���[���̍X�V

```bash
# ���ׂẴT�u���W���[�����ŐV�łɍX�V
git submodule update --remote --recursive

# ����̃T�u���W���[���̂ݍX�V
git submodule update --remote NSys/external/imgui
```

### �T�u���W���[���̏�Ԋm�F

```bash
# �T�u���W���[���̏�Ԃ�\��
git submodule status

# �T�u���W���[���̏ڍ׏���\��
git submodule foreach 'git status'
```

## �g���u���V���[�e�B���O

### �T�u���W���[������̏ꍇ

1. `git submodule status` �����s���ăT�u���W���[���̏�Ԃ��m�F
2. ��̃f�B���N�g�����\�������ꍇ�́A��L�́u���@3�v�����s

### �r���h�G���[����������ꍇ

1. ImGui�t�@�C�����������z�u����Ă��邩�m�F
2. �v���W�F�N�g�̃C���N���[�h�p�X���������ݒ肳��Ă��邩�m�F
3. �K�v�Ȉˑ��֌W���C���X�g�[������Ă��邩�m�F

## ���C�Z���X

���̃v���W�F�N�g�͓K�؂ȃ��C�Z���X�̉��Œ񋟂���Ă��܂��B�ڍׂ� `LICENSE.txt` ���Q�Ƃ��Ă��������B

## �v��

�v���W�F�N�g�ւ̍v�������}���܂��B�v�����N�G�X�g��C�V���[�̕񍐂����C�y�ɂ��肢���܂��B

## �T�|�[�g

��肪���������ꍇ�́A�ȉ��̎菇�őΏ����Ă��������F

1. ����README�̃g���u���V���[�e�B���O�Z�N�V�������m�F
2. �����̃C�V���[������
3. �V�����C�V���[���쐬�i�ڍׂȏ����܂߂Ă��������j