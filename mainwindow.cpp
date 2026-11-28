#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "ClientFile.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QInputDialog>
#include <QDateTime>
#include <QDir>
#include <QLocale>
#include <QPageSize>
#include <QPdfWriter>
#include <QSaveFile>
#include <QTextDocument>
#include <QFile>
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
        "Тип клиента", "VIP", "Баланс, коп.", "Дата открытия",
        "Счета в другом банке", "Специальное предложение"
    };

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(static_cast<int>(database.size()));
    const int offerColumn = table->columnCount() - 1;

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
            QString::fromStdString(client.getOpeningDate().toString()),
            client.hasOtherBankAccounts() ? QStringLiteral("Да") : QStringLiteral("Нет")
        };

        for (int column = 0; column < values.size(); ++column) {
            table->setItem(row, column, new QTableWidgetItem(values[column]));
        }

        if (client.hasOtherBankAccounts()) {
            auto* offerButton = new QPushButton("Отправить предложение");
            const int clientId = client.getId();

            connect(offerButton, &QPushButton::clicked, this, [this, clientId] {
                sendSpecialOffer(clientId);
            });

            table->setCellWidget(row, offerColumn, offerButton);
        } else {
            table->removeCellWidget(row, offerColumn);
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

void MainWindow::sendSpecialOffer(int clientId)
{
    const Client* client = database.findById(clientId);
    if (!client || !client->hasOtherBankAccounts()) {
        return;
    }

    const QString name = QString::fromStdString(client->getName());

    bool accepted = false;
    const double rate = QInputDialog::getDouble(
        this,
        "Специальное предложение",
        QStringLiteral("Ставка для клиента %1, % годовых").arg(name),
        10.0, 0.01, 100.0, 2, &accepted);

    if (!accepted) {
        return;
    }

    const QDir projectDir(QString::fromUtf8(COURSEWORK_PROJECT_DIR));

    QFile templateFile(projectDir.filePath(QStringLiteral("special_offer.html")));
    if (!templateFile.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка PDF",
                             "Не удалось открыть special_offer.html.");
        return;
    }

    const QString htmlTemplate = QString::fromUtf8(templateFile.readAll());
    if (templateFile.error() != QFileDevice::NoError || htmlTemplate.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка PDF",
                             "Шаблон пуст или не удалось его прочитать.");
        return;
    }

    const QString folder = QStringLiteral("users/%1").arg(clientId);

    if (!projectDir.mkpath(folder)) {
        QMessageBox::warning(this, "Ошибка PDF",
                             "Не удалось создать папку клиента.");
        return;
    }

    const QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss-zzz");
    const QString path =
        projectDir.filePath(folder + "/offer-" + timestamp + ".pdf");

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Ошибка PDF", file.errorString());
        return;
    }

    {
        QPdfWriter writer(&file);
        writer.setPageSize(QPageSize(QPageSize::A4));
        writer.setTitle("Специальное предложение по вкладу");

        const QString rateText = QLocale(QLocale::Russian).toString(rate, 'f', 2);
        QTextDocument document;
        document.setHtml(htmlTemplate.arg(name.toHtmlEscaped(), rateText));
        document.print(&writer);
    }

    if (file.pos() <= 0 || !file.commit()) {
        QMessageBox::warning(this, "Ошибка PDF", "Не удалось сохранить PDF.");
        return;
    }

    statusBar()->showMessage(
        QStringLiteral("Предложение сохранено: %1").arg(QDir::toNativeSeparators(path)),
        7000);
}