#ifndef APPSTYLES_H
#define APPSTYLES_H

#include <QString>

// Namespace для стилей приложения
// Содержит методы для получения стилей главного окна и кнопок
namespace AppStyles {
    // Возвращает стили для главного окна и всех UI элементов
    QString getMainWindowStyles();
    
    // Возвращает стили для конкретного типа кнопки
    // buttonType: "add", "edit", "delete", "complete", "resume", "ok", "cancel" или пустая строка для базовых стилей
    QString getButtonStyles(const QString &buttonType);
    
    // Возвращает стили для диалогов (UserManager, TaskEditor и т.д.)
    QString getDialogStyles();
}

#endif // APPSTYLES_H

