#pragma once
#include "uart/ProtocolSender.h"
#include "list.h"
/*
*此文件由GUI工具生成
*文件功能：用于处理用户的逻辑相应代码
*功能说明：
*========================onButtonClick_XXXX
当页面中的按键按下后系统会调用对应的函数，XXX代表GUI工具里面的[ID值]名称，
如Button1,当返回值为false的时候系统将不再处理这个按键，返回true的时候系统将会继续处理此按键。比如SYS_BACK.
*========================onSlideWindowItemClick_XXXX(int index) 
当页面中存在滑动窗口并且用户点击了滑动窗口的图标后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如slideWindow1;index 代表按下图标的偏移值
*========================onSeekBarChange_XXXX(int progress) 
当页面中存在滑动条并且用户改变了进度后系统会调用此函数,XXX代表GUI工具里面的[ID值]名称，
如SeekBar1;progress 代表当前的进度值
*========================ogetListItemCount_XXXX() 
当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表的总数目,XXX代表GUI工具里面的[ID值]名称，
如List1;返回值为当前列表的总条数
*========================oobtainListItemData_XXXX(ZKListView::ZKListItem *pListItem, int index)
 当页面中存在滑动列表的时候，更新的时候系统会调用此接口获取列表当前条目下的内容信息,XXX代表GUI工具里面的[ID值]名称，
如List1;pListItem 是贴图中的单条目对象，index是列表总目的偏移量。具体见函数说明
*========================常用接口===============
*LOGD(...)  打印调试信息的接口
*mTextXXXPtr->setText("****") 在控件TextXXX上显示文字****
*mButton1Ptr->setSelected(true); 将控件mButton1设置为选中模式，图片会切换成选中图片，按钮文字会切换为选中后的颜色
*mSeekBarPtr->setProgress(12) 在控件mSeekBar上将进度调整到12
*mListView1Ptr->refreshListView() 让mListView1 重新刷新，当列表数据变化后调用
*mDashbroadView1Ptr->setTargetAngle(120) 在控件mDashbroadView1上指针显示角度调整到120度
*
* 在Eclipse编辑器中  使用 “alt + /”  快捷键可以打开智能提示
*/

#ifdef SUPPORT_PLAYER_MODULE
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>
#include "hotplugdetect.h"
#include "statusbarconfig.h"

#define DIR_TYPE        "文件夹"
#define FILE_TYPE       "文件"

typedef struct _FileTree_t
{
    char name[256];
    char time[32];
    int dirFlag;
    int depth;
    int childCnt;
    long size;
    list_t headNodeList;
    list_t childNodeList;
} FileTree_t;

typedef struct _FileChildInfo_t
{
	char name[256];
	char time[32];
	int dirFlag;
	long size;
} FileChildInfo_t;

typedef enum _FileFilter_e
{
	MP4_FILTER = 0,
	AVI_FILTER,
	WAV_FILTER,
	FLV_FILTER,
	MKV_FILTER,
	MP3_FILTER,
	MOV_FILTER,
	RM_FILTER,
	RMVB_FILTER,
	WMV_FTLTER,
	VOB_FILTER,
	MPG_FILTER,
	TS_FILTER,
	DAT_FILTER,
	M4A_FILTER,
	MKA_FILTER,
	RA_FILTER,
	APE_FILTER,
	ADTS_FILTER,
	ASF_FILTER,
	WMA_FILTER,
	WEBM_FILTER,
	DIVX_FILTER,
	OGM_FILTER,
	TRP_FILTER,
	TP_FILTER,
	PNG_FILTER,
//	BMP_FILTER,
	JPG_FILTER,
	FILE_FILTER_NUM
} FileFilter_e;

static FileTree_t *g_pFileRoot = NULL;
static FileTree_t *g_pCurFileNode = NULL;
static std::vector<FileChildInfo_t> sFileChildren;
static Mutex lLock;
static char g_udiskPath[256] = {0};		// udisk root path
static char g_curDir[256] = {0};		// last access path
static char g_selectPath[256] = {0};	// current dir full path
static int bigMonth[] = {1, 3, 5, 7, 8, 10, 12};
static char *g_pFileFilter[FILE_FILTER_NUM] = {
		".mp4",
		".avi",
		".wav",
		".flv",
		".mkv",
		".mp3",
		".mov",
		".rm",
		".rmvb",
		".wmv",
		".vob",
		".mpg",
		".ts",
		".dat",
		".m4a",
		".mka",
		".ra",
		".ape",
		".adts",
		".asf",
		".wma",
		".webm",
		".divx",
		".ogm",
		".trp",
		".tp",
		"png",
//		"bmp",
		"jpg"
};

char *getDayOfWeek(int day)
{
    switch (day)
    {
        case 0:
            return "周日";
        case 1:
            return "周一";
        case 2:
            return "周二";
        case 3:
            return "周三";
        case 4:
            return "周四";
        case 5:
            return "周五";
        case 6:
            return "周六";
    }

    printf("invalid day %d\n", day);
    return NULL;
}

// month: [1, 12]
int getBigMonthCount(int month)
{
    int i = 0;
    int count = 0;

    for (i = 0; i < sizeof(bigMonth)/sizeof(int); i++)
    {
        if (month > bigMonth[i])
            count++;
        else
            break;
    }

    //printf("count is %d, curMonth is %d\n", count, month);
    return count;
}

// month: [1, 12]
int getDateOfMonth(int year, int month, int date)
{
    int febDays = 28;
    int monthDays = 30;
    int curMonthDate = 0;

    if ((year%400 == 0) || (year%100 != 0 && year%4 == 0))
    {
        febDays = 29;
    }

    if (month > 2)
    {
        curMonthDate = date - ((month-2)*monthDays + febDays + getBigMonthCount(month));
    }
    else if (month > 1)
    {
        curMonthDate = date - ((month-1)*monthDays + getBigMonthCount(month));
    }
    else
    {
        curMonthDate = date;
    }

    //printf("date is %d, curMonthDate is %d, febDays is %d\n", date, curMonthDate, febDays);
    return curMonthDate;
}

static int FilterFiles(char *pFileName, char **pFilter, int filterCnt)
{
    int i = 0;
    int fileNameLen = strlen(pFileName);

    for(; i < filterCnt; i++)
    {
        int filterLen = strlen(pFilter[i]);

        if (fileNameLen > filterLen)
        {
            if (!strncmp(pFileName+(fileNameLen-filterLen), pFilter[i], filterLen))
                return 0;
        }
    }

    return -1;
}

int InitFileTreeRoot(FileTree_t **root, char *pRootName)
{
	if (*root)
	{
		printf("file tree is not empty, please clear first\n");
		return -1;
	}

	*root = (FileTree_t*)malloc(sizeof(FileTree_t));
    memset(*root, 0, sizeof(FileTree_t));
    (*root)->dirFlag = 1;
    strcpy((*root)->name, pRootName);
    INIT_LIST_HEAD(&(*root)->headNodeList);
    INIT_LIST_HEAD(&(*root)->childNodeList);

    return 0;
}

int CreateFileTree(FileTree_t *root)
{
    DIR *pDir =  NULL;
    struct dirent *ent;
    struct stat statbuf;
    struct tm *modifytm;
    FileTree_t *child = NULL;
    int dirFlag = 0;

    pDir = opendir(root->name);
    if (!pDir)
    {
        //printf("%s directory is not exist or open failed\n", root->name);
        return -1;
    }

    lstat(root->name, &statbuf);
    modifytm = localtime(&(statbuf.st_mtime));
    root->size = statbuf.st_size;
    sprintf(root->time, "%d/%d/%d %d:%d:%d %s", modifytm->tm_year+1900, modifytm->tm_mon+1,
            getDateOfMonth(modifytm->tm_year+1900, modifytm->tm_mon+1, modifytm->tm_yday+1),
            modifytm->tm_hour, modifytm->tm_min, modifytm->tm_sec,
            getDayOfWeek(modifytm->tm_wday));

    while ((ent=readdir(pDir)) != NULL)
    {
        if (ent->d_type & DT_DIR)
        {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                continue;

            dirFlag = 1;
        }
        else
        {
        	// file filter
			if (FilterFiles(ent->d_name, g_pFileFilter, FILE_FILTER_NUM))
				continue;

            dirFlag = 0;
        }

        child = (FileTree_t*)malloc(sizeof(FileTree_t));
        memset(child, 0, sizeof(FileTree_t));
        INIT_LIST_HEAD(&child->headNodeList);;
        INIT_LIST_HEAD(&child->childNodeList);
        if (!strcmp(root->name, "/"))
            sprintf(child->name, "%s%s", root->name, ent->d_name);
        else
            sprintf(child->name, "%s/%s", root->name, ent->d_name);
        child->depth = root->depth + 1;
        root->childCnt++;
        list_add_tail(&child->childNodeList, &root->headNodeList);

        if (dirFlag)
        {
            child->dirFlag = 1;
            CreateFileTree(child);
        }
        else
        {
            child->dirFlag = 0;
            lstat(child->name, &statbuf);
            modifytm = localtime(&(statbuf.st_mtime));
            child->size = statbuf.st_size;
            sprintf(child->time, "%d/%d/%d %d:%d:%d %s", modifytm->tm_year+1900, modifytm->tm_mon+1,
                    getDateOfMonth(modifytm->tm_year+1900, modifytm->tm_mon+1, modifytm->tm_yday+1),
                    modifytm->tm_hour, modifytm->tm_min, modifytm->tm_sec,
                    getDayOfWeek(modifytm->tm_wday));
        }
    }

    closedir(pDir);
    pDir = NULL;

    return 0;
}

// sort by asc code by file type and then by name in ascending order
void SortFileTree(FileTree_t *root)
{
	FileTree_t *pos = NULL;
	FileTree_t *posN = NULL;
	int i = 0;

	if (!root)
	{
		printf("FileTree is not exist\n");
		return;
	}

//	printf("name:%-50sdirFlag:%-5dsize:%-8ld KB time:%-32sdepth:%-5dchildCnt:%-5d\n", root->name, root->dirFlag, (root->size+1023)/1024,
//			root->time, root->depth, root->childCnt);

	if (list_empty(&root->headNodeList))
	{
		return;
	}

	// do resort
	for (i = 0; i < root->childCnt; i++)
	{
		list_for_each_entry_safe(pos, posN, &root->headNodeList, childNodeList)
		{
			if (pos->childNodeList.prev != &root->headNodeList)
			{
				// compare with the prev
				FileTree_t *front = list_entry(pos->childNodeList.prev, FileTree_t, childNodeList);

				if ((front->dirFlag && pos->dirFlag) || (!front->dirFlag && !pos->dirFlag))
				{
					if (strcmp(pos->name, front->name) < 0)
					{
						list_del(&pos->childNodeList);
						list_add_tail(&pos->childNodeList, &front->childNodeList);
					}
				}
				else if (!front->dirFlag && pos->dirFlag)
				{
					list_del(&pos->childNodeList);
					list_add_tail(&pos->childNodeList, &front->childNodeList);
				}
			}
		}
	}

	list_for_each_entry(pos, &root->headNodeList, childNodeList)
	{
		SortFileTree(pos);
	}
}

void BrowseFileTree(FileTree_t *root)
{
    FileTree_t *pos = NULL;

    if (!root)
    {
        printf("FileTree is not exist\n");
        return;
    }

    printf("name:%-50sdirFlag:%-5dsize:%-8ld KB time:%-32sdepth:%-5dchildCnt:%-5d\n", root->name, root->dirFlag, (root->size+1023)/1024,
            root->time, root->depth, root->childCnt);

    if (list_empty(&root->headNodeList))
    {
        return;
    }

    list_for_each_entry(pos, &root->headNodeList, childNodeList)
    {
        BrowseFileTree(pos);
    }
}

FileTree_t *FindFileTreeNode(FileTree_t *root, char *name)
{
    FileTree_t *pos = NULL;
    FileTree_t *result = NULL;

    if (!root)
        return NULL;

    if (!strcmp(root->name, name))
        return root;

    if (list_empty(&root->headNodeList))
    {
        //printf("%s is not exist in %s\n", name, root->name);
        return NULL;
    }

    list_for_each_entry(pos, &(root->headNodeList), childNodeList)
    {
        if ((result = FindFileTreeNode(pos, name)) != NULL)
            return result;
    }

    return NULL;
}

int InsertFileTreeNode(FileTree_t *father, FileTree_t *child)
{
    child->depth = father->depth + 1;
    father->childCnt++;
    INIT_LIST_HEAD(&child->childNodeList);
    list_add_tail(&child->childNodeList, &father->headNodeList);

    return 0;
}

int DestroyFileTree(FileTree_t *root)
{
    FileTree_t *pos = NULL;
    FileTree_t *posN = NULL;

    if (!root)
    {
        printf("root is NULL\n");
        return 0;
    }

    list_for_each_entry_safe(pos, posN, &root->headNodeList, childNodeList)
    {
        DestroyFileTree(pos);
    }

    free(root);
    root = NULL;

    return 0;
}

int DeleteFileTreeNode(FileTree_t *father, FileTree_t *child)
{
    list_del(&child->childNodeList);
    child->depth = 0;
    father->childCnt--;

    DestroyFileTree(child);
    return 0;
}

void ShowCurrentDir(FileTree_t *pRoot, std::vector<FileChildInfo_t>* pChildInfo)
{
	FileTree_t *pos = NULL;

	if (!pRoot)
		return;

	pChildInfo->clear();
	mTextview_curPathPtr->setText(pRoot->name);

	list_for_each_entry(pos, &(pRoot->headNodeList), childNodeList)
	{
		FileChildInfo_t childInfo;
		char fileName[256];
		char *p = NULL;
		memset(fileName, 0, sizeof(fileName));
		memset(&childInfo, 0, sizeof(FileChildInfo_t));
		strcpy(fileName, pos->name);
		p = strrchr(fileName, '/');
		*p = 0;
		strcpy(childInfo.name, pos->name+strlen(fileName)+1);
		strcpy(childInfo.time, pos->time);
		childInfo.dirFlag = pos->dirFlag;
		childInfo.size = pos->size;
		pChildInfo->push_back(childInfo);
	}
	mListview_playlistPtr->setSelection(0);
	mListview_playlistPtr->refreshListView();
}

void GetPrevFile(char *pCurFileFullName, char *pPrevFileFullName, int prevFilePathLen)
{
	char tmpPath[256];
	int i = 0;
	int findCurFile = 0;

	for (i = 0; i < sFileChildren.size(); i++)
	{
		FileChildInfo_t &childInfo = sFileChildren.at(i);
		memset(tmpPath, 0, sizeof(tmpPath));
		sprintf(tmpPath, "%s/%s", g_curDir, childInfo.name);

		if (!strcmp(tmpPath, pCurFileFullName))
		{
			findCurFile = 1;
			break;
		}
	}

	while (findCurFile)
	{
		i = (--i + sFileChildren.size()) % sFileChildren.size();
		FileChildInfo_t &childInfo = sFileChildren.at(i);

		if (!childInfo.dirFlag)
		{
			memset(pPrevFileFullName, 0, prevFilePathLen);
			sprintf(pPrevFileFullName, "%s/%s", g_curDir, childInfo.name);
			break;
		}
	}
}

void GetNextFile(char *pCurFileFullName, char *pNextFileFullName, int nextFilePathLen)
{
	char tmpPath[256];
	int i = 0;
	int findCurFile = 0;

	for (i = 0; i < sFileChildren.size(); i++)
	{
		FileChildInfo_t &childInfo = sFileChildren.at(i);
		memset(tmpPath, 0, sizeof(tmpPath));
		sprintf(tmpPath, "%s/%s", g_curDir, childInfo.name);

		if (!strcmp(tmpPath, pCurFileFullName))
		{
			findCurFile = 1;
			break;
		}
	}

	while (findCurFile)
	{
		i = (++i + sFileChildren.size()) % sFileChildren.size();
		FileChildInfo_t &childInfo = sFileChildren.at(i);

		if (!childInfo.dirFlag)
		{
			memset(pNextFileFullName, 0, nextFilePathLen);
			sprintf(pNextFileFullName, "%s/%s", g_curDir, childInfo.name);
			break;
		}
	}
}

int IsMediaStreamFile(char *pCurFileFullName)		// judge if it's audio/video file or picture file
{
	int fileNameLen = strlen(pCurFileFullName);

	for(int i = PNG_FILTER; i < FILE_FILTER_NUM; i++)
	{
		int filterLen = strlen(g_pFileFilter[i]);

		if (fileNameLen > filterLen)
		{
			if (!strncmp(pCurFileFullName+(fileNameLen-filterLen), g_pFileFilter[i], filterLen))
				return 0;
		}
	}

	return 1;
}

void DetectUsbStatus(UsbParam_t *pstUsbParam)		// action 0, connect; action 1, disconnect
{
	if (!pstUsbParam->action)
		EASYUICONTEXT->goHome();
}
#endif

/**
 * 注册定时器
 * 填充数组用于注册定时器
 * 注意：id不能重复
 */
static S_ACTIVITY_TIMEER REGISTER_ACTIVITY_TIMER_TAB[] = {
	//{0,  6000}, //定时器id=0, 时间间隔6秒
	//{1,  1000},
};

/**
 * 当界面构造时触发
 */
static void onUI_init(){
    //Tips :添加 UI初始化的显示代码到这里,如:mText1Ptr->setText("123");

}

/**
 * 当切换到该界面时触发
 */
static void onUI_intent(const Intent *intentPtr) {
    if (intentPtr != NULL) {
        //TODO
    }
#ifdef SUPPORT_PLAYER_MODULE
    FileTree_t *pos = NULL;
    // init usb dev

    if (!SSTAR_GetUsbCurrentStatus())
	{
    	mTextview_tipsPtr->setVisible(true);
    	mButton_updirPtr->setVisible(false);
		mTextview_playlist_titlePtr->setVisible(false);
		mListview_playlistPtr->setVisible(false);
		mTextview_curPathPtr->setVisible(false);
		mTextview_curPath_titlePtr->setVisible(false);
		return;
	}
    else
    {
    	SSTAR_GetUsbPath((char*)g_udiskPath, sizeof(g_udiskPath));
    	SSTAR_RegisterUsbListener(DetectUsbStatus);

    	mTextview_tipsPtr->setVisible(false);
    	mTextview_playlist_titlePtr->setVisible(true);
		mListview_playlistPtr->setVisible(true);
		mTextview_curPathPtr->setVisible(true);
		mTextview_curPath_titlePtr->setVisible(true);

		if (!strlen(g_curDir) || !strcmp(g_curDir, g_udiskPath))
			mButton_updirPtr->setVisible(false);
    }

    // create filetree according to rootpath
	if (InitFileTreeRoot(&g_pFileRoot, g_udiskPath))
		return -1;

	CreateFileTree(g_pFileRoot);

	// sort file tree
	SortFileTree(g_pFileRoot);

	g_pCurFileNode = g_pFileRoot;
	memset(g_selectPath, 0, sizeof(g_selectPath));
	//strcpy(g_selectPath, g_udiskPath);
	memset(g_curDir, 0, sizeof(g_curDir));
	strcpy(g_curDir, g_udiskPath);
	printf("udisk root path is %s\n", g_curDir);

	// init listview
	ShowCurrentDir(g_pCurFileNode, &sFileChildren);
#endif
}

/*
 * 当界面显示时触发
 */
static void onUI_show() {

}

/*
 * 当界面隐藏时触发
 */
static void onUI_hide() {

}

/*
 * 当界面完全退出时触发
 */
static void onUI_quit() {
#ifdef SUPPORT_PLAYER_MODULE
	DestroyFileTree(g_pFileRoot);
	g_pFileRoot = NULL;
	SSTAR_UnRegisterUsbListener(DetectUsbStatus);
#endif

	ShowStatusBar(1, 0, 0);
}

/**
 * 串口数据回调接口
 */
static void onProtocolDataUpdate(const SProtocolData &data) {

}

/**
 * 定时器触发函数
 * 不建议在此函数中写耗时操作，否则将影响UI刷新
 * 参数： id
 *         当前所触发定时器的id，与注册时的id相同
 * 返回值: true
 *             继续运行当前定时器
 *         false
 *             停止运行当前定时器
 */
static bool onUI_Timer(int id){
	switch (id) {

		default:
			break;
	}
    return true;
}

/**
 * 有新的触摸事件时触发
 * 参数：ev
 *         新的触摸事件
 * 返回值：true
 *            表示该触摸事件在此被拦截，系统不再将此触摸事件传递到控件上
 *         false
 *            触摸事件将继续传递到控件上
 */
static bool onplaylistActivityTouchEvent(const MotionEvent &ev) {
    switch (ev.mActionStatus) {
		case MotionEvent::E_ACTION_DOWN://触摸按下
			//LOGD("时刻 = %ld 坐标  x = %d, y = %d", ev.mEventTime, ev.mX, ev.mY);
			break;
		case MotionEvent::E_ACTION_MOVE://触摸滑动
			break;
		case MotionEvent::E_ACTION_UP:  //触摸抬起
			break;
		default:
			break;
	}
	return false;
}
static bool onButtonClick_sys_back(ZKButton *pButton) {
    //LOGD(" ButtonClick sys_back !!!\n");
    return false;
}

static int getListItemCount_Listview_playlist(const ZKListView *pListView) {
    //LOGD("getListItemCount_Listview_playlist !\n");
	int childCnt = 0;
#ifdef SUPPORT_PLAYER_MODULE
	childCnt = sFileChildren.size();
	// printf("childcnt is %d\n", childCnt);
#endif
    return childCnt;
}

static void obtainListItemData_Listview_playlist(ZKListView *pListView,ZKListView::ZKListItem *pListItem, int index) {
    //LOGD(" obtainListItemData_ Listview_playlist  !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	ZKListView::ZKListSubItem *pIconItem = pListItem->findSubItemByID(ID_PLAYLIST_SubItem_icon);
	ZKListView::ZKListSubItem *pTypeItem = pListItem->findSubItemByID(ID_PLAYLIST_SubItem_filetype);
	ZKListView::ZKListSubItem *pNameItem = pListItem->findSubItemByID(ID_PLAYLIST_SubItem_filename);
	ZKListView::ZKListSubItem *pSizeItem = pListItem->findSubItemByID(ID_PLAYLIST_SubItem_filesize);
	ZKListView::ZKListSubItem *pModifyTimeItem = pListItem->findSubItemByID(ID_PLAYLIST_SubItem_modifytime);
	char szSize[32];
	memset(szSize, 0, sizeof(szSize));

	lLock.lock();
	const FileChildInfo_t &childInfo = sFileChildren.at(index);
	lLock.unlock();

	// set file type icon
	if (childInfo.dirFlag)
	{
		pIconItem->setBackgroundPic("player/folder.png");
		pTypeItem->setText(DIR_TYPE);
	}
	else
	{
		pIconItem->setBackgroundPic("player/file.png");
		pTypeItem->setText(FILE_TYPE);
	}

	pNameItem->setText(childInfo.name);
	sprintf(szSize, "%d KB", (childInfo.size+1023)/1024);
	pSizeItem->setText(szSize);
	pModifyTimeItem->setText(childInfo.time);
#endif
}

static void onListItemClick_Listview_playlist(ZKListView *pListView, int index, int id) {
    //LOGD(" onListItemClick_ Listview_playlist  !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	lLock.lock();
	const FileChildInfo_t &childInfo = sFileChildren.at(index);
	lLock.unlock();

//	char tmp[256] = {0};
//	strcpy(tmp, g_selectPath);
	printf("dir path is %s\n", g_curDir);
	memset(g_selectPath, 0, sizeof(g_selectPath));
	sprintf(g_selectPath, "%s/%s", g_curDir, childInfo.name);
	//strcat(g_selectPath, childInfo.name);
	//sprintf(g_selectPath, "%s/%s", test, childInfo.name);
	printf("cur selectPath is %s, filename is %s\n", g_selectPath, childInfo.name);

	if (childInfo.dirFlag)
	{
		// update children list
		memset(g_curDir, 0 , sizeof(g_curDir));
		strcpy(g_curDir, g_selectPath);
		g_pCurFileNode = FindFileTreeNode(g_pFileRoot, g_curDir);

		if (g_pCurFileNode)
		{
			ShowCurrentDir(g_pCurFileNode, &sFileChildren);

			if (g_pCurFileNode->depth)
				mButton_updirPtr->setVisible(true);
		}
	}
	else
	{
		// enter to player
		Intent* intent = new Intent();
		intent->putExtra("filepath", g_selectPath);
		EASYUICONTEXT->openActivity("playerActivity", intent);
	}
#endif
}
static bool onButtonClick_Button_updir(ZKButton *pButton) {
    //LOGD(" ButtonClick Button_updir !!!\n");
#ifdef SUPPORT_PLAYER_MODULE
	if (g_pCurFileNode->depth)
	{
		printf("cur dirpath is %s\n", g_curDir);
		char *p = strrchr(g_curDir, '/');
		*p = 0;
		printf("cur dirpath is %s\n", g_curDir);
		g_pCurFileNode = FindFileTreeNode(g_pFileRoot, g_curDir);
		ShowCurrentDir(g_pCurFileNode, &sFileChildren);

		if (!g_pCurFileNode->depth)
			mButton_updirPtr->setVisible(false);
	}
#endif

    return false;
}
