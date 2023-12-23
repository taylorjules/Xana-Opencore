// Xana. Multi-agent program for build Opencore Loader
// Author Alice Sherbakova.

#ifndef XANA_H
#define XANA_H

#include <QString>
#include <QSqlDatabase>
#include <iostream>
#include <string>
#include <QSqlQuery>
#include <filesystem>
#include "md5.h"

using namespace std;

inline const char * const BoolToString(bool b)
{
    return b ? "true" : "false";
}

class Xana {
public:
    Xana();

    ~Xana() {
        this -> Speak("Освобождение памяти");
        delete [] this -> kexts_url;
        delete [] this -> kexts_binary;
        delete [] this -> kexts_restrict;
        delete [] this -> errors;
        delete [] this -> errors_name;
        delete [] this -> ssdts_sample;
        delete [] this -> booter_quirks;
        delete [] this -> kernel_quircks;
        delete [] this -> booter_quirks_bool;
        delete [] this -> kernel_quircks_bool;
        delete [] this -> boot_arguments;
        this -> Speak("Освобождение памяти успешно");
    }

    bool ConnectXana() {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("data.sqlite");

        this -> errors_name[0] = "XanaCPUHalted";
        this -> errors_name[1] = "XanaChipsetHalted";
        this -> errors_name[2] = "XanaGPUHalted";
        this -> errors_name[3] = "XanaNetworkHalted";
        this -> errors_name[4] = "XanaAudioHalted";
        this -> errors_name[5] = "XanaGraphicHalted";

        return db.open();
    }

    int setCPU(string cpuF, string chipset) {
        this -> Speak("Получаю управление от высшего агента.");
        QSqlQuery query;
        query.exec("SELECT * FROM `xanaCPU` WHERE `name` LIKE '" + QString::fromStdString(cpuF) + "'");
        query.next();
        if (query.isNull("name") == false) {
            this -> addKext("urlLoad", "Lilu");
            this -> Speak("Я определила процессор: " + query.value("name").toString().toStdString());
            this -> Speak("Семейство: " + query.value("family").toString().toStdString());
            this -> Speak("Передаю данные процессора высшему агенту (Сборщик 1.0v)");
            this -> cpu[0] = query.value("id").toString().toStdString();
            this -> cpu[1] = query.value("name").toString().toStdString();
            this -> cpu[2] = query.value("family").toString().toStdString();
            this -> cpu[3] = query.value("igpu").toString().toStdString();

            query.exec("SELECT * FROM `xanaChipsets` WHERE `family` LIKE '" + QString::fromStdString(this -> cpu[2]) + "'");
            query.next();

            this -> cpu[5] = query.value("id").toString().toStdString();
            this -> Speak("Данные переданы");

            query.exec("SELECT * FROM `xanaCPU` WHERE `name` LIKE '" + QString::fromStdString(cpuF) + "'");
            query.next();

            if (query.value("igpu").toString().toStdString() == "NSL") {
                this -> Speak("Встроенную видео-карту завести нельзя");
                this -> igpu_work = false;
            } else if (query.value("igpu").toString().toStdString() == "0") {
                this -> Speak("Встроенная видео-карта отсутствует");
                this -> igpu_work = false;
            } else {
                this -> Speak("Вижу встроенную графику, определяю...");
                this -> igpu_work = true;
                query.exec("SELECT * FROM `xana_iGPU` WHERE `id` LIKE '" + QString::fromStdString(this -> cpu[3]) + "'");
                query.next();
                this -> Speak("Графика - " + query.value("name").toString().toStdString());
                this -> cpu[9] = query.value("name").toString().toStdString();
            }

            this -> Speak("Предварительная проверка чипсета ");
            query.exec("SELECT * FROM `xanaChipsets` WHERE `family` LIKE '" + QString::fromStdString(this -> cpu[2]) + "'");
            query.next();
            std::vector < std::string > chps = this -> toArray(query.value("chipsets").toString().toStdString());
            int ukaz_chipa = this -> findValue(chps, chipset);
            if (ukaz_chipa != 404) {
                if (chps[ukaz_chipa] == chipset) {
                    this -> Speak("Чипсет определен: - " + chipset);
                    this -> Speak("Передаю параметры чипсета высшему агенту");
                    cpu[4] = chipset;
                } else {
                    this -> setErrorFlag(1);
                    return 1;
                }
            } else {
                this -> setErrorFlag(1);
                return 1;
            }

            this -> Speak("Определяю SSDT...");
            query.exec("SELECT * FROM `xanaSSDT` WHERE `chipset` LIKE '" + QString::fromStdString(this -> cpu[4]) + "'");
            query.next();
            this -> ssdt = this -> toArray(query.value("path").toString().toStdString());
            this -> Speak("Передаю ssdt высшему агенту");

            this -> Speak("Определяю маскировку процессора...");
            query.exec("SELECT * FROM `xanaChipsets` WHERE `family` LIKE '" + QString::fromStdString(this -> cpu[2]) + "'");
            query.next();
            if (query.value("cpuid1data").toString().toStdString() != "") {
                this -> Speak("Вижу смысл маскировать процессор");
                this -> Speak("cpuid1data: " + query.value("cpuid1data").toString().toStdString());
                this -> Speak("cpuid1mask: " + query.value("cpuid1mask").toString().toStdString());
                this -> Speak("Передаю данные высшему агенту");

                this -> cpu[7] = query.value("cpuid1data").toString().toStdString();
                this -> cpu[8] = query.value("cpuid1mask").toString().toStdString();
            } else {
                this -> Speak("Не вижу смысла маскировать процессор");
            }
            this -> Speak("Прение оконечно. Передаю управление высшему агенту");
        } else {
            this -> setErrorFlag(0);
            return 1;
        }
        return 0;
    }

    int setGPU(string gpu_name) {
        this -> Speak("Получаю управление от высшего агента");
        QSqlQuery query;
        query.exec("SELECT * FROM `xanaGPU` WHERE `name` LIKE '" + QString::fromStdString(gpu_name) + "'");
        query.next();
        if (query.isNull("name") == false) {
            this -> Speak("Определена дискретная видеокарта - " + query.value("name").toString().toStdString());
            this -> gpu_work = true;
        } else {
            this -> setErrorFlag(2);
            return 1;
        }
        this -> Speak("Прение оконечно. Передаю управление высшему агенту");
        return 0;
    }

    int setNetworkCard(string name_network) {
        this -> Speak("Получаю управление от высшего агента");
        QSqlQuery query;
        query.exec("SELECT * FROM `ethernetKexts` WHERE `family` LIKE '" + QString::fromStdString(name_network) + "'");
        query.next();
        if (query.isNull("id") == false) {
            this -> Speak("Определена сетевая карта - " + query.value("family").toString().toStdString());
            this -> Speak("Определяю кексты и исполняемые данные");
            this -> Speak("Передаю данные высшему агенту");
            vector < string > hh = this -> toArray(query.value("kexts").toString().toStdString());
            if (hh[0] != "") {
                this -> network_card_work = true;
                this -> addKext(hh[0], hh[1]);
            } else {
                this -> setErrorFlag(1);
                return 1;
            }
        } else {
            this -> Speak("Сетевая карта не может быть инициализирована");
            this -> setErrorFlag(3);
            return 1;
        }
        this -> Speak("Прение оконечно. Передаю управление высшему агенту");
        return 0;
    }

    int setAudioLayout(string codec) {
        this -> Speak("Получаю управление от высшего агента");
        this -> Speak("Определяю Аудио-кодек");
        QSqlQuery query;
        query.exec("SELECT * FROM `audioCodecXana` WHERE `codec` LIKE '" + QString::fromStdString(codec) + "'");
        query.next();
        if (query.isNull("id") == false) {
            this -> audio_work = true;
            this -> Speak("Определен слой - " + query.value("layout").toString().toStdString());
            this -> Speak("Передаю слой высшему агенту");
            this -> layout_audio = query.value("layout").toString().toStdString();
        } else {
            this -> setErrorFlag(4);
            return 1;
        }
        this -> Speak("Прение оконечно. Передаю управление высшему агенту");
        return 0;
    }

    int build() {
        this -> Speak("Получаю управление от высшего агента, начинаю инициализацию сборки EFI");

        int result = this -> checkCondicinal();

        if (result == 6439) {
            this -> Speak("Ошибок не обнаружено");
        } else {
            this -> Speak("Сборка не возможна, ошибка " + this -> errors_name[result]);
            return result;
        }

        this -> initSsdt();
        this -> initBooterQuircks();
        this -> initKexts();
        this -> initKernelQuircks();
        this -> initBootArguments();
        this -> detectAudioDevice();
        this -> initDirectoryes();

        return 0;
    }

private:
    // Agent of Errors
    bool * errors = new bool[10] {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false
    };
    string * errors_name = new string[10];
    string & str_replace(const string & search,
                        const string & replace, string & subject) {
        string buffer;

        int sealeng = search.length();
        int strleng = subject.length();

        if (sealeng == 0)
            return subject; //no change

        for (int i = 0, j = 0; i < strleng; j = 0) {
            while (i + j < strleng && j < sealeng && subject[i + j] == search[j])
                j++;
            if (j == sealeng) //found 'search'
            {
                buffer.append(replace);
                i += sealeng;
            } else {
                buffer.append( & subject[i++], 1);
            }
        }
        subject = buffer;
        return subject;
    }

    void Speak(string text) {
        cout << "Xana: " << text << endl;
    }

    void setErrorFlag(int flag) {
        this -> Speak("ERROR " + this -> errors_name[flag]);
        this -> errors[flag] = true;
    }

    std::vector < std::string > explode(std::string separator, std::string input) {
        std::vector < std::string > vec;
        for (unsigned long i {
                 0
             }; i < input.length(); ++i) {
            int pos = input.find(separator, i);
            if (pos < 0) {
                vec.push_back(input.substr(i));
                break;
            }
            int count = pos - i;
            vec.push_back(input.substr(i, count));
            i = pos + separator.length() - 1;
        }
        return vec;
    }

    int findValue(const std::vector < string > & data, string value) {
        auto result {
            std::find(begin(data), end(data), value)
        };
        if (result == end(data))
            return 404;
        else
            return (result - begin(data));
    }

    // Агент процессора
    string cpu[10];
    bool igpu_work = false;
    string workspace;
    std::vector < std::string > ssdt;

    // Агент дискретной видеокарты
    bool gpu_work = false;

    // Агент сетевой карты
    bool network_card_work = false;

    //Агент аудиокарты
    bool audio_work = false;
    string layout_audio;

    // Agent of Kexts Restrict Memory
    string * kexts_url = new string[500];
    string * kexts_binary = new string[500];
    string * kexts_restrict = new string[500];
    int ukaz_kexta = 1;

    // AGENTS OF BUILD OPENCORE
    string * ssdts_sample = new string[500];
    string * boot_arguments = new string[15];

    string audio_device;

    // AGENTS OF OPENCORE FOR BOOT QUIRKES
    string bq_name[22] {
        "FixupAppleEfiImages",
        "AllowRelocationBlock",
        "AvoidRuntimeDefrag",
        "DevirtualiseMmio",
        "DisableSingleUser",
        "DisableVariableWrite",
        "DiscardHibernateMap",
        "EnableSafeModeSlide",
        "EnableWriteUnprotector",
        "ForceBooterSignature",
        "ForceExitBootServices",
        "ProtectMemoryRegions",
        "ProtectSecureBoot",
        "ProtectUefiServices",
        "ProvideCustomSlide",
        "ProvideMaxSlide",
        "RebuildAppleMemoryMap",
        "ResizeAppleGpuBars",
        "SetupVirtualMap",
        "SignalAppleOS",
        "SyncRuntimePermissions"
    };

    bool * booter_quirks_bool = new bool[22] {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false
    };
    string * booter_quirks = new string[22];

    void addBooterQuirk(string name, bool value) {
        string * tt = new string();
        if (value) {
            * tt = "true";
        } else {
            * tt = "false";
        }
        this -> booter_quirks -> append("<key>" + name + "</key>" + "<" + * tt + "/>");
        delete tt;
    }

    void initBooterQuircks() {
        this -> Speak("Инициализация Booter Quricks");

        int* family = new int(stoi(this->cpu[5]));

        this -> booter_quirks_bool[0] = true;
        this -> Speak(this -> bq_name[0] + ": TRUE");

        this -> booter_quirks_bool[1] = false;
        this -> Speak(this -> bq_name[1] + ": FALSE");

        this -> booter_quirks_bool[2] = true;
        this -> Speak(this -> bq_name[2] + ": TRUE");

        if (*family >= 8 && *family <= 13) {
            this -> booter_quirks_bool[3] = true;
            this -> Speak(this -> bq_name[3] + ": TRUE");
        } else {
            this -> booter_quirks_bool[3] = false;
            this -> Speak(this -> bq_name[3] + ": FALSE");
        }

        this -> booter_quirks_bool[4] = false;
        this -> Speak(this -> bq_name[4] + ": FALSE");

        this -> booter_quirks_bool[5] = false;
        this -> Speak(this -> bq_name[5] + ": FALSE");

        this -> booter_quirks_bool[6] = false;
        this -> Speak(this -> bq_name[6] + ": FALSE");

        this -> booter_quirks_bool[7] = true;
        this -> Speak(this -> bq_name[7] + ": TRUE");

        if (*family <= 6 || *family == 14) {
            this -> booter_quirks_bool[8] = true;
            this -> Speak(this -> bq_name[8] + ": TRUE");
        } else {
            this -> booter_quirks_bool[8] = false;
            this -> Speak(this -> bq_name[8] + ": FALSE");
        }

        this -> booter_quirks_bool[9] = false;
        this -> Speak(this -> bq_name[9] + ": FALSE");

        this -> booter_quirks_bool[10] = false;
        this -> Speak(this -> bq_name[10] + ": FALSE");

        this -> booter_quirks_bool[11] = false;
        this -> Speak(this -> bq_name[11] + ": FALSE");

        this -> booter_quirks_bool[12] = false;
        this -> Speak(this -> bq_name[12] + ": FALSE");

        this -> booter_quirks_bool[13] = true;
        this -> Speak(this -> bq_name[13] + ": TRUE");

        this -> booter_quirks_bool[14] = true;
        this -> Speak(this -> bq_name[14] + ": TRUE");

        this -> booter_quirks_bool[15] = false;
        this -> Speak(this -> bq_name[15] + ": INT 0");

        delete family;

        for (int i = 0; i < 22; i++) {
            this -> addBooterQuirk(this -> bq_name[i], this -> booter_quirks_bool[i]);
        }
    }



    // AGENTS OF OPENCORE FOR KERNEL QUIRKES
    string kq_name[23]
    {
        "AppleCpuPmCfgLock",
        "AppleXcpmCfgLock",
        "AppleXcpmExtraMsrs",
        "AppleXcpmForceBoost",
        "CustomPciSerialDevice",
        "CustomSMBIOSGuid",
        "DisableIoMapper",
        "DisableIoMapperMapping",
        "DisableLinkeditJettison",
        "DisableRtcChecksum",
        "ExtendBTFeatureFlags",
        "ExternalDiskIcons",
        "ForceAquantiaEthernet",
        "ForceSecureBootScheme",
        "IncreasePciBarSize",
        "LapicKernelPanic",
        "LegacyCommpage",
        "PanicNoKextDump",
        "PowerTimeoutKernelPanic",
        "ProvideCurrentCpuInfo",
        "SetApfsTrimTimeout",
        "ThirdPartyDrives",
        "XhciPortLimit"
    };

    bool* kernel_quircks_bool = new bool[23] {
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
        false
    };

    string * kernel_quircks = new string[23];

    void addKernelQuirk(string name, bool value) {
        string * tt = new string();
        if (value) {
            * tt = "true";
        } else {
            * tt = "false";
        }
        this -> kernel_quircks -> append("<key>" + name + "</key>" + "<" + * tt + "/>");
        delete tt;
    }

    void initKernelQuircks()
    {

        int* family = new int(stoi(this->cpu[5]));

        this->kernel_quircks_bool[0] = false;
        this->kernel_quircks_bool[1] = true;

        if (*family == 14)
        {
            this->kernel_quircks_bool[2] = true;
        }
        else
        {
            this->kernel_quircks_bool[2] = false;
        }

        if (*family == 14)
        {
            this->kernel_quircks_bool[3] = true;
        }
        else
        {
            this->kernel_quircks_bool[3] = false;
        }

        this->kernel_quircks_bool[4] = false;
        this->kernel_quircks_bool[5] = false;
        this->kernel_quircks_bool[6] = true;
        this->kernel_quircks_bool[7] = true;
        this->kernel_quircks_bool[8] = true;
        this->kernel_quircks_bool[9] = true;
        this->kernel_quircks_bool[10] = false;

        if (*family == 14 or *family == 1 or *family == 2)
        {
            this->kernel_quircks_bool[11] = true;
        }
        else
        {
            this->kernel_quircks_bool[11] = false;
        }

        this->kernel_quircks_bool[12] = false;
        this->kernel_quircks_bool[13] = false;

        if (*family == 14)
        {
            this->kernel_quircks_bool[14] = true;
        }
        else
        {
            this->kernel_quircks_bool[14] = false;
        }

        if (*family <= 2)
        {
            this->kernel_quircks_bool[15] = true;
        }
        else
        {
            this->kernel_quircks_bool[15] = false;
        }

        this->kernel_quircks_bool[16] = false;
        this->kernel_quircks_bool[17] = true;
        this->kernel_quircks_bool[18] = true;

        if (*family == 10 or *family == 12 or *family == 13)
        {
            this->kernel_quircks_bool[19] = true;
        }
        else
        {
            this->kernel_quircks_bool[19] = false;
        }

        this->kernel_quircks_bool[20] = false;
        this->kernel_quircks_bool[21] = false;
        this->kernel_quircks_bool[22] = true;

        delete family;

        for (int i = 0; i < 22; i++) {
            this->Speak(this->kq_name[i] + " = " + BoolToString(this -> kernel_quircks_bool[i]));
            this -> addKernelQuirk(this -> kq_name[i], this -> kernel_quircks_bool[i]);
        }

    }

    // XANA CONNECTORS
    std::vector < std::string > toArray(string json) {
        json = this -> str_replace("[", "", json);
        json = this -> str_replace("]", "", json);
        json = this -> str_replace("{", "", json);
        json = this -> str_replace("}", "", json);
        json = this -> str_replace("\"", "", json);
        json = this -> str_replace("\",\"", ", ", json);
        json = this -> str_replace(", ", ",", json);
        json = this -> str_replace(",", ", ", json);

        return this -> explode(", ", json);
    }

    // ADD KEXT TO THE MEMORY
    void addKext(string url, string binary_file, bool restrict = false) {
        this -> Speak("Подгрузка кекста " + binary_file);
        this -> kexts_url[this -> ukaz_kexta] = "https://skidblandir.ru/kexts/" + url;
        this -> kexts_binary[this -> ukaz_kexta] = binary_file;
        this -> kexts_restrict[this -> ukaz_kexta] = restrict;
        this -> ukaz_kexta++;
    }

    // ADD SDDT TO THE MEMORY
    void addSSDT(string path) {
        this -> ssdts_sample -> append("<dict><key>Comment</key><string></string><key>Enabled</key><true/><key>Path</key><string>" + path + "</string></dict>");
    }


    //AGENTS OF BUILD

    int checkCondicinal() {
        this -> Speak("Проверяю основные положения");
        if (this -> igpu_work == false && this -> gpu_work == false) {
            this -> setErrorFlag(5);
        }
        for (int i = 0; i <= 4; i++) {
            if (this -> errors[i] == true) {
                return i;
            }
        }

        return 6439;
    }

    void initSsdt() {
        this -> Speak("Инициализация ssdt");

        for (string path : this -> ssdt)
            this -> addSSDT(path);
    }



    void initKexts() {
        this -> addKext("VirtualSMC.kext.zip", "VirtualSMC");
        this -> addKext("AppleALC.kext.zip", "AppleALC");
        this -> addKext("ECEnabler.kext.zip", "ECEnabler");
        this -> addKext("CpuTscSync.kext.zip", "CpuTscSync");
        this -> addKext("SMCProcessor.kext.zip", "SMCProcessor");
        this -> addKext("SMCSuperIO.kext.zip", "SMCSuperIO");
        this -> addKext("RestrictEvents.kext.zip", "RestrictEvents");
        this -> addKext("WhateverGreen.kext.zip", "WhateverGreen");
        this -> addKext("USBToolBox.kext.zip", "USBToolBox");
        this -> addKext("UTBMap.kext.zip", "UTBMap", true);
        this -> addKext("NoTouchID.kext.zip", "NoTouchID");
        this -> addKext("CPUFriend.kext.zip", "CPUFriend");
        this -> addKext("CPUFriendDataProvider.kext.zip", "CPUFriendDataProvider", true);
        this -> addKext("HibernationFixup.kext.zip", "HibernationFixup");
        this -> addKext("RTCMemoryFixup.kext.zip", "RTCMemoryFixup");
        this -> addKext("FeatureUnlock.kext.zip", "FeatureUnlock");

        if (this -> cpu[2] == "Sandy Bridge"
            or this -> cpu[2] == "Ivy Bridge") {
            this -> addKext("CryptexFixup.kext.zip", "CryptexFixup");
        }

        if (this -> cpu[2] == "X99") {
            this -> addKext("USB_Injector x99.kext.zip", "USB_Injector x99", true);
            this -> addKext("XLNCUSBFix.kext.zip", "XLNCUSBFix", true);
            this -> addKext("AppleMCEReporterDisabler.kext.zip", "AppleMCEReporterDisabler", true);
        }
    }

    void addBootArgument(string argument)
    {
        this->boot_arguments->append(argument);
    }

    void initBootArguments()
    {
        this->addBootArgument("-v");
        this->addBootArgument("-lilubetaall");
        this->addBootArgument("keepsyms=1");
        this->addBootArgument("debug=0x100");
        this->addBootArgument("ipc_control_port_options=0");

        if (this->gpu_work)
            this->addBootArgument("agdpmod=pikera");
        else if (this->igpu_work)
            this->addBootArgument("agdpmod=vit9696");

        if (this->audio_work)
            this->addBootArgument("alcid="+this->layout_audio);

        int* family = new int(stoi(this->cpu[5]));
        if (*family == 6 or *family == 5)
            if (this->igpu_work)
            {
                this->addBootArgument("lilucpu=9");
                this->addBootArgument("-igfxsklaskbl");
            }


        if (*family == 14)
        {
            this->addBootArgument("npci=0x3000");
            this->addBootArgument("tlbto_us=0");
            this->addBootArgument("-cputsclock");
        }

        delete family;

    }

    void detectAudioDevice()
    {
        if (stoi(this->cpu[5]) == 14 or stoi(this->cpu[5]) < 5)
            this->audio_device = "PciRoot(0x0)/Pci(0x1B,0x0)";
        else
            this->audio_device = "PciRoot(0x0)/Pci(0x1F,0x3)";
    }

    string generateHash()
    {
        int rand = random();
        return md5(to_string(rand));
    }

    void mkdir(string path)
    {
        filesystem::create_directory(path);
    }

    void initDirectoryes()
    {
        this->workspace = "Workspaces/" + this -> generateHash();
        this->mkdir(this->workspace);
        this->mkdir(this->workspace+"/EFI/");
        this->mkdir(this->workspace+"/EFI/OC/");
        this->mkdir(this->workspace+"/EFI/BOOT/");
        this->mkdir(this->workspace+"/EFI/OC/Kexts/");
        this->mkdir(this->workspace+"/EFI/OC/ACPI/");

    }


};

#endif // XANA_H
