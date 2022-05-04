import 'package:flutter/material.dart';
import 'package:papy/refer/const.dart';

class ModeMenu extends StatefulWidget {
  ModeMenu({
    Key? key,
    required this.mode,
    required this.share,
    required this.delete,
    required this.onSelectEditMode,
    required this.onPressButton,
    required this.onMore,
  }) : super(key: key);

  EditMode mode;
  bool share;
  bool delete;
  Function onSelectEditMode;
  Function onPressButton;
  void Function() onMore;

  @override
  _ModeMenuState createState() => _ModeMenuState();
}

class _ModeMenuState extends State<ModeMenu> {
  @override
  void initState() {
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      children: [
        IconButton(
          icon: const Icon(Icons.question_mark),
          onPressed: widget.onMore,
        ),
        if (!widget.delete && widget.share)
          IconButton(
            icon: const Icon(Icons.share),
            onPressed: () => widget.onPressButton(EditSubMenu.share),
          ),
        if (widget.delete)
          IconButton(
            icon: const Icon(Icons.delete_outlined),
            onPressed: () => widget.onPressButton(EditSubMenu.delete),
          ),
        if (widget.delete)
          IconButton(
            icon: const Icon(Icons.close),
            onPressed: () => widget.onPressButton(EditSubMenu.close),
          ),
        if (widget.share || widget.delete)
          const SizedBox(width: 32,),
        IconButton(
          icon: Icon(widget.mode == EditMode.text
              ? Icons.text_snippet : Icons.text_snippet_outlined
          ),
          onPressed: () => widget.onSelectEditMode(EditMode.text),
        ),
        IconButton(
          icon: Icon(widget.mode == EditMode.draw
              ? Icons.format_paint : Icons.format_paint_outlined
          ),
          onPressed: () => widget.onSelectEditMode(EditMode.draw),
        ),
        IconButton(
          icon: Icon(widget.mode == EditMode.picture
              ? Icons.picture_in_picture : Icons.picture_in_picture_outlined
          ),
          onPressed: () => widget.onSelectEditMode(EditMode.picture),
        )
      ],
    );
  }
}