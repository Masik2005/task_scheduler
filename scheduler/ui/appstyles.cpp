#include "appstyles.h"

QString AppStyles::getMainWindowStyles()
{
    return QString(
        "QMainWindow {"
        "    background-color: #f5f5f5;"
        "}"
        
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #d0d0d0;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background-color: white;"
        "}"
        
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    subcontrol-position: top left;"
        "    padding: 0 8px;"
        "    color: #2c3e50;"
        "    font-size: 12pt;"
        "}"
        
        "QLineEdit {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "}"
        
        "QLineEdit:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #ecf0f1;"
        "}"
        
        "QComboBox {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 6px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "    min-width: 120px;"
        "}"
        
        "QComboBox:hover {"
        "    border: 2px solid #3498db;"
        "}"
        
        "QComboBox::drop-down {"
        "    border: none;"
        "    width: 25px;"
        "}"
        
        "QComboBox::down-arrow {"
        "    image: none;"
        "    border-left: 5px solid transparent;"
        "    border-right: 5px solid transparent;"
        "    border-top: 6px solid #34495e;"
        "    margin-right: 5px;"
        "}"
        
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "    min-width: 100px;"
        "}"
        
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        
        "QPushButton:disabled {"
        "    background-color: #bdc3c7;"
        "    color: #7f8c8d;"
        "}"
        
        "QListWidget {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "    padding: 5px;"
        "    font-size: 11pt;"
        "}"
        
        "QListWidget::item {"
        "    border: 1px solid #ecf0f1;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "    margin: 3px;"
        "    background-color: transparent;"
        "}"
        
        "QListWidget::item:hover {"
        "    border: 2px solid #3498db;"
        "}"
        
        "QListWidget::item:selected {"
        "    border: 2px solid #2980b9;"
        "    background-color: rgba(52, 152, 219, 0.5);"
        "}"
        
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 11pt;"
        "}"
        
        "QStatusBar {"
        "    background-color: #34495e;"
        "    color: white;"
        "    font-size: 10pt;"
        "    padding: 5px;"
        "}"
        
        "QStatusBar QLabel {"
        "    color: white;"
        "    background-color: transparent;"
        "}"
        
        "QMenuBar {"
        "    background-color: #34495e;"
        "    color: white;"
        "    font-size: 11pt;"
        "    padding: 5px;"
        "}"
        
        "QMenuBar::item:selected {"
        "    background-color: #3498db;"
        "    border-radius: 3px;"
        "}"
        
        "QMenu {"
        "    background-color: white;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
        
        "QMenu::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border-radius: 3px;"
        "}"
        
        "QToolBar {"
        "    background-color: #ecf0f1;"
        "    border: none;"
        "    spacing: 5px;"
        "    padding: 5px;"
        "}"
        
        "QToolBar::separator {"
        "    background-color: #bdc3c7;"
        "    width: 2px;"
        "    margin: 5px;"
        "}"
    );
}

QString AppStyles::getButtonStyles(const QString &buttonType)
{
    if (buttonType == "add") {
        return QString(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #1e8449;"
            "}"
        );
    } else if (buttonType == "edit") {
        return QString(
            "QPushButton {"
            "    background-color: #f39c12;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e67e22;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #d35400;"
            "}"
        );
    } else if (buttonType == "delete") {
        return QString(
            "QPushButton {"
            "    background-color: #e74c3c;"
            "}"
            "QPushButton:hover {"
            "    background-color: #c0392b;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #a93226;"
            "}"
        );
    } else if (buttonType == "complete") {
        return QString(
            "QPushButton {"
            "    background-color: #9b59b6;"
            "}"
            "QPushButton:hover {"
            "    background-color: #8e44ad;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #7d3c98;"
            "}"
        );
    } else if (buttonType == "resume") {
        return QString(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #1e8449;"
            "}"
        );
    } else if (buttonType == "ok") {
        return QString(
            "QPushButton {"
            "    background-color: #27ae60;"
            "}"
            "QPushButton:hover {"
            "    background-color: #229954;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #1e8449;"
            "}"
        );
    } else if (buttonType == "cancel") {
        return QString(
            "QPushButton {"
            "    background-color: #95a5a6;"
            "}"
            "QPushButton:hover {"
            "    background-color: #7f8c8d;"
            "}"
            "QPushButton:pressed {"
            "    background-color: #6c7a7b;"
            "}"
        );
    }
    return QString();
}

QString AppStyles::getDialogStyles()
{
    return QString(
        "QDialog {"
        "    background-color: #f5f5f5;"
        "}"
        
        "QListWidget {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 8px;"
        "    background-color: white;"
        "    padding: 5px;"
        "    font-size: 11pt;"
        "}"
        
        "QListWidget::item {"
        "    border: 1px solid #ecf0f1;"
        "    border-radius: 5px;"
        "    padding: 10px;"
        "    margin: 3px;"
        "    background-color: white;"
        "}"
        
        "QListWidget::item:hover {"
        "    background-color: #ecf0f1;"
        "    border: 1px solid #3498db;"
        "}"
        
        "QListWidget::item:selected {"
        "    background-color: #3498db;"
        "    color: white;"
        "}"
        
        "QLineEdit, QTextEdit, QDateTimeEdit, QComboBox, QSpinBox {"
        "    border: 2px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    padding: 8px;"
        "    font-size: 11pt;"
        "    background-color: white;"
        "}"
        
        "QLineEdit:focus, QTextEdit:focus, QDateTimeEdit:focus, QComboBox:focus, QSpinBox:focus {"
        "    border: 2px solid #3498db;"
        "    background-color: #ecf0f1;"
        "}"
        
        "QPushButton {"
        "    background-color: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 10px 20px;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "    min-width: 100px;"
        "}"
        
        "QPushButton:hover {"
        "    background-color: #2980b9;"
        "}"
        
        "QPushButton:pressed {"
        "    background-color: #21618c;"
        "}"
        
        "QLabel {"
        "    color: #2c3e50;"
        "    font-size: 11pt;"
        "    font-weight: bold;"
        "}"
    );
}

