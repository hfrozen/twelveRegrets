//import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:papy/refer/translates.dart';
import 'package:papy/pages/edit.dart';
import 'package:papy/pages/sub/letter_list.dart';

// 검색창에서 키보드가 한번 떠 버리면 편집창으로 갈 때 이 페이지도 다시 그리게한다. 왜???
class SummariesPage extends StatefulWidget {
  const SummariesPage({Key? key, this.forcedEmpty}) : super(key: key);
  final bool? forcedEmpty;

  @override
  _SummariesPageState createState() => _SummariesPageState();
}

class _SummariesPageState extends State<SummariesPage> {
  bool _searchView = false;
  final TextEditingController _searchTextController = TextEditingController();
  final FocusNode _focusNode = FocusNode();
  final _letterListStateKey = GlobalKey<LetterListState>();  // for query
  int _selectCount = 0;

  @override
  void initState() {
    super.initState();
    print('DBG>Summaries>initState() forcedEmpty=${widget.forcedEmpty}');
  }

  @override
  Widget build(BuildContext context) {
    print('DBG>summaries>build() search:$_searchView');

    Widget searchWidget = Row(
      crossAxisAlignment: CrossAxisAlignment.center,
      mainAxisAlignment: MainAxisAlignment.start,
      children: [
        AnimatedOpacity(
          duration: const Duration(milliseconds: 500),
          curve: Curves.fastOutSlowIn,
          opacity: _searchView ? 1.0 : 0.0,
          child: IconButton(
            padding: const EdgeInsets.all(0.0),
            icon: const Icon(Icons.close, size: 24,),
            onPressed: _toggleSearch,
          ),
        ),
        Padding(
          padding: const EdgeInsets.only(top: 8.0, bottom: 8.0),
          child: AnimatedContainer(
            duration: const Duration(milliseconds: 500),
            curve: Curves.fastOutSlowIn,
            height: 50,
            width: _searchView ? MediaQuery.of(context).size.width - 48 : 0.0,
            decoration: BoxDecoration(
              color: Colors.black.withAlpha(50),
              borderRadius: BorderRadius.circular(40.0),
            ),
            child: TextField(
              controller: _searchTextController,
              focusNode: _focusNode,
              decoration: InputDecoration(
                border: InputBorder.none,
                focusedBorder: InputBorder.none,
                enabledBorder: InputBorder.none,
                errorBorder: InputBorder.none,
                disabledBorder: InputBorder.none,
                contentPadding:
                  const EdgeInsets.only(left: 15, bottom: 7, top: 0, right: 15),
                hintText: Localization.of(context)!.tr('typeToSearch')!,
              ),
              onChanged: _onSearchQueryChanged,
            ),
          ),
        ),
      ],
    );
    
    Widget searchIcon = AnimatedOpacity(
      duration: const Duration(milliseconds: 500),
      curve: Curves.fastOutSlowIn,
      opacity: !_searchView ? 1.0 : 0.0,
      child: Align(
        alignment: Alignment.centerRight,
        child: IconButton(
          padding: const EdgeInsets.all(0.0),
          icon: const Icon(Icons.search),
          onPressed: _toggleSearch,
        ),
      ),
    );

    Widget selectedIcon = Align(
      alignment: Alignment.centerRight,
      child: Row(
        children: [
          IconButton(
            padding: const EdgeInsets.all(0),
            icon: const Icon(Icons.delete_outlined),
            onPressed: () => { _deleteSelected() },
          ),
          IconButton(
            padding: const EdgeInsets.all(0),
            icon: const Icon(Icons.close),
            onPressed: () => { _cancelSelected() },
          ),
        ],
      )
    );
    
    return Scaffold(
      appBar: AppBar(
        title: Text(Localization.of(context)!.tr('notepad')!,),
        actions: [
          Stack(
            children: [
              searchWidget,
              _selectCount == 0 ? searchIcon : selectedIcon,
            ],
          ),
        ],
      ),
      body: LetterList(
        key: _letterListStateKey,
        onSelectItem: _onSelectItem,
        onRealignListener: _onRealignListener,
        forcedEmpty: widget.forcedEmpty!,
      ),
      floatingActionButton: !_searchView
        ? FloatingActionButton(
          tooltip: Localization.of(context)!.tr('addNote'),
          onPressed: () => {
            Get.to(() => EditPage(
              letter: null,
              drawing: null,
              onModified: null)
            )
          },
          child: const Icon(Icons.add),
        )
      : null,
    );
  }

  void _toggleSearch() {
    setState(() {
      _searchView = !_searchView;
      _searchTextController.text = '';
      _onSearchQueryChanged('');
      if (_searchView) {
        _focusNode.requestFocus();
      } else {
        FocusScope.of(context).requestFocus(FocusNode());
      }
    });
    print('DBG>summarise>_toggleSearch() _searchView:$_searchView');
  }

  void _onSearchQueryChanged(String query) {
    print('DBG>summaries>_onSearchQueryChanged()');
    if (_letterListStateKey.currentState != null) {
      _letterListStateKey.currentState!.onQueryTextChanged(query);
    }
  }

  void _deleteSelected() {
    print('DBG>summaries>_deleteSelect()');
    _letterListStateKey.currentState!.deleteSelected();
    setState(() {});
  }

  void _cancelSelected() {
    print('DBG>summaries>_cancelSelect()');
    _letterListStateKey.currentState!.clearSelectedLength();
    setState(() {
      _selectCount = 0;
    });
  }

  void _onSelectItem() {
    print('DBG>summaries>_onSelectItem()');
    setState(() {
      _selectCount = _letterListStateKey.currentState!.selectedLength;
    });
  }

  void _onRealignListener() {
    print('DBG>summaries>_onRealgnListener()');
    setState(() {});
  }
}
