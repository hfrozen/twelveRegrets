import 'dart:convert';
import 'dart:async';
import 'dart:typed_data';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:get/get.dart';
import 'package:flutter_staggered_grid_view/flutter_staggered_grid_view.dart';
import 'package:zefyrka/zefyrka.dart';
import 'package:papy/base/letter.dart';
import 'package:papy/base/how.dart';
import 'package:papy/pages/edit.dart';
import 'package:papy/refer/translates.dart';
import 'package:papy/refer/const.dart';

class _PreviewList1 {
  _PreviewList1(this.id, this.check, this.data);
  int id;
  bool check = false;
  Uint8List? data;    // = null;
}

class LetterList1 extends StatefulWidget {
  const LetterList1({
    Key? key,
    required this.onSelectItem,
    required this.onRealignListener,
    required this.forcedEmpty,
  }) : super(key: key);

  final Function onSelectItem;
  final Function onRealignListener;
  final bool forcedEmpty;
  @override
  LetterListState1 createState() => LetterListState1();
}

// remove '_' for Globalkey
class LetterListState1 extends State<LetterList1>
    with TickerProviderStateMixin {
  bool modalVisible = false;
  bool shareMode = false;
  bool screenShootEffectVisible = false;
  String searchQuery = '';
  final List<_PreviewList1> _previewer = [];
  late final List _selectedList = [];

  //AnimationController? _animationController;
  //CurvedAnimation? _animation;
  //final GlobalKey _repaintKey = GlobalKey();

  void onQueryTextChanged(String query) {
    print('DBG>letter_list>onQueryTextChanged() setState');
    setState(() {
      searchQuery = query;
    });
  }

  @override
  void initState() {
    super.initState();
    print('DBG>letter_list>initState() forcedEmpty=${widget.forcedEmpty}');
  }

  @override
  void dispose() {
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    print('DBG>letter_list>build() searchQuery:$searchQuery');
    return _letterList();
  }

  StreamBuilder<List<Letter>> _letterList() {
    final db = Provider.of<LetterDatabase>(context);
    return StreamBuilder(
      stream: db.watchEntriesWithText(searchQuery),
      builder: (context, AsyncSnapshot snapshot) {
        final letters = snapshot.data ?? [];
        return Stack(
          children: [
            Visibility(
              visible: searchQuery.isNotEmpty && letters.length == 0 &&
                  snapshot.data != null && !widget.forcedEmpty,
              //AnimatedOpacity(
              //  duration: const Duration(milliseconds: 300),
              //  opacity: searchQuery.isNotEmpty &&
              //      letters.length == 0 && snapshot.data != null &&
              //      !widget.forcedEmpty ? 1.0 : 0.0,
              child: Padding(
                padding: const EdgeInsets.all(4.0),
                child: Center(
                  child: Text(
                    '\"' + searchQuery + '\"' +
                        Localization.of(context)!.tr('notFound')!,
                    textAlign: TextAlign.center,
                    style: const TextStyle(fontSize: 18),
                  ),
                ),
              ),
            ),
            if (!widget.forcedEmpty)
              StaggeredGridView.countBuilder(
                  padding: const EdgeInsets.all(4),
                  itemCount: letters.length,
                  crossAxisCount: Environs.rowViews,
                  mainAxisSpacing: 4,
                  crossAxisSpacing: 4,
                  staggeredTileBuilder: (index) => const StaggeredTile.fit(1),
                  itemBuilder: (context, index) {
                    Letter letter = letters[snapshot.data.length - index - 1];
                    print('DBG>letter_list>build() index=$index');
                    return Container(
                      padding: const EdgeInsets.all(0),
                      decoration: BoxDecoration(
                        border: Border.all(
                          color: Environs.defaultBackgroundColor, //Colors.grey,
                          style: BorderStyle.solid,
                          width: 1,
                        ),
                        borderRadius: BorderRadius.circular(4.0),
                      ),
                      child: _letterPreview(letter),
                    );
                  }
              ),
            Visibility(
              visible: letters.length == 0 && searchQuery.isEmpty &&
                  snapshot.data != null && widget.forcedEmpty,
              //AnimatedOpacity(
              //  opacity: (snapshot.data == null ||
              //      letters.isNotEmpty || searchQuery.isNotEmpty) &&
              //      !widget.forcedEmpty ? 0.0 : 1.0,
              //  duration: const Duration(milliseconds: 3000),
              // AnimatedOpacity가 별로 우아하지도 않고 또 이 뒤에 버튼이 있으면
              // 비 활성화가 되지않고 동작해버린다.
              child: Center(
                child: Column(
                  mainAxisAlignment: MainAxisAlignment.center,
                  crossAxisAlignment: CrossAxisAlignment.center,
                  children: [
                    const Image(
                      image: AssetImage('assets/papyrus.png'),
                      fit: BoxFit.fill,
                      width: 256,
                      height: 256,
                    ),
                    const SizedBox(height: 64),
                    Text(
                      '내용이 없습니다.',
                      style: TextStyle(
                        fontSize: 20,
                        color: Environs.defaultColor,
                      ),
                    ),
                    const SizedBox(height: 32),
                    ElevatedButton(
                      onPressed: () => {
                        Get.to(() =>
                            EditPage(
                                letter: null,
                                drawing: null,
                                onModified: null
                            ),
                        ),
                      },
                      child: const Text('시작하기'),
                      style: ElevatedButton.styleFrom(
                          primary: Environs.defaultColor,
                          minimumSize: const Size(240, 64),
                          textStyle: const TextStyle(
                              fontSize: 24,
                              fontWeight: FontWeight.bold
                          )
                      ),
                    ),
                  ],
                ),
              ),
            ),
          ],
        );
      },
    );
  }

  Widget _letterPreview(Letter letter) {
    final db = Provider.of<LetterDatabase>(context, listen: false);
    Hows hows = HowsConvertor.fromLetter(letter);
    var plainStory =
    NotusDocument.fromJson(json.decode(letter.story) as List).toPlainText();
    int index;

    return InkWell(
      onLongPress: () => {
        print('DBG>letter_list>_letterItem() onLongPress ${_selectedList.length}'),
        if (_selectedList.isEmpty) {
          _toggleSelectItem(letter),
          print('DBG>letter_list>_letterItem() add ${_selectedList.length}'),
        }
      },
      onTap: () => {
        if (_selectedList.isNotEmpty) {
          _toggleSelectItem(letter),
        } else {
          db.getDrawing(letter.drawId!).then((drawing) => Get.to(() =>
              EditPage(
                  letter: letter,
                  drawing: drawing,
                  onModified: onModifiedFromEditPage
              )
          ))
        }
      },
      child: Stack(
          children: [
            Visibility(
              visible: _selectedList.isNotEmpty ? true : false,
              child: Align (
                alignment: Alignment.topRight,
                child: Icon(
                    _selectedList.contains(letter.id)
                        ? Icons.check_circle_rounded : Icons.circle_rounded
                ),
              ),
            ),
            Align(
              alignment: Alignment.topLeft,
              child: letter.view == 0
                  ? Container(
                  width: MediaQuery.of(context).size.width / Environs.rowViews,
                  //height: 100,
                  decoration: BoxDecoration(
                    color: hows.backgroundColor,
                  ),
                  constraints: BoxConstraints(
                    maxHeight: MediaQuery.of(context).size.height /
                        Environs.columnViews,
                  ),
                  child: Text(
                    plainStory,
                    overflow: TextOverflow.ellipsis,
                    style: TextStyle(
                      color: hows.foregroundColor,
                      fontSize: 16,
                    ),
                  )
              )
                  : (index = _previewer.indexWhere((element)
              => element.id == letter.id)) >= 0
                  && _previewer[index].data != null
                  ? Image.memory(_previewer[index].data!)
                  : StreamBuilder(
                stream: _loadPreviewStream(letter),
                builder: (_, AsyncSnapshot<Drawing> snapshot) {
                  if (!snapshot.hasData) {
                    return const Text('please wait...');
                  }
                  var bytes = (jsonDecode(snapshot.data!.preview!) as List)
                      .map((entry) => entry as int).toList();
                  Uint8List data = Uint8List.fromList(bytes);
                  if ((_previewer.indexWhere((element) => element.id == letter.id)) < 0) {
                    _PreviewList1 list = _PreviewList1(
                        letter.id, false, data);
                    _previewer.add(list);
                  }
                  print('DBG>letter_list> add preview ${letter.id} length ${_previewer.length}');
                  return Image.memory(data);
                },
              ),
            ),
          ]
      ),
    );
  }

  Stream<Drawing> _loadPreviewStream(Letter letter) {
    final db = Provider.of<LetterDatabase>(context, listen: false);
    late final StreamController<Drawing> controller;
    controller = StreamController<Drawing>(   //.broadcast(
        onListen: () async {
          await db.getDrawing(letter.drawId!).then((drawing)
          => controller.add(drawing));
          await controller.close();
        }
    );
    return controller.stream;
  }

  void _toggleSelectItem(Letter letter) {
    if (_selectedList.contains(letter.id)) {
      setState(() { _selectedList.remove(letter.id); });
    } else {
      setState(() { _selectedList.add(letter.id); });
    }
    widget.onSelectItem();
  }

  void onModifiedFromEditPage(int id) {
    if (id >= 0) {
      print('DBG>letter_list>onModifiedFromEditPage()');
      deletePreview(id);
      setState(() {});
    }
  }

  void deletePreview(int id) {  // letter.id
    _previewer.removeWhere((element) => element.id == id);
    print('DBG>letter_list>delete preview id=$id length=${_previewer.length}');
  }

  // direct call from SummariesPage by key
  void deleteSelected() {
    final db = Provider.of<LetterDatabase>(context, listen: false);
    // =>indexWhere
    for (int id in _selectedList) { // letter.id
      if (id != null) {
        Future<Letter> letter = db.getLetter(id);
        letter.then((targetLetter) {
          if (targetLetter.drawId != null) {
            Future<Drawing> drawing = db.getDrawing(targetLetter.drawId!);
            drawing.then((targetDrawing) {
              db.deleteDrawing(targetDrawing).then((_) =>
                  db.deleteLetter(targetLetter));
            });
          } else {
            db.deleteLetter(targetLetter);
          }
        });
        print('DBG>letter_list>deleteSelected()');
        deletePreview(id);
      }
    }
    _selectedList.clear();
    //widget.onRealignListener();
    setState(() {});
    print('DBG>letter_list2>delete length=${_selectedList.length}');
  }

  // direct call from SummariesPage by key
  get selectedLength => _selectedList.length;
  void clearSelectedLength() {
    setState(() {
      _selectedList.clear();
    });
  }
}