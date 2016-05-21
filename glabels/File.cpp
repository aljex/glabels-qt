/*  File.cpp
 *
 *  Copyright (C) 2014  Jim Evins <evins@snaught.com>
 *
 *  This file is part of gLabels-qt.
 *
 *  gLabels-qt is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  gLabels-qt is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with gLabels-qt.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "File.h"

#include "MainWindow.h"
#include "LabelModel.h"
#include "SelectProductDialog.h"
#include "XmlLabelParser.h"
#include "XmlLabelCreator.h"
#include "FileUtil.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>


/// @TODO keep track of cwd between open/save dialogs

///
/// New Label Dialog
///
bool File::newLabel( MainWindow *window )
{
	SelectProductDialog dialog( window );
	dialog.exec();

	const glabels::Template* tmplate = dialog.tmplate();
	if ( tmplate )
	{
		LabelModel* label = new LabelModel();
		label->setTmplate( tmplate );
		label->clearModified();

		// Intelligently decide to rotate label by default
		const glabels::Frame* frame = tmplate->frames().first();
		label->setRotate( frame->h() > frame->w() );

		// Either apply to current window or open a new one
		if ( window->isEmpty() )
		{
			window->setModel( label );
		}
		else
		{
			MainWindow *newWindow = new MainWindow();
			newWindow->setModel( label );
			newWindow->show();
		}

		return true;
	}
	else
	{
		return false;
	}
}


///
/// Open File Dialog
///
void File::open( MainWindow *window )
{
	QString fileName =
		QFileDialog::getOpenFileName( window,
					      tr("gLabels - Open Project"),
					      ".",
					      tr("glabels files (*.glabels);;All files (*)")
			);
	if ( !fileName.isEmpty() )
	{
		LabelModel *label = XmlLabelParser::readFile( fileName );
		if ( label )
		{
			label->setFileName( fileName );
				
			// Either apply to current window or open a new one
			if ( window->isEmpty() )
			{
				window->setModel( label );
			}
			else
			{
				MainWindow *newWindow = new MainWindow();
				newWindow->setModel( label );
				newWindow->show();
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText( tr("Unable to open \"") + fileName + tr("\".") );
			msgBox.setStandardButtons( QMessageBox::Ok );
			msgBox.setDefaultButton( QMessageBox::Ok );
			msgBox.exec();
		}
	}
}


///
/// Save file
///
bool File::save( MainWindow *window )
{
	if ( window->model()->fileName().isEmpty() )
	{
		return saveAs( window );
	}

	if ( !window->model()->isModified() )
	{
		return true;
	}

	XmlLabelCreator::writeFile( window->model(), window->model()->fileName() );
	window->model()->clearModified();
	
	return true;
}


///
/// Save file as
///
bool File::saveAs( MainWindow *window )
{
	QString rawFileName =
		QFileDialog::getSaveFileName( window,
					      tr("gLabels - Save Project As"),
					      ".",
					      tr("glabels files (*.glabels);;All files (*)"),
					      0,
					      QFileDialog::DontConfirmOverwrite
			);
	if ( !rawFileName.isEmpty() )
	{
		QString fileName = FileUtil::addExtension( rawFileName, ".glabels" );
			
			
		if ( QFileInfo(fileName).exists() )
		{
			QMessageBox msgBox( window );
			msgBox.setWindowTitle( tr("Save Label As") );
			msgBox.setIcon( QMessageBox::Warning );
			msgBox.setText( tr("%1 already exists.").arg(fileName) );
			msgBox.setInformativeText( tr("Do you want to replace it?") );
			msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
			msgBox.setDefaultButton( QMessageBox::No );

			if ( msgBox.exec() == QMessageBox::No )
			{
				return saveAs( window );
			}
		}
			
		XmlLabelCreator::writeFile( window->model(), fileName );
		window->model()->setFileName( fileName );
		window->model()->clearModified();
		
		return true;
	}

	return false;
}


///
/// Print file
///
void File::print( MainWindow *window )
{
	qDebug() << "ACTION: file->print";
}


///
/// Close file
///
void File::close( MainWindow *window )
{
	window->close();
}


///
/// Exit, closing all windows
///
void File::exit()
{
	foreach ( QWidget* qwidget, QApplication::topLevelWidgets() )
	{
		if ( MainWindow* window = qobject_cast<MainWindow*>(qwidget) )
		{
			window->close();
		}
	}
}
