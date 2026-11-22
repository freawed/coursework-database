#include "clientdialog.h"
#include "ui_clientdialog.h"
#include <stdexcept>

ClientDialog::ClientDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientDialog)
{
    ui->setupUi(this);
    setWindowTitle("Добавить клиента");
    ui->typeCombo->addItem("Физлицо");
    ui->typeCombo->addItem("Юрлицо");

    ui->buttonBox->setStandardButtons(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ClientDialog::submitRequested);
    connect(ui->buttonBox, &QDialogButtonBox::rejected,
            this, &QDialog::reject);
}

ClientDialog::~ClientDialog()
{
    delete ui;
}

Client ClientDialog::readClient() const
{
    bool ok = false;
    const int id = ui->idEdit->text().toInt(&ok);
    if (!ok) {
        throw std::invalid_argument("ID должен быть целым числом.");
    }

    const std::int64_t balance = ui->balanceEdit->text().toLongLong(&ok);
    if (!ok) {
        throw std::invalid_argument("Введите баланс целым числом копеек.");
    }

    Date birthDate;
    if (!birthDate.setFromString(ui->birthDateEdit->text().toStdString())) {
        throw std::invalid_argument("Введите дату рождения в формате ДД.ММ.ГГГГ.");
    }

    Date openingDate;
    if (!openingDate.setFromString(ui->openingDateEdit->text().toStdString())) {
        throw std::invalid_argument("Введите дату открытия в формате ДД.ММ.ГГГГ.");
    }

    const ClientType type = ui->typeCombo->currentIndex() == 0
                                ? ClientType::Individual : ClientType::Business;

    return Client(id, ui->nameEdit->text().toStdString(), birthDate,
                  ui->phoneEdit->text().toStdString(),
                  ui->accountEdit->text().toStdString(), type,
                  ui->vipCheck->isChecked(), balance, openingDate);
}