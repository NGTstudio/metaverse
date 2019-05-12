#include "ObjectEditor.h"


#include "PlayerPhysics.h"
#include "../dll/include/IndigoMesh.h"
#include "../indigo/TextureServer.h"
#include "../indigo/globals.h"
#include "../graphics/Map2D.h"
#include "../graphics/imformatdecoder.h"
#include "../graphics/ImageMap.h"
#include "../maths/vec3.h"
#include "../maths/GeometrySampling.h"
#include "../utils/Lock.h"
#include "../utils/Mutex.h"
#include "../utils/Clock.h"
#include "../utils/Timer.h"
#include "../utils/Platform.h"
#include "../utils/FileUtils.h"
#include "../utils/Reference.h"
#include "../utils/StringUtils.h"
#include "../utils/CameraController.h"
#include "../utils/TaskManager.h"
#include "../qt/SignalBlocker.h"
#include "../qt/QtUtils.h"
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QErrorMessage>
#include <set>
#include <stack>
#include <algorithm>


ObjectEditor::ObjectEditor(QWidget *parent)
:	QWidget(parent),
	selected_mat_index(0)
{
	setupUi(this);

	this->modelFileSelectWidget->force_use_last_dir_setting = true;
	this->scriptFileSelectWidget->force_use_last_dir_setting = true;

	this->scaleXDoubleSpinBox->setMinimum(0.00001);
	this->scaleYDoubleSpinBox->setMinimum(0.00001);
	this->scaleZDoubleSpinBox->setMinimum(0.00001);

	connect(this->matEditor,				SIGNAL(materialChanged()),			this, SIGNAL(objectChanged()));

	connect(this->modelFileSelectWidget,	SIGNAL(filenameChanged(QString&)),	this, SIGNAL(objectChanged()));
	connect(this->scriptFileSelectWidget,	SIGNAL(filenameChanged(QString&)),	this, SIGNAL(objectChanged()));

	connect(this->contentTextEdit,			SIGNAL(textChanged()),				this, SIGNAL(objectChanged()));
	connect(this->targetURLLineEdit,		SIGNAL(textChanged(const QString&)),this, SIGNAL(objectChanged()));
	connect(this->targetURLLineEdit,		SIGNAL(textChanged(const QString&)),this, SLOT(targetURLChanged()));

	connect(this->posXDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->posYDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->posZDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));

	connect(this->scaleXDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->scaleYDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->scaleZDoubleSpinBox,		SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	
	connect(this->rotAxisXDoubleSpinBox,	SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->rotAxisYDoubleSpinBox,	SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->rotAxisZDoubleSpinBox,	SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));
	connect(this->rotAngleDoubleSpinBox,	SIGNAL(valueChanged(double)),		this, SIGNAL(objectChanged()));

	this->visitURLLabel->hide();
}


ObjectEditor::~ObjectEditor()
{
}


void ObjectEditor::setFromObject(const WorldObject& ob, int selected_mat_index_)
{
	switch(ob.object_type)
	{
	case WorldObject::ObjectType_Generic: this->objectTypeLabel->setText("Generic"); break;
	case WorldObject::ObjectType_Hypercard: this->objectTypeLabel->setText("Hypercard"); break;
	case WorldObject::ObjectType_VoxelGroup: this->objectTypeLabel->setText("Voxel Group"); break;
	}

	this->cloned_materials.resize(ob.materials.size());
	for(size_t i=0; i<ob.materials.size(); ++i)
		this->cloned_materials[i] = ob.materials[i]->clone();

	const std::string creator_name = !ob.creator_name.empty() ? ob.creator_name :
		(ob.creator_id.valid() ? ("user id: " + ob.creator_id.toString()) : "[Unknown]");

	this->createdByLabel->setText(QtUtils::toQString(creator_name));
	this->createdTimeLabel->setText(QtUtils::toQString(ob.created_time.timeAgoDescription()));

	this->selected_mat_index = selected_mat_index_;
	this->modelFileSelectWidget->setFilename(QtUtils::toQString(ob.model_url));
	this->scriptFileSelectWidget->setFilename(QtUtils::toQString(ob.script_url));

	{
		SignalBlocker b(this->contentTextEdit);
		this->contentTextEdit->setText(QtUtils::toQString(ob.content));
	}
	{
		SignalBlocker b(this->targetURLLineEdit);
		this->targetURLLineEdit->setText(QtUtils::toQString(ob.target_url));
	}

	SignalBlocker::setValue(this->posXDoubleSpinBox, ob.pos.x);
	SignalBlocker::setValue(this->posYDoubleSpinBox, ob.pos.y);
	SignalBlocker::setValue(this->posZDoubleSpinBox, ob.pos.z);

	SignalBlocker::setValue(this->scaleXDoubleSpinBox, ob.scale.x);
	SignalBlocker::setValue(this->scaleYDoubleSpinBox, ob.scale.y);
	SignalBlocker::setValue(this->scaleZDoubleSpinBox, ob.scale.z);
	
	SignalBlocker::setValue(this->rotAxisXDoubleSpinBox, ob.axis.x);
	SignalBlocker::setValue(this->rotAxisYDoubleSpinBox, ob.axis.y);
	SignalBlocker::setValue(this->rotAxisZDoubleSpinBox, ob.axis.z);
	SignalBlocker::setValue(this->rotAngleDoubleSpinBox, ob.angle);

	WorldMaterialRef selected_mat;
	if(selected_mat_index >= 0 && selected_mat_index < (int)ob.materials.size())
		selected_mat = ob.materials[selected_mat_index];
	else
		selected_mat = new WorldMaterial();
	
	if(ob.object_type == WorldObject::ObjectType_Hypercard)
	{
		this->materialsGroupBox->hide();
		this->modelLabel->hide();
		this->modelFileSelectWidget->hide();
	}
	else if(ob.object_type == WorldObject::ObjectType_VoxelGroup)
	{
		this->materialsGroupBox->show();
		this->modelLabel->hide();
		this->modelFileSelectWidget->hide();
	}
	else
	{
		this->materialsGroupBox->show();
		this->modelLabel->show();
		this->modelFileSelectWidget->show();
	}

	if(ob.object_type == WorldObject::ObjectType_VoxelGroup || ob.object_type == WorldObject::ObjectType_Generic)
	{
		this->matEditor->setFromMaterial(*selected_mat);

		// Set materials combobox
		SignalBlocker blocker(this->materialComboBox);
		this->materialComboBox->clear();
		for(size_t i=0; i<ob.materials.size(); ++i)
			this->materialComboBox->addItem(QtUtils::toQString("Material " + toString(i)), (int)i);

		this->materialComboBox->setCurrentIndex(selected_mat_index);
	}


	this->targetURLLabel->setVisible(ob.object_type == WorldObject::ObjectType_Hypercard);
	this->targetURLLineEdit->setVisible(ob.object_type == WorldObject::ObjectType_Hypercard);
	this->visitURLLabel->setVisible(ob.object_type == WorldObject::ObjectType_Hypercard && !ob.target_url.empty());
}


void ObjectEditor::updateObjectPos(const WorldObject& ob)
{
	SignalBlocker::setValue(this->posXDoubleSpinBox, ob.pos.x);
	SignalBlocker::setValue(this->posYDoubleSpinBox, ob.pos.y);
	SignalBlocker::setValue(this->posZDoubleSpinBox, ob.pos.z);
}


void ObjectEditor::toObject(WorldObject& ob_out)
{
	ob_out.model_url  = QtUtils::toIndString(this->modelFileSelectWidget->filename());
	ob_out.script_url = QtUtils::toIndString(this->scriptFileSelectWidget->filename());
	ob_out.content    = QtUtils::toIndString(this->contentTextEdit->toPlainText());
	ob_out.target_url    = QtUtils::toIndString(this->targetURLLineEdit->text());

	ob_out.pos.x = this->posXDoubleSpinBox->value();
	ob_out.pos.y = this->posYDoubleSpinBox->value();
	ob_out.pos.z = this->posZDoubleSpinBox->value();

	ob_out.scale.x = (float)this->scaleXDoubleSpinBox->value();
	ob_out.scale.y = (float)this->scaleYDoubleSpinBox->value();
	ob_out.scale.z = (float)this->scaleZDoubleSpinBox->value();

	ob_out.axis.x = (float)this->rotAxisXDoubleSpinBox->value();
	ob_out.axis.y = (float)this->rotAxisYDoubleSpinBox->value();
	ob_out.axis.z = (float)this->rotAxisZDoubleSpinBox->value();
	ob_out.angle  = (float)this->rotAngleDoubleSpinBox->value();

	if(ob_out.axis.length() < 1.0e-5f)
	{
		ob_out.axis = Vec3f(0,0,1);
		ob_out.angle = 0;
	}

	if(selected_mat_index >= cloned_materials.size())
	{
		cloned_materials.resize(selected_mat_index + 1);
		for(size_t i=0; i<cloned_materials.size(); ++i)
			if(cloned_materials[i].isNull())
				cloned_materials[i] = new WorldMaterial();
	}

	this->matEditor->toMaterial(*cloned_materials[selected_mat_index]);

	ob_out.materials.resize(cloned_materials.size());
	for(size_t i=0; i<cloned_materials.size(); ++i)
		ob_out.materials[i] = cloned_materials[i]->clone();

}


void ObjectEditor::setControlsEnabled(bool enabled)
{
	this->setEnabled(enabled);
}


void ObjectEditor::setControlsEditable(bool editable)
{
	this->modelFileSelectWidget->setReadOnly(!editable);
	this->scriptFileSelectWidget->setReadOnly(!editable);
	this->contentTextEdit->setReadOnly(!editable);
	this->targetURLLineEdit->setReadOnly(!editable);

	this->posXDoubleSpinBox->setReadOnly(!editable);
	this->posYDoubleSpinBox->setReadOnly(!editable);
	this->posZDoubleSpinBox->setReadOnly(!editable);

	this->scaleXDoubleSpinBox->setReadOnly(!editable);
	this->scaleYDoubleSpinBox->setReadOnly(!editable);
	this->scaleZDoubleSpinBox->setReadOnly(!editable);

	this->rotAxisXDoubleSpinBox->setReadOnly(!editable);
	this->rotAxisYDoubleSpinBox->setReadOnly(!editable);
	this->rotAxisZDoubleSpinBox->setReadOnly(!editable);
	this->rotAngleDoubleSpinBox->setReadOnly(!editable);

	this->matEditor->setControlsEditable(editable);
}


void ObjectEditor::on_visitURLLabel_linkActivated(const QString&)
{
	std::string url = QtUtils::toStdString(this->targetURLLineEdit->text());
	if(StringUtils::containsString(url, "://"))
	{
		// URL already has protocol prefix
		const std::string protocol = url.substr(0, url.find("://", 0));
		if(protocol == "http" || protocol == "https")
		{
			QDesktopServices::openUrl(QtUtils::toQString(url));
		}
		else
		{
			// Don't open this URL, might be something potentially unsafe like a file on disk
			QErrorMessage m;
			m.showMessage("This URL is potentially unsafe and will not be opened.");
			m.exec();
		}
	}
	else
	{
		url = "http://" + url;
		QDesktopServices::openUrl(QtUtils::toQString(url));
	}
	
}


void ObjectEditor::on_materialComboBox_currentIndexChanged(int index)
{
	this->selected_mat_index = index;

	if(index < this->cloned_materials.size())
		this->matEditor->setFromMaterial(*this->cloned_materials[index]);
}


void ObjectEditor::on_newMaterialPushButton_clicked(bool checked)
{
	this->selected_mat_index = this->materialComboBox->count();

	this->materialComboBox->addItem(QtUtils::toQString("Material " + toString(selected_mat_index)), selected_mat_index);

	{
		SignalBlocker blocker(this->materialComboBox);
		this->materialComboBox->setCurrentIndex(this->selected_mat_index);
	}

	this->cloned_materials.push_back(new WorldMaterial());
	this->matEditor->setFromMaterial(*this->cloned_materials.back());

	emit objectChanged();
}


void ObjectEditor::targetURLChanged()
{
	this->visitURLLabel->setVisible(!this->targetURLLineEdit->text().isEmpty());
}


void ObjectEditor::materialSelectedInBrowser(const std::string& path)
{
	// Load material
	try
	{
		WorldMaterialRef mat = WorldMaterial::loadFromXMLOnDisk(path);

		if(selected_mat_index >= 0 && selected_mat_index < this->cloned_materials.size())
		{
			this->cloned_materials[this->selected_mat_index] = mat;
			this->matEditor->setFromMaterial(*mat);

			emit objectChanged();
		}
	}
	catch(Indigo::Exception& e)
	{
		QErrorMessage m;
		m.showMessage("Error while opening material: " + QtUtils::toQString(e.what()));
		m.exec();
	}
}
