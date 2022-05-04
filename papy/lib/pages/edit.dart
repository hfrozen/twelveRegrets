import 'dart:io';
import 'dart:ui' as ui;
import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'dart:core';
import 'package:flutter/material.dart' hide kToolbarHeight;
import 'package:flutter/rendering.dart';
import 'package:flutter/services.dart';
import 'package:path_provider/path_provider.dart';
import 'package:provider/provider.dart';
import 'package:drift/drift.dart' hide Column;
import 'package:get/get.dart' hide Value;
import 'package:zefyrka/zefyrka.dart';
import 'package:flutter_share/flutter_share.dart';
import 'package:quiver/strings.dart';
import 'package:image/image.dart' as img;
import 'package:image_picker/image_picker.dart';

import 'package:papy/base/letter.dart';
import 'package:papy/base/how.dart';
import 'package:papy/refer/const.dart';
import 'package:papy/refer/translates.dart';
import 'package:papy/tools/painter.dart';
import 'package:papy/pages/toolbars/mode_menu.dart';
import 'package:papy/pages/toolbars/bottom_menu.dart';
import 'package:papy/pages/sub/picture_list.dart';

//ignore: must_be_immutable
class EditPage extends StatefulWidget {
  EditPage({Key? key,
    required this.letter, required this.drawing,
    required this.onModified,
  }) : super(key: key);

  Letter? letter;
  Drawing? drawing;
  Function? onModified;

  @override
  _EditPageState createState() => _EditPageState();
}

class _EditPageState extends State<EditPage>
    with SingleTickerProviderStateMixin {
  late LetterDatabase db;

  late ZefyrController _textController;
  late PainterController _paintController;
  final ScrollController _scrollController = ScrollController();

  final bool holdAttributes = true;
  EditMode _mode = EditMode.none;
  final FocusNode _focusNode = FocusNode();
  double _defaultHeight = Environs.defaultPaintHeight;  // for un_limit scroll
  Timer? _timer;
  bool _selectedFigureForDelete = false;
  bool _sampleInitialized = false;
  late int _sampleWidth;
  late int _sampleHeight;
  final bool _share = true;   // false when edit is null
  dynamic _pickCameraError;

  final GlobalKey _repaintGlobalKey = GlobalKey();

  @override
  void initState() {
    super.initState();

    _mode = EditMode.none;
    db = Provider.of<LetterDatabase>(context, listen: false);
    _textController = ZefyrController(_loadDocument());
    _textController.addListener(startSaveTimer);
    _textController.document.changes.listen((change) =>
        _documentChangeListener(change));

    _paintController = PainterController(
        widget.drawing != null ? widget.drawing!.paths : null,
        widget.drawing != null ? widget.drawing!.figures : null,
        compressionLevel: Environs.pathCompressionLevel
    );
    if (widget.letter != null) {
      Hows hows = HowsConvertor.fromLetter(widget.letter!);
      _paintController.thickness = hows.thickness;
      _paintController.foregroundColor = hows.foregroundColor;
      _paintController.backgroundColor = hows.backgroundColor;
    } else {
      _paintController.thickness = Environs.defaultThickness;
      _paintController.foregroundColor = Environs().foregroundColors.last;
      _paintController.backgroundColor = Environs().backgroundColors.last;
    }
    _paintController.setOnInterimUpdateListener(startSaveTimer);
    _paintController.setOnImageUpListener(onImageUpListener);
    _paintController.setSelectItemListener(onSelectItemListener);

    _scrollController.addListener(_scrollListener);

    _defaultHeight = Environs.defaultPaintHeight;
    Offset max = _paintController.getMaxCoordinatesInPathTrack();
    if (max.dy > _defaultHeight)  _defaultHeight = max.dy;
    print('DBG>edit>initState() height=$_defaultHeight max=$max');
  }

  @override
  void dispose() {
    _timer?.cancel();
    if (_mode == EditMode.text) {
      //SystemChannels.textInput.invokeMethod('TextInput.hide');
      _focusNode.requestFocus(FocusNode());
    }
    _focusNode.dispose();
    _textController.dispose();
    _paintController.dispose();
    _scrollController.removeListener(_scrollListener);
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    if (!_sampleInitialized) {
      _sampleWidth = MediaQuery.of(context).size.width ~/ Environs.rowViews;
      _sampleHeight = MediaQuery.of(context).size.height ~/ Environs.columnViews;
      //print('DBG>edit() sampling size=$_sampleWidth $_sampleHeight');
      _sampleInitialized = true;
    }

    return WillPopScope(
      onWillPop: _onWillPop,
      child: Scaffold(
        //resizeToAvoidBottomInset: false,
        appBar: AppBar(
          title: Text(Localization.of(context)!.tr('note')!),
          actions: [
            ModeMenu(
              mode: _mode,
              share: _share,
              delete: _selectedFigureForDelete,
              onSelectEditMode: _changeType,
              onPressButton: _onSubMenus,
              onMore: _aboutMore,
            ),
          ],
        ),
        // BODY IS SCAFFOLD, ALWAYS BOTTOM APP ON KEYBOARD !!!
        body: Scaffold(
          body: Column(
            children: [
              Expanded(
                child: _edit(),
              ),
            ],
          ),
          //_edit(),
          bottomNavigationBar: _mode != EditMode.none
            ? BottomMenu(
              textController: _textController,
              paintController: _paintController,
              mode: _mode,
              //onPressed: (() => {})
              onUpdateHowListener: () => setState(() { startSaveTimer(); }),
              onPressAlbum: () => Get.to(() => PictureList(
                onSelectImage: _onSelectImageFromPictureList,
              )),
              onPressCamera: _getImageFromCamera,
            )
            : null,
        ),
      ),
    );
  }

  _edit() {
    Color color;
    if (widget.drawing == null || widget.drawing!.paths!.length < 3) {
      color = _paintController.backgroundColor;
    } else {
      color = Colors.transparent;
    }

    Widget _child = Stack(
      children: [
        //==== text begin =================================
        IgnorePointer(
          ignoring: _mode != EditMode.text ? true : false,
          child: Container(
            color: color, //Colors.transparent,
            padding: const EdgeInsets.only(left: 16, right: 0),
            child: ZefyrEditor(
              controller: _textController,
              focusNode: _focusNode,
              scrollable: false,
              scrollController: _scrollController,
              autofocus: _mode == EditMode.text ? true : false,
              readOnly: _mode == EditMode.text ? false : true,
              enableInteractiveSelection: _mode == EditMode.text ? true : false,
            ),
          ),
        ),
        //==== text end ===================================
        //==== draw begin =================================
        IgnorePointer(
            ignoring: _mode == EditMode.text ? true : false,
            child: Container(
              height: _defaultHeight,
              child: Opacity(
                opacity: 0.8,
                child: Painter(_paintController, _mode),
              ),
            ),
          ),
        //==== draw end ===================================
      ],
    );

    return SingleChildScrollView(
      controller: _scrollController,
      physics: _mode == EditMode.draw || _mode == EditMode.picture
          ? const NeverScrollableScrollPhysics()
          : const AlwaysScrollableScrollPhysics(),
      child: RepaintBoundary(
        key: _repaintGlobalKey,
        child: _child,
      ),
    );
  }

  _documentChangeListener(NotusChange change) {
    if (_mode != EditMode.text || !holdAttributes) return;
    var value = '';
    for (var index = 0; index < change.change.length; index ++) {
      final operation = change.change.elementAt(index);
      if (operation.isInsert) value = operation.value;
    }
    if (value == '\n') {
      //print('DBG>find cr...');
      TextSelection selection = _textController.selection;
      if (selection.start > 0) {
        final start = selection.start - 1;
        final length = selection.end - start - 1;
        NotusStyle style = _textController.document.collectStyle(start, length);
        style = style.mergeAll(_textController.toggledStyles);
        if (style.isNotEmpty) {
          for (var element in style.values) {
            print('each: ${element.key} ${element.value}');
            _textController.formatSelection(element);
          }
        }
      }
    }
  }

  _scrollListener() {
    if (_scrollController.position.pixels ==
        _scrollController.position.maxScrollExtent) {
      setState(() {
        _defaultHeight += MediaQuery.of(context).size.height;
      });
      print('DBG>edit>_scrollListener() extend');
    }
  }

  _aboutMore() {
    //if (_mode != EditMode.text) return;
    // 글자를 입력하지 않고서는 getSelection()이 되지 않는다.
    final attrs = _textController.getSelectionStyle().values;
    TextSelection selection = _textController.selection;
    print('DBG>edit>aboutMore() ${selection.start} ${selection.end} $attrs');
    attrs.forEach(print);
    int? v;
    for (var e in attrs) {
      if (e.key == 'color') {
        v = e.value;
      }
    }
    print('1st. find color: $v');
    if (selection.start > 0) {
      final start = selection.start - 1;
      final length = selection.end - start - 1;
      var lineStyle = _textController.document.collectStyle(start, length);
      lineStyle = lineStyle.mergeAll(_textController.toggledStyles);
      print('2nd. merge style: $start $length $lineStyle');
    }
    final colors = attrs.firstWhere((element) => element.key == 'color',
          orElse: () => NotusAttribute.color.fromInt(0));
    print('3nd. find color: ${colors.value}');
    print('DEBUG>edit>viewport:${_scrollController.position.viewportDimension}');
    print('DEBUG>${MediaQuery.of(context).size.height} ${MediaQuery.of(context).viewInsets.bottom} ${AppBar().preferredSize.height} ${MediaQuery.of(context).padding.top} ${MediaQuery.of(context).viewInsets.bottom}');
  }

  _changeType(EditMode mode) {
    EditMode old = _mode;
    EditMode fresh;
    _paintController.setTargetFigure(-1);
    if (mode == old) { fresh = EditMode.none; }
    else  { fresh = mode; }
    setState(() {
      _mode = fresh;
    });
    if (old != EditMode.text && fresh == EditMode.text) {
      _focusNode.requestFocus();
    } else if (old == EditMode.text && fresh != EditMode.text) {
      FocusScope.of(context).requestFocus(FocusNode());
    }
    _paintController.figureSelectMode = false;
  }

  Future<void> _getImageFromCamera() async {
    try {
      print('DBG>edit>_getImageFromCamera() entry');
      final XFile? file = await ImagePicker().pickImage(
        source: ImageSource.camera,
        maxHeight: null,
        maxWidth: null,
        imageQuality: null,
      );
      if (file != null) {
        _paintController
            .addFigureTrailFile(file.path, MediaQuery.of(context).size);
      }
      print('DBG>edit>_getImageFromCamera() path=${file!.path}');
    } catch (e) {
      _pickCameraError = e;
      print(e);
    }
  }

  void _onSelectImageFromPictureList(String path) {
    _paintController.addFigureTrailFile(path, MediaQuery.of(context).size);
  }

  void _onSubMenus(EditSubMenu button) {
    switch (button) {
      case EditSubMenu.share:
        _shareScreenshot();
        break;
      case EditSubMenu.delete:
        print('DBG>edit>_onSubButtons.delete');
        _paintController.removeFigureTrail();
        setState(() {});
        break;
      case EditSubMenu.close:
        _paintController.figureSelectMode = false;
        setState(() {
          _selectedFigureForDelete = false;
        });
        break;
      default:  break;
    }
  }

  void onSelectItemListener() {
    setState(() {
      // display basket
      _selectedFigureForDelete = _paintController.figureSelectMode;
    });
  }

  void onImageUpListener() {
    setState(() {});
  }

  Future<bool> _onWillPop() async {
    _timer?.cancel();
    return _save(isPop: true);
  }

  Future<Uint8List> _getScreenshot(int width, int height) async {
    RenderRepaintBoundary boundary =
        _repaintGlobalKey.currentContext!.findRenderObject()
                    as RenderRepaintBoundary;
    ui.Image image = await boundary.toImage();
    ByteData? bytes = await image.toByteData(format: ui.ImageByteFormat.png);
    Uint8List pngBytes = bytes!.buffer.asUint8List();
    img.Image? src = img.decodeImage(pngBytes);
    img.Image dest = img.copyCrop(src!, 0, 0, width, height);
    return img.encodePng(dest) as Uint8List;
  }

  Future<void> _shareScreenshot() async {
    Directory? _directory;
    if (Platform.isAndroid) {
      _directory = await getExternalStorageDirectory();
    } else {
      _directory = await getApplicationDocumentsDirectory();
    }
    final String localPath =
        '${_directory!.path}/${DateTime.now().toIso8601String()}.png';

    int width = MediaQuery.of(context).size.width.toInt();
    int height = (MediaQuery.of(context).size.height -
        MediaQuery.of(context).viewPadding.top - kToolbarHeight).toInt();
    Future<Uint8List> future = _getScreenshot(width, height);
    future.then((sampleBytes) {
      print('DBG>edit>sharing...');
      File(localPath).writeAsBytes(sampleBytes.toList());
      FlutterShare.shareFile(
        title: 'share',
        filePath: localPath,
        fileType: 'image/png',
      );
    });
  }

  startSaveTimer() {
    _timer?.cancel();
    return;   // ?????
    _timer = Timer(const Duration(milliseconds: 500), () => {_save()});
  }

  int isModify() {
    String pathTrack = _paintController.getPathTrack;
    String figureTrail = _paintController.getFigureTrail;
    String how = _getHows();
    final plainStory = _textController.document.toPlainText();
      //jsonEncode(_textController.document.toPlainText().trim());

    if (widget.letter == null && plainStory.isNotEmpty) { return 1; }
    if (widget.letter != null) {
      if (widget.letter!.plainStory != plainStory) {
        //print('DBG>edit>_save() 1=${widget.letter!.plainStory} 2=$storyPlain');
        return 2;
      }
      if (widget.letter!.how != how) { return 3; }
    }
    if (widget.drawing == null) {
      if (pathTrack.isNotEmpty) { return 4; }
      if (figureTrail.isNotEmpty) { return 5; }
    }
    if (widget.drawing != null) {
      if (widget.drawing!.paths != pathTrack) { return 6; }
      if (widget.drawing!.figures != figureTrail) { return 7; }
    }
    return 0;
  }

  Future<bool> _save({isPop = false}) async {
    print('DBG>edit>_save()');
    String pathTrack = _paintController.getPathTrack;
    String figureTrail = _paintController.getFigureTrail;

    if (isNotBlank(_textController.document.toPlainText()) ||
          pathTrack.length > 2 || figureTrail.length > 2) {
      print('DBG>_save path:${pathTrack.length} figure:${figureTrail.length}');
      int diff = isModify();
      if (diff > 0) {
        print('DBG>edit>_save cause from $diff');
        if (pathTrack.length > 2 || figureTrail.length > 2) {
          Future<Uint8List> sampleBytes =
                  _getScreenshot(_sampleWidth, _sampleHeight);
          sampleBytes.then((value) {
            print('DBG>edit>_save() has preview ${value.lengthInBytes}');
            final sampleScreen = jsonEncode(value); // serializedPreview
            //print('DBG>edit>_save() has preview encode size=${preview.length}');
            return _store(sampleScreen);
          });
        } else {
          print('DBG>edit>_save() has not preview');
          return _store(null);
        }
      }
    } else {
      print('DBG>_save not found any content');
      if (widget.letter != null && isPop) {
        print('DBG>_save delete this content');
        db.deleteLetter(widget.letter!);
      }
    }
    return true;
  }

  Future<bool> _store(String? preview) async {
    print('DBG>edit>_store()');
    String pathTrack = _paintController.getPathTrack;
    String figureTrail = _paintController.getFigureTrail;
    String how = _getHows();
    final story = jsonEncode(_textController.document);

    int view = preview == null ? 0 : 1;
    if (widget.letter != null && widget.drawing != null) {
      print('DBG>edit>_store() update');
      if (widget.letter!.story != story || widget.letter!.view != view ||
          widget.letter!.how != how) {
        var newLetter = widget.letter!.copyWith(
          view: view,
          story: story,
          plainStory: _textController.document.toPlainText(),
          how: how,
        );
        db.updateLetter(newLetter);
        widget.letter = newLetter;
        print('DBG>edit>_store> update letterId=${newLetter.id} story=${newLetter.story.length} how=${newLetter.how.length} drawId=${newLetter.drawId}');
      }
      if (widget.drawing?.paths != pathTrack
          || widget.drawing?.figures != figureTrail
          || widget.drawing?.preview != preview) {
        // if text change, preview change
        var newDrawing = widget.drawing!.copyWith(
            paths: pathTrack,
            figures: figureTrail,
            preview: preview);
        db.updateDrawing(newDrawing);
        widget.drawing = newDrawing;
      }
    } else {
      print('DBG>edit>_store() insert');
      db.insertDrawing(DrawingsCompanion(
          paths: Value(pathTrack), figures: Value(figureTrail),
          preview: Value(preview)))
          .then((drawingId) {
        widget.drawing =
            Drawing(id: drawingId, paths: pathTrack,
                figures: figureTrail, preview: preview);
        db.insertLetter(LettersCompanion(
            view: Value(view),
            story: Value(story),
            plainStory: Value(_textController.document.toPlainText()),
            date: Value(DateTime.now()),
            how: Value(how),
            drawId: Value(drawingId)))
            .then((letterId) {
          widget.letter = Letter(
              id: letterId,
              view: view,
              story: story,
              plainStory: _textController.document.toPlainText(),
              date: DateTime.now(),
              how: how,
              drawId: drawingId);
        });
      });
    }
    if (widget.onModified != null) {
      widget.onModified!.call(widget.letter!.id);
    }
    return true;
  }

  String _getHows() {
    return json.encode(Hows(
      _paintController.foregroundColor,
      _paintController.backgroundColor,
      _paintController.thickness,
    ));
  }

  NotusDocument _loadDocument() {
    if (widget.letter != null) {
      print('DBG>edit>_loadDocument() fromJson');
      return NotusDocument.fromJson(json.decode(widget.letter!.story) as List);
    } else {
      print('DBG>edit>_loadDocument() null');
      return NotusDocument();
    }
  }
}
