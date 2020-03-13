/*
 * ZKButton.h
 *
 *  Created on: Jun 10, 2017
 *      Author: guoxs
 */

#ifndef _CONTROL_ZKBUTTON_H_
#define _CONTROL_ZKBUTTON_H_

#include "ZKTextView.h"

class ZKButtonPrivate;

/**
 * @brief 按钮控件
 */
class ZKButton : public ZKTextView {
	ZK_DECLARE_PRIVATE(ZKButton)

public:
	ZKButton(ZKBase *pParent);
	virtual ~ZKButton();

protected:
	ZKButton(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual const char* getClassName() const { return ZK_BUTTON; }

	virtual void onDraw(ZKCanvas *pCanvas);

private:
	void _section_(zk) parseButtonAttributeFromJson(const Json::Value &json);
};

#endif /* _CONTROL_ZKBUTTON_H_ */
