#ifndef _WEATHER_H_
#define _WEATHER_H_

#include <QMainWindow>
#include <QTimer>
#include <QtNetwork/QHttp>
#include <QtCore/QUrl>
#include <QMap>
#include "ui_mainwindow.h"

class weather : public QMainWindow
{
    Q_OBJECT
    public:
    weather();
    ~weather();

    typedef enum
    {
        STATE_LOCAL,
        STATE_NORTH_TEXAS,
        STATE_TEXAS,
        STATE_US,
        STATE_POLLEN,
        STATE_7_DAY,
        STATE_CURRENT_TEMPS,
        STATE_DISABLED,         // Must be next to last
        STATE_LAST
    } HTTP_STATE;

    protected:
    void resizeEvent(QResizeEvent * event);

    public slots:
    void timer_start_download();
    void timer_animate();
    void about();
    void download_complete(bool error);
    void sslErrors ( const QList<QSslError> & errors );

    private:
    void increment_state();

#define MAX_ANIMATE 6
    QString _current_path;
    void update_images();
    void set_pixmap(QLabel *label, int index, QImage const &image);
    QHttp::ConnectionMode _mode;
    HTTP_STATE _state;
    int _state_index;
    int _animate_index;
    int _animate_delay_index;
    QHttp _http;
    QTimer _timer_reload;
    QTimer _timer_animate;
    QImage _images[STATE_LAST][MAX_ANIMATE];
    Ui_MainWindow _mw;
    QMap<HTTP_STATE, QString> _hostnames;
    QMap<HTTP_STATE, QString> _paths;
    QMap<HTTP_STATE, bool> _animate;
};

#endif // _WEATHER_H_
