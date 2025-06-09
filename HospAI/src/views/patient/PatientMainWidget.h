#ifndef PATIENTMAINWIDGET_H
#define PATIENTMAINWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include "ChatWidget.h"
#include "FAQWidget.h"
#include "AppointmentWidget.h"

class PatientMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PatientMainWidget(QWidget *parent = nullptr);

private:
    void setupUI();
    
    QVBoxLayout* m_mainLayout;
    QTabWidget* m_tabWidget;
    
    ChatWidget* m_chatWidget;
    FAQWidget* m_faqWidget;
    AppointmentWidget* m_appointmentWidget;
};

#endif // PATIENTMAINWIDGET_H 