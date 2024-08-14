#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>
#include <QtGui/qlineedit.h>
#include <QtGui/qboxlayout.h>
#include <QtGui/qfiledialog.h>
#include <QtCore/qsettings.h>
#include <QtGui/qformlayout.h>
#include <QtGui/qcombobox.h>
#include <QtGui/qdesktopservices.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qlistwidget.h>
#include <QtGui/qgroupbox.h>

#include "dzapp.h"
#include "dzscene.h"
#include "dzstyle.h"
#include "dzmainwindow.h"
#include "dzactionmgr.h"
#include "dzaction.h"
#include "dzskeleton.h"
#include "qstandarditemmodel.h"

#include "DzBlenderDialog.h"
#include "DzBridgeMorphSelectionDialog.h"
#include "DzBridgeSubdivisionDialog.h"

#include "version.h"

/*****************************
Local definitions
*****************************/
#define DAZ_BRIDGE_PLUGIN_NAME "Daz To Blender"

#include "dzbridge.h"

DzBlenderDialog::DzBlenderDialog(QWidget* parent) :
	 DzBridgeDialog(parent, DAZ_BRIDGE_PLUGIN_NAME)
{
	 intermediateFolderEdit = nullptr;
	 intermediateFolderButton = nullptr;

	 settings = new QSettings("Daz 3D", "DazToBlender");

	 // Declarations
	 int margin = style()->pixelMetric(DZ_PM_GeneralMargin);
	 int wgtHeight = style()->pixelMetric(DZ_PM_ButtonHeight);
	 int btnMinWidth = style()->pixelMetric(DZ_PM_ButtonMinWidth);

	 // Set the dialog title
	 int revision = PLUGIN_REV % 1000;
#ifdef _DEBUG
	 setWindowTitle(tr("Daz To Blender Bridge %1 v%2.%3.%4 PreRelease Build").arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(revision).arg(PLUGIN_BUILD));
#else
	 setWindowTitle(tr("Daz To Blender Bridge %1 v%2.%3").arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(revision));
#endif

	 QString sDazAppDir = dzApp->getHomePath().replace("\\","/");
	 QString sPdfPath = sDazAppDir + "/docs/Plugins" + "/Daz to Blender/Daz to Blender.pdf";
	 QString sSetupModeString = tr("\
<div style=\"background-color:#282f41;\" align=center>\
<img src=\":/DazBridgeBlender/banner.jpg\" width=\"370\" height=\"95\" align=\"center\" hspace=\"0\" vspace=\"0\">\
<table width=100% cellpadding=8 cellspacing=2 style=\"vertical-align:middle; font-size:x-large; font-weight:bold; background-color:#FFAA00;foreground-color:#FFFFFF\" align=center>\
  <tr>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://www.daz3d.com/blender-bridge#faq\">FAQ</a></div></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://youtu.be/uyZb545tDks\">Installation Video</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://youtu.be/iYUjVWGiSyM\">Tutorial Video</a></td>\
  </tr>\
  <tr>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"file:///") + sPdfPath + tr("\">PDF</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://www.daz3d.com/forums/categories/blender-discussion\">Forums</a></td>\
    <td width=33% style=\"text-align:center; background-color:#282f41;\"><div align=center><a href=\"https://github.com/daz3d/DazToBlender/issues\">Report Bug</a></td>\
  </tr>\
</table>\
</div>\
");
	 m_WelcomeLabel->setText(sSetupModeString);

//	 // Disable Subdivision UI
//	 subdivisionEnabledCheckBox->setChecked(false);
//	 subdivisionEnabledCheckBox->setDisabled(true);
//	 subdivisionButton->setToolTip(tr("Subdivision Baking Disabled"));
//	 subdivisionButton->setWhatsThis(tr("Blender 2.8+ now supports built-in Catmull-Clark Subdivision Surfaces \
//like Daz Studio. This is much faster and should be used instead of baking out subdivision levels during the \
//Bridge Export process."));
//	 subdivisionEnabledCheckBox->setToolTip(tr("Subdivision Baking Disabled."));
//	subdivisionEnabledCheckBox->setWhatsThis(tr("Blender 2.8+ now supports built-in Catmull-Clark Subdivision Surfaces \
//like Daz Studio. This is much faster and should be used instead of baking out subdivision levels during the \
//Bridge Export process."));
//	 //	 subdivisionButton->setDisabled(true);
//	 disconnect(subdivisionButton, 0, this, 0);
//	 connect(subdivisionButton, SIGNAL(released()), this, SLOT(HandleDisabledChooseSubdivisionsButton()));

	 // Disable Unsupported AssetType ComboBox Options
	 QStandardItemModel* model = qobject_cast<QStandardItemModel*>(assetTypeCombo->model());
	 QStandardItem* item = nullptr;
	 item = model->findItems("Environment").first();
	 if (item) item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
	 item = model->findItems("Pose").first();
	 if (item) item->setFlags(item->flags() & ~Qt::ItemIsEnabled);

	 // Connect new asset type handler
	 connect(assetTypeCombo, SIGNAL(activated(int)), this, SLOT(HandleAssetTypeComboChange(int)));

	 // Intermediate Folder
	 QHBoxLayout* intermediateFolderLayout = new QHBoxLayout();
	 intermediateFolderLayout->setSpacing(0);
	 intermediateFolderEdit = new QLineEdit(this);
	 //intermediateFolderButton = new QPushButton("...", this);
	 intermediateFolderButton = new DzBridgeBrowseButton(this);
	 intermediateFolderLayout->addWidget(intermediateFolderEdit);
	 intermediateFolderLayout->addWidget(intermediateFolderButton);
	 connect(intermediateFolderButton, SIGNAL(released()), this, SLOT(HandleSelectIntermediateFolderButton()));

	 // Advanced Options
#ifdef __LEGACY_PATHS__
	 assetNameEdit->setValidator(new QRegExpValidator(QRegExp("*"), this));
//	 intermediateFolderEdit->setVisible(false);
//	 intermediateFolderButton->setVisible(false);
#endif
//	 QFormLayout* advancedLayout = qobject_cast<QFormLayout*>(advancedSettingsGroupBox->layout());
	 if (advancedLayout)
	 {
		 advancedLayout->addRow("Intermediate Folder", intermediateFolderLayout);
		 // reposition the Open Intermediate Folder button so it aligns with the center section
		 advancedLayout->removeWidget(m_OpenIntermediateFolderButton);
		 advancedLayout->addRow("", m_OpenIntermediateFolderButton);
	 }

	 QString sBlenderVersionString = tr("DazToBlender Bridge %1 v%2.%3.%4").arg(PLUGIN_MAJOR).arg(PLUGIN_MINOR).arg(revision).arg(PLUGIN_BUILD);
	 setBridgeVersionStringAndLabel(sBlenderVersionString);

	 // Configure Target Plugin Installer
	 renameTargetPluginInstaller("Blender Addon Installer");
	 m_TargetSoftwareVersionCombo->clear();
	 m_TargetSoftwareVersionCombo->addItem("Select Blender Version");
     m_TargetSoftwareVersionCombo->addItem("Blender 2.83");
	 m_TargetSoftwareVersionCombo->addItem("Blender 2.93");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.0");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.1");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.2");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.3");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.4");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.5");
	 m_TargetSoftwareVersionCombo->addItem("Blender 3.6");
	 m_TargetSoftwareVersionCombo->addItem("Blender 4.0");
	 m_TargetSoftwareVersionCombo->addItem("Blender 4.1");
	 m_TargetSoftwareVersionCombo->addItem("Blender 4.2");
	 m_TargetSoftwareVersionCombo->addItem("Custom Addon Path");
	 showTargetPluginInstaller(true);

	 // Make the dialog fit its contents, with a minimum width, and lock it down
	 resize(QSize(500, 0).expandedTo(minimumSizeHint()));
	 setFixedWidth(width());
	 setFixedHeight(height());

	 update();

	 // Help
	 assetNameEdit->setWhatsThis("This is the name the asset will use in Blender.");
	 assetTypeCombo->setWhatsThis("Skeletal Mesh for something with moving parts, like a character\nStatic Mesh for things like props\nAnimation for a character animation.");
	 intermediateFolderEdit->setWhatsThis("DazToBlender will collect the assets in a subfolder under this folder.  Blender will import them from here.");
	 intermediateFolderButton->setWhatsThis("DazToBlender will collect the assets in a subfolder under this folder.  Blender will import them from here.");
	 m_wTargetPluginInstaller->setWhatsThis("You can install the Blender Addon by selecting the desired Blender version and then clicking Install.");

	 // Set Defaults
	 resetToDefaults();

	 // Load Settings
	 loadSavedSettings();

	 // Daz Ultra
	 m_WelcomeLabel->hide();
	 setWindowTitle(tr("Blender Export Options"));
	 this->m_wPdfButton->show();
	 this->m_wSupportButton->show();
	 this->m_wYoutubeButton->show();

}

bool DzBlenderDialog::loadSavedSettings()
{
	DzBridgeDialog::loadSavedSettings();

	if (!settings->value("IntermediatePath").isNull())
	{
		QString directoryName = settings->value("IntermediatePath").toString();
		directoryName = directoryName.replace("\\", "/");
		intermediateFolderEdit->setText(directoryName);
	}
	else
	{
//		QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToBlender";
		QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/DAZ 3D/Bridges/Daz To Blender/";
		DefaultPath = DefaultPath.replace("\\", "/");
		intermediateFolderEdit->setText(DefaultPath);
	}

	return true;
}

void DzBlenderDialog::accept()
{
	saveSettings();

	if (m_bSetupMode)
		return  DzBasicDialog::reject();

	return DzBasicDialog::accept();
}

void DzBlenderDialog::saveSettings()
{
	if (settings == nullptr || m_bDontSaveSettings) return;

	DzBridgeDialog::saveSettings();

	QString sIntermediateFolderPath = intermediateFolderEdit->text();
	if (sIntermediateFolderPath == "")
	{
#ifdef __LEGACY_PATHS__
		sIntermediateFolderPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/DAZ 3D/Bridges/Daz To Blender/";
#else
		sIntermediateFolderPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToBlender";
#endif
	}
	sIntermediateFolderPath = sIntermediateFolderPath.replace("\\", "/");
	settings->setValue("IntermediatePath", sIntermediateFolderPath);

}

void DzBlenderDialog::resetToDefaults()
{
	m_bDontSaveSettings = true;
	DzBridgeDialog::resetToDefaults();

//	QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToBlender";
	QString DefaultPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/DAZ 3D/Bridges/Daz To Blender/";
	intermediateFolderEdit->setText(DefaultPath);

	DzNode* Selection = dzScene->getPrimarySelection();
#ifdef __LEGACY_PATHS__
	if (Selection != nullptr)
		assetNameEdit->setText(Selection->getLabel());
#else
	if (dzScene->getFilename().length() > 0)
	{
		QFileInfo fileInfo = QFileInfo(dzScene->getFilename());
		assetNameEdit->setText(fileInfo.baseName().remove(QRegExp("[^A-Za-z0-9_]")));
	}
	else if (dzScene->getPrimarySelection())
	{
		assetNameEdit->setText(Selection->getLabel().remove(QRegExp("[^A-Za-z0-9_]")));
	}
#endif

	if (qobject_cast<DzSkeleton*>(Selection))
	{
		assetTypeCombo->setCurrentIndex(0);
	}
	else
	{
		assetTypeCombo->setCurrentIndex(1);
	}
	m_bDontSaveSettings = false;
}

void DzBlenderDialog::HandleSelectIntermediateFolderButton()
{
	 // DB (2021-05-15): prepopulate with existing folder string
	 QString directoryName = "/home";
	 if (settings != nullptr && settings->value("IntermediatePath").isNull() != true)
	 {
		 directoryName = settings->value("IntermediatePath").toString();
	 }
	 directoryName = QFileDialog::getExistingDirectory(this, tr("Choose Directory"),
		  directoryName,
		  QFileDialog::ShowDirsOnly
		  | QFileDialog::DontResolveSymlinks);

	 if (directoryName != NULL)
	 {
		 intermediateFolderEdit->setText(directoryName);
		 if (settings != nullptr)
		 {
			 settings->setValue("IntermediatePath", directoryName);
		 }
	 }
}

void DzBlenderDialog::HandleAssetTypeComboChange(int state)
{
	QString assetNameString = assetNameEdit->text();

	// enable/disable Morphs and Subdivision only if Skeletal selected
	if (assetTypeCombo->currentText() != "Skeletal Mesh")
	{
		morphsEnabledCheckBox->setChecked(false);
		subdivisionEnabledCheckBox->setChecked(false);
	}

}

#include <QProcessEnvironment>

void DzBlenderDialog::HandleTargetPluginInstallerButton()
{
	// Get Software Versio
	DzBridgeDialog::m_sEmbeddedFilesPath = ":/DazBridgeBlender";
	QString sBinariesFile = "/blenderaddon.zip";
	QProcessEnvironment env(QProcessEnvironment::systemEnvironment());
#ifdef __APPLE__
    QString sAppData = QDir::homePath() + "/Library/Application Support";
    QString sDestinationPath = sAppData + "/Blender";
#else
    QString sAppData = env.value("USERPROFILE") + "/Appdata/Roaming";
    QString sDestinationPath = sAppData + "/Blender Foundation/Blender";
#endif
	QString softwareVersion = m_TargetSoftwareVersionCombo->currentText();
    if (softwareVersion.contains("2.83"))
    {
        sDestinationPath += "/2.83/scripts";
    }
    else if (softwareVersion.contains("2.93"))
	{
		sDestinationPath += "/2.93/scripts";
	}
	else if (softwareVersion.contains("3.0"))
	{
		sDestinationPath += "/3.0/scripts";
	}
	else if (softwareVersion.contains("3.1"))
	{
		sDestinationPath += "/3.1/scripts";
	}
	else if (softwareVersion.contains("3.2"))
	{
		sDestinationPath += "/3.2/scripts";
	}
	else if (softwareVersion.contains("3.3"))
	{
		sDestinationPath += "/3.3/scripts";
	}
	else if (softwareVersion.contains("3.4"))
	{
		sDestinationPath += "/3.4/scripts";
	}
	else if (softwareVersion.contains("3.5"))
	{
		sDestinationPath += "/3.5/scripts";
	}
	else if (softwareVersion.contains("3.6"))
	{
		sDestinationPath += "/3.6/scripts";
	}
	else if (softwareVersion.contains("4.0"))
	{
		sDestinationPath += "/4.0/scripts";
	}
	else if (softwareVersion.contains("4.1"))
	{
		sDestinationPath += "/4.1/scripts";
	}
	else if (softwareVersion.contains("4.2"))
	{
		sDestinationPath += "/4.2/scripts";
	}
	else if (softwareVersion.contains("Custom"))
	{
		// Get Destination Folder
		sDestinationPath = QFileDialog::getExistingDirectory(this,
			tr("Please select a Blender Version Folder. DazToBlender will install into the correct addons subfolder."),
			sDestinationPath,
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		if (sDestinationPath == NULL)
		{
			// User hit cancel: return without addition popups
			return;
		}
	}
	else
	{
		// Warning, not a valid addons folder path
		QMessageBox::information(0, "DazToBlender Bridge",
			tr("Please select a Blender version."));
		return;
	}

	// fix path separators
	sDestinationPath = sDestinationPath.replace("\\", "/");
	// load with default values
	QString sRootPath = sDestinationPath;
	QString sPluginsPath = sRootPath;
	// verify plugin path
	bool bIsPluginPath = false;	
	//if (sPluginsPath.endsWith("/addons", Qt::CaseInsensitive)==false)
 //   {
 //       sPluginsPath += "/addons";
 //   }

	if (QRegExp(".*/blender/\\d+\\.\\d+$").exactMatch(sDestinationPath.toLower()) == true)
	{
		sRootPath = sDestinationPath;
		sPluginsPath = sRootPath + "/scripts/addons";
	}
	else if (QRegExp(".*/blender/\\d+\\.\\d+/scripts$").exactMatch(sDestinationPath.toLower()) == true)
	{
		sRootPath = sDestinationPath;
		sPluginsPath = sRootPath + "/addons";
	}
	else if (QRegExp(".*/blender/\\d+\\.\\d+/scripts/addons$").exactMatch(sDestinationPath.toLower()) == true)
	{
		sPluginsPath = sDestinationPath;
		sRootPath = QFileInfo(sPluginsPath).dir().path();
	}
	// Less accurate fallback
	else if (QRegExp(".*/\\d+\\.\\d+$").exactMatch(sDestinationPath.toLower()) == true)
	{
		sRootPath = sDestinationPath;
		sPluginsPath = sRootPath + "/scripts/addons";
	}

	// Check for "/scripts/addon" at end of path
	if (sPluginsPath.endsWith("/scripts/addons", Qt::CaseInsensitive))
	{
		QString sScriptsPath = QString(sPluginsPath).replace("/scripts/addons", "/scripts", Qt::CaseInsensitive);
		QString sConfigPath = QString(sPluginsPath).replace("/scripts/addons", "/config", Qt::CaseInsensitive);
		if (QDir(sPluginsPath).exists() || QDir(sScriptsPath).exists() || QDir(sConfigPath).exists())
		{
			bIsPluginPath = true;
		}
	}

	if (bIsPluginPath == false)
	{
		// Warning, not a valid plugins folder path
		auto userChoice = QMessageBox::warning(0, "Daz To Blender",
			tr("The destination folder may not be a valid Blender Addons folder.  Please make sure \
Blender is properly installed or the custom scripts path is properly configured in Blender \
Preferences:\n\n") + sPluginsPath + tr("\n\nYou can choose to Abort and select a new folder, \
or Ignore this warning and install the addon anyway."),
QMessageBox::Ignore | QMessageBox::Abort,
QMessageBox::Abort);
		if (userChoice == QMessageBox::StandardButton::Abort)
			return;

	}

	// create plugins folder if does not exist
	if (QDir(sPluginsPath).exists() == false)
	{
		QDir().mkpath(sPluginsPath);
	}

	bool bInstallSuccessful = false;
	bInstallSuccessful = installEmbeddedArchive(sBinariesFile, sPluginsPath);

	if (bInstallSuccessful)
	{
		QMessageBox::information(0, "Daz To Blender",
			tr("Blender Addon successfully installed to: ") + sPluginsPath +
			tr("\n\nIf Blender is running, please quit and restart Blender to continue \
Bridge Export process."));
	}
	else
	{
		QMessageBox::warning(0, "Daz To Blender",
			tr("Sorry, an unknown error occured. Unable to install Blender \
Addon to: ") + sPluginsPath);
		return;
	}

	return;
}

void DzBlenderDialog::HandleDisabledChooseSubdivisionsButton()
{
	QMessageBox msgBox;
	msgBox.setTextFormat(Qt::RichText);
	msgBox.setWindowTitle("Daz To Blender: Subdivision Baking is currently disabled");
	msgBox.setText(tr("Sorry, DazToBlender's Subdivision Baking functionallity is currently disabled \
while it is being redesigned.<br><br> \
Since version 2.8+, Blender has supported built-in Catmull-Clark Subdivision Surfaces \
like Daz Studio. This is much faster and should be used instead of baking out subdivision \
levels during the Bridge Export process.<br><br>You can find out more about Blender's built-in \
Subdivision Support here:<br><br>\
<a href=\"https://docs.blender.org/manual/en/3.1/modeling/modifiers/generate/subdivision_surface.html\">Subdivision Surface, Blender 3.1 Manual</a><br><br>"));
	msgBox.setStandardButtons(QMessageBox::Ok);
	msgBox.exec();
	return;
}

void DzBlenderDialog::HandleOpenIntermediateFolderButton(QString sFolderPath)
{
	QString sIntermediateFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + QDir::separator() + "DazToBlender";
#if __LEGACY_PATHS__
	sIntermediateFolder == "";
	if (intermediateFolderEdit != nullptr)
	{
		sIntermediateFolder = intermediateFolderEdit->text();
	}
	if (sIntermediateFolder == "")
	{
		sIntermediateFolder = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/DAZ 3D/Bridges/Daz To Blender";
		// add back to edit widget
		if (intermediateFolderEdit) {
			intermediateFolderEdit->setText(sIntermediateFolder);
		}
	}
	if (QFile(sIntermediateFolder).exists() == false)
	{
		QDir().mkpath(sIntermediateFolder);
	}
	if (!sIntermediateFolder.endsWith("/Exports", Qt::CaseInsensitive) && QFile(sIntermediateFolder + "/Exports").exists())
	{
		sIntermediateFolder += "/Exports";
	}
#else
	if (intermediateFolderEdit != nullptr)
	{
		sIntermediateFolder = intermediateFolderEdit->text();
	}
#endif
	DzBridgeDialog::HandleOpenIntermediateFolderButton(sIntermediateFolder);
}

void DzBlenderDialog::refreshAsset()
{
	DzBridgeDialog::refreshAsset();

#if __LEGACY_PATHS__
	DzNode* Selection = dzScene->getPrimarySelection();
	if (Selection != nullptr)
	{
		assetNameEdit->setText(Selection->getLabel());
	}
#endif

}

#include <QDesktopServices>
#include <QUrl>
void DzBlenderDialog::HandlePdfButton()
{
	QString sDazAppDir = dzApp->getHomePath().replace("\\", "/");
	QString sPdfPath = sDazAppDir + "/docs/Plugins" + "/Daz to Blender/Daz to Blender.pdf";
	QDesktopServices::openUrl(QUrl(sPdfPath));
}

void DzBlenderDialog::HandleYoutubeButton()
{
	QString url = "https://youtu.be/iYUjVWGiSyM";
	QDesktopServices::openUrl(QUrl(url));
}

void DzBlenderDialog::HandleSupportButton()
{
	QString url = "https://bugs.daz3d.com/hc/en-us/requests/new";
	QDesktopServices::openUrl(QUrl(url));
}


#include "moc_DzBlenderDialog.cpp"
