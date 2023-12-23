#include <QCoreApplication>
#include <iostream>
#include "xana.h"
#include <QSqlDatabase>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Xana* xana = new Xana();
    bool ok = xana->ConnectXana();
    if (ok == true)
    {
        xana->setCPU("Core i5-12400", "Intel Z690");
        xana->setGPU("RX 6600");
        xana->setNetworkCard("Realtek 8111");
        xana->setAudioLayout("ALC887");
        xana->build();
        QSqlDatabase::database().close();
    }
    else
    {
        cout << "Подключение к Xana не удалось." << endl;
    }
    delete xana;
    a.exit();
    return a.exec();
}
