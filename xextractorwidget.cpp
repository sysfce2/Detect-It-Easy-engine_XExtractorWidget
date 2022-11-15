/* Copyright (c) 2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "xextractorwidget.h"

#include "ui_xextractorwidget.h"

XExtractorWidget::XExtractorWidget(QWidget *pParent) : XShortcutsWidget(pParent), ui(new Ui::XExtractorWidget)
{
    ui->setupUi(this);

    g_pDevice = nullptr;
    g_options = {};

    QList<XComboBoxEx::CUSTOM_FLAG> listCustomFlags;

    QList<XBinary::FT> listFileTypes = XExtractor::getAvailableFileTypes();

    qint32 nNumberOfRecords = listFileTypes.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        XComboBoxEx::_addCustomFlag(&listCustomFlags, listFileTypes.at(i), XBinary::fileTypeIdToString(listFileTypes.at(i)), false);
    }

    ui->comboBoxOptions->addCustomFlags(listCustomFlags);
}

XExtractorWidget::~XExtractorWidget()
{
    delete ui;
}

void XExtractorWidget::setData(QIODevice *pDevice, XExtractor::OPTIONS options, bool bAuto)
{
    g_pDevice = pDevice;
    g_options = options;

    qint32 nNumberOfRecords = options.listFileTypes.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        ui->comboBoxOptions->setCustomFlag(options.listFileTypes.at(i));
    }

    ui->checkBoxDeepScan->setChecked(options.bDeepScan);

    if (bAuto) {
        reload();
    }
}

void XExtractorWidget::reload()
{
    ui->labelSize->setText(XBinary::valueToHexEx(g_pDevice->size()));

    XExtractor::DATA extractor_data = {};

    QList<quint64> listFlags = ui->comboBoxOptions->getCustomFlags();

    qint32 nNumberOfRecords = listFlags.count();

    for (qint32 i = 0; i < nNumberOfRecords; i++) {
        extractor_data.options.listFileTypes.append((XBinary::FT)listFlags.at(i));
    }

    extractor_data.options.bDeepScan = ui->checkBoxDeepScan->isChecked();

    DialogExtractorProcess dep(XOptions::getMainWidget(this), g_pDevice, &extractor_data);

    dep.showDialogDelay(1000);

    if (dep.isSuccess()) {
        QAbstractItemModel *pOldModel = ui->tableViewResult->model();

        qint32 nNumberOfRecords = extractor_data.listRecords.count();

        QStandardItemModel *pModel = new QStandardItemModel(nNumberOfRecords, 4);

        pModel->setHeaderData(0, Qt::Horizontal, tr("Offset"));
        pModel->setHeaderData(1, Qt::Horizontal, tr("Size"));
        pModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
        pModel->setHeaderData(3, Qt::Horizontal, QString(""));

        for (qint32 i = 0; i < nNumberOfRecords; i++) {
            {
                QStandardItem *pItem = new QStandardItem;

                pItem->setText(XBinary::valueToHexEx(extractor_data.listRecords.at(i).nOffset));

                pItem->setData(extractor_data.listRecords.at(i).nOffset, Qt::UserRole + 0);
                pItem->setData(extractor_data.listRecords.at(i).nSize, Qt::UserRole + 1);
                pItem->setData(extractor_data.listRecords.at(i).sExt, Qt::UserRole + 2);

                pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                pModel->setItem(i, 0, pItem);
            }
            {
                QStandardItem *pItem = new QStandardItem;

                pItem->setText(XBinary::valueToHexEx(extractor_data.listRecords.at(i).nSize));

                pItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

                pModel->setItem(i, 1, pItem);
            }
            {
                QStandardItem *pItem = new QStandardItem;

                pItem->setText(XBinary::fileTypeIdToString(extractor_data.listRecords.at(i).fileType));

                pModel->setItem(i, 2, pItem);
            }
            {
                QStandardItem *pItem = new QStandardItem;

                pItem->setText(extractor_data.listRecords.at(i).sString);

                pModel->setItem(i, 3, pItem);
            }
            //            {
            //                QPushButton *result=new QPushButton(this); // TODO
            //                remove

            //                result->setText(tr("Dump"));
            //                result->setProperty("OFFSET",extractor_data.listRecords.at(i).nOffset);
            //                result->setProperty("SIZE",extractor_data.listRecords.at(i).nSize);

            //                connect(result,SIGNAL(clicked()),this,SLOT(pushButtonAction()));

            //                ui->tableWidgetResult->setCellWidget(i,4,result);
            //            }
        }

        ui->tableViewResult->setModel(pModel);

        deleteOldAbstractModel(&pOldModel);

        ui->tableViewResult->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
        ui->tableViewResult->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
        ui->tableViewResult->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
        ui->tableViewResult->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    }
}

DumpProcess::RECORD XExtractorWidget::getDumpProcessRecord(QModelIndex index)
{
    DumpProcess::RECORD result = {};

    result.nOffset = ui->tableViewResult->model()->data(index, Qt::UserRole + 0).toLongLong();
    result.nSize = ui->tableViewResult->model()->data(index, Qt::UserRole + 1).toLongLong();
    QString sExt = ui->tableViewResult->model()->data(index, Qt::UserRole + 2).toString();

    QString sName = QString("%1_%2").arg(XBinary::valueToHexEx(result.nOffset), XBinary::valueToHexEx(result.nSize));

    result.sFileName = XBinary::getResultFileName(g_pDevice, QString("%1.%2").arg(sName, sExt));

    return result;
}

void XExtractorWidget::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
    // TODO
}

void XExtractorWidget::on_pushButtonScan_clicked()
{
    reload();
}

void XExtractorWidget::on_pushButtonSave_clicked()
{
    XShortcutsWidget::saveModel(ui->tableViewResult->model(), XBinary::getResultFileName(g_pDevice, QString("%1.txt").arg(tr("Extract"))));
}

void XExtractorWidget::on_pushButtonDumpAll_clicked()
{
    QString sDirectory = QFileDialog::getExistingDirectory(this, tr("Dump all"), XBinary::getDeviceDirectory(g_pDevice));

    if (!sDirectory.isEmpty()) {
        qint32 nNumberOfRecords = ui->tableViewResult->model()->rowCount();

        if (nNumberOfRecords) {
            QList<DumpProcess::RECORD> listRecords;

            for (qint32 i = 0; i < nNumberOfRecords; i++) {
                QModelIndex index = ui->tableViewResult->model()->index(i, 0);

                DumpProcess::RECORD record = getDumpProcessRecord(index);

                record.sFileName = sDirectory + QDir::separator() + QFileInfo(record.sFileName).fileName();

                listRecords.append(record);
            }

            DialogDumpProcess dd(this);

            dd.setData(g_pDevice, listRecords, DumpProcess::DT_OFFSET);

            dd.showDialogDelay(1000);
        }
    }
}

void XExtractorWidget::on_tableViewResult_customContextMenuRequested(const QPoint &pos)
{
    int nRow = ui->tableViewResult->currentIndex().row();

    if (nRow != -1) {
        QMenu contextMenu(this);

        QMenu menuFollowIn(tr("Follow in"), this);

        QAction actionDump(tr("Dump to file"), this);
        connect(&actionDump, SIGNAL(triggered()), this, SLOT(dumpToFile()));
        contextMenu.addAction(&actionDump);

        QAction actionHex(tr("Hex"), this);
        // actionHex.setShortcut(getShortcuts()->getShortcut(X_ID_DISASM_FOLLOWIN_HEX));
        connect(&actionHex, SIGNAL(triggered()), this, SLOT(_hexSlot()));

        if (g_options.bMenu_Hex) {
            menuFollowIn.addAction(&actionHex);

            contextMenu.addMenu(&menuFollowIn);
        }

        contextMenu.exec(ui->tableViewResult->viewport()->mapToGlobal(pos));
    }
}

void XExtractorWidget::dumpToFile()
{
    int nRow = ui->tableViewResult->currentIndex().row();

    if (nRow != -1) {
        QModelIndex index = ui->tableViewResult->selectionModel()->selectedIndexes().at(0);

        DumpProcess::RECORD record = getDumpProcessRecord(index);

        record.sFileName = QFileDialog::getSaveFileName(this, tr("Save dump"), record.sFileName);

        if (!record.sFileName.isEmpty()) {
            DialogDumpProcess dd(this);

            dd.setData(g_pDevice, record, DumpProcess::DT_OFFSET);

            dd.showDialogDelay(1000);
        }
    }
}

void XExtractorWidget::_hexSlot()
{
    if (g_options.bMenu_Hex) {
        int nRow = ui->tableViewResult->currentIndex().row();

        if (nRow != -1) {
            QModelIndex index = ui->tableViewResult->selectionModel()->selectedIndexes().at(0);

            DumpProcess::RECORD record = getDumpProcessRecord(index);

            emit showOffsetHex(record.nOffset, record.nSize);
        }
    }
}
