/*
 * $Log: xinfo.h,v $
 * Revision 1.4  2000/04/20 05:31:32  krishna
 * Added headers.
 *
 * Revision 1.3  1998/07/17  08:05:16  krishna
 * Added RCS header
 *
 */

#ifndef XINFO_H
#define XINFO_H

#include <stdio.h>
#include <math.h>
#include <time.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <X11/keysymdef.h>
#include <X11/Xmu/StdCmap.h>
#include <Xm/Xm.h>
#include <Xm/AccColorT.h>
#include <Xm/AccTextT.h>
#include <Xm/ActivatableT.h>
#include <Xm/ArrowB.h>
#include <Xm/ArrowBG.h>
#include <Xm/ArrowBGP.h>
#include <Xm/ArrowBP.h>
#include <Xm/AtomMgr.h>
//#include <Xm/BaseClassI.h>
#include <Xm/BaseClassP.h>
//#include <Xm/BitmapsI.h>
#include <Xm/BulletinB.h>
//#include <Xm/BulletinBI.h>
#include <Xm/BulletinBP.h>
#include <Xm/ButtonBox.h>
#include <Xm/ButtonBoxP.h>
//#include <Xm/CacheI.h>
#include <Xm/CacheP.h>
//#include <Xm/CallbackI.h>
#include <Xm/CareVisualT.h>
//#include <Xm/CareVisualTI.h>
#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
//#include <Xm/CascadeBGI.h>
#include <Xm/CascadeBGP.h>
//#include <Xm/CascadeBI.h>
#include <Xm/CascadeBP.h>
//#include <Xm/ClipWindTI.h>
#include <Xm/ClipWindowP.h>
//#include <Xm/CntrbmI.h>
//#include <Xm/ColorI.h>
//#include <Xm/ColorObjI.h>
#include <Xm/ColorObjP.h>
#include <Xm/ColorP.h>
#include <Xm/ColorS.h>
#include <Xm/ColorSP.h>
#include <Xm/Column.h>
#include <Xm/ColumnP.h>
#include <Xm/ComboBox.h>
#include <Xm/ComboBox2.h>
#include <Xm/ComboBox2P.h>
#include <Xm/ComboBoxP.h>
#include <Xm/Command.h>
//#include <Xm/CommandI.h>
#include <Xm/CommandP.h>
#include <Xm/ContItemT.h>
#include <Xm/Container.h>
#include <Xm/ContainerP.h>
#include <Xm/ContainerT.h>
#include <Xm/CutPaste.h>
//#include <Xm/CutPasteI.h>
#include <Xm/DataF.h>
//#include <Xm/DataFP.h>
#include <Xm/DataFSelP.h>
#include <Xm/DesktopP.h>
//#include <Xm/DestI.h>
#include <Xm/DialogS.h>
#include <Xm/DialogSEP.h>
#include <Xm/DialogSP.h>
#include <Xm/DialogSavvyT.h>
#include <Xm/Display.h>
//#include <Xm/DisplayI.h>
#include <Xm/DisplayP.h>
//#include <Xm/DragBSI.h>
#include <Xm/DragC.h>
//#include <Xm/DragCI.h>
#include <Xm/DragCP.h>
#include <Xm/DragDrop.h>
//#include <Xm/DragICCI.h>
#include <Xm/DragIcon.h>
//#include <Xm/DragIconI.h>
#include <Xm/DragIconP.h>
#include <Xm/DragOverS.h>
//#include <Xm/DragOverSI.h>
#include <Xm/DragOverSP.h>
//#include <Xm/DragUnderI.h>
//#include <Xm/DrawI.h>
#include <Xm/DrawP.h>
#include <Xm/DrawUtils.h>
#include <Xm/DrawingA.h>
//#include <Xm/DrawingAI.h>
#include <Xm/DrawingAP.h>
#include <Xm/DrawnB.h>
#include <Xm/DrawnBP.h>
#include <Xm/DropDown.h>
#include <Xm/DropDownP.h>
#include <Xm/DropSMgr.h>
//#include <Xm/DropSMgrI.h>
#include <Xm/DropSMgrP.h>
#include <Xm/DropTrans.h>
#include <Xm/DropTransP.h>
//#include <Xm/EditresComI.h>
#include <Xm/Ext.h>
#include <Xm/Ext18List.h>
#include <Xm/Ext18ListP.h>
//#include <Xm/ExtObjectI.h>
#include <Xm/ExtObjectP.h>
#include <Xm/ExtP.h>
#include <Xm/FileSB.h>
#include <Xm/FileSBP.h>
#include <Xm/FontS.h>
#include <Xm/FontSP.h>
#include <Xm/Form.h>
#include <Xm/FormP.h>
#include <Xm/Frame.h>
#include <Xm/FrameP.h>
//#include <Xm/GMUtilsI.h>
#include <Xm/Gadget.h>
//#include <Xm/GadgetI.h>
#include <Xm/GadgetP.h>
//#include <Xm/GadgetUtiI.h>
//#include <Xm/GeoUtilsI.h>
#include <Xm/GrabShell.h>
#include <Xm/GrabShellP.h>
//#include <Xm/HashI.h>
#include <Xm/Hierarchy.h>
#include <Xm/HierarchyP.h>
#include <Xm/IconBox.h>
#include <Xm/IconBoxP.h>
#include <Xm/IconButton.h>
#include <Xm/IconButtonP.h>
#include <Xm/IconFile.h>
#include <Xm/IconFileP.h>
#include <Xm/IconG.h>
//#include <Xm/IconGI.h>
#include <Xm/IconGP.h>
#include <Xm/IconH.h>
#include <Xm/IconHP.h>
//#include <Xm/ImageCachI.h>
#include <Xm/JoinSideT.h>
//#include <Xm/JpegI.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
//#include <Xm/LabelGI.h>
#include <Xm/LabelGP.h>
//#include <Xm/LabelI.h>
#include <Xm/LabelP.h>
#include <Xm/LayoutT.h>
#include <Xm/List.h>
#include <Xm/ListP.h>
#include <Xm/MainW.h>
#include <Xm/MainWP.h>
#include <Xm/Manager.h>
//#include <Xm/ManagerI.h>
#include <Xm/ManagerP.h>
//#include <Xm/MapEventsI.h>
//#include <Xm/MenuProcI.h>
#include <Xm/MenuProcP.h>
#include <Xm/MenuShell.h>
//#include <Xm/MenuShellI.h>
#include <Xm/MenuShellP.h>
//#include <Xm/MenuStateI.h>
#include <Xm/MenuStateP.h>
#include <Xm/MenuT.h>
//#include <Xm/MenuUtilI.h>
#include <Xm/MenuUtilP.h>
#include <Xm/MessageB.h>
//#include <Xm/MessageBI.h>
#include <Xm/MessageBP.h>
//#include <Xm/MessagesI.h>
#include <Xm/MultiList.h>
#include <Xm/MultiListP.h>
#include <Xm/MwmUtil.h>
#include <Xm/NavigatorT.h>
#include <Xm/Notebook.h>
#include <Xm/NotebookP.h>
#include <Xm/Outline.h>
#include <Xm/OutlineP.h>
#include <Xm/Paned.h>
#include <Xm/PanedP.h>
#include <Xm/PanedW.h>
#include <Xm/PanedWP.h>
#include <Xm/Picture.h>
#include <Xm/PictureP.h>
//#include <Xm/PixConvI.h>
//#include <Xm/PngI.h>
#include <Xm/PointInT.h>
#include <Xm/Primitive.h>
//#include <Xm/PrimitiveI.h>
#include <Xm/PrimitiveP.h>
//#include <Xm/Print.h>
//#include <Xm/PrintSI.h>
//#include <Xm/PrintSP.h>
#include <Xm/Protocols.h>
//#include <Xm/ProtocolsI.h>
#include <Xm/ProtocolsP.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/PushBGP.h>
#include <Xm/PushBP.h>
//#include <Xm/RCHookI.h>
//#include <Xm/RCLayoutI.h>
#include <Xm/RCLayoutP.h>
//#include <Xm/RCMenuI.h>
#include <Xm/RCMenuP.h>
//#include <Xm/ReadImageI.h>
//#include <Xm/RegionI.h>
#include <Xm/RepType.h>
//#include <Xm/RepTypeI.h>
//#include <Xm/ResConverI.h>
//#include <Xm/ResEncodI.h>
//#include <Xm/ResIndI.h>
#include <Xm/RowColumn.h>
//#include <Xm/RowColumnI.h>
#include <Xm/RowColumnP.h>
#include <Xm/SSpinB.h>
#include <Xm/SSpinBP.h>
#include <Xm/SashP.h>
#include <Xm/Scale.h>
#include <Xm/ScaleP.h>
#include <Xm/Screen.h>
//#include <Xm/ScreenI.h>
#include <Xm/ScreenP.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrollBarP.h>
//#include <Xm/ScrollFramTI.h>
#include <Xm/ScrollFrameT.h>
#include <Xm/ScrolledW.h>
//#include <Xm/ScrolledWI.h>
#include <Xm/ScrolledWP.h>
#include <Xm/SelectioB.h>
//#include <Xm/SelectioBI.h>
#include <Xm/SelectioBP.h>
#include <Xm/SeparatoG.h>
//#include <Xm/SeparatoGI.h>
#include <Xm/SeparatoGP.h>
#include <Xm/Separator.h>
#include <Xm/SeparatorP.h>
#include <Xm/ShellEP.h>
#include <Xm/SlideC.h>
#include <Xm/SlideCP.h>
#include <Xm/SpecRenderT.h>
#include <Xm/SpinB.h>
#include <Xm/SpinBP.h>
//#include <Xm/SyntheticI.h>
#include <Xm/TabBox.h>
#include <Xm/TabBoxP.h>
#include <Xm/TabList.h>
#include <Xm/TabStack.h>
#include <Xm/TabStackP.h>
#include <Xm/TakesDefT.h>
#include <Xm/TearOffBP.h>
//#include <Xm/TearOffI.h>
#include <Xm/TearOffP.h>
#include <Xm/Text.h>
//#include <Xm/TextDIconI.h>
#include <Xm/TextF.h>
//#include <Xm/TextFI.h>
//#include <Xm/TextFP.h>
//#include <Xm/TextFSelI.h>
#include <Xm/TextFSelP.h>
//#include <Xm/TextI.h>
//#include <Xm/TextInI.h>
#include <Xm/TextInP.h>
//#include <Xm/TextOutI.h>
#include <Xm/TextOutP.h>
#include <Xm/TextP.h>
//#include <Xm/TextSelI.h>
#include <Xm/TextSelP.h>
//#include <Xm/TextStrSoI.h>
#include <Xm/TextStrSoP.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
//#include <Xm/ToggleBGI.h>
#include <Xm/ToggleBGP.h>
#include <Xm/ToggleBP.h>
#include <Xm/ToolTipCT.h>
//#include <Xm/ToolTipI.h>
#include <Xm/ToolTipT.h>
//#include <Xm/TraitI.h>
#include <Xm/TraitP.h>
#include <Xm/Transfer.h>
//#include <Xm/TransferI.h>
#include <Xm/TransferP.h>
#include <Xm/TransferT.h>
#include <Xm/TransltnsP.h>
//#include <Xm/TravActI.h>
#include <Xm/TravConT.h>
//#include <Xm/TraversalI.h>
#include <Xm/Tree.h>
#include <Xm/TreeP.h>
#include <Xm/TxtPropCv.h>
#include <Xm/UnhighlightT.h>
//#include <Xm/UniqueEvnI.h>
#include <Xm/UnitTypeT.h>
//#include <Xm/VaSimpleI.h>
#include <Xm/VaSimpleP.h>
#include <Xm/VendorS.h>
//#include <Xm/VendorSEI.h>
#include <Xm/VendorSEP.h>
//#include <Xm/VendorSI.h>
#include <Xm/VendorSP.h>
#include <Xm/VirtKeys.h>
//#include <Xm/VirtKeysI.h>
#include <Xm/VirtKeysP.h>
#include <Xm/XmAll.h>
//#include <Xm/XmI.h>
#include <Xm/XmIm.h>
//#include <Xm/XmImI.h>
//#include <Xm/XmMsgI.h>
#include <Xm/XmP.h>
//#include <Xm/XmRenderTI.h>
#include <Xm/XmStrDefs.h>
#include <Xm/XmStrDefs22.h>
#include <Xm/XmStrDefs23.h>
//#include <Xm/XmStrDefsI.h>
//#include <Xm/XmStringI.h>
//#include <Xm/XmTabListI.h>
#include <Xm/Xmfuncs.h>
//#include <Xm/XmosI.h>
#include <Xm/XmosP.h>
#include <Xm/Xmos_r.h>
#include <Xm/Xmpoll.h>
//#include <Xm/XpmI.h>
#include <Xm/XpmP.h>
#include <Xm/xmlist.h>


/*
 * All X windows used for graphics need one of these
 */ 

typedef struct INFO { 
    Screen *screen;
    int screen_num;
    Display *display;
    GC gc;
    Window win;
    Colormap cmap;
    XColor color[10];   /* use 2-9 until .freq changes */
    Pixmap pixmap;      /* background of every grapics window */
    XFontStruct *font;
 } INFO;
void setupcmap(Widget, INFO *);
#endif /* XINFO_H */






