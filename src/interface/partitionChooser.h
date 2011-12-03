#ifndef PARTITIONCHOOSER_H_
#define PARTITIONCHOOSER_H_

#include "evt_partitionChooser.h"

class PartitionChooser {
public:
	enum Exception {
		ERR_CHKBUTTON_NOT_FOUND
	};
	enum MountExceptionType {
		MOUNT_FAILED,
		UMOUNT_FAILED,
		MOUNT_ERR_NO_FSTAB,
		SUB_MOUNT_FAILED,
		SUB_UMOUNT_FAILED
	};
	virtual void setEventListener(EventListener_partitionChooser& eventListener)=0;
	virtual std::string getSelectedDevice()=0;
	virtual void addPartitionSelectorItem(std::string const& device, std::string const& type, std::string const& label)=0;
	virtual void clearPartitionSelector()=0;
	virtual void addSubmountpoint(std::string const& mountpoint, bool isMounted)=0;
	virtual void removeAllSubmountpoints()=0;
	virtual void showErrorMessage(MountExceptionType type)=0;
	virtual void hide()=0;
	virtual void show()=0;
	virtual void setSubmountpointSelectionState(std::string const& submountpoint, bool new_isSelected)=0;
	virtual void setIsMounted(bool isMounted)=0;
	virtual void setIsStartedManually(bool val)=0;
};

#endif
