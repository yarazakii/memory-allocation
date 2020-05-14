#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGui>
#include<QtCore>
#include <QVector>
#include <QMessageBox>
#include <map>
#include <QtAlgorithms>
#include<QGraphicsItem>
#include <QGraphicsScene>



QVector<location> segments;
QVector<location> holes;
QVector<location> used;

// Compares two locations according to their bases.
bool compareBase(location x, location y)
{
    return (x.base < y.base);
}

// Compares two locations according to their sizes.
bool compareSize(location i1, location i2)
{
    return (i1.Size() < i2.Size());
}

/*************BEST-FIT Memory Allocation*****************/
// Method to assign memory to a block using the best fit memory allocation scheme
bool BestFit(QVector<location> segment, QVector<location> &holes, QVector<location> &used)
{
    //sort the holes according to their size ==> ascendingly
    std::sort(holes.begin(),holes.end(), compareSize);

    //make a copy of holes vector to check if all segments fit
    QVector<location> holes_copy;
    for (int i = 0; i < holes.size(); i++)
    {
        holes_copy.push_back(holes[i]);
    }

    for (int j = 0; j < segment.size(); j++)
    {
        bool flag = 0;

        // find suitable blocks according to its size ad assign to it
        for (int i = 0; i < holes_copy.size(); i++)
        {
            if (holes_copy[i].Size() >= segment[j].Size())
            {
                // Reduce available memory in this block.(adjust the hole base and size)
                holes_copy[i].limit -= segment[j].Size();
                holes_copy[i].base += segment[j].Size();
                flag = 1;
               std:: sort(holes_copy.begin(), holes_copy.end(), compareSize);
                break;
            }

        }
        //the segment doesn't find an appropriate hole
        if (flag == 0)
            return 0;
    }

    for (int j = 0; j < segment.size(); j++)
    {
        for (int i = 0; i < holes.size(); i++)
        {
            if (holes[i].Size() >= segment[j].Size())		//segment is smaller than hole
            {
                segment[j].base = holes[i].base;
                used.push_back(segment[j]);			//put the segment in used vector
                holes[i].base += segment[j].limit;		//adjust hole base & limit
                holes[i].limit -= segment[j].limit;
                //delete the hole if its size becomes equal zero
                if (holes[i].Size() == 0)
                {
                    //pop this hole from the vector of holes by its index
                    holes.erase(holes.begin() + i);
                }
                //re-sort the holes vector
                std::sort(holes.begin(), holes.end(), compareSize);
                break;
            }
        }

    }

    return 1;
}


/***********FIRST-FIT Memory Allocation*************/
bool first_fit(QVector<location> segments, QVector<location> &holes, QVector<location> &used)
{
    //sort holes according to base
    for (int i = 0; i < holes.size(); i++)
    {
        std::sort(holes.begin(), holes.end(), compareBase);
    }

    //make a copy of holes vector to check if all segments fit
    QVector<location> holes_copy;
    for (int i = 0; i < holes.size(); i++)
    {
        holes_copy.push_back(holes[i]);
    }

    //check if all segments fit
    for (int i = 0; i < segments.size(); i++)
    {
        bool fits = 0;
        for (int j = 0; j < holes_copy.size(); j++)
        {
            if (segments[i].limit <= holes_copy[j].limit)
            {
                holes_copy[j].limit -= segments[i].limit;
                fits = 1;
                break;
            }
        }
        if (fits == 0)
            return 0;
    }


    for (int i = 0; i < segments.size(); i++)
    {
        for (int j = 0; j < holes.size(); j++)
        {
            if (segments[i].limit < holes[j].limit)		//segment is smaller than hole
            {
                segments[i].base = holes[j].base;
                used.push_back(segments[i]);			//put the segment in used vector
                holes[j].base += segments[i].limit;		//adjust hole base & limit
                holes[j].limit -= segments[i].limit;
                break;
            }
            else if (segments[i].limit == holes[j].limit)	//segment fits the hole perfectly
            {
                segments[i].base = holes[j].base;
                used.push_back(segments[i]);				//put the segment in used vector
                holes.erase(holes.begin() + j);				//remove hole completely
                break;
            }
        }
    }

    return true;
}
void deallocate(QString process_name, QVector<location> &holes, QVector<location> &used)
{
    int k = 0;
     std::sort(holes.begin(), holes.end(), compareBase);
    for (auto i = used.begin() ; i < used.end() ; i++)
    {
        bool flag = 0;
        location trial = *i;
        if (trial.process == process_name)
        {
            flag = 1;
            bool found = 0;
            //to search for any special cases

            for (int j = 0; j < holes.size(); j++)
            {

                //special case in the holes vector
                if (j<holes.size()-1 && (holes[j].base + holes[j].limit) == trial.base &&
                                    (trial.base + trial.limit) == holes[j+1].base)
                {
                    holes[j].limit += (holes[j + 1].limit + trial.limit);
                    holes.erase(holes.begin() + (j+1));
                    found = 1;
                    break;
                }
                else if ((holes[j].base + holes[j].limit) == trial.base) //hole upwards
                {
                    holes[j].limit += trial.limit;
                    found = 1;
                    break;
                }
                else if ((trial.base + trial.limit) == holes[j].base)  //hole downwards
                {
                    holes[j].base -= trial.limit;
                    holes[j].limit += trial.limit;
                    found = 1;
                    break;
                }
            }

            if(found==0)
                holes.push_back(trial);
        }

        std::sort(holes.begin(), holes.end(), compareBase);
        if (flag==1)
        {
            used.erase(i);
            i--;
        }
        k++;
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->groupBox_2->hide();


    ui->treeWidget->setColumnCount(1);
    ui->treeWidget->setHeaderHidden(true);


     // memory_size= ui->memorysize->value();

}

MainWindow::~MainWindow()
{
    delete ui;
}

QVector <int> holes_base;
QVector <int> holes_limit;
QVector<QString> process_name;
QVector <QString> segment_name;
QVector<int> numberofsegments;
QVector <int> segment_limit;
int numberofholes;
int memory_size;
int allocated_method;
int n=0;


void MainWindow:: AddRoot(QString name)
{
    QTreeWidgetItem *tree= new QTreeWidgetItem(ui->treeWidget);
    tree->setText(0,name);
    for (int i=0;i<segment_name.size();i++){

    AddChild(tree,segment_name[i]);

    }

}
void MainWindow::AddChild(QTreeWidgetItem *parent ,QString name)
{
    QTreeWidgetItem *tree= new QTreeWidgetItem();
    tree->setText(0,name);

    parent->addChild(tree);

}

void MainWindow::on_update_clicked()
{
    //hena 3amalt vector elholes
   // numberofholes++;
    tableWidget = new QTableWidget(this);
   location l;
   l.base=ui->base->value();
   l.limit=ui->limit->value();
   holes.push_back(l);
   memory_size= ui->memorysize->value();
   holes_base.push_back(ui->base->value());
   holes_limit.push_back(ui->limit->value());


   int hole=holes_base.back()+holes_limit.back();
if (holes_base.back() > memory_size | hole> memory_size)
{
 QMessageBox::warning(this , "Can't enter this holes", "please change hole base or limit ");
 holes_base.pop_back();
 holes_limit.pop_back();
 holes.pop_back();
}
else
{
    int row= ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row,0, new QTableWidgetItem(QString::number(numberofholes)));
    ui->tableWidget->setItem(row,1, new QTableWidgetItem(QString::number(holes_base.back())));
    ui->tableWidget->setItem(row,2, new QTableWidgetItem(QString::number(holes_limit.back())));
       numberofholes++;
}
}



void MainWindow::on_update_2_clicked()
{
    //hena 3amalt vector elsegments

    location l;
    l.segment=ui->segmentname->text();
    l.limit=ui->segmentlimit->value();
    l.process=ui->processname->text();
    segments.push_back(l);

    segment_name.push_back(ui->segmentname->text());
    segment_limit.push_back(ui->segmentlimit->value());
    process_name.push_back(ui->processname->text());
    numberofsegments.push_back(ui->numberofsegments->value());

    ui->segmentname->clear();
    ui->segmentlimit->clear();

    n++;
    if (n==ui->numberofsegments->value())
    {
        //allocate

        allocated_method=ui->comboBox->currentIndex();
        QString currentprocess=ui->processname->text();
        QVector<location> temp;
        for (int i=0;i< segments.size();i++)
        {
            if (currentprocess==segments[i].process)
            {
                temp.push_back(segments[i]);
            }
        }
        if (allocated_method==0)
        {

            if( first_fit(temp,holes,used))
            {
                draw(holes,used);
                QMessageBox::information(this,"info","Process is allocated");
            }
            else {
                QMessageBox::warning(this,"warning","Process doesn't fit");
            }

        }
        else
        {
            if(BestFit(temp,holes,used))
            {
                draw(holes,used);
                QMessageBox::information(this,"info","Process is allocated");
            }
            else {
                QMessageBox::warning(this,"warning","Process doesn't fit");
            }

        }

        ui->tableWidget_2->setRowCount(0);
        int flag=0;
        for (int i=0;i<used.size();i++)
        {
            if(used[i].process==currentprocess)
            {
                flag=1;
                int row= ui->tableWidget_2->rowCount();
                ui->tableWidget_2->insertRow(row);
                ui->tableWidget_2->setItem(row,0, new QTableWidgetItem(used[i].segment));
                ui->tableWidget_2->setItem(row,1, new QTableWidgetItem(QString::number(used[i].limit)));
                ui->tableWidget_2->setItem(row,2, new QTableWidgetItem(QString::number(used[i].base)));
            }
        }
        if (flag==0)
        {
            int row= ui->tableWidget_2->rowCount();
            ui->tableWidget_2->insertRow(row);
            ui->tableWidget_2->setItem(row,0, new QTableWidgetItem("Process"));
            ui->tableWidget_2->setItem(row,1, new QTableWidgetItem("is"));
            ui->tableWidget_2->setItem(row,2, new QTableWidgetItem("deallocated"));
        }

        //QMessageBox::information(this,"title","Thanks, please enter next Process");
        ui->processname->clear();
        ui->numberofsegments->clear();
        AddRoot(process_name.back());
        segment_name.erase(segment_name.begin(),segment_name.end());
        n=0;
    }


}

void MainWindow:: draw( QVector<location> holes , QVector<location> used)
{
    int height=650;
    float factor=height/memory_size;
    scene= new QGraphicsScene();
    ui->graphicsView->setScene(scene);
    QBrush redbrush (Qt::red);
    QBrush bluebrush(Qt::blue);
    QBrush yellow (Qt::yellow);
    QPen pen(Qt::black);
    //pen.setWidth(2);
    //scene->addRect(0,0,200,height,pen,redbrush);
    //memory->setFlag(QGraphicsItem::ItemIsMovable);
    QGraphicsItem *start=scene->addText(QString::number(0));
    start->setPos(-30,-10);

    QGraphicsItem *memorysize=scene->addText(QString::number(memory_size));
    memorysize->setPos(-30,memory_size*factor-10);

for (int i=0;i<holes.size();i++)
{
    scene->addRect(0,holes[i].base*factor,200,holes[i].Size()*factor,pen,bluebrush);
    QGraphicsItem *start1=scene->addText(QString::number(holes[i].base));
    start1->setPos(-30,holes[i].base*factor-10);
    //QGraphicsItem *end=scene->addText(QString::number(holes[i].End()));
    //end->setPos(-30,holes[i].End()*factor);

}

for (int i=0;i<used.size();i++)
{
    if(used[i].process.startsWith("used space"))
    {
        scene->addRect(0,used[i].base*factor,200,used[i].Size()*factor,pen,redbrush);
        QGraphicsItem *start1=scene->addText(QString::number(used[i].base));
        start1->setPos(-30,used[i].base*factor-10);

        //QGraphicsItem *end=scene->addText(QString::number(used[i].End()));
        //end->setPos(-30,used[i].End()*factor);

        QGraphicsItem *name=scene->addText(used[i].process);
        name->setPos(60,((used[i].base+used[i].End())/2)*factor-10);
    }
    else
    {
        scene->addRect(0,used[i].base*factor,200,used[i].Size()*factor,pen,yellow);
        QGraphicsItem *start1=scene->addText(QString::number(used[i].base));
        start1->setPos(-30,used[i].base*factor-10);

        //QGraphicsItem *end=scene->addText(QString::number(used[i].End()));
        //end->setPos(-30,used[i].End()*factor);

        QGraphicsItem *name=scene->addText(used[i].segment+" of "+used[i].process);
        name->setPos(60,((used[i].base+used[i].End())/2)*factor-10);
    }
}
QGraphicsTextItem *text1 = scene->addText("holes");
text1->setPos(75 , memory_size*factor + 7);
scene->addRect(120 ,memory_size*factor + 15,15,10,pen,bluebrush);
QGraphicsTextItem *text2 = scene->addText("used spaces");
text2->setPos(35 , memory_size*factor + 22);
scene->addRect(120 ,memory_size*factor + 30,15,10,pen,redbrush);
QGraphicsTextItem *text3 = scene->addText("segments");
text3->setPos(50 , memory_size*factor + 37);
scene->addRect(120 ,memory_size*factor + 45,15,10,pen,yellow);
}


void MainWindow::on_done_clicked()
{
       memory_size= ui->memorysize->value();
    if (numberofholes==0)
    {
        QMessageBox::information(this,"title","please,enter at least one hole");
        return;
    }


    ui->tableWidget->setRowCount(0);
    ui->memorysize->clear();
    ui->base->clear();
    ui->limit->clear();
    ui->groupholes->hide();
    ui->groupBox_2->show();

  //  memory_size= ui->memorysize->value();

    //hena di 7esbet elused w hatethom f vector used

    int alloc_base=0;
    int x=1;
    for (int i=0;i<holes.size();i++)
    {
        if(i==0 && holes[i].base==0)
        {
            alloc_base=holes[i].End();
        }
        else
        {
            location l;
            l.base=alloc_base;
            l.limit=holes[i].base-l.base;
            l.process="used space "+QString::number(x);
            used.push_back(l);
            alloc_base=holes[i].End();
            x++;
         }
    }
    if(holes[holes.size()-1].End()!=memory_size)
    {
        location l;
        l.base=holes[holes.size()-1].End();
        l.limit=memory_size - holes[holes.size()-1].End();
        l.process="used space"+QString::number(x);
        used.push_back(l);
    }

    for (int i=0;i<used.size();i++)
    {
       AddRoot(used[i].process);
    }

//    scene= new QGraphicsScene();
//    ui->graphicsView->setScene(scene);
//    QBrush redbrush (Qt::red);
//    QBrush bluebrush(Qt::blue);
//    QPen pen(Qt::black);
//    pen.setWidth(2);
//    scene->addRect(0,0,200,memory_size,pen,redbrush);
//    //memory->setFlag(QGraphicsItem::ItemIsMovable);
//    QGraphicsItem *start=scene->addText(QString::number(0));
//    start->setPos(-30,0);

//    QGraphicsItem *memorysize=scene->addText(QString::number(memory_size));
//    memorysize->setPos(-30,memory_size);

//for (int i=0;i<numberofholes;i++)
//{
//    scene->addRect(0,holes[i].base,200,holes[i].Size(),pen,bluebrush);
//    QGraphicsItem *start1=scene->addText(QString::number(holes[i].base));
//    start1->setPos(-30,holes[i].base);
//    QGraphicsItem *end=scene->addText(QString::number(holes[i].End()));
//    end->setPos(-30,holes[i].End());

//}
draw(holes,used);


}




void MainWindow::on_reset_clicked()
{
    numberofholes=0;


   ui->processname->clear();
   ui->numberofsegments->clear();
   ui->segmentname->clear();
   ui->segmentlimit->clear();
   ui->tableWidget->setRowCount(0);
   ui->tableWidget_2->setRowCount(0);
  ui->memorysize->clear();
  ui->base->clear();
  ui->limit->clear();
  numberofholes = 0;
  memory_size = 0;
  allocated_method= 0;
  n = 0;
  //clearing vectors
   holes.erase(holes.begin(),holes.end());
   used.erase(used.begin(),used.end());
   segments.erase(segments.begin(),segments.end());
   process_name.erase(process_name.begin(),process_name.end());
   numberofsegments.erase(numberofsegments.begin(),numberofsegments.end());
   segment_name.erase(segment_name.begin(),segment_name.end());
   segment_limit.erase(segment_limit.begin(),segment_limit.end());
   holes_base.erase(holes_base.begin(),holes_base.end());
   holes_limit.erase(holes_limit.begin(),holes_limit.end());
// clearing graphics view
   scene = new QGraphicsScene(this);
   ui->graphicsView->setScene(scene);
   scene->clear();
// clear treewidget
    ui->treeWidget->clear();
    ui->groupBox_2->hide();
    ui->groupholes->show();


}


void MainWindow::on_allocate_clicked()
{
    allocated_method=ui->comboBox->currentIndex();
    QString currentprocess=ui->treeWidget->currentItem()->text(0);
    QVector<location> temp;
    for (int i=0;i< segments.size();i++)
    {
        if (currentprocess==segments[i].process)
        {
            temp.push_back(segments[i]);
        }
    }
    if (allocated_method==0)
    {

        if( first_fit(temp,holes,used))
        {
            draw(holes,used);
        }
        else {
            QMessageBox::warning(this,"warning","Process doesn't fit");
        }

    }
    else
    {
        if(BestFit(temp,holes,used))
        {
            draw(holes,used);
        }
        else {
            QMessageBox::warning(this,"warning","Process doesn't fit");
        }

    }

    ui->tableWidget_2->setRowCount(0);
    int flag=0;
    for (int i=0;i<used.size();i++)
    {
        if(used[i].process==currentprocess)
        {
            flag=1;
            int row= ui->tableWidget_2->rowCount();
            ui->tableWidget_2->insertRow(row);
            ui->tableWidget_2->setItem(row,0, new QTableWidgetItem(used[i].segment));
            ui->tableWidget_2->setItem(row,1, new QTableWidgetItem(QString::number(used[i].limit)));
            ui->tableWidget_2->setItem(row,2, new QTableWidgetItem(QString::number(used[i].base)));
        }
    }
    if (flag==0)
    {
        int row= ui->tableWidget_2->rowCount();
        ui->tableWidget_2->insertRow(row);
        ui->tableWidget_2->setItem(row,0, new QTableWidgetItem("Process"));
        ui->tableWidget_2->setItem(row,1, new QTableWidgetItem("is"));
        ui->tableWidget_2->setItem(row,2, new QTableWidgetItem("deallocated"));
    }
}

void MainWindow::on_deallocate_clicked()
{

    QString currentprocess=ui->treeWidget->currentItem()->text(0);
    deallocate(currentprocess,holes,used);
    draw(holes,used);
    ui->tableWidget_2->setRowCount(0);
    int flag=0;
    for (int i=0;i<used.size();i++)
    {
        if(used[i].process==currentprocess)
        {
            flag=1;
            int row= ui->tableWidget_2->rowCount();
            ui->tableWidget_2->insertRow(row);
            ui->tableWidget_2->setItem(row,0, new QTableWidgetItem(used[i].segment));
            ui->tableWidget_2->setItem(row,1, new QTableWidgetItem(QString::number(used[i].limit)));
            ui->tableWidget_2->setItem(row,2, new QTableWidgetItem(QString::number(used[i].base)));
        }
    }
    if (flag==0)
    {
        int row= ui->tableWidget_2->rowCount();
        ui->tableWidget_2->insertRow(row);
        ui->tableWidget_2->setItem(row,0, new QTableWidgetItem("Process"));
        ui->tableWidget_2->setItem(row,1, new QTableWidgetItem("is"));
        ui->tableWidget_2->setItem(row,2, new QTableWidgetItem("deallocated"));
    }
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    QString process=item->text(0);
    ui->tableWidget_2->setRowCount(0);
    int flag=0;
    for (int i=0;i<used.size();i++)
    {
        if(used[i].process==process){
            flag=1;
            int row= ui->tableWidget_2->rowCount();
            ui->tableWidget_2->insertRow(row);
            ui->tableWidget_2->setItem(row,0, new QTableWidgetItem(used[i].segment));
            ui->tableWidget_2->setItem(row,1, new QTableWidgetItem(QString::number(used[i].limit)));
            ui->tableWidget_2->setItem(row,2, new QTableWidgetItem(QString::number(used[i].base)));
        }
    }
    if (flag==0)
    {
        int row= ui->tableWidget_2->rowCount();
        ui->tableWidget_2->insertRow(row);
        ui->tableWidget_2->setItem(row,0, new QTableWidgetItem("Process"));
        ui->tableWidget_2->setItem(row,1, new QTableWidgetItem("is"));
        ui->tableWidget_2->setItem(row,2, new QTableWidgetItem("deallocated"));
    }
}
