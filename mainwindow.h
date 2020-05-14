#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QtGui>
#include <QtCore>
#include <QTreeWidget>
#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct location
{
    QString process = "";
    QString segment = "";
    int base;
    int limit;
    int End() { return base + limit; }
    int Size() { return  limit; }
};
class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
       QGraphicsScene *scene;

private slots:

    void on_update_clicked();
    void AddRoot(QString name);
    void AddChild( QTreeWidgetItem *parent,QString name );
    void on_done_clicked();
    void on_update_2_clicked();
    void on_reset_clicked();
   void draw( QVector<location> holes , QVector<location> used);
    void on_allocate_clicked();

    void on_deallocate_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
    QTableWidget *tableWidget;

    QGraphicsRectItem *memory;






};
#endif // MAINWINDOW_H
