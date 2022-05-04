import 'package:flutter/material.dart';
import 'package:zefyrka/zefyrka.dart';
import 'package:papy/tools/painter.dart';

class MultiIconButton extends StatefulWidget {
  final IconData? iconData;
  final double? iconSize;
  final NotusAttribute? attribute;
  final ZefyrController? textController;
  //final bool Function()? getState;
  final Function? getState;
  final PainterController? paintController;
  final Function? doAction;
  final String? tooltip;
  const MultiIconButton({
    Key? key,
    required this.iconData,
    this.iconSize,
    this.attribute,
    this.textController,
    this.getState,
    this.paintController,
    this.doAction,
    this.tooltip,
  }) : super(key: key);

  @override
  _MultiIconButtonState createState() => _MultiIconButtonState();
}

class _MultiIconButtonState extends State<MultiIconButton> {
  bool _isPushed = false;
  late double _iconSize;
  String? _tooltip;

  void _didChangeValue() {
    //print('DBG>multi_button>_didChangeValue()');
    setState(() => _checkPush());
  }

  @override
  void initState() {
    super.initState();
    _iconSize = widget.iconSize ?? 24.0;
    _tooltip = widget.tooltip;
    _checkPush();
    //print('DBG>multi_button>initState() size:$_iconSize ${widget.iconSize}');
    if (widget.textController != null) {
      widget.textController!.addListener(_didChangeValue);
    }
    if (widget.paintController != null) {
      widget.paintController!.addListener(_didChangeValue);
    }
  }

  @override
  void didUpdateWidget(covariant MultiIconButton oldWidget) {
    //print('DBG>multi_button>didUpdateWidget() name:${widget.name}');
    super.didUpdateWidget(oldWidget);
    if (widget.textController != null && //oldWidget != null &&
        oldWidget.textController != widget.textController) {
      if (oldWidget.textController != null) {
        oldWidget.textController!.removeListener(_didChangeValue);
      }
      widget.textController!.addListener(_didChangeValue);
      _checkPush();
    }
    if (widget.paintController != null &&
        oldWidget.paintController != widget.paintController) {
      if (oldWidget.paintController != null) {
        oldWidget.paintController!.removeListener(_didChangeValue);
      }
      widget.paintController!.addListener(_didChangeValue);
      _checkPush();
    }
  }

  @override
  void dispose() {
    print('DBG>multi_button>dispose()');
    if (widget.textController != null) {
      widget.textController!.removeListener(_didChangeValue);
    }
    if (widget.paintController != null) {
      widget.paintController!.removeListener(_didChangeValue);
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    bool isEnabled = true;
    if (widget.attribute != null) {
      final isInCodeBlock =
          widget.textController!.getSelectionStyle()
              .containsSame(NotusAttribute.block.code);
      isEnabled =
          !isInCodeBlock || widget.attribute == NotusAttribute.block.code;
    }
    return _buttonBuilder(context, widget.iconData!,
        _iconSize, _tooltip, _isPushed, isEnabled ? _push : null);
  }

  void _push() {
    if (widget.attribute != null && widget.textController != null) {
      if (_isPushed) {
        if (!widget.attribute!.isUnset) {
          widget.textController!.formatSelection(widget.attribute!.unset);
        }
      } else {
        widget.textController!.formatSelection(widget.attribute!);
      }
    } else if (widget.doAction != null) {
      widget.doAction!.call();
    }
  }

  void _checkPush() {
    if (widget.attribute != null && widget.textController != null) {
      if (widget.attribute!.isUnset) {
        _isPushed = !widget.textController!.getSelectionStyle()
                .contains(widget.attribute!);
      } else {
        _isPushed = widget.textController!.getSelectionStyle()
                .containsSame(widget.attribute!);
      }
    } else if (widget.getState != null) {
      if (widget.iconSize != null) {
        double t = widget.getState!.call();
        _iconSize = t + 10;
        if (widget.tooltip != null) {
          _tooltip = widget.tooltip! + '${t.toInt()}';
        }
        //print('DBG>multi_button>iconSize:$_iconSize');
      } else {
        _isPushed = widget.getState!.call();
      }
    }
  }
}

Widget _buttonBuilder(
    BuildContext context,
    IconData icon,
    double? size,
    String? tooltip,
    bool isPushed, VoidCallback? onPressed,
  ){
  final theme = Theme.of(context);
  final isEnabled = onPressed != null;
  final iconColor = isEnabled
      ? isPushed
            ? Colors.blue
            : theme.canvasColor
      : theme.disabledColor;
  final fillColor = isPushed ? theme.canvasColor : Colors.blue;
  //print('DBG>_buttonBuilder>size:$size');
  return PushableIconButton(
    highlightElevation: 0,
    hoverElevation: 0,
    size: 40,
    icon: Icon(icon, size: size, color: iconColor),
    fillColor: fillColor,
    onPressed: onPressed,
    tooltip: tooltip,
  );
}

class PushableIconButton extends StatelessWidget {
  final VoidCallback? onPressed;
  final Widget? icon;
  final double size;
  final Color? fillColor;
  final String? tooltip;
  final double hoverElevation;
  final double highlightElevation;
  const PushableIconButton({
    Key? key,
    required this.onPressed,
    this.icon,
    this.size = 40,
    this.fillColor,
    this.tooltip,
    this.hoverElevation = 1,
    this.highlightElevation = 1,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    //final key = GlobalKey<State<Tooltip>>();
    Widget _child = ConstrainedBox(
      constraints: BoxConstraints.tightFor(width: size, height: size),
      child: RawMaterialButton(
        visualDensity: VisualDensity.compact,
        shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(2)),
        padding: EdgeInsets.zero,
        fillColor: fillColor,
        elevation: 0,
        hoverElevation: hoverElevation,
        highlightElevation: highlightElevation,
        onPressed: onPressed,
        child: icon,
      ),
    );

    return tooltip != null
      ? Tooltip(
      message: tooltip,
      preferBelow: false,
      showDuration: const Duration(seconds: 1),
      waitDuration: const Duration(milliseconds: 50),
      child: _child,
      )
      : _child;
  }
}
