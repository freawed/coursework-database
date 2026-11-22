#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ClientFile.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <exception>
#include "clientdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(ui->openButton, &QPushButton::clicked,
            this, &MainWindow::openDatabase);
    connect(ui->addButton, &QPushButton::clicked,
            this, &MainWindow::addClient);

    ui->clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(ui->editButton, &QPushButton::clicked,
            this, &MainWindow::editClient);
    connect(ui->deleteButton, &QPushButton::clicked,
            this, &MainWindow::deleteClient);

    connect(ui->saveButton, &QPushButton::clicked,
            this, &MainWindow::saveDatabase);

    connect(ui->mergeButton, &QPushButton::clicked,
            this, &MainWindow::mergeDatabase);

    connect(ui->searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::filterClients);

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

void MainWindow::saveDatabase()
{
    QFileDialog dialog(this, "Сохранить базу");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setNameFilter("TSV (*.tsv)");
    dialog.setDefaultSuffix("tsv");

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    const QString path = dialog.selectedFiles().value(0);

    try {
        ClientFile::save(database, std::filesystem::path(path.toStdWString()));
        statusBar()->showMessage("База сохранена.", 3000);
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Ошибка сохранения",
                             QString::fromUtf8(error.what()));
    }
}

void MainWindow::mergeDatabase()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Выбрать базу для объединения", {}, "TSV (*.tsv)");

    if (path.isEmpty()) {
        return;
    }

    try {
        ClientDatabase incoming;
        ClientFile::load(incoming, std::filesystem::path(path.toStdWString()));

        if (!database.merge(incoming)) {
            QMessageBox::warning(this, "Объединение",
                                 "Совпадает номер счёта или закончились свободные ID.");
            return;
        }

        refreshTable();
        statusBar()->showMessage("Базы объединены.", 3000);
    } catch (const std::exception& error) {
        QMessageBox::warning(this, "Ошибка объединения",
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
    filterClients();
}

void MainWindow::addClient()
{
    ClientDialog dialog(this);

    connect(&dialog, &ClientDialog::submitRequested, &dialog, [this, &dialog] {
        try {
            if (!database.add(dialog.readClient())) {
                QMessageBox::warning(&dialog, "Добавление клиента",
                                     "Этот ID или номер счёта уже занят.");
                return;
            }

            dialog.accept();
        } catch (const std::exception& error) {
            QMessageBox::warning(&dialog, "Ошибка ввода",
                                 QString::fromUtf8(error.what()));
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void MainWindow::editClient()
{
    auto* table = ui->clientsTable;
    const auto selected = table->selectedItems();

    if (selected.isEmpty()) {
        QMessageBox::information(this, "Редактирование", "Выбери клиента в таблице.");
        return;
    }

    const int row = selected.first()->row();
    const int id = table->item(row, 0)->text().toInt();
    const Client* client = database.findById(id);

    if (client == nullptr) {
        QMessageBox::warning(this, "Редактирование", "Клиент не найден.");
        return;
    }

    ClientDialog dialog(this);
    dialog.setClient(*client);

    connect(&dialog, &ClientDialog::submitRequested, &dialog, [this, &dialog] {
        try {
            if (!database.update(dialog.readClient())) {
                QMessageBox::warning(&dialog, "Редактирование",
                                     "Этот номер счёта уже занят.");
                return;
            }

            dialog.accept();
        } catch (const std::exception& error) {
            QMessageBox::warning(&dialog, "Ошибка ввода",
                                 QString::fromUtf8(error.what()));
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        refreshTable();
    }
}

void MainWindow::deleteClient()
{
    auto* table = ui->clientsTable;
    const auto selected = table->selectedItems();

    if (selected.isEmpty()) {
        QMessageBox::information(this, "Удаление", "Выбери клиента в таблице.");
        return;
    }

    const int row = selected.first()->row();
    const int id = table->item(row, 0)->text().toInt();

    const auto answer = QMessageBox::question(
        this,
        "Удаление клиента",
        QStringLiteral("Удалить клиента с ID %1?").arg(id),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (answer != QMessageBox::Yes) {
        return;
    }

    if (!database.remove(id)) {
        QMessageBox::warning(this, "Удаление", "Клиент не найден.");
        return;
    }

    refreshTable();
}

void MainWindow::filterClients()
{
    const QString query = ui->searchEdit->text().trimmed();

    QSet<int> matchingIds;
    if (!query.isEmpty()) {
        const auto matches = database.findByName(query.toStdString());
        for (const Client& client : matches) {
            matchingIds.insert(client.getId());
        }
    }

    auto* table = ui->clientsTable;
    table->clearSelection();

    for (int row = 0; row < table->rowCount(); ++row) {
        const int id = table->item(row, 0)->text().toInt();
        const bool visible = query.isEmpty() || matchingIds.contains(id);
        table->setRowHidden(row, !visible);
    }
}