import 'package:flutter/material.dart';
import 'package:zefyrka/zefyrka.dart';
import 'package:papy/pages/toolbars/multi_button.dart';
import 'package:papy/tools/painter.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/tools/dialog.dart';

// 라이브러리 수정
// 120 at theme.dart
//final baseStyle = defaultStyle.style.copyWith(
//  fontSize: 18.0, //16.0,
//  height: 1.1,    //1.3,
//);
//const baseSpacing = VerticalSpacing(top: 2.0, bottom: 2.0);
////(top: 6.0, bottom: 10);

class BottomMenu extends StatefulWidget {
  BottomMenu({
    Key? key,
    required this.textController,
    required this.paintController,
    required this.mode,
    required this.onUpdateHowListener,
    required this.onPressAlbum,
    required this.onPressCamera,
  }) : super(key: key);

  final ZefyrController textController;
  final PainterController paintController;
  EditMode mode;
  final Function onUpdateHowListener;
  final Function onPressAlbum;
  final Function onPressCamera;

  @override
  _BottomMenuState createState() => _BottomMenuState();
}

class _BottomMenuState extends State<BottomMenu> {
  bool _selectedTextColor = false;
  bool _selectedBackgroundColor = false;
  bool _selectedForegroundColor = false;

  @override
  void initState() {
    print('DBG>bottom_menu>initState()');
    super.initState();
    //widget.paintController.setOnMenuRealignListener(_realign);
  }

  Color _getTextColor() {
    final attributes = widget.textController.getSelectionStyle().values;
    final value = attributes.firstWhere((element) => element.key == 'color',
        orElse: () => NotusAttribute.color.fromInt(0));
    return Color(value.value);
  }

  bool _textColor(bool act) {
    // 글자를 입력하지 않고서는 getSelection()이 되지 않는다.
    if (act) {
      setState(() {
        _selectedTextColor = !_selectedTextColor;
        if (_selectedTextColor) {
          _selectedForegroundColor = false;
          _selectedBackgroundColor = false;
        }
      });
    }
    return _selectedTextColor;
  }

  bool _foregroundColor(bool act) {
    if (act) {
      setState(() {
        _selectedForegroundColor = !_selectedForegroundColor;
        if (_selectedForegroundColor) {
          _selectedTextColor = false;
          _selectedBackgroundColor = false;
        }
      });
    }
    return _selectedForegroundColor;
  }

  bool _backgroundColor(bool act) {
    if (act) {
      setState(() {
        _selectedBackgroundColor = !_selectedBackgroundColor;
        if (_selectedBackgroundColor) {
          _selectedTextColor = false;
          _selectedForegroundColor = false;
        }
      });
    }
    return _selectedBackgroundColor;
  }

  void _onColorChange(color) {
    setState(() {
      if (_selectedTextColor) {
        _selectedTextColor = false;
        widget.textController.formatSelection(
            NotusAttribute.color.fromInt(color.value));
      } else if (_selectedForegroundColor) {
        _selectedForegroundColor = false;
        widget.paintController.foregroundColor = color;
      } else if (_selectedBackgroundColor) {
        _selectedBackgroundColor = false;
        widget.paintController.backgroundColor = color;
      }
    });
  }

  void _realign() {
    print('DBG>bottom_menu>realign()');
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    bool selectedColor = _selectedTextColor || _selectedForegroundColor ||
      _selectedBackgroundColor;
    return SizedBox(
      height: selectedColor ? eToolbarHeight * 2 : eToolbarHeight,
      child: Column(
        mainAxisAlignment: MainAxisAlignment.end,
        children: [
          SizedBox(
            width: MediaQuery.of(context).size.width,
            height: selectedColor ? eToolbarHeight : 0,
            child: selectedColor
              ? ColorPicker(
                foreOrBack: _selectedBackgroundColor ? false : true,
                currentColor: _selectedBackgroundColor
                    ? widget.paintController.backgroundColor
                    : _selectedForegroundColor
                      ? widget.paintController.foregroundColor
                        : _getTextColor(),
                backgroundColor: widget.paintController.backgroundColor,
                height: eToolbarHeight,
                onColorChanged: _onColorChange,
              )
              : null,
          ),
          BottomAppBar(
            color: Environs.defaultColor,
            child: IconTheme (
              data: IconThemeData(
                color: Theme.of(context).colorScheme.onPrimary,
              ),
              child: Row(
                children: [
                  const Spacer(),
                  SizedBox(
                    height: eToolbarHeight,
                    child: widget.mode == EditMode.text
                      ? TextTools.make(
                        textController: widget.textController,
                        paintController: widget.paintController,
                        textColor: _textColor,
                        backgroundColor: _backgroundColor,
                      )
                      : widget.mode == EditMode.draw
                        ? DrawTools.make(
                          controller: widget.paintController,
                          updateHowListener: widget.onUpdateHowListener,
                          foregroundColor: _foregroundColor,
                          backgroundColor: _backgroundColor,
                        )
                        : widget.mode == EditMode.picture
                          ? PickTools.make(
                            controller: widget.paintController,
                            selectFromCamera: widget.onPressCamera,
                            selectFromAlbum: widget.onPressAlbum,
                            backgroundColor: _backgroundColor,
                          )
                          : null,
                  ),
                ]
              ),
            ),
          ),
        ]
      ),
    );
  }
}

class TextTools extends StatefulWidget implements PreferredSizeWidget {
  final List<Widget> children;
  const TextTools({Key? key, required this.children}) : super(key: key);

  factory TextTools.make({
    Key? key,
    required ZefyrController textController,
    required PainterController paintController,
    required Function textColor,
    required Function backgroundColor,
    List<Widget> leading = const <Widget>[],
    List<Widget> trailing = const <Widget>[],
  }) {
    return TextTools(key: key, children: [
      ...leading,
      MultiIconButton(
        iconData: Icons.text_fields,
        iconSize: 15,
        attribute: NotusAttribute.heading.unset,
        textController: textController,   // for redraw icon
        tooltip: '크기:18',
      ),
      MultiIconButton(
        iconData: Icons.text_fields,
        iconSize: 18,
        attribute: NotusAttribute.heading.level3,
        textController: textController,
        tooltip: '크기:20',
      ),
      MultiIconButton(
        iconData: Icons.text_fields,
        iconSize: 21,
        attribute: NotusAttribute.heading.level2,
        textController: textController,
        tooltip: '크기:24',
      ),
      MultiIconButton(
        iconData: Icons.text_fields,
        iconSize: 24,
        attribute: NotusAttribute.heading.level1,
        textController: textController,
        tooltip: '크기:34',
      ),
      MultiIconButton(
        iconData: Icons.format_bold,
        attribute: NotusAttribute.bold,
        textController: textController,
      ),
      MultiIconButton(
        iconData: Icons.format_italic,
        attribute: NotusAttribute.italic,
        textController: textController,
      ),
      MultiIconButton(
        iconData: Icons.image,
        getState: () { return textColor(false); },
        textController: textController,
        doAction: () {
          textColor(true);
          textController.formatSelection(NotusAttribute.block.quote.unset);
          paintController.backgroundColor = paintController.backgroundColor;
        },
      ),
      MultiIconButton(
        iconData: Icons.image_outlined,
        getState: () { return backgroundColor(false); },
        paintController: paintController,
        doAction: () {
          backgroundColor(true);
          paintController.backgroundColor = paintController.backgroundColor;
          textController.formatSelection(NotusAttribute.block.quote.unset);
          // 단지 리스너를 동작시키려는 것이다.
          // 동작을 먼저하고 리스너를 동기시켜야 동작이 된 후의 상태를 읽어온다.
        },
      ),
      ...trailing,
    ]);
  }

  @override
  _TextToolsState createState() => _TextToolsState();

  @override
  Size get preferredSize => const Size.fromHeight(eToolbarHeight);
}

class _TextToolsState extends State<TextTools> {
  @override
  Widget build(BuildContext context) {
    return Container(
        padding: const EdgeInsets.symmetric(horizontal: 8),
        constraints: BoxConstraints.tightFor(height: widget.preferredSize.height),
        child: SingleChildScrollView(
            scrollDirection: Axis.horizontal,
            child: Row(
              children: widget.children,
            )
        )
    );
  }
}

class DrawTools extends StatefulWidget implements PreferredSizeWidget {
  final List<Widget> children;
  const DrawTools({Key? key, required this.children}) : super(key: key);

  //late String tooltipWidth = '폭:';

  factory DrawTools.make({
    Key? key,
    required PainterController controller,
    required Function updateHowListener,
    required Function foregroundColor,
    required Function backgroundColor,
    List<Widget> leading = const <Widget>[],
    List<Widget> trailing = const <Widget>[],
  }) {
    return DrawTools(key: key, children: [
      ...leading,
      MultiIconButton(iconData: Icons.draw,
        getState: () => controller.drawMode,
        paintController: controller,
        doAction: () => controller.eraseMode = false,
      ),
      MultiIconButton(
        iconData: Icons.cleaning_services,
        getState: () => controller.eraseMode,
        paintController: controller,
        doAction: () => controller.eraseMode = true,
      ),
      MultiIconButton(
        iconData: Icons.undo,
        doAction: controller.undoPathTrack,
      ),
      MultiIconButton(
        iconData: Icons.delete,
        doAction: controller.clearPathTrack,
      ),
      MultiIconButton(
        iconData: Icons.waves,
        iconSize: 24,   //
        paintController: controller,
        getState: () {
          //print('DBG>bottom_menu>getState() ${controller.thickness}');
          return controller.thickness; // 2~14
        },
        doAction: () {
          //print('DBG>bottom_menu>waves doAction()');
          double t = controller.thickness; // 2~14
          t += 2.0;
          if (t > Environs.maxThickness)  t = Environs.minThickness;
          controller.thickness = t;
          updateHowListener.call();
        },
        tooltip: '폭:',
      ),
      MultiIconButton(
        iconData: Icons.image,
        getState: () { return foregroundColor(false); },
        paintController: controller,
        doAction: () {
          foregroundColor(true);
          controller.foregroundColor = controller.foregroundColor;
        },
      ),
      MultiIconButton(
        iconData: Icons.image_outlined,
        getState: () { return backgroundColor(false); },
        paintController: controller,
        doAction: () {
          backgroundColor(true);
          controller.backgroundColor = controller.backgroundColor;
        }
      ),
      ...trailing,
    ]);
  }

  @override
  _DrawToolsState createState() => _DrawToolsState();

  @override
  Size get preferredSize => const Size.fromHeight(eToolbarHeight);
}

class _DrawToolsState extends State<DrawTools> {
  @override
  Widget build(BuildContext context) {
    return Container(
        padding: const EdgeInsets.symmetric(horizontal: 8),
        constraints: BoxConstraints.tightFor(height: widget.preferredSize.height),
        child: SingleChildScrollView(
            scrollDirection: Axis.horizontal,
            child: Row(
              children: widget.children,
            )
        )
    );
  }
}

class PickTools extends StatefulWidget implements PreferredSizeWidget {
  final List<Widget> children;
  const PickTools({Key? key, required this.children}) : super(key: key);

  factory PickTools.make({
    Key? key,
    required PainterController controller,
    required Function selectFromCamera,
    required Function selectFromAlbum,
    required Function backgroundColor,
    List<Widget> leading = const <Widget>[],
    List<Widget> trailing = const <Widget>[],
  }) {
    return PickTools(key: key, children: [
      ...leading,
      MultiIconButton(
        iconData: Icons.camera,
        doAction: selectFromCamera,
      ),
      MultiIconButton(
        iconData: Icons.album,
        doAction: selectFromAlbum,
      ),
      MultiIconButton(
        iconData: Icons.image_outlined,
        getState: () { return backgroundColor(false); },
        paintController: controller,
        doAction: () {
          backgroundColor(true);
          controller.backgroundColor = controller.backgroundColor;
        },
      ),
      ...trailing,
    ]);
  }

  @override
  _PickToolsState createState() => _PickToolsState();

  @override
  Size get preferredSize => const Size.fromHeight(eToolbarHeight);
}

class _PickToolsState extends State<PickTools> {
  @override
  Widget build(BuildContext context) {
    return Container(
      padding: const EdgeInsets.symmetric(horizontal: 8),
      constraints: BoxConstraints.tightFor(height: widget.preferredSize.height),
      child: SingleChildScrollView(
        scrollDirection: Axis.horizontal,
        child: Row(
          children: widget.children,
        )
      )
    );
  }
}
