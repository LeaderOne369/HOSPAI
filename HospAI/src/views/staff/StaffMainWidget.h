#ifndef STAFFMAINWIDGET_H
#define STAFFMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "ConsultationWidget.h"
#include "StatsWidget.h"
#include "KnowledgeBaseWidget.h"

class StaffMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StaffMainWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    ConsultationWidget* m_consultationWidget;
    StatsWidget* m_statsWidget;
    KnowledgeBaseWidget* m_knowledgeBaseWidget;
};

#endif // STAFFMAINWIDGET_H 