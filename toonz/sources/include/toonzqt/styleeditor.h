

#ifndef STYLEEDITOR_H
#define STYLEEDITOR_H

// TnzCore includes
#include "tcommon.h"
#include "tfilepath.h"
#include "tpixel.h"
#include "tpalette.h"

// TnzLib includes
#include "toonz/tpalettehandle.h"
#include "toonz/txshlevelhandle.h"
#include "toonz/txshlevel.h"
//#include "toonz/preferences.h" //iwsw commented out temporarily

// TnzQt includes
#include "toonzqt/checkbox.h"
#include "toonzqt/intfield.h"
#include "toonzqt/doublefield.h"
#include "toonzqt/colorfield.h"
#include "toonzqt/tabbar.h"

//Toonz includes
//#include "../toonz/tapp.h" //iwsw commented out temporarily

// Qt includes
#include <QWidget>
#include <QFrame>
#include <QTabBar>
#include <QSlider>
#include <QToolButton>
#include <QScrollArea>
#include <QGLWidget>
#include <QMouseEvent>
#include <QPointF>

//iwsw commented out temporarily
//#include "ghibli_3dlut_util.h"

#undef DVAPI
#undef DVVAR
#ifdef TOONZQT_EXPORTS
#define DVAPI DV_EXPORT_API
#define DVVAR DV_EXPORT_VAR
#else
#define DVAPI DV_IMPORT_API
#define DVVAR DV_IMPORT_VAR
#endif

//=============================================

//    Forward declarations

class TColorStyle;
class TPalette;

class TXshLevelHandle;
class PaletteController;

class QGridLayout;
class QLabel;
class QStackedWidget;
class QSlider;
class QRadioButton;
class QButtonGroup;
class QPushButton;
class QTabWidget;
class QToolBar;

class ColorSquaredWheel;
class TabBarContainter;
class StyleChooser;
class StyleEditor;

//=============================================

using namespace DVGui;

//=============================================================================
namespace StyleEditorGUI
{
//=============================================================================

enum ColorChannel {
	eRed = 0,
	eGreen,
	eBlue,
	eAlpha,
	eHue,
	eSaturation,
	eValue
};

//=============================================================================
/*! \brief The ColorModel provides an object to manage color change and
		its transformation from rgb value to hsv value and vice versa.

		This object change color using its rgb channel or its hsv channel;
		if you change a color channel class assure you that other channel not change.
*/
class DVAPI ColorModel
{
	int m_channels[7];
	void rgb2hsv();
	void hsv2rgb();

public:
	ColorModel();

	void setTPixel(const TPixel32 &color);
	TPixel32 getTPixel() const;

	void setValue(ColorChannel channel, int value);
	void setValues(ColorChannel channel, int u, int v);
	int getValue(ColorChannel channel) const;
	void getValues(ColorChannel channel, int &u, int &v);

	inline int r() const { return m_channels[0]; }
	inline int g() const { return m_channels[1]; }
	inline int b() const { return m_channels[2]; }
	inline int a() const { return m_channels[3]; }
	inline int h() const { return m_channels[4]; }
	inline int s() const { return m_channels[5]; }
	inline int v() const { return m_channels[6]; }

	bool operator==(const ColorModel &cm)
	{
		int i;
		for (i = 0; i < 7; i++)
			if (m_channels[i] != cm.getValue(ColorChannel(i)))
				return false;
		return true;
	}
};

//=============================================

enum CurrentWheel {
	none,
	leftWheel,
	rightTriangle
};

class DVAPI HexagonalColorWheel : public QGLWidget
{
	Q_OBJECT

	// backgoround color (R160, G160, B160)
	QColor m_bgColor;
	Q_PROPERTY(QColor BGColor READ getBGColor WRITE setBGColor)

	ColorModel m_color;
	QPointF m_wheelPosition;
	float m_triEdgeLen;
	float m_triHeight;
	QPointF m_wp[7], m_leftp[3];

	CurrentWheel m_currentWheel;

	//iwsw commented out temporarily  about 3DLUT
	//Ghibli3DLutUtil * m_ghibli3DLutUtil;

private:
	void drawCurrentColorMark();
	void clickLeftWheel(const QPoint &pos);
	void clickRightTriangle(const QPoint &pos);

public:
	HexagonalColorWheel(QWidget *parent);
	void setColor(const ColorModel &color) { m_color = color; };

	~HexagonalColorWheel();

	void setBGColor(const QColor &color) { m_bgColor = color; }
	QColor getBGColor() const { return m_bgColor; }

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	QSize SizeHint() const
	{
		return QSize(300, 200);
	};

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

signals:
	void colorChanged(const ColorModel &color, bool isDragging);
};

//=============================================================================
/*! \brief The SquaredColorWheel is a squared color to change color.

		Inherits \b QWidget.

		This object show a square faded from one color channel to another color channel,
		the two channel represent x and y axis of square.
		It's possible to choose viewed shade using \b setChannel(). 
		Click in square change current SquaredColorWheel.
*/
class DVAPI SquaredColorWheel : public QWidget
{
	Q_OBJECT
	ColorChannel m_channel;
	ColorModel m_color;

public:
	SquaredColorWheel(QWidget *parent);

	/*! Doesn't call update(). */
	void setColor(const ColorModel &color);

protected:
	void paintEvent(QPaintEvent *event);

	void click(const QPoint &pos);
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

public slots:
	/*! Connect channels to the two square axes:
			\li eRed : connect x-axis to eGreen and y-axis to eBlue;
			\li eGreen : connect x-axis to eRed and y-axis to eBlue;
			\li eBlue : connect x-axis to eRed and y-axis to eGreen;
			\li eHue : connect x-axis to eSaturation and y-axis to eValue;
			\li eSaturation : connect x-axis to eHue and y-axis to eValue;
			\li eValue : connect x-axis to eHue and y-axis to eSaturation; */
	void setChannel(int channel);

signals:
	void colorChanged(const ColorModel &color, bool isDragging);
};

//=============================================================================
/*! \brief The ColorSlider is used to set a color channel.

		Inherits \b QSlider.

		This object show a bar which colors differ from minimum to maximum channel color
		value.
*/
class DVAPI ColorSlider : public QSlider
{
	Q_OBJECT
public:
	ColorSlider(Qt::Orientation orientation, QWidget *parent = 0);

	/*! set channel and color. doesn't call update(). */
	void setChannel(ColorChannel channel);
	void setColor(const ColorModel &color);

	ColorChannel getChannel() const { return m_channel; }

protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

	//	QIcon getFirstArrowIcon();
	//	QIcon getLastArrowIcon();
	//	QRect getFirstArrowRect();
	//	QRect getLastArrowRect();

private:
	ColorChannel m_channel;
	ColorModel m_color;
};

//=============================================================================
// ArrowButton

class ArrowButton : public QToolButton
{
	Q_OBJECT

	Qt::Orientation m_orientaion;
	bool m_isFirstArrow;

	int m_timerId;
	int m_firstTimerId;

public:
	ArrowButton(QWidget *parent = 0, Qt::Orientation orientation = Qt::Horizontal, bool m_isFirstArrow = true);

protected:
	void stopTime(int timerId);
	void timerEvent(QTimerEvent *event);
	void notifyChanged();

protected slots:
	void onPressed();
	void onRelease();

signals:
	void add();
	void remove();
};

//=============================================================================
/*! \brief The ColorSliderBar is a colorSlider with  two arrow to add or remove one to current value.

		Inherits \b QToolBar.
*/
class DVAPI ColorSliderBar : public QWidget
{
	Q_OBJECT

	ColorSlider *m_colorSlider;

public:
	ColorSliderBar(QWidget *parent = 0, Qt::Orientation orientation = Qt::Vertical);

	void setValue(int value) { m_colorSlider->setValue(value); }
	void setRange(int minValue, int maxValue) { m_colorSlider->setRange(minValue, maxValue); }

	void setChannel(ColorChannel channel) { return m_colorSlider->setChannel(channel); }
	void setColor(const ColorModel &color) { return m_colorSlider->setColor(color); }

	ColorChannel getChannel() const { return m_colorSlider->getChannel(); }

protected slots:
	void onRemove();
	void onAdd();

signals:
	void valueChanged(int);
	void valueChanged();
};

//=============================================================================
/*! \brief The ChannelLineEdit is a cutomized version of IntLineEdit for channel value.
    It calls selectAll() at the moment of the first click.
*/
class ChannelLineEdit : public IntLineEdit
{
	Q_OBJECT

	bool m_isEditing;

public:
	ChannelLineEdit(QWidget *parent, int value, int minValue, int maxValue)
		: IntLineEdit(parent, value, minValue, maxValue), m_isEditing(false)
	{
	}

protected:
	void mousePressEvent(QMouseEvent *);
	void focusOutEvent(QFocusEvent *);
	void paintEvent(QPaintEvent *);
};

//=============================================================================
/*! \brief ColorChannelControl is the widget used to show/edit a channel

		Inherits \b QWidget.

		The ColorChannelControl is composed of three object: a label \b QLabel
		to show the channel name, and an \b IntLineEdit and a ColorSlider to show/edit the
		channel value.
*/
class DVAPI ColorChannelControl : public QWidget
{
	Q_OBJECT
	QLabel *m_label;
	ChannelLineEdit *m_field;
	ColorSlider *m_slider;

	ColorChannel m_channel;
	ColorModel m_color;

	int m_value;
	bool m_signalEnabled;

public:
	ColorChannelControl(ColorChannel channel, QWidget *parent = 0);
	void setColor(const ColorModel &color);

protected slots:
	void onFieldChanged();
	void onSliderChanged(int value);
	void onSliderReleased();

	void onAddButtonClicked();
	void onSubButtonClicked();

signals:
	void colorChanged(const ColorModel &color, bool isDragging);
};

//=============================================================================
/*! \brief The StyleEditorPage is the base class of StyleEditor pages.
 
		Inherits \b QFrame.
		Inherited by \b PlainColorPage and \b StyleChooserPage.
*/
class StyleEditorPage : public QFrame
{
public:
	StyleEditorPage(QWidget *parent);
};

//=============================================================================
/*! \brief The ColorParameterSelector is used for styles having more
    than one color parameter to select the current one.

		Inherits \b QWidget.
*/
class ColorParameterSelector : public QWidget
{
	Q_OBJECT

	std::vector<QColor> m_colors;
	int m_index;
	const QSize m_chipSize;
	const QPoint m_chipOrigin, m_chipDelta;

public:
	ColorParameterSelector(QWidget *parent);
	int getSelected() const { return m_index; }
	void setStyle(const TColorStyle &style);
	void clear();

signals:
	void colorParamChanged();

protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
};

//=============================================================================
/*! \brief The PlainColorPage is used to control the color parameter. 

		Inherits \b StyleEditorPage.

		The PlainColorPage is made of a \b SquaredColorWheel and a \b ColorSlider, 
		a collection of \b ColorChannelControl, and a number of radio button (to control
		the ColorWheel behaviour).
*/
class PlainColorPage : public StyleEditorPage
{
	Q_OBJECT

	//ColorSliderBar *m_verticalSlider;
	//QRadioButton *m_modeButtons[7];
	ColorChannelControl *m_channelControls[7];
	//SquaredColorWheel *m_squaredColorWheel; //iwsw not used

	HexagonalColorWheel *m_hexagonalColorWheel;

	ColorModel m_color;
	bool m_signalEnabled;

	void updateControls();

public:
	PlainColorPage(QWidget *parent = 0);
	~PlainColorPage() {}

	void setColor(const TColorStyle &style, int colorParameterIndex);

protected:
	void resizeEvent(QResizeEvent *);

signals:
	void colorChanged(const ColorModel &, bool isDragging);

protected slots:
	void onWheelChanged(const ColorModel &color, bool isDragging);
	//void onWheelSliderChanged(int value);
	//void onWheelSliderReleased();

public slots:
	//void setWheelChannel(int channel);
	void onControlChanged(const ColorModel &color, bool isDragging);
};

//=============================================================================
/*! \brief The StyleChooserPage is the base class of pages with texture, 
    special style and custom style. It features a collection of selectable 'chips'. 

		Inherits \b StyleEditorPage.
*/
class StyleChooserPage : public StyleEditorPage
{
	Q_OBJECT

protected:
	QPoint m_chipOrigin;
	QSize m_chipSize;
	int m_chipPerRow;
	static TFilePath m_rootPath;

public:
	StyleChooserPage(QWidget *parent = 0);

	QSize getChipSize() const { return m_chipSize; }

	virtual bool loadIfNeeded() = 0;
	virtual int getChipCount() const = 0;

	virtual void drawChip(QPainter &p, QRect rect, int index) = 0;
	virtual void onSelect(int index) {}

	//! \see StyleEditor::setRootPath()
	//TOGLIERE
	static void setRootPath(const TFilePath &rootPath);
	static TFilePath getRootPath() { return m_rootPath; }

protected:
	int m_currentIndex;

	int posToIndex(const QPoint &pos) const;

	void paintEvent(QPaintEvent *);
	void resizeEvent(QResizeEvent *) { computeSize(); }

	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event) {}
	void mouseReleaseEvent(QMouseEvent *event);
protected slots:
	void computeSize();
signals:
	void styleSelected(const TColorStyle &style);
};

//=============================================================================

/*!
  \brief    The SettingsPage is used to show/edit style parameters.

  \details  This class stores the GUI for editing a \a copy of the
            current color style. Updates of the actual current color
            style are \a not performed directly by this class.
*/

class SettingsPage : public QScrollArea
{
	Q_OBJECT

	QGridLayout *m_paramsLayout;

	QCheckBox *m_autoFillCheckBox;
	QWidget *m_autopaintToggleBox;

	TColorStyleP m_editedStyle; //!< A copy of the current style being edited by the Style Editor.

	bool m_updating; //!< Whether the page is copying style content to its widget,
	//!  to be displayed.
public:
	SettingsPage(QWidget *parent);

	void setStyle(const TColorStyleP &editedStyle);
	void updateValues();

	void enableAutopaintToggle(bool enabled);

signals:

	void paramStyleChanged(bool isDragging); //!< Signals that the edited style has changed.

private slots:

	void onAutofillChanged();
	void onValueChanged(bool isDragging = false);
};

//=============================================================================
} // namespace StyleEditorGUI
//=============================================================================

using namespace StyleEditorGUI;

//=============================================================================
// StyleEditor
//-----------------------------------------------------------------------------

class DVAPI StyleEditor : public QWidget
{
	Q_OBJECT

	PaletteController *m_paletteController;
	TPaletteHandle *m_paletteHandle;
	TPaletteHandle *m_cleanupPaletteHandle;

	TXshLevelHandle *m_levelHandle; //!< for clearing the level cache when the color changed

	TabBar *m_styleBar;
	QStackedWidget *m_styleChooser;

	StyleSample *m_newColor; //!< New style viewer (lower-right panel side).
	StyleSample *m_oldColor; //!< Old style viewer (lower-right panel side).

#ifndef STUDENT
	QPushButton *m_autoButton;  //!< "Auto Apply" checkbox on the right panel side.
	QPushButton *m_applyButton; //!< "Apply" button on the right panel side.
#endif

	QToolBar *m_toolBar;							  //!< Lower toolbar.
	ColorParameterSelector *m_colorParameterSelector; //!< Secondary color parameter selector in the lower toolbar.

	TabBarContainter *m_tabBarContainer; //!< Tabs container for style types.

	QLabel *m_statusLabel; //!< showing the information of the current palette and style.

	PlainColorPage *m_plainColorPage;
	StyleChooserPage *m_textureStylePage;
	StyleEditorPage *m_specialStylePage;
	StyleChooserPage *m_customStylePage;
	StyleChooserPage *m_vectorBrushesStylePage;
	SettingsPage *m_settingsPage;

	TColorStyleP m_oldStyle;	//!< A copy of current style \a before the last change.
	TColorStyleP m_editedStyle; //!< The currently edited style. Please observe that this is
								//!  a \b copy of currently selected style, since style edits
								//!  may be not <I>automatically applied</I>.
	bool m_enabled;
	bool m_enabledOnlyFirstTab;
	bool m_enabledFirstAndLastTab;

public:
	StyleEditor(PaletteController *, QWidget *parent = 0);
	~StyleEditor();

	void setPaletteHandle(TPaletteHandle *paletteHandle);
	TPaletteHandle *getPaletteHandle() const { return m_paletteHandle; }

	void setLevelHandle(TXshLevelHandle *levelHandle) { m_levelHandle = levelHandle; }

	TPalette *getPalette() { return m_paletteHandle->getPalette(); }
	int getStyleIndex() { return m_paletteHandle->getStyleIndex(); }

	/*! rootPath generally is STUFFDIR/Library. Contains directories 'textures' and
			'custom styles' */
	//TOGLIERE
	void setRootPath(const TFilePath &rootPath);

	void enableAutopaintToggle(bool enabled)
	{
		m_settingsPage->enableAutopaintToggle(enabled);
	}

protected:
	/*! Return false if style is linked and style must be set to null.*/
	bool setStyle(TColorStyle *currentStyle);

	void setEditedStyleToStyle(const TColorStyle *style); //!< Clones the supplied style and considers that as the edited one.
	void setOldStyleToStyle(const TColorStyle *style);	//!< Clones the supplied style and considers that as the previously current one.
														  //!  \todo  Why is this not assimilated to setCurrentStyleToStyle()?

	/*! Return style parameter index selected in \b ColorParameterSelector. */
	int getColorParam() const
	{
		return m_colorParameterSelector->getSelected();
	}

	/*! Set StyleEditor view to \b enabled. If \b enabledOnlyFirstTab or if \b enabledFirstAndLastTab
			is true hide other tab, pay attention \b enabled must be true or StyleEditor is disabled. */
	void enable(bool enabled, bool enabledOnlyFirstTab = false, bool enabledFirstAndLastTab = false);

protected:
	void showEvent(QShowEvent *);
	void hideEvent(QHideEvent *);

protected slots:

	void onStyleSwitched();
	void onStyleChanged();
	void onCleanupStyleChanged();
	void onOldStyleClicked(const TColorStyle &);

	// called (e.g.) by PaletteController when an other StyleEditor change the toggle
	void enableColorAutoApply(bool enabled);

	// when colorAutoApply==false this slot is called when the current color changes
	void setColorSample(const TPixel32 &color);

	// chiamato quando viene modificato uno slider o la color wheel
	void onColorChanged(const ColorModel &, bool isDragging);

	void selectStyle(const TColorStyle &style);

	void applyButtonClicked();
	void autoCheckChanged(bool value);

	void setPage(int index);

	void onColorParamChanged();

	void onParamStyleChanged(bool isDragging);

private:
	QFrame *createBottomWidget();
	void updateTabBar();

	void copyEditedStyleToPalette(bool isDragging);
};

#endif // STYLEEDITOR_H
