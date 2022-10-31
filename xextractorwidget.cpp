/* Copyright (c) 2022 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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

XExtractorWidget::XExtractorWidget(QWidget *pParent) :
    XShortcutsWidget(pParent),
    ui(new Ui::XExtractorWidget)
{
    ui->setupUi(this);

    g_pDevice=nullptr;
    g_options={};
}

XExtractorWidget::~XExtractorWidget()
{
    delete ui;
}

void XExtractorWidget::setData(QIODevice *pDevice,XExtractor::OPTIONS options,bool bAuto)
{
    g_pDevice=pDevice;
    g_options=options;

    if(bAuto)
    {
        reload();
    }
}

void XExtractorWidget::reload()
{
    XExtractor::DATA extractor_data={};

    extractor_data.options=g_options;

    DialogExtractorProcess dep(XOptions::getMainWidget(this),g_pDevice,&extractor_data);

    dep.showDialogDelay(1000);

    if(dep.isSuccess())
    {
        ui->tableWidgetResult->clear();

        qint32 nNumberOfRecords=extractor_data.listRecords.count();

        QStringList slHeader;
        slHeader.append(tr("Offset"));
        slHeader.append(tr("Size"));
        slHeader.append(tr("Type"));
        slHeader.append(QString(""));
        slHeader.append(QString(""));

        ui->tableWidgetResult->setRowCount(nNumberOfRecords);
        ui->tableWidgetResult->setColumnCount(slHeader.size());

        ui->tableWidgetResult->setHorizontalHeaderLabels(slHeader);
        ui->tableWidgetResult->horizontalHeader()->setVisible(true);

        for(qint32 i=0;i<nNumberOfRecords;i++)
        {
            {
                QTableWidgetItem *pItem=new QTableWidgetItem;

                pItem->setText(XBinary::valueToHexEx(extractor_data.listRecords.at(i).nOffset));

                ui->tableWidgetResult->setItem(i,0,pItem);
            }
            {
                QTableWidgetItem *pItem=new QTableWidgetItem;

                pItem->setText(XBinary::valueToHexEx(extractor_data.listRecords.at(i).nSize));

                ui->tableWidgetResult->setItem(i,1,pItem);
            }
            {
                QTableWidgetItem *pItem=new QTableWidgetItem;

                pItem->setText(XBinary::fileTypeIdToString(extractor_data.listRecords.at(i).fileType));

                ui->tableWidgetResult->setItem(i,2,pItem);
            }
            {
                QTableWidgetItem *pItem=new QTableWidgetItem;

                pItem->setText(extractor_data.listRecords.at(i).sString);

                ui->tableWidgetResult->setItem(i,3,pItem);
            }
            {
                QPushButton *result=new QPushButton(this); // TODO remove

                result->setText(tr("Dump"));
                result->setProperty("OFFSET",extractor_data.listRecords.at(i).nOffset);
                result->setProperty("SIZE",extractor_data.listRecords.at(i).nSize);

                connect(result,SIGNAL(clicked()),this,SLOT(pushButtonAction()));

                ui->tableWidgetResult->setCellWidget(i,4,result);
            }
        }

        ui->tableWidgetResult->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Interactive);
        ui->tableWidgetResult->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);
        ui->tableWidgetResult->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Interactive);
        ui->tableWidgetResult->horizontalHeader()->setSectionResizeMode(3,QHeaderView::Stretch);
        ui->tableWidgetResult->horizontalHeader()->setSectionResizeMode(4,QHeaderView::Interactive);
    }
}

void XExtractorWidget::pushButtonAction()
{
    QWidget *pWidget=qobject_cast<QWidget *>(sender());
    qint64 nOffset=pWidget->property("OFFSET").toLongLong();
    qint64 nSize=pWidget->property("SIZE").toLongLong();

    QString sName=QString("%1_%2").arg(XBinary::valueToHexEx(nOffset),XBinary::valueToHexEx(nSize));

    QString sSaveFileName=XBinary::getResultFileName(g_pDevice,QString("%1.bin").arg(sName)); // TODO FileType to ext
    QString sFileName=QFileDialog::getSaveFileName(this,tr("Save dump"),sSaveFileName,QString("%1 (*.bin)").arg(tr("Raw data")));

    if(!sFileName.isEmpty())
    {
        DialogDumpProcess dd(this,g_pDevice,nOffset,nSize,sFileName,DumpProcess::DT_OFFSET);

        dd.showDialogDelay(1000);
    }
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
    XShortcutsWidget::saveModel(ui->tableWidgetResult->model(),XBinary::getResultFileName(g_pDevice,QString("%1.txt").arg(tr("Extract"))));
}

void XExtractorWidget::on_pushButtonDumpAll_clicked()
{
    // TODO
}
