#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ClientFile.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>

#include <exception>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(ui->openButton, &QPushButton::clicked,
            this, &MainWindow::openDatabase);

    refreshTable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openDatabase()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Открыть базу", {}, "TSV (*.tsv)");

    if (path.isEmpty()) {
        return;
    }

    try {
        ClientFile::load(database, std::filesystem::path(path.toStdWString()));
        refreshTable();
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Ошибка открытия",
                             QString::fromUtf8(error.what()));
    }
}

void MainWindow::refreshTable()
{
    auto* table = ui->clientsTable;

    const QStringList headers{
        "ID", "ФИО", "Дата рождения", "Телефон", "Номер счёта",
        "Тип клиента", "VIP", "Баланс, коп.", "Дата открытия"
    };

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(static_cast<int>(database.size()));

    int row = 0;
    for (const Client& client : database) {
        const QStringList values{
            QString::number(client.getId()),
            QString::fromStdString(client.getName()),
            QString::fromStdString(client.getBirthDate().toString()),
            QString::fromStdString(client.getPhoneNumber()),
            QString::fromStdString(client.getAccountNumber()),
            client.getClientType() == ClientType::Individual
                ? QStringLiteral("Физлицо") : QStringLiteral("Юрлицо"),
            client.isVip() ? QStringLiteral("Да") : QStringLiteral("Нет"),
            QString::number(static_cast<qlonglong>(client.getBalanceInKopecks())),
            QString::fromStdString(client.getOpeningDate().toString())
        };

        for (int column = 0; column < values.size(); ++column) {
            table->setItem(row, column, new QTableWidgetItem(values[column]));
        }

        ++row;
    }

    table->resizeColumnsToContents();
}