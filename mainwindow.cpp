#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QFont>
#include <QTimer>

static QString toString(unsigned long long number) {
    QList<unsigned int> parts;
    parts.append(number % 1000);
    int tmp = number / 1000;
    while (tmp > 0) {
        parts.insert(0, tmp % 1000);
        tmp /= 1000;
    }
    QString result = QString::number(parts.first());
    for (int i=1; i<parts.size(); i++) {
        result += QStringLiteral("'%1").arg(parts[i], 3, 10, QChar('0'));
    }
    return result;
};

static QString formatValue(unsigned long long valueInGr) {
    auto zl=valueInGr/100;
    auto gr=valueInGr%100;
    return QStringLiteral("%1,%2").arg(toString(zl)).arg(gr,2,10, QChar('0'));
}

static QString inWords(unsigned long long number) {
    auto static const k = 1000ULL;
    if (number == 0)
        return QStringLiteral("zero");
    if (number >= k*k*k*k)
        return QString::number(number);

    static const QList<QStringList> units = {
        { QStringLiteral(" tysiąc"), QStringLiteral(" tysiące"), QStringLiteral(" tysięcy") },
        { QStringLiteral(" milion"), QStringLiteral(" miliony"), QStringLiteral(" milionów") },
        { QStringLiteral(" miliard"), QStringLiteral(" miliardy"), QStringLiteral(" miliardów") },
    };

    QList<unsigned int> parts;
    parts.append(number % 1000);
    int tmp = number / 1000;
    while (tmp > 0) {
        parts.append(tmp % 1000);
        tmp /= 1000;
    }

    auto partInWord = [](unsigned int number) {
        auto h = number / 100;
        auto d = (number % 100) / 10;
        auto u = number % 10;

        QString result;
        if (h > 0) {
            static QString hundreds[] = {
                QString(),
                QStringLiteral("sto"),
                QStringLiteral("dwieście"),
                QStringLiteral("trzysta"),
                QStringLiteral("czterysta"),
                QStringLiteral("pięćset"),
                QStringLiteral("sześćset"),
                QStringLiteral("siedemset"),
                QStringLiteral("osiemset"),
                QStringLiteral("dziewięćset"),
            };
            result = hundreds[h];
        }
        if (d >= 2) {
            static QString dozens[] = {
                QString(),
                QString(),
                QStringLiteral("dwadzieścia"),
                QStringLiteral("trzydzieści"),
                QStringLiteral("czterdzieści"),
                QStringLiteral("pięćdziesiąt"),
                QStringLiteral("sześćdziesiąt"),
                QStringLiteral("siedemdziesiąt"),
                QStringLiteral("osiemdziesiąt"),
                QStringLiteral("dziewięćdziesiąt"),
            };
            if (!result.isEmpty())
                result.append(' ');
            result.append(dozens[d]);
        }
        if (d == 1) {
            static QString teens[] = {
                QStringLiteral("dziesięć"),
                QStringLiteral("jedenaście"),
                QStringLiteral("dwanaście"),
                QStringLiteral("trzynaście"),
                QStringLiteral("czternaście"),
                QStringLiteral("piętnaście"),
                QStringLiteral("szesnaście"),
                QStringLiteral("siedemnaście"),
                QStringLiteral("osiemnaście"),
                QStringLiteral("dziewiętnaście"),
            };
            if (!result.isEmpty())
                result.append(' ');
            result.append(teens[u]);
        }
        else if (u > 0) {
            static QString unity[] = {
                QString(),
                QStringLiteral("jeden"),
                QStringLiteral("dwa"),
                QStringLiteral("trzy"),
                QStringLiteral("cztery"),
                QStringLiteral("pięć"),
                QStringLiteral("sześć"),
                QStringLiteral("siedem"),
                QStringLiteral("osiem"),
                QStringLiteral("dziewięć"),
            };
            if (!result.isEmpty())
                result.append(' ');
            result.append(unity[u]);
        }
        return result;
    };

    QString result;
    for (int i=parts.size()-1; i>=0; i--) {
        unsigned int number = parts[i];
        if (number == 0)
            continue;
        auto numberInWord = partInWord(number);
        if (i > 0) {
            auto unit = [](int index, unsigned int number) {
                const auto &list = units[index];
                if (number == 1)
                    return list[0];
                auto d = (number % 100) / 10;
                auto u = number % 10;
                if (d != 1 && u >= 2 && u <= 4)
                    return list[1];
                return list[2];
            }(i-1, number);
            numberInWord += unit;
        }
        if (!result.isEmpty())
            result.append(' ');
        result.append(numberInWord);
    }
    return result;
}

static QString formatValueInWords(unsigned long long valueInGr) {
    auto zl=valueInGr/100;
    auto gr=valueInGr%100;

    auto zlInWords = inWords(zl);
    auto unit = [](int number) {
        if (number == 1)
            return QStringLiteral("złoty");
        int d = (number % 100) / 10;
        int u = number % 10;
        if (d != 1 && u >= 2 && u <= 4)
            return QStringLiteral("złote");
        return QStringLiteral("złotych");
    }(zl%1000);
    return QStringLiteral("%1 %2 %3/100").arg(zlInWords, unit).arg(gr, 2, 10, QChar('0'));
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mMapping = {
        {ui->spinBox_500zl, { 50000, ui->suma_500zl }},
        {ui->spinBox_200zl, { 20000, ui->suma_200zl }},
        {ui->spinBox_100zl, { 10000, ui->suma_100zl }},
        {ui->spinBox_50zl,  {  5000, ui->suma_50zl  }},
        {ui->spinBox_20zl,  {  2000, ui->suma_20zl  }},
        {ui->spinBox_10zl,  {  1000, ui->suma_10zl  }},
        {ui->spinBox_5zl,   {   500, ui->suma_5zl   }},
        {ui->spinBox_2zl,   {   200, ui->suma_2zl   }},
        {ui->spinBox_1zl,   {   100, ui->suma_1zl   }},
        {ui->spinBox_50gr,  {    50, ui->suma_50gr  }},
        {ui->spinBox_20gr,  {    20, ui->suma_20gr  }},
        {ui->spinBox_10gr,  {    10, ui->suma_10gr  }},
        {ui->spinBox_5gr,   {     5, ui->suma_5gr   }},
        {ui->spinBox_2gr,   {     2, ui->suma_2gr   }},
        {ui->spinBox_1gr,   {     1, ui->suma_1gr   }},
    };

    auto zeroString = formatValue(0);
    ui->suma->setText(zeroString);

    for (auto iter = mMapping.begin(); iter != mMapping.end(); iter++) {
        auto spinBox = iter.key();
        auto label = iter.value().second;
        spinBox->setValue(0);
        label->setText(zeroString);
        QObject::connect(spinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &MainWindow::onSpinBoxValueChanged);
    }

    QObject::connect(ui->drukuj, &QPushButton::clicked, this, &MainWindow::onPrintPressed);
    QObject::connect(qApp, &QApplication::focusChanged, this, &MainWindow::onFocusChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSpinBoxValueChanged(int count)
{
    auto spinBox = qobject_cast<QSpinBox*>(sender());
    if (spinBox == nullptr) {
        qWarning() << Q_FUNC_INFO << "Invalid sender";
        return;
    }
    auto iter = mMapping.find(spinBox);
    if (iter == mMapping.end()) {
        qWarning() << Q_FUNC_INFO << "Unable to find sender on QHash mMapping";
        return;
    }
    auto value = iter->first;
    auto lineEdit = iter->second;
    auto product = static_cast<unsigned long long>(value)*count;
    lineEdit->setText(formatValue(product));
    recalculateSum();
}

void MainWindow::onPrintPressed()
{
    QPrintDialog printDialog;
    auto dialogResult = printDialog.exec();
    if (dialogResult != QDialog::Accepted)
        return;
    auto printer = printDialog.printer();
    if (printer == nullptr) {
        qWarning() << Q_FUNC_INFO << "null printer";
        return;
    }
    printer->setPageOrientation(QPageLayout::Landscape);

    auto leftRect = printer->pageRect();
    auto margin = leftRect.x();
    leftRect.moveTo(0,0);
    leftRect.setWidth(leftRect.width()/2-margin);
    auto rightRect = leftRect;
    rightRect.moveTo(leftRect.width()+2*margin, 0);

    QPainter painter;
    if (!painter.begin(printer)) {
        qWarning() << Q_FUNC_INFO << "QPainter::begin(QPrinter*) failed";
        return;
    }

    auto list = [this](){
        QList<QPair<unsigned long, int>> result;
        QMap<unsigned long, int> mOrderedMap;
        for (auto iter = mMapping.begin(); iter != mMapping.end(); iter++) {
            auto nominal = iter.value().first;
            auto count = iter.key()->value();
            mOrderedMap.insert(nominal, count);
        }
        for (auto iter = mOrderedMap.begin(); iter != mOrderedMap.end(); iter++) {
            auto &nominal = iter.key();
            auto &count = iter.value();
            result.insert(0, { nominal, count });
        }
        return result;
    }();

    painter.drawRect(leftRect);
    painter.drawRect(rightRect);
    auto font = painter.font();
    font.setPixelSize(16);
    painter.setFont(font);

    int lineSpace = 4;
    int top = 0;

    static const auto lineFlags = Qt::TextWordWrap;

    auto printLine = [&printer, &painter, &leftRect, &rightRect, &top, &lineSpace](const QString &line) {
        auto lineRect = painter.boundingRect(0, 0, leftRect.width(), leftRect.height(), lineFlags, line);
        if (lineRect.height() + top > rightRect.height()) {
            if (top == 0) {
                qWarning() << "Line to big to fit on page";
                return false;
            }
            if (!printer->newPage()) {
                qWarning() << "QPrinter::newPage() failed";
                return false;
            }
            top = 0;
        }
        lineRect.moveTo(leftRect.x(), top);
        painter.drawText(lineRect, lineFlags, line, nullptr);
        lineRect.moveTo(rightRect.x(), top);
        painter.drawText(lineRect, lineFlags, line, nullptr);
        top += lineRect.height() + lineSpace;
        return true;
    };

    unsigned long long total = 0;
    for (auto &pair : list) {
        auto &nominal = pair.first;
        auto &count = pair.second;
        auto product = static_cast<unsigned long long>(nominal)*count;
        auto line = QStringLiteral("%1 x %2 = %3").arg(formatValue(nominal)).arg(count).arg(formatValue(product));
        if (!printLine(line))
            return;
        total += product;
    }

    font.setBold(true);
    painter.setFont(font);
    top += 12;

    QStringList boldLines = {
        QStringLiteral("SUMA: %1").arg(formatValue(total)),
        QStringLiteral("SŁOWNIE: %1").arg(formatValueInWords(total))
    };

    for (auto &line : boldLines) {
        if (!printLine(line))
            return;
    }

    if (!painter.end()) {
        qWarning() << "QPainter::end() failed";
        return;
    }
}

void MainWindow::onFocusChanged(QWidget *, QWidget *newWidget)
{
    auto spinBox = qobject_cast<QSpinBox*>(newWidget);
    if (spinBox == nullptr)
        return;
    auto selectAll = [spinBox]() {
        spinBox->selectAll();
    };
    QTimer::singleShot(0, this, selectAll);
}

void MainWindow::recalculateSum()
{
    unsigned long long total = 0;
    for (auto iter = mMapping.begin(); iter != mMapping.end(); iter++) {
        auto spinBox = iter.key();
        int count = spinBox->value();
        unsigned long long nominal = iter->first;
        auto product = count * nominal;
        total += product;
    }
    ui->suma->setText(formatValue(total));
    qInfo().noquote() << ui->suma->text() << formatValueInWords(total);
}

