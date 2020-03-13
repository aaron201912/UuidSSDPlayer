/*
 * ZKTextView.h
 *
 *  Created on: Jun 10, 2017
 *      Author: guoxs
 */

#ifndef _CONTROL_ZKTEXTVIEW_H_
#define _CONTROL_ZKTEXTVIEW_H_

#include "ZKBase.h"

class ZKTextViewPrivate;

/**
 * @brief 文本控件
 */
class ZKTextView : public ZKBase {
	ZK_DECLARE_PRIVATE(ZKTextView)

public:
	ZKTextView(ZKBase *pParent);
	virtual ~ZKTextView();

	/**
	 * @brief 设置string文本
	 */
	void setText(const std::string &text);

	/**
	 * @brief 设置字符串文本
	 */
	void setText(const char *text);

	/**
	 * @brief 设置字符文本
	 */
	void setText(char text);

	/**
	 * @brief 设置数字文本
	 */
	void setText(int text);

	/**
	 * @brief 获取文本内容
	 */
	const std::string& getText() const;

	/**
	 * @brief 支持多国语言设置接口
	 */
	void setTextTr(const char *name);

	void reloadTextTr();

	/**
	 * @brief 设置文本颜色
	 * @param color 颜色值为0x RGB，不支持alpha
	 */
	void setTextColor(int color);

public:
	/**
	 * @brief 文本内容改变监听接口
	 */
	class ITextChangeListener {
	public:
		virtual ~ITextChangeListener() { }
		virtual void onTextChanged(ZKTextView *pTextView, const std::string &text) = 0;
	};

	void setTextChangeListener(ITextChangeListener *pListener);

protected:
	ZKTextView(ZKBase *pParent, ZKBasePrivate *pBP);

	virtual void onBeforeCreateWindow(const Json::Value &json);
	virtual void onAfterCreateWindow(const Json::Value &json);
	virtual const char* getClassName() const { return ZK_TEXTVIEW; }

	virtual void onDraw(ZKCanvas *pCanvas);
	virtual void onTimer(int id);

private:
	void _section_(zk) parseTextViewAttributeFromJson(const Json::Value &json);
};

#endif /* _CONTROL_ZKTEXTVIEW_H_ */
