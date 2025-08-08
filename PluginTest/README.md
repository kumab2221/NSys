# PluginTest

NSys�v���O�C���A�[�L�e�N�`���̃e�X�g�p�v���W�F�N�g�ł��B

## �v���W�F�N�g�\��

```
PluginTest/
������ external/
��   ������ imgui/          # ImGui���C�u�����i�T�u���W���[���j
������ dllmain.cpp         # �v���O�C���̃��C���G���g���[�|�C���g
������ framework.h         # ���ʃw�b�_�[�t�@�C��
������ PluginTest.vcxproj  # Visual Studio �v���W�F�N�g�t�@�C��
������ README.md           # ���̃t�@�C��
```

## �@�\

- ImGui�R���e�L�X�g�����L���ă��C���A�v���P�[�V�����ƘA�g
- �V���v���ȃE�B���h�E�ƃR���g���[���̕\��
- �v���O�C���̕\��/��\���؂�ւ��@�\

## �v���O�C���C���^�[�t�F�[�X

### �G�N�X�|�[�g�֐�

```cpp
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
```

- `shared_context`: ���C���A�v���P�[�V�������狤�L�����ImGui�R���e�L�X�g
- `inputs`: ���̓f�[�^�i���݂͖��g�p�j
- `outputs`: �o�̓f�[�^�i���݂͖��g�p�j

### ������

```cpp
extern "C" __declspec(dllexport) void run(ImGuiContext* shared_context, const void* inputs, void* outputs)
{
    // ImGui�̃R���e�L�X�g���Z�b�g
    ImGui::SetCurrentContext(shared_context);

    // �v���O�C����UI��`��
    static bool show_window = true;
    static float slider_value = 0.0f;

    ImGui::Begin("DLL Window");
    ImGui::Text("Hello from DLL!");
    ImGui::SliderFloat("Slider", &slider_value, 0.0f, 1.0f);
    ImGui::Checkbox("Show Window", &show_window);
    ImGui::End();
}
```

## �r���h���@

1. Visual Studio��`PluginTest.vcxproj`���J��
2. �ݒ��Release x64�ɕύX
3. �r���h�����s
4. �������ꂽDLL��`Plugins/PluginTest/`�t�H���_�ɔz�u

## �v���O�C���̔z�u

���C���A�v���P�[�V�����͈ȉ��̍\���Ńv���O�C����T���܂��F

```
NSys.exe
������ Plugins/
    ������ PluginTest/
        ������ PluginTest.dll
```

## �Q�l����

- [ImGui Documentation](https://github.com/ocornut/imgui)
- [NSys ���C���v���W�F�N�g](../NSys/README.md)
