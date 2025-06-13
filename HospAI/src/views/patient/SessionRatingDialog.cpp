#include "SessionRatingDialog.h"
#include <QMessageBox>
#include <QApplication>

SessionRatingDialog::SessionRatingDialog(int sessionId, int patientId, int staffId, 
                                        const QString& staffName, QWidget *parent)
    : QDialog(parent)
    , m_sessionId(sessionId)
    , m_patientId(patientId)
    , m_staffId(staffId)
    , m_staffName(staffName)
    , m_currentRating(0)
    , m_dbManager(DatabaseManager::instance())
{
    setupUI();
    setWindowTitle("会话评价");
    setModal(true);
    resize(450, 350);
}

SessionRatingDialog::~SessionRatingDialog()
{
}

void SessionRatingDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 标题
    m_titleLabel = new QLabel("感谢您使用医院智慧客服系统！", this);
    m_titleLabel->setStyleSheet(R"(
        QLabel {
            font-size: 18px;
            font-weight: bold;
            color: #2C3E50;
            text-align: center;
        }
    )");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    
    // 描述
    m_descLabel = new QLabel(QString("请为 %1 的服务质量进行评价：").arg(m_staffName), this);
    m_descLabel->setStyleSheet(R"(
        QLabel {
            font-size: 14px;
            color: #5D6D7E;
            text-align: center;
        }
    )");
    m_descLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_descLabel);
    
    // 评分区域
    setupRatingButtons();
    
    // 评论区域
    m_commentLabel = new QLabel("您的意见和建议（选填）：", this);
    m_commentLabel->setStyleSheet("font-size: 13px; color: #34495E; font-weight: bold;");
    m_mainLayout->addWidget(m_commentLabel);
    
    m_commentEdit = new QTextEdit(this);
    m_commentEdit->setPlaceholderText("请分享您的使用体验，帮助我们改进服务质量...");
    m_commentEdit->setMaximumHeight(80);
    m_commentEdit->setStyleSheet(R"(
        QTextEdit {
            border: 2px solid #BDC3C7;
            border-radius: 8px;
            padding: 8px;
            font-size: 12px;
            background-color: #FAFBFC;
        }
        QTextEdit:focus {
            border-color: #3498DB;
            background-color: white;
        }
    )");
    m_mainLayout->addWidget(m_commentEdit);
    
    // 按钮区域
    m_buttonLayout = new QHBoxLayout;
    m_buttonLayout->addStretch();
    
    m_skipButton = new QPushButton("跳过", this);
    m_skipButton->setStyleSheet(R"(
        QPushButton {
            background-color: #95A5A6;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #7F8C8D;
        }
        QPushButton:pressed {
            background-color: #6C7B7D;
        }
    )");
    m_buttonLayout->addWidget(m_skipButton);
    
    m_submitButton = new QPushButton("提交评价", this);
    m_submitButton->setStyleSheet(R"(
        QPushButton {
            background-color: #3498DB;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #2980B9;
        }
        QPushButton:pressed {
            background-color: #21618C;
        }
        QPushButton:disabled {
            background-color: #BDC3C7;
            color: #85929E;
        }
    )");
    m_submitButton->setEnabled(false);
    m_buttonLayout->addWidget(m_submitButton);
    
    m_mainLayout->addLayout(m_buttonLayout);
    
    // 连接信号
    connect(m_submitButton, &QPushButton::clicked, this, &SessionRatingDialog::onSubmitRating);
    connect(m_skipButton, &QPushButton::clicked, this, &SessionRatingDialog::onSkipRating);
}

void SessionRatingDialog::setupRatingButtons()
{
    m_ratingGroup = new QGroupBox("服务评分", this);
    m_ratingGroup->setStyleSheet(R"(
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: #34495E;
            border: 2px solid #E8F4FD;
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 10px;
            background-color: #F8FBFF;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");
    
    m_ratingLayout = new QHBoxLayout(m_ratingGroup);
    m_ratingLayout->setSpacing(10);
    m_ratingLayout->addStretch();
    
    // 创建5个星形评分按钮
    m_ratingButtonGroup = new QButtonGroup(this);
    
    for (int i = 1; i <= 5; ++i) {
        QPushButton* starButton = new QPushButton(this);
        starButton->setText("⭐");
        starButton->setCheckable(true);
        starButton->setFixedSize(50, 50);
        starButton->setStyleSheet(R"(
            QPushButton {
                font-size: 24px;
                border: 2px solid #E8F4FD;
                border-radius: 25px;
                background-color: white;
            }
            QPushButton:hover {
                background-color: #FEF9E7;
                border-color: #F39C12;
            }
            QPushButton:checked {
                background-color: #FDF2E9;
                border-color: #E67E22;
                color: #F39C12;
            }
        )");
        
        m_ratingButtonGroup->addButton(starButton, i);
        m_starButtons.append(starButton);
        m_ratingLayout->addWidget(starButton);
    }
    
    m_ratingLayout->addStretch();
    m_mainLayout->addWidget(m_ratingGroup);
    
    // 连接评分变化信号
    connect(m_ratingButtonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked),
            this, [this](QAbstractButton* button) {
                m_currentRating = m_ratingButtonGroup->id(button);
                updateStarDisplay();
                updateSubmitButton();
            });
}

void SessionRatingDialog::updateStarDisplay()
{
    for (int i = 0; i < m_starButtons.size(); ++i) {
        QPushButton* button = m_starButtons[i];
        if (i < m_currentRating) {
            button->setChecked(true);
            button->setText("⭐");
        } else {
            button->setChecked(false);
            button->setText("☆");
        }
    }
}

void SessionRatingDialog::onRatingChanged()
{
    updateSubmitButton();
}

void SessionRatingDialog::updateSubmitButton()
{
    m_submitButton->setEnabled(m_currentRating > 0);
}

void SessionRatingDialog::onSubmitRating()
{
    if (m_currentRating <= 0) {
        QMessageBox::warning(this, "提示", "请先选择评分！");
        return;
    }
    
    QString comment = m_commentEdit->toPlainText().trimmed();
    
    if (m_dbManager->addSessionRating(m_sessionId, m_patientId, m_staffId, m_currentRating, comment)) {
        QMessageBox::information(this, "感谢", "评价提交成功！感谢您的反馈。");
        accept();
    } else {
        QMessageBox::warning(this, "错误", "评价提交失败，请重试。");
    }
}

void SessionRatingDialog::onSkipRating()
{
    int ret = QMessageBox::question(this, "确认", 
                                   "确定要跳过评价吗？您的反馈对我们很重要。", 
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        reject();
    }
}

int SessionRatingDialog::getRating() const
{
    return m_currentRating;
}

QString SessionRatingDialog::getComment() const
{
    return m_commentEdit->toPlainText().trimmed();
}

// MOC generated automatically 