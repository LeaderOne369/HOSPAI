#ifndef SESSIONRATINGDIALOG_H
#define SESSIONRATINGDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QSpacerItem>
#include "../../core/DatabaseManager.h"

class SessionRatingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SessionRatingDialog(int sessionId, int patientId, int staffId, 
                               const QString& staffName, QWidget *parent = nullptr);
    ~SessionRatingDialog();

    int getRating() const;
    QString getComment() const;

private slots:
    void onRatingChanged();
    void onSubmitRating();
    void onSkipRating();

private:
    void setupUI();
    void setupRatingButtons();
    void updateSubmitButton();
    void updateStarDisplay();

    // UI 组件
    QVBoxLayout* m_mainLayout;
    QLabel* m_titleLabel;
    QLabel* m_descLabel;
    
    QGroupBox* m_ratingGroup;
    QHBoxLayout* m_ratingLayout;
    QButtonGroup* m_ratingButtonGroup;
    QList<QPushButton*> m_starButtons;
    
    QLabel* m_commentLabel;
    QTextEdit* m_commentEdit;
    
    QHBoxLayout* m_buttonLayout;
    QPushButton* m_submitButton;
    QPushButton* m_skipButton;
    
    // 数据
    int m_sessionId;
    int m_patientId;
    int m_staffId;
    QString m_staffName;
    int m_currentRating;
    
    DatabaseManager* m_dbManager;
};

#endif // SESSIONRATINGDIALOG_H 