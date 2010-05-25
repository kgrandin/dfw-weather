
#include <QFile>
#include <QTime>
#include "qdebug.h"
#include "weather.h"
#include <QMessageBox>

#define ARRAY_SIZE(a)   (sizeof(a)/sizeof((a)[0]))

char const * g_combo_selection[] =
{
    // NOTE: Must match weather::HTTP_STATE
    "Fort Worth",       // STATE_LOCAL,
    "North Texas",      // STATE_NORTH_TEXAS,
    "Texas",            // STATE_TEXAS,
    "US",               // STATE_US,
    "Pollen",           // STATE_POLLEN,
    "7 Day Forecast",   // STATE_7_DAY,
    "Current Temps",    // STATE_CURRENT_TEMPS,
    "Disabled",         // STATE_DISABLED,
};

weather::weather(): QMainWindow(), _http(this), _timer_reload(this), _timer_animate(this)
{
    _animate_index = 0;
    _animate_delay_index = 2;

    _mw.setupUi(this);

    // Default to use SSL for connection.
    _mode = QHttp::ConnectionModeHttps;
    //_mode = QHttp::ConnectionModeHttp;

    for (unsigned int i = 0; i < ARRAY_SIZE(g_combo_selection); i++)
    {
        _mw.comboBox_left->addItem(g_combo_selection[i]);
        _mw.comboBox_right->addItem(g_combo_selection[i]);
        _mw.comboBox_bottom_left->addItem(g_combo_selection[i]);
        _mw.comboBox_bottom_right->addItem(g_combo_selection[i]);
        _mw.comboBox_bottom_middle->addItem(g_combo_selection[i]);
    }
    _mw.comboBox_left->setCurrentIndex(STATE_LOCAL);
    _mw.comboBox_right->setCurrentIndex(STATE_TEXAS);
    _mw.comboBox_bottom_left->setCurrentIndex(STATE_POLLEN);
    _mw.comboBox_bottom_right->setCurrentIndex(STATE_7_DAY);
    _mw.comboBox_bottom_middle->setCurrentIndex(STATE_CURRENT_TEMPS);

    _hostnames[STATE_LOCAL] = "weather.myfoxdfw.com";
    _paths[STATE_LOCAL] = "/maps/KDFW/metro/radar/d640x480/xm0.jpeg";
    _animate[STATE_LOCAL] = true;

    _hostnames[STATE_NORTH_TEXAS] = "weather.myfoxdfw.com";
    _paths[STATE_NORTH_TEXAS] = "/maps/KDFW/regional/radar/d640x480/m0_wdt.jpeg";
    _animate[STATE_NORTH_TEXAS] = true;

    _hostnames[STATE_US] = "weather.myfoxdfw.com";
    _paths[STATE_US] = "/maps/sector/us-national/radar/d640x480/m0_wdt.jpeg";
    _animate[STATE_US] = true;

    _hostnames[STATE_TEXAS] = "weather.myfoxdfw.com";
    _paths[STATE_TEXAS] = "/maps/sector/us-southcentral/radar/d640x480/m0_wdt.jpeg";
    _animate[STATE_TEXAS] = true;

    _hostnames[STATE_POLLEN] = "media2.myfoxdfw.com";
    _paths[STATE_POLLEN] = "/weather/web_health.jpg";
    _animate[STATE_POLLEN] = false;

    _hostnames[STATE_7_DAY] = "media2.myfoxdfw.com";
    _paths[STATE_7_DAY] = "/weather/web_7_day.jpg";
    _animate[STATE_7_DAY] = false;
 
    _hostnames[STATE_CURRENT_TEMPS] = "media2.myfoxdfw.com";
    _paths[STATE_CURRENT_TEMPS] = "/weather/web_adi_temps.jpg";
    _animate[STATE_CURRENT_TEMPS] = false;
 
    connect(&_http, SIGNAL(done(bool)), this, SLOT(download_complete(bool)));
    connect(&_timer_reload, SIGNAL(timeout()), this, SLOT(timer_start_download()));
    connect(_mw.actionAbout, SIGNAL(activated()), this, SLOT(about()));
    connect(_mw.actionRefresh, SIGNAL(activated()), this, SLOT(refresh()));
    connect(&_timer_animate, SIGNAL(timeout()), this, SLOT(timer_animate()));
    connect(&_http, SIGNAL(sslErrors ( const QList<QSslError> & )), 
            this, SLOT(sslErrors ( const QList<QSslError> & )));

    _timer_animate.start(500);

    timer_start_download();

    // Refresh every 10 minutes.
    _timer_reload.start(10 * 60 * 1000);
}

weather::~weather()
{
}

void weather::refresh()
{
    timer_start_download();
}

void weather::sslErrors ( const QList<QSslError> & errors )
{
    (void) errors;
    _http.ignoreSslErrors();
}

void weather::timer_animate()
{
    //qDebug() << "animate " << _animate_index;
    set_pixmap(_mw.image_left, _mw.comboBox_left->currentIndex(), 
            _images[_mw.comboBox_left->currentIndex()][_animate_index]);
    set_pixmap(_mw.image_right, _mw.comboBox_right->currentIndex(), 
            _images[_mw.comboBox_right->currentIndex()][_animate_index]);
    set_pixmap(_mw.image_bottom_left, _mw.comboBox_bottom_left->currentIndex(), 
            _images[_mw.comboBox_bottom_left->currentIndex()][_animate_index]);
    set_pixmap(_mw.image_bottom_right, _mw.comboBox_bottom_right->currentIndex(), 
            _images[_mw.comboBox_bottom_right->currentIndex()][_animate_index]);
    set_pixmap(_mw.image_bottom_middle, _mw.comboBox_bottom_middle->currentIndex(), 
            _images[_mw.comboBox_bottom_middle->currentIndex()][_animate_index]);

    if (_animate_index == 0)
    {
        if (_animate_delay_index <= 0)
        {
            _animate_index = MAX_ANIMATE - 1;
        }
        _animate_delay_index--;
    } else
    {
        _animate_delay_index = 2;
        _animate_index--;
    }
}

void weather::timer_start_download()
{
    _http.abort();

    _state = STATE_LOCAL;
    _state_index = 0;

    _mw.statusbar->showMessage(QString("Loading: ") + g_combo_selection[_state]);

    _http.setHost(_hostnames[_state], _mode);
    _current_path = _paths[_state];
    _http.get(_current_path);
}

void weather::resizeEvent(QResizeEvent * event)
{
    (void) event;
    update_images();
}

void weather::set_pixmap(QLabel *label, int index, QImage const &image)
{
    if (index == STATE_DISABLED)
    {
        label->setVisible(false);
    } else
    {
        label->setVisible(true);
    }

    if (image.height() > 0)
    {
        label->setPixmap(QPixmap::fromImage(
                    image.scaled(label->width(), label->height(), Qt::KeepAspectRatio), 
                    Qt::AutoColor));
    }
}

void weather::update_images()
{
}

void weather::download_complete(bool error)
{
    QByteArray data = _http.readAll();
    QImage image;

    if (error)
    {
        qDebug() << "Error:" << _http.errorString();
    }

    //qDebug() << "Load: " << _current_path << ":" << _state << ":" << _state_index << ":" << data.length();
    image.loadFromData(data);
    _images[_state][_state_index] = image;

    //QFile file("out.dat");
    //file.open(QIODevice::WriteOnly);
    //file.write(data);
    //file.close();

    if (_animate[_state])
    {
        if (_state_index + 1 >= MAX_ANIMATE)
        {
            increment_state();
            _state_index = 0;
            _current_path = _paths[_state];
        } else
        {
            QString new_str;

            _state_index++;
            if (_current_path.contains(QRegExp("[0-9].jpeg$")))
            {
                new_str.sprintf("%d.jpeg", _state_index);
                _current_path.replace(QRegExp("[0-9].jpeg$"), new_str);
            } else
            {
                new_str.sprintf("m%d_wdt.jpeg", _state_index);
                _current_path.replace(QRegExp("m[0-9]_wdt.jpeg$"), new_str);
            }
        }
    } else
    {
        increment_state();
        _state_index = 0;

        _current_path = _paths[_state];
    }

    if ((STATE_DISABLED != _state) && 
            (STATE_LAST != _state))
    {
        _http.setHost(_hostnames[_state], _mode);
        _http.get(_current_path);
        _mw.statusbar->showMessage(QString("Loading: ") + g_combo_selection[_state]);
    } else
    {
        QTime current_time = QTime::currentTime();
        QDate current_date = QDate::currentDate();

        _mw.statusbar->showMessage(QString("Load complete: ") + current_time.toString("hh:mm ap") + 
            " (" + current_date.toString("dddd MMMM d") + ")");
        _state = STATE_LOCAL;
        _state_index = 0;
    }

    update_images();
}

void weather::about()
{
    QMessageBox::information(this, 
        "Fort Worth Weather",
        "Fort Worth, Texas weather display\n"
        "License: GPL\n"
        "Author: Keith Grandin");
}

void weather::increment_state()
{
    _state = static_cast<HTTP_STATE>(static_cast<int>(_state) + 1);
    if (_state > STATE_LAST)
    {
        _state = STATE_LAST;
        qDebug() << "state overflow " << _state;
    }
}
